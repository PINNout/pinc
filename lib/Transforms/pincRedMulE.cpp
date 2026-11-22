#include "mlir/Dialect/Func/IR/FuncOps.h"
#include "mlir/IR/PatternMatch.h"
#include "mlir/Rewrite/FrozenRewritePatternSet.h"
#include "mlir/Transforms/GreedyPatternRewriteDriver.h"
#include "mlir/Dialect/MemRef/IR/MemRef.h"
#include "mlir/Dialect/EmitC/IR/EmitC.h"
#include "mlir/Dialect/Linalg/IR/Linalg.h"

#include "pinc/pincPasses.h"
#include <mlir/Dialect/Arith/IR/Arith.h>

namespace mlir::pinc {
#define GEN_PASS_DEF_PINCREDMULE
#include "pinc/pincPasses.h.inc"

  namespace {
    class PincMatmulRewriter : public OpRewritePattern<linalg::MatmulOp> {
    public:
      using OpRewritePattern<linalg::MatmulOp>::OpRewritePattern;
      LogicalResult matchAndRewrite(linalg::MatmulOp op,
                                    PatternRewriter &rewriter) const final {
        for (auto operand : op->getOperands()) {
	  auto memrefOperand = dyn_cast<MemRefType>(operand.getType());
          if (!memrefOperand) 
	    return rewriter.notifyMatchFailure(op, "operands are not all MemRef types");
          if (!memrefOperand.getElementType().isF16())
            return rewriter.notifyMatchFailure(op,
                                               "operands are not all F16 type");
          if (!memrefOperand.hasStaticShape() ||
              !(memrefOperand.getShape() == ArrayRef<int64_t>{16, 16}))
            return rewriter.notifyMatchFailure(op,
                                               "operands shape are not 16x16");
        }
        if (op.hasUserDefinedMaps())
          return rewriter.notifyMatchFailure(op, "op does not match the required layout (has defined maps)");

        // passed gauntlet, modify op
        auto dim = rewriter.create<arith::ConstantIntOp>(
            op.getLoc(), rewriter.getI32Type(), 16);
        auto size = rewriter.create<arith::ConstantIntOp>(
            op.getLoc(), rewriter.getI32Type(), 512);
        auto zero = rewriter.create<arith::ConstantIntOp>(
            op.getLoc(), rewriter.getI32Type(), 0);
        auto one = rewriter.create<arith::ConstantIntOp>(
            op.getLoc(), rewriter.getI32Type(), 1);
	auto wfe = rewriter.create<emitc::LiteralOp>(op.getLoc(), rewriter.getI32Type(), "WFE");
	auto hartid = rewriter.create<emitc::CallOpaqueOp>(op.getLoc(), TypeRange{rewriter.getI32Type()}, "get_hartid", ValueRange{});

        auto l1_a = rewriter.create<emitc::CallOpaqueOp>(
            op.getLoc(), TypeRange{rewriter.getI32Type()}, "get_l1_base",
            ValueRange{hartid.getResult(0)});
        auto l1_b = rewriter.create<arith::AddIOp>(
            op.getLoc(), rewriter.getI32Type(), l1_a.getResult(0), size);
        auto l1_c = rewriter.create<arith::AddIOp>(
            op.getLoc(), rewriter.getI32Type(), l1_b, size);
        auto pAf = rewriter.create<memref::ExtractAlignedPointerAsIndexOp>(
            op.getLoc(), op.getOperand(0));
        auto pBf = rewriter.create<memref::ExtractAlignedPointerAsIndexOp>(
            op.getLoc(), op.getOperand(1));
        auto pCf = rewriter.create<memref::ExtractAlignedPointerAsIndexOp>(
            op.getLoc(), op.getOperand(2));
        auto pA = rewriter.create<arith::IndexCastOp>(
            op.getLoc(), rewriter.getI32Type(), pAf);
        auto pB = rewriter.create<arith::IndexCastOp>(
            op.getLoc(), rewriter.getI32Type(), pBf);
        auto pC = rewriter.create<arith::IndexCastOp>(
            op.getLoc(), rewriter.getI32Type(), pCf);

        rewriter.create<emitc::CallOpaqueOp>(
            op.getLoc(),
            TypeRange{},
            "idma_memcpy_1d", ValueRange{zero, pA, l1_a.getResult(0), size});
        rewriter.create<emitc::CallOpaqueOp>(
            op.getLoc(), TypeRange{}, "eu_idma_wait_a2o",
            ValueRange{wfe});

        rewriter.create<emitc::CallOpaqueOp>(
            op.getLoc(),
            TypeRange{},
            "idma_memcpy_1d", ValueRange{zero, pB, l1_b, size});
        rewriter.create<emitc::CallOpaqueOp>(
            op.getLoc(), TypeRange{}, "eu_idma_wait_a2o",
            ValueRange{wfe});

        rewriter.create<emitc::CallOpaqueOp>(
            op.getLoc(),
            TypeRange{},
            "idma_memcpy_1d", ValueRange{zero, pC, l1_c, size});
        rewriter.create<emitc::CallOpaqueOp>(
            op.getLoc(), TypeRange{}, "eu_idma_wait_a2o",
            ValueRange{wfe});

        rewriter.create<emitc::CallOpaqueOp>(
            op.getLoc(),
            TypeRange{},
            "redmule_gemm",
            ValueRange{l1_a.getResult(0), l1_b, l1_c, dim, dim, dim});
        rewriter.create<emitc::CallOpaqueOp>(
            op.getLoc(), TypeRange{}, "eu_redmule_wait",
            ValueRange{wfe});

	rewriter.create<emitc::CallOpaqueOp>(
            op.getLoc(),
            TypeRange{},
            "idma_memcpy_1d", ValueRange{one, pC, l1_c, size});
        rewriter.create<emitc::CallOpaqueOp>(
            op.getLoc(), TypeRange{}, "eu_idma_wait_o2a",
            ValueRange{wfe});
	
	rewriter.eraseOp(op);
        return success();
      };
    };
    class PincRedMulE
      : public impl::PincRedMulEBase<PincRedMulE> {
    public:
      using impl::PincRedMulEBase<PincRedMulE>::PincRedMulEBase;
      void runOnOperation() override {
        OpBuilder builder = OpBuilder::atBlockBegin(getOperation().getBody());
        Location loc = getOperation().getLoc();
        builder.create<emitc::IncludeOp>(loc, "stddef.h", true);
        builder.create<emitc::IncludeOp>(loc, "stdint.h", true);
        builder.create<emitc::IncludeOp>(loc, "tile.h", false);
        builder.create<emitc::IncludeOp>(loc, "idma.h", false);
        builder.create<emitc::IncludeOp>(loc, "redmule.h", false);
        builder.create<emitc::IncludeOp>(loc, "eventunit.h", false);
        RewritePatternSet patterns(&getContext());
        patterns.add<PincMatmulRewriter>(&getContext());
        FrozenRewritePatternSet patternSet(std::move(patterns));
        if (failed(applyPatternsGreedily(getOperation(), patternSet)))
	  signalPassFailure();
      };
    };
  } // namespace
} // namespace mlir::pinc

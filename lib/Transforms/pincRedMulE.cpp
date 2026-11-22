#include "mlir/Dialect/Func/IR/FuncOps.h"
#include "mlir/IR/PatternMatch.h"
#include "mlir/Rewrite/FrozenRewritePatternSet.h"
#include "mlir/Transforms/GreedyPatternRewriteDriver.h"
#include "mlir/Dialect/Arith/IR/ArithDialect.h"
#include "mlir/Dialect/MemRef/IR/MemRefDialect.h"
#include "mlir/Dialect/EmitC/IR/EmitC.h"
#include "mlir/Dialect/Linalg/IR/Linalg.h"

#include "pinc/pincPasses.h"

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
	auto hartid = rewriter.create<emitc::CallOpaqueOp>(op.getLoc(), TypeRange{rewriter.getI32Type()}, "get_hartid", ValueRange{});
	
	auto l1_a = rewriter.create<emitc::CallOpaqueOp>(
							 op.getLoc(), TypeRange{rewriter.getI32Type()}, "get_l1_base",
							 ValueRange{hartid.getResult(0)});
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

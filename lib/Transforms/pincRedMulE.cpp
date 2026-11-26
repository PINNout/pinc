#include "mlir/Dialect/EmitC/IR/EmitC.h"
#include "mlir/Dialect/Func/IR/FuncOps.h"
#include "mlir/Dialect/Linalg/IR/Linalg.h"
#include "mlir/Dialect/MemRef/IR/MemRef.h"
#include "mlir/IR/PatternMatch.h"
#include "mlir/Rewrite/FrozenRewritePatternSet.h"
#include "mlir/Transforms/GreedyPatternRewriteDriver.h"

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
                                PatternRewriter &r) const final {
    for (auto operand : op->getOperands()) {
      auto memrefOperand = dyn_cast<MemRefType>(operand.getType());
      if (!memrefOperand)
        return r.notifyMatchFailure(op, "operands are not all MemRef types");
      if (!memrefOperand.getElementType().isF16())
        return r.notifyMatchFailure(op, "operands are not all F16 type");
      if (!memrefOperand.hasStaticShape() ||
          !(memrefOperand.getShape() == ArrayRef<int64_t>{16, 16}))
        return r.notifyMatchFailure(op, "operands shape are not 16x16");
    }
    if (op.hasUserDefinedMaps())
      return r.notifyMatchFailure(
          op, "op does not match the required layout (has defined maps)");

    // passed gauntlet, modify op
    auto dim = arith::ConstantIntOp::create(r, op.getLoc(), r.getI32Type(), 16);
    auto size =
        arith::ConstantIntOp::create(r, op.getLoc(), r.getI32Type(), 512);
    auto zero = arith::ConstantIntOp::create(r, op.getLoc(), r.getI32Type(), 0);
    auto one = arith::ConstantIntOp::create(r, op.getLoc(), r.getI32Type(), 1);
    auto wfe = emitc::LiteralOp::create(r, op.getLoc(), r.getI32Type(), "WFE");
    auto hartid = emitc::CallOpaqueOp::create(
        r, op.getLoc(), TypeRange{r.getI32Type()}, "get_hartid", ValueRange{});

    auto l1_a = emitc::CallOpaqueOp::create(
        r, op.getLoc(), TypeRange{r.getI32Type()}, "get_l1_base",
        ValueRange{hartid.getResult(0)});
    auto l1_b = arith::AddIOp::create(r, op.getLoc(), r.getI32Type(),
                                      l1_a.getResult(0), size);
    auto l1_c =
        arith::AddIOp::create(r, op.getLoc(), r.getI32Type(), l1_b, size);
    auto pAf = memref::ExtractAlignedPointerAsIndexOp::create(r, op.getLoc(),
                                                              op.getOperand(0));
    auto pBf = memref::ExtractAlignedPointerAsIndexOp::create(r, op.getLoc(),
                                                              op.getOperand(1));
    auto pCf = memref::ExtractAlignedPointerAsIndexOp::create(r, op.getLoc(),
                                                              op.getOperand(2));
    auto pA = arith::IndexCastOp::create(r, op.getLoc(), r.getI32Type(), pAf);
    auto pB = arith::IndexCastOp::create(r, op.getLoc(), r.getI32Type(), pBf);
    auto pC = arith::IndexCastOp::create(r, op.getLoc(), r.getI32Type(), pCf);

    emitc::CallOpaqueOp::create(r, op.getLoc(), TypeRange{}, "idma_memcpy_1d",
                                ValueRange{zero, pA, l1_a.getResult(0), size});
    emitc::CallOpaqueOp::create(r, op.getLoc(), TypeRange{}, "eu_idma_wait_a2o",
                                ValueRange{wfe});

    emitc::CallOpaqueOp::create(r, op.getLoc(), TypeRange{}, "idma_memcpy_1d",
                                ValueRange{zero, pB, l1_b, size});
    emitc::CallOpaqueOp::create(r, op.getLoc(), TypeRange{}, "eu_idma_wait_a2o",
                                ValueRange{wfe});

    emitc::CallOpaqueOp::create(r, op.getLoc(), TypeRange{}, "idma_memcpy_1d",
                                ValueRange{zero, pC, l1_c, size});
    emitc::CallOpaqueOp::create(r, op.getLoc(), TypeRange{}, "eu_idma_wait_a2o",
                                ValueRange{wfe});

    emitc::CallOpaqueOp::create(
        r, op.getLoc(), TypeRange{}, "redmule_gemm",
        ValueRange{l1_a.getResult(0), l1_b, l1_c, dim, dim, dim});
    emitc::CallOpaqueOp::create(r, op.getLoc(), TypeRange{}, "eu_redmule_wait",
                                ValueRange{wfe});

    emitc::CallOpaqueOp::create(r, op.getLoc(), TypeRange{}, "idma_memcpy_1d",
                                ValueRange{one, pC, l1_c, size});
    emitc::CallOpaqueOp::create(r, op.getLoc(), TypeRange{}, "eu_idma_wait_o2a",
                                ValueRange{wfe});

    r.eraseOp(op);
    return success();
  };
};
class PincRedMulE : public impl::PincRedMulEBase<PincRedMulE> {
public:
  using impl::PincRedMulEBase<PincRedMulE>::PincRedMulEBase;
  void runOnOperation() override {
    OpBuilder builder = OpBuilder::atBlockBegin(getOperation().getBody());
    Location loc = getOperation().getLoc();
    emitc::IncludeOp::create(builder, loc, "stddef.h", true);
    emitc::IncludeOp::create(builder, loc, "stdint.h", true);
    emitc::IncludeOp::create(builder, loc, "tile.h", false);
    emitc::IncludeOp::create(builder, loc, "idma.h", false);
    emitc::IncludeOp::create(builder, loc, "redmule.h", false);
    emitc::IncludeOp::create(builder, loc, "eventunit.h", false);
    RewritePatternSet patterns(&getContext());
    patterns.add<PincMatmulRewriter>(&getContext());
    FrozenRewritePatternSet patternSet(std::move(patterns));
    if (failed(applyPatternsGreedily(getOperation(), patternSet)))
      signalPassFailure();
  };
};
} // namespace
} // namespace mlir::pinc

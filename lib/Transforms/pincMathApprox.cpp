#include "pinc/pincPasses.h"
#include <mlir/Dialect/Arith/IR/Arith.h>
#include <mlir/Dialect/Arith/IR/ArithAttributes.h>
#include <mlir/Dialect/Math/IR/Math.h>
#include <mlir/Dialect/Math/Transforms/Passes.h>
#include <mlir/IR/PatternMatch.h>
#include <mlir/Rewrite/FrozenRewritePatternSet.h>
#include <mlir/Transforms/GreedyPatternRewriteDriver.h>

namespace mlir::pinc {
#define GEN_PASS_DEF_PINCMATHAPPROX
#include "pinc/pincPasses.h.inc"

namespace {
class PincMathRewriter : public OpRewritePattern<math::FloorOp> {
public:
  using OpRewritePattern<math::FloorOp>::OpRewritePattern;
  LogicalResult matchAndRewrite(math::FloorOp op,
                                PatternRewriter &r) const final {
    auto t_i = arith::FPToSIOp::create(
        r, op.getLoc(), r.getIntegerType(op.getType().getIntOrFloatBitWidth()),
        op.getOperand());
    auto t = arith::SIToFPOp::create(r, op.getLoc(), op.getType(), t_i);
    auto one = arith::ConstantOp::create(r, op.getLoc(),
                                         r.getFloatAttr(op.getType(), 1.0));
    auto tminus1 = arith::SubFOp::create(r, op.getLoc(), t.getOut(), one);
    auto cond = arith::CmpFOp::create(r, op.getLoc(), arith::CmpFPredicate::OGT,
                                      t, op.getOperand());
    auto res = arith::SelectOp::create(r, op.getLoc(), cond, tminus1, t);
    r.replaceOp(op, res);
    return success();
  };
};

class PincMathApprox : public impl::PincMathApproxBase<PincMathApprox> {
public:
  using impl::PincMathApproxBase<PincMathApprox>::PincMathApproxBase;
  void runOnOperation() override {
    RewritePatternSet patterns(&getContext());
    auto predicate = [](StringRef op) {
      return (math::TanhOp::getOperationName() == op) ||
             (math::SinOp::getOperationName() == op);
    };
    populateMathPolynomialApproximationPatterns(patterns, predicate);
    patterns.add<PincMathRewriter>(&getContext());
    FrozenRewritePatternSet patternSet(std::move(patterns));
    if (failed(applyPatternsGreedily(getOperation(), patternSet)))
      signalPassFailure();
  };
};
} // namespace
} // namespace mlir::pinc

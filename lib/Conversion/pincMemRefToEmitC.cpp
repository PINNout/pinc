#include "mlir/Conversion/MemRefToEmitC/MemRefToEmitC.h"

#include "mlir/Conversion/EmitCCommon/TypeConverter.h"
#include "mlir/Conversion/ConvertToEmitC/ToEmitCInterface.h"
#include "mlir/Dialect/EmitC/IR/EmitC.h"
#include "mlir/Dialect/EmitC/Transforms/TypeConversions.h"
#include "mlir/Dialect/MemRef/IR/MemRef.h"
#include "mlir/IR/PatternMatch.h"
#include "mlir/Transforms/DialectConversion.h"
#include <mlir/Support/LLVM.h>
#include "pinc/pincPasses.h"

namespace mlir::pinc {
#define GEN_PASS_DEF_PINCMEMREFTOEMITC
#include "pinc/pincPasses.h.inc"

  namespace {
    class PincMemRefToEmitCConverter
      : public OpConversionPattern<memref::ExtractAlignedPointerAsIndexOp> {
    public:
      using OpConversionPattern<memref::ExtractAlignedPointerAsIndexOp>::OpConversionPattern;

      LogicalResult
      matchAndRewrite(memref::ExtractAlignedPointerAsIndexOp op,
		      OpAdaptor operands,
		      ConversionPatternRewriter &rewriter) const override {
        auto arrayTy =
            dyn_cast<emitc::ArrayType>(operands.getSource().getType());
        if (!arrayTy)
          return rewriter.notifyMatchFailure(
              op, "Operand(s) are not of the correct type");

        auto zero = rewriter.create<emitc::ConstantOp>(
            op.getLoc(), emitc::SizeTType::get(rewriter.getContext()),
            rewriter.getIndexAttr(0));
	SmallVector<Value> indices(arrayTy.getShape().size(), zero);
        auto sbscr = rewriter.create<emitc::SubscriptOp>(
							 op.getLoc(), emitc::LValueType::get(arrayTy.getElementType()),
            operands.getSource(), indices);

        auto addr = rewriter.create<emitc::AddressOfOp>(
							op.getLoc(), emitc::PointerType::get(arrayTy.getElementType()), sbscr);

        auto cast = rewriter.create<emitc::CastOp>(
            op.getLoc(), getTypeConverter()->convertType(op.getType()), addr);
	
        rewriter.replaceOp(op, cast);
	return success();
      };
    };
    class PincMemRefToEmitC
        : public impl::PincMemRefToEmitCBase<PincMemRefToEmitC> {
    public:
      using impl::PincMemRefToEmitCBase<
          PincMemRefToEmitC>::PincMemRefToEmitCBase;
      void runOnOperation() override {
        EmitCTypeConverter converter(&getContext());
	populateEmitCSizeTTypeConversions(converter);
        RewritePatternSet patterns(&getContext());
        patterns.add<PincMemRefToEmitCConverter>(converter, &getContext());

        ConversionTarget target(getContext());
        target.addIllegalOp<memref::ExtractAlignedPointerAsIndexOp>();
	target.addLegalDialect<emitc::EmitCDialect>();

        if (failed(applyPartialConversion(getOperation(), target,
                                          std::move(patterns))))
	    return signalPassFailure();
      };
    };
  } // namespace
}

#include "mlir/IR/MLIRContext.h"
#include "mlir/InitAllDialects.h"
#include "mlir/InitAllPasses.h"
#include "mlir/InitAllExtensions.h"
#include "mlir/Support/FileUtilities.h"
#include "mlir/Tools/mlir-opt/MlirOptMain.h"
#include "pinc/pincPasses.h"

int main(int argc, char **argv) {
  mlir::registerAllPasses();
  mlir::pinc::registerPasses();

  mlir::DialectRegistry registry;
  mlir::registerAllDialects(registry);
  mlir::registerAllExtensions(registry);

  return mlir::asMainReturnCode(
				mlir::MlirOptMain(argc, argv, "pinc optimiser driver\n", registry));
}

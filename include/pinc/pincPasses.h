#ifndef PINC_PINCPASSES_H
#define PINC_PINCPASSES_H

#include "mlir/Pass/Pass.h"
#include <memory>

namespace mlir {
namespace pinc {
#define GEN_PASS_DECL
#include "pinc/pincPasses.h.inc"

#define GEN_PASS_REGISTRATION
#include "pinc/pincPasses.h.inc"
} // namespace standalone
} // namespace mlir

#endif

add_mlir_library(MLIRpinc
  pincRedMulE.cpp

  ADDITIONAL_HEADER_DIRS
  ${PROJECT_SOURCE_DIR}/include/pinc

  DEPENDS
  MLIRpincPassesIncGen

  LINK_LIBS PUBLIC
  MLIRIR
  MLIRPass
  MLIRFuncDialect
  MLIRArithDialect
  MLIRMemRefDialect
  MLIREmitCDialect
  )

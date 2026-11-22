add_mlir_library(MLIRpincConversion
  pincMemRefToEmitC.cpp

  ADDITIONAL_HEADER_DIRS
  ${PROJECT_SOURCE_DIR}/include/pinc

  DEPENDS
  MLIRpincPassesIncGen

  LINK_LIBS PUBLIC
  MLIRIR
  MLIRPass
  MLIREmitCTransforms
  MLIRTransforms
  MLIRMemRefDialect
  MLIREmitCDialect
  )

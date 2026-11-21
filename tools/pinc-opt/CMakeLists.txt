set(LIBS
  MLIROptLib
  MLIRRegisterAllDialects
  MLIRRegisterAllPasses
  MLIRRegisterAllExtensions
  MLIRpinc
)
add_llvm_executable(pinc-opt pinc-opt.cpp)

llvm_update_compile_flags(pinc-opt)
target_link_libraries(pinc-opt PRIVATE ${LIBS})

mlir_check_all_link_libraries(pinc-opt)

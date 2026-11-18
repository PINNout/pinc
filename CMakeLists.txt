cmake_minimum_required(VERSION 3.20)
project(pinc LANGUAGES CXX C)

set(CMAKE_BUILD_WITH_INSTALL_NAME_DIR ON)

set(CMAKE_CXX_STANDARD 17 CACHE STRING "C++ standard to conform to")

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
  set(EXTERNAL_PROJECT_BUILD FALSE)
  find_package(MLIR REQUIRED CONFIG)

  # Define the default argument to use by `lit` when testing.
  set(LLVM_LIT_ARGS "-sv" CACHE STRING "Default options for lit")

  message(STATUS "Using MLIRConfig.cmake in: ${MLIR_DIR}")
  message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")

  set(LLVM_RUNTIME_OUTPUT_INTDIR ${CMAKE_BINARY_DIR}/bin)
  set(LLVM_LIBRARY_OUTPUT_INTDIR ${CMAKE_BINARY_DIR}/lib)
  set(MLIR_BINARY_DIR ${CMAKE_BINARY_DIR})

  list(APPEND CMAKE_MODULE_PATH "${MLIR_CMAKE_DIR}")
  list(APPEND CMAKE_MODULE_PATH "${LLVM_CMAKE_DIR}")

  # LLVM_DEFINITIONS is a space-separated string of command-line flags (e.g.
  # "-D_GNU_SOURCE -D__STDC_CONSTANT_MACROS"), so split it into a CMake list
  # before passing it to add_definitions(). Do this before including
  # HandleLLVMOptions, which overwrites LLVM_DEFINITIONS in the current scope.
  separate_arguments(LLVM_DEFINITIONS_LIST NATIVE_COMMAND ${LLVM_DEFINITIONS})
  add_definitions(${LLVM_DEFINITIONS_LIST})

  include(TableGen)
  include(AddLLVM)
  include(AddMLIR)
  # Configure C/C++ compiler and linker flags matching the LLVM/MLIR build.
  # Note: If your project builds third-party C++ dependencies from source that
  # do not depend on LLVM/MLIR, add them before including HandleLLVMOptions so
  # they are not affected by LLVM-specific warning, assertion, or linker flags.
  include(HandleLLVMOptions)
endif()

if(MLIR_ENABLE_BINDINGS_PYTHON)
  include(MLIRDetectPythonEnv)
  mlir_configure_python_dev_packages()
endif()

set(PINC_SOURCE_DIR ${PROJECT_SOURCE_DIR})
set(PINC_BINARY_DIR ${PROJECT_BINARY_DIR})
include_directories(${LLVM_INCLUDE_DIRS})
include_directories(${MLIR_INCLUDE_DIRS})
include_directories(${PINC_SOURCE_DIR}/include)
include_directories(${PINC_BINARY_DIR}/include)
link_directories(${LLVM_BUILD_LIBRARY_DIR})

add_subdirectory(tools)

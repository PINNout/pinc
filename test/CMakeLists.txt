llvm_canonicalize_cmake_booleans(
  MLIR_ENABLE_BINDINGS_PYTHON
)

configure_lit_site_cfg(
  ${CMAKE_CURRENT_SOURCE_DIR}/lit.site.cfg.py.in
  ${CMAKE_CURRENT_BINARY_DIR}/lit.site.cfg.py
  MAIN_CONFIG
  ${CMAKE_CURRENT_SOURCE_DIR}/lit.cfg.py
)

set(PINC_TEST_DEPENDS
  FileCheck count not
  pinc-opt
)

#list(APPEND PINC_TEST_DEPENDS pincPlugin)

if(MLIR_ENABLE_BINDINGS_PYTHON)
  list(APPEND PINC_TEST_DEPENDS pincPythonModules)
endif()

add_lit_testsuite(check-pinc "Running the pinc regression tests"
  ${CMAKE_CURRENT_BINARY_DIR}
  DEPENDS ${PINC_TEST_DEPENDS}
)

set_target_properties(check-pinc PROPERTIES FOLDER "Tests")

add_lit_testsuites(PINC ${CMAKE_CURRENT_SOURCE_DIR} DEPENDS ${PINC_TEST_DEPENDS})


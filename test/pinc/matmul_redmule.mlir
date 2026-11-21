// RUN: pinc-opt --redmule %s | FileCheck --implicit-check-not=linalg.matmul %s

// CHECK: stddef.h
// CHECK: stdint.h
// CHECK: tile.h
// CHECK: idma.h
// CHECK: redmule.h
// CHECK: eventunit.h

// CHECK-LABEL: func.func public @main
// CHECK: linalg.fill
// CHECK: idma_memcpy_1d
// CHECK: eu_idma_wait_a2o
// CHECK: idma_memcpy_1d
// CHECK: eu_idma_wait_a2o
// CHECK: idma_memcpy_1d
// CHECK: eu_idma_wait_a2o
// CHECK: redmule_gemm
// CHECK: eu_redmule_wait
// CHECK: idma_memcpy_1d
// CHECK: eu_idma_wait_o2a
func.func public @main(%arg0: memref<16x16xf16>, %arg1: memref<16x16xf16>, %arg2: memref<16x16xf16> {jax.result_info = "result"}) {
%cst = arith.constant 0.000000e+00 : f16
linalg.fill ins(%cst : f16) outs(%arg2 : memref<16x16xf16>)
linalg.matmul ins(%arg0, %arg1 : memref<16x16xf16>, memref<16x16xf16>) outs(%arg2 : memref<16x16xf16>)
return
}

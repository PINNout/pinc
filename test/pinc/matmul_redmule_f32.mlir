// RUN: pinc-opt --pinc-redmule %s | FileCheck %s --implicit-check-not=redmule_gemm --implicit-check-not=idma_memcpy_1d

// CHECK-LABEL: func.func public @main
// CHECK: linalg.fill
// CHECK: linalg.matmul
func.func public @main(%arg0: memref<16x16xf32>, %arg1: memref<16x16xf32>, %arg2: memref<16x16xf32> {jax.result_info = "result"}) {
%cst = arith.constant 0.000000e+00 : f32
linalg.fill ins(%cst : f32) outs(%arg2 : memref<16x16xf32>)
linalg.matmul ins(%arg0, %arg1 : memref<16x16xf32>, memref<16x16xf32>) outs(%arg2 : memref<16x16xf32>)
return
}

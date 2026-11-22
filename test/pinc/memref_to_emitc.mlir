// RUN: pinc-opt \
// RUN:   --convert-to-emitc \
// RUN:   --pinc-memref-to-emitc \
// RUN:   --reconcile-unrealized-casts %s \
// RUN:  | FileCheck %s \
// RUN:   --implicit-check-not=extract_aligned_pointer_as_index \
// RUN:   --implicit-check-not=unrealized_conversion_cast
func.func private @use_addr(i32)

// CHECK-LABEL: emitc.func @main
// CHECK: subscript
// CHECK: address_of
// CHECK: cast {{.*}} : !emitc.ptr<f16>

func.func public @main(%arg0: memref<16x16xf16>){
  %0 = memref.extract_aligned_pointer_as_index %arg0: memref<16x16xf16> -> index
  %1 = arith.index_cast %0: index to i32
  func.call @use_addr(%1) : (i32) -> ()
  return
}

// RUN: pinc-opt \
// RUN:   --pinc-math-approx %s \
// RUN:  | FileCheck %s \
// RUN:   --implicit-check-not=math.tanh \
// RUN:   --implicit-check-not=math.sin \
// RUN:   --implicit-check-not=math.floor

// CHECK-LABEL: @tanh_f32
// CHECK: arith.divf
func.func public @tanh_f32(%arg0: f32) -> f32 {
  %0 = math.tanh %arg0 : f32
  return %0 : f32
}

// CHECK-LABEL: @sin_f32
// CHECK: arith.fptosi
// CHECK: math.fma
func.func public @sin_f32(%arg0: f32) -> f32 { 
  %0 = math.sin %arg0 : f32
  return %0 : f32
}

// CHECK-LABEL: @floor_f32
// CHECK: arith.fptosi
// CHECK: arith.sitofp
// CHECK: arith.select
func.func public @floor_f32(%arg0: f32) -> f32 {
  %0 = math.floor %arg0 : f32
  return %0 : f32
}

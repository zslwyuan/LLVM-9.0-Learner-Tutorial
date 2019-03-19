// RUN: %clang -cc1 %s -triple x86_64-linux-pc -emit-llvm -o - | FileCheck %s -check-prefix LIN
// RUN: %clang -cc1 %s -triple x86_64-windows-pc -emit-llvm -o - | FileCheck %s -check-prefix WIN

// CHECK: @[[VAR:[a-zA-Z0-9_]+]] = internal constant { i65 } { i65 17 }, align 8
// CHECK: @[[ARRAY:[a-zA-Za-zA-Z0-9_]+]] = internal constant [3 x i65] [i65 1, i65 2, i65 3], align 16

// template <int Bits>
// using ap_int = __attribute__((__ap_int(Bits))) unsigned;

template <int Bits>
using ap_int = __attribute__((__ap_int(Bits))) int;

void NewDelete() {
  ap_int<37> *p = new ap_int<37>;
  // LIN: = call i8* @_Znwm(i64 8)
  // WIN: = call i8* @"??2@YAPEAX_K@Z"(i64 8)

  ap_int<37> *p2 = new ap_int<37>[10];
  // LIN: = call i8* @_Znam(i64 80)
  // WIN: = call i8* @"??_U@YAPEAX_K@Z"(i64 80)

  delete[] p2;
  // LIN: call void @_ZdaPv(i8* %{{[0-9]+}})
  // WIN: call void @"??_V@YAXPEAX@Z"(i8* %{{[0-9]+}})

  delete p;
  // LIN: call void @_ZdlPv(i8* %{{[0-9]+}})
  // WIN: call void @"??3@YAXPEAX@Z"(i8* %{{[0-9]+}})
}

typedef int int65_tt __attribute__((__ap_int(65)));
typedef unsigned int uint65_tt __attribute__((__ap_int(65)));
bool SignedConvertToBool() {
  // CHECK: define
  // CHECK: %[[AP_INT:[a-zA-Z0-9_]+]] = alloca i65
  // CHECK: %[[B:[a-zA-Z0-9_]+]] = alloca i8
  // CHECK: %[[FIFTH_BIT:[a-zA-Z0-9_]+]] = alloca i8
  int65_tt an_ap_int = 10;
  bool b = an_ap_int;
  // CHECK: %[[LOAD:[a-zA-Z0-9_]+]] = load i65, i65* %[[AP_INT]]
  // CHECK: %[[TO_BOOL:[a-zA-Z0-9_]+]] = icmp ne i65 %[[LOAD]], 0
  // CHECK: %[[FROM_BOOL:[a-zA-Z0-9_]+]] = zext i1 %[[TO_BOOL]] to i8
  // CHECK: store i8 %[[FROM_BOOL]], i8* %[[B]]

  bool the_5th_bit = (an_ap_int >> 5) & 1;

  // CHECK: %[[LOAD_1:[a-zA-Z0-9_]+]] = load i65, i65* %[[AP_INT]]
  // CHECK: %[[SHR:[a-zA-Z0-9_]+]] = ashr i65 %[[LOAD_1]]
  // CHECK: %[[AND:[a-zA-Z0-9_]+]] = and i65 %[[SHR]]
  // CHECK: %[[TO_BOOL_1:[a-zA-Z0-9_]+]] = icmp ne i65 %[[AND]], 0
  // CHECK: %[[FROM_BOOL_2:[a-zA-Z0-9_]+]] = zext i1 %[[TO_BOOL_1]] to i8
  // CHECK: store i8 %[[FROM_BOOL_2]], i8* %[[FIFTH_BIT]]

  return an_ap_int;
  // CHECK: %[[RET_CONV:[a-zA-Z0-9_]+]] = load i65, i65* %[[AP_INT]]
  // CHECK: %[[TO_BOOL_3:[a-zA-Z0-9_]+]] = icmp ne i65 %[[RET_CONV]], 0
  // CHECK: ret i1 %[[TO_BOOL_3]]
}

bool UnsignedConvertToBool() {
  // CHECK: define
  // CHECK: %[[AP_INT:[a-zA-Z0-9_]+]] = alloca i65
  // CHECK: %[[B:[a-zA-Z0-9_]+]] = alloca i8
  // CHECK: %[[FIFTH_BIT:[a-zA-Z0-9_]+]] = alloca i8
  uint65_tt an_ap_int = 10;
  bool b = an_ap_int;
  // CHECK: %[[LOAD:[a-zA-Z0-9_]+]] = load i65, i65* %[[AP_INT]]
  // CHECK: %[[TO_BOOL:[a-zA-Z0-9_]+]] = icmp ne i65 %[[LOAD]], 0
  // CHECK: %[[FROM_BOOL:[a-zA-Z0-9_]+]] = zext i1 %[[TO_BOOL]] to i8
  // CHECK: store i8 %[[FROM_BOOL]], i8* %[[B]]

  bool the_5th_bit = (an_ap_int >> 5) & 1;

  // CHECK: %[[LOAD_1:[a-zA-Z0-9_]+]] = load i65, i65* %[[AP_INT]]
  // CHECK: %[[SHR:[a-zA-Z0-9_]+]] = lshr i65 %[[LOAD_1]]
  // CHECK: %[[AND:[a-zA-Z0-9_]+]] = and i65 %[[SHR]]
  // CHECK: %[[TO_BOOL_1:[a-zA-Z0-9_]+]] = icmp ne i65 %[[AND]], 0
  // CHECK: %[[FROM_BOOL_2:[a-zA-Z0-9_]+]] = zext i1 %[[TO_BOOL_1]] to i8
  // CHECK: store i8 %[[FROM_BOOL_2]], i8* %[[FIFTH_BIT]]

  return an_ap_int;
  // CHECK: %[[RET_CONV:[a-zA-Z0-9_]+]] = load i65, i65* %[[AP_INT]]
  // CHECK: %[[TO_BOOL_3:[a-zA-Z0-9_]+]] = icmp ne i65 %[[RET_CONV]], 0
  // CHECK: ret i1 %[[TO_BOOL_3]]
}

void OtherBoolConverts() {
  ap_int<5> s(0);
  // CHECK: store i5 0, i5* [[S:%.+]]
  auto t1 = s + true;
  // CHECK: [[S_VAL1:%.+]] = load i5, i5* [[S]]
  // CHECK: [[ADD1:%.+]] = add i5 [[S_VAL1]], 1
  auto t2 = true + s;
  // CHECK: [[S_VAL2:%.+]] = load i5, i5* [[S]]
  // CHECK: [[ADD2:%.+]] = add i5 1, [[S_VAL2]]
  s += true;
  // CHECK: [[S_VAL3:%.+]] = load i5, i5* [[S]]
  // CHECK: [[ADD3:%.+]] = add i5 [[S_VAL3]], 1
  // CHECK: store i5 [[ADD3]], i5* [[S]]

  bool b = true;
  // CHECK: store i8 1, i8* [[B:%.+]]
  b += s;
  // CHECK: [[S_VAL4:%.+]] = load i5, i5* [[S]]
  // CHECK: [[B_VAL:%.+]] = load i8, i8* [[B]]
  // CHECK: [[TO_BOOL1:%.+]] = trunc i8 [[B_VAL]] to i1
  // CHECK: [[CONV1:%.+]] = zext i1 [[TO_BOOL1]] to i5
  // CHECK: [[ADD4:%.+]] = add i5 [[CONV1]], [[S_VAL4]]
  // CHECK: [[TO_BOOL2:%.+]] = icmp ne i5 [[ADD4]]
  // CHECK: [[FROM_BOOL1:%.+]] = zext i1 [[TO_BOOL2]] to i8
  // CHECK: store i8 [[FROM_BOOL1]], i8* [[B]]

  ap_int<2> s1(0);
  // CHECK: store i1 false, i1* [[S1:%.+]]
  auto t3 = s1 + true;
  // CHECK: [[S1_VAL1:%.+]] = load i1, i1* [[S1]]
  // CHECK: [[ADD5:%.+]] = add i1 [[S1_VAL1]], true
  auto t4 = true + s1;
  // CHECK: [[S1_VAL2:%.+]] = load i1, i1* [[S1]]
  // CHECK: [[ADD6:%.+]] = add i1 true, [[S1_VAL2]]
  s1 += true;
  // CHECK: [[S1_VAL3:%.+]] = load i1, i1* [[S1]]
  // CHECK: [[ADD7:%.+]] = add i1 [[S1_VAL3]], true
  // CHECK: store i1 [[ADD7]], i1* [[S1]]

  bool b1 = true;
  // CHECK: store i8 1, i8* [[B1:%.+]]
  b1 += s1;
  // CHECK: [[S1_VAL4:%.+]] = load i1, i1* [[S1]]
  // CHECK: [[B1_VAL:%.+]] = load i8, i8* [[B1]]
  // CHECK: [[TO_BOOL3:%.+]] = trunc i8 [[B1_VAL]] to i1
  // CHECK: [[ADD8:%.+]] = add i1 [[TO_BOOL3]]
  // CHECK: [[TO_BOOL4:%.+]] = icmp ne i1 [[ADD8]]
  // CHECK: [[FROM_BOOL2:%.+]] = zext i1 [[TO_BOOL4]]
  // CHECK: store i8 [[FROM_BOOL2]], i8* [[B1]]
}

//using ap_int77 = unsigned int __attribute__((__ap_int(77)));
using ap_int77 = int __attribute__((__ap_int(77)));

// Constexpr checks:
constexpr int65_tt an_int = 17;

int65_tt int_usage() {
  return an_int;
  //CHECK: ret i65 17
}

constexpr ap_int77 u1 = 15;
constexpr ap_int77 u2 = 4;
constexpr ap_int77 u3 = u1 + u2;
constexpr ap_int77 u4 = u1 - u2;
constexpr ap_int77 u5 = u1 / u2;
constexpr ap_int77 u6 = u1 * u2;
constexpr ap_int77 u7 = u1 % u2;
constexpr ap_int77 u8 = u1 | u2;
constexpr ap_int77 u9 = u1 & u2;
constexpr ap_int77 u0 = u1 ^ u2;
constexpr ap_int77 ua = ~u1;

constexpr ap_int77 s1 = 15;
constexpr ap_int77 s2 = 4;
constexpr ap_int77 s3 = s1 + s2;
constexpr ap_int77 s4 = s1 - s2;
constexpr ap_int77 s5 = s1 / s2;
constexpr ap_int77 s6 = s1 * s2;
constexpr ap_int77 s7 = s1 % s2;
constexpr ap_int77 s8 = s1 | s2;
constexpr ap_int77 s9 = s1 & s2;
constexpr ap_int77 s0 = s1 ^ s2;
constexpr ap_int77 sa = ~s1;

ap_int77 unsigned_usages(int i) {
  switch (i) {
  case 1:
    return u1;
    // CHECK: store i77 15
  case 2:
    return u2;
    // CHECK: store i77 4
  case 3:
    return u3;
    // CHECK: store i77 19
  case 4:
    return u4;
    // CHECK: store i77 11
  case 5:
    return u5;
    // CHECK: store i77 3
  case 6:
    return u6;
    // CHECK: store i77 60
  case 7:
    return u7;
    // CHECK: store i77 3
  case 8:
    return u8;
    // CHECK: store i77 15
  case 9:
    return u9;
    // CHECK: store i77 4
  case 0:
    return u0;
    // CHECK: store i77 11
  case 10:
    return ua;
    // CHECK: store i77 -16
  }
  return 0;
}

ap_int77 signed_usages(int i) {
  switch (i) {
  case 1:
    return s1;
    // CHECK: store i77 15
  case 2:
    return s2;
    // CHECK: store i77 4
  case 3:
    return s3;
    // CHECK: store i77 19
  case 4:
    return s4;
    // CHECK: store i77 11
  case 5:
    return s5;
    // CHECK: store i77 3
  case 6:
    return s6;
    // CHECK: store i77 60
  case 7:
    return s7;
    // CHECK: store i77 3
  case 8:
    return s8;
    // CHECK: store i77 15
  case 9:
    return s9;
    // CHECK: store i77 4
  case 0:
    return s0;
    // CHECK: store i77 11
  case 10:
    return sa;
    // CHECK: store i77 -16
  }
  return 0;
}

// test constexpr constructors
class Base {
  int65_tt an_int;

public:
  constexpr Base(int init) : an_int(init) {}
};

class Derived : public Base {
public:
  constexpr Derived(int init) : Base(init) {}
};

constexpr Derived a_derived = 17;
Derived derrived_usage() {
  return a_derived;
}

// test constexpr array.
constexpr int65_tt an_array[] = {1, 2, 3};

int65_tt array_usage(int idx) {
  return an_array[idx];
}

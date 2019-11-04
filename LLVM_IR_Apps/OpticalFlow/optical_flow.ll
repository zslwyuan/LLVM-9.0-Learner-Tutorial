; ModuleID = 'optical_flow.bc'
source_filename = "optical_flow.bc"
target datalayout = "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v64:64:64-v128:128:128-a0:0:64-s0:64:64-f80:128:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%"class.std::ios_base::Init" = type {}
%struct.ap_uint = type { %struct.ap_int_base }
%struct.ap_int_base = type { %struct.ssdm_int }
%struct.ssdm_int = type { i8 }
%struct.gradient_t = type { float, float, float }
%struct.outer_t = type { [6 x float] }
%struct.tensor_t = type { [6 x float] }
%struct.ap_uint.3 = type { %struct.ap_int_base.4 }
%struct.ap_int_base.4 = type { %struct.ssdm_int.5 }
%struct.ssdm_int.5 = type { i64 }
%"struct.std::nothrow_t" = type {}
%"class.std::locale::id" = type { i64 }
%"class.std::basic_istream" = type { i32 (...)**, i64, %"class.std::basic_ios" }
%"class.std::basic_ios" = type { %"class.std::ios_base", %"class.std::basic_ostream"*, i8, i1, %"class.std::basic_streambuf"*, %"class.std::ctype"*, %"class.std::num_put"*, %"class.std::num_get"* }
%"class.std::ios_base" = type { i32 (...)**, i64, i64, i17, i17, i17, %"struct.std::ios_base::_Callback_list"*, %"struct.std::ios_base::_Words", [8 x %"struct.std::ios_base::_Words"], i32, %"struct.std::ios_base::_Words"*, %"class.std::locale" }
%"struct.std::ios_base::_Callback_list" = type { %"struct.std::ios_base::_Callback_list"*, void (i2, %"class.std::ios_base"*, i32)*, i32, i32 }
%"struct.std::ios_base::_Words" = type { i8*, i64 }
%"class.std::locale" = type { %"class.std::locale::_Impl"* }
%"class.std::locale::_Impl" = type { i32, %"class.std::locale::facet"**, i64, %"class.std::locale::facet"**, i8** }
%"class.std::locale::facet" = type { i32 (...)**, i32 }
%"class.std::basic_ostream" = type { i32 (...)**, %"class.std::basic_ios" }
%"class.std::basic_streambuf" = type { i32 (...)**, i8*, i8*, i8*, i8*, i8*, i8*, %"class.std::locale" }
%"class.std::ctype" = type { %"class.std::locale::facet", %struct.__locale_struct*, i1, i32*, i32*, i16*, i8, [256 x i8], [256 x i8], i8 }
%struct.__locale_struct = type { [13 x %struct.__locale_data*], i16*, i32*, i32*, [13 x i8*] }
%struct.__locale_data = type opaque
%"class.std::num_put" = type { %"class.std::locale::facet" }
%"class.std::num_get" = type { %"class.std::locale::facet" }
%"class.std::basic_istream.6" = type { i32 (...)**, i64, %"class.std::basic_ios.8" }
%"class.std::basic_ios.8" = type { %"class.std::ios_base", %"class.std::basic_ostream.9"*, i32, i1, %"class.std::basic_streambuf.10"*, %"class.std::ctype.11"*, %"class.std::num_put.12"*, %"class.std::num_get.13"* }
%"class.std::basic_ostream.9" = type { i32 (...)**, %"class.std::basic_ios.8" }
%"class.std::basic_streambuf.10" = type { i32 (...)**, i32*, i32*, i32*, i32*, i32*, i32*, %"class.std::locale" }
%"class.std::ctype.11" = type { %"class.std::__ctype_abstract_base", %struct.__locale_struct*, i1, [128 x i8], [256 x i32], [16 x i16], [16 x i64] }
%"class.std::__ctype_abstract_base" = type { %"class.std::locale::facet" }
%"class.std::num_put.12" = type { %"class.std::locale::facet" }
%"class.std::num_get.13" = type { %"class.std::locale::facet" }
%struct._IO_FILE_plus = type opaque
%"class.hls::Window" = type { [5 x [5 x float]] }
%struct.ap_uint.15 = type { %struct.ap_int_base.16 }
%struct.ap_int_base.16 = type { %struct.ssdm_int.17 }
%struct.ssdm_int.17 = type { i32 }
%struct.ap_int_base.18 = type { %struct.ssdm_int.19 }
%struct.ssdm_int.19 = type { i34 }
%"class.hls::LineBuffer" = type { [7 x [1024 x %struct.gradient_t]] }
%"class.hls::Window.0" = type { [1 x [7 x %struct.gradient_t]] }
%"class.hls::LineBuffer.1" = type { [3 x [1024 x %struct.outer_t]] }
%"class.hls::Window.2" = type { [1 x [3 x %struct.tensor_t]] }
%struct.velocity_t = type { float, float }
%struct.ap_range_ref = type { %struct.ap_int_base.4*, i32, i32 }
%struct.ap_int_base.22 = type { %struct.ssdm_int.23 }
%struct.ssdm_int.23 = type { i32 }
%struct.ap_int_base.20 = type { %struct.ssdm_int.21 }
%struct.ssdm_int.21 = type { i1 }

$_ZN7ap_uintILi8EEC1Ev = comdat any

$_ZN3hls6WindowILi5ELi5EfEC1Ev = comdat any

$_ZN3hls6WindowILi5ELi5EfE17shift_pixels_leftEv = comdat any

$_ZN3hls6WindowILi5ELi5EfE12insert_pixelEfii = comdat any

$_ZN3hls6WindowILi5ELi5EfE6getvalEii = comdat any

$_ZN3hls10LineBufferILi7ELi1024E10gradient_tLi0EEC1Ev = comdat any

$_ZN3hls10LineBufferILi7ELi1024E10gradient_tLi0EE15shift_pixels_upEi = comdat any

$_ZN3hls10LineBufferILi7ELi1024E10gradient_tLi0EE17insert_bottom_rowES1_i = comdat any

$_ZN3hls10LineBufferILi7ELi1024E10gradient_tLi0EE6getvalEii = comdat any

$_ZN3hls6WindowILi1ELi7E10gradient_tEC1Ev = comdat any

$_ZN3hls6WindowILi1ELi7E10gradient_tE17shift_pixels_leftEv = comdat any

$_ZN3hls6WindowILi1ELi7E10gradient_tE12insert_pixelES1_ii = comdat any

$_ZN3hls6WindowILi1ELi7E10gradient_tE6getvalEii = comdat any

$_ZN3hls10LineBufferILi3ELi1024E7outer_tLi0EEC1Ev = comdat any

$_ZN3hls10LineBufferILi3ELi1024E7outer_tLi0EE15shift_pixels_upEi = comdat any

$_ZN3hls10LineBufferILi3ELi1024E7outer_tLi0EE17insert_bottom_rowES1_i = comdat any

$_ZN3hls10LineBufferILi3ELi1024E7outer_tLi0EE6getvalEii = comdat any

$_ZN3hls6WindowILi1ELi3E8tensor_tEC1Ev = comdat any

$_ZN3hls6WindowILi1ELi3E8tensor_tE17shift_pixels_leftEv = comdat any

$_ZN3hls6WindowILi1ELi3E8tensor_tE12insert_pixelES1_ii = comdat any

$_ZN3hls6WindowILi1ELi3E8tensor_tE6getvalEii = comdat any

$_ZN7ap_uintILi64EEC1Ev = comdat any

$_ZN7ap_uintILi64EEaSERKS0_ = comdat any

$_ZN11ap_int_baseILi64ELb0ELb1EEclEii = comdat any

$_ZNK12ap_range_refILi64ELb0EEcvyEv = comdat any

$_ZNK12ap_range_refILi64ELb0EE9to_uint64Ev = comdat any

$_ZN7ap_uintILi32EEC1Ev = comdat any

$_ZN7ap_uintILi32EEaSERKS0_ = comdat any

$_ZN7ap_uintILi32EEC1Ei = comdat any

$_ZltILi32ELb0EEbRK11ap_int_baseIXT_EXT0_EXleT_Li64EEEi = comdat any

$_ZNK11ap_int_baseILi32ELb0ELb1EEcvjEv = comdat any

$_ZplILi32ELb0EEN11ap_int_baseIXT_EXT0_EXleT_Li64EEE5RTypeIXLi32EEXLb1EEE4plusERKS1_i = comdat any

$_ZNK11ap_int_baseILi34ELb1ELb1EEcvxEv = comdat any

$_ZN11ap_int_baseILi32ELb0ELb1EEppEi = comdat any

$_ZN11ap_int_baseILi32ELb0ELb1EEpLILi1ELb0EEERS0_RKS_IXT_EXT0_EXleT_Li64EEE = comdat any

$_ZN11ap_int_baseILi1ELb0ELb1EEC1Ei = comdat any

$_ZN11ap_int_baseILi1ELb0ELb1EEC2Ei = comdat any

$_ZN8ssdm_intILi1ELb0EEC2Ev = comdat any

$_ZplILi32ELb0ELi32ELb1EEN11ap_int_baseIXT_EXT0_EXleT_Li64EEE5RTypeIXT1_EXT2_EE4plusERKS1_RKS0_IXT1_EXT2_EXleT1_Li64EEE = comdat any

$_ZN11ap_int_baseILi32ELb1ELb1EEC1Ei = comdat any

$_ZN11ap_int_baseILi32ELb1ELb1EEC2Ei = comdat any

$_ZN8ssdm_intILi32ELb1EEC2Ev = comdat any

$_ZN11ap_int_baseILi34ELb1ELb1EEC1ILi32ELb0EEERKS_IXT_EXT0_EXleT_Li64EEE = comdat any

$_ZN11ap_int_baseILi34ELb1ELb1EEC1ILi32ELb1EEERKS_IXT_EXT0_EXleT_Li64EEE = comdat any

$_ZN11ap_int_baseILi34ELb1ELb1EEC1Ev = comdat any

$_ZN11ap_int_baseILi34ELb1ELb1EEC2Ev = comdat any

$_ZN8ssdm_intILi34ELb1EEC2Ev = comdat any

$_ZN11ap_int_baseILi34ELb1ELb1EEC2ILi32ELb1EEERKS_IXT_EXT0_EXleT_Li64EEE = comdat any

$_ZN11ap_int_baseILi34ELb1ELb1EEC2ILi32ELb0EEERKS_IXT_EXT0_EXleT_Li64EEE = comdat any

$_ZNK11ap_int_baseILi32ELb0ELb1EEltILi32ELb1EEEbRKS_IXT_EXT0_EXleT_Li64EEE = comdat any

$_ZN7ap_uintILi32EEC2Ei = comdat any

$_ZN11ap_int_baseILi32ELb0ELb1EEC2Ev = comdat any

$_ZN8ssdm_intILi32ELb0EEC2Ev = comdat any

$_ZN7ap_uintILi32EEC2Ev = comdat any

$_ZN3hls6WindowILi1ELi3E8tensor_tEC2Ev = comdat any

$_ZN3hls10LineBufferILi3ELi1024E7outer_tLi0EEC2Ev = comdat any

$_ZN3hls6WindowILi1ELi7E10gradient_tEC2Ev = comdat any

$_ZN3hls10LineBufferILi7ELi1024E10gradient_tLi0EEC2Ev = comdat any

$_ZN3hls6WindowILi5ELi5EfEC2Ev = comdat any

$_ZN7ap_uintILi64EEC2Ev = comdat any

$_ZN11ap_int_baseILi64ELb0ELb1EEC2Ev = comdat any

$_ZN8ssdm_intILi64ELb0EEC2Ev = comdat any

$_ZN12ap_range_refILi64ELb0EEC1EP11ap_int_baseILi64ELb0ELb1EEii = comdat any

$_ZN12ap_range_refILi64ELb0EEC2EP11ap_int_baseILi64ELb0ELb1EEii = comdat any

$_ZN7ap_uintILi8EEC2Ev = comdat any

$_ZN11ap_int_baseILi8ELb0ELb1EEC2Ev = comdat any

$_ZN8ssdm_intILi8ELb0EEC2Ev = comdat any

@_ZStL8__ioinit = internal global %"class.std::ios_base::Init" zeroinitializer, align 1
@_ZN18log_apfixed_reduceL2p1E = internal constant i32 4, align 4
@_ZN3hlsL3lutE = internal global [256 x %struct.ap_uint] zeroinitializer, align 16
@.str = private unnamed_addr constant [8 x i8] c"ap_fifo\00", align 1
@.str2 = private unnamed_addr constant [1 x i8] zeroinitializer, align 1
@_ZZ16gradient_xy_calcPA1024_fS0_S0_E3buf = internal global [5 x [1024 x float]] zeroinitializer, align 16
@.str3 = private unnamed_addr constant [9 x i8] c"COMPLETE\00", align 1
@_ZZ16gradient_xy_calcPA1024_fS0_S0_E12GRAD_WEIGHTS = internal constant [5 x i32] [i32 1, i32 -8, i32 0, i32 8, i32 -1], align 16
@.str4 = private unnamed_addr constant [14 x i8] c"GRAD_XY_OUTER\00", align 1
@.str5 = private unnamed_addr constant [14 x i8] c"GRAD_XY_INNER\00", align 1
@.str6 = private unnamed_addr constant [15 x i8] c"GRAD_XY_XYGRAD\00", align 1
@_ZZ15gradient_z_calcPA1024_fS0_S0_S0_S0_S0_E12GRAD_WEIGHTS = internal constant [5 x i32] [i32 1, i32 -8, i32 0, i32 8, i32 -1], align 16
@.str7 = private unnamed_addr constant [13 x i8] c"GRAD_Z_OUTER\00", align 1
@.str8 = private unnamed_addr constant [13 x i8] c"GRAD_Z_INNER\00", align 1
@_ZZ17gradient_weight_yPA1024_fS0_S0_PA1024_10gradient_tE11GRAD_FILTER = internal constant [7 x float] [float 0x3FB353F7C0000000, float 0x3FC10624E0000000, float 0x3FC7EC56E0000000, float 0x3FD2944680000000, float 0x3FC7EC56E0000000, float 0x3FC10624E0000000, float 0x3FB353F7C0000000], align 16
@.str9 = private unnamed_addr constant [20 x i8] c"GRAD_WEIGHT_Y_OUTER\00", align 1
@.str10 = private unnamed_addr constant [20 x i8] c"GRAD_WEIGHT_Y_INNER\00", align 1
@.str11 = private unnamed_addr constant [18 x i8] c"GRAD_WEIGHT_Y_ACC\00", align 1
@_ZZ17gradient_weight_xPA1024_10gradient_tS1_E11GRAD_FILTER = internal constant [7 x float] [float 0x3FB353F7C0000000, float 0x3FC10624E0000000, float 0x3FC7EC56E0000000, float 0x3FD2944680000000, float 0x3FC7EC56E0000000, float 0x3FC10624E0000000, float 0x3FB353F7C0000000], align 16
@.str12 = private unnamed_addr constant [20 x i8] c"GRAD_WEIGHT_X_OUTER\00", align 1
@.str13 = private unnamed_addr constant [20 x i8] c"GRAD_WEIGHT_X_INNER\00", align 1
@.str14 = private unnamed_addr constant [18 x i8] c"GRAD_WEIGHT_X_ACC\00", align 1
@.str15 = private unnamed_addr constant [12 x i8] c"OUTER_OUTER\00", align 1
@.str16 = private unnamed_addr constant [12 x i8] c"OUTER_INNER\00", align 1
@_ZZ15tensor_weight_yPA1024_7outer_tPA1024_8tensor_tE13TENSOR_FILTER = internal constant [3 x float] [float 0x3FD4C154C0000000, float 0x3FD67BB300000000, float 0x3FD4C154C0000000], align 4
@.str17 = private unnamed_addr constant [22 x i8] c"TENSOR_WEIGHT_Y_OUTER\00", align 1
@.str18 = private unnamed_addr constant [22 x i8] c"TENSOR_WEIGHT_Y_INNER\00", align 1
@.str19 = private unnamed_addr constant [25 x i8] c"TENSOR_WEIGHT_Y_TMP_INIT\00", align 1
@.str20 = private unnamed_addr constant [25 x i8] c"TENSOR_WEIGHT_Y_ACC_INIT\00", align 1
@.str21 = private unnamed_addr constant [26 x i8] c"TENSOR_WEIGHT_Y_TMP_OUTER\00", align 1
@.str22 = private unnamed_addr constant [26 x i8] c"TENSOR_WEIGHT_Y_TMP_INNER\00", align 1
@_ZZ15tensor_weight_xPA1024_8tensor_tS1_E13TENSOR_FILTER = internal constant [3 x float] [float 0x3FD4C154C0000000, float 0x3FD67BB300000000, float 0x3FD4C154C0000000], align 4
@.str23 = private unnamed_addr constant [22 x i8] c"TENSOR_WEIGHT_X_OUTER\00", align 1
@.str24 = private unnamed_addr constant [22 x i8] c"TENSOR_WEIGHT_X_INNER\00", align 1
@.str25 = private unnamed_addr constant [25 x i8] c"TENSOR_WEIGHT_X_TMP_INIT\00", align 1
@.str26 = private unnamed_addr constant [25 x i8] c"TENSOR_WEIGHT_X_ACC_INIT\00", align 1
@.str27 = private unnamed_addr constant [26 x i8] c"TENSOR_WEIGHT_X_TMP_OUTER\00", align 1
@.str28 = private unnamed_addr constant [26 x i8] c"TENSOR_WEIGHT_X_TMP_INNER\00", align 1
@_ZZ9flow_calcPA1024_8tensor_tPA1024_10velocity_tE3buf = internal global [2 x float] zeroinitializer, align 4
@.str29 = private unnamed_addr constant [11 x i8] c"FLOW_OUTER\00", align 1
@.str30 = private unnamed_addr constant [11 x i8] c"FLOW_INNER\00", align 1
@_ZZ12optical_flowPA1024_7ap_uintILi64EEPA1024_10velocity_tE10gradient_x = internal global [436 x [1024 x float]] zeroinitializer, align 16
@_ZZ12optical_flowPA1024_7ap_uintILi64EEPA1024_10velocity_tE10gradient_y = internal global [436 x [1024 x float]] zeroinitializer, align 16
@_ZZ12optical_flowPA1024_7ap_uintILi64EEPA1024_10velocity_tE10gradient_z = internal global [436 x [1024 x float]] zeroinitializer, align 16
@_ZZ12optical_flowPA1024_7ap_uintILi64EEPA1024_10velocity_tE10y_filtered = internal global [436 x [1024 x %struct.gradient_t]] zeroinitializer, align 16
@_ZZ12optical_flowPA1024_7ap_uintILi64EEPA1024_10velocity_tE17filtered_gradient = internal global [436 x [1024 x %struct.gradient_t]] zeroinitializer, align 16
@_ZZ12optical_flowPA1024_7ap_uintILi64EEPA1024_10velocity_tE11out_product = internal global [436 x [1024 x %struct.outer_t]] zeroinitializer, align 16
@_ZZ12optical_flowPA1024_7ap_uintILi64EEPA1024_10velocity_tE8tensor_y = internal global [436 x [1024 x %struct.tensor_t]] zeroinitializer, align 16
@_ZZ12optical_flowPA1024_7ap_uintILi64EEPA1024_10velocity_tE6tensor = internal global [436 x [1024 x %struct.tensor_t]] zeroinitializer, align 16
@_ZZ12optical_flowPA1024_7ap_uintILi64EEPA1024_10velocity_tE8frame1_a = internal global [436 x [1024 x float]] zeroinitializer, align 16
@_ZZ12optical_flowPA1024_7ap_uintILi64EEPA1024_10velocity_tE8frame2_a = internal global [436 x [1024 x float]] zeroinitializer, align 16
@_ZZ12optical_flowPA1024_7ap_uintILi64EEPA1024_10velocity_tE8frame4_a = internal global [436 x [1024 x float]] zeroinitializer, align 16
@_ZZ12optical_flowPA1024_7ap_uintILi64EEPA1024_10velocity_tE8frame5_a = internal global [436 x [1024 x float]] zeroinitializer, align 16
@_ZZ12optical_flowPA1024_7ap_uintILi64EEPA1024_10velocity_tE8frame3_a = internal global [436 x [1024 x float]] zeroinitializer, align 16
@_ZZ12optical_flowPA1024_7ap_uintILi64EEPA1024_10velocity_tE8frame3_b = internal global [436 x [1024 x float]] zeroinitializer, align 16
@_ZZ12optical_flowPA1024_7ap_uintILi64EEPA1024_10velocity_tE3buf = internal global %struct.ap_uint.3 zeroinitializer, align 8
@_ZGVZ12optical_flowPA1024_7ap_uintILi64EEPA1024_10velocity_tE3buf = internal global i8 0
@.str31 = private unnamed_addr constant [16 x i8] c"FRAMES_CP_OUTER\00", align 1
@.str32 = private unnamed_addr constant [16 x i8] c"FRAMES_CP_INNER\00", align 1
@signgam = external global i32
@_ZN9__gnu_cxx24__numeric_traits_integer8__digitsE = external constant i32
@_ZN9__gnu_cxx25__numeric_traits_floating14__max_digits10E = external constant i32
@_ZN9__gnu_cxx25__numeric_traits_floating10__digits10E = external constant i32
@_ZN9__gnu_cxx25__numeric_traits_floating16__max_exponent10E = external constant i32
@__daylight = external global i32
@daylight = external global i32
@getdate_err = external global i32
@optarg = external global i8*
@optind = external global i32
@opterr = external global i32
@optopt = external global i32
@_ZSt7nothrow = external global %"struct.std::nothrow_t"
@_ZNSt6locale7_S_onceE = external global i32
@_ZNSt6locale5facet7_S_onceE = external global i32
@_ZNSt6locale2id11_S_refcountE = external global i32
@_ZNSt7collate2idE = external global %"class.std::locale::id"
@_ZNSt8ios_base4Init11_S_refcountE = external global i32
@_ZNSt5ctype2idE = external global %"class.std::locale::id"
@_ZNSt5ctypeIcE2idE = external global %"class.std::locale::id"
@_ZNSt5ctypeIwE2idE = external global %"class.std::locale::id"
@_ZNSt10__num_base12_S_atoms_outE = external global i8*
@_ZNSt10__num_base11_S_atoms_inE = external global i8*
@_ZNSt8numpunct2idE = external global %"class.std::locale::id"
@_ZNSt7num_get2idE = external global %"class.std::locale::id"
@_ZNSt7num_put2idE = external global %"class.std::locale::id"
@_ZSt3cin = external global %"class.std::basic_istream"
@_ZSt4cout = external global %"class.std::basic_ostream"
@_ZSt4cerr = external global %"class.std::basic_ostream"
@_ZSt4clog = external global %"class.std::basic_ostream"
@_ZSt4wcin = external global %"class.std::basic_istream.6"
@_ZSt5wcout = external global %"class.std::basic_ostream.9"
@_ZSt5wcerr = external global %"class.std::basic_ostream.9"
@_ZSt5wclog = external global %"class.std::basic_ostream.9"
@_IO_2_1_stdin_ = external global %struct._IO_FILE_plus
@_IO_2_1_stdout_ = external global %struct._IO_FILE_plus
@_IO_2_1_stderr_ = external global %struct._IO_FILE_plus
@sys_nerr = external global i32
@_sys_nerr = external global i32
@_ZN8LowerTri5dummyE = external constant i32
@_ZN8LowerTriIXT_ELj0EE5dummyE = external constant i32
@_ZN11LowerTriDim5dummyE = external constant i32
@_ZN11LowerTriDim6dummy2E = external constant i32
@_ZN11LowerTriDim6dummy3E = external constant i32
@_ZN11LowerTriDimIXT_EXT0_ELi0EE5dummyE = external constant i32
@_ZN11LowerTriDimIXT_EXT0_ELi0EE6dummy2E = external constant i32
@_ZN11LowerTriDimIXT_EXT0_ELi0EE6dummy3E = external constant i32
@.str33 = private unnamed_addr constant [49 x i8] c"row >= 0 && row < ROWS && col >= 0 && col < COLS\00", align 1
@.str34 = private unnamed_addr constant [81 x i8] c"/opt/Xilinx_201802/Vivado/2018.2/common/technology/autopilot/hls/hls_video_mem.h\00", align 1
@__PRETTY_FUNCTION__._ZN3hls6WindowILi1ELi3E8tensor_tE6getvalEii = private unnamed_addr constant [84 x i8] c"T &hls::Window<1, 3, tensor_t>::getval(int, int) [ROWS = 1, COLS = 3, T = tensor_t]\00", align 1
@__PRETTY_FUNCTION__._ZN3hls6WindowILi1ELi3E8tensor_tE12insert_pixelES1_ii = private unnamed_addr constant [95 x i8] c"void hls::Window<1, 3, tensor_t>::insert_pixel(T, int, int) [ROWS = 1, COLS = 3, T = tensor_t]\00", align 1
@.str35 = private unnamed_addr constant [12 x i8] c"hls_label_6\00", align 1
@.str36 = private unnamed_addr constant [12 x i8] c"hls_label_7\00", align 1
@__PRETTY_FUNCTION__._ZN3hls10LineBufferILi3ELi1024E7outer_tLi0EE6getvalEii = private unnamed_addr constant [108 x i8] c"T &hls::LineBuffer<3, 1024, outer_t, 0>::getval(int, int) [ROWS = 3, COLS = 1024, T = outer_t, RESHAPE = 0]\00", align 1
@.str37 = private unnamed_addr constant [23 x i8] c"col >= 0 && col < COLS\00", align 1
@__PRETTY_FUNCTION__._ZN3hls10LineBufferILi3ELi1024E7outer_tLi0EE17insert_bottom_rowES1_i = private unnamed_addr constant [119 x i8] c"void hls::LineBuffer<3, 1024, outer_t, 0>::insert_bottom_row(T, int) [ROWS = 3, COLS = 1024, T = outer_t, RESHAPE = 0]\00", align 1
@__PRETTY_FUNCTION__._ZN3hls10LineBufferILi3ELi1024E7outer_tLi0EE15shift_pixels_upEi = private unnamed_addr constant [114 x i8] c"void hls::LineBuffer<3, 1024, outer_t, 0>::shift_pixels_up(int) [ROWS = 3, COLS = 1024, T = outer_t, RESHAPE = 0]\00", align 1
@.str38 = private unnamed_addr constant [12 x i8] c"hls_label_5\00", align 1
@__PRETTY_FUNCTION__._ZN3hls6WindowILi1ELi7E10gradient_tE6getvalEii = private unnamed_addr constant [88 x i8] c"T &hls::Window<1, 7, gradient_t>::getval(int, int) [ROWS = 1, COLS = 7, T = gradient_t]\00", align 1
@__PRETTY_FUNCTION__._ZN3hls6WindowILi1ELi7E10gradient_tE12insert_pixelES1_ii = private unnamed_addr constant [99 x i8] c"void hls::Window<1, 7, gradient_t>::insert_pixel(T, int, int) [ROWS = 1, COLS = 7, T = gradient_t]\00", align 1
@.str39 = private unnamed_addr constant [12 x i8] c"hls_label_3\00", align 1
@.str40 = private unnamed_addr constant [12 x i8] c"hls_label_4\00", align 1
@__PRETTY_FUNCTION__._ZN3hls10LineBufferILi7ELi1024E10gradient_tLi0EE6getvalEii = private unnamed_addr constant [114 x i8] c"T &hls::LineBuffer<7, 1024, gradient_t, 0>::getval(int, int) [ROWS = 7, COLS = 1024, T = gradient_t, RESHAPE = 0]\00", align 1
@__PRETTY_FUNCTION__._ZN3hls10LineBufferILi7ELi1024E10gradient_tLi0EE17insert_bottom_rowES1_i = private unnamed_addr constant [125 x i8] c"void hls::LineBuffer<7, 1024, gradient_t, 0>::insert_bottom_row(T, int) [ROWS = 7, COLS = 1024, T = gradient_t, RESHAPE = 0]\00", align 1
@__PRETTY_FUNCTION__._ZN3hls10LineBufferILi7ELi1024E10gradient_tLi0EE15shift_pixels_upEi = private unnamed_addr constant [120 x i8] c"void hls::LineBuffer<7, 1024, gradient_t, 0>::shift_pixels_up(int) [ROWS = 7, COLS = 1024, T = gradient_t, RESHAPE = 0]\00", align 1
@.str41 = private unnamed_addr constant [12 x i8] c"hls_label_2\00", align 1
@__PRETTY_FUNCTION__._ZN3hls6WindowILi5ELi5EfE6getvalEii = private unnamed_addr constant [78 x i8] c"T &hls::Window<5, 5, float>::getval(int, int) [ROWS = 5, COLS = 5, T = float]\00", align 1
@__PRETTY_FUNCTION__._ZN3hls6WindowILi5ELi5EfE12insert_pixelEfii = private unnamed_addr constant [89 x i8] c"void hls::Window<5, 5, float>::insert_pixel(T, int, int) [ROWS = 5, COLS = 5, T = float]\00", align 1
@.str42 = private unnamed_addr constant [12 x i8] c"hls_label_0\00", align 1
@.str43 = private unnamed_addr constant [12 x i8] c"hls_label_1\00", align 1
@llvm.global_ctors = appending global [1 x { i32, void ()* }] [{ i32, void ()* } { i32 65535, void ()* @_GLOBAL__I_a }]

; Function Attrs: nounwind
define internal void @__cxx_global_var_init() #0 section ".text.startup" {
  call void @_ZNSt8ios_base4InitC1Ev(%"class.std::ios_base::Init"* @_ZStL8__ioinit)
  %1 = call i32 @atexit(void ()* @__dtor__ZStL8__ioinit)
  ret void
}

declare void @_ZNSt8ios_base4InitC1Ev(%"class.std::ios_base::Init"*)

declare void @_ZNSt8ios_base4InitD1Ev(%"class.std::ios_base::Init"*)

; Function Attrs: nounwind
define internal void @__dtor__ZStL8__ioinit() #0 section ".text.startup" {
  call void @_ZNSt8ios_base4InitD1Ev(%"class.std::ios_base::Init"* @_ZStL8__ioinit)
  ret void
}

; Function Attrs: nounwind
declare i32 @atexit(void ()*) #0

; Function Attrs: nounwind
define internal void @__cxx_global_var_init1() #0 section ".text.startup" {
  br label %1

; <label>:1:                                      ; preds = %1, %0
  %2 = phi %struct.ap_uint* [ getelementptr inbounds ([256 x %struct.ap_uint], [256 x %struct.ap_uint]* @_ZN3hlsL3lutE, i32 0, i32 0), %0 ], [ %3, %1 ]
  call void @_ZN7ap_uintILi8EEC1Ev(%struct.ap_uint* %2)
  %3 = getelementptr inbounds %struct.ap_uint, %struct.ap_uint* %2, i64 1
  %4 = icmp eq %struct.ap_uint* %3, getelementptr inbounds ([256 x %struct.ap_uint], [256 x %struct.ap_uint]* @_ZN3hlsL3lutE, i64 1, i64 0)
  br i1 %4, label %5, label %1

; <label>:5:                                      ; preds = %1
  ret void
}

; Function Attrs: alwaysinline inlinehint nounwind uwtable
define linkonce_odr void @_ZN7ap_uintILi8EEC1Ev(%struct.ap_uint* %this) unnamed_addr #1 comdat align 2 {
  %1 = alloca %struct.ap_uint*, align 8
  store %struct.ap_uint* %this, %struct.ap_uint** %1, align 8
  %2 = load %struct.ap_uint*, %struct.ap_uint** %1
  call void @_ZN7ap_uintILi8EEC2Ev(%struct.ap_uint* %2)
  ret void
}

; Function Attrs: nounwind uwtable
define void @_Z16gradient_xy_calcPA1024_fS0_S0_([1024 x float]* %frame, [1024 x float]* %gradient_x, [1024 x float]* %gradient_y) #2 {
  %1 = alloca [1024 x float]*, align 8
  %2 = alloca [1024 x float]*, align 8
  %3 = alloca [1024 x float]*, align 8
  %smallbuf = alloca [5 x float], align 16
  %window = alloca %"class.hls::Window", align 4
  %r = alloca i32, align 4
  %c = alloca i32, align 4
  %i = alloca i32, align 4
  %i1 = alloca i32, align 4
  %i2 = alloca i32, align 4
  %i3 = alloca i32, align 4
  %x_grad = alloca float, align 4
  %y_grad = alloca float, align 4
  %i4 = alloca i32, align 4
  store [1024 x float]* %frame, [1024 x float]** %1, align 8
  store [1024 x float]* %gradient_x, [1024 x float]** %2, align 8
  store [1024 x float]* %gradient_y, [1024 x float]** %3, align 8
  %4 = load [1024 x float]*, [1024 x float]** %1, align 8
  call void (...) @_ssdm_SpecArrayDimSize([1024 x float]* %4, i32 436) #0
  %5 = load [1024 x float]*, [1024 x float]** %2, align 8
  call void (...) @_ssdm_SpecArrayDimSize([1024 x float]* %5, i32 436) #0
  %6 = load [1024 x float]*, [1024 x float]** %3, align 8
  call void (...) @_ssdm_SpecArrayDimSize([1024 x float]* %6, i32 436) #0
  %7 = load [1024 x float]*, [1024 x float]** %1, align 8
  call void (...) @_ssdm_op_SpecInterface([1024 x float]* %7, i8* getelementptr inbounds ([8 x i8], [8 x i8]* @.str, i32 0, i32 0), i32 0, i32 0, i8* getelementptr inbounds ([1 x i8], [1 x i8]* @.str2, i32 0, i32 0), i32 0, i32 0, i8* getelementptr inbounds ([1 x i8], [1 x i8]* @.str2, i32 0, i32 0), i8* getelementptr inbounds ([1 x i8], [1 x i8]* @.str2, i32 0, i32 0), i8* getelementptr inbounds ([1 x i8], [1 x i8]* @.str2, i32 0, i32 0), i32 0, i32 0, i32 0, i32 0, i8* getelementptr inbounds ([1 x i8], [1 x i8]* @.str2, i32 0, i32 0), i8* getelementptr inbounds ([1 x i8], [1 x i8]* @.str2, i32 0, i32 0)) #0
  %8 = load [1024 x float]*, [1024 x float]** %2, align 8
  call void (...) @_ssdm_op_SpecInterface([1024 x float]* %8, i8* getelementptr inbounds ([8 x i8], [8 x i8]* @.str, i32 0, i32 0), i32 0, i32 0, i8* getelementptr inbounds ([1 x i8], [1 x i8]* @.str2, i32 0, i32 0), i32 0, i32 0, i8* getelementptr inbounds ([1 x i8], [1 x i8]* @.str2, i32 0, i32 0), i8* getelementptr inbounds ([1 x i8], [1 x i8]* @.str2, i32 0, i32 0), i8* getelementptr inbounds ([1 x i8], [1 x i8]* @.str2, i32 0, i32 0), i32 0, i32 0, i32 0, i32 0, i8* getelementptr inbounds ([1 x i8], [1 x i8]* @.str2, i32 0, i32 0), i8* getelementptr inbounds ([1 x i8], [1 x i8]* @.str2, i32 0, i32 0)) #0
  %9 = load [1024 x float]*, [1024 x float]** %3, align 8
  call void (...) @_ssdm_op_SpecInterface([1024 x float]* %9, i8* getelementptr inbounds ([8 x i8], [8 x i8]* @.str, i32 0, i32 0), i32 0, i32 0, i8* getelementptr inbounds ([1 x i8], [1 x i8]* @.str2, i32 0, i32 0), i32 0, i32 0, i8* getelementptr inbounds ([1 x i8], [1 x i8]* @.str2, i32 0, i32 0), i8* getelementptr inbounds ([1 x i8], [1 x i8]* @.str2, i32 0, i32 0), i8* getelementptr inbounds ([1 x i8], [1 x i8]* @.str2, i32 0, i32 0), i32 0, i32 0, i32 0, i32 0, i8* getelementptr inbounds ([1 x i8], [1 x i8]* @.str2, i32 0, i32 0), i8* getelementptr inbounds ([1 x i8], [1 x i8]* @.str2, i32 0, i32 0)) #0
  call void (...) @_ssdm_SpecArrayPartition([5 x [1024 x float]]* @_ZZ16gradient_xy_calcPA1024_fS0_S0_E3buf, i32 1, i8* getelementptr inbounds ([9 x i8], [9 x i8]* @.str3, i32 0, i32 0), i32 0, i8* getelementptr inbounds ([1 x i8], [1 x i8]* @.str2, i32 0, i32 0)) #0
  %10 = getelementptr inbounds [5 x float], [5 x float]* %smallbuf, i32 0, i32 0
  call void (...) @_ssdm_SpecArrayPartition(float* %10, i32 0, i8* getelementptr inbounds ([9 x i8], [9 x i8]* @.str3, i32 0, i32 0), i32 0, i8* getelementptr inbounds ([1 x i8], [1 x i8]* @.str2, i32 0, i32 0)) #0
  call void @_ZN3hls6WindowILi5ELi5EfEC1Ev(%"class.hls::Window"* %window)
  call void (...) @_ssdm_SpecConstant(i32* getelementptr inbounds ([5 x i32], [5 x i32]* @_ZZ16gradient_xy_calcPA1024_fS0_S0_E12GRAD_WEIGHTS, i32 0, i32 0)) #0
  br label %11

; <label>:11:                                     ; preds = %0
  store i32 0, i32* %r, align 4
  br label %12

; <label>:12:                                     ; preds = %237, %11
  %13 = load i32, i32* %r, align 4
  %14 = icmp slt i32 %13, 438
  br i1 %14, label %15, label %240

; <label>:15:                                     ; preds = %12
  call void (...) @_ssdm_op_SpecLoopName(i8* getelementptr inbounds ([14 x i8], [14 x i8]* @.str4, i32 0, i32 0)) #0
  call void (...) @_ssdm_RegionBegin(i8* getelementptr inbounds ([14 x i8], [14 x i8]* @.str4, i32 0, i32 0)) #0
  br label %16

; <label>:16:                                     ; preds = %15
  store i32 0, i32* %c, align 4
  br label %17

; <label>:17:                                     ; preds = %233, %16
  %18 = load i32, i32* %c, align 4
  %19 = icmp slt i32 %18, 1026
  br i1 %19, label %20, label %236

; <label>:20:                                     ; preds = %17
  call void (...) @_ssdm_op_SpecLoopName(i8* getelementptr inbounds ([14 x i8], [14 x i8]* @.str5, i32 0, i32 0)) #0
  call void (...) @_ssdm_RegionBegin(i8* getelementptr inbounds ([14 x i8], [14 x i8]* @.str5, i32 0, i32 0)) #0
  call void (...) @_ssdm_op_SpecPipeline(i32 1, i32 1, i32 1, i32 0, i8* getelementptr inbounds ([1 x i8], [1 x i8]* @.str2, i32 0, i32 0)) #0
  call void (...) @_ssdm_Unroll(i32 1, i32 0, i32 2, i8* getelementptr inbounds ([1 x i8], [1 x i8]* @.str2, i32 0, i32 0)) #0
  store i32 0, i32* %i, align 4
  br label %21

; <label>:21:                                     ; preds = %36, %20
  %22 = load i32, i32* %i, align 4
  %23 = icmp slt i32 %22, 4
  br i1 %23, label %24, label %39

; <label>:24:                                     ; preds = %21
  %25 = load i32, i32* %c, align 4
  %26 = sext i32 %25 to i64
  %27 = load i32, i32* %i, align 4
  %28 = add nsw i32 %27, 1
  %29 = sext i32 %28 to i64
  %30 = getelementptr inbounds [5 x [1024 x float]], [5 x [1024 x float]]* @_ZZ16gradient_xy_calcPA1024_fS0_S0_E3buf, i32 0, i64 %29
  %31 = getelementptr inbounds [1024 x float], [1024 x float]* %30, i32 0, i64 %26
  %32 = load float, float* %31, align 4
  %33 = load i32, i32* %i, align 4
  %34 = sext i32 %33 to i64
  %35 = getelementptr inbounds [5 x float], [5 x float]* %smallbuf, i32 0, i64 %34
  store float %32, float* %35, align 4
  br label %36

; <label>:36:                                     ; preds = %24
  %37 = load i32, i32* %i, align 4
  %38 = add nsw i32 %37, 1
  store i32 %38, i32* %i, align 4
  br label %21

; <label>:39:                                     ; preds = %21
  %40 = load i32, i32* %r, align 4
  %41 = icmp slt i32 %40, 436
  br i1 %41, label %42, label %55

; <label>:42:                                     ; preds = %39
  %43 = load i32, i32* %c, align 4
  %44 = icmp slt i32 %43, 1024
  br i1 %44, label %45, label %55

; <label>:45:                                     ; preds = %42
  %46 = load i32, i32* %c, align 4
  %47 = sext i32 %46 to i64
  %48 = load i32, i32* %r, align 4
  %49 = sext i32 %48 to i64
  %50 = load [1024 x float]*, [1024 x float]** %1, align 8
  %51 = getelementptr inbounds [1024 x float], [1024 x float]* %50, i64 %49
  %52 = getelementptr inbounds [1024 x float], [1024 x float]* %51, i32 0, i64 %47
  %53 = load float, float* %52, align 4
  %54 = getelementptr inbounds [5 x float], [5 x float]* %smallbuf, i32 0, i64 4
  store float %53, float* %54, align 4
  br label %61

; <label>:55:                                     ; preds = %42, %39
  %56 = load i32, i32* %c, align 4
  %57 = icmp slt i32 %56, 1024
  br i1 %57, label %58, label %60

; <label>:58:                                     ; preds = %55
  %59 = getelementptr inbounds [5 x float], [5 x float]* %smallbuf, i32 0, i64 4
  store float 0.000000e+00, float* %59, align 4
  br label %60

; <label>:60:                                     ; preds = %58, %55
  br label %61

; <label>:61:                                     ; preds = %60, %45
  %62 = load i32, i32* %r, align 4
  %63 = icmp slt i32 %62, 436
  br i1 %63, label %64, label %91

; <label>:64:                                     ; preds = %61
  %65 = load i32, i32* %c, align 4
  %66 = icmp slt i32 %65, 1024
  br i1 %66, label %67, label %91

; <label>:67:                                     ; preds = %64
  store i32 0, i32* %i1, align 4
  br label %68

; <label>:68:                                     ; preds = %82, %67
  %69 = load i32, i32* %i1, align 4
  %70 = icmp slt i32 %69, 4
  br i1 %70, label %71, label %85

; <label>:71:                                     ; preds = %68
  %72 = load i32, i32* %i1, align 4
  %73 = sext i32 %72 to i64
  %74 = getelementptr inbounds [5 x float], [5 x float]* %smallbuf, i32 0, i64 %73
  %75 = load float, float* %74, align 4
  %76 = load i32, i32* %c, align 4
  %77 = sext i32 %76 to i64
  %78 = load i32, i32* %i1, align 4
  %79 = sext i32 %78 to i64
  %80 = getelementptr inbounds [5 x [1024 x float]], [5 x [1024 x float]]* @_ZZ16gradient_xy_calcPA1024_fS0_S0_E3buf, i32 0, i64 %79
  %81 = getelementptr inbounds [1024 x float], [1024 x float]* %80, i32 0, i64 %77
  store float %75, float* %81, align 4
  br label %82

; <label>:82:                                     ; preds = %71
  %83 = load i32, i32* %i1, align 4
  %84 = add nsw i32 %83, 1
  store i32 %84, i32* %i1, align 4
  br label %68

; <label>:85:                                     ; preds = %68
  %86 = getelementptr inbounds [5 x float], [5 x float]* %smallbuf, i32 0, i64 4
  %87 = load float, float* %86, align 4
  %88 = load i32, i32* %c, align 4
  %89 = sext i32 %88 to i64
  %90 = getelementptr inbounds [1024 x float], [1024 x float]* getelementptr inbounds ([5 x [1024 x float]], [5 x [1024 x float]]* @_ZZ16gradient_xy_calcPA1024_fS0_S0_E3buf, i32 0, i64 4), i32 0, i64 %89
  store float %87, float* %90, align 4
  br label %119

; <label>:91:                                     ; preds = %64, %61
  %92 = load i32, i32* %c, align 4
  %93 = icmp slt i32 %92, 1024
  br i1 %93, label %94, label %118

; <label>:94:                                     ; preds = %91
  store i32 0, i32* %i2, align 4
  br label %95

; <label>:95:                                     ; preds = %109, %94
  %96 = load i32, i32* %i2, align 4
  %97 = icmp slt i32 %96, 4
  br i1 %97, label %98, label %112

; <label>:98:                                     ; preds = %95
  %99 = load i32, i32* %i2, align 4
  %100 = sext i32 %99 to i64
  %101 = getelementptr inbounds [5 x float], [5 x float]* %smallbuf, i32 0, i64 %100
  %102 = load float, float* %101, align 4
  %103 = load i32, i32* %c, align 4
  %104 = sext i32 %103 to i64
  %105 = load i32, i32* %i2, align 4
  %106 = sext i32 %105 to i64
  %107 = getelementptr inbounds [5 x [1024 x float]], [5 x [1024 x float]]* @_ZZ16gradient_xy_calcPA1024_fS0_S0_E3buf, i32 0, i64 %106
  %108 = getelementptr inbounds [1024 x float], [1024 x float]* %107, i32 0, i64 %104
  store float %102, float* %108, align 4
  br label %109

; <label>:109:                                    ; preds = %98
  %110 = load i32, i32* %i2, align 4
  %111 = add nsw i32 %110, 1
  store i32 %111, i32* %i2, align 4
  br label %95

; <label>:112:                                    ; preds = %95
  %113 = getelementptr inbounds [5 x float], [5 x float]* %smallbuf, i32 0, i64 4
  %114 = load float, float* %113, align 4
  %115 = load i32, i32* %c, align 4
  %116 = sext i32 %115 to i64
  %117 = getelementptr inbounds [1024 x float], [1024 x float]* getelementptr inbounds ([5 x [1024 x float]], [5 x [1024 x float]]* @_ZZ16gradient_xy_calcPA1024_fS0_S0_E3buf, i32 0, i64 4), i32 0, i64 %116
  store float %114, float* %117, align 4
  br label %118

; <label>:118:                                    ; preds = %112, %91
  br label %119

; <label>:119:                                    ; preds = %118, %85
  %120 = load i32, i32* %r, align 4
  %121 = icmp slt i32 %120, 436
  br i1 %121, label %122, label %139

; <label>:122:                                    ; preds = %119
  %123 = load i32, i32* %c, align 4
  %124 = icmp slt i32 %123, 1024
  br i1 %124, label %125, label %139

; <label>:125:                                    ; preds = %122
  call void @_ZN3hls6WindowILi5ELi5EfE17shift_pixels_leftEv(%"class.hls::Window"* %window)
  store i32 0, i32* %i3, align 4
  br label %126

; <label>:126:                                    ; preds = %135, %125
  %127 = load i32, i32* %i3, align 4
  %128 = icmp slt i32 %127, 5
  br i1 %128, label %129, label %138

; <label>:129:                                    ; preds = %126
  %130 = load i32, i32* %i3, align 4
  %131 = sext i32 %130 to i64
  %132 = getelementptr inbounds [5 x float], [5 x float]* %smallbuf, i32 0, i64 %131
  %133 = load float, float* %132, align 4
  %134 = load i32, i32* %i3, align 4
  call void @_ZN3hls6WindowILi5ELi5EfE12insert_pixelEfii(%"class.hls::Window"* %window, float %133, i32 %134, i32 4)
  br label %135

; <label>:135:                                    ; preds = %129
  %136 = load i32, i32* %i3, align 4
  %137 = add nsw i32 %136, 1
  store i32 %137, i32* %i3, align 4
  br label %126

; <label>:138:                                    ; preds = %126
  br label %140

; <label>:139:                                    ; preds = %122, %119
  call void @_ZN3hls6WindowILi5ELi5EfE17shift_pixels_leftEv(%"class.hls::Window"* %window)
  call void @_ZN3hls6WindowILi5ELi5EfE12insert_pixelEfii(%"class.hls::Window"* %window, float 0.000000e+00, i32 0, i32 4)
  call void @_ZN3hls6WindowILi5ELi5EfE12insert_pixelEfii(%"class.hls::Window"* %window, float 0.000000e+00, i32 1, i32 4)
  call void @_ZN3hls6WindowILi5ELi5EfE12insert_pixelEfii(%"class.hls::Window"* %window, float 0.000000e+00, i32 2, i32 4)
  call void @_ZN3hls6WindowILi5ELi5EfE12insert_pixelEfii(%"class.hls::Window"* %window, float 0.000000e+00, i32 3, i32 4)
  call void @_ZN3hls6WindowILi5ELi5EfE12insert_pixelEfii(%"class.hls::Window"* %window, float 0.000000e+00, i32 4, i32 4)
  br label %140

; <label>:140:                                    ; preds = %139, %138
  store float 0.000000e+00, float* %x_grad, align 4
  store float 0.000000e+00, float* %y_grad, align 4
  %141 = load i32, i32* %r, align 4
  %142 = icmp sge i32 %141, 4
  br i1 %142, label %143, label %206

; <label>:143:                                    ; preds = %140
  %144 = load i32, i32* %r, align 4
  %145 = icmp slt i32 %144, 436
  br i1 %145, label %146, label %206

; <label>:146:                                    ; preds = %143
  %147 = load i32, i32* %c, align 4
  %148 = icmp sge i32 %147, 4
  br i1 %148, label %149, label %206

; <label>:149:                                    ; preds = %146
  %150 = load i32, i32* %c, align 4
  %151 = icmp slt i32 %150, 1024
  br i1 %151, label %152, label %206

; <label>:152:                                    ; preds = %149
  br label %153

; <label>:153:                                    ; preds = %152
  store i32 0, i32* %i4, align 4
  br label %154

; <label>:154:                                    ; preds = %180, %153
  %155 = load i32, i32* %i4, align 4
  %156 = icmp slt i32 %155, 5
  br i1 %156, label %157, label %183

; <label>:157:                                    ; preds = %154
  call void (...) @_ssdm_op_SpecLoopName(i8* getelementptr inbounds ([15 x i8], [15 x i8]* @.str6, i32 0, i32 0)) #0
  call void (...) @_ssdm_RegionBegin(i8* getelementptr inbounds ([15 x i8], [15 x i8]* @.str6, i32 0, i32 0)) #0
  %158 = load i32, i32* %i4, align 4
  %159 = call float* @_ZN3hls6WindowILi5ELi5EfE6getvalEii(%"class.hls::Window"* %window, i32 2, i32 %158)
  %160 = load float, float* %159
  %161 = load i32, i32* %i4, align 4
  %162 = sext i32 %161 to i64
  %163 = getelementptr inbounds [5 x i32], [5 x i32]* @_ZZ16gradient_xy_calcPA1024_fS0_S0_E12GRAD_WEIGHTS, i32 0, i64 %162
  %164 = load i32, i32* %163, align 4
  %165 = sitofp i32 %164 to float
  %166 = fmul float %160, %165
  %167 = load float, float* %x_grad, align 4
  %168 = fadd float %167, %166
  store float %168, float* %x_grad, align 4
  %169 = load i32, i32* %i4, align 4
  %170 = call float* @_ZN3hls6WindowILi5ELi5EfE6getvalEii(%"class.hls::Window"* %window, i32 %169, i32 2)
  %171 = load float, float* %170
  %172 = load i32, i32* %i4, align 4
  %173 = sext i32 %172 to i64
  %174 = getelementptr inbounds [5 x i32], [5 x i32]* @_ZZ16gradient_xy_calcPA1024_fS0_S0_E12GRAD_WEIGHTS, i32 0, i64 %173
  %175 = load i32, i32* %174, align 4
  %176 = sitofp i32 %175 to float
  %177 = fmul float %171, %176
  %178 = load float, float* %y_grad, align 4
  %179 = fadd float %178, %177
  store float %179, float* %y_grad, align 4
  call void (...) @_ssdm_RegionEnd(i8* getelementptr inbounds ([15 x i8], [15 x i8]* @.str6, i32 0, i32 0)) #0
  br label %180

; <label>:180:                                    ; preds = %157
  %181 = load i32, i32* %i4, align 4
  %182 = add nsw i32 %181, 1
  store i32 %182, i32* %i4, align 4
  br label %154

; <label>:183:                                    ; preds = %154
  %184 = load float, float* %x_grad, align 4
  %185 = fdiv float %184, 1.200000e+01
  %186 = load i32, i32* %c, align 4
  %187 = sub nsw i32 %186, 2
  %188 = sext i32 %187 to i64
  %189 = load i32, i32* %r, align 4
  %190 = sub nsw i32 %189, 2
  %191 = sext i32 %190 to i64
  %192 = load [1024 x float]*, [1024 x float]** %2, align 8
  %193 = getelementptr inbounds [1024 x float], [1024 x float]* %192, i64 %191
  %194 = getelementptr inbounds [1024 x float], [1024 x float]* %193, i32 0, i64 %188
  store float %185, float* %194, align 4
  %195 = load float, float* %y_grad, align 4
  %196 = fdiv float %195, 1.200000e+01
  %197 = load i32, i32* %c, align 4
  %198 = sub nsw i32 %197, 2
  %199 = sext i32 %198 to i64
  %200 = load i32, i32* %r, align 4
  %201 = sub nsw i32 %200, 2
  %202 = sext i32 %201 to i64
  %203 = load [1024 x float]*, [1024 x float]** %3, align 8
  %204 = getelementptr inbounds [1024 x float], [1024 x float]* %203, i64 %202
  %205 = getelementptr inbounds [1024 x float], [1024 x float]* %204, i32 0, i64 %199
  store float %196, float* %205, align 4
  br label %232

; <label>:206:                                    ; preds = %149, %146, %143, %140
  %207 = load i32, i32* %r, align 4
  %208 = icmp sge i32 %207, 2
  br i1 %208, label %209, label %231

; <label>:209:                                    ; preds = %206
  %210 = load i32, i32* %c, align 4
  %211 = icmp sge i32 %210, 2
  br i1 %211, label %212, label %231

; <label>:212:                                    ; preds = %209
  %213 = load i32, i32* %c, align 4
  %214 = sub nsw i32 %213, 2
  %215 = sext i32 %214 to i64
  %216 = load i32, i32* %r, align 4
  %217 = sub nsw i32 %216, 2
  %218 = sext i32 %217 to i64
  %219 = load [1024 x float]*, [1024 x float]** %2, align 8
  %220 = getelementptr inbounds [1024 x float], [1024 x float]* %219, i64 %218
  %221 = getelementptr inbounds [1024 x float], [1024 x float]* %220, i32 0, i64 %215
  store float 0.000000e+00, float* %221, align 4
  %222 = load i32, i32* %c, align 4
  %223 = sub nsw i32 %222, 2
  %224 = sext i32 %223 to i64
  %225 = load i32, i32* %r, align 4
  %226 = sub nsw i32 %225, 2
  %227 = sext i32 %226 to i64
  %228 = load [1024 x float]*, [1024 x float]** %3, align 8
  %229 = getelementptr inbounds [1024 x float], [1024 x float]* %228, i64 %227
  %230 = getelementptr inbounds [1024 x float], [1024 x float]* %229, i32 0, i64 %224
  store float 0.000000e+00, float* %230, align 4
  br label %231

; <label>:231:                                    ; preds = %212, %209, %206
  br label %232

; <label>:232:                                    ; preds = %231, %183
  call void (...) @_ssdm_RegionEnd(i8* getelementptr inbounds ([14 x i8], [14 x i8]* @.str5, i32 0, i32 0)) #0
  br label %233

; <label>:233:                                    ; preds = %232
  %234 = load i32, i32* %c, align 4
  %235 = add nsw i32 %234, 1
  store i32 %235, i32* %c, align 4
  br label %17

; <label>:236:                                    ; preds = %17
  call void (...) @_ssdm_RegionEnd(i8* getelementptr inbounds ([14 x i8], [14 x i8]* @.str4, i32 0, i32 0)) #0
  br label %237

; <label>:237:                                    ; preds = %236
  %238 = load i32, i32* %r, align 4
  %239 = add nsw i32 %238, 1
  store i32 %239, i32* %r, align 4
  br label %12

; <label>:240:                                    ; preds = %12
  ret void
}

; Function Attrs: nounwind readnone speculatable
declare void @llvm.dbg.declare(metadata, metadata) #3

; Function Attrs: nounwind
declare void @_ssdm_SpecArrayDimSize(...) #0

; Function Attrs: nounwind
declare void @_ssdm_op_SpecInterface(...) #0

; Function Attrs: nounwind
declare void @_ssdm_SpecArrayPartition(...) #0

; Function Attrs: nounwind uwtable
define linkonce_odr void @_ZN3hls6WindowILi5ELi5EfEC1Ev(%"class.hls::Window"* %this) unnamed_addr #2 comdat align 2 {
  %1 = alloca %"class.hls::Window"*, align 8
  store %"class.hls::Window"* %this, %"class.hls::Window"** %1, align 8
  %2 = load %"class.hls::Window"*, %"class.hls::Window"** %1
  call void @_ZN3hls6WindowILi5ELi5EfEC2Ev(%"class.hls::Window"* %2)
  ret void
}

; Function Attrs: nounwind
declare void @_ssdm_SpecConstant(...) #0

; Function Attrs: nounwind
declare void @_ssdm_op_SpecLoopName(...) #0

; Function Attrs: nounwind
declare void @_ssdm_RegionBegin(...) #0

; Function Attrs: nounwind
declare void @_ssdm_op_SpecPipeline(...) #0

; Function Attrs: nounwind
declare void @_ssdm_Unroll(...) #0

; Function Attrs: nounwind uwtable
define linkonce_odr void @_ZN3hls6WindowILi5ELi5EfE17shift_pixels_leftEv(%"class.hls::Window"* %this) #2 comdat align 2 {
  %1 = alloca %"class.hls::Window"*, align 8
  %i = alloca %struct.ap_uint.15, align 4
  %j = alloca %struct.ap_uint.15, align 4
  %2 = alloca %struct.ap_uint.15, align 4
  %3 = alloca %struct.ap_uint.15, align 4
  %4 = alloca %struct.ap_int_base.18, align 8
  %5 = alloca %struct.ap_int_base.16, align 4
  %6 = alloca %struct.ap_int_base.16, align 4
  store %"class.hls::Window"* %this, %"class.hls::Window"** %1, align 8
  %7 = load %"class.hls::Window"*, %"class.hls::Window"** %1
  call void (...) @_ssdm_InlineSelf(i32 0, i8* getelementptr inbounds ([1 x i8], [1 x i8]* @.str2, i32 0, i32 0)) #0
  call void @_ZN7ap_uintILi32EEC1Ev(%struct.ap_uint.15* %i)
  call void @_ZN7ap_uintILi32EEC1Ev(%struct.ap_uint.15* %j)
  call void @_ZN7ap_uintILi32EEC1Ei(%struct.ap_uint.15* %2, i32 0)
  %8 = call %struct.ap_uint.15* @_ZN7ap_uintILi32EEaSERKS0_(%struct.ap_uint.15* %i, %struct.ap_uint.15* %2)
  br label %9

; <label>:9:                                      ; preds = %39, %0
  %10 = bitcast %struct.ap_uint.15* %i to %struct.ap_int_base.16*
  %11 = call zeroext i1 @_ZltILi32ELb0EEbRK11ap_int_baseIXT_EXT0_EXleT_Li64EEEi(%struct.ap_int_base.16* %10, i32 5)
  br i1 %11, label %12, label %41

; <label>:12:                                     ; preds = %9
  call void (...) @_ssdm_RegionBegin(i8* getelementptr inbounds ([12 x i8], [12 x i8]* @.str42, i32 0, i32 0)) #0
  call void (...) @_ssdm_Unroll(i32 0, i32 0, i32 0, i8* getelementptr inbounds ([1 x i8], [1 x i8]* @.str2, i32 0, i32 0)) #0
  call void @_ZN7ap_uintILi32EEC1Ei(%struct.ap_uint.15* %3, i32 0)
  %13 = call %struct.ap_uint.15* @_ZN7ap_uintILi32EEaSERKS0_(%struct.ap_uint.15* %j, %struct.ap_uint.15* %3)
  br label %14

; <label>:14:                                     ; preds = %36, %12
  %15 = bitcast %struct.ap_uint.15* %j to %struct.ap_int_base.16*
  %16 = call zeroext i1 @_ZltILi32ELb0EEbRK11ap_int_baseIXT_EXT0_EXleT_Li64EEEi(%struct.ap_int_base.16* %15, i32 4)
  br i1 %16, label %17, label %38

; <label>:17:                                     ; preds = %14
  call void (...) @_ssdm_RegionBegin(i8* getelementptr inbounds ([12 x i8], [12 x i8]* @.str43, i32 0, i32 0)) #0
  call void (...) @_ssdm_Unroll(i32 0, i32 0, i32 0, i8* getelementptr inbounds ([1 x i8], [1 x i8]* @.str2, i32 0, i32 0)) #0
  %18 = bitcast %struct.ap_uint.15* %j to %struct.ap_int_base.16*
  call void @_ZplILi32ELb0EEN11ap_int_baseIXT_EXT0_EXleT_Li64EEE5RTypeIXLi32EEXLb1EEE4plusERKS1_i(%struct.ap_int_base.18* sret %4, %struct.ap_int_base.16* %18, i32 1)
  %19 = call i64 @_ZNK11ap_int_baseILi34ELb1ELb1EEcvxEv(%struct.ap_int_base.18* %4)
  %20 = bitcast %struct.ap_uint.15* %i to %struct.ap_int_base.16*
  %21 = call i32 @_ZNK11ap_int_baseILi32ELb0ELb1EEcvjEv(%struct.ap_int_base.16* %20)
  %22 = zext i32 %21 to i64
  %23 = getelementptr inbounds %"class.hls::Window", %"class.hls::Window"* %7, i32 0, i32 0
  %24 = getelementptr inbounds [5 x [5 x float]], [5 x [5 x float]]* %23, i32 0, i64 %22
  %25 = getelementptr inbounds [5 x float], [5 x float]* %24, i32 0, i64 %19
  %26 = load float, float* %25, align 4
  %27 = bitcast %struct.ap_uint.15* %j to %struct.ap_int_base.16*
  %28 = call i32 @_ZNK11ap_int_baseILi32ELb0ELb1EEcvjEv(%struct.ap_int_base.16* %27)
  %29 = zext i32 %28 to i64
  %30 = bitcast %struct.ap_uint.15* %i to %struct.ap_int_base.16*
  %31 = call i32 @_ZNK11ap_int_baseILi32ELb0ELb1EEcvjEv(%struct.ap_int_base.16* %30)
  %32 = zext i32 %31 to i64
  %33 = getelementptr inbounds %"class.hls::Window", %"class.hls::Window"* %7, i32 0, i32 0
  %34 = getelementptr inbounds [5 x [5 x float]], [5 x [5 x float]]* %33, i32 0, i64 %32
  %35 = getelementptr inbounds [5 x float], [5 x float]* %34, i32 0, i64 %29
  store float %26, float* %35, align 4
  call void (...) @_ssdm_RegionEnd(i8* getelementptr inbounds ([12 x i8], [12 x i8]* @.str43, i32 0, i32 0)) #0
  br label %36

; <label>:36:                                     ; preds = %17
  %37 = bitcast %struct.ap_uint.15* %j to %struct.ap_int_base.16*
  call void @_ZN11ap_int_baseILi32ELb0ELb1EEppEi(%struct.ap_int_base.16* sret %5, %struct.ap_int_base.16* %37, i32 0)
  br label %14

; <label>:38:                                     ; preds = %14
  call void (...) @_ssdm_RegionEnd(i8* getelementptr inbounds ([12 x i8], [12 x i8]* @.str42, i32 0, i32 0)) #0
  br label %39

; <label>:39:                                     ; preds = %38
  %40 = bitcast %struct.ap_uint.15* %i to %struct.ap_int_base.16*
  call void @_ZN11ap_int_baseILi32ELb0ELb1EEppEi(%struct.ap_int_base.16* sret %6, %struct.ap_int_base.16* %40, i32 0)
  br label %9

; <label>:41:                                     ; preds = %9
  ret void
}

; Function Attrs: nounwind uwtable
define linkonce_odr void @_ZN3hls6WindowILi5ELi5EfE12insert_pixelEfii(%"class.hls::Window"* %this, float %value, i32 %row, i32 %col) #2 comdat align 2 {
  %1 = alloca %"class.hls::Window"*, align 8
  %2 = alloca float, align 4
  %3 = alloca i32, align 4
  %4 = alloca i32, align 4
  store %"class.hls::Window"* %this, %"class.hls::Window"** %1, align 8
  store float %value, float* %2, align 4
  store i32 %row, i32* %3, align 4
  store i32 %col, i32* %4, align 4
  %5 = load %"class.hls::Window"*, %"class.hls::Window"** %1
  call void (...) @_ssdm_InlineSelf(i32 0, i8* getelementptr inbounds ([1 x i8], [1 x i8]* @.str2, i32 0, i32 0)) #0
  %6 = load i32, i32* %3, align 4
  %7 = icmp sge i32 %6, 0
  br i1 %7, label %8, label %17

; <label>:8:                                      ; preds = %0
  %9 = load i32, i32* %3, align 4
  %10 = icmp slt i32 %9, 5
  br i1 %10, label %11, label %17

; <label>:11:                                     ; preds = %8
  %12 = load i32, i32* %4, align 4
  %13 = icmp sge i32 %12, 0
  br i1 %13, label %14, label %17

; <label>:14:                                     ; preds = %11
  %15 = load i32, i32* %4, align 4
  %16 = icmp slt i32 %15, 5
  br label %17

; <label>:17:                                     ; preds = %14, %11, %8, %0
  %18 = phi i1 [ false, %11 ], [ false, %8 ], [ false, %0 ], [ %16, %14 ]
  br i1 %18, label %19, label %20

; <label>:19:                                     ; preds = %17
  br label %22

; <label>:20:                                     ; preds = %17
  call void @__assert_fail(i8* getelementptr inbounds ([49 x i8], [49 x i8]* @.str33, i32 0, i32 0), i8* getelementptr inbounds ([81 x i8], [81 x i8]* @.str34, i32 0, i32 0), i32 266, i8* getelementptr inbounds ([89 x i8], [89 x i8]* @__PRETTY_FUNCTION__._ZN3hls6WindowILi5ELi5EfE12insert_pixelEfii, i32 0, i32 0)) #5
  unreachable
                                                  ; No predecessors!
  br label %22

; <label>:22:                                     ; preds = %21, %19
  %23 = load float, float* %2, align 4
  %24 = load i32, i32* %4, align 4
  %25 = sext i32 %24 to i64
  %26 = load i32, i32* %3, align 4
  %27 = sext i32 %26 to i64
  %28 = getelementptr inbounds %"class.hls::Window", %"class.hls::Window"* %5, i32 0, i32 0
  %29 = getelementptr inbounds [5 x [5 x float]], [5 x [5 x float]]* %28, i32 0, i64 %27
  %30 = getelementptr inbounds [5 x float], [5 x float]* %29, i32 0, i64 %25
  store float %23, float* %30, align 4
  ret void
}

; Function Attrs: nounwind uwtable
define linkonce_odr float* @_ZN3hls6WindowILi5ELi5EfE6getvalEii(%"class.hls::Window"* %this, i32 %row, i32 %col) #2 comdat align 2 {
  %1 = alloca %"class.hls::Window"*, align 8
  %2 = alloca i32, align 4
  %3 = alloca i32, align 4
  store %"class.hls::Window"* %this, %"class.hls::Window"** %1, align 8
  store i32 %row, i32* %2, align 4
  store i32 %col, i32* %3, align 4
  %4 = load %"class.hls::Window"*, %"class.hls::Window"** %1
  call void (...) @_ssdm_InlineSelf(i32 0, i8* getelementptr inbounds ([1 x i8], [1 x i8]* @.str2, i32 0, i32 0)) #0
  %5 = load i32, i32* %2, align 4
  %6 = icmp sge i32 %5, 0
  br i1 %6, label %7, label %16

; <label>:7:                                      ; preds = %0
  %8 = load i32, i32* %2, align 4
  %9 = icmp slt i32 %8, 5
  br i1 %9, label %10, label %16

; <label>:10:                                     ; preds = %7
  %11 = load i32, i32* %3, align 4
  %12 = icmp sge i32 %11, 0
  br i1 %12, label %13, label %16

; <label>:13:                                     ; preds = %10
  %14 = load i32, i32* %3, align 4
  %15 = icmp slt i32 %14, 5
  br label %16

; <label>:16:                                     ; preds = %13, %10, %7, %0
  %17 = phi i1 [ false, %10 ], [ false, %7 ], [ false, %0 ], [ %15, %13 ]
  br i1 %17, label %18, label %19

; <label>:18:                                     ; preds = %16
  br label %21

; <label>:19:                                     ; preds = %16
  call void @__assert_fail(i8* getelementptr inbounds ([49 x i8], [49 x i8]* @.str33, i32 0, i32 0), i8* getelementptr inbounds ([81 x i8], [81 x i8]* @.str34, i32 0, i32 0), i32 492, i8* getelementptr inbounds ([78 x i8], [78 x i8]* @__PRETTY_FUNCTION__._ZN3hls6WindowILi5ELi5EfE6getvalEii, i32 0, i32 0)) #5
  unreachable
                                                  ; No predecessors!
  br label %21

; <label>:21:                                     ; preds = %20, %18
  %22 = load i32, i32* %3, align 4
  %23 = sext i32 %22 to i64
  %24 = load i32, i32* %2, align 4
  %25 = sext i32 %24 to i64
  %26 = getelementptr inbounds %"class.hls::Window", %"class.hls::Window"* %4, i32 0, i32 0
  %27 = getelementptr inbounds [5 x [5 x float]], [5 x [5 x float]]* %26, i32 0, i64 %25
  %28 = getelementptr inbounds [5 x float], [5 x float]* %27, i32 0, i64 %23
  ret float* %28
}

; Function Attrs: nounwind
declare void @_ssdm_RegionEnd(...) #0

; Function Attrs: nounwind uwtable
define void @_Z15gradient_z_calcPA1024_fS0_S0_S0_S0_S0_([1024 x float]* %frame1, [1024 x float]* %frame2, [1024 x float]* %frame3, [1024 x float]* %frame4, [1024 x float]* %frame5, [1024 x float]* %gradient_z) #2 {
  %1 = alloca [1024 x float]*, align 8
  %2 = alloca [1024 x float]*, align 8
  %3 = alloca [1024 x float]*, align 8
  %4 = alloca [1024 x float]*, align 8
  %5 = alloca [1024 x float]*, align 8
  %6 = alloca [1024 x float]*, align 8
  %r = alloca i32, align 4
  %c = alloca i32, align 4
  store [1024 x float]* %frame1, [1024 x float]** %1, align 8
  store [1024 x float]* %frame2, [1024 x float]** %2, align 8
  store [1024 x float]* %frame3, [1024 x float]** %3, align 8
  store [1024 x float]* %frame4, [1024 x float]** %4, align 8
  store [1024 x float]* %frame5, [1024 x float]** %5, align 8
  store [1024 x float]* %gradient_z, [1024 x float]** %6, align 8
  %7 = load [1024 x float]*, [1024 x float]** %1, align 8
  call void (...) @_ssdm_SpecArrayDimSize([1024 x float]* %7, i32 436) #0
  %8 = load [1024 x float]*, [1024 x float]** %2, align 8
  call void (...) @_ssdm_SpecArrayDimSize([1024 x float]* %8, i32 436) #0
  %9 = load [1024 x float]*, [1024 x float]** %3, align 8
  call void (...) @_ssdm_SpecArrayDimSize([1024 x float]* %9, i32 436) #0
  %10 = load [1024 x float]*, [1024 x float]** %4, align 8
  call void (...) @_ssdm_SpecArrayDimSize([1024 x float]* %10, i32 436) #0
  %11 = load [1024 x float]*, [1024 x float]** %5, align 8
  call void (...) @_ssdm_SpecArrayDimSize([1024 x float]* %11, i32 436) #0
  %12 = load [1024 x float]*, [1024 x float]** %6, align 8
  call void (...) @_ssdm_SpecArrayDimSize([1024 x float]* %12, i32 436) #0
  %13 = load [1024 x float]*, [1024 x float]** %1, align 8
  call void (...) @_ssdm_op_SpecInterface([1024 x float]* %13, i8* getelementptr inbounds ([8 x i8], [8 x i8]* @.str, i32 0, i32 0), i32 0, i32 0, i8* getelementptr inbounds ([1 x i8], [1 x i8]* @.str2, i32 0, i32 0), i32 0, i32 0, i8* getelementptr inbounds ([1 x i8], [1 x i8]* @.str2, i32 0, i32 0), i8* getelementptr inbounds ([1 x i8], [1 x i8]* @.str2, i32 0, i32 0), i8* getelementptr inbounds ([1 x i8], [1 x i8]* @.str2, i32 0, i32 0), i32 0, i32 0, i32 0, i32 0, i8* getelementptr inbounds ([1 x i8], [1 x i8]* @.str2, i32 0, i32 0), i8* getelementptr inbounds ([1 x i8], [1 x i8]* @.str2, i32 0, i32 0)) #0
  %14 = load [1024 x float]*, [1024 x float]** %2, align 8
  call void (...) @_ssdm_op_SpecInterface([1024 x float]* %14, i8* getelementptr inbounds ([8 x i8], [8 x i8]* @.str, i32 0, i32 0), i32 0, i32 0, i8* getelementptr inbounds ([1 x i8], [1 x i8]* @.str2, i32 0, i32 0), i32 0, i32 0, i8* getelementptr inbounds ([1 x i8], [1 x i8]* @.str2, i32 0, i32 0), i8* getelementptr inbounds ([1 x i8], [1 x i8]* @.str2, i32 0, i32 0), i8* getelementptr inbounds ([1 x i8], [1 x i8]* @.str2, i32 0, i32 0), i32 0, i32 0, i32 0, i32 0, i8* getelementptr inbounds ([1 x i8], [1 x i8]* @.str2, i32 0, i32 0), i8* getelementptr inbounds ([1 x i8], [1 x i8]* @.str2, i32 0, i32 0)) #0
  %15 = load [1024 x float]*, [1024 x float]** %3, align 8
  call void (...) @_ssdm_op_SpecInterface([1024 x float]* %15, i8* getelementptr inbounds ([8 x i8], [8 x i8]* @.str, i32 0, i32 0), i32 0, i32 0, i8* getelementptr inbounds ([1 x i8], [1 x i8]* @.str2, i32 0, i32 0), i32 0, i32 0, i8* getelementptr inbounds ([1 x i8], [1 x i8]* @.str2, i32 0, i32 0), i8* getelementptr inbounds ([1 x i8], [1 x i8]* @.str2, i32 0, i32 0), i8* getelementptr inbounds ([1 x i8], [1 x i8]* @.str2, i32 0, i32 0), i32 0, i32 0, i32 0, i32 0, i8* getelementptr inbounds ([1 x i8], [1 x i8]* @.str2, i32 0, i32 0), i8* getelementptr inbounds ([1 x i8], [1 x i8]* @.str2, i32 0, i32 0)) #0
  %16 = load [1024 x float]*, [1024 x float]** %4, align 8
  call void (...) @_ssdm_op_SpecInterface([1024 x float]* %16, i8* getelementptr inbounds ([8 x i8], [8 x i8]* @.str, i32 0, i32 0), i32 0, i32 0, i8* getelementptr inbounds ([1 x i8], [1 x i8]* @.str2, i32 0, i32 0), i32 0, i32 0, i8* getelementptr inbounds ([1 x i8], [1 x i8]* @.str2, i32 0, i32 0), i8* getelementptr inbounds ([1 x i8], [1 x i8]* @.str2, i32 0, i32 0), i8* getelementptr inbounds ([1 x i8], [1 x i8]* @.str2, i32 0, i32 0), i32 0, i32 0, i32 0, i32 0, i8* getelementptr inbounds ([1 x i8], [1 x i8]* @.str2, i32 0, i32 0), i8* getelementptr inbounds ([1 x i8], [1 x i8]* @.str2, i32 0, i32 0)) #0
  %17 = load [1024 x float]*, [1024 x float]** %5, align 8
  call void (...) @_ssdm_op_SpecInterface([1024 x float]* %17, i8* getelementptr inbounds ([8 x i8], [8 x i8]* @.str, i32 0, i32 0), i32 0, i32 0, i8* getelementptr inbounds ([1 x i8], [1 x i8]* @.str2, i32 0, i32 0), i32 0, i32 0, i8* getelementptr inbounds ([1 x i8], [1 x i8]* @.str2, i32 0, i32 0), i8* getelementptr inbounds ([1 x i8], [1 x i8]* @.str2, i32 0, i32 0), i8* getelementptr inbounds ([1 x i8], [1 x i8]* @.str2, i32 0, i32 0), i32 0, i32 0, i32 0, i32 0, i8* getelementptr inbounds ([1 x i8], [1 x i8]* @.str2, i32 0, i32 0), i8* getelementptr inbounds ([1 x i8], [1 x i8]* @.str2, i32 0, i32 0)) #0
  %18 = load [1024 x float]*, [1024 x float]** %6, align 8
  call void (...) @_ssdm_op_SpecInterface([1024 x float]* %18, i8* getelementptr inbounds ([8 x i8], [8 x i8]* @.str, i32 0, i32 0), i32 0, i32 0, i8* getelementptr inbounds ([1 x i8], [1 x i8]* @.str2, i32 0, i32 0), i32 0, i32 0, i8* getelementptr inbounds ([1 x i8], [1 x i8]* @.str2, i32 0, i32 0), i8* getelementptr inbounds ([1 x i8], [1 x i8]* @.str2, i32 0, i32 0), i8* getelementptr inbounds ([1 x i8], [1 x i8]* @.str2, i32 0, i32 0), i32 0, i32 0, i32 0, i32 0, i8* getelementptr inbounds ([1 x i8], [1 x i8]* @.str2, i32 0, i32 0), i8* getelementptr inbounds ([1 x i8], [1 x i8]* @.str2, i32 0, i32 0)) #0
  call void (...) @_ssdm_SpecConstant(i32* getelementptr inbounds ([5 x i32], [5 x i32]* @_ZZ15gradient_z_calcPA1024_fS0_S0_S0_S0_S0_E12GRAD_WEIGHTS, i32 0, i32 0)) #0
  br label %19

; <label>:19:                                     ; preds = %0
  store i32 0, i32* %r, align 4
  br label %20

; <label>:20:                                     ; preds = %100, %19
  %21 = load i32, i32* %r, align 4
  %22 = icmp slt i32 %21, 436
  br i1 %22, label %23, label %103

; <label>:23:                                     ; preds = %20
  call void (...) @_ssdm_op_SpecLoopName(i8* getelementptr inbounds ([13 x i8], [13 x i8]* @.str7, i32 0, i32 0)) #0
  call void (...) @_ssdm_RegionBegin(i8* getelementptr inbounds ([13 x i8], [13 x i8]* @.str7, i32 0, i32 0)) #0
  br label %24

; <label>:24:                                     ; preds = %23
  store i32 0, i32* %c, align 4
  br label %25

; <label>:25:                                     ; preds = %96, %24
  %26 = load i32, i32* %c, align 4
  %27 = icmp slt i32 %26, 1024
  br i1 %27, label %28, label %99

; <label>:28:                                     ; preds = %25
  call void (...) @_ssdm_op_SpecLoopName(i8* getelementptr inbounds ([13 x i8], [13 x i8]* @.str8, i32 0, i32 0)) #0
  call void (...) @_ssdm_RegionBegin(i8* getelementptr inbounds ([13 x i8], [13 x i8]* @.str8, i32 0, i32 0)) #0
  call void (...) @_ssdm_op_SpecPipeline(i32 1, i32 1, i32 1, i32 0, i8* getelementptr inbounds ([1 x i8], [1 x i8]* @.str2, i32 0, i32 0)) #0
  %29 = load i32, i32* %c, align 4
  %30 = sext i32 %29 to i64
  %31 = load i32, i32* %r, align 4
  %32 = sext i32 %31 to i64
  %33 = load [1024 x float]*, [1024 x float]** %1, align 8
  %34 = getelementptr inbounds [1024 x float], [1024 x float]* %33, i64 %32
  %35 = getelementptr inbounds [1024 x float], [1024 x float]* %34, i32 0, i64 %30
  %36 = load float, float* %35, align 4
  %37 = load i32, i32* getelementptr inbounds ([5 x i32], [5 x i32]* @_ZZ15gradient_z_calcPA1024_fS0_S0_S0_S0_S0_E12GRAD_WEIGHTS, i32 0, i64 0), align 4
  %38 = sitofp i32 %37 to float
  %39 = fmul float %36, %38
  %40 = load i32, i32* %c, align 4
  %41 = sext i32 %40 to i64
  %42 = load i32, i32* %r, align 4
  %43 = sext i32 %42 to i64
  %44 = load [1024 x float]*, [1024 x float]** %2, align 8
  %45 = getelementptr inbounds [1024 x float], [1024 x float]* %44, i64 %43
  %46 = getelementptr inbounds [1024 x float], [1024 x float]* %45, i32 0, i64 %41
  %47 = load float, float* %46, align 4
  %48 = load i32, i32* getelementptr inbounds ([5 x i32], [5 x i32]* @_ZZ15gradient_z_calcPA1024_fS0_S0_S0_S0_S0_E12GRAD_WEIGHTS, i32 0, i64 1), align 4
  %49 = sitofp i32 %48 to float
  %50 = fmul float %47, %49
  %51 = fadd float %39, %50
  %52 = load i32, i32* %c, align 4
  %53 = sext i32 %52 to i64
  %54 = load i32, i32* %r, align 4
  %55 = sext i32 %54 to i64
  %56 = load [1024 x float]*, [1024 x float]** %3, align 8
  %57 = getelementptr inbounds [1024 x float], [1024 x float]* %56, i64 %55
  %58 = getelementptr inbounds [1024 x float], [1024 x float]* %57, i32 0, i64 %53
  %59 = load float, float* %58, align 4
  %60 = load i32, i32* getelementptr inbounds ([5 x i32], [5 x i32]* @_ZZ15gradient_z_calcPA1024_fS0_S0_S0_S0_S0_E12GRAD_WEIGHTS, i32 0, i64 2), align 4
  %61 = sitofp i32 %60 to float
  %62 = fmul float %59, %61
  %63 = fadd float %51, %62
  %64 = load i32, i32* %c, align 4
  %65 = sext i32 %64 to i64
  %66 = load i32, i32* %r, align 4
  %67 = sext i32 %66 to i64
  %68 = load [1024 x float]*, [1024 x float]** %4, align 8
  %69 = getelementptr inbounds [1024 x float], [1024 x float]* %68, i64 %67
  %70 = getelementptr inbounds [1024 x float], [1024 x float]* %69, i32 0, i64 %65
  %71 = load float, float* %70, align 4
  %72 = load i32, i32* getelementptr inbounds ([5 x i32], [5 x i32]* @_ZZ15gradient_z_calcPA1024_fS0_S0_S0_S0_S0_E12GRAD_WEIGHTS, i32 0, i64 3), align 4
  %73 = sitofp i32 %72 to float
  %74 = fmul float %71, %73
  %75 = fadd float %63, %74
  %76 = load i32, i32* %c, align 4
  %77 = sext i32 %76 to i64
  %78 = load i32, i32* %r, align 4
  %79 = sext i32 %78 to i64
  %80 = load [1024 x float]*, [1024 x float]** %5, align 8
  %81 = getelementptr inbounds [1024 x float], [1024 x float]* %80, i64 %79
  %82 = getelementptr inbounds [1024 x float], [1024 x float]* %81, i32 0, i64 %77
  %83 = load float, float* %82, align 4
  %84 = load i32, i32* getelementptr inbounds ([5 x i32], [5 x i32]* @_ZZ15gradient_z_calcPA1024_fS0_S0_S0_S0_S0_E12GRAD_WEIGHTS, i32 0, i64 4), align 4
  %85 = sitofp i32 %84 to float
  %86 = fmul float %83, %85
  %87 = fadd float %75, %86
  %88 = fdiv float %87, 1.200000e+01
  %89 = load i32, i32* %c, align 4
  %90 = sext i32 %89 to i64
  %91 = load i32, i32* %r, align 4
  %92 = sext i32 %91 to i64
  %93 = load [1024 x float]*, [1024 x float]** %6, align 8
  %94 = getelementptr inbounds [1024 x float], [1024 x float]* %93, i64 %92
  %95 = getelementptr inbounds [1024 x float], [1024 x float]* %94, i32 0, i64 %90
  store float %88, float* %95, align 4
  call void (...) @_ssdm_RegionEnd(i8* getelementptr inbounds ([13 x i8], [13 x i8]* @.str8, i32 0, i32 0)) #0
  br label %96

; <label>:96:                                     ; preds = %28
  %97 = load i32, i32* %c, align 4
  %98 = add nsw i32 %97, 1
  store i32 %98, i32* %c, align 4
  br label %25

; <label>:99:                                     ; preds = %25
  call void (...) @_ssdm_RegionEnd(i8* getelementptr inbounds ([13 x i8], [13 x i8]* @.str7, i32 0, i32 0)) #0
  br label %100

; <label>:100:                                    ; preds = %99
  %101 = load i32, i32* %r, align 4
  %102 = add nsw i32 %101, 1
  store i32 %102, i32* %r, align 4
  br label %20

; <label>:103:                                    ; preds = %20
  ret void
}

; Function Attrs: nounwind uwtable
define void @_Z17gradient_weight_yPA1024_fS0_S0_PA1024_10gradient_t([1024 x float]* %gradient_x, [1024 x float]* %gradient_y, [1024 x float]* %gradient_z, [1024 x %struct.gradient_t]* %filt_grad) #2 {
  %1 = alloca [1024 x float]*, align 8
  %2 = alloca [1024 x float]*, align 8
  %3 = alloca [1024 x float]*, align 8
  %4 = alloca [1024 x %struct.gradient_t]*, align 8
  %buf = alloca %"class.hls::LineBuffer", align 4
  %r = alloca i32, align 4
  %c = alloca i32, align 4
  %tmp = alloca %struct.gradient_t, align 4
  %5 = alloca %struct.gradient_t, align 4
  %tmp1 = alloca %struct.gradient_t, align 4
  %6 = alloca %struct.gradient_t, align 4
  %acc = alloca %struct.gradient_t, align 4
  %i = alloca i32, align 4
  store [1024 x float]* %gradient_x, [1024 x float]** %1, align 8
  store [1024 x float]* %gradient_y, [1024 x float]** %2, align 8
  store [1024 x float]* %gradient_z, [1024 x float]** %3, align 8
  store [1024 x %struct.gradient_t]* %filt_grad, [1024 x %struct.gradient_t]** %4, align 8
  %7 = load [1024 x float]*, [1024 x float]** %1, align 8
  call void (...) @_ssdm_SpecArrayDimSize([1024 x float]* %7, i32 436) #0
  %8 = load [1024 x float]*, [1024 x float]** %2, align 8
  call void (...) @_ssdm_SpecArrayDimSize([1024 x float]* %8, i32 436) #0
  %9 = load [1024 x float]*, [1024 x float]** %3, align 8
  call void (...) @_ssdm_SpecArrayDimSize([1024 x float]* %9, i32 436) #0
  %10 = load [1024 x %struct.gradient_t]*, [1024 x %struct.gradient_t]** %4, align 8
  call void (...) @_ssdm_SpecArrayDimSize([1024 x %struct.gradient_t]* %10, i32 436) #0
  %11 = load [1024 x float]*, [1024 x float]** %1, align 8
  call void (...) @_ssdm_op_SpecInterface([1024 x float]* %11, i8* getelementptr inbounds ([8 x i8], [8 x i8]* @.str, i32 0, i32 0), i32 0, i32 0, i8* getelementptr inbounds ([1 x i8], [1 x i8]* @.str2, i32 0, i32 0), i32 0, i32 0, i8* getelementptr inbounds ([1 x i8], [1 x i8]* @.str2, i32 0, i32 0), i8* getelementptr inbounds ([1 x i8], [1 x i8]* @.str2, i32 0, i32 0), i8* getelementptr inbounds ([1 x i8], [1 x i8]* @.str2, i32 0, i32 0), i32 0, i32 0, i32 0, i32 0, i8* getelementptr inbounds ([1 x i8], [1 x i8]* @.str2, i32 0, i32 0), i8* getelementptr inbounds ([1 x i8], [1 x i8]* @.str2, i32 0, i32 0)) #0
  %12 = load [1024 x float]*, [1024 x float]** %2, align 8
  call void (...) @_ssdm_op_SpecInterface([1024 x float]* %12, i8* getelementptr inbounds ([8 x i8], [8 x i8]* @.str, i32 0, i32 0), i32 0, i32 0, i8* getelementptr inbounds ([1 x i8], [1 x i8]* @.str2, i32 0, i32 0), i32 0, i32 0, i8* getelementptr inbounds ([1 x i8], [1 x i8]* @.str2, i32 0, i32 0), i8* getelementptr inbounds ([1 x i8], [1 x i8]* @.str2, i32 0, i32 0), i8* getelementptr inbounds ([1 x i8], [1 x i8]* @.str2, i32 0, i32 0), i32 0, i32 0, i32 0, i32 0, i8* getelementptr inbounds ([1 x i8], [1 x i8]* @.str2, i32 0, i32 0), i8* getelementptr inbounds ([1 x i8], [1 x i8]* @.str2, i32 0, i32 0)) #0
  %13 = load [1024 x float]*, [1024 x float]** %3, align 8
  call void (...) @_ssdm_op_SpecInterface([1024 x float]* %13, i8* getelementptr inbounds ([8 x i8], [8 x i8]* @.str, i32 0, i32 0), i32 0, i32 0, i8* getelementptr inbounds ([1 x i8], [1 x i8]* @.str2, i32 0, i32 0), i32 0, i32 0, i8* getelementptr inbounds ([1 x i8], [1 x i8]* @.str2, i32 0, i32 0), i8* getelementptr inbounds ([1 x i8], [1 x i8]* @.str2, i32 0, i32 0), i8* getelementptr inbounds ([1 x i8], [1 x i8]* @.str2, i32 0, i32 0), i32 0, i32 0, i32 0, i32 0, i8* getelementptr inbounds ([1 x i8], [1 x i8]* @.str2, i32 0, i32 0), i8* getelementptr inbounds ([1 x i8], [1 x i8]* @.str2, i32 0, i32 0)) #0
  %14 = load [1024 x %struct.gradient_t]*, [1024 x %struct.gradient_t]** %4, align 8
  call void (...) @_ssdm_op_SpecInterface([1024 x %struct.gradient_t]* %14, i8* getelementptr inbounds ([8 x i8], [8 x i8]* @.str, i32 0, i32 0), i32 0, i32 0, i8* getelementptr inbounds ([1 x i8], [1 x i8]* @.str2, i32 0, i32 0), i32 0, i32 0, i8* getelementptr inbounds ([1 x i8], [1 x i8]* @.str2, i32 0, i32 0), i8* getelementptr inbounds ([1 x i8], [1 x i8]* @.str2, i32 0, i32 0), i8* getelementptr inbounds ([1 x i8], [1 x i8]* @.str2, i32 0, i32 0), i32 0, i32 0, i32 0, i32 0, i8* getelementptr inbounds ([1 x i8], [1 x i8]* @.str2, i32 0, i32 0), i8* getelementptr inbounds ([1 x i8], [1 x i8]* @.str2, i32 0, i32 0)) #0
  call void @_ZN3hls10LineBufferILi7ELi1024E10gradient_tLi0EEC1Ev(%"class.hls::LineBuffer"* %buf)
  call void (...) @_ssdm_SpecConstant(float* getelementptr inbounds ([7 x float], [7 x float]* @_ZZ17gradient_weight_yPA1024_fS0_S0_PA1024_10gradient_tE11GRAD_FILTER, i32 0, i32 0)) #0
  br label %15

; <label>:15:                                     ; preds = %0
  store i32 0, i32* %r, align 4
  br label %16

; <label>:16:                                     ; preds = %191, %15
  %17 = load i32, i32* %r, align 4
  %18 = icmp slt i32 %17, 439
  br i1 %18, label %19, label %194

; <label>:19:                                     ; preds = %16
  call void (...) @_ssdm_op_SpecLoopName(i8* getelementptr inbounds ([20 x i8], [20 x i8]* @.str9, i32 0, i32 0)) #0
  call void (...) @_ssdm_RegionBegin(i8* getelementptr inbounds ([20 x i8], [20 x i8]* @.str9, i32 0, i32 0)) #0
  br label %20

; <label>:20:                                     ; preds = %19
  store i32 0, i32* %c, align 4
  br label %21

; <label>:21:                                     ; preds = %187, %20
  %22 = load i32, i32* %c, align 4
  %23 = icmp slt i32 %22, 1024
  br i1 %23, label %24, label %190

; <label>:24:                                     ; preds = %21
  call void (...) @_ssdm_op_SpecLoopName(i8* getelementptr inbounds ([20 x i8], [20 x i8]* @.str10, i32 0, i32 0)) #0
  call void (...) @_ssdm_RegionBegin(i8* getelementptr inbounds ([20 x i8], [20 x i8]* @.str10, i32 0, i32 0)) #0
  call void (...) @_ssdm_op_SpecPipeline(i32 1, i32 1, i32 1, i32 0, i8* getelementptr inbounds ([1 x i8], [1 x i8]* @.str2, i32 0, i32 0)) #0
  %25 = load i32, i32* %r, align 4
  %26 = icmp slt i32 %25, 436
  br i1 %26, label %27, label %68

; <label>:27:                                     ; preds = %24
  %28 = load i32, i32* %c, align 4
  call void @_ZN3hls10LineBufferILi7ELi1024E10gradient_tLi0EE15shift_pixels_upEi(%"class.hls::LineBuffer"* %buf, i32 %28)
  %29 = load i32, i32* %c, align 4
  %30 = sext i32 %29 to i64
  %31 = load i32, i32* %r, align 4
  %32 = sext i32 %31 to i64
  %33 = load [1024 x float]*, [1024 x float]** %1, align 8
  %34 = getelementptr inbounds [1024 x float], [1024 x float]* %33, i64 %32
  %35 = getelementptr inbounds [1024 x float], [1024 x float]* %34, i32 0, i64 %30
  %36 = load float, float* %35, align 4
  %37 = getelementptr inbounds %struct.gradient_t, %struct.gradient_t* %tmp, i32 0, i32 0
  store float %36, float* %37, align 4
  %38 = load i32, i32* %c, align 4
  %39 = sext i32 %38 to i64
  %40 = load i32, i32* %r, align 4
  %41 = sext i32 %40 to i64
  %42 = load [1024 x float]*, [1024 x float]** %2, align 8
  %43 = getelementptr inbounds [1024 x float], [1024 x float]* %42, i64 %41
  %44 = getelementptr inbounds [1024 x float], [1024 x float]* %43, i32 0, i64 %39
  %45 = load float, float* %44, align 4
  %46 = getelementptr inbounds %struct.gradient_t, %struct.gradient_t* %tmp, i32 0, i32 1
  store float %45, float* %46, align 4
  %47 = load i32, i32* %c, align 4
  %48 = sext i32 %47 to i64
  %49 = load i32, i32* %r, align 4
  %50 = sext i32 %49 to i64
  %51 = load [1024 x float]*, [1024 x float]** %3, align 8
  %52 = getelementptr inbounds [1024 x float], [1024 x float]* %51, i64 %50
  %53 = getelementptr inbounds [1024 x float], [1024 x float]* %52, i32 0, i64 %48
  %54 = load float, float* %53, align 4
  %55 = getelementptr inbounds %struct.gradient_t, %struct.gradient_t* %tmp, i32 0, i32 2
  store float %54, float* %55, align 4
  %56 = bitcast %struct.gradient_t* %5 to i8*
  %57 = bitcast %struct.gradient_t* %tmp to i8*
  %58 = getelementptr inbounds %struct.gradient_t, %struct.gradient_t* %5, i32 0, i32 0
  %59 = getelementptr inbounds %struct.gradient_t, %struct.gradient_t* %tmp, i32 0, i32 0
  %60 = load float, float* %59
  store float %60, float* %58
  %61 = getelementptr inbounds %struct.gradient_t, %struct.gradient_t* %5, i32 0, i32 1
  %62 = getelementptr inbounds %struct.gradient_t, %struct.gradient_t* %tmp, i32 0, i32 1
  %63 = load float, float* %62
  store float %63, float* %61
  %64 = getelementptr inbounds %struct.gradient_t, %struct.gradient_t* %5, i32 0, i32 2
  %65 = getelementptr inbounds %struct.gradient_t, %struct.gradient_t* %tmp, i32 0, i32 2
  %66 = load float, float* %65
  store float %66, float* %64
  %67 = load i32, i32* %c, align 4
  call void @_ZN3hls10LineBufferILi7ELi1024E10gradient_tLi0EE17insert_bottom_rowES1_i(%"class.hls::LineBuffer"* %buf, %struct.gradient_t* byval align 8 %5, i32 %67)
  br label %85

; <label>:68:                                     ; preds = %24
  %69 = load i32, i32* %c, align 4
  call void @_ZN3hls10LineBufferILi7ELi1024E10gradient_tLi0EE15shift_pixels_upEi(%"class.hls::LineBuffer"* %buf, i32 %69)
  %70 = getelementptr inbounds %struct.gradient_t, %struct.gradient_t* %tmp1, i32 0, i32 0
  store float 0.000000e+00, float* %70, align 4
  %71 = getelementptr inbounds %struct.gradient_t, %struct.gradient_t* %tmp1, i32 0, i32 1
  store float 0.000000e+00, float* %71, align 4
  %72 = getelementptr inbounds %struct.gradient_t, %struct.gradient_t* %tmp1, i32 0, i32 2
  store float 0.000000e+00, float* %72, align 4
  %73 = bitcast %struct.gradient_t* %6 to i8*
  %74 = bitcast %struct.gradient_t* %tmp1 to i8*
  %75 = getelementptr inbounds %struct.gradient_t, %struct.gradient_t* %6, i32 0, i32 0
  %76 = getelementptr inbounds %struct.gradient_t, %struct.gradient_t* %tmp1, i32 0, i32 0
  %77 = load float, float* %76
  store float %77, float* %75
  %78 = getelementptr inbounds %struct.gradient_t, %struct.gradient_t* %6, i32 0, i32 1
  %79 = getelementptr inbounds %struct.gradient_t, %struct.gradient_t* %tmp1, i32 0, i32 1
  %80 = load float, float* %79
  store float %80, float* %78
  %81 = getelementptr inbounds %struct.gradient_t, %struct.gradient_t* %6, i32 0, i32 2
  %82 = getelementptr inbounds %struct.gradient_t, %struct.gradient_t* %tmp1, i32 0, i32 2
  %83 = load float, float* %82
  store float %83, float* %81
  %84 = load i32, i32* %c, align 4
  call void @_ZN3hls10LineBufferILi7ELi1024E10gradient_tLi0EE17insert_bottom_rowES1_i(%"class.hls::LineBuffer"* %buf, %struct.gradient_t* byval align 8 %6, i32 %84)
  br label %85

; <label>:85:                                     ; preds = %68, %27
  %86 = getelementptr inbounds %struct.gradient_t, %struct.gradient_t* %acc, i32 0, i32 0
  store float 0.000000e+00, float* %86, align 4
  %87 = getelementptr inbounds %struct.gradient_t, %struct.gradient_t* %acc, i32 0, i32 1
  store float 0.000000e+00, float* %87, align 4
  %88 = getelementptr inbounds %struct.gradient_t, %struct.gradient_t* %acc, i32 0, i32 2
  store float 0.000000e+00, float* %88, align 4
  %89 = load i32, i32* %r, align 4
  %90 = icmp sge i32 %89, 6
  br i1 %90, label %91, label %162

; <label>:91:                                     ; preds = %85
  %92 = load i32, i32* %r, align 4
  %93 = icmp slt i32 %92, 436
  br i1 %93, label %94, label %162

; <label>:94:                                     ; preds = %91
  br label %95

; <label>:95:                                     ; preds = %94
  store i32 0, i32* %i, align 4
  br label %96

; <label>:96:                                     ; preds = %139, %95
  %97 = load i32, i32* %i, align 4
  %98 = icmp slt i32 %97, 7
  br i1 %98, label %99, label %142

; <label>:99:                                     ; preds = %96
  call void (...) @_ssdm_op_SpecLoopName(i8* getelementptr inbounds ([18 x i8], [18 x i8]* @.str11, i32 0, i32 0)) #0
  call void (...) @_ssdm_RegionBegin(i8* getelementptr inbounds ([18 x i8], [18 x i8]* @.str11, i32 0, i32 0)) #0
  %100 = load i32, i32* %i, align 4
  %101 = load i32, i32* %c, align 4
  %102 = call %struct.gradient_t* @_ZN3hls10LineBufferILi7ELi1024E10gradient_tLi0EE6getvalEii(%"class.hls::LineBuffer"* %buf, i32 %100, i32 %101)
  %103 = getelementptr inbounds %struct.gradient_t, %struct.gradient_t* %102, i32 0, i32 0
  %104 = load float, float* %103, align 4
  %105 = load i32, i32* %i, align 4
  %106 = sext i32 %105 to i64
  %107 = getelementptr inbounds [7 x float], [7 x float]* @_ZZ17gradient_weight_yPA1024_fS0_S0_PA1024_10gradient_tE11GRAD_FILTER, i32 0, i64 %106
  %108 = load float, float* %107, align 4
  %109 = fmul float %104, %108
  %110 = getelementptr inbounds %struct.gradient_t, %struct.gradient_t* %acc, i32 0, i32 0
  %111 = load float, float* %110, align 4
  %112 = fadd float %111, %109
  store float %112, float* %110, align 4
  %113 = load i32, i32* %i, align 4
  %114 = load i32, i32* %c, align 4
  %115 = call %struct.gradient_t* @_ZN3hls10LineBufferILi7ELi1024E10gradient_tLi0EE6getvalEii(%"class.hls::LineBuffer"* %buf, i32 %113, i32 %114)
  %116 = getelementptr inbounds %struct.gradient_t, %struct.gradient_t* %115, i32 0, i32 1
  %117 = load float, float* %116, align 4
  %118 = load i32, i32* %i, align 4
  %119 = sext i32 %118 to i64
  %120 = getelementptr inbounds [7 x float], [7 x float]* @_ZZ17gradient_weight_yPA1024_fS0_S0_PA1024_10gradient_tE11GRAD_FILTER, i32 0, i64 %119
  %121 = load float, float* %120, align 4
  %122 = fmul float %117, %121
  %123 = getelementptr inbounds %struct.gradient_t, %struct.gradient_t* %acc, i32 0, i32 1
  %124 = load float, float* %123, align 4
  %125 = fadd float %124, %122
  store float %125, float* %123, align 4
  %126 = load i32, i32* %i, align 4
  %127 = load i32, i32* %c, align 4
  %128 = call %struct.gradient_t* @_ZN3hls10LineBufferILi7ELi1024E10gradient_tLi0EE6getvalEii(%"class.hls::LineBuffer"* %buf, i32 %126, i32 %127)
  %129 = getelementptr inbounds %struct.gradient_t, %struct.gradient_t* %128, i32 0, i32 2
  %130 = load float, float* %129, align 4
  %131 = load i32, i32* %i, align 4
  %132 = sext i32 %131 to i64
  %133 = getelementptr inbounds [7 x float], [7 x float]* @_ZZ17gradient_weight_yPA1024_fS0_S0_PA1024_10gradient_tE11GRAD_FILTER, i32 0, i64 %132
  %134 = load float, float* %133, align 4
  %135 = fmul float %130, %134
  %136 = getelementptr inbounds %struct.gradient_t, %struct.gradient_t* %acc, i32 0, i32 2
  %137 = load float, float* %136, align 4
  %138 = fadd float %137, %135
  store float %138, float* %136, align 4
  call void (...) @_ssdm_RegionEnd(i8* getelementptr inbounds ([18 x i8], [18 x i8]* @.str11, i32 0, i32 0)) #0
  br label %139

; <label>:139:                                    ; preds = %99
  %140 = load i32, i32* %i, align 4
  %141 = add nsw i32 %140, 1
  store i32 %141, i32* %i, align 4
  br label %96

; <label>:142:                                    ; preds = %96
  %143 = load i32, i32* %c, align 4
  %144 = sext i32 %143 to i64
  %145 = load i32, i32* %r, align 4
  %146 = sub nsw i32 %145, 3
  %147 = sext i32 %146 to i64
  %148 = load [1024 x %struct.gradient_t]*, [1024 x %struct.gradient_t]** %4, align 8
  %149 = getelementptr inbounds [1024 x %struct.gradient_t], [1024 x %struct.gradient_t]* %148, i64 %147
  %150 = getelementptr inbounds [1024 x %struct.gradient_t], [1024 x %struct.gradient_t]* %149, i32 0, i64 %144
  %151 = bitcast %struct.gradient_t* %150 to i8*
  %152 = bitcast %struct.gradient_t* %acc to i8*
  %153 = getelementptr inbounds %struct.gradient_t, %struct.gradient_t* %150, i32 0, i32 0
  %154 = getelementptr inbounds %struct.gradient_t, %struct.gradient_t* %acc, i32 0, i32 0
  %155 = load float, float* %154
  store float %155, float* %153
  %156 = getelementptr inbounds %struct.gradient_t, %struct.gradient_t* %150, i32 0, i32 1
  %157 = getelementptr inbounds %struct.gradient_t, %struct.gradient_t* %acc, i32 0, i32 1
  %158 = load float, float* %157
  store float %158, float* %156
  %159 = getelementptr inbounds %struct.gradient_t, %struct.gradient_t* %150, i32 0, i32 2
  %160 = getelementptr inbounds %struct.gradient_t, %struct.gradient_t* %acc, i32 0, i32 2
  %161 = load float, float* %160
  store float %161, float* %159
  br label %186

; <label>:162:                                    ; preds = %91, %85
  %163 = load i32, i32* %r, align 4
  %164 = icmp sge i32 %163, 2
  br i1 %164, label %165, label %185

; <label>:165:                                    ; preds = %162
  %166 = load i32, i32* %c, align 4
  %167 = sext i32 %166 to i64
  %168 = load i32, i32* %r, align 4
  %169 = sub nsw i32 %168, 3
  %170 = sext i32 %169 to i64
  %171 = load [1024 x %struct.gradient_t]*, [1024 x %struct.gradient_t]** %4, align 8
  %172 = getelementptr inbounds [1024 x %struct.gradient_t], [1024 x %struct.gradient_t]* %171, i64 %170
  %173 = getelementptr inbounds [1024 x %struct.gradient_t], [1024 x %struct.gradient_t]* %172, i32 0, i64 %167
  %174 = bitcast %struct.gradient_t* %173 to i8*
  %175 = bitcast %struct.gradient_t* %acc to i8*
  %176 = getelementptr inbounds %struct.gradient_t, %struct.gradient_t* %173, i32 0, i32 0
  %177 = getelementptr inbounds %struct.gradient_t, %struct.gradient_t* %acc, i32 0, i32 0
  %178 = load float, float* %177
  store float %178, float* %176
  %179 = getelementptr inbounds %struct.gradient_t, %struct.gradient_t* %173, i32 0, i32 1
  %180 = getelementptr inbounds %struct.gradient_t, %struct.gradient_t* %acc, i32 0, i32 1
  %181 = load float, float* %180
  store float %181, float* %179
  %182 = getelementptr inbounds %struct.gradient_t, %struct.gradient_t* %173, i32 0, i32 2
  %183 = getelementptr inbounds %struct.gradient_t, %struct.gradient_t* %acc, i32 0, i32 2
  %184 = load float, float* %183
  store float %184, float* %182
  br label %185

; <label>:185:                                    ; preds = %165, %162
  br label %186

; <label>:186:                                    ; preds = %185, %142
  call void (...) @_ssdm_RegionEnd(i8* getelementptr inbounds ([20 x i8], [20 x i8]* @.str10, i32 0, i32 0)) #0
  br label %187

; <label>:187:                                    ; preds = %186
  %188 = load i32, i32* %c, align 4
  %189 = add nsw i32 %188, 1
  store i32 %189, i32* %c, align 4
  br label %21

; <label>:190:                                    ; preds = %21
  call void (...) @_ssdm_RegionEnd(i8* getelementptr inbounds ([20 x i8], [20 x i8]* @.str9, i32 0, i32 0)) #0
  br label %191

; <label>:191:                                    ; preds = %190
  %192 = load i32, i32* %r, align 4
  %193 = add nsw i32 %192, 1
  store i32 %193, i32* %r, align 4
  br label %16

; <label>:194:                                    ; preds = %16
  ret void
}

; Function Attrs: nounwind uwtable
define linkonce_odr void @_ZN3hls10LineBufferILi7ELi1024E10gradient_tLi0EEC1Ev(%"class.hls::LineBuffer"* %this) unnamed_addr #2 comdat align 2 {
  %1 = alloca %"class.hls::LineBuffer"*, align 8
  store %"class.hls::LineBuffer"* %this, %"class.hls::LineBuffer"** %1, align 8
  %2 = load %"class.hls::LineBuffer"*, %"class.hls::LineBuffer"** %1
  call void @_ZN3hls10LineBufferILi7ELi1024E10gradient_tLi0EEC2Ev(%"class.hls::LineBuffer"* %2)
  ret void
}

; Function Attrs: nounwind uwtable
define linkonce_odr void @_ZN3hls10LineBufferILi7ELi1024E10gradient_tLi0EE15shift_pixels_upEi(%"class.hls::LineBuffer"* %this, i32 %col) #2 comdat align 2 {
  %1 = alloca %"class.hls::LineBuffer"*, align 8
  %2 = alloca i32, align 4
  %i = alloca %struct.ap_uint.15, align 4
  %3 = alloca %struct.ap_uint.15, align 4
  %4 = alloca %struct.ap_int_base.18, align 8
  %5 = alloca %struct.ap_int_base.16, align 4
  store %"class.hls::LineBuffer"* %this, %"class.hls::LineBuffer"** %1, align 8
  store i32 %col, i32* %2, align 4
  %6 = load %"class.hls::LineBuffer"*, %"class.hls::LineBuffer"** %1
  call void (...) @_ssdm_InlineSelf(i32 0, i8* getelementptr inbounds ([1 x i8], [1 x i8]* @.str2, i32 0, i32 0)) #0
  %7 = load i32, i32* %2, align 4
  %8 = icmp sge i32 %7, 0
  br i1 %8, label %9, label %12

; <label>:9:                                      ; preds = %0
  %10 = load i32, i32* %2, align 4
  %11 = icmp slt i32 %10, 1024
  br label %12

; <label>:12:                                     ; preds = %9, %0
  %13 = phi i1 [ false, %0 ], [ %11, %9 ]
  br i1 %13, label %14, label %15

; <label>:14:                                     ; preds = %12
  br label %17

; <label>:15:                                     ; preds = %12
  call void @__assert_fail(i8* getelementptr inbounds ([23 x i8], [23 x i8]* @.str37, i32 0, i32 0), i8* getelementptr inbounds ([81 x i8], [81 x i8]* @.str34, i32 0, i32 0), i32 718, i8* getelementptr inbounds ([120 x i8], [120 x i8]* @__PRETTY_FUNCTION__._ZN3hls10LineBufferILi7ELi1024E10gradient_tLi0EE15shift_pixels_upEi, i32 0, i32 0)) #5
  unreachable
                                                  ; No predecessors!
  br label %17

; <label>:17:                                     ; preds = %16, %14
  call void @_ZN7ap_uintILi32EEC1Ev(%struct.ap_uint.15* %i)
  call void @_ZN7ap_uintILi32EEC1Ei(%struct.ap_uint.15* %3, i32 0)
  %18 = call %struct.ap_uint.15* @_ZN7ap_uintILi32EEaSERKS0_(%struct.ap_uint.15* %i, %struct.ap_uint.15* %3)
  br label %19

; <label>:19:                                     ; preds = %49, %17
  %20 = bitcast %struct.ap_uint.15* %i to %struct.ap_int_base.16*
  %21 = call zeroext i1 @_ZltILi32ELb0EEbRK11ap_int_baseIXT_EXT0_EXleT_Li64EEEi(%struct.ap_int_base.16* %20, i32 6)
  br i1 %21, label %22, label %51

; <label>:22:                                     ; preds = %19
  call void (...) @_ssdm_RegionBegin(i8* getelementptr inbounds ([12 x i8], [12 x i8]* @.str41, i32 0, i32 0)) #0
  call void (...) @_ssdm_Unroll(i32 0, i32 0, i32 0, i8* getelementptr inbounds ([1 x i8], [1 x i8]* @.str2, i32 0, i32 0)) #0
  %23 = load i32, i32* %2, align 4
  %24 = sext i32 %23 to i64
  %25 = bitcast %struct.ap_uint.15* %i to %struct.ap_int_base.16*
  %26 = call i32 @_ZNK11ap_int_baseILi32ELb0ELb1EEcvjEv(%struct.ap_int_base.16* %25)
  %27 = zext i32 %26 to i64
  %28 = getelementptr inbounds %"class.hls::LineBuffer", %"class.hls::LineBuffer"* %6, i32 0, i32 0
  %29 = getelementptr inbounds [7 x [1024 x %struct.gradient_t]], [7 x [1024 x %struct.gradient_t]]* %28, i32 0, i64 %27
  %30 = getelementptr inbounds [1024 x %struct.gradient_t], [1024 x %struct.gradient_t]* %29, i32 0, i64 %24
  %31 = load i32, i32* %2, align 4
  %32 = sext i32 %31 to i64
  %33 = bitcast %struct.ap_uint.15* %i to %struct.ap_int_base.16*
  call void @_ZplILi32ELb0EEN11ap_int_baseIXT_EXT0_EXleT_Li64EEE5RTypeIXLi32EEXLb1EEE4plusERKS1_i(%struct.ap_int_base.18* sret %4, %struct.ap_int_base.16* %33, i32 1)
  %34 = call i64 @_ZNK11ap_int_baseILi34ELb1ELb1EEcvxEv(%struct.ap_int_base.18* %4)
  %35 = getelementptr inbounds %"class.hls::LineBuffer", %"class.hls::LineBuffer"* %6, i32 0, i32 0
  %36 = getelementptr inbounds [7 x [1024 x %struct.gradient_t]], [7 x [1024 x %struct.gradient_t]]* %35, i32 0, i64 %34
  %37 = getelementptr inbounds [1024 x %struct.gradient_t], [1024 x %struct.gradient_t]* %36, i32 0, i64 %32
  %38 = bitcast %struct.gradient_t* %30 to i8*
  %39 = bitcast %struct.gradient_t* %37 to i8*
  %40 = getelementptr inbounds %struct.gradient_t, %struct.gradient_t* %30, i32 0, i32 0
  %41 = getelementptr inbounds %struct.gradient_t, %struct.gradient_t* %37, i32 0, i32 0
  %42 = load float, float* %41
  store float %42, float* %40
  %43 = getelementptr inbounds %struct.gradient_t, %struct.gradient_t* %30, i32 0, i32 1
  %44 = getelementptr inbounds %struct.gradient_t, %struct.gradient_t* %37, i32 0, i32 1
  %45 = load float, float* %44
  store float %45, float* %43
  %46 = getelementptr inbounds %struct.gradient_t, %struct.gradient_t* %30, i32 0, i32 2
  %47 = getelementptr inbounds %struct.gradient_t, %struct.gradient_t* %37, i32 0, i32 2
  %48 = load float, float* %47
  store float %48, float* %46
  call void (...) @_ssdm_RegionEnd(i8* getelementptr inbounds ([12 x i8], [12 x i8]* @.str41, i32 0, i32 0)) #0
  br label %49

; <label>:49:                                     ; preds = %22
  %50 = bitcast %struct.ap_uint.15* %i to %struct.ap_int_base.16*
  call void @_ZN11ap_int_baseILi32ELb0ELb1EEppEi(%struct.ap_int_base.16* sret %5, %struct.ap_int_base.16* %50, i32 0)
  br label %19

; <label>:51:                                     ; preds = %19
  ret void
}

; Function Attrs: nounwind uwtable
define linkonce_odr void @_ZN3hls10LineBufferILi7ELi1024E10gradient_tLi0EE17insert_bottom_rowES1_i(%"class.hls::LineBuffer"* %this, %struct.gradient_t* byval align 8 %value, i32 %col) #2 comdat align 2 {
  %1 = alloca %"class.hls::LineBuffer"*, align 8
  %2 = alloca i32, align 4
  store %"class.hls::LineBuffer"* %this, %"class.hls::LineBuffer"** %1, align 8
  store i32 %col, i32* %2, align 4
  %3 = load %"class.hls::LineBuffer"*, %"class.hls::LineBuffer"** %1
  call void (...) @_ssdm_InlineSelf(i32 0, i8* getelementptr inbounds ([1 x i8], [1 x i8]* @.str2, i32 0, i32 0)) #0
  %4 = load i32, i32* %2, align 4
  %5 = icmp sge i32 %4, 0
  br i1 %5, label %6, label %9

; <label>:6:                                      ; preds = %0
  %7 = load i32, i32* %2, align 4
  %8 = icmp slt i32 %7, 1024
  br label %9

; <label>:9:                                      ; preds = %6, %0
  %10 = phi i1 [ false, %0 ], [ %8, %6 ]
  br i1 %10, label %11, label %12

; <label>:11:                                     ; preds = %9
  br label %14

; <label>:12:                                     ; preds = %9
  call void @__assert_fail(i8* getelementptr inbounds ([23 x i8], [23 x i8]* @.str37, i32 0, i32 0), i8* getelementptr inbounds ([81 x i8], [81 x i8]* @.str34, i32 0, i32 0), i32 757, i8* getelementptr inbounds ([125 x i8], [125 x i8]* @__PRETTY_FUNCTION__._ZN3hls10LineBufferILi7ELi1024E10gradient_tLi0EE17insert_bottom_rowES1_i, i32 0, i32 0)) #5
  unreachable
                                                  ; No predecessors!
  br label %14

; <label>:14:                                     ; preds = %13, %11
  %15 = load i32, i32* %2, align 4
  %16 = sext i32 %15 to i64
  %17 = getelementptr inbounds %"class.hls::LineBuffer", %"class.hls::LineBuffer"* %3, i32 0, i32 0
  %18 = getelementptr inbounds [7 x [1024 x %struct.gradient_t]], [7 x [1024 x %struct.gradient_t]]* %17, i32 0, i64 6
  %19 = getelementptr inbounds [1024 x %struct.gradient_t], [1024 x %struct.gradient_t]* %18, i32 0, i64 %16
  %20 = bitcast %struct.gradient_t* %19 to i8*
  %21 = bitcast %struct.gradient_t* %value to i8*
  %22 = getelementptr inbounds %struct.gradient_t, %struct.gradient_t* %19, i32 0, i32 0
  %23 = getelementptr inbounds %struct.gradient_t, %struct.gradient_t* %value, i32 0, i32 0
  %24 = load float, float* %23
  store float %24, float* %22
  %25 = getelementptr inbounds %struct.gradient_t, %struct.gradient_t* %19, i32 0, i32 1
  %26 = getelementptr inbounds %struct.gradient_t, %struct.gradient_t* %value, i32 0, i32 1
  %27 = load float, float* %26
  store float %27, float* %25
  %28 = getelementptr inbounds %struct.gradient_t, %struct.gradient_t* %19, i32 0, i32 2
  %29 = getelementptr inbounds %struct.gradient_t, %struct.gradient_t* %value, i32 0, i32 2
  %30 = load float, float* %29
  store float %30, float* %28
  ret void
}

; Function Attrs: nounwind uwtable
define linkonce_odr %struct.gradient_t* @_ZN3hls10LineBufferILi7ELi1024E10gradient_tLi0EE6getvalEii(%"class.hls::LineBuffer"* %this, i32 %row, i32 %col) #2 comdat align 2 {
  %1 = alloca %"class.hls::LineBuffer"*, align 8
  %2 = alloca i32, align 4
  %3 = alloca i32, align 4
  store %"class.hls::LineBuffer"* %this, %"class.hls::LineBuffer"** %1, align 8
  store i32 %row, i32* %2, align 4
  store i32 %col, i32* %3, align 4
  %4 = load %"class.hls::LineBuffer"*, %"class.hls::LineBuffer"** %1
  call void (...) @_ssdm_InlineSelf(i32 0, i8* getelementptr inbounds ([1 x i8], [1 x i8]* @.str2, i32 0, i32 0)) #0
  %5 = load i32, i32* %2, align 4
  %6 = icmp sge i32 %5, 0
  br i1 %6, label %7, label %16

; <label>:7:                                      ; preds = %0
  %8 = load i32, i32* %2, align 4
  %9 = icmp slt i32 %8, 7
  br i1 %9, label %10, label %16

; <label>:10:                                     ; preds = %7
  %11 = load i32, i32* %3, align 4
  %12 = icmp sge i32 %11, 0
  br i1 %12, label %13, label %16

; <label>:13:                                     ; preds = %10
  %14 = load i32, i32* %3, align 4
  %15 = icmp slt i32 %14, 1024
  br label %16

; <label>:16:                                     ; preds = %13, %10, %7, %0
  %17 = phi i1 [ false, %10 ], [ false, %7 ], [ false, %0 ], [ %15, %13 ]
  br i1 %17, label %18, label %19

; <label>:18:                                     ; preds = %16
  br label %21

; <label>:19:                                     ; preds = %16
  call void @__assert_fail(i8* getelementptr inbounds ([49 x i8], [49 x i8]* @.str33, i32 0, i32 0), i8* getelementptr inbounds ([81 x i8], [81 x i8]* @.str34, i32 0, i32 0), i32 834, i8* getelementptr inbounds ([114 x i8], [114 x i8]* @__PRETTY_FUNCTION__._ZN3hls10LineBufferILi7ELi1024E10gradient_tLi0EE6getvalEii, i32 0, i32 0)) #5
  unreachable
                                                  ; No predecessors!
  br label %21

; <label>:21:                                     ; preds = %20, %18
  %22 = load i32, i32* %3, align 4
  %23 = sext i32 %22 to i64
  %24 = load i32, i32* %2, align 4
  %25 = sext i32 %24 to i64
  %26 = getelementptr inbounds %"class.hls::LineBuffer", %"class.hls::LineBuffer"* %4, i32 0, i32 0
  %27 = getelementptr inbounds [7 x [1024 x %struct.gradient_t]], [7 x [1024 x %struct.gradient_t]]* %26, i32 0, i64 %25
  %28 = getelementptr inbounds [1024 x %struct.gradient_t], [1024 x %struct.gradient_t]* %27, i32 0, i64 %23
  ret %struct.gradient_t* %28
}

; Function Attrs: nounwind uwtable
define void @_Z17gradient_weight_xPA1024_10gradient_tS1_([1024 x %struct.gradient_t]* %y_filt, [1024 x %struct.gradient_t]* %filt_grad) #2 {
  %1 = alloca [1024 x %struct.gradient_t]*, align 8
  %2 = alloca [1024 x %struct.gradient_t]*, align 8
  %buf = alloca %"class.hls::Window.0", align 4
  %r = alloca i32, align 4
  %c = alloca i32, align 4
  %tmp = alloca %struct.gradient_t, align 4
  %3 = alloca %struct.gradient_t, align 4
  %acc = alloca %struct.gradient_t, align 4
  %i = alloca i32, align 4
  store [1024 x %struct.gradient_t]* %y_filt, [1024 x %struct.gradient_t]** %1, align 8
  store [1024 x %struct.gradient_t]* %filt_grad, [1024 x %struct.gradient_t]** %2, align 8
  %4 = load [1024 x %struct.gradient_t]*, [1024 x %struct.gradient_t]** %1, align 8
  call void (...) @_ssdm_SpecArrayDimSize([1024 x %struct.gradient_t]* %4, i32 436) #0
  %5 = load [1024 x %struct.gradient_t]*, [1024 x %struct.gradient_t]** %2, align 8
  call void (...) @_ssdm_SpecArrayDimSize([1024 x %struct.gradient_t]* %5, i32 436) #0
  %6 = load [1024 x %struct.gradient_t]*, [1024 x %struct.gradient_t]** %1, align 8
  call void (...) @_ssdm_op_SpecInterface([1024 x %struct.gradient_t]* %6, i8* getelementptr inbounds ([8 x i8], [8 x i8]* @.str, i32 0, i32 0), i32 0, i32 0, i8* getelementptr inbounds ([1 x i8], [1 x i8]* @.str2, i32 0, i32 0), i32 0, i32 0, i8* getelementptr inbounds ([1 x i8], [1 x i8]* @.str2, i32 0, i32 0), i8* getelementptr inbounds ([1 x i8], [1 x i8]* @.str2, i32 0, i32 0), i8* getelementptr inbounds ([1 x i8], [1 x i8]* @.str2, i32 0, i32 0), i32 0, i32 0, i32 0, i32 0, i8* getelementptr inbounds ([1 x i8], [1 x i8]* @.str2, i32 0, i32 0), i8* getelementptr inbounds ([1 x i8], [1 x i8]* @.str2, i32 0, i32 0)) #0
  %7 = load [1024 x %struct.gradient_t]*, [1024 x %struct.gradient_t]** %2, align 8
  call void (...) @_ssdm_op_SpecInterface([1024 x %struct.gradient_t]* %7, i8* getelementptr inbounds ([8 x i8], [8 x i8]* @.str, i32 0, i32 0), i32 0, i32 0, i8* getelementptr inbounds ([1 x i8], [1 x i8]* @.str2, i32 0, i32 0), i32 0, i32 0, i8* getelementptr inbounds ([1 x i8], [1 x i8]* @.str2, i32 0, i32 0), i8* getelementptr inbounds ([1 x i8], [1 x i8]* @.str2, i32 0, i32 0), i8* getelementptr inbounds ([1 x i8], [1 x i8]* @.str2, i32 0, i32 0), i32 0, i32 0, i32 0, i32 0, i8* getelementptr inbounds ([1 x i8], [1 x i8]* @.str2, i32 0, i32 0), i8* getelementptr inbounds ([1 x i8], [1 x i8]* @.str2, i32 0, i32 0)) #0
  call void @_ZN3hls6WindowILi1ELi7E10gradient_tEC1Ev(%"class.hls::Window.0"* %buf)
  call void (...) @_ssdm_SpecConstant(float* getelementptr inbounds ([7 x float], [7 x float]* @_ZZ17gradient_weight_xPA1024_10gradient_tS1_E11GRAD_FILTER, i32 0, i32 0)) #0
  br label %8

; <label>:8:                                      ; preds = %0
  store i32 0, i32* %r, align 4
  br label %9

; <label>:9:                                      ; preds = %157, %8
  %10 = load i32, i32* %r, align 4
  %11 = icmp slt i32 %10, 436
  br i1 %11, label %12, label %160

; <label>:12:                                     ; preds = %9
  call void (...) @_ssdm_op_SpecLoopName(i8* getelementptr inbounds ([20 x i8], [20 x i8]* @.str12, i32 0, i32 0)) #0
  call void (...) @_ssdm_RegionBegin(i8* getelementptr inbounds ([20 x i8], [20 x i8]* @.str12, i32 0, i32 0)) #0
  br label %13

; <label>:13:                                     ; preds = %12
  store i32 0, i32* %c, align 4
  br label %14

; <label>:14:                                     ; preds = %153, %13
  %15 = load i32, i32* %c, align 4
  %16 = icmp slt i32 %15, 1027
  br i1 %16, label %17, label %156

; <label>:17:                                     ; preds = %14
  call void (...) @_ssdm_op_SpecLoopName(i8* getelementptr inbounds ([20 x i8], [20 x i8]* @.str13, i32 0, i32 0)) #0
  call void (...) @_ssdm_RegionBegin(i8* getelementptr inbounds ([20 x i8], [20 x i8]* @.str13, i32 0, i32 0)) #0
  call void (...) @_ssdm_op_SpecPipeline(i32 1, i32 1, i32 1, i32 0, i8* getelementptr inbounds ([1 x i8], [1 x i8]* @.str2, i32 0, i32 0)) #0
  call void @_ZN3hls6WindowILi1ELi7E10gradient_tE17shift_pixels_leftEv(%"class.hls::Window.0"* %buf)
  %18 = load i32, i32* %c, align 4
  %19 = icmp slt i32 %18, 1024
  br i1 %19, label %20, label %39

; <label>:20:                                     ; preds = %17
  %21 = load i32, i32* %c, align 4
  %22 = sext i32 %21 to i64
  %23 = load i32, i32* %r, align 4
  %24 = sext i32 %23 to i64
  %25 = load [1024 x %struct.gradient_t]*, [1024 x %struct.gradient_t]** %1, align 8
  %26 = getelementptr inbounds [1024 x %struct.gradient_t], [1024 x %struct.gradient_t]* %25, i64 %24
  %27 = getelementptr inbounds [1024 x %struct.gradient_t], [1024 x %struct.gradient_t]* %26, i32 0, i64 %22
  %28 = bitcast %struct.gradient_t* %tmp to i8*
  %29 = bitcast %struct.gradient_t* %27 to i8*
  %30 = getelementptr inbounds %struct.gradient_t, %struct.gradient_t* %tmp, i32 0, i32 0
  %31 = getelementptr inbounds %struct.gradient_t, %struct.gradient_t* %27, i32 0, i32 0
  %32 = load float, float* %31
  store float %32, float* %30
  %33 = getelementptr inbounds %struct.gradient_t, %struct.gradient_t* %tmp, i32 0, i32 1
  %34 = getelementptr inbounds %struct.gradient_t, %struct.gradient_t* %27, i32 0, i32 1
  %35 = load float, float* %34
  store float %35, float* %33
  %36 = getelementptr inbounds %struct.gradient_t, %struct.gradient_t* %tmp, i32 0, i32 2
  %37 = getelementptr inbounds %struct.gradient_t, %struct.gradient_t* %27, i32 0, i32 2
  %38 = load float, float* %37
  store float %38, float* %36
  br label %43

; <label>:39:                                     ; preds = %17
  %40 = getelementptr inbounds %struct.gradient_t, %struct.gradient_t* %tmp, i32 0, i32 0
  store float 0.000000e+00, float* %40, align 4
  %41 = getelementptr inbounds %struct.gradient_t, %struct.gradient_t* %tmp, i32 0, i32 1
  store float 0.000000e+00, float* %41, align 4
  %42 = getelementptr inbounds %struct.gradient_t, %struct.gradient_t* %tmp, i32 0, i32 2
  store float 0.000000e+00, float* %42, align 4
  br label %43

; <label>:43:                                     ; preds = %39, %20
  %44 = bitcast %struct.gradient_t* %3 to i8*
  %45 = bitcast %struct.gradient_t* %tmp to i8*
  %46 = getelementptr inbounds %struct.gradient_t, %struct.gradient_t* %3, i32 0, i32 0
  %47 = getelementptr inbounds %struct.gradient_t, %struct.gradient_t* %tmp, i32 0, i32 0
  %48 = load float, float* %47
  store float %48, float* %46
  %49 = getelementptr inbounds %struct.gradient_t, %struct.gradient_t* %3, i32 0, i32 1
  %50 = getelementptr inbounds %struct.gradient_t, %struct.gradient_t* %tmp, i32 0, i32 1
  %51 = load float, float* %50
  store float %51, float* %49
  %52 = getelementptr inbounds %struct.gradient_t, %struct.gradient_t* %3, i32 0, i32 2
  %53 = getelementptr inbounds %struct.gradient_t, %struct.gradient_t* %tmp, i32 0, i32 2
  %54 = load float, float* %53
  store float %54, float* %52
  call void @_ZN3hls6WindowILi1ELi7E10gradient_tE12insert_pixelES1_ii(%"class.hls::Window.0"* %buf, %struct.gradient_t* byval align 8 %3, i32 0, i32 6)
  %55 = getelementptr inbounds %struct.gradient_t, %struct.gradient_t* %acc, i32 0, i32 0
  store float 0.000000e+00, float* %55, align 4
  %56 = getelementptr inbounds %struct.gradient_t, %struct.gradient_t* %acc, i32 0, i32 1
  store float 0.000000e+00, float* %56, align 4
  %57 = getelementptr inbounds %struct.gradient_t, %struct.gradient_t* %acc, i32 0, i32 2
  store float 0.000000e+00, float* %57, align 4
  %58 = load i32, i32* %c, align 4
  %59 = icmp sge i32 %58, 6
  br i1 %59, label %60, label %128

; <label>:60:                                     ; preds = %43
  %61 = load i32, i32* %c, align 4
  %62 = icmp slt i32 %61, 1024
  br i1 %62, label %63, label %128

; <label>:63:                                     ; preds = %60
  br label %64

; <label>:64:                                     ; preds = %63
  store i32 0, i32* %i, align 4
  br label %65

; <label>:65:                                     ; preds = %105, %64
  %66 = load i32, i32* %i, align 4
  %67 = icmp slt i32 %66, 7
  br i1 %67, label %68, label %108

; <label>:68:                                     ; preds = %65
  call void (...) @_ssdm_op_SpecLoopName(i8* getelementptr inbounds ([18 x i8], [18 x i8]* @.str14, i32 0, i32 0)) #0
  call void (...) @_ssdm_RegionBegin(i8* getelementptr inbounds ([18 x i8], [18 x i8]* @.str14, i32 0, i32 0)) #0
  %69 = load i32, i32* %i, align 4
  %70 = call %struct.gradient_t* @_ZN3hls6WindowILi1ELi7E10gradient_tE6getvalEii(%"class.hls::Window.0"* %buf, i32 0, i32 %69)
  %71 = getelementptr inbounds %struct.gradient_t, %struct.gradient_t* %70, i32 0, i32 0
  %72 = load float, float* %71, align 4
  %73 = load i32, i32* %i, align 4
  %74 = sext i32 %73 to i64
  %75 = getelementptr inbounds [7 x float], [7 x float]* @_ZZ17gradient_weight_xPA1024_10gradient_tS1_E11GRAD_FILTER, i32 0, i64 %74
  %76 = load float, float* %75, align 4
  %77 = fmul float %72, %76
  %78 = getelementptr inbounds %struct.gradient_t, %struct.gradient_t* %acc, i32 0, i32 0
  %79 = load float, float* %78, align 4
  %80 = fadd float %79, %77
  store float %80, float* %78, align 4
  %81 = load i32, i32* %i, align 4
  %82 = call %struct.gradient_t* @_ZN3hls6WindowILi1ELi7E10gradient_tE6getvalEii(%"class.hls::Window.0"* %buf, i32 0, i32 %81)
  %83 = getelementptr inbounds %struct.gradient_t, %struct.gradient_t* %82, i32 0, i32 1
  %84 = load float, float* %83, align 4
  %85 = load i32, i32* %i, align 4
  %86 = sext i32 %85 to i64
  %87 = getelementptr inbounds [7 x float], [7 x float]* @_ZZ17gradient_weight_xPA1024_10gradient_tS1_E11GRAD_FILTER, i32 0, i64 %86
  %88 = load float, float* %87, align 4
  %89 = fmul float %84, %88
  %90 = getelementptr inbounds %struct.gradient_t, %struct.gradient_t* %acc, i32 0, i32 1
  %91 = load float, float* %90, align 4
  %92 = fadd float %91, %89
  store float %92, float* %90, align 4
  %93 = load i32, i32* %i, align 4
  %94 = call %struct.gradient_t* @_ZN3hls6WindowILi1ELi7E10gradient_tE6getvalEii(%"class.hls::Window.0"* %buf, i32 0, i32 %93)
  %95 = getelementptr inbounds %struct.gradient_t, %struct.gradient_t* %94, i32 0, i32 2
  %96 = load float, float* %95, align 4
  %97 = load i32, i32* %i, align 4
  %98 = sext i32 %97 to i64
  %99 = getelementptr inbounds [7 x float], [7 x float]* @_ZZ17gradient_weight_xPA1024_10gradient_tS1_E11GRAD_FILTER, i32 0, i64 %98
  %100 = load float, float* %99, align 4
  %101 = fmul float %96, %100
  %102 = getelementptr inbounds %struct.gradient_t, %struct.gradient_t* %acc, i32 0, i32 2
  %103 = load float, float* %102, align 4
  %104 = fadd float %103, %101
  store float %104, float* %102, align 4
  call void (...) @_ssdm_RegionEnd(i8* getelementptr inbounds ([18 x i8], [18 x i8]* @.str14, i32 0, i32 0)) #0
  br label %105

; <label>:105:                                    ; preds = %68
  %106 = load i32, i32* %i, align 4
  %107 = add nsw i32 %106, 1
  store i32 %107, i32* %i, align 4
  br label %65

; <label>:108:                                    ; preds = %65
  %109 = load i32, i32* %c, align 4
  %110 = sub nsw i32 %109, 3
  %111 = sext i32 %110 to i64
  %112 = load i32, i32* %r, align 4
  %113 = sext i32 %112 to i64
  %114 = load [1024 x %struct.gradient_t]*, [1024 x %struct.gradient_t]** %2, align 8
  %115 = getelementptr inbounds [1024 x %struct.gradient_t], [1024 x %struct.gradient_t]* %114, i64 %113
  %116 = getelementptr inbounds [1024 x %struct.gradient_t], [1024 x %struct.gradient_t]* %115, i32 0, i64 %111
  %117 = bitcast %struct.gradient_t* %116 to i8*
  %118 = bitcast %struct.gradient_t* %acc to i8*
  %119 = getelementptr inbounds %struct.gradient_t, %struct.gradient_t* %116, i32 0, i32 0
  %120 = getelementptr inbounds %struct.gradient_t, %struct.gradient_t* %acc, i32 0, i32 0
  %121 = load float, float* %120
  store float %121, float* %119
  %122 = getelementptr inbounds %struct.gradient_t, %struct.gradient_t* %116, i32 0, i32 1
  %123 = getelementptr inbounds %struct.gradient_t, %struct.gradient_t* %acc, i32 0, i32 1
  %124 = load float, float* %123
  store float %124, float* %122
  %125 = getelementptr inbounds %struct.gradient_t, %struct.gradient_t* %116, i32 0, i32 2
  %126 = getelementptr inbounds %struct.gradient_t, %struct.gradient_t* %acc, i32 0, i32 2
  %127 = load float, float* %126
  store float %127, float* %125
  br label %152

; <label>:128:                                    ; preds = %60, %43
  %129 = load i32, i32* %c, align 4
  %130 = icmp sge i32 %129, 3
  br i1 %130, label %131, label %151

; <label>:131:                                    ; preds = %128
  %132 = load i32, i32* %c, align 4
  %133 = sub nsw i32 %132, 3
  %134 = sext i32 %133 to i64
  %135 = load i32, i32* %r, align 4
  %136 = sext i32 %135 to i64
  %137 = load [1024 x %struct.gradient_t]*, [1024 x %struct.gradient_t]** %2, align 8
  %138 = getelementptr inbounds [1024 x %struct.gradient_t], [1024 x %struct.gradient_t]* %137, i64 %136
  %139 = getelementptr inbounds [1024 x %struct.gradient_t], [1024 x %struct.gradient_t]* %138, i32 0, i64 %134
  %140 = bitcast %struct.gradient_t* %139 to i8*
  %141 = bitcast %struct.gradient_t* %acc to i8*
  %142 = getelementptr inbounds %struct.gradient_t, %struct.gradient_t* %139, i32 0, i32 0
  %143 = getelementptr inbounds %struct.gradient_t, %struct.gradient_t* %acc, i32 0, i32 0
  %144 = load float, float* %143
  store float %144, float* %142
  %145 = getelementptr inbounds %struct.gradient_t, %struct.gradient_t* %139, i32 0, i32 1
  %146 = getelementptr inbounds %struct.gradient_t, %struct.gradient_t* %acc, i32 0, i32 1
  %147 = load float, float* %146
  store float %147, float* %145
  %148 = getelementptr inbounds %struct.gradient_t, %struct.gradient_t* %139, i32 0, i32 2
  %149 = getelementptr inbounds %struct.gradient_t, %struct.gradient_t* %acc, i32 0, i32 2
  %150 = load float, float* %149
  store float %150, float* %148
  br label %151

; <label>:151:                                    ; preds = %131, %128
  br label %152

; <label>:152:                                    ; preds = %151, %108
  call void (...) @_ssdm_RegionEnd(i8* getelementptr inbounds ([20 x i8], [20 x i8]* @.str13, i32 0, i32 0)) #0
  br label %153

; <label>:153:                                    ; preds = %152
  %154 = load i32, i32* %c, align 4
  %155 = add nsw i32 %154, 1
  store i32 %155, i32* %c, align 4
  br label %14

; <label>:156:                                    ; preds = %14
  call void (...) @_ssdm_RegionEnd(i8* getelementptr inbounds ([20 x i8], [20 x i8]* @.str12, i32 0, i32 0)) #0
  br label %157

; <label>:157:                                    ; preds = %156
  %158 = load i32, i32* %r, align 4
  %159 = add nsw i32 %158, 1
  store i32 %159, i32* %r, align 4
  br label %9

; <label>:160:                                    ; preds = %9
  ret void
}

; Function Attrs: nounwind uwtable
define linkonce_odr void @_ZN3hls6WindowILi1ELi7E10gradient_tEC1Ev(%"class.hls::Window.0"* %this) unnamed_addr #2 comdat align 2 {
  %1 = alloca %"class.hls::Window.0"*, align 8
  store %"class.hls::Window.0"* %this, %"class.hls::Window.0"** %1, align 8
  %2 = load %"class.hls::Window.0"*, %"class.hls::Window.0"** %1
  call void @_ZN3hls6WindowILi1ELi7E10gradient_tEC2Ev(%"class.hls::Window.0"* %2)
  ret void
}

; Function Attrs: nounwind uwtable
define linkonce_odr void @_ZN3hls6WindowILi1ELi7E10gradient_tE17shift_pixels_leftEv(%"class.hls::Window.0"* %this) #2 comdat align 2 {
  %1 = alloca %"class.hls::Window.0"*, align 8
  %i = alloca %struct.ap_uint.15, align 4
  %j = alloca %struct.ap_uint.15, align 4
  %2 = alloca %struct.ap_uint.15, align 4
  %3 = alloca %struct.ap_uint.15, align 4
  %4 = alloca %struct.ap_int_base.18, align 8
  %5 = alloca %struct.ap_int_base.16, align 4
  %6 = alloca %struct.ap_int_base.16, align 4
  store %"class.hls::Window.0"* %this, %"class.hls::Window.0"** %1, align 8
  %7 = load %"class.hls::Window.0"*, %"class.hls::Window.0"** %1
  call void (...) @_ssdm_InlineSelf(i32 0, i8* getelementptr inbounds ([1 x i8], [1 x i8]* @.str2, i32 0, i32 0)) #0
  call void @_ZN7ap_uintILi32EEC1Ev(%struct.ap_uint.15* %i)
  call void @_ZN7ap_uintILi32EEC1Ev(%struct.ap_uint.15* %j)
  call void @_ZN7ap_uintILi32EEC1Ei(%struct.ap_uint.15* %2, i32 0)
  %8 = call %struct.ap_uint.15* @_ZN7ap_uintILi32EEaSERKS0_(%struct.ap_uint.15* %i, %struct.ap_uint.15* %2)
  br label %9

; <label>:9:                                      ; preds = %49, %0
  %10 = bitcast %struct.ap_uint.15* %i to %struct.ap_int_base.16*
  %11 = call zeroext i1 @_ZltILi32ELb0EEbRK11ap_int_baseIXT_EXT0_EXleT_Li64EEEi(%struct.ap_int_base.16* %10, i32 1)
  br i1 %11, label %12, label %51

; <label>:12:                                     ; preds = %9
  call void (...) @_ssdm_RegionBegin(i8* getelementptr inbounds ([12 x i8], [12 x i8]* @.str39, i32 0, i32 0)) #0
  call void (...) @_ssdm_Unroll(i32 0, i32 0, i32 0, i8* getelementptr inbounds ([1 x i8], [1 x i8]* @.str2, i32 0, i32 0)) #0
  call void @_ZN7ap_uintILi32EEC1Ei(%struct.ap_uint.15* %3, i32 0)
  %13 = call %struct.ap_uint.15* @_ZN7ap_uintILi32EEaSERKS0_(%struct.ap_uint.15* %j, %struct.ap_uint.15* %3)
  br label %14

; <label>:14:                                     ; preds = %46, %12
  %15 = bitcast %struct.ap_uint.15* %j to %struct.ap_int_base.16*
  %16 = call zeroext i1 @_ZltILi32ELb0EEbRK11ap_int_baseIXT_EXT0_EXleT_Li64EEEi(%struct.ap_int_base.16* %15, i32 6)
  br i1 %16, label %17, label %48

; <label>:17:                                     ; preds = %14
  call void (...) @_ssdm_RegionBegin(i8* getelementptr inbounds ([12 x i8], [12 x i8]* @.str40, i32 0, i32 0)) #0
  call void (...) @_ssdm_Unroll(i32 0, i32 0, i32 0, i8* getelementptr inbounds ([1 x i8], [1 x i8]* @.str2, i32 0, i32 0)) #0
  %18 = bitcast %struct.ap_uint.15* %j to %struct.ap_int_base.16*
  %19 = call i32 @_ZNK11ap_int_baseILi32ELb0ELb1EEcvjEv(%struct.ap_int_base.16* %18)
  %20 = zext i32 %19 to i64
  %21 = bitcast %struct.ap_uint.15* %i to %struct.ap_int_base.16*
  %22 = call i32 @_ZNK11ap_int_baseILi32ELb0ELb1EEcvjEv(%struct.ap_int_base.16* %21)
  %23 = zext i32 %22 to i64
  %24 = getelementptr inbounds %"class.hls::Window.0", %"class.hls::Window.0"* %7, i32 0, i32 0
  %25 = getelementptr inbounds [1 x [7 x %struct.gradient_t]], [1 x [7 x %struct.gradient_t]]* %24, i32 0, i64 %23
  %26 = getelementptr inbounds [7 x %struct.gradient_t], [7 x %struct.gradient_t]* %25, i32 0, i64 %20
  %27 = bitcast %struct.ap_uint.15* %j to %struct.ap_int_base.16*
  call void @_ZplILi32ELb0EEN11ap_int_baseIXT_EXT0_EXleT_Li64EEE5RTypeIXLi32EEXLb1EEE4plusERKS1_i(%struct.ap_int_base.18* sret %4, %struct.ap_int_base.16* %27, i32 1)
  %28 = call i64 @_ZNK11ap_int_baseILi34ELb1ELb1EEcvxEv(%struct.ap_int_base.18* %4)
  %29 = bitcast %struct.ap_uint.15* %i to %struct.ap_int_base.16*
  %30 = call i32 @_ZNK11ap_int_baseILi32ELb0ELb1EEcvjEv(%struct.ap_int_base.16* %29)
  %31 = zext i32 %30 to i64
  %32 = getelementptr inbounds %"class.hls::Window.0", %"class.hls::Window.0"* %7, i32 0, i32 0
  %33 = getelementptr inbounds [1 x [7 x %struct.gradient_t]], [1 x [7 x %struct.gradient_t]]* %32, i32 0, i64 %31
  %34 = getelementptr inbounds [7 x %struct.gradient_t], [7 x %struct.gradient_t]* %33, i32 0, i64 %28
  %35 = bitcast %struct.gradient_t* %26 to i8*
  %36 = bitcast %struct.gradient_t* %34 to i8*
  %37 = getelementptr inbounds %struct.gradient_t, %struct.gradient_t* %26, i32 0, i32 0
  %38 = getelementptr inbounds %struct.gradient_t, %struct.gradient_t* %34, i32 0, i32 0
  %39 = load float, float* %38
  store float %39, float* %37
  %40 = getelementptr inbounds %struct.gradient_t, %struct.gradient_t* %26, i32 0, i32 1
  %41 = getelementptr inbounds %struct.gradient_t, %struct.gradient_t* %34, i32 0, i32 1
  %42 = load float, float* %41
  store float %42, float* %40
  %43 = getelementptr inbounds %struct.gradient_t, %struct.gradient_t* %26, i32 0, i32 2
  %44 = getelementptr inbounds %struct.gradient_t, %struct.gradient_t* %34, i32 0, i32 2
  %45 = load float, float* %44
  store float %45, float* %43
  call void (...) @_ssdm_RegionEnd(i8* getelementptr inbounds ([12 x i8], [12 x i8]* @.str40, i32 0, i32 0)) #0
  br label %46

; <label>:46:                                     ; preds = %17
  %47 = bitcast %struct.ap_uint.15* %j to %struct.ap_int_base.16*
  call void @_ZN11ap_int_baseILi32ELb0ELb1EEppEi(%struct.ap_int_base.16* sret %5, %struct.ap_int_base.16* %47, i32 0)
  br label %14

; <label>:48:                                     ; preds = %14
  call void (...) @_ssdm_RegionEnd(i8* getelementptr inbounds ([12 x i8], [12 x i8]* @.str39, i32 0, i32 0)) #0
  br label %49

; <label>:49:                                     ; preds = %48
  %50 = bitcast %struct.ap_uint.15* %i to %struct.ap_int_base.16*
  call void @_ZN11ap_int_baseILi32ELb0ELb1EEppEi(%struct.ap_int_base.16* sret %6, %struct.ap_int_base.16* %50, i32 0)
  br label %9

; <label>:51:                                     ; preds = %9
  ret void
}

; Function Attrs: nounwind uwtable
define linkonce_odr void @_ZN3hls6WindowILi1ELi7E10gradient_tE12insert_pixelES1_ii(%"class.hls::Window.0"* %this, %struct.gradient_t* byval align 8 %value, i32 %row, i32 %col) #2 comdat align 2 {
  %1 = alloca %"class.hls::Window.0"*, align 8
  %2 = alloca i32, align 4
  %3 = alloca i32, align 4
  store %"class.hls::Window.0"* %this, %"class.hls::Window.0"** %1, align 8
  store i32 %row, i32* %2, align 4
  store i32 %col, i32* %3, align 4
  %4 = load %"class.hls::Window.0"*, %"class.hls::Window.0"** %1
  call void (...) @_ssdm_InlineSelf(i32 0, i8* getelementptr inbounds ([1 x i8], [1 x i8]* @.str2, i32 0, i32 0)) #0
  %5 = load i32, i32* %2, align 4
  %6 = icmp sge i32 %5, 0
  br i1 %6, label %7, label %16

; <label>:7:                                      ; preds = %0
  %8 = load i32, i32* %2, align 4
  %9 = icmp slt i32 %8, 1
  br i1 %9, label %10, label %16

; <label>:10:                                     ; preds = %7
  %11 = load i32, i32* %3, align 4
  %12 = icmp sge i32 %11, 0
  br i1 %12, label %13, label %16

; <label>:13:                                     ; preds = %10
  %14 = load i32, i32* %3, align 4
  %15 = icmp slt i32 %14, 7
  br label %16

; <label>:16:                                     ; preds = %13, %10, %7, %0
  %17 = phi i1 [ false, %10 ], [ false, %7 ], [ false, %0 ], [ %15, %13 ]
  br i1 %17, label %18, label %19

; <label>:18:                                     ; preds = %16
  br label %21

; <label>:19:                                     ; preds = %16
  call void @__assert_fail(i8* getelementptr inbounds ([49 x i8], [49 x i8]* @.str33, i32 0, i32 0), i8* getelementptr inbounds ([81 x i8], [81 x i8]* @.str34, i32 0, i32 0), i32 266, i8* getelementptr inbounds ([99 x i8], [99 x i8]* @__PRETTY_FUNCTION__._ZN3hls6WindowILi1ELi7E10gradient_tE12insert_pixelES1_ii, i32 0, i32 0)) #5
  unreachable
                                                  ; No predecessors!
  br label %21

; <label>:21:                                     ; preds = %20, %18
  %22 = load i32, i32* %3, align 4
  %23 = sext i32 %22 to i64
  %24 = load i32, i32* %2, align 4
  %25 = sext i32 %24 to i64
  %26 = getelementptr inbounds %"class.hls::Window.0", %"class.hls::Window.0"* %4, i32 0, i32 0
  %27 = getelementptr inbounds [1 x [7 x %struct.gradient_t]], [1 x [7 x %struct.gradient_t]]* %26, i32 0, i64 %25
  %28 = getelementptr inbounds [7 x %struct.gradient_t], [7 x %struct.gradient_t]* %27, i32 0, i64 %23
  %29 = bitcast %struct.gradient_t* %28 to i8*
  %30 = bitcast %struct.gradient_t* %value to i8*
  %31 = getelementptr inbounds %struct.gradient_t, %struct.gradient_t* %28, i32 0, i32 0
  %32 = getelementptr inbounds %struct.gradient_t, %struct.gradient_t* %value, i32 0, i32 0
  %33 = load float, float* %32
  store float %33, float* %31
  %34 = getelementptr inbounds %struct.gradient_t, %struct.gradient_t* %28, i32 0, i32 1
  %35 = getelementptr inbounds %struct.gradient_t, %struct.gradient_t* %value, i32 0, i32 1
  %36 = load float, float* %35
  store float %36, float* %34
  %37 = getelementptr inbounds %struct.gradient_t, %struct.gradient_t* %28, i32 0, i32 2
  %38 = getelementptr inbounds %struct.gradient_t, %struct.gradient_t* %value, i32 0, i32 2
  %39 = load float, float* %38
  store float %39, float* %37
  ret void
}

; Function Attrs: nounwind uwtable
define linkonce_odr %struct.gradient_t* @_ZN3hls6WindowILi1ELi7E10gradient_tE6getvalEii(%"class.hls::Window.0"* %this, i32 %row, i32 %col) #2 comdat align 2 {
  %1 = alloca %"class.hls::Window.0"*, align 8
  %2 = alloca i32, align 4
  %3 = alloca i32, align 4
  store %"class.hls::Window.0"* %this, %"class.hls::Window.0"** %1, align 8
  store i32 %row, i32* %2, align 4
  store i32 %col, i32* %3, align 4
  %4 = load %"class.hls::Window.0"*, %"class.hls::Window.0"** %1
  call void (...) @_ssdm_InlineSelf(i32 0, i8* getelementptr inbounds ([1 x i8], [1 x i8]* @.str2, i32 0, i32 0)) #0
  %5 = load i32, i32* %2, align 4
  %6 = icmp sge i32 %5, 0
  br i1 %6, label %7, label %16

; <label>:7:                                      ; preds = %0
  %8 = load i32, i32* %2, align 4
  %9 = icmp slt i32 %8, 1
  br i1 %9, label %10, label %16

; <label>:10:                                     ; preds = %7
  %11 = load i32, i32* %3, align 4
  %12 = icmp sge i32 %11, 0
  br i1 %12, label %13, label %16

; <label>:13:                                     ; preds = %10
  %14 = load i32, i32* %3, align 4
  %15 = icmp slt i32 %14, 7
  br label %16

; <label>:16:                                     ; preds = %13, %10, %7, %0
  %17 = phi i1 [ false, %10 ], [ false, %7 ], [ false, %0 ], [ %15, %13 ]
  br i1 %17, label %18, label %19

; <label>:18:                                     ; preds = %16
  br label %21

; <label>:19:                                     ; preds = %16
  call void @__assert_fail(i8* getelementptr inbounds ([49 x i8], [49 x i8]* @.str33, i32 0, i32 0), i8* getelementptr inbounds ([81 x i8], [81 x i8]* @.str34, i32 0, i32 0), i32 492, i8* getelementptr inbounds ([88 x i8], [88 x i8]* @__PRETTY_FUNCTION__._ZN3hls6WindowILi1ELi7E10gradient_tE6getvalEii, i32 0, i32 0)) #5
  unreachable
                                                  ; No predecessors!
  br label %21

; <label>:21:                                     ; preds = %20, %18
  %22 = load i32, i32* %3, align 4
  %23 = sext i32 %22 to i64
  %24 = load i32, i32* %2, align 4
  %25 = sext i32 %24 to i64
  %26 = getelementptr inbounds %"class.hls::Window.0", %"class.hls::Window.0"* %4, i32 0, i32 0
  %27 = getelementptr inbounds [1 x [7 x %struct.gradient_t]], [1 x [7 x %struct.gradient_t]]* %26, i32 0, i64 %25
  %28 = getelementptr inbounds [7 x %struct.gradient_t], [7 x %struct.gradient_t]* %27, i32 0, i64 %23
  ret %struct.gradient_t* %28
}

; Function Attrs: nounwind uwtable
define void @_Z13outer_productPA1024_10gradient_tPA1024_7outer_t([1024 x %struct.gradient_t]* %gradient, [1024 x %struct.outer_t]* %outer_product) #2 {
  %1 = alloca [1024 x %struct.gradient_t]*, align 8
  %2 = alloca [1024 x %struct.outer_t]*, align 8
  %r = alloca i32, align 4
  %c = alloca i32, align 4
  %grad = alloca %struct.gradient_t, align 4
  %out = alloca %struct.outer_t, align 4
  store [1024 x %struct.gradient_t]* %gradient, [1024 x %struct.gradient_t]** %1, align 8
  store [1024 x %struct.outer_t]* %outer_product, [1024 x %struct.outer_t]** %2, align 8
  %3 = load [1024 x %struct.gradient_t]*, [1024 x %struct.gradient_t]** %1, align 8
  call void (...) @_ssdm_SpecArrayDimSize([1024 x %struct.gradient_t]* %3, i32 436) #0
  %4 = load [1024 x %struct.outer_t]*, [1024 x %struct.outer_t]** %2, align 8
  call void (...) @_ssdm_SpecArrayDimSize([1024 x %struct.outer_t]* %4, i32 436) #0
  %5 = load [1024 x %struct.gradient_t]*, [1024 x %struct.gradient_t]** %1, align 8
  call void (...) @_ssdm_op_SpecInterface([1024 x %struct.gradient_t]* %5, i8* getelementptr inbounds ([8 x i8], [8 x i8]* @.str, i32 0, i32 0), i32 0, i32 0, i8* getelementptr inbounds ([1 x i8], [1 x i8]* @.str2, i32 0, i32 0), i32 0, i32 0, i8* getelementptr inbounds ([1 x i8], [1 x i8]* @.str2, i32 0, i32 0), i8* getelementptr inbounds ([1 x i8], [1 x i8]* @.str2, i32 0, i32 0), i8* getelementptr inbounds ([1 x i8], [1 x i8]* @.str2, i32 0, i32 0), i32 0, i32 0, i32 0, i32 0, i8* getelementptr inbounds ([1 x i8], [1 x i8]* @.str2, i32 0, i32 0), i8* getelementptr inbounds ([1 x i8], [1 x i8]* @.str2, i32 0, i32 0)) #0
  %6 = load [1024 x %struct.outer_t]*, [1024 x %struct.outer_t]** %2, align 8
  call void (...) @_ssdm_op_SpecInterface([1024 x %struct.outer_t]* %6, i8* getelementptr inbounds ([8 x i8], [8 x i8]* @.str, i32 0, i32 0), i32 0, i32 0, i8* getelementptr inbounds ([1 x i8], [1 x i8]* @.str2, i32 0, i32 0), i32 0, i32 0, i8* getelementptr inbounds ([1 x i8], [1 x i8]* @.str2, i32 0, i32 0), i8* getelementptr inbounds ([1 x i8], [1 x i8]* @.str2, i32 0, i32 0), i8* getelementptr inbounds ([1 x i8], [1 x i8]* @.str2, i32 0, i32 0), i32 0, i32 0, i32 0, i32 0, i8* getelementptr inbounds ([1 x i8], [1 x i8]* @.str2, i32 0, i32 0), i8* getelementptr inbounds ([1 x i8], [1 x i8]* @.str2, i32 0, i32 0)) #0
  br label %7

; <label>:7:                                      ; preds = %0
  store i32 0, i32* %r, align 4
  br label %8

; <label>:8:                                      ; preds = %110, %7
  %9 = load i32, i32* %r, align 4
  %10 = icmp slt i32 %9, 436
  br i1 %10, label %11, label %113

; <label>:11:                                     ; preds = %8
  call void (...) @_ssdm_op_SpecLoopName(i8* getelementptr inbounds ([12 x i8], [12 x i8]* @.str15, i32 0, i32 0)) #0
  call void (...) @_ssdm_RegionBegin(i8* getelementptr inbounds ([12 x i8], [12 x i8]* @.str15, i32 0, i32 0)) #0
  br label %12

; <label>:12:                                     ; preds = %11
  store i32 0, i32* %c, align 4
  br label %13

; <label>:13:                                     ; preds = %106, %12
  %14 = load i32, i32* %c, align 4
  %15 = icmp slt i32 %14, 1024
  br i1 %15, label %16, label %109

; <label>:16:                                     ; preds = %13
  call void (...) @_ssdm_op_SpecLoopName(i8* getelementptr inbounds ([12 x i8], [12 x i8]* @.str16, i32 0, i32 0)) #0
  call void (...) @_ssdm_RegionBegin(i8* getelementptr inbounds ([12 x i8], [12 x i8]* @.str16, i32 0, i32 0)) #0
  call void (...) @_ssdm_op_SpecPipeline(i32 1, i32 1, i32 1, i32 0, i8* getelementptr inbounds ([1 x i8], [1 x i8]* @.str2, i32 0, i32 0)) #0
  %17 = load i32, i32* %c, align 4
  %18 = sext i32 %17 to i64
  %19 = load i32, i32* %r, align 4
  %20 = sext i32 %19 to i64
  %21 = load [1024 x %struct.gradient_t]*, [1024 x %struct.gradient_t]** %1, align 8
  %22 = getelementptr inbounds [1024 x %struct.gradient_t], [1024 x %struct.gradient_t]* %21, i64 %20
  %23 = getelementptr inbounds [1024 x %struct.gradient_t], [1024 x %struct.gradient_t]* %22, i32 0, i64 %18
  %24 = bitcast %struct.gradient_t* %grad to i8*
  %25 = bitcast %struct.gradient_t* %23 to i8*
  %26 = getelementptr inbounds %struct.gradient_t, %struct.gradient_t* %grad, i32 0, i32 0
  %27 = getelementptr inbounds %struct.gradient_t, %struct.gradient_t* %23, i32 0, i32 0
  %28 = load float, float* %27
  store float %28, float* %26
  %29 = getelementptr inbounds %struct.gradient_t, %struct.gradient_t* %grad, i32 0, i32 1
  %30 = getelementptr inbounds %struct.gradient_t, %struct.gradient_t* %23, i32 0, i32 1
  %31 = load float, float* %30
  store float %31, float* %29
  %32 = getelementptr inbounds %struct.gradient_t, %struct.gradient_t* %grad, i32 0, i32 2
  %33 = getelementptr inbounds %struct.gradient_t, %struct.gradient_t* %23, i32 0, i32 2
  %34 = load float, float* %33
  store float %34, float* %32
  %35 = getelementptr inbounds %struct.gradient_t, %struct.gradient_t* %grad, i32 0, i32 0
  %36 = load float, float* %35, align 4
  %37 = getelementptr inbounds %struct.gradient_t, %struct.gradient_t* %grad, i32 0, i32 0
  %38 = load float, float* %37, align 4
  %39 = fmul float %36, %38
  %40 = getelementptr inbounds %struct.outer_t, %struct.outer_t* %out, i32 0, i32 0
  %41 = getelementptr inbounds [6 x float], [6 x float]* %40, i32 0, i64 0
  store float %39, float* %41, align 4
  %42 = getelementptr inbounds %struct.gradient_t, %struct.gradient_t* %grad, i32 0, i32 1
  %43 = load float, float* %42, align 4
  %44 = getelementptr inbounds %struct.gradient_t, %struct.gradient_t* %grad, i32 0, i32 1
  %45 = load float, float* %44, align 4
  %46 = fmul float %43, %45
  %47 = getelementptr inbounds %struct.outer_t, %struct.outer_t* %out, i32 0, i32 0
  %48 = getelementptr inbounds [6 x float], [6 x float]* %47, i32 0, i64 1
  store float %46, float* %48, align 4
  %49 = getelementptr inbounds %struct.gradient_t, %struct.gradient_t* %grad, i32 0, i32 2
  %50 = load float, float* %49, align 4
  %51 = getelementptr inbounds %struct.gradient_t, %struct.gradient_t* %grad, i32 0, i32 2
  %52 = load float, float* %51, align 4
  %53 = fmul float %50, %52
  %54 = getelementptr inbounds %struct.outer_t, %struct.outer_t* %out, i32 0, i32 0
  %55 = getelementptr inbounds [6 x float], [6 x float]* %54, i32 0, i64 2
  store float %53, float* %55, align 4
  %56 = getelementptr inbounds %struct.gradient_t, %struct.gradient_t* %grad, i32 0, i32 0
  %57 = load float, float* %56, align 4
  %58 = getelementptr inbounds %struct.gradient_t, %struct.gradient_t* %grad, i32 0, i32 1
  %59 = load float, float* %58, align 4
  %60 = fmul float %57, %59
  %61 = getelementptr inbounds %struct.outer_t, %struct.outer_t* %out, i32 0, i32 0
  %62 = getelementptr inbounds [6 x float], [6 x float]* %61, i32 0, i64 3
  store float %60, float* %62, align 4
  %63 = getelementptr inbounds %struct.gradient_t, %struct.gradient_t* %grad, i32 0, i32 0
  %64 = load float, float* %63, align 4
  %65 = getelementptr inbounds %struct.gradient_t, %struct.gradient_t* %grad, i32 0, i32 2
  %66 = load float, float* %65, align 4
  %67 = fmul float %64, %66
  %68 = getelementptr inbounds %struct.outer_t, %struct.outer_t* %out, i32 0, i32 0
  %69 = getelementptr inbounds [6 x float], [6 x float]* %68, i32 0, i64 4
  store float %67, float* %69, align 4
  %70 = getelementptr inbounds %struct.gradient_t, %struct.gradient_t* %grad, i32 0, i32 1
  %71 = load float, float* %70, align 4
  %72 = getelementptr inbounds %struct.gradient_t, %struct.gradient_t* %grad, i32 0, i32 2
  %73 = load float, float* %72, align 4
  %74 = fmul float %71, %73
  %75 = getelementptr inbounds %struct.outer_t, %struct.outer_t* %out, i32 0, i32 0
  %76 = getelementptr inbounds [6 x float], [6 x float]* %75, i32 0, i64 5
  store float %74, float* %76, align 4
  %77 = load i32, i32* %c, align 4
  %78 = sext i32 %77 to i64
  %79 = load i32, i32* %r, align 4
  %80 = sext i32 %79 to i64
  %81 = load [1024 x %struct.outer_t]*, [1024 x %struct.outer_t]** %2, align 8
  %82 = getelementptr inbounds [1024 x %struct.outer_t], [1024 x %struct.outer_t]* %81, i64 %80
  %83 = getelementptr inbounds [1024 x %struct.outer_t], [1024 x %struct.outer_t]* %82, i32 0, i64 %78
  %84 = bitcast %struct.outer_t* %83 to i8*
  %85 = bitcast %struct.outer_t* %out to i8*
  %86 = getelementptr inbounds %struct.outer_t, %struct.outer_t* %83, i32 0, i32 0
  %87 = getelementptr inbounds %struct.outer_t, %struct.outer_t* %out, i32 0, i32 0
  %88 = getelementptr inbounds [6 x float], [6 x float]* %86, i32 0, i32 0
  %89 = getelementptr inbounds [6 x float], [6 x float]* %87, i32 0, i32 0
  %90 = load float, float* %89
  store float %90, float* %88
  %91 = getelementptr inbounds [6 x float], [6 x float]* %86, i32 0, i32 1
  %92 = getelementptr inbounds [6 x float], [6 x float]* %87, i32 0, i32 1
  %93 = load float, float* %92
  store float %93, float* %91
  %94 = getelementptr inbounds [6 x float], [6 x float]* %86, i32 0, i32 2
  %95 = getelementptr inbounds [6 x float], [6 x float]* %87, i32 0, i32 2
  %96 = load float, float* %95
  store float %96, float* %94
  %97 = getelementptr inbounds [6 x float], [6 x float]* %86, i32 0, i32 3
  %98 = getelementptr inbounds [6 x float], [6 x float]* %87, i32 0, i32 3
  %99 = load float, float* %98
  store float %99, float* %97
  %100 = getelementptr inbounds [6 x float], [6 x float]* %86, i32 0, i32 4
  %101 = getelementptr inbounds [6 x float], [6 x float]* %87, i32 0, i32 4
  %102 = load float, float* %101
  store float %102, float* %100
  %103 = getelementptr inbounds [6 x float], [6 x float]* %86, i32 0, i32 5
  %104 = getelementptr inbounds [6 x float], [6 x float]* %87, i32 0, i32 5
  %105 = load float, float* %104
  store float %105, float* %103
  call void (...) @_ssdm_RegionEnd(i8* getelementptr inbounds ([12 x i8], [12 x i8]* @.str16, i32 0, i32 0)) #0
  br label %106

; <label>:106:                                    ; preds = %16
  %107 = load i32, i32* %c, align 4
  %108 = add nsw i32 %107, 1
  store i32 %108, i32* %c, align 4
  br label %13

; <label>:109:                                    ; preds = %13
  call void (...) @_ssdm_RegionEnd(i8* getelementptr inbounds ([12 x i8], [12 x i8]* @.str15, i32 0, i32 0)) #0
  br label %110

; <label>:110:                                    ; preds = %109
  %111 = load i32, i32* %r, align 4
  %112 = add nsw i32 %111, 1
  store i32 %112, i32* %r, align 4
  br label %8

; <label>:113:                                    ; preds = %8
  ret void
}

; Function Attrs: nounwind uwtable
define void @_Z15tensor_weight_yPA1024_7outer_tPA1024_8tensor_t([1024 x %struct.outer_t]* %outer, [1024 x %struct.tensor_t]* %tensor_y) #2 {
  %1 = alloca [1024 x %struct.outer_t]*, align 8
  %2 = alloca [1024 x %struct.tensor_t]*, align 8
  %buf = alloca %"class.hls::LineBuffer.1", align 4
  %r = alloca i32, align 4
  %c = alloca i32, align 4
  %tmp = alloca %struct.outer_t, align 4
  %i = alloca i32, align 4
  %3 = alloca %struct.outer_t, align 4
  %acc = alloca %struct.tensor_t, align 4
  %k = alloca i32, align 4
  %i1 = alloca i32, align 4
  %k2 = alloca float, align 4
  %component = alloca i32, align 4
  store [1024 x %struct.outer_t]* %outer, [1024 x %struct.outer_t]** %1, align 8
  store [1024 x %struct.tensor_t]* %tensor_y, [1024 x %struct.tensor_t]** %2, align 8
  %4 = load [1024 x %struct.outer_t]*, [1024 x %struct.outer_t]** %1, align 8
  call void (...) @_ssdm_SpecArrayDimSize([1024 x %struct.outer_t]* %4, i32 436) #0
  %5 = load [1024 x %struct.tensor_t]*, [1024 x %struct.tensor_t]** %2, align 8
  call void (...) @_ssdm_SpecArrayDimSize([1024 x %struct.tensor_t]* %5, i32 436) #0
  %6 = load [1024 x %struct.outer_t]*, [1024 x %struct.outer_t]** %1, align 8
  call void (...) @_ssdm_op_SpecInterface([1024 x %struct.outer_t]* %6, i8* getelementptr inbounds ([8 x i8], [8 x i8]* @.str, i32 0, i32 0), i32 0, i32 0, i8* getelementptr inbounds ([1 x i8], [1 x i8]* @.str2, i32 0, i32 0), i32 0, i32 0, i8* getelementptr inbounds ([1 x i8], [1 x i8]* @.str2, i32 0, i32 0), i8* getelementptr inbounds ([1 x i8], [1 x i8]* @.str2, i32 0, i32 0), i8* getelementptr inbounds ([1 x i8], [1 x i8]* @.str2, i32 0, i32 0), i32 0, i32 0, i32 0, i32 0, i8* getelementptr inbounds ([1 x i8], [1 x i8]* @.str2, i32 0, i32 0), i8* getelementptr inbounds ([1 x i8], [1 x i8]* @.str2, i32 0, i32 0)) #0
  %7 = load [1024 x %struct.tensor_t]*, [1024 x %struct.tensor_t]** %2, align 8
  call void (...) @_ssdm_op_SpecInterface([1024 x %struct.tensor_t]* %7, i8* getelementptr inbounds ([8 x i8], [8 x i8]* @.str, i32 0, i32 0), i32 0, i32 0, i8* getelementptr inbounds ([1 x i8], [1 x i8]* @.str2, i32 0, i32 0), i32 0, i32 0, i8* getelementptr inbounds ([1 x i8], [1 x i8]* @.str2, i32 0, i32 0), i8* getelementptr inbounds ([1 x i8], [1 x i8]* @.str2, i32 0, i32 0), i8* getelementptr inbounds ([1 x i8], [1 x i8]* @.str2, i32 0, i32 0), i32 0, i32 0, i32 0, i32 0, i8* getelementptr inbounds ([1 x i8], [1 x i8]* @.str2, i32 0, i32 0), i8* getelementptr inbounds ([1 x i8], [1 x i8]* @.str2, i32 0, i32 0)) #0
  call void @_ZN3hls10LineBufferILi3ELi1024E7outer_tLi0EEC1Ev(%"class.hls::LineBuffer.1"* %buf)
  call void (...) @_ssdm_SpecConstant(float* getelementptr inbounds ([3 x float], [3 x float]* @_ZZ15tensor_weight_yPA1024_7outer_tPA1024_8tensor_tE13TENSOR_FILTER, i32 0, i32 0)) #0
  br label %8

; <label>:8:                                      ; preds = %0
  store i32 0, i32* %r, align 4
  br label %9

; <label>:9:                                      ; preds = %210, %8
  %10 = load i32, i32* %r, align 4
  %11 = icmp slt i32 %10, 437
  br i1 %11, label %12, label %213

; <label>:12:                                     ; preds = %9
  call void (...) @_ssdm_op_SpecLoopName(i8* getelementptr inbounds ([22 x i8], [22 x i8]* @.str17, i32 0, i32 0)) #0
  call void (...) @_ssdm_RegionBegin(i8* getelementptr inbounds ([22 x i8], [22 x i8]* @.str17, i32 0, i32 0)) #0
  br label %13

; <label>:13:                                     ; preds = %12
  store i32 0, i32* %c, align 4
  br label %14

; <label>:14:                                     ; preds = %206, %13
  %15 = load i32, i32* %c, align 4
  %16 = icmp slt i32 %15, 1024
  br i1 %16, label %17, label %209

; <label>:17:                                     ; preds = %14
  call void (...) @_ssdm_op_SpecLoopName(i8* getelementptr inbounds ([22 x i8], [22 x i8]* @.str18, i32 0, i32 0)) #0
  call void (...) @_ssdm_RegionBegin(i8* getelementptr inbounds ([22 x i8], [22 x i8]* @.str18, i32 0, i32 0)) #0
  call void (...) @_ssdm_op_SpecPipeline(i32 1, i32 1, i32 1, i32 0, i8* getelementptr inbounds ([1 x i8], [1 x i8]* @.str2, i32 0, i32 0)) #0
  call void (...) @_ssdm_DataPack(%struct.outer_t* %tmp, i32 0, i32 0, i8* getelementptr inbounds ([1 x i8], [1 x i8]* @.str2, i32 0, i32 0), i8* getelementptr inbounds ([1 x i8], [1 x i8]* @.str2, i32 0, i32 0), i8* getelementptr inbounds ([1 x i8], [1 x i8]* @.str2, i32 0, i32 0)) #0
  %18 = getelementptr inbounds %"class.hls::LineBuffer.1", %"class.hls::LineBuffer.1"* %buf, i32 0, i32 0
  %19 = getelementptr inbounds [3 x [1024 x %struct.outer_t]], [3 x [1024 x %struct.outer_t]]* %18, i32 0, i64 0
  %20 = getelementptr inbounds [1024 x %struct.outer_t], [1024 x %struct.outer_t]* %19, i32 0, i32 0
  call void (...) @_ssdm_DataPack(%struct.outer_t* %20, i32 0, i32 0, i8* getelementptr inbounds ([1 x i8], [1 x i8]* @.str2, i32 0, i32 0), i8* getelementptr inbounds ([1 x i8], [1 x i8]* @.str2, i32 0, i32 0), i8* getelementptr inbounds ([1 x i8], [1 x i8]* @.str2, i32 0, i32 0)) #0
  %21 = load i32, i32* %c, align 4
  call void @_ZN3hls10LineBufferILi3ELi1024E7outer_tLi0EE15shift_pixels_upEi(%"class.hls::LineBuffer.1"* %buf, i32 %21)
  %22 = load i32, i32* %r, align 4
  %23 = icmp slt i32 %22, 436
  br i1 %23, label %24, label %54

; <label>:24:                                     ; preds = %17
  %25 = load i32, i32* %c, align 4
  %26 = sext i32 %25 to i64
  %27 = load i32, i32* %r, align 4
  %28 = sext i32 %27 to i64
  %29 = load [1024 x %struct.outer_t]*, [1024 x %struct.outer_t]** %1, align 8
  %30 = getelementptr inbounds [1024 x %struct.outer_t], [1024 x %struct.outer_t]* %29, i64 %28
  %31 = getelementptr inbounds [1024 x %struct.outer_t], [1024 x %struct.outer_t]* %30, i32 0, i64 %26
  %32 = bitcast %struct.outer_t* %tmp to i8*
  %33 = bitcast %struct.outer_t* %31 to i8*
  %34 = getelementptr inbounds %struct.outer_t, %struct.outer_t* %tmp, i32 0, i32 0
  %35 = getelementptr inbounds %struct.outer_t, %struct.outer_t* %31, i32 0, i32 0
  %36 = getelementptr inbounds [6 x float], [6 x float]* %34, i32 0, i32 0
  %37 = getelementptr inbounds [6 x float], [6 x float]* %35, i32 0, i32 0
  %38 = load float, float* %37
  store float %38, float* %36
  %39 = getelementptr inbounds [6 x float], [6 x float]* %34, i32 0, i32 1
  %40 = getelementptr inbounds [6 x float], [6 x float]* %35, i32 0, i32 1
  %41 = load float, float* %40
  store float %41, float* %39
  %42 = getelementptr inbounds [6 x float], [6 x float]* %34, i32 0, i32 2
  %43 = getelementptr inbounds [6 x float], [6 x float]* %35, i32 0, i32 2
  %44 = load float, float* %43
  store float %44, float* %42
  %45 = getelementptr inbounds [6 x float], [6 x float]* %34, i32 0, i32 3
  %46 = getelementptr inbounds [6 x float], [6 x float]* %35, i32 0, i32 3
  %47 = load float, float* %46
  store float %47, float* %45
  %48 = getelementptr inbounds [6 x float], [6 x float]* %34, i32 0, i32 4
  %49 = getelementptr inbounds [6 x float], [6 x float]* %35, i32 0, i32 4
  %50 = load float, float* %49
  store float %50, float* %48
  %51 = getelementptr inbounds [6 x float], [6 x float]* %34, i32 0, i32 5
  %52 = getelementptr inbounds [6 x float], [6 x float]* %35, i32 0, i32 5
  %53 = load float, float* %52
  store float %53, float* %51
  br label %68

; <label>:54:                                     ; preds = %17
  br label %55

; <label>:55:                                     ; preds = %54
  store i32 0, i32* %i, align 4
  br label %56

; <label>:56:                                     ; preds = %64, %55
  %57 = load i32, i32* %i, align 4
  %58 = icmp slt i32 %57, 6
  br i1 %58, label %59, label %67

; <label>:59:                                     ; preds = %56
  call void (...) @_ssdm_op_SpecLoopName(i8* getelementptr inbounds ([25 x i8], [25 x i8]* @.str19, i32 0, i32 0)) #0
  call void (...) @_ssdm_RegionBegin(i8* getelementptr inbounds ([25 x i8], [25 x i8]* @.str19, i32 0, i32 0)) #0
  %60 = load i32, i32* %i, align 4
  %61 = sext i32 %60 to i64
  %62 = getelementptr inbounds %struct.outer_t, %struct.outer_t* %tmp, i32 0, i32 0
  %63 = getelementptr inbounds [6 x float], [6 x float]* %62, i32 0, i64 %61
  store float 0.000000e+00, float* %63, align 4
  call void (...) @_ssdm_RegionEnd(i8* getelementptr inbounds ([25 x i8], [25 x i8]* @.str19, i32 0, i32 0)) #0
  br label %64

; <label>:64:                                     ; preds = %59
  %65 = load i32, i32* %i, align 4
  %66 = add nsw i32 %65, 1
  store i32 %66, i32* %i, align 4
  br label %56

; <label>:67:                                     ; preds = %56
  br label %68

; <label>:68:                                     ; preds = %67, %24
  %69 = bitcast %struct.outer_t* %3 to i8*
  %70 = bitcast %struct.outer_t* %tmp to i8*
  %71 = getelementptr inbounds %struct.outer_t, %struct.outer_t* %3, i32 0, i32 0
  %72 = getelementptr inbounds %struct.outer_t, %struct.outer_t* %tmp, i32 0, i32 0
  %73 = getelementptr inbounds [6 x float], [6 x float]* %71, i32 0, i32 0
  %74 = getelementptr inbounds [6 x float], [6 x float]* %72, i32 0, i32 0
  %75 = load float, float* %74
  store float %75, float* %73
  %76 = getelementptr inbounds [6 x float], [6 x float]* %71, i32 0, i32 1
  %77 = getelementptr inbounds [6 x float], [6 x float]* %72, i32 0, i32 1
  %78 = load float, float* %77
  store float %78, float* %76
  %79 = getelementptr inbounds [6 x float], [6 x float]* %71, i32 0, i32 2
  %80 = getelementptr inbounds [6 x float], [6 x float]* %72, i32 0, i32 2
  %81 = load float, float* %80
  store float %81, float* %79
  %82 = getelementptr inbounds [6 x float], [6 x float]* %71, i32 0, i32 3
  %83 = getelementptr inbounds [6 x float], [6 x float]* %72, i32 0, i32 3
  %84 = load float, float* %83
  store float %84, float* %82
  %85 = getelementptr inbounds [6 x float], [6 x float]* %71, i32 0, i32 4
  %86 = getelementptr inbounds [6 x float], [6 x float]* %72, i32 0, i32 4
  %87 = load float, float* %86
  store float %87, float* %85
  %88 = getelementptr inbounds [6 x float], [6 x float]* %71, i32 0, i32 5
  %89 = getelementptr inbounds [6 x float], [6 x float]* %72, i32 0, i32 5
  %90 = load float, float* %89
  store float %90, float* %88
  %91 = load i32, i32* %c, align 4
  call void @_ZN3hls10LineBufferILi3ELi1024E7outer_tLi0EE17insert_bottom_rowES1_i(%"class.hls::LineBuffer.1"* %buf, %struct.outer_t* byval align 8 %3, i32 %91)
  br label %92

; <label>:92:                                     ; preds = %68
  store i32 0, i32* %k, align 4
  br label %93

; <label>:93:                                     ; preds = %101, %92
  %94 = load i32, i32* %k, align 4
  %95 = icmp slt i32 %94, 6
  br i1 %95, label %96, label %104

; <label>:96:                                     ; preds = %93
  call void (...) @_ssdm_op_SpecLoopName(i8* getelementptr inbounds ([25 x i8], [25 x i8]* @.str20, i32 0, i32 0)) #0
  call void (...) @_ssdm_RegionBegin(i8* getelementptr inbounds ([25 x i8], [25 x i8]* @.str20, i32 0, i32 0)) #0
  %97 = load i32, i32* %k, align 4
  %98 = sext i32 %97 to i64
  %99 = getelementptr inbounds %struct.tensor_t, %struct.tensor_t* %acc, i32 0, i32 0
  %100 = getelementptr inbounds [6 x float], [6 x float]* %99, i32 0, i64 %98
  store float 0.000000e+00, float* %100, align 4
  call void (...) @_ssdm_RegionEnd(i8* getelementptr inbounds ([25 x i8], [25 x i8]* @.str20, i32 0, i32 0)) #0
  br label %101

; <label>:101:                                    ; preds = %96
  %102 = load i32, i32* %k, align 4
  %103 = add nsw i32 %102, 1
  store i32 %103, i32* %k, align 4
  br label %93

; <label>:104:                                    ; preds = %93
  %105 = load i32, i32* %r, align 4
  %106 = icmp sge i32 %105, 2
  br i1 %106, label %107, label %171

; <label>:107:                                    ; preds = %104
  %108 = load i32, i32* %r, align 4
  %109 = icmp slt i32 %108, 436
  br i1 %109, label %110, label %171

; <label>:110:                                    ; preds = %107
  br label %111

; <label>:111:                                    ; preds = %110
  store i32 0, i32* %i1, align 4
  br label %112

; <label>:112:                                    ; preds = %167, %111
  %113 = load i32, i32* %i1, align 4
  %114 = icmp slt i32 %113, 3
  br i1 %114, label %115, label %170

; <label>:115:                                    ; preds = %112
  call void (...) @_ssdm_op_SpecLoopName(i8* getelementptr inbounds ([26 x i8], [26 x i8]* @.str21, i32 0, i32 0)) #0
  call void (...) @_ssdm_RegionBegin(i8* getelementptr inbounds ([26 x i8], [26 x i8]* @.str21, i32 0, i32 0)) #0
  %116 = load i32, i32* %i1, align 4
  %117 = load i32, i32* %c, align 4
  %118 = call %struct.outer_t* @_ZN3hls10LineBufferILi3ELi1024E7outer_tLi0EE6getvalEii(%"class.hls::LineBuffer.1"* %buf, i32 %116, i32 %117)
  %119 = bitcast %struct.outer_t* %tmp to i8*
  %120 = bitcast %struct.outer_t* %118 to i8*
  %121 = getelementptr inbounds %struct.outer_t, %struct.outer_t* %tmp, i32 0, i32 0
  %122 = getelementptr inbounds %struct.outer_t, %struct.outer_t* %118, i32 0, i32 0
  %123 = getelementptr inbounds [6 x float], [6 x float]* %121, i32 0, i32 0
  %124 = getelementptr inbounds [6 x float], [6 x float]* %122, i32 0, i32 0
  %125 = load float, float* %124
  store float %125, float* %123
  %126 = getelementptr inbounds [6 x float], [6 x float]* %121, i32 0, i32 1
  %127 = getelementptr inbounds [6 x float], [6 x float]* %122, i32 0, i32 1
  %128 = load float, float* %127
  store float %128, float* %126
  %129 = getelementptr inbounds [6 x float], [6 x float]* %121, i32 0, i32 2
  %130 = getelementptr inbounds [6 x float], [6 x float]* %122, i32 0, i32 2
  %131 = load float, float* %130
  store float %131, float* %129
  %132 = getelementptr inbounds [6 x float], [6 x float]* %121, i32 0, i32 3
  %133 = getelementptr inbounds [6 x float], [6 x float]* %122, i32 0, i32 3
  %134 = load float, float* %133
  store float %134, float* %132
  %135 = getelementptr inbounds [6 x float], [6 x float]* %121, i32 0, i32 4
  %136 = getelementptr inbounds [6 x float], [6 x float]* %122, i32 0, i32 4
  %137 = load float, float* %136
  store float %137, float* %135
  %138 = getelementptr inbounds [6 x float], [6 x float]* %121, i32 0, i32 5
  %139 = getelementptr inbounds [6 x float], [6 x float]* %122, i32 0, i32 5
  %140 = load float, float* %139
  store float %140, float* %138
  %141 = load i32, i32* %i1, align 4
  %142 = sext i32 %141 to i64
  %143 = getelementptr inbounds [3 x float], [3 x float]* @_ZZ15tensor_weight_yPA1024_7outer_tPA1024_8tensor_tE13TENSOR_FILTER, i32 0, i64 %142
  %144 = load float, float* %143, align 4
  store float %144, float* %k2, align 4
  br label %145

; <label>:145:                                    ; preds = %115
  store i32 0, i32* %component, align 4
  br label %146

; <label>:146:                                    ; preds = %163, %145
  %147 = load i32, i32* %component, align 4
  %148 = icmp slt i32 %147, 6
  br i1 %148, label %149, label %166

; <label>:149:                                    ; preds = %146
  call void (...) @_ssdm_op_SpecLoopName(i8* getelementptr inbounds ([26 x i8], [26 x i8]* @.str22, i32 0, i32 0)) #0
  call void (...) @_ssdm_RegionBegin(i8* getelementptr inbounds ([26 x i8], [26 x i8]* @.str22, i32 0, i32 0)) #0
  %150 = load i32, i32* %component, align 4
  %151 = sext i32 %150 to i64
  %152 = getelementptr inbounds %struct.outer_t, %struct.outer_t* %tmp, i32 0, i32 0
  %153 = getelementptr inbounds [6 x float], [6 x float]* %152, i32 0, i64 %151
  %154 = load float, float* %153, align 4
  %155 = load float, float* %k2, align 4
  %156 = fmul float %154, %155
  %157 = load i32, i32* %component, align 4
  %158 = sext i32 %157 to i64
  %159 = getelementptr inbounds %struct.tensor_t, %struct.tensor_t* %acc, i32 0, i32 0
  %160 = getelementptr inbounds [6 x float], [6 x float]* %159, i32 0, i64 %158
  %161 = load float, float* %160, align 4
  %162 = fadd float %161, %156
  store float %162, float* %160, align 4
  call void (...) @_ssdm_RegionEnd(i8* getelementptr inbounds ([26 x i8], [26 x i8]* @.str22, i32 0, i32 0)) #0
  br label %163

; <label>:163:                                    ; preds = %149
  %164 = load i32, i32* %component, align 4
  %165 = add nsw i32 %164, 1
  store i32 %165, i32* %component, align 4
  br label %146

; <label>:166:                                    ; preds = %146
  call void (...) @_ssdm_RegionEnd(i8* getelementptr inbounds ([26 x i8], [26 x i8]* @.str21, i32 0, i32 0)) #0
  br label %167

; <label>:167:                                    ; preds = %166
  %168 = load i32, i32* %i1, align 4
  %169 = add nsw i32 %168, 1
  store i32 %169, i32* %i1, align 4
  br label %112

; <label>:170:                                    ; preds = %112
  br label %171

; <label>:171:                                    ; preds = %170, %107, %104
  %172 = load i32, i32* %r, align 4
  %173 = icmp sge i32 %172, 1
  br i1 %173, label %174, label %205

; <label>:174:                                    ; preds = %171
  %175 = load i32, i32* %c, align 4
  %176 = sext i32 %175 to i64
  %177 = load i32, i32* %r, align 4
  %178 = sub nsw i32 %177, 1
  %179 = sext i32 %178 to i64
  %180 = load [1024 x %struct.tensor_t]*, [1024 x %struct.tensor_t]** %2, align 8
  %181 = getelementptr inbounds [1024 x %struct.tensor_t], [1024 x %struct.tensor_t]* %180, i64 %179
  %182 = getelementptr inbounds [1024 x %struct.tensor_t], [1024 x %struct.tensor_t]* %181, i32 0, i64 %176
  %183 = bitcast %struct.tensor_t* %182 to i8*
  %184 = bitcast %struct.tensor_t* %acc to i8*
  %185 = getelementptr inbounds %struct.tensor_t, %struct.tensor_t* %182, i32 0, i32 0
  %186 = getelementptr inbounds %struct.tensor_t, %struct.tensor_t* %acc, i32 0, i32 0
  %187 = getelementptr inbounds [6 x float], [6 x float]* %185, i32 0, i32 0
  %188 = getelementptr inbounds [6 x float], [6 x float]* %186, i32 0, i32 0
  %189 = load float, float* %188
  store float %189, float* %187
  %190 = getelementptr inbounds [6 x float], [6 x float]* %185, i32 0, i32 1
  %191 = getelementptr inbounds [6 x float], [6 x float]* %186, i32 0, i32 1
  %192 = load float, float* %191
  store float %192, float* %190
  %193 = getelementptr inbounds [6 x float], [6 x float]* %185, i32 0, i32 2
  %194 = getelementptr inbounds [6 x float], [6 x float]* %186, i32 0, i32 2
  %195 = load float, float* %194
  store float %195, float* %193
  %196 = getelementptr inbounds [6 x float], [6 x float]* %185, i32 0, i32 3
  %197 = getelementptr inbounds [6 x float], [6 x float]* %186, i32 0, i32 3
  %198 = load float, float* %197
  store float %198, float* %196
  %199 = getelementptr inbounds [6 x float], [6 x float]* %185, i32 0, i32 4
  %200 = getelementptr inbounds [6 x float], [6 x float]* %186, i32 0, i32 4
  %201 = load float, float* %200
  store float %201, float* %199
  %202 = getelementptr inbounds [6 x float], [6 x float]* %185, i32 0, i32 5
  %203 = getelementptr inbounds [6 x float], [6 x float]* %186, i32 0, i32 5
  %204 = load float, float* %203
  store float %204, float* %202
  br label %205

; <label>:205:                                    ; preds = %174, %171
  call void (...) @_ssdm_RegionEnd(i8* getelementptr inbounds ([22 x i8], [22 x i8]* @.str18, i32 0, i32 0)) #0
  br label %206

; <label>:206:                                    ; preds = %205
  %207 = load i32, i32* %c, align 4
  %208 = add nsw i32 %207, 1
  store i32 %208, i32* %c, align 4
  br label %14

; <label>:209:                                    ; preds = %14
  call void (...) @_ssdm_RegionEnd(i8* getelementptr inbounds ([22 x i8], [22 x i8]* @.str17, i32 0, i32 0)) #0
  br label %210

; <label>:210:                                    ; preds = %209
  %211 = load i32, i32* %r, align 4
  %212 = add nsw i32 %211, 1
  store i32 %212, i32* %r, align 4
  br label %9

; <label>:213:                                    ; preds = %9
  ret void
}

; Function Attrs: nounwind uwtable
define linkonce_odr void @_ZN3hls10LineBufferILi3ELi1024E7outer_tLi0EEC1Ev(%"class.hls::LineBuffer.1"* %this) unnamed_addr #2 comdat align 2 {
  %1 = alloca %"class.hls::LineBuffer.1"*, align 8
  store %"class.hls::LineBuffer.1"* %this, %"class.hls::LineBuffer.1"** %1, align 8
  %2 = load %"class.hls::LineBuffer.1"*, %"class.hls::LineBuffer.1"** %1
  call void @_ZN3hls10LineBufferILi3ELi1024E7outer_tLi0EEC2Ev(%"class.hls::LineBuffer.1"* %2)
  ret void
}

; Function Attrs: nounwind
declare void @_ssdm_DataPack(...) #0

; Function Attrs: nounwind uwtable
define linkonce_odr void @_ZN3hls10LineBufferILi3ELi1024E7outer_tLi0EE15shift_pixels_upEi(%"class.hls::LineBuffer.1"* %this, i32 %col) #2 comdat align 2 {
  %1 = alloca %"class.hls::LineBuffer.1"*, align 8
  %2 = alloca i32, align 4
  %i = alloca %struct.ap_uint.15, align 4
  %3 = alloca %struct.ap_uint.15, align 4
  %4 = alloca %struct.ap_int_base.18, align 8
  %5 = alloca %struct.ap_int_base.16, align 4
  store %"class.hls::LineBuffer.1"* %this, %"class.hls::LineBuffer.1"** %1, align 8
  store i32 %col, i32* %2, align 4
  %6 = load %"class.hls::LineBuffer.1"*, %"class.hls::LineBuffer.1"** %1
  call void (...) @_ssdm_InlineSelf(i32 0, i8* getelementptr inbounds ([1 x i8], [1 x i8]* @.str2, i32 0, i32 0)) #0
  %7 = load i32, i32* %2, align 4
  %8 = icmp sge i32 %7, 0
  br i1 %8, label %9, label %12

; <label>:9:                                      ; preds = %0
  %10 = load i32, i32* %2, align 4
  %11 = icmp slt i32 %10, 1024
  br label %12

; <label>:12:                                     ; preds = %9, %0
  %13 = phi i1 [ false, %0 ], [ %11, %9 ]
  br i1 %13, label %14, label %15

; <label>:14:                                     ; preds = %12
  br label %17

; <label>:15:                                     ; preds = %12
  call void @__assert_fail(i8* getelementptr inbounds ([23 x i8], [23 x i8]* @.str37, i32 0, i32 0), i8* getelementptr inbounds ([81 x i8], [81 x i8]* @.str34, i32 0, i32 0), i32 718, i8* getelementptr inbounds ([114 x i8], [114 x i8]* @__PRETTY_FUNCTION__._ZN3hls10LineBufferILi3ELi1024E7outer_tLi0EE15shift_pixels_upEi, i32 0, i32 0)) #5
  unreachable
                                                  ; No predecessors!
  br label %17

; <label>:17:                                     ; preds = %16, %14
  call void @_ZN7ap_uintILi32EEC1Ev(%struct.ap_uint.15* %i)
  call void @_ZN7ap_uintILi32EEC1Ei(%struct.ap_uint.15* %3, i32 0)
  %18 = call %struct.ap_uint.15* @_ZN7ap_uintILi32EEaSERKS0_(%struct.ap_uint.15* %i, %struct.ap_uint.15* %3)
  br label %19

; <label>:19:                                     ; preds = %60, %17
  %20 = bitcast %struct.ap_uint.15* %i to %struct.ap_int_base.16*
  %21 = call zeroext i1 @_ZltILi32ELb0EEbRK11ap_int_baseIXT_EXT0_EXleT_Li64EEEi(%struct.ap_int_base.16* %20, i32 2)
  br i1 %21, label %22, label %62

; <label>:22:                                     ; preds = %19
  call void (...) @_ssdm_RegionBegin(i8* getelementptr inbounds ([12 x i8], [12 x i8]* @.str38, i32 0, i32 0)) #0
  call void (...) @_ssdm_Unroll(i32 0, i32 0, i32 0, i8* getelementptr inbounds ([1 x i8], [1 x i8]* @.str2, i32 0, i32 0)) #0
  %23 = load i32, i32* %2, align 4
  %24 = sext i32 %23 to i64
  %25 = bitcast %struct.ap_uint.15* %i to %struct.ap_int_base.16*
  %26 = call i32 @_ZNK11ap_int_baseILi32ELb0ELb1EEcvjEv(%struct.ap_int_base.16* %25)
  %27 = zext i32 %26 to i64
  %28 = getelementptr inbounds %"class.hls::LineBuffer.1", %"class.hls::LineBuffer.1"* %6, i32 0, i32 0
  %29 = getelementptr inbounds [3 x [1024 x %struct.outer_t]], [3 x [1024 x %struct.outer_t]]* %28, i32 0, i64 %27
  %30 = getelementptr inbounds [1024 x %struct.outer_t], [1024 x %struct.outer_t]* %29, i32 0, i64 %24
  %31 = load i32, i32* %2, align 4
  %32 = sext i32 %31 to i64
  %33 = bitcast %struct.ap_uint.15* %i to %struct.ap_int_base.16*
  call void @_ZplILi32ELb0EEN11ap_int_baseIXT_EXT0_EXleT_Li64EEE5RTypeIXLi32EEXLb1EEE4plusERKS1_i(%struct.ap_int_base.18* sret %4, %struct.ap_int_base.16* %33, i32 1)
  %34 = call i64 @_ZNK11ap_int_baseILi34ELb1ELb1EEcvxEv(%struct.ap_int_base.18* %4)
  %35 = getelementptr inbounds %"class.hls::LineBuffer.1", %"class.hls::LineBuffer.1"* %6, i32 0, i32 0
  %36 = getelementptr inbounds [3 x [1024 x %struct.outer_t]], [3 x [1024 x %struct.outer_t]]* %35, i32 0, i64 %34
  %37 = getelementptr inbounds [1024 x %struct.outer_t], [1024 x %struct.outer_t]* %36, i32 0, i64 %32
  %38 = bitcast %struct.outer_t* %30 to i8*
  %39 = bitcast %struct.outer_t* %37 to i8*
  %40 = getelementptr inbounds %struct.outer_t, %struct.outer_t* %30, i32 0, i32 0
  %41 = getelementptr inbounds %struct.outer_t, %struct.outer_t* %37, i32 0, i32 0
  %42 = getelementptr inbounds [6 x float], [6 x float]* %40, i32 0, i32 0
  %43 = getelementptr inbounds [6 x float], [6 x float]* %41, i32 0, i32 0
  %44 = load float, float* %43
  store float %44, float* %42
  %45 = getelementptr inbounds [6 x float], [6 x float]* %40, i32 0, i32 1
  %46 = getelementptr inbounds [6 x float], [6 x float]* %41, i32 0, i32 1
  %47 = load float, float* %46
  store float %47, float* %45
  %48 = getelementptr inbounds [6 x float], [6 x float]* %40, i32 0, i32 2
  %49 = getelementptr inbounds [6 x float], [6 x float]* %41, i32 0, i32 2
  %50 = load float, float* %49
  store float %50, float* %48
  %51 = getelementptr inbounds [6 x float], [6 x float]* %40, i32 0, i32 3
  %52 = getelementptr inbounds [6 x float], [6 x float]* %41, i32 0, i32 3
  %53 = load float, float* %52
  store float %53, float* %51
  %54 = getelementptr inbounds [6 x float], [6 x float]* %40, i32 0, i32 4
  %55 = getelementptr inbounds [6 x float], [6 x float]* %41, i32 0, i32 4
  %56 = load float, float* %55
  store float %56, float* %54
  %57 = getelementptr inbounds [6 x float], [6 x float]* %40, i32 0, i32 5
  %58 = getelementptr inbounds [6 x float], [6 x float]* %41, i32 0, i32 5
  %59 = load float, float* %58
  store float %59, float* %57
  call void (...) @_ssdm_RegionEnd(i8* getelementptr inbounds ([12 x i8], [12 x i8]* @.str38, i32 0, i32 0)) #0
  br label %60

; <label>:60:                                     ; preds = %22
  %61 = bitcast %struct.ap_uint.15* %i to %struct.ap_int_base.16*
  call void @_ZN11ap_int_baseILi32ELb0ELb1EEppEi(%struct.ap_int_base.16* sret %5, %struct.ap_int_base.16* %61, i32 0)
  br label %19

; <label>:62:                                     ; preds = %19
  ret void
}

; Function Attrs: nounwind uwtable
define linkonce_odr void @_ZN3hls10LineBufferILi3ELi1024E7outer_tLi0EE17insert_bottom_rowES1_i(%"class.hls::LineBuffer.1"* %this, %struct.outer_t* byval align 8 %value, i32 %col) #2 comdat align 2 {
  %1 = alloca %"class.hls::LineBuffer.1"*, align 8
  %2 = alloca i32, align 4
  store %"class.hls::LineBuffer.1"* %this, %"class.hls::LineBuffer.1"** %1, align 8
  store i32 %col, i32* %2, align 4
  %3 = load %"class.hls::LineBuffer.1"*, %"class.hls::LineBuffer.1"** %1
  call void (...) @_ssdm_InlineSelf(i32 0, i8* getelementptr inbounds ([1 x i8], [1 x i8]* @.str2, i32 0, i32 0)) #0
  %4 = load i32, i32* %2, align 4
  %5 = icmp sge i32 %4, 0
  br i1 %5, label %6, label %9

; <label>:6:                                      ; preds = %0
  %7 = load i32, i32* %2, align 4
  %8 = icmp slt i32 %7, 1024
  br label %9

; <label>:9:                                      ; preds = %6, %0
  %10 = phi i1 [ false, %0 ], [ %8, %6 ]
  br i1 %10, label %11, label %12

; <label>:11:                                     ; preds = %9
  br label %14

; <label>:12:                                     ; preds = %9
  call void @__assert_fail(i8* getelementptr inbounds ([23 x i8], [23 x i8]* @.str37, i32 0, i32 0), i8* getelementptr inbounds ([81 x i8], [81 x i8]* @.str34, i32 0, i32 0), i32 757, i8* getelementptr inbounds ([119 x i8], [119 x i8]* @__PRETTY_FUNCTION__._ZN3hls10LineBufferILi3ELi1024E7outer_tLi0EE17insert_bottom_rowES1_i, i32 0, i32 0)) #5
  unreachable
                                                  ; No predecessors!
  br label %14

; <label>:14:                                     ; preds = %13, %11
  %15 = load i32, i32* %2, align 4
  %16 = sext i32 %15 to i64
  %17 = getelementptr inbounds %"class.hls::LineBuffer.1", %"class.hls::LineBuffer.1"* %3, i32 0, i32 0
  %18 = getelementptr inbounds [3 x [1024 x %struct.outer_t]], [3 x [1024 x %struct.outer_t]]* %17, i32 0, i64 2
  %19 = getelementptr inbounds [1024 x %struct.outer_t], [1024 x %struct.outer_t]* %18, i32 0, i64 %16
  %20 = bitcast %struct.outer_t* %19 to i8*
  %21 = bitcast %struct.outer_t* %value to i8*
  %22 = getelementptr inbounds %struct.outer_t, %struct.outer_t* %19, i32 0, i32 0
  %23 = getelementptr inbounds %struct.outer_t, %struct.outer_t* %value, i32 0, i32 0
  %24 = getelementptr inbounds [6 x float], [6 x float]* %22, i32 0, i32 0
  %25 = getelementptr inbounds [6 x float], [6 x float]* %23, i32 0, i32 0
  %26 = load float, float* %25
  store float %26, float* %24
  %27 = getelementptr inbounds [6 x float], [6 x float]* %22, i32 0, i32 1
  %28 = getelementptr inbounds [6 x float], [6 x float]* %23, i32 0, i32 1
  %29 = load float, float* %28
  store float %29, float* %27
  %30 = getelementptr inbounds [6 x float], [6 x float]* %22, i32 0, i32 2
  %31 = getelementptr inbounds [6 x float], [6 x float]* %23, i32 0, i32 2
  %32 = load float, float* %31
  store float %32, float* %30
  %33 = getelementptr inbounds [6 x float], [6 x float]* %22, i32 0, i32 3
  %34 = getelementptr inbounds [6 x float], [6 x float]* %23, i32 0, i32 3
  %35 = load float, float* %34
  store float %35, float* %33
  %36 = getelementptr inbounds [6 x float], [6 x float]* %22, i32 0, i32 4
  %37 = getelementptr inbounds [6 x float], [6 x float]* %23, i32 0, i32 4
  %38 = load float, float* %37
  store float %38, float* %36
  %39 = getelementptr inbounds [6 x float], [6 x float]* %22, i32 0, i32 5
  %40 = getelementptr inbounds [6 x float], [6 x float]* %23, i32 0, i32 5
  %41 = load float, float* %40
  store float %41, float* %39
  ret void
}

; Function Attrs: nounwind uwtable
define linkonce_odr %struct.outer_t* @_ZN3hls10LineBufferILi3ELi1024E7outer_tLi0EE6getvalEii(%"class.hls::LineBuffer.1"* %this, i32 %row, i32 %col) #2 comdat align 2 {
  %1 = alloca %"class.hls::LineBuffer.1"*, align 8
  %2 = alloca i32, align 4
  %3 = alloca i32, align 4
  store %"class.hls::LineBuffer.1"* %this, %"class.hls::LineBuffer.1"** %1, align 8
  store i32 %row, i32* %2, align 4
  store i32 %col, i32* %3, align 4
  %4 = load %"class.hls::LineBuffer.1"*, %"class.hls::LineBuffer.1"** %1
  call void (...) @_ssdm_InlineSelf(i32 0, i8* getelementptr inbounds ([1 x i8], [1 x i8]* @.str2, i32 0, i32 0)) #0
  %5 = load i32, i32* %2, align 4
  %6 = icmp sge i32 %5, 0
  br i1 %6, label %7, label %16

; <label>:7:                                      ; preds = %0
  %8 = load i32, i32* %2, align 4
  %9 = icmp slt i32 %8, 3
  br i1 %9, label %10, label %16

; <label>:10:                                     ; preds = %7
  %11 = load i32, i32* %3, align 4
  %12 = icmp sge i32 %11, 0
  br i1 %12, label %13, label %16

; <label>:13:                                     ; preds = %10
  %14 = load i32, i32* %3, align 4
  %15 = icmp slt i32 %14, 1024
  br label %16

; <label>:16:                                     ; preds = %13, %10, %7, %0
  %17 = phi i1 [ false, %10 ], [ false, %7 ], [ false, %0 ], [ %15, %13 ]
  br i1 %17, label %18, label %19

; <label>:18:                                     ; preds = %16
  br label %21

; <label>:19:                                     ; preds = %16
  call void @__assert_fail(i8* getelementptr inbounds ([49 x i8], [49 x i8]* @.str33, i32 0, i32 0), i8* getelementptr inbounds ([81 x i8], [81 x i8]* @.str34, i32 0, i32 0), i32 834, i8* getelementptr inbounds ([108 x i8], [108 x i8]* @__PRETTY_FUNCTION__._ZN3hls10LineBufferILi3ELi1024E7outer_tLi0EE6getvalEii, i32 0, i32 0)) #5
  unreachable
                                                  ; No predecessors!
  br label %21

; <label>:21:                                     ; preds = %20, %18
  %22 = load i32, i32* %3, align 4
  %23 = sext i32 %22 to i64
  %24 = load i32, i32* %2, align 4
  %25 = sext i32 %24 to i64
  %26 = getelementptr inbounds %"class.hls::LineBuffer.1", %"class.hls::LineBuffer.1"* %4, i32 0, i32 0
  %27 = getelementptr inbounds [3 x [1024 x %struct.outer_t]], [3 x [1024 x %struct.outer_t]]* %26, i32 0, i64 %25
  %28 = getelementptr inbounds [1024 x %struct.outer_t], [1024 x %struct.outer_t]* %27, i32 0, i64 %23
  ret %struct.outer_t* %28
}

; Function Attrs: nounwind uwtable
define void @_Z15tensor_weight_xPA1024_8tensor_tS1_([1024 x %struct.tensor_t]* %tensor_y, [1024 x %struct.tensor_t]* %tensor) #2 {
  %1 = alloca [1024 x %struct.tensor_t]*, align 8
  %2 = alloca [1024 x %struct.tensor_t]*, align 8
  %buf = alloca %"class.hls::Window.2", align 4
  %r = alloca i32, align 4
  %c = alloca i32, align 4
  %tmp = alloca %struct.tensor_t, align 4
  %i = alloca i32, align 4
  %3 = alloca %struct.tensor_t, align 4
  %acc = alloca %struct.tensor_t, align 4
  %k = alloca i32, align 4
  %i1 = alloca i32, align 4
  %component = alloca i32, align 4
  store [1024 x %struct.tensor_t]* %tensor_y, [1024 x %struct.tensor_t]** %1, align 8
  store [1024 x %struct.tensor_t]* %tensor, [1024 x %struct.tensor_t]** %2, align 8
  %4 = load [1024 x %struct.tensor_t]*, [1024 x %struct.tensor_t]** %1, align 8
  call void (...) @_ssdm_SpecArrayDimSize([1024 x %struct.tensor_t]* %4, i32 436) #0
  %5 = load [1024 x %struct.tensor_t]*, [1024 x %struct.tensor_t]** %2, align 8
  call void (...) @_ssdm_SpecArrayDimSize([1024 x %struct.tensor_t]* %5, i32 436) #0
  %6 = load [1024 x %struct.tensor_t]*, [1024 x %struct.tensor_t]** %1, align 8
  call void (...) @_ssdm_op_SpecInterface([1024 x %struct.tensor_t]* %6, i8* getelementptr inbounds ([8 x i8], [8 x i8]* @.str, i32 0, i32 0), i32 0, i32 0, i8* getelementptr inbounds ([1 x i8], [1 x i8]* @.str2, i32 0, i32 0), i32 0, i32 0, i8* getelementptr inbounds ([1 x i8], [1 x i8]* @.str2, i32 0, i32 0), i8* getelementptr inbounds ([1 x i8], [1 x i8]* @.str2, i32 0, i32 0), i8* getelementptr inbounds ([1 x i8], [1 x i8]* @.str2, i32 0, i32 0), i32 0, i32 0, i32 0, i32 0, i8* getelementptr inbounds ([1 x i8], [1 x i8]* @.str2, i32 0, i32 0), i8* getelementptr inbounds ([1 x i8], [1 x i8]* @.str2, i32 0, i32 0)) #0
  %7 = load [1024 x %struct.tensor_t]*, [1024 x %struct.tensor_t]** %2, align 8
  call void (...) @_ssdm_op_SpecInterface([1024 x %struct.tensor_t]* %7, i8* getelementptr inbounds ([8 x i8], [8 x i8]* @.str, i32 0, i32 0), i32 0, i32 0, i8* getelementptr inbounds ([1 x i8], [1 x i8]* @.str2, i32 0, i32 0), i32 0, i32 0, i8* getelementptr inbounds ([1 x i8], [1 x i8]* @.str2, i32 0, i32 0), i8* getelementptr inbounds ([1 x i8], [1 x i8]* @.str2, i32 0, i32 0), i8* getelementptr inbounds ([1 x i8], [1 x i8]* @.str2, i32 0, i32 0), i32 0, i32 0, i32 0, i32 0, i8* getelementptr inbounds ([1 x i8], [1 x i8]* @.str2, i32 0, i32 0), i8* getelementptr inbounds ([1 x i8], [1 x i8]* @.str2, i32 0, i32 0)) #0
  call void @_ZN3hls6WindowILi1ELi3E8tensor_tEC1Ev(%"class.hls::Window.2"* %buf)
  call void (...) @_ssdm_SpecConstant(float* getelementptr inbounds ([3 x float], [3 x float]* @_ZZ15tensor_weight_xPA1024_8tensor_tS1_E13TENSOR_FILTER, i32 0, i32 0)) #0
  br label %8

; <label>:8:                                      ; preds = %0
  store i32 0, i32* %r, align 4
  br label %9

; <label>:9:                                      ; preds = %203, %8
  %10 = load i32, i32* %r, align 4
  %11 = icmp slt i32 %10, 436
  br i1 %11, label %12, label %206

; <label>:12:                                     ; preds = %9
  call void (...) @_ssdm_op_SpecLoopName(i8* getelementptr inbounds ([22 x i8], [22 x i8]* @.str23, i32 0, i32 0)) #0
  call void (...) @_ssdm_RegionBegin(i8* getelementptr inbounds ([22 x i8], [22 x i8]* @.str23, i32 0, i32 0)) #0
  br label %13

; <label>:13:                                     ; preds = %12
  store i32 0, i32* %c, align 4
  br label %14

; <label>:14:                                     ; preds = %199, %13
  %15 = load i32, i32* %c, align 4
  %16 = icmp slt i32 %15, 1025
  br i1 %16, label %17, label %202

; <label>:17:                                     ; preds = %14
  call void (...) @_ssdm_op_SpecLoopName(i8* getelementptr inbounds ([22 x i8], [22 x i8]* @.str24, i32 0, i32 0)) #0
  call void (...) @_ssdm_RegionBegin(i8* getelementptr inbounds ([22 x i8], [22 x i8]* @.str24, i32 0, i32 0)) #0
  call void (...) @_ssdm_op_SpecPipeline(i32 1, i32 1, i32 1, i32 0, i8* getelementptr inbounds ([1 x i8], [1 x i8]* @.str2, i32 0, i32 0)) #0
  call void @_ZN3hls6WindowILi1ELi3E8tensor_tE17shift_pixels_leftEv(%"class.hls::Window.2"* %buf)
  %18 = load i32, i32* %c, align 4
  %19 = icmp slt i32 %18, 1024
  br i1 %19, label %20, label %50

; <label>:20:                                     ; preds = %17
  %21 = load i32, i32* %c, align 4
  %22 = sext i32 %21 to i64
  %23 = load i32, i32* %r, align 4
  %24 = sext i32 %23 to i64
  %25 = load [1024 x %struct.tensor_t]*, [1024 x %struct.tensor_t]** %1, align 8
  %26 = getelementptr inbounds [1024 x %struct.tensor_t], [1024 x %struct.tensor_t]* %25, i64 %24
  %27 = getelementptr inbounds [1024 x %struct.tensor_t], [1024 x %struct.tensor_t]* %26, i32 0, i64 %22
  %28 = bitcast %struct.tensor_t* %tmp to i8*
  %29 = bitcast %struct.tensor_t* %27 to i8*
  %30 = getelementptr inbounds %struct.tensor_t, %struct.tensor_t* %tmp, i32 0, i32 0
  %31 = getelementptr inbounds %struct.tensor_t, %struct.tensor_t* %27, i32 0, i32 0
  %32 = getelementptr inbounds [6 x float], [6 x float]* %30, i32 0, i32 0
  %33 = getelementptr inbounds [6 x float], [6 x float]* %31, i32 0, i32 0
  %34 = load float, float* %33
  store float %34, float* %32
  %35 = getelementptr inbounds [6 x float], [6 x float]* %30, i32 0, i32 1
  %36 = getelementptr inbounds [6 x float], [6 x float]* %31, i32 0, i32 1
  %37 = load float, float* %36
  store float %37, float* %35
  %38 = getelementptr inbounds [6 x float], [6 x float]* %30, i32 0, i32 2
  %39 = getelementptr inbounds [6 x float], [6 x float]* %31, i32 0, i32 2
  %40 = load float, float* %39
  store float %40, float* %38
  %41 = getelementptr inbounds [6 x float], [6 x float]* %30, i32 0, i32 3
  %42 = getelementptr inbounds [6 x float], [6 x float]* %31, i32 0, i32 3
  %43 = load float, float* %42
  store float %43, float* %41
  %44 = getelementptr inbounds [6 x float], [6 x float]* %30, i32 0, i32 4
  %45 = getelementptr inbounds [6 x float], [6 x float]* %31, i32 0, i32 4
  %46 = load float, float* %45
  store float %46, float* %44
  %47 = getelementptr inbounds [6 x float], [6 x float]* %30, i32 0, i32 5
  %48 = getelementptr inbounds [6 x float], [6 x float]* %31, i32 0, i32 5
  %49 = load float, float* %48
  store float %49, float* %47
  br label %64

; <label>:50:                                     ; preds = %17
  br label %51

; <label>:51:                                     ; preds = %50
  store i32 0, i32* %i, align 4
  br label %52

; <label>:52:                                     ; preds = %60, %51
  %53 = load i32, i32* %i, align 4
  %54 = icmp slt i32 %53, 6
  br i1 %54, label %55, label %63

; <label>:55:                                     ; preds = %52
  call void (...) @_ssdm_op_SpecLoopName(i8* getelementptr inbounds ([25 x i8], [25 x i8]* @.str25, i32 0, i32 0)) #0
  call void (...) @_ssdm_RegionBegin(i8* getelementptr inbounds ([25 x i8], [25 x i8]* @.str25, i32 0, i32 0)) #0
  %56 = load i32, i32* %i, align 4
  %57 = sext i32 %56 to i64
  %58 = getelementptr inbounds %struct.tensor_t, %struct.tensor_t* %tmp, i32 0, i32 0
  %59 = getelementptr inbounds [6 x float], [6 x float]* %58, i32 0, i64 %57
  store float 0.000000e+00, float* %59, align 4
  call void (...) @_ssdm_RegionEnd(i8* getelementptr inbounds ([25 x i8], [25 x i8]* @.str25, i32 0, i32 0)) #0
  br label %60

; <label>:60:                                     ; preds = %55
  %61 = load i32, i32* %i, align 4
  %62 = add nsw i32 %61, 1
  store i32 %62, i32* %i, align 4
  br label %52

; <label>:63:                                     ; preds = %52
  br label %64

; <label>:64:                                     ; preds = %63, %20
  %65 = bitcast %struct.tensor_t* %3 to i8*
  %66 = bitcast %struct.tensor_t* %tmp to i8*
  %67 = getelementptr inbounds %struct.tensor_t, %struct.tensor_t* %3, i32 0, i32 0
  %68 = getelementptr inbounds %struct.tensor_t, %struct.tensor_t* %tmp, i32 0, i32 0
  %69 = getelementptr inbounds [6 x float], [6 x float]* %67, i32 0, i32 0
  %70 = getelementptr inbounds [6 x float], [6 x float]* %68, i32 0, i32 0
  %71 = load float, float* %70
  store float %71, float* %69
  %72 = getelementptr inbounds [6 x float], [6 x float]* %67, i32 0, i32 1
  %73 = getelementptr inbounds [6 x float], [6 x float]* %68, i32 0, i32 1
  %74 = load float, float* %73
  store float %74, float* %72
  %75 = getelementptr inbounds [6 x float], [6 x float]* %67, i32 0, i32 2
  %76 = getelementptr inbounds [6 x float], [6 x float]* %68, i32 0, i32 2
  %77 = load float, float* %76
  store float %77, float* %75
  %78 = getelementptr inbounds [6 x float], [6 x float]* %67, i32 0, i32 3
  %79 = getelementptr inbounds [6 x float], [6 x float]* %68, i32 0, i32 3
  %80 = load float, float* %79
  store float %80, float* %78
  %81 = getelementptr inbounds [6 x float], [6 x float]* %67, i32 0, i32 4
  %82 = getelementptr inbounds [6 x float], [6 x float]* %68, i32 0, i32 4
  %83 = load float, float* %82
  store float %83, float* %81
  %84 = getelementptr inbounds [6 x float], [6 x float]* %67, i32 0, i32 5
  %85 = getelementptr inbounds [6 x float], [6 x float]* %68, i32 0, i32 5
  %86 = load float, float* %85
  store float %86, float* %84
  call void @_ZN3hls6WindowILi1ELi3E8tensor_tE12insert_pixelES1_ii(%"class.hls::Window.2"* %buf, %struct.tensor_t* byval align 8 %3, i32 0, i32 2)
  br label %87

; <label>:87:                                     ; preds = %64
  store i32 0, i32* %k, align 4
  br label %88

; <label>:88:                                     ; preds = %96, %87
  %89 = load i32, i32* %k, align 4
  %90 = icmp slt i32 %89, 6
  br i1 %90, label %91, label %99

; <label>:91:                                     ; preds = %88
  call void (...) @_ssdm_op_SpecLoopName(i8* getelementptr inbounds ([25 x i8], [25 x i8]* @.str26, i32 0, i32 0)) #0
  call void (...) @_ssdm_RegionBegin(i8* getelementptr inbounds ([25 x i8], [25 x i8]* @.str26, i32 0, i32 0)) #0
  %92 = load i32, i32* %k, align 4
  %93 = sext i32 %92 to i64
  %94 = getelementptr inbounds %struct.tensor_t, %struct.tensor_t* %acc, i32 0, i32 0
  %95 = getelementptr inbounds [6 x float], [6 x float]* %94, i32 0, i64 %93
  store float 0.000000e+00, float* %95, align 4
  call void (...) @_ssdm_RegionEnd(i8* getelementptr inbounds ([25 x i8], [25 x i8]* @.str26, i32 0, i32 0)) #0
  br label %96

; <label>:96:                                     ; preds = %91
  %97 = load i32, i32* %k, align 4
  %98 = add nsw i32 %97, 1
  store i32 %98, i32* %k, align 4
  br label %88

; <label>:99:                                     ; preds = %88
  %100 = load i32, i32* %c, align 4
  %101 = icmp sge i32 %100, 2
  br i1 %101, label %102, label %164

; <label>:102:                                    ; preds = %99
  %103 = load i32, i32* %c, align 4
  %104 = icmp slt i32 %103, 1024
  br i1 %104, label %105, label %164

; <label>:105:                                    ; preds = %102
  br label %106

; <label>:106:                                    ; preds = %105
  store i32 0, i32* %i1, align 4
  br label %107

; <label>:107:                                    ; preds = %160, %106
  %108 = load i32, i32* %i1, align 4
  %109 = icmp slt i32 %108, 3
  br i1 %109, label %110, label %163

; <label>:110:                                    ; preds = %107
  call void (...) @_ssdm_op_SpecLoopName(i8* getelementptr inbounds ([26 x i8], [26 x i8]* @.str27, i32 0, i32 0)) #0
  call void (...) @_ssdm_RegionBegin(i8* getelementptr inbounds ([26 x i8], [26 x i8]* @.str27, i32 0, i32 0)) #0
  %111 = load i32, i32* %i1, align 4
  %112 = call %struct.tensor_t* @_ZN3hls6WindowILi1ELi3E8tensor_tE6getvalEii(%"class.hls::Window.2"* %buf, i32 0, i32 %111)
  %113 = bitcast %struct.tensor_t* %tmp to i8*
  %114 = bitcast %struct.tensor_t* %112 to i8*
  %115 = getelementptr inbounds %struct.tensor_t, %struct.tensor_t* %tmp, i32 0, i32 0
  %116 = getelementptr inbounds %struct.tensor_t, %struct.tensor_t* %112, i32 0, i32 0
  %117 = getelementptr inbounds [6 x float], [6 x float]* %115, i32 0, i32 0
  %118 = getelementptr inbounds [6 x float], [6 x float]* %116, i32 0, i32 0
  %119 = load float, float* %118
  store float %119, float* %117
  %120 = getelementptr inbounds [6 x float], [6 x float]* %115, i32 0, i32 1
  %121 = getelementptr inbounds [6 x float], [6 x float]* %116, i32 0, i32 1
  %122 = load float, float* %121
  store float %122, float* %120
  %123 = getelementptr inbounds [6 x float], [6 x float]* %115, i32 0, i32 2
  %124 = getelementptr inbounds [6 x float], [6 x float]* %116, i32 0, i32 2
  %125 = load float, float* %124
  store float %125, float* %123
  %126 = getelementptr inbounds [6 x float], [6 x float]* %115, i32 0, i32 3
  %127 = getelementptr inbounds [6 x float], [6 x float]* %116, i32 0, i32 3
  %128 = load float, float* %127
  store float %128, float* %126
  %129 = getelementptr inbounds [6 x float], [6 x float]* %115, i32 0, i32 4
  %130 = getelementptr inbounds [6 x float], [6 x float]* %116, i32 0, i32 4
  %131 = load float, float* %130
  store float %131, float* %129
  %132 = getelementptr inbounds [6 x float], [6 x float]* %115, i32 0, i32 5
  %133 = getelementptr inbounds [6 x float], [6 x float]* %116, i32 0, i32 5
  %134 = load float, float* %133
  store float %134, float* %132
  br label %135

; <label>:135:                                    ; preds = %110
  store i32 0, i32* %component, align 4
  br label %136

; <label>:136:                                    ; preds = %156, %135
  %137 = load i32, i32* %component, align 4
  %138 = icmp slt i32 %137, 6
  br i1 %138, label %139, label %159

; <label>:139:                                    ; preds = %136
  call void (...) @_ssdm_op_SpecLoopName(i8* getelementptr inbounds ([26 x i8], [26 x i8]* @.str28, i32 0, i32 0)) #0
  call void (...) @_ssdm_RegionBegin(i8* getelementptr inbounds ([26 x i8], [26 x i8]* @.str28, i32 0, i32 0)) #0
  %140 = load i32, i32* %component, align 4
  %141 = sext i32 %140 to i64
  %142 = getelementptr inbounds %struct.tensor_t, %struct.tensor_t* %tmp, i32 0, i32 0
  %143 = getelementptr inbounds [6 x float], [6 x float]* %142, i32 0, i64 %141
  %144 = load float, float* %143, align 4
  %145 = load i32, i32* %i1, align 4
  %146 = sext i32 %145 to i64
  %147 = getelementptr inbounds [3 x float], [3 x float]* @_ZZ15tensor_weight_xPA1024_8tensor_tS1_E13TENSOR_FILTER, i32 0, i64 %146
  %148 = load float, float* %147, align 4
  %149 = fmul float %144, %148
  %150 = load i32, i32* %component, align 4
  %151 = sext i32 %150 to i64
  %152 = getelementptr inbounds %struct.tensor_t, %struct.tensor_t* %acc, i32 0, i32 0
  %153 = getelementptr inbounds [6 x float], [6 x float]* %152, i32 0, i64 %151
  %154 = load float, float* %153, align 4
  %155 = fadd float %154, %149
  store float %155, float* %153, align 4
  call void (...) @_ssdm_RegionEnd(i8* getelementptr inbounds ([26 x i8], [26 x i8]* @.str28, i32 0, i32 0)) #0
  br label %156

; <label>:156:                                    ; preds = %139
  %157 = load i32, i32* %component, align 4
  %158 = add nsw i32 %157, 1
  store i32 %158, i32* %component, align 4
  br label %136

; <label>:159:                                    ; preds = %136
  call void (...) @_ssdm_RegionEnd(i8* getelementptr inbounds ([26 x i8], [26 x i8]* @.str27, i32 0, i32 0)) #0
  br label %160

; <label>:160:                                    ; preds = %159
  %161 = load i32, i32* %i1, align 4
  %162 = add nsw i32 %161, 1
  store i32 %162, i32* %i1, align 4
  br label %107

; <label>:163:                                    ; preds = %107
  br label %164

; <label>:164:                                    ; preds = %163, %102, %99
  %165 = load i32, i32* %c, align 4
  %166 = icmp sge i32 %165, 1
  br i1 %166, label %167, label %198

; <label>:167:                                    ; preds = %164
  %168 = load i32, i32* %c, align 4
  %169 = sub nsw i32 %168, 1
  %170 = sext i32 %169 to i64
  %171 = load i32, i32* %r, align 4
  %172 = sext i32 %171 to i64
  %173 = load [1024 x %struct.tensor_t]*, [1024 x %struct.tensor_t]** %2, align 8
  %174 = getelementptr inbounds [1024 x %struct.tensor_t], [1024 x %struct.tensor_t]* %173, i64 %172
  %175 = getelementptr inbounds [1024 x %struct.tensor_t], [1024 x %struct.tensor_t]* %174, i32 0, i64 %170
  %176 = bitcast %struct.tensor_t* %175 to i8*
  %177 = bitcast %struct.tensor_t* %acc to i8*
  %178 = getelementptr inbounds %struct.tensor_t, %struct.tensor_t* %175, i32 0, i32 0
  %179 = getelementptr inbounds %struct.tensor_t, %struct.tensor_t* %acc, i32 0, i32 0
  %180 = getelementptr inbounds [6 x float], [6 x float]* %178, i32 0, i32 0
  %181 = getelementptr inbounds [6 x float], [6 x float]* %179, i32 0, i32 0
  %182 = load float, float* %181
  store float %182, float* %180
  %183 = getelementptr inbounds [6 x float], [6 x float]* %178, i32 0, i32 1
  %184 = getelementptr inbounds [6 x float], [6 x float]* %179, i32 0, i32 1
  %185 = load float, float* %184
  store float %185, float* %183
  %186 = getelementptr inbounds [6 x float], [6 x float]* %178, i32 0, i32 2
  %187 = getelementptr inbounds [6 x float], [6 x float]* %179, i32 0, i32 2
  %188 = load float, float* %187
  store float %188, float* %186
  %189 = getelementptr inbounds [6 x float], [6 x float]* %178, i32 0, i32 3
  %190 = getelementptr inbounds [6 x float], [6 x float]* %179, i32 0, i32 3
  %191 = load float, float* %190
  store float %191, float* %189
  %192 = getelementptr inbounds [6 x float], [6 x float]* %178, i32 0, i32 4
  %193 = getelementptr inbounds [6 x float], [6 x float]* %179, i32 0, i32 4
  %194 = load float, float* %193
  store float %194, float* %192
  %195 = getelementptr inbounds [6 x float], [6 x float]* %178, i32 0, i32 5
  %196 = getelementptr inbounds [6 x float], [6 x float]* %179, i32 0, i32 5
  %197 = load float, float* %196
  store float %197, float* %195
  br label %198

; <label>:198:                                    ; preds = %167, %164
  call void (...) @_ssdm_RegionEnd(i8* getelementptr inbounds ([22 x i8], [22 x i8]* @.str24, i32 0, i32 0)) #0
  br label %199

; <label>:199:                                    ; preds = %198
  %200 = load i32, i32* %c, align 4
  %201 = add nsw i32 %200, 1
  store i32 %201, i32* %c, align 4
  br label %14

; <label>:202:                                    ; preds = %14
  call void (...) @_ssdm_RegionEnd(i8* getelementptr inbounds ([22 x i8], [22 x i8]* @.str23, i32 0, i32 0)) #0
  br label %203

; <label>:203:                                    ; preds = %202
  %204 = load i32, i32* %r, align 4
  %205 = add nsw i32 %204, 1
  store i32 %205, i32* %r, align 4
  br label %9

; <label>:206:                                    ; preds = %9
  ret void
}

; Function Attrs: nounwind uwtable
define linkonce_odr void @_ZN3hls6WindowILi1ELi3E8tensor_tEC1Ev(%"class.hls::Window.2"* %this) unnamed_addr #2 comdat align 2 {
  %1 = alloca %"class.hls::Window.2"*, align 8
  store %"class.hls::Window.2"* %this, %"class.hls::Window.2"** %1, align 8
  %2 = load %"class.hls::Window.2"*, %"class.hls::Window.2"** %1
  call void @_ZN3hls6WindowILi1ELi3E8tensor_tEC2Ev(%"class.hls::Window.2"* %2)
  ret void
}

; Function Attrs: nounwind uwtable
define linkonce_odr void @_ZN3hls6WindowILi1ELi3E8tensor_tE17shift_pixels_leftEv(%"class.hls::Window.2"* %this) #2 comdat align 2 {
  %1 = alloca %"class.hls::Window.2"*, align 8
  %i = alloca %struct.ap_uint.15, align 4
  %j = alloca %struct.ap_uint.15, align 4
  %2 = alloca %struct.ap_uint.15, align 4
  %3 = alloca %struct.ap_uint.15, align 4
  %4 = alloca %struct.ap_int_base.18, align 8
  %5 = alloca %struct.ap_int_base.16, align 4
  %6 = alloca %struct.ap_int_base.16, align 4
  store %"class.hls::Window.2"* %this, %"class.hls::Window.2"** %1, align 8
  %7 = load %"class.hls::Window.2"*, %"class.hls::Window.2"** %1
  call void (...) @_ssdm_InlineSelf(i32 0, i8* getelementptr inbounds ([1 x i8], [1 x i8]* @.str2, i32 0, i32 0)) #0
  call void @_ZN7ap_uintILi32EEC1Ev(%struct.ap_uint.15* %i)
  call void @_ZN7ap_uintILi32EEC1Ev(%struct.ap_uint.15* %j)
  call void @_ZN7ap_uintILi32EEC1Ei(%struct.ap_uint.15* %2, i32 0)
  %8 = call %struct.ap_uint.15* @_ZN7ap_uintILi32EEaSERKS0_(%struct.ap_uint.15* %i, %struct.ap_uint.15* %2)
  br label %9

; <label>:9:                                      ; preds = %60, %0
  %10 = bitcast %struct.ap_uint.15* %i to %struct.ap_int_base.16*
  %11 = call zeroext i1 @_ZltILi32ELb0EEbRK11ap_int_baseIXT_EXT0_EXleT_Li64EEEi(%struct.ap_int_base.16* %10, i32 1)
  br i1 %11, label %12, label %62

; <label>:12:                                     ; preds = %9
  call void (...) @_ssdm_RegionBegin(i8* getelementptr inbounds ([12 x i8], [12 x i8]* @.str35, i32 0, i32 0)) #0
  call void (...) @_ssdm_Unroll(i32 0, i32 0, i32 0, i8* getelementptr inbounds ([1 x i8], [1 x i8]* @.str2, i32 0, i32 0)) #0
  call void @_ZN7ap_uintILi32EEC1Ei(%struct.ap_uint.15* %3, i32 0)
  %13 = call %struct.ap_uint.15* @_ZN7ap_uintILi32EEaSERKS0_(%struct.ap_uint.15* %j, %struct.ap_uint.15* %3)
  br label %14

; <label>:14:                                     ; preds = %57, %12
  %15 = bitcast %struct.ap_uint.15* %j to %struct.ap_int_base.16*
  %16 = call zeroext i1 @_ZltILi32ELb0EEbRK11ap_int_baseIXT_EXT0_EXleT_Li64EEEi(%struct.ap_int_base.16* %15, i32 2)
  br i1 %16, label %17, label %59

; <label>:17:                                     ; preds = %14
  call void (...) @_ssdm_RegionBegin(i8* getelementptr inbounds ([12 x i8], [12 x i8]* @.str36, i32 0, i32 0)) #0
  call void (...) @_ssdm_Unroll(i32 0, i32 0, i32 0, i8* getelementptr inbounds ([1 x i8], [1 x i8]* @.str2, i32 0, i32 0)) #0
  %18 = bitcast %struct.ap_uint.15* %j to %struct.ap_int_base.16*
  %19 = call i32 @_ZNK11ap_int_baseILi32ELb0ELb1EEcvjEv(%struct.ap_int_base.16* %18)
  %20 = zext i32 %19 to i64
  %21 = bitcast %struct.ap_uint.15* %i to %struct.ap_int_base.16*
  %22 = call i32 @_ZNK11ap_int_baseILi32ELb0ELb1EEcvjEv(%struct.ap_int_base.16* %21)
  %23 = zext i32 %22 to i64
  %24 = getelementptr inbounds %"class.hls::Window.2", %"class.hls::Window.2"* %7, i32 0, i32 0
  %25 = getelementptr inbounds [1 x [3 x %struct.tensor_t]], [1 x [3 x %struct.tensor_t]]* %24, i32 0, i64 %23
  %26 = getelementptr inbounds [3 x %struct.tensor_t], [3 x %struct.tensor_t]* %25, i32 0, i64 %20
  %27 = bitcast %struct.ap_uint.15* %j to %struct.ap_int_base.16*
  call void @_ZplILi32ELb0EEN11ap_int_baseIXT_EXT0_EXleT_Li64EEE5RTypeIXLi32EEXLb1EEE4plusERKS1_i(%struct.ap_int_base.18* sret %4, %struct.ap_int_base.16* %27, i32 1)
  %28 = call i64 @_ZNK11ap_int_baseILi34ELb1ELb1EEcvxEv(%struct.ap_int_base.18* %4)
  %29 = bitcast %struct.ap_uint.15* %i to %struct.ap_int_base.16*
  %30 = call i32 @_ZNK11ap_int_baseILi32ELb0ELb1EEcvjEv(%struct.ap_int_base.16* %29)
  %31 = zext i32 %30 to i64
  %32 = getelementptr inbounds %"class.hls::Window.2", %"class.hls::Window.2"* %7, i32 0, i32 0
  %33 = getelementptr inbounds [1 x [3 x %struct.tensor_t]], [1 x [3 x %struct.tensor_t]]* %32, i32 0, i64 %31
  %34 = getelementptr inbounds [3 x %struct.tensor_t], [3 x %struct.tensor_t]* %33, i32 0, i64 %28
  %35 = bitcast %struct.tensor_t* %26 to i8*
  %36 = bitcast %struct.tensor_t* %34 to i8*
  %37 = getelementptr inbounds %struct.tensor_t, %struct.tensor_t* %26, i32 0, i32 0
  %38 = getelementptr inbounds %struct.tensor_t, %struct.tensor_t* %34, i32 0, i32 0
  %39 = getelementptr inbounds [6 x float], [6 x float]* %37, i32 0, i32 0
  %40 = getelementptr inbounds [6 x float], [6 x float]* %38, i32 0, i32 0
  %41 = load float, float* %40
  store float %41, float* %39
  %42 = getelementptr inbounds [6 x float], [6 x float]* %37, i32 0, i32 1
  %43 = getelementptr inbounds [6 x float], [6 x float]* %38, i32 0, i32 1
  %44 = load float, float* %43
  store float %44, float* %42
  %45 = getelementptr inbounds [6 x float], [6 x float]* %37, i32 0, i32 2
  %46 = getelementptr inbounds [6 x float], [6 x float]* %38, i32 0, i32 2
  %47 = load float, float* %46
  store float %47, float* %45
  %48 = getelementptr inbounds [6 x float], [6 x float]* %37, i32 0, i32 3
  %49 = getelementptr inbounds [6 x float], [6 x float]* %38, i32 0, i32 3
  %50 = load float, float* %49
  store float %50, float* %48
  %51 = getelementptr inbounds [6 x float], [6 x float]* %37, i32 0, i32 4
  %52 = getelementptr inbounds [6 x float], [6 x float]* %38, i32 0, i32 4
  %53 = load float, float* %52
  store float %53, float* %51
  %54 = getelementptr inbounds [6 x float], [6 x float]* %37, i32 0, i32 5
  %55 = getelementptr inbounds [6 x float], [6 x float]* %38, i32 0, i32 5
  %56 = load float, float* %55
  store float %56, float* %54
  call void (...) @_ssdm_RegionEnd(i8* getelementptr inbounds ([12 x i8], [12 x i8]* @.str36, i32 0, i32 0)) #0
  br label %57

; <label>:57:                                     ; preds = %17
  %58 = bitcast %struct.ap_uint.15* %j to %struct.ap_int_base.16*
  call void @_ZN11ap_int_baseILi32ELb0ELb1EEppEi(%struct.ap_int_base.16* sret %5, %struct.ap_int_base.16* %58, i32 0)
  br label %14

; <label>:59:                                     ; preds = %14
  call void (...) @_ssdm_RegionEnd(i8* getelementptr inbounds ([12 x i8], [12 x i8]* @.str35, i32 0, i32 0)) #0
  br label %60

; <label>:60:                                     ; preds = %59
  %61 = bitcast %struct.ap_uint.15* %i to %struct.ap_int_base.16*
  call void @_ZN11ap_int_baseILi32ELb0ELb1EEppEi(%struct.ap_int_base.16* sret %6, %struct.ap_int_base.16* %61, i32 0)
  br label %9

; <label>:62:                                     ; preds = %9
  ret void
}

; Function Attrs: nounwind uwtable
define linkonce_odr void @_ZN3hls6WindowILi1ELi3E8tensor_tE12insert_pixelES1_ii(%"class.hls::Window.2"* %this, %struct.tensor_t* byval align 8 %value, i32 %row, i32 %col) #2 comdat align 2 {
  %1 = alloca %"class.hls::Window.2"*, align 8
  %2 = alloca i32, align 4
  %3 = alloca i32, align 4
  store %"class.hls::Window.2"* %this, %"class.hls::Window.2"** %1, align 8
  store i32 %row, i32* %2, align 4
  store i32 %col, i32* %3, align 4
  %4 = load %"class.hls::Window.2"*, %"class.hls::Window.2"** %1
  call void (...) @_ssdm_InlineSelf(i32 0, i8* getelementptr inbounds ([1 x i8], [1 x i8]* @.str2, i32 0, i32 0)) #0
  %5 = load i32, i32* %2, align 4
  %6 = icmp sge i32 %5, 0
  br i1 %6, label %7, label %16

; <label>:7:                                      ; preds = %0
  %8 = load i32, i32* %2, align 4
  %9 = icmp slt i32 %8, 1
  br i1 %9, label %10, label %16

; <label>:10:                                     ; preds = %7
  %11 = load i32, i32* %3, align 4
  %12 = icmp sge i32 %11, 0
  br i1 %12, label %13, label %16

; <label>:13:                                     ; preds = %10
  %14 = load i32, i32* %3, align 4
  %15 = icmp slt i32 %14, 3
  br label %16

; <label>:16:                                     ; preds = %13, %10, %7, %0
  %17 = phi i1 [ false, %10 ], [ false, %7 ], [ false, %0 ], [ %15, %13 ]
  br i1 %17, label %18, label %19

; <label>:18:                                     ; preds = %16
  br label %21

; <label>:19:                                     ; preds = %16
  call void @__assert_fail(i8* getelementptr inbounds ([49 x i8], [49 x i8]* @.str33, i32 0, i32 0), i8* getelementptr inbounds ([81 x i8], [81 x i8]* @.str34, i32 0, i32 0), i32 266, i8* getelementptr inbounds ([95 x i8], [95 x i8]* @__PRETTY_FUNCTION__._ZN3hls6WindowILi1ELi3E8tensor_tE12insert_pixelES1_ii, i32 0, i32 0)) #5
  unreachable
                                                  ; No predecessors!
  br label %21

; <label>:21:                                     ; preds = %20, %18
  %22 = load i32, i32* %3, align 4
  %23 = sext i32 %22 to i64
  %24 = load i32, i32* %2, align 4
  %25 = sext i32 %24 to i64
  %26 = getelementptr inbounds %"class.hls::Window.2", %"class.hls::Window.2"* %4, i32 0, i32 0
  %27 = getelementptr inbounds [1 x [3 x %struct.tensor_t]], [1 x [3 x %struct.tensor_t]]* %26, i32 0, i64 %25
  %28 = getelementptr inbounds [3 x %struct.tensor_t], [3 x %struct.tensor_t]* %27, i32 0, i64 %23
  %29 = bitcast %struct.tensor_t* %28 to i8*
  %30 = bitcast %struct.tensor_t* %value to i8*
  %31 = getelementptr inbounds %struct.tensor_t, %struct.tensor_t* %28, i32 0, i32 0
  %32 = getelementptr inbounds %struct.tensor_t, %struct.tensor_t* %value, i32 0, i32 0
  %33 = getelementptr inbounds [6 x float], [6 x float]* %31, i32 0, i32 0
  %34 = getelementptr inbounds [6 x float], [6 x float]* %32, i32 0, i32 0
  %35 = load float, float* %34
  store float %35, float* %33
  %36 = getelementptr inbounds [6 x float], [6 x float]* %31, i32 0, i32 1
  %37 = getelementptr inbounds [6 x float], [6 x float]* %32, i32 0, i32 1
  %38 = load float, float* %37
  store float %38, float* %36
  %39 = getelementptr inbounds [6 x float], [6 x float]* %31, i32 0, i32 2
  %40 = getelementptr inbounds [6 x float], [6 x float]* %32, i32 0, i32 2
  %41 = load float, float* %40
  store float %41, float* %39
  %42 = getelementptr inbounds [6 x float], [6 x float]* %31, i32 0, i32 3
  %43 = getelementptr inbounds [6 x float], [6 x float]* %32, i32 0, i32 3
  %44 = load float, float* %43
  store float %44, float* %42
  %45 = getelementptr inbounds [6 x float], [6 x float]* %31, i32 0, i32 4
  %46 = getelementptr inbounds [6 x float], [6 x float]* %32, i32 0, i32 4
  %47 = load float, float* %46
  store float %47, float* %45
  %48 = getelementptr inbounds [6 x float], [6 x float]* %31, i32 0, i32 5
  %49 = getelementptr inbounds [6 x float], [6 x float]* %32, i32 0, i32 5
  %50 = load float, float* %49
  store float %50, float* %48
  ret void
}

; Function Attrs: nounwind uwtable
define linkonce_odr %struct.tensor_t* @_ZN3hls6WindowILi1ELi3E8tensor_tE6getvalEii(%"class.hls::Window.2"* %this, i32 %row, i32 %col) #2 comdat align 2 {
  %1 = alloca %"class.hls::Window.2"*, align 8
  %2 = alloca i32, align 4
  %3 = alloca i32, align 4
  store %"class.hls::Window.2"* %this, %"class.hls::Window.2"** %1, align 8
  store i32 %row, i32* %2, align 4
  store i32 %col, i32* %3, align 4
  %4 = load %"class.hls::Window.2"*, %"class.hls::Window.2"** %1
  call void (...) @_ssdm_InlineSelf(i32 0, i8* getelementptr inbounds ([1 x i8], [1 x i8]* @.str2, i32 0, i32 0)) #0
  %5 = load i32, i32* %2, align 4
  %6 = icmp sge i32 %5, 0
  br i1 %6, label %7, label %16

; <label>:7:                                      ; preds = %0
  %8 = load i32, i32* %2, align 4
  %9 = icmp slt i32 %8, 1
  br i1 %9, label %10, label %16

; <label>:10:                                     ; preds = %7
  %11 = load i32, i32* %3, align 4
  %12 = icmp sge i32 %11, 0
  br i1 %12, label %13, label %16

; <label>:13:                                     ; preds = %10
  %14 = load i32, i32* %3, align 4
  %15 = icmp slt i32 %14, 3
  br label %16

; <label>:16:                                     ; preds = %13, %10, %7, %0
  %17 = phi i1 [ false, %10 ], [ false, %7 ], [ false, %0 ], [ %15, %13 ]
  br i1 %17, label %18, label %19

; <label>:18:                                     ; preds = %16
  br label %21

; <label>:19:                                     ; preds = %16
  call void @__assert_fail(i8* getelementptr inbounds ([49 x i8], [49 x i8]* @.str33, i32 0, i32 0), i8* getelementptr inbounds ([81 x i8], [81 x i8]* @.str34, i32 0, i32 0), i32 492, i8* getelementptr inbounds ([84 x i8], [84 x i8]* @__PRETTY_FUNCTION__._ZN3hls6WindowILi1ELi3E8tensor_tE6getvalEii, i32 0, i32 0)) #5
  unreachable
                                                  ; No predecessors!
  br label %21

; <label>:21:                                     ; preds = %20, %18
  %22 = load i32, i32* %3, align 4
  %23 = sext i32 %22 to i64
  %24 = load i32, i32* %2, align 4
  %25 = sext i32 %24 to i64
  %26 = getelementptr inbounds %"class.hls::Window.2", %"class.hls::Window.2"* %4, i32 0, i32 0
  %27 = getelementptr inbounds [1 x [3 x %struct.tensor_t]], [1 x [3 x %struct.tensor_t]]* %26, i32 0, i64 %25
  %28 = getelementptr inbounds [3 x %struct.tensor_t], [3 x %struct.tensor_t]* %27, i32 0, i64 %23
  ret %struct.tensor_t* %28
}

; Function Attrs: nounwind uwtable
define void @_Z9flow_calcPA1024_8tensor_tPA1024_10velocity_t([1024 x %struct.tensor_t]* %tensors, [1024 x %struct.velocity_t]* %outputs) #2 {
  %1 = alloca [1024 x %struct.tensor_t]*, align 8
  %2 = alloca [1024 x %struct.velocity_t]*, align 8
  %r = alloca i32, align 4
  %c = alloca i32, align 4
  %denom = alloca float, align 4
  store [1024 x %struct.tensor_t]* %tensors, [1024 x %struct.tensor_t]** %1, align 8
  store [1024 x %struct.velocity_t]* %outputs, [1024 x %struct.velocity_t]** %2, align 8
  %3 = load [1024 x %struct.tensor_t]*, [1024 x %struct.tensor_t]** %1, align 8
  call void (...) @_ssdm_SpecArrayDimSize([1024 x %struct.tensor_t]* %3, i32 436) #0
  %4 = load [1024 x %struct.velocity_t]*, [1024 x %struct.velocity_t]** %2, align 8
  call void (...) @_ssdm_SpecArrayDimSize([1024 x %struct.velocity_t]* %4, i32 436) #0
  %5 = load [1024 x %struct.tensor_t]*, [1024 x %struct.tensor_t]** %1, align 8
  call void (...) @_ssdm_op_SpecInterface([1024 x %struct.tensor_t]* %5, i8* getelementptr inbounds ([8 x i8], [8 x i8]* @.str, i32 0, i32 0), i32 0, i32 0, i8* getelementptr inbounds ([1 x i8], [1 x i8]* @.str2, i32 0, i32 0), i32 0, i32 0, i8* getelementptr inbounds ([1 x i8], [1 x i8]* @.str2, i32 0, i32 0), i8* getelementptr inbounds ([1 x i8], [1 x i8]* @.str2, i32 0, i32 0), i8* getelementptr inbounds ([1 x i8], [1 x i8]* @.str2, i32 0, i32 0), i32 0, i32 0, i32 0, i32 0, i8* getelementptr inbounds ([1 x i8], [1 x i8]* @.str2, i32 0, i32 0), i8* getelementptr inbounds ([1 x i8], [1 x i8]* @.str2, i32 0, i32 0)) #0
  br label %6

; <label>:6:                                      ; preds = %0
  store i32 0, i32* %r, align 4
  br label %7

; <label>:7:                                      ; preds = %185, %6
  %8 = load i32, i32* %r, align 4
  %9 = icmp slt i32 %8, 436
  br i1 %9, label %10, label %188

; <label>:10:                                     ; preds = %7
  call void (...) @_ssdm_op_SpecLoopName(i8* getelementptr inbounds ([11 x i8], [11 x i8]* @.str29, i32 0, i32 0)) #0
  call void (...) @_ssdm_RegionBegin(i8* getelementptr inbounds ([11 x i8], [11 x i8]* @.str29, i32 0, i32 0)) #0
  br label %11

; <label>:11:                                     ; preds = %10
  store i32 0, i32* %c, align 4
  br label %12

; <label>:12:                                     ; preds = %181, %11
  %13 = load i32, i32* %c, align 4
  %14 = icmp slt i32 %13, 1024
  br i1 %14, label %15, label %184

; <label>:15:                                     ; preds = %12
  call void (...) @_ssdm_op_SpecLoopName(i8* getelementptr inbounds ([11 x i8], [11 x i8]* @.str30, i32 0, i32 0)) #0
  call void (...) @_ssdm_RegionBegin(i8* getelementptr inbounds ([11 x i8], [11 x i8]* @.str30, i32 0, i32 0)) #0
  call void (...) @_ssdm_op_SpecPipeline(i32 1, i32 1, i32 1, i32 0, i8* getelementptr inbounds ([1 x i8], [1 x i8]* @.str2, i32 0, i32 0)) #0
  %16 = load i32, i32* %r, align 4
  %17 = icmp sge i32 %16, 2
  br i1 %17, label %18, label %161

; <label>:18:                                     ; preds = %15
  %19 = load i32, i32* %r, align 4
  %20 = icmp slt i32 %19, 434
  br i1 %20, label %21, label %161

; <label>:21:                                     ; preds = %18
  %22 = load i32, i32* %c, align 4
  %23 = icmp sge i32 %22, 2
  br i1 %23, label %24, label %161

; <label>:24:                                     ; preds = %21
  %25 = load i32, i32* %c, align 4
  %26 = icmp slt i32 %25, 1022
  br i1 %26, label %27, label %161

; <label>:27:                                     ; preds = %24
  %28 = load i32, i32* %c, align 4
  %29 = sext i32 %28 to i64
  %30 = load i32, i32* %r, align 4
  %31 = sext i32 %30 to i64
  %32 = load [1024 x %struct.tensor_t]*, [1024 x %struct.tensor_t]** %1, align 8
  %33 = getelementptr inbounds [1024 x %struct.tensor_t], [1024 x %struct.tensor_t]* %32, i64 %31
  %34 = getelementptr inbounds [1024 x %struct.tensor_t], [1024 x %struct.tensor_t]* %33, i32 0, i64 %29
  %35 = getelementptr inbounds %struct.tensor_t, %struct.tensor_t* %34, i32 0, i32 0
  %36 = getelementptr inbounds [6 x float], [6 x float]* %35, i32 0, i64 0
  %37 = load float, float* %36, align 4
  %38 = load i32, i32* %c, align 4
  %39 = sext i32 %38 to i64
  %40 = load i32, i32* %r, align 4
  %41 = sext i32 %40 to i64
  %42 = load [1024 x %struct.tensor_t]*, [1024 x %struct.tensor_t]** %1, align 8
  %43 = getelementptr inbounds [1024 x %struct.tensor_t], [1024 x %struct.tensor_t]* %42, i64 %41
  %44 = getelementptr inbounds [1024 x %struct.tensor_t], [1024 x %struct.tensor_t]* %43, i32 0, i64 %39
  %45 = getelementptr inbounds %struct.tensor_t, %struct.tensor_t* %44, i32 0, i32 0
  %46 = getelementptr inbounds [6 x float], [6 x float]* %45, i32 0, i64 1
  %47 = load float, float* %46, align 4
  %48 = fmul float %37, %47
  %49 = load i32, i32* %c, align 4
  %50 = sext i32 %49 to i64
  %51 = load i32, i32* %r, align 4
  %52 = sext i32 %51 to i64
  %53 = load [1024 x %struct.tensor_t]*, [1024 x %struct.tensor_t]** %1, align 8
  %54 = getelementptr inbounds [1024 x %struct.tensor_t], [1024 x %struct.tensor_t]* %53, i64 %52
  %55 = getelementptr inbounds [1024 x %struct.tensor_t], [1024 x %struct.tensor_t]* %54, i32 0, i64 %50
  %56 = getelementptr inbounds %struct.tensor_t, %struct.tensor_t* %55, i32 0, i32 0
  %57 = getelementptr inbounds [6 x float], [6 x float]* %56, i32 0, i64 3
  %58 = load float, float* %57, align 4
  %59 = load i32, i32* %c, align 4
  %60 = sext i32 %59 to i64
  %61 = load i32, i32* %r, align 4
  %62 = sext i32 %61 to i64
  %63 = load [1024 x %struct.tensor_t]*, [1024 x %struct.tensor_t]** %1, align 8
  %64 = getelementptr inbounds [1024 x %struct.tensor_t], [1024 x %struct.tensor_t]* %63, i64 %62
  %65 = getelementptr inbounds [1024 x %struct.tensor_t], [1024 x %struct.tensor_t]* %64, i32 0, i64 %60
  %66 = getelementptr inbounds %struct.tensor_t, %struct.tensor_t* %65, i32 0, i32 0
  %67 = getelementptr inbounds [6 x float], [6 x float]* %66, i32 0, i64 3
  %68 = load float, float* %67, align 4
  %69 = fmul float %58, %68
  %70 = fsub float %48, %69
  store float %70, float* %denom, align 4
  %71 = load i32, i32* %c, align 4
  %72 = sext i32 %71 to i64
  %73 = load i32, i32* %r, align 4
  %74 = sext i32 %73 to i64
  %75 = load [1024 x %struct.tensor_t]*, [1024 x %struct.tensor_t]** %1, align 8
  %76 = getelementptr inbounds [1024 x %struct.tensor_t], [1024 x %struct.tensor_t]* %75, i64 %74
  %77 = getelementptr inbounds [1024 x %struct.tensor_t], [1024 x %struct.tensor_t]* %76, i32 0, i64 %72
  %78 = getelementptr inbounds %struct.tensor_t, %struct.tensor_t* %77, i32 0, i32 0
  %79 = getelementptr inbounds [6 x float], [6 x float]* %78, i32 0, i64 5
  %80 = load float, float* %79, align 4
  %81 = load i32, i32* %c, align 4
  %82 = sext i32 %81 to i64
  %83 = load i32, i32* %r, align 4
  %84 = sext i32 %83 to i64
  %85 = load [1024 x %struct.tensor_t]*, [1024 x %struct.tensor_t]** %1, align 8
  %86 = getelementptr inbounds [1024 x %struct.tensor_t], [1024 x %struct.tensor_t]* %85, i64 %84
  %87 = getelementptr inbounds [1024 x %struct.tensor_t], [1024 x %struct.tensor_t]* %86, i32 0, i64 %82
  %88 = getelementptr inbounds %struct.tensor_t, %struct.tensor_t* %87, i32 0, i32 0
  %89 = getelementptr inbounds [6 x float], [6 x float]* %88, i32 0, i64 3
  %90 = load float, float* %89, align 4
  %91 = fmul float %80, %90
  %92 = load i32, i32* %c, align 4
  %93 = sext i32 %92 to i64
  %94 = load i32, i32* %r, align 4
  %95 = sext i32 %94 to i64
  %96 = load [1024 x %struct.tensor_t]*, [1024 x %struct.tensor_t]** %1, align 8
  %97 = getelementptr inbounds [1024 x %struct.tensor_t], [1024 x %struct.tensor_t]* %96, i64 %95
  %98 = getelementptr inbounds [1024 x %struct.tensor_t], [1024 x %struct.tensor_t]* %97, i32 0, i64 %93
  %99 = getelementptr inbounds %struct.tensor_t, %struct.tensor_t* %98, i32 0, i32 0
  %100 = getelementptr inbounds [6 x float], [6 x float]* %99, i32 0, i64 4
  %101 = load float, float* %100, align 4
  %102 = load i32, i32* %c, align 4
  %103 = sext i32 %102 to i64
  %104 = load i32, i32* %r, align 4
  %105 = sext i32 %104 to i64
  %106 = load [1024 x %struct.tensor_t]*, [1024 x %struct.tensor_t]** %1, align 8
  %107 = getelementptr inbounds [1024 x %struct.tensor_t], [1024 x %struct.tensor_t]* %106, i64 %105
  %108 = getelementptr inbounds [1024 x %struct.tensor_t], [1024 x %struct.tensor_t]* %107, i32 0, i64 %103
  %109 = getelementptr inbounds %struct.tensor_t, %struct.tensor_t* %108, i32 0, i32 0
  %110 = getelementptr inbounds [6 x float], [6 x float]* %109, i32 0, i64 1
  %111 = load float, float* %110, align 4
  %112 = fmul float %101, %111
  %113 = fsub float %91, %112
  %114 = load float, float* %denom, align 4
  %115 = fdiv float %113, %114
  store float %115, float* getelementptr inbounds ([2 x float], [2 x float]* @_ZZ9flow_calcPA1024_8tensor_tPA1024_10velocity_tE3buf, i32 0, i64 0), align 4
  %116 = load i32, i32* %c, align 4
  %117 = sext i32 %116 to i64
  %118 = load i32, i32* %r, align 4
  %119 = sext i32 %118 to i64
  %120 = load [1024 x %struct.tensor_t]*, [1024 x %struct.tensor_t]** %1, align 8
  %121 = getelementptr inbounds [1024 x %struct.tensor_t], [1024 x %struct.tensor_t]* %120, i64 %119
  %122 = getelementptr inbounds [1024 x %struct.tensor_t], [1024 x %struct.tensor_t]* %121, i32 0, i64 %117
  %123 = getelementptr inbounds %struct.tensor_t, %struct.tensor_t* %122, i32 0, i32 0
  %124 = getelementptr inbounds [6 x float], [6 x float]* %123, i32 0, i64 4
  %125 = load float, float* %124, align 4
  %126 = load i32, i32* %c, align 4
  %127 = sext i32 %126 to i64
  %128 = load i32, i32* %r, align 4
  %129 = sext i32 %128 to i64
  %130 = load [1024 x %struct.tensor_t]*, [1024 x %struct.tensor_t]** %1, align 8
  %131 = getelementptr inbounds [1024 x %struct.tensor_t], [1024 x %struct.tensor_t]* %130, i64 %129
  %132 = getelementptr inbounds [1024 x %struct.tensor_t], [1024 x %struct.tensor_t]* %131, i32 0, i64 %127
  %133 = getelementptr inbounds %struct.tensor_t, %struct.tensor_t* %132, i32 0, i32 0
  %134 = getelementptr inbounds [6 x float], [6 x float]* %133, i32 0, i64 3
  %135 = load float, float* %134, align 4
  %136 = fmul float %125, %135
  %137 = load i32, i32* %c, align 4
  %138 = sext i32 %137 to i64
  %139 = load i32, i32* %r, align 4
  %140 = sext i32 %139 to i64
  %141 = load [1024 x %struct.tensor_t]*, [1024 x %struct.tensor_t]** %1, align 8
  %142 = getelementptr inbounds [1024 x %struct.tensor_t], [1024 x %struct.tensor_t]* %141, i64 %140
  %143 = getelementptr inbounds [1024 x %struct.tensor_t], [1024 x %struct.tensor_t]* %142, i32 0, i64 %138
  %144 = getelementptr inbounds %struct.tensor_t, %struct.tensor_t* %143, i32 0, i32 0
  %145 = getelementptr inbounds [6 x float], [6 x float]* %144, i32 0, i64 5
  %146 = load float, float* %145, align 4
  %147 = load i32, i32* %c, align 4
  %148 = sext i32 %147 to i64
  %149 = load i32, i32* %r, align 4
  %150 = sext i32 %149 to i64
  %151 = load [1024 x %struct.tensor_t]*, [1024 x %struct.tensor_t]** %1, align 8
  %152 = getelementptr inbounds [1024 x %struct.tensor_t], [1024 x %struct.tensor_t]* %151, i64 %150
  %153 = getelementptr inbounds [1024 x %struct.tensor_t], [1024 x %struct.tensor_t]* %152, i32 0, i64 %148
  %154 = getelementptr inbounds %struct.tensor_t, %struct.tensor_t* %153, i32 0, i32 0
  %155 = getelementptr inbounds [6 x float], [6 x float]* %154, i32 0, i64 0
  %156 = load float, float* %155, align 4
  %157 = fmul float %146, %156
  %158 = fsub float %136, %157
  %159 = load float, float* %denom, align 4
  %160 = fdiv float %158, %159
  store float %160, float* getelementptr inbounds ([2 x float], [2 x float]* @_ZZ9flow_calcPA1024_8tensor_tPA1024_10velocity_tE3buf, i32 0, i64 1), align 4
  br label %162

; <label>:161:                                    ; preds = %24, %21, %18, %15
  store float 0.000000e+00, float* getelementptr inbounds ([2 x float], [2 x float]* @_ZZ9flow_calcPA1024_8tensor_tPA1024_10velocity_tE3buf, i32 0, i64 1), align 4
  store float 0.000000e+00, float* getelementptr inbounds ([2 x float], [2 x float]* @_ZZ9flow_calcPA1024_8tensor_tPA1024_10velocity_tE3buf, i32 0, i64 0), align 4
  br label %162

; <label>:162:                                    ; preds = %161, %27
  %163 = load float, float* getelementptr inbounds ([2 x float], [2 x float]* @_ZZ9flow_calcPA1024_8tensor_tPA1024_10velocity_tE3buf, i32 0, i64 0), align 4
  %164 = load i32, i32* %c, align 4
  %165 = sext i32 %164 to i64
  %166 = load i32, i32* %r, align 4
  %167 = sext i32 %166 to i64
  %168 = load [1024 x %struct.velocity_t]*, [1024 x %struct.velocity_t]** %2, align 8
  %169 = getelementptr inbounds [1024 x %struct.velocity_t], [1024 x %struct.velocity_t]* %168, i64 %167
  %170 = getelementptr inbounds [1024 x %struct.velocity_t], [1024 x %struct.velocity_t]* %169, i32 0, i64 %165
  %171 = getelementptr inbounds %struct.velocity_t, %struct.velocity_t* %170, i32 0, i32 0
  store float %163, float* %171, align 4
  %172 = load float, float* getelementptr inbounds ([2 x float], [2 x float]* @_ZZ9flow_calcPA1024_8tensor_tPA1024_10velocity_tE3buf, i32 0, i64 1), align 4
  %173 = load i32, i32* %c, align 4
  %174 = sext i32 %173 to i64
  %175 = load i32, i32* %r, align 4
  %176 = sext i32 %175 to i64
  %177 = load [1024 x %struct.velocity_t]*, [1024 x %struct.velocity_t]** %2, align 8
  %178 = getelementptr inbounds [1024 x %struct.velocity_t], [1024 x %struct.velocity_t]* %177, i64 %176
  %179 = getelementptr inbounds [1024 x %struct.velocity_t], [1024 x %struct.velocity_t]* %178, i32 0, i64 %174
  %180 = getelementptr inbounds %struct.velocity_t, %struct.velocity_t* %179, i32 0, i32 1
  store float %172, float* %180, align 4
  call void (...) @_ssdm_RegionEnd(i8* getelementptr inbounds ([11 x i8], [11 x i8]* @.str30, i32 0, i32 0)) #0
  br label %181

; <label>:181:                                    ; preds = %162
  %182 = load i32, i32* %c, align 4
  %183 = add nsw i32 %182, 1
  store i32 %183, i32* %c, align 4
  br label %12

; <label>:184:                                    ; preds = %12
  call void (...) @_ssdm_RegionEnd(i8* getelementptr inbounds ([11 x i8], [11 x i8]* @.str29, i32 0, i32 0)) #0
  br label %185

; <label>:185:                                    ; preds = %184
  %186 = load i32, i32* %r, align 4
  %187 = add nsw i32 %186, 1
  store i32 %187, i32* %r, align 4
  br label %7

; <label>:188:                                    ; preds = %7
  ret void
}

; Function Attrs: nounwind uwtable
define void @_Z12optical_flowPA1024_7ap_uintILi64EEPA1024_10velocity_t([1024 x %struct.ap_uint.3]* %frames, [1024 x %struct.velocity_t]* %outputs) #2 {
  %1 = alloca [1024 x %struct.ap_uint.3]*, align 8
  %2 = alloca [1024 x %struct.velocity_t]*, align 8
  %r = alloca i32, align 4
  %c = alloca i32, align 4
  %3 = alloca %struct.ap_range_ref, align 8
  %4 = alloca %struct.ap_range_ref, align 8
  %5 = alloca %struct.ap_range_ref, align 8
  %6 = alloca %struct.ap_range_ref, align 8
  %7 = alloca %struct.ap_range_ref, align 8
  %8 = alloca %struct.ap_range_ref, align 8
  store [1024 x %struct.ap_uint.3]* %frames, [1024 x %struct.ap_uint.3]** %1, align 8
  store [1024 x %struct.velocity_t]* %outputs, [1024 x %struct.velocity_t]** %2, align 8
  %9 = load [1024 x %struct.ap_uint.3]*, [1024 x %struct.ap_uint.3]** %1, align 8
  call void (...) @_ssdm_SpecArrayDimSize([1024 x %struct.ap_uint.3]* %9, i32 436) #0
  %10 = load [1024 x %struct.velocity_t]*, [1024 x %struct.velocity_t]** %2, align 8
  call void (...) @_ssdm_SpecArrayDimSize([1024 x %struct.velocity_t]* %10, i32 436) #0
  %11 = load [1024 x %struct.velocity_t]*, [1024 x %struct.velocity_t]** %2, align 8
  call void (...) @_ssdm_DataPack([1024 x %struct.velocity_t]* %11, i32 0, i32 0, i8* getelementptr inbounds ([1 x i8], [1 x i8]* @.str2, i32 0, i32 0), i8* getelementptr inbounds ([1 x i8], [1 x i8]* @.str2, i32 0, i32 0), i8* getelementptr inbounds ([1 x i8], [1 x i8]* @.str2, i32 0, i32 0)) #0
  %12 = load [1024 x %struct.velocity_t]*, [1024 x %struct.velocity_t]** %2, align 8
  call void (...) @_ssdm_SpecStream([1024 x %struct.velocity_t]* %12, i32 0, i32 10, i8* getelementptr inbounds ([1 x i8], [1 x i8]* @.str2, i32 0, i32 0)) #0
  %13 = load [1024 x %struct.ap_uint.3]*, [1024 x %struct.ap_uint.3]** %1, align 8
  call void (...) @_ssdm_SpecStream([1024 x %struct.ap_uint.3]* %13, i32 0, i32 10, i8* getelementptr inbounds ([1 x i8], [1 x i8]* @.str2, i32 0, i32 0)) #0
  call void (...) @_ssdm_op_SpecDataflowPipeline(i32 -1, i8* getelementptr inbounds ([1 x i8], [1 x i8]* @.str2, i32 0, i32 0)) #0
  call void (...) @_ssdm_SpecStream([436 x [1024 x float]]* @_ZZ12optical_flowPA1024_7ap_uintILi64EEPA1024_10velocity_tE10gradient_x, i32 0, i32 10, i8* getelementptr inbounds ([1 x i8], [1 x i8]* @.str2, i32 0, i32 0)) #0
  call void (...) @_ssdm_SpecStream([436 x [1024 x float]]* @_ZZ12optical_flowPA1024_7ap_uintILi64EEPA1024_10velocity_tE10gradient_y, i32 0, i32 10, i8* getelementptr inbounds ([1 x i8], [1 x i8]* @.str2, i32 0, i32 0)) #0
  call void (...) @_ssdm_SpecStream([436 x [1024 x float]]* @_ZZ12optical_flowPA1024_7ap_uintILi64EEPA1024_10velocity_tE10gradient_z, i32 0, i32 4096, i8* getelementptr inbounds ([1 x i8], [1 x i8]* @.str2, i32 0, i32 0)) #0
  call void (...) @_ssdm_SpecStream([436 x [1024 x %struct.gradient_t]]* @_ZZ12optical_flowPA1024_7ap_uintILi64EEPA1024_10velocity_tE10y_filtered, i32 0, i32 10, i8* getelementptr inbounds ([1 x i8], [1 x i8]* @.str2, i32 0, i32 0)) #0
  call void (...) @_ssdm_SpecStream([436 x [1024 x %struct.gradient_t]]* @_ZZ12optical_flowPA1024_7ap_uintILi64EEPA1024_10velocity_tE17filtered_gradient, i32 0, i32 10, i8* getelementptr inbounds ([1 x i8], [1 x i8]* @.str2, i32 0, i32 0)) #0
  call void (...) @_ssdm_SpecStream([436 x [1024 x %struct.outer_t]]* @_ZZ12optical_flowPA1024_7ap_uintILi64EEPA1024_10velocity_tE11out_product, i32 0, i32 10, i8* getelementptr inbounds ([1 x i8], [1 x i8]* @.str2, i32 0, i32 0)) #0
  call void (...) @_ssdm_DataPack([436 x [1024 x %struct.outer_t]]* @_ZZ12optical_flowPA1024_7ap_uintILi64EEPA1024_10velocity_tE11out_product, i32 0, i32 0, i8* getelementptr inbounds ([1 x i8], [1 x i8]* @.str2, i32 0, i32 0), i8* getelementptr inbounds ([1 x i8], [1 x i8]* @.str2, i32 0, i32 0), i8* getelementptr inbounds ([1 x i8], [1 x i8]* @.str2, i32 0, i32 0)) #0
  call void (...) @_ssdm_SpecStream([436 x [1024 x %struct.tensor_t]]* @_ZZ12optical_flowPA1024_7ap_uintILi64EEPA1024_10velocity_tE8tensor_y, i32 0, i32 10, i8* getelementptr inbounds ([1 x i8], [1 x i8]* @.str2, i32 0, i32 0)) #0
  call void (...) @_ssdm_DataPack([436 x [1024 x %struct.tensor_t]]* @_ZZ12optical_flowPA1024_7ap_uintILi64EEPA1024_10velocity_tE8tensor_y, i32 0, i32 0, i8* getelementptr inbounds ([1 x i8], [1 x i8]* @.str2, i32 0, i32 0), i8* getelementptr inbounds ([1 x i8], [1 x i8]* @.str2, i32 0, i32 0), i8* getelementptr inbounds ([1 x i8], [1 x i8]* @.str2, i32 0, i32 0)) #0
  call void (...) @_ssdm_SpecStream([436 x [1024 x %struct.tensor_t]]* @_ZZ12optical_flowPA1024_7ap_uintILi64EEPA1024_10velocity_tE6tensor, i32 0, i32 10, i8* getelementptr inbounds ([1 x i8], [1 x i8]* @.str2, i32 0, i32 0)) #0
  call void (...) @_ssdm_DataPack([436 x [1024 x %struct.tensor_t]]* @_ZZ12optical_flowPA1024_7ap_uintILi64EEPA1024_10velocity_tE6tensor, i32 0, i32 0, i8* getelementptr inbounds ([1 x i8], [1 x i8]* @.str2, i32 0, i32 0), i8* getelementptr inbounds ([1 x i8], [1 x i8]* @.str2, i32 0, i32 0), i8* getelementptr inbounds ([1 x i8], [1 x i8]* @.str2, i32 0, i32 0)) #0
  call void (...) @_ssdm_SpecStream([436 x [1024 x float]]* @_ZZ12optical_flowPA1024_7ap_uintILi64EEPA1024_10velocity_tE8frame1_a, i32 0, i32 10, i8* getelementptr inbounds ([1 x i8], [1 x i8]* @.str2, i32 0, i32 0)) #0
  call void (...) @_ssdm_SpecStream([436 x [1024 x float]]* @_ZZ12optical_flowPA1024_7ap_uintILi64EEPA1024_10velocity_tE8frame2_a, i32 0, i32 10, i8* getelementptr inbounds ([1 x i8], [1 x i8]* @.str2, i32 0, i32 0)) #0
  call void (...) @_ssdm_SpecStream([436 x [1024 x float]]* @_ZZ12optical_flowPA1024_7ap_uintILi64EEPA1024_10velocity_tE8frame4_a, i32 0, i32 10, i8* getelementptr inbounds ([1 x i8], [1 x i8]* @.str2, i32 0, i32 0)) #0
  call void (...) @_ssdm_SpecStream([436 x [1024 x float]]* @_ZZ12optical_flowPA1024_7ap_uintILi64EEPA1024_10velocity_tE8frame5_a, i32 0, i32 10, i8* getelementptr inbounds ([1 x i8], [1 x i8]* @.str2, i32 0, i32 0)) #0
  call void (...) @_ssdm_SpecStream([436 x [1024 x float]]* @_ZZ12optical_flowPA1024_7ap_uintILi64EEPA1024_10velocity_tE8frame3_a, i32 0, i32 10, i8* getelementptr inbounds ([1 x i8], [1 x i8]* @.str2, i32 0, i32 0)) #0
  call void (...) @_ssdm_SpecStream([436 x [1024 x float]]* @_ZZ12optical_flowPA1024_7ap_uintILi64EEPA1024_10velocity_tE8frame3_b, i32 0, i32 10, i8* getelementptr inbounds ([1 x i8], [1 x i8]* @.str2, i32 0, i32 0)) #0
  %14 = load i8, i8* @_ZGVZ12optical_flowPA1024_7ap_uintILi64EEPA1024_10velocity_tE3buf, align 1
  %15 = icmp eq i8 %14, 0
  br i1 %15, label %16, label %17

; <label>:16:                                     ; preds = %0
  call void @_ZN7ap_uintILi64EEC1Ev(%struct.ap_uint.3* @_ZZ12optical_flowPA1024_7ap_uintILi64EEPA1024_10velocity_tE3buf)
  store i8 1, i8* @_ZGVZ12optical_flowPA1024_7ap_uintILi64EEPA1024_10velocity_tE3buf
  br label %17

; <label>:17:                                     ; preds = %16, %0
  br label %18

; <label>:18:                                     ; preds = %17
  store i32 0, i32* %r, align 4
  br label %19

; <label>:19:                                     ; preds = %94, %18
  %20 = load i32, i32* %r, align 4
  %21 = icmp slt i32 %20, 436
  br i1 %21, label %22, label %97

; <label>:22:                                     ; preds = %19
  call void (...) @_ssdm_op_SpecLoopName(i8* getelementptr inbounds ([16 x i8], [16 x i8]* @.str31, i32 0, i32 0)) #0
  call void (...) @_ssdm_RegionBegin(i8* getelementptr inbounds ([16 x i8], [16 x i8]* @.str31, i32 0, i32 0)) #0
  br label %23

; <label>:23:                                     ; preds = %22
  store i32 0, i32* %c, align 4
  br label %24

; <label>:24:                                     ; preds = %90, %23
  %25 = load i32, i32* %c, align 4
  %26 = icmp slt i32 %25, 1024
  br i1 %26, label %27, label %93

; <label>:27:                                     ; preds = %24
  call void (...) @_ssdm_op_SpecLoopName(i8* getelementptr inbounds ([16 x i8], [16 x i8]* @.str32, i32 0, i32 0)) #0
  call void (...) @_ssdm_RegionBegin(i8* getelementptr inbounds ([16 x i8], [16 x i8]* @.str32, i32 0, i32 0)) #0
  call void (...) @_ssdm_op_SpecPipeline(i32 1, i32 1, i32 1, i32 0, i8* getelementptr inbounds ([1 x i8], [1 x i8]* @.str2, i32 0, i32 0)) #0
  %28 = load i32, i32* %c, align 4
  %29 = sext i32 %28 to i64
  %30 = load i32, i32* %r, align 4
  %31 = sext i32 %30 to i64
  %32 = load [1024 x %struct.ap_uint.3]*, [1024 x %struct.ap_uint.3]** %1, align 8
  %33 = getelementptr inbounds [1024 x %struct.ap_uint.3], [1024 x %struct.ap_uint.3]* %32, i64 %31
  %34 = getelementptr inbounds [1024 x %struct.ap_uint.3], [1024 x %struct.ap_uint.3]* %33, i32 0, i64 %29
  %35 = call %struct.ap_uint.3* @_ZN7ap_uintILi64EEaSERKS0_(%struct.ap_uint.3* @_ZZ12optical_flowPA1024_7ap_uintILi64EEPA1024_10velocity_tE3buf, %struct.ap_uint.3* %34)
  call void @_ZN11ap_int_baseILi64ELb0ELb1EEclEii(%struct.ap_range_ref* sret %3, %struct.ap_int_base.4* getelementptr inbounds (%struct.ap_uint.3, %struct.ap_uint.3* @_ZZ12optical_flowPA1024_7ap_uintILi64EEPA1024_10velocity_tE3buf, i32 0, i32 0), i32 7, i32 0)
  %36 = call i64 @_ZNK12ap_range_refILi64ELb0EEcvyEv(%struct.ap_range_ref* %3)
  %37 = uitofp i64 %36 to float
  %38 = fdiv float %37, 2.550000e+02
  %39 = load i32, i32* %c, align 4
  %40 = sext i32 %39 to i64
  %41 = load i32, i32* %r, align 4
  %42 = sext i32 %41 to i64
  %43 = getelementptr inbounds [436 x [1024 x float]], [436 x [1024 x float]]* @_ZZ12optical_flowPA1024_7ap_uintILi64EEPA1024_10velocity_tE8frame1_a, i32 0, i64 %42
  %44 = getelementptr inbounds [1024 x float], [1024 x float]* %43, i32 0, i64 %40
  store float %38, float* %44, align 4
  call void @_ZN11ap_int_baseILi64ELb0ELb1EEclEii(%struct.ap_range_ref* sret %4, %struct.ap_int_base.4* getelementptr inbounds (%struct.ap_uint.3, %struct.ap_uint.3* @_ZZ12optical_flowPA1024_7ap_uintILi64EEPA1024_10velocity_tE3buf, i32 0, i32 0), i32 15, i32 8)
  %45 = call i64 @_ZNK12ap_range_refILi64ELb0EEcvyEv(%struct.ap_range_ref* %4)
  %46 = uitofp i64 %45 to float
  %47 = fdiv float %46, 2.550000e+02
  %48 = load i32, i32* %c, align 4
  %49 = sext i32 %48 to i64
  %50 = load i32, i32* %r, align 4
  %51 = sext i32 %50 to i64
  %52 = getelementptr inbounds [436 x [1024 x float]], [436 x [1024 x float]]* @_ZZ12optical_flowPA1024_7ap_uintILi64EEPA1024_10velocity_tE8frame2_a, i32 0, i64 %51
  %53 = getelementptr inbounds [1024 x float], [1024 x float]* %52, i32 0, i64 %49
  store float %47, float* %53, align 4
  call void @_ZN11ap_int_baseILi64ELb0ELb1EEclEii(%struct.ap_range_ref* sret %5, %struct.ap_int_base.4* getelementptr inbounds (%struct.ap_uint.3, %struct.ap_uint.3* @_ZZ12optical_flowPA1024_7ap_uintILi64EEPA1024_10velocity_tE3buf, i32 0, i32 0), i32 23, i32 16)
  %54 = call i64 @_ZNK12ap_range_refILi64ELb0EEcvyEv(%struct.ap_range_ref* %5)
  %55 = uitofp i64 %54 to float
  %56 = fdiv float %55, 2.550000e+02
  %57 = load i32, i32* %c, align 4
  %58 = sext i32 %57 to i64
  %59 = load i32, i32* %r, align 4
  %60 = sext i32 %59 to i64
  %61 = getelementptr inbounds [436 x [1024 x float]], [436 x [1024 x float]]* @_ZZ12optical_flowPA1024_7ap_uintILi64EEPA1024_10velocity_tE8frame3_a, i32 0, i64 %60
  %62 = getelementptr inbounds [1024 x float], [1024 x float]* %61, i32 0, i64 %58
  store float %56, float* %62, align 4
  call void @_ZN11ap_int_baseILi64ELb0ELb1EEclEii(%struct.ap_range_ref* sret %6, %struct.ap_int_base.4* getelementptr inbounds (%struct.ap_uint.3, %struct.ap_uint.3* @_ZZ12optical_flowPA1024_7ap_uintILi64EEPA1024_10velocity_tE3buf, i32 0, i32 0), i32 23, i32 16)
  %63 = call i64 @_ZNK12ap_range_refILi64ELb0EEcvyEv(%struct.ap_range_ref* %6)
  %64 = uitofp i64 %63 to float
  %65 = fdiv float %64, 2.550000e+02
  %66 = load i32, i32* %c, align 4
  %67 = sext i32 %66 to i64
  %68 = load i32, i32* %r, align 4
  %69 = sext i32 %68 to i64
  %70 = getelementptr inbounds [436 x [1024 x float]], [436 x [1024 x float]]* @_ZZ12optical_flowPA1024_7ap_uintILi64EEPA1024_10velocity_tE8frame3_b, i32 0, i64 %69
  %71 = getelementptr inbounds [1024 x float], [1024 x float]* %70, i32 0, i64 %67
  store float %65, float* %71, align 4
  call void @_ZN11ap_int_baseILi64ELb0ELb1EEclEii(%struct.ap_range_ref* sret %7, %struct.ap_int_base.4* getelementptr inbounds (%struct.ap_uint.3, %struct.ap_uint.3* @_ZZ12optical_flowPA1024_7ap_uintILi64EEPA1024_10velocity_tE3buf, i32 0, i32 0), i32 31, i32 24)
  %72 = call i64 @_ZNK12ap_range_refILi64ELb0EEcvyEv(%struct.ap_range_ref* %7)
  %73 = uitofp i64 %72 to float
  %74 = fdiv float %73, 2.550000e+02
  %75 = load i32, i32* %c, align 4
  %76 = sext i32 %75 to i64
  %77 = load i32, i32* %r, align 4
  %78 = sext i32 %77 to i64
  %79 = getelementptr inbounds [436 x [1024 x float]], [436 x [1024 x float]]* @_ZZ12optical_flowPA1024_7ap_uintILi64EEPA1024_10velocity_tE8frame4_a, i32 0, i64 %78
  %80 = getelementptr inbounds [1024 x float], [1024 x float]* %79, i32 0, i64 %76
  store float %74, float* %80, align 4
  call void @_ZN11ap_int_baseILi64ELb0ELb1EEclEii(%struct.ap_range_ref* sret %8, %struct.ap_int_base.4* getelementptr inbounds (%struct.ap_uint.3, %struct.ap_uint.3* @_ZZ12optical_flowPA1024_7ap_uintILi64EEPA1024_10velocity_tE3buf, i32 0, i32 0), i32 39, i32 32)
  %81 = call i64 @_ZNK12ap_range_refILi64ELb0EEcvyEv(%struct.ap_range_ref* %8)
  %82 = uitofp i64 %81 to float
  %83 = fdiv float %82, 2.550000e+02
  %84 = load i32, i32* %c, align 4
  %85 = sext i32 %84 to i64
  %86 = load i32, i32* %r, align 4
  %87 = sext i32 %86 to i64
  %88 = getelementptr inbounds [436 x [1024 x float]], [436 x [1024 x float]]* @_ZZ12optical_flowPA1024_7ap_uintILi64EEPA1024_10velocity_tE8frame5_a, i32 0, i64 %87
  %89 = getelementptr inbounds [1024 x float], [1024 x float]* %88, i32 0, i64 %85
  store float %83, float* %89, align 4
  call void (...) @_ssdm_RegionEnd(i8* getelementptr inbounds ([16 x i8], [16 x i8]* @.str32, i32 0, i32 0)) #0
  br label %90

; <label>:90:                                     ; preds = %27
  %91 = load i32, i32* %c, align 4
  %92 = add nsw i32 %91, 1
  store i32 %92, i32* %c, align 4
  br label %24

; <label>:93:                                     ; preds = %24
  call void (...) @_ssdm_RegionEnd(i8* getelementptr inbounds ([16 x i8], [16 x i8]* @.str31, i32 0, i32 0)) #0
  br label %94

; <label>:94:                                     ; preds = %93
  %95 = load i32, i32* %r, align 4
  %96 = add nsw i32 %95, 1
  store i32 %96, i32* %r, align 4
  br label %19

; <label>:97:                                     ; preds = %19
  call void @_Z16gradient_xy_calcPA1024_fS0_S0_([1024 x float]* getelementptr inbounds ([436 x [1024 x float]], [436 x [1024 x float]]* @_ZZ12optical_flowPA1024_7ap_uintILi64EEPA1024_10velocity_tE8frame3_a, i32 0, i32 0), [1024 x float]* getelementptr inbounds ([436 x [1024 x float]], [436 x [1024 x float]]* @_ZZ12optical_flowPA1024_7ap_uintILi64EEPA1024_10velocity_tE10gradient_x, i32 0, i32 0), [1024 x float]* getelementptr inbounds ([436 x [1024 x float]], [436 x [1024 x float]]* @_ZZ12optical_flowPA1024_7ap_uintILi64EEPA1024_10velocity_tE10gradient_y, i32 0, i32 0))
  call void @_Z15gradient_z_calcPA1024_fS0_S0_S0_S0_S0_([1024 x float]* getelementptr inbounds ([436 x [1024 x float]], [436 x [1024 x float]]* @_ZZ12optical_flowPA1024_7ap_uintILi64EEPA1024_10velocity_tE8frame1_a, i32 0, i32 0), [1024 x float]* getelementptr inbounds ([436 x [1024 x float]], [436 x [1024 x float]]* @_ZZ12optical_flowPA1024_7ap_uintILi64EEPA1024_10velocity_tE8frame2_a, i32 0, i32 0), [1024 x float]* getelementptr inbounds ([436 x [1024 x float]], [436 x [1024 x float]]* @_ZZ12optical_flowPA1024_7ap_uintILi64EEPA1024_10velocity_tE8frame3_b, i32 0, i32 0), [1024 x float]* getelementptr inbounds ([436 x [1024 x float]], [436 x [1024 x float]]* @_ZZ12optical_flowPA1024_7ap_uintILi64EEPA1024_10velocity_tE8frame4_a, i32 0, i32 0), [1024 x float]* getelementptr inbounds ([436 x [1024 x float]], [436 x [1024 x float]]* @_ZZ12optical_flowPA1024_7ap_uintILi64EEPA1024_10velocity_tE8frame5_a, i32 0, i32 0), [1024 x float]* getelementptr inbounds ([436 x [1024 x float]], [436 x [1024 x float]]* @_ZZ12optical_flowPA1024_7ap_uintILi64EEPA1024_10velocity_tE10gradient_z, i32 0, i32 0))
  call void @_Z17gradient_weight_yPA1024_fS0_S0_PA1024_10gradient_t([1024 x float]* getelementptr inbounds ([436 x [1024 x float]], [436 x [1024 x float]]* @_ZZ12optical_flowPA1024_7ap_uintILi64EEPA1024_10velocity_tE10gradient_x, i32 0, i32 0), [1024 x float]* getelementptr inbounds ([436 x [1024 x float]], [436 x [1024 x float]]* @_ZZ12optical_flowPA1024_7ap_uintILi64EEPA1024_10velocity_tE10gradient_y, i32 0, i32 0), [1024 x float]* getelementptr inbounds ([436 x [1024 x float]], [436 x [1024 x float]]* @_ZZ12optical_flowPA1024_7ap_uintILi64EEPA1024_10velocity_tE10gradient_z, i32 0, i32 0), [1024 x %struct.gradient_t]* getelementptr inbounds ([436 x [1024 x %struct.gradient_t]], [436 x [1024 x %struct.gradient_t]]* @_ZZ12optical_flowPA1024_7ap_uintILi64EEPA1024_10velocity_tE10y_filtered, i32 0, i32 0))
  call void @_Z17gradient_weight_xPA1024_10gradient_tS1_([1024 x %struct.gradient_t]* getelementptr inbounds ([436 x [1024 x %struct.gradient_t]], [436 x [1024 x %struct.gradient_t]]* @_ZZ12optical_flowPA1024_7ap_uintILi64EEPA1024_10velocity_tE10y_filtered, i32 0, i32 0), [1024 x %struct.gradient_t]* getelementptr inbounds ([436 x [1024 x %struct.gradient_t]], [436 x [1024 x %struct.gradient_t]]* @_ZZ12optical_flowPA1024_7ap_uintILi64EEPA1024_10velocity_tE17filtered_gradient, i32 0, i32 0))
  call void @_Z13outer_productPA1024_10gradient_tPA1024_7outer_t([1024 x %struct.gradient_t]* getelementptr inbounds ([436 x [1024 x %struct.gradient_t]], [436 x [1024 x %struct.gradient_t]]* @_ZZ12optical_flowPA1024_7ap_uintILi64EEPA1024_10velocity_tE17filtered_gradient, i32 0, i32 0), [1024 x %struct.outer_t]* getelementptr inbounds ([436 x [1024 x %struct.outer_t]], [436 x [1024 x %struct.outer_t]]* @_ZZ12optical_flowPA1024_7ap_uintILi64EEPA1024_10velocity_tE11out_product, i32 0, i32 0))
  call void @_Z15tensor_weight_yPA1024_7outer_tPA1024_8tensor_t([1024 x %struct.outer_t]* getelementptr inbounds ([436 x [1024 x %struct.outer_t]], [436 x [1024 x %struct.outer_t]]* @_ZZ12optical_flowPA1024_7ap_uintILi64EEPA1024_10velocity_tE11out_product, i32 0, i32 0), [1024 x %struct.tensor_t]* getelementptr inbounds ([436 x [1024 x %struct.tensor_t]], [436 x [1024 x %struct.tensor_t]]* @_ZZ12optical_flowPA1024_7ap_uintILi64EEPA1024_10velocity_tE8tensor_y, i32 0, i32 0))
  call void @_Z15tensor_weight_xPA1024_8tensor_tS1_([1024 x %struct.tensor_t]* getelementptr inbounds ([436 x [1024 x %struct.tensor_t]], [436 x [1024 x %struct.tensor_t]]* @_ZZ12optical_flowPA1024_7ap_uintILi64EEPA1024_10velocity_tE8tensor_y, i32 0, i32 0), [1024 x %struct.tensor_t]* getelementptr inbounds ([436 x [1024 x %struct.tensor_t]], [436 x [1024 x %struct.tensor_t]]* @_ZZ12optical_flowPA1024_7ap_uintILi64EEPA1024_10velocity_tE6tensor, i32 0, i32 0))
  %98 = load [1024 x %struct.velocity_t]*, [1024 x %struct.velocity_t]** %2, align 8
  call void @_Z9flow_calcPA1024_8tensor_tPA1024_10velocity_t([1024 x %struct.tensor_t]* getelementptr inbounds ([436 x [1024 x %struct.tensor_t]], [436 x [1024 x %struct.tensor_t]]* @_ZZ12optical_flowPA1024_7ap_uintILi64EEPA1024_10velocity_tE6tensor, i32 0, i32 0), [1024 x %struct.velocity_t]* %98)
  ret void
}

; Function Attrs: nounwind
declare void @_ssdm_SpecStream(...) #0

; Function Attrs: nounwind
declare void @_ssdm_op_SpecDataflowPipeline(...) #0

; Function Attrs: alwaysinline inlinehint nounwind uwtable
define linkonce_odr void @_ZN7ap_uintILi64EEC1Ev(%struct.ap_uint.3* %this) unnamed_addr #1 comdat align 2 {
  %1 = alloca %struct.ap_uint.3*, align 8
  store %struct.ap_uint.3* %this, %struct.ap_uint.3** %1, align 8
  %2 = load %struct.ap_uint.3*, %struct.ap_uint.3** %1
  call void @_ZN7ap_uintILi64EEC2Ev(%struct.ap_uint.3* %2)
  ret void
}

; Function Attrs: alwaysinline inlinehint nounwind uwtable
define linkonce_odr %struct.ap_uint.3* @_ZN7ap_uintILi64EEaSERKS0_(%struct.ap_uint.3* %this, %struct.ap_uint.3* %op2) #1 comdat align 2 {
  %1 = alloca %struct.ap_uint.3*, align 8
  %2 = alloca %struct.ap_uint.3*, align 8
  store %struct.ap_uint.3* %this, %struct.ap_uint.3** %1, align 8
  store %struct.ap_uint.3* %op2, %struct.ap_uint.3** %2, align 8
  %3 = load %struct.ap_uint.3*, %struct.ap_uint.3** %1
  %4 = load %struct.ap_uint.3*, %struct.ap_uint.3** %2, align 8
  %5 = bitcast %struct.ap_uint.3* %4 to %struct.ssdm_int.5*
  %6 = getelementptr inbounds %struct.ssdm_int.5, %struct.ssdm_int.5* %5, i32 0, i32 0
  %7 = load i64, i64* %6, align 8
  %8 = bitcast %struct.ap_uint.3* %3 to %struct.ap_int_base.4*
  %9 = bitcast %struct.ap_int_base.4* %8 to %struct.ssdm_int.5*
  %10 = getelementptr inbounds %struct.ssdm_int.5, %struct.ssdm_int.5* %9, i32 0, i32 0
  store i64 %7, i64* %10, align 8
  ret %struct.ap_uint.3* %3
}

; Function Attrs: alwaysinline inlinehint nounwind uwtable
define linkonce_odr void @_ZN11ap_int_baseILi64ELb0ELb1EEclEii(%struct.ap_range_ref* noalias sret %agg.result, %struct.ap_int_base.4* %this, i32 %Hi, i32 %Lo) #1 comdat align 2 {
  %1 = alloca %struct.ap_int_base.4*, align 8
  %2 = alloca i32, align 4
  %3 = alloca i32, align 4
  store %struct.ap_int_base.4* %this, %struct.ap_int_base.4** %1, align 8
  store i32 %Hi, i32* %2, align 4
  store i32 %Lo, i32* %3, align 4
  %4 = load %struct.ap_int_base.4*, %struct.ap_int_base.4** %1
  %5 = load i32, i32* %2, align 4
  %6 = load i32, i32* %3, align 4
  call void @_ZN12ap_range_refILi64ELb0EEC1EP11ap_int_baseILi64ELb0ELb1EEii(%struct.ap_range_ref* %agg.result, %struct.ap_int_base.4* %4, i32 %5, i32 %6)
  ret void
}

; Function Attrs: alwaysinline inlinehint nounwind uwtable
define linkonce_odr i64 @_ZNK12ap_range_refILi64ELb0EEcvyEv(%struct.ap_range_ref* %this) #1 comdat align 2 {
  %1 = alloca %struct.ap_range_ref*, align 8
  store %struct.ap_range_ref* %this, %struct.ap_range_ref** %1, align 8
  %2 = load %struct.ap_range_ref*, %struct.ap_range_ref** %1
  %3 = call i64 @_ZNK12ap_range_refILi64ELb0EE9to_uint64Ev(%struct.ap_range_ref* %2)
  ret i64 %3
}

; Function Attrs: alwaysinline inlinehint nounwind uwtable
define linkonce_odr i64 @_ZNK12ap_range_refILi64ELb0EE9to_uint64Ev(%struct.ap_range_ref* %this) #1 comdat align 2 {
  %1 = alloca %struct.ap_range_ref*, align 8
  %__Result__ = alloca i64, align 8
  %__Val2__ = alloca i64, align 8
  store %struct.ap_range_ref* %this, %struct.ap_range_ref** %1, align 8
  %2 = load %struct.ap_range_ref*, %struct.ap_range_ref** %1
  store i64 0, i64* %__Result__, align 8
  %3 = getelementptr inbounds %struct.ap_range_ref, %struct.ap_range_ref* %2, i32 0, i32 0
  %4 = load %struct.ap_int_base.4*, %struct.ap_int_base.4** %3, align 8
  %5 = bitcast %struct.ap_int_base.4* %4 to %struct.ssdm_int.5*
  %6 = getelementptr inbounds %struct.ssdm_int.5, %struct.ssdm_int.5* %5, i32 0, i32 0
  %7 = load i64, i64* %6, align 8
  store i64 %7, i64* %__Val2__, align 8
  %8 = bitcast i64* %__Result__ to i8*
  %9 = bitcast i64* %__Val2__ to i8*
  %10 = getelementptr inbounds %struct.ap_range_ref, %struct.ap_range_ref* %2, i32 0, i32 1
  %11 = load i32, i32* %10, align 4
  %12 = getelementptr inbounds %struct.ap_range_ref, %struct.ap_range_ref* %2, i32 0, i32 2
  %13 = load i32, i32* %12, align 4
  %14 = load i64, i64* %__Val2__
  %15 = call i64 @llvm.part.select.i64(i64 %14, i32 %11, i32 %13)
  store i64 %15, i64* %__Result__
  %16 = load i64, i64* %__Result__, align 8
  ret i64 %16
}

; Function Attrs: nounwind readnone
declare i64 @llvm.part.select.i64(i64, i32, i32) #4

; Function Attrs: nounwind
declare void @_ssdm_InlineSelf(...) #0

; Function Attrs: noreturn nounwind
declare void @__assert_fail(i8*, i8*, i32, i8*) #5

; Function Attrs: alwaysinline inlinehint nounwind uwtable
define linkonce_odr void @_ZN7ap_uintILi32EEC1Ev(%struct.ap_uint.15* %this) unnamed_addr #1 comdat align 2 {
  %1 = alloca %struct.ap_uint.15*, align 8
  store %struct.ap_uint.15* %this, %struct.ap_uint.15** %1, align 8
  %2 = load %struct.ap_uint.15*, %struct.ap_uint.15** %1
  call void @_ZN7ap_uintILi32EEC2Ev(%struct.ap_uint.15* %2)
  ret void
}

; Function Attrs: alwaysinline inlinehint nounwind uwtable
define linkonce_odr %struct.ap_uint.15* @_ZN7ap_uintILi32EEaSERKS0_(%struct.ap_uint.15* %this, %struct.ap_uint.15* %op2) #1 comdat align 2 {
  %1 = alloca %struct.ap_uint.15*, align 8
  %2 = alloca %struct.ap_uint.15*, align 8
  store %struct.ap_uint.15* %this, %struct.ap_uint.15** %1, align 8
  store %struct.ap_uint.15* %op2, %struct.ap_uint.15** %2, align 8
  %3 = load %struct.ap_uint.15*, %struct.ap_uint.15** %1
  %4 = load %struct.ap_uint.15*, %struct.ap_uint.15** %2, align 8
  %5 = bitcast %struct.ap_uint.15* %4 to %struct.ssdm_int.17*
  %6 = getelementptr inbounds %struct.ssdm_int.17, %struct.ssdm_int.17* %5, i32 0, i32 0
  %7 = load i32, i32* %6, align 4
  %8 = bitcast %struct.ap_uint.15* %3 to %struct.ap_int_base.16*
  %9 = bitcast %struct.ap_int_base.16* %8 to %struct.ssdm_int.17*
  %10 = getelementptr inbounds %struct.ssdm_int.17, %struct.ssdm_int.17* %9, i32 0, i32 0
  store i32 %7, i32* %10, align 4
  ret %struct.ap_uint.15* %3
}

; Function Attrs: alwaysinline inlinehint nounwind uwtable
define linkonce_odr void @_ZN7ap_uintILi32EEC1Ei(%struct.ap_uint.15* %this, i32 %val) unnamed_addr #1 comdat align 2 {
  %1 = alloca %struct.ap_uint.15*, align 8
  %2 = alloca i32, align 4
  store %struct.ap_uint.15* %this, %struct.ap_uint.15** %1, align 8
  store i32 %val, i32* %2, align 4
  %3 = load %struct.ap_uint.15*, %struct.ap_uint.15** %1
  %4 = load i32, i32* %2, align 4
  call void @_ZN7ap_uintILi32EEC2Ei(%struct.ap_uint.15* %3, i32 %4)
  ret void
}

; Function Attrs: alwaysinline inlinehint nounwind uwtable
define linkonce_odr zeroext i1 @_ZltILi32ELb0EEbRK11ap_int_baseIXT_EXT0_EXleT_Li64EEEi(%struct.ap_int_base.16* %op, i32 %op2) #1 comdat {
  %1 = alloca %struct.ap_int_base.16*, align 8
  %2 = alloca i32, align 4
  %3 = alloca %struct.ap_int_base.22, align 4
  store %struct.ap_int_base.16* %op, %struct.ap_int_base.16** %1, align 8
  store i32 %op2, i32* %2, align 4
  %4 = load %struct.ap_int_base.16*, %struct.ap_int_base.16** %1, align 8
  %5 = load i32, i32* %2, align 4
  call void @_ZN11ap_int_baseILi32ELb1ELb1EEC1Ei(%struct.ap_int_base.22* %3, i32 %5)
  %6 = call zeroext i1 @_ZNK11ap_int_baseILi32ELb0ELb1EEltILi32ELb1EEEbRKS_IXT_EXT0_EXleT_Li64EEE(%struct.ap_int_base.16* %4, %struct.ap_int_base.22* %3)
  ret i1 %6
}

; Function Attrs: alwaysinline inlinehint nounwind uwtable
define linkonce_odr i32 @_ZNK11ap_int_baseILi32ELb0ELb1EEcvjEv(%struct.ap_int_base.16* %this) #1 comdat align 2 {
  %1 = alloca %struct.ap_int_base.16*, align 8
  store %struct.ap_int_base.16* %this, %struct.ap_int_base.16** %1, align 8
  %2 = load %struct.ap_int_base.16*, %struct.ap_int_base.16** %1
  %3 = bitcast %struct.ap_int_base.16* %2 to %struct.ssdm_int.17*
  %4 = getelementptr inbounds %struct.ssdm_int.17, %struct.ssdm_int.17* %3, i32 0, i32 0
  %5 = load i32, i32* %4, align 4
  ret i32 %5
}

; Function Attrs: alwaysinline inlinehint nounwind uwtable
define linkonce_odr void @_ZplILi32ELb0EEN11ap_int_baseIXT_EXT0_EXleT_Li64EEE5RTypeIXLi32EEXLb1EEE4plusERKS1_i(%struct.ap_int_base.18* noalias sret %agg.result, %struct.ap_int_base.16* %op, i32 %i_op) #1 comdat {
  %1 = alloca %struct.ap_int_base.16*, align 8
  %2 = alloca i32, align 4
  %3 = alloca %struct.ap_int_base.22, align 4
  store %struct.ap_int_base.16* %op, %struct.ap_int_base.16** %1, align 8
  store i32 %i_op, i32* %2, align 4
  %4 = load %struct.ap_int_base.16*, %struct.ap_int_base.16** %1, align 8
  %5 = load i32, i32* %2, align 4
  call void @_ZN11ap_int_baseILi32ELb1ELb1EEC1Ei(%struct.ap_int_base.22* %3, i32 %5)
  call void @_ZplILi32ELb0ELi32ELb1EEN11ap_int_baseIXT_EXT0_EXleT_Li64EEE5RTypeIXT1_EXT2_EE4plusERKS1_RKS0_IXT1_EXT2_EXleT1_Li64EEE(%struct.ap_int_base.18* sret %agg.result, %struct.ap_int_base.16* %4, %struct.ap_int_base.22* %3)
  ret void
}

; Function Attrs: alwaysinline inlinehint nounwind uwtable
define linkonce_odr i64 @_ZNK11ap_int_baseILi34ELb1ELb1EEcvxEv(%struct.ap_int_base.18* %this) #1 comdat align 2 {
  %1 = alloca %struct.ap_int_base.18*, align 8
  store %struct.ap_int_base.18* %this, %struct.ap_int_base.18** %1, align 8
  %2 = load %struct.ap_int_base.18*, %struct.ap_int_base.18** %1
  %3 = bitcast %struct.ap_int_base.18* %2 to %struct.ssdm_int.19*
  %4 = getelementptr inbounds %struct.ssdm_int.19, %struct.ssdm_int.19* %3, i32 0, i32 0
  %5 = load i34, i34* %4, align 8
  %6 = sext i34 %5 to i64
  ret i64 %6
}

; Function Attrs: alwaysinline inlinehint nounwind uwtable
define linkonce_odr void @_ZN11ap_int_baseILi32ELb0ELb1EEppEi(%struct.ap_int_base.16* noalias sret %agg.result, %struct.ap_int_base.16* %this, i32) #1 comdat align 2 {
  %2 = alloca %struct.ap_int_base.16*, align 8
  %3 = alloca i32, align 4
  %4 = alloca %struct.ap_int_base.20, align 1
  store %struct.ap_int_base.16* %this, %struct.ap_int_base.16** %2, align 8
  store i32 %0, i32* %3, align 4
  %5 = load %struct.ap_int_base.16*, %struct.ap_int_base.16** %2
  %6 = bitcast %struct.ap_int_base.16* %agg.result to i8*
  %7 = bitcast %struct.ap_int_base.16* %5 to i8*
  %8 = getelementptr inbounds %struct.ap_int_base.16, %struct.ap_int_base.16* %agg.result, i32 0, i32 0
  %9 = getelementptr inbounds %struct.ap_int_base.16, %struct.ap_int_base.16* %5, i32 0, i32 0
  %10 = getelementptr inbounds %struct.ssdm_int.17, %struct.ssdm_int.17* %8, i32 0, i32 0
  %11 = getelementptr inbounds %struct.ssdm_int.17, %struct.ssdm_int.17* %9, i32 0, i32 0
  %12 = load i32, i32* %11
  store i32 %12, i32* %10
  call void @_ZN11ap_int_baseILi1ELb0ELb1EEC1Ei(%struct.ap_int_base.20* %4, i32 1)
  %13 = call %struct.ap_int_base.16* @_ZN11ap_int_baseILi32ELb0ELb1EEpLILi1ELb0EEERS0_RKS_IXT_EXT0_EXleT_Li64EEE(%struct.ap_int_base.16* %5, %struct.ap_int_base.20* %4)
  ret void
}

; Function Attrs: alwaysinline inlinehint nounwind uwtable
define linkonce_odr %struct.ap_int_base.16* @_ZN11ap_int_baseILi32ELb0ELb1EEpLILi1ELb0EEERS0_RKS_IXT_EXT0_EXleT_Li64EEE(%struct.ap_int_base.16* %this, %struct.ap_int_base.20* %op2) #1 comdat align 2 {
  %1 = alloca %struct.ap_int_base.16*, align 8
  %2 = alloca %struct.ap_int_base.20*, align 8
  store %struct.ap_int_base.16* %this, %struct.ap_int_base.16** %1, align 8
  store %struct.ap_int_base.20* %op2, %struct.ap_int_base.20** %2, align 8
  %3 = load %struct.ap_int_base.16*, %struct.ap_int_base.16** %1
  %4 = load %struct.ap_int_base.20*, %struct.ap_int_base.20** %2, align 8
  %5 = bitcast %struct.ap_int_base.20* %4 to %struct.ssdm_int.21*
  %6 = getelementptr inbounds %struct.ssdm_int.21, %struct.ssdm_int.21* %5, i32 0, i32 0
  %7 = load i1, i1* %6, align 1
  %8 = zext i1 %7 to i32
  %9 = bitcast %struct.ap_int_base.16* %3 to %struct.ssdm_int.17*
  %10 = getelementptr inbounds %struct.ssdm_int.17, %struct.ssdm_int.17* %9, i32 0, i32 0
  %11 = load i32, i32* %10, align 4
  %12 = add i32 %11, %8
  store i32 %12, i32* %10, align 4
  ret %struct.ap_int_base.16* %3
}

; Function Attrs: alwaysinline inlinehint nounwind uwtable
define linkonce_odr void @_ZN11ap_int_baseILi1ELb0ELb1EEC1Ei(%struct.ap_int_base.20* %this, i32 %op) unnamed_addr #1 comdat align 2 {
  %1 = alloca %struct.ap_int_base.20*, align 8
  %2 = alloca i32, align 4
  store %struct.ap_int_base.20* %this, %struct.ap_int_base.20** %1, align 8
  store i32 %op, i32* %2, align 4
  %3 = load %struct.ap_int_base.20*, %struct.ap_int_base.20** %1
  %4 = load i32, i32* %2, align 4
  call void @_ZN11ap_int_baseILi1ELb0ELb1EEC2Ei(%struct.ap_int_base.20* %3, i32 %4)
  ret void
}

; Function Attrs: alwaysinline inlinehint nounwind uwtable
define linkonce_odr void @_ZN11ap_int_baseILi1ELb0ELb1EEC2Ei(%struct.ap_int_base.20* %this, i32 %op) unnamed_addr #1 comdat align 2 {
  %1 = alloca %struct.ap_int_base.20*, align 8
  %2 = alloca i32, align 4
  store %struct.ap_int_base.20* %this, %struct.ap_int_base.20** %1, align 8
  store i32 %op, i32* %2, align 4
  %3 = load %struct.ap_int_base.20*, %struct.ap_int_base.20** %1
  %4 = bitcast %struct.ap_int_base.20* %3 to %struct.ssdm_int.21*
  call void @_ZN8ssdm_intILi1ELb0EEC2Ev(%struct.ssdm_int.21* %4)
  %5 = load i32, i32* %2, align 4
  %6 = trunc i32 %5 to i1
  %7 = bitcast %struct.ap_int_base.20* %3 to %struct.ssdm_int.21*
  %8 = getelementptr inbounds %struct.ssdm_int.21, %struct.ssdm_int.21* %7, i32 0, i32 0
  store i1 %6, i1* %8, align 1
  ret void
}

; Function Attrs: alwaysinline inlinehint nounwind uwtable
define linkonce_odr void @_ZN8ssdm_intILi1ELb0EEC2Ev(%struct.ssdm_int.21* %this) unnamed_addr #1 comdat align 2 {
  %1 = alloca %struct.ssdm_int.21*, align 8
  store %struct.ssdm_int.21* %this, %struct.ssdm_int.21** %1, align 8
  %2 = load %struct.ssdm_int.21*, %struct.ssdm_int.21** %1
  ret void
}

; Function Attrs: alwaysinline inlinehint nounwind uwtable
define linkonce_odr void @_ZplILi32ELb0ELi32ELb1EEN11ap_int_baseIXT_EXT0_EXleT_Li64EEE5RTypeIXT1_EXT2_EE4plusERKS1_RKS0_IXT1_EXT2_EXleT1_Li64EEE(%struct.ap_int_base.18* noalias sret %agg.result, %struct.ap_int_base.16* %op, %struct.ap_int_base.22* %op2) #1 comdat {
  %1 = alloca %struct.ap_int_base.16*, align 8
  %2 = alloca %struct.ap_int_base.22*, align 8
  %lhs = alloca %struct.ap_int_base.18, align 8
  %rhs = alloca %struct.ap_int_base.18, align 8
  store %struct.ap_int_base.16* %op, %struct.ap_int_base.16** %1, align 8
  store %struct.ap_int_base.22* %op2, %struct.ap_int_base.22** %2, align 8
  %3 = load %struct.ap_int_base.16*, %struct.ap_int_base.16** %1, align 8
  call void @_ZN11ap_int_baseILi34ELb1ELb1EEC1ILi32ELb0EEERKS_IXT_EXT0_EXleT_Li64EEE(%struct.ap_int_base.18* %lhs, %struct.ap_int_base.16* %3)
  %4 = load %struct.ap_int_base.22*, %struct.ap_int_base.22** %2, align 8
  call void @_ZN11ap_int_baseILi34ELb1ELb1EEC1ILi32ELb1EEERKS_IXT_EXT0_EXleT_Li64EEE(%struct.ap_int_base.18* %rhs, %struct.ap_int_base.22* %4)
  call void @_ZN11ap_int_baseILi34ELb1ELb1EEC1Ev(%struct.ap_int_base.18* %agg.result)
  %5 = bitcast %struct.ap_int_base.18* %lhs to %struct.ssdm_int.19*
  %6 = getelementptr inbounds %struct.ssdm_int.19, %struct.ssdm_int.19* %5, i32 0, i32 0
  %7 = load i34, i34* %6, align 8
  %8 = bitcast %struct.ap_int_base.18* %rhs to %struct.ssdm_int.19*
  %9 = getelementptr inbounds %struct.ssdm_int.19, %struct.ssdm_int.19* %8, i32 0, i32 0
  %10 = load i34, i34* %9, align 8
  %11 = add nsw i34 %7, %10
  %12 = bitcast %struct.ap_int_base.18* %agg.result to %struct.ssdm_int.19*
  %13 = getelementptr inbounds %struct.ssdm_int.19, %struct.ssdm_int.19* %12, i32 0, i32 0
  store i34 %11, i34* %13, align 8
  ret void
}

; Function Attrs: alwaysinline inlinehint nounwind uwtable
define linkonce_odr void @_ZN11ap_int_baseILi32ELb1ELb1EEC1Ei(%struct.ap_int_base.22* %this, i32 %op) unnamed_addr #1 comdat align 2 {
  %1 = alloca %struct.ap_int_base.22*, align 8
  %2 = alloca i32, align 4
  store %struct.ap_int_base.22* %this, %struct.ap_int_base.22** %1, align 8
  store i32 %op, i32* %2, align 4
  %3 = load %struct.ap_int_base.22*, %struct.ap_int_base.22** %1
  %4 = load i32, i32* %2, align 4
  call void @_ZN11ap_int_baseILi32ELb1ELb1EEC2Ei(%struct.ap_int_base.22* %3, i32 %4)
  ret void
}

; Function Attrs: alwaysinline inlinehint nounwind uwtable
define linkonce_odr void @_ZN11ap_int_baseILi32ELb1ELb1EEC2Ei(%struct.ap_int_base.22* %this, i32 %op) unnamed_addr #1 comdat align 2 {
  %1 = alloca %struct.ap_int_base.22*, align 8
  %2 = alloca i32, align 4
  store %struct.ap_int_base.22* %this, %struct.ap_int_base.22** %1, align 8
  store i32 %op, i32* %2, align 4
  %3 = load %struct.ap_int_base.22*, %struct.ap_int_base.22** %1
  %4 = bitcast %struct.ap_int_base.22* %3 to %struct.ssdm_int.23*
  call void @_ZN8ssdm_intILi32ELb1EEC2Ev(%struct.ssdm_int.23* %4)
  %5 = load i32, i32* %2, align 4
  %6 = bitcast %struct.ap_int_base.22* %3 to %struct.ssdm_int.23*
  %7 = getelementptr inbounds %struct.ssdm_int.23, %struct.ssdm_int.23* %6, i32 0, i32 0
  store i32 %5, i32* %7, align 4
  ret void
}

; Function Attrs: alwaysinline inlinehint nounwind uwtable
define linkonce_odr void @_ZN8ssdm_intILi32ELb1EEC2Ev(%struct.ssdm_int.23* %this) unnamed_addr #1 comdat align 2 {
  %1 = alloca %struct.ssdm_int.23*, align 8
  store %struct.ssdm_int.23* %this, %struct.ssdm_int.23** %1, align 8
  %2 = load %struct.ssdm_int.23*, %struct.ssdm_int.23** %1
  ret void
}

; Function Attrs: alwaysinline inlinehint nounwind uwtable
define linkonce_odr void @_ZN11ap_int_baseILi34ELb1ELb1EEC1ILi32ELb0EEERKS_IXT_EXT0_EXleT_Li64EEE(%struct.ap_int_base.18* %this, %struct.ap_int_base.16* %op) unnamed_addr #1 comdat align 2 {
  %1 = alloca %struct.ap_int_base.18*, align 8
  %2 = alloca %struct.ap_int_base.16*, align 8
  store %struct.ap_int_base.18* %this, %struct.ap_int_base.18** %1, align 8
  store %struct.ap_int_base.16* %op, %struct.ap_int_base.16** %2, align 8
  %3 = load %struct.ap_int_base.18*, %struct.ap_int_base.18** %1
  %4 = load %struct.ap_int_base.16*, %struct.ap_int_base.16** %2
  call void @_ZN11ap_int_baseILi34ELb1ELb1EEC2ILi32ELb0EEERKS_IXT_EXT0_EXleT_Li64EEE(%struct.ap_int_base.18* %3, %struct.ap_int_base.16* %4)
  ret void
}

; Function Attrs: alwaysinline inlinehint nounwind uwtable
define linkonce_odr void @_ZN11ap_int_baseILi34ELb1ELb1EEC1ILi32ELb1EEERKS_IXT_EXT0_EXleT_Li64EEE(%struct.ap_int_base.18* %this, %struct.ap_int_base.22* %op) unnamed_addr #1 comdat align 2 {
  %1 = alloca %struct.ap_int_base.18*, align 8
  %2 = alloca %struct.ap_int_base.22*, align 8
  store %struct.ap_int_base.18* %this, %struct.ap_int_base.18** %1, align 8
  store %struct.ap_int_base.22* %op, %struct.ap_int_base.22** %2, align 8
  %3 = load %struct.ap_int_base.18*, %struct.ap_int_base.18** %1
  %4 = load %struct.ap_int_base.22*, %struct.ap_int_base.22** %2
  call void @_ZN11ap_int_baseILi34ELb1ELb1EEC2ILi32ELb1EEERKS_IXT_EXT0_EXleT_Li64EEE(%struct.ap_int_base.18* %3, %struct.ap_int_base.22* %4)
  ret void
}

; Function Attrs: alwaysinline inlinehint nounwind uwtable
define linkonce_odr void @_ZN11ap_int_baseILi34ELb1ELb1EEC1Ev(%struct.ap_int_base.18* %this) unnamed_addr #1 comdat align 2 {
  %1 = alloca %struct.ap_int_base.18*, align 8
  store %struct.ap_int_base.18* %this, %struct.ap_int_base.18** %1, align 8
  %2 = load %struct.ap_int_base.18*, %struct.ap_int_base.18** %1
  call void @_ZN11ap_int_baseILi34ELb1ELb1EEC2Ev(%struct.ap_int_base.18* %2)
  ret void
}

; Function Attrs: alwaysinline inlinehint nounwind uwtable
define linkonce_odr void @_ZN11ap_int_baseILi34ELb1ELb1EEC2Ev(%struct.ap_int_base.18* %this) unnamed_addr #1 comdat align 2 {
  %1 = alloca %struct.ap_int_base.18*, align 8
  store %struct.ap_int_base.18* %this, %struct.ap_int_base.18** %1, align 8
  %2 = load %struct.ap_int_base.18*, %struct.ap_int_base.18** %1
  %3 = bitcast %struct.ap_int_base.18* %2 to %struct.ssdm_int.19*
  call void @_ZN8ssdm_intILi34ELb1EEC2Ev(%struct.ssdm_int.19* %3)
  ret void
}

; Function Attrs: alwaysinline inlinehint nounwind uwtable
define linkonce_odr void @_ZN8ssdm_intILi34ELb1EEC2Ev(%struct.ssdm_int.19* %this) unnamed_addr #1 comdat align 2 {
  %1 = alloca %struct.ssdm_int.19*, align 8
  store %struct.ssdm_int.19* %this, %struct.ssdm_int.19** %1, align 8
  %2 = load %struct.ssdm_int.19*, %struct.ssdm_int.19** %1
  ret void
}

; Function Attrs: alwaysinline inlinehint nounwind uwtable
define linkonce_odr void @_ZN11ap_int_baseILi34ELb1ELb1EEC2ILi32ELb1EEERKS_IXT_EXT0_EXleT_Li64EEE(%struct.ap_int_base.18* %this, %struct.ap_int_base.22* %op) unnamed_addr #1 comdat align 2 {
  %1 = alloca %struct.ap_int_base.18*, align 8
  %2 = alloca %struct.ap_int_base.22*, align 8
  store %struct.ap_int_base.18* %this, %struct.ap_int_base.18** %1, align 8
  store %struct.ap_int_base.22* %op, %struct.ap_int_base.22** %2, align 8
  %3 = load %struct.ap_int_base.18*, %struct.ap_int_base.18** %1
  %4 = bitcast %struct.ap_int_base.18* %3 to %struct.ssdm_int.19*
  call void @_ZN8ssdm_intILi34ELb1EEC2Ev(%struct.ssdm_int.19* %4)
  %5 = load %struct.ap_int_base.22*, %struct.ap_int_base.22** %2, align 8
  %6 = bitcast %struct.ap_int_base.22* %5 to %struct.ssdm_int.23*
  %7 = getelementptr inbounds %struct.ssdm_int.23, %struct.ssdm_int.23* %6, i32 0, i32 0
  %8 = load i32, i32* %7, align 4
  %9 = sext i32 %8 to i34
  %10 = bitcast %struct.ap_int_base.18* %3 to %struct.ssdm_int.19*
  %11 = getelementptr inbounds %struct.ssdm_int.19, %struct.ssdm_int.19* %10, i32 0, i32 0
  store i34 %9, i34* %11, align 8
  ret void
}

; Function Attrs: alwaysinline inlinehint nounwind uwtable
define linkonce_odr void @_ZN11ap_int_baseILi34ELb1ELb1EEC2ILi32ELb0EEERKS_IXT_EXT0_EXleT_Li64EEE(%struct.ap_int_base.18* %this, %struct.ap_int_base.16* %op) unnamed_addr #1 comdat align 2 {
  %1 = alloca %struct.ap_int_base.18*, align 8
  %2 = alloca %struct.ap_int_base.16*, align 8
  store %struct.ap_int_base.18* %this, %struct.ap_int_base.18** %1, align 8
  store %struct.ap_int_base.16* %op, %struct.ap_int_base.16** %2, align 8
  %3 = load %struct.ap_int_base.18*, %struct.ap_int_base.18** %1
  %4 = bitcast %struct.ap_int_base.18* %3 to %struct.ssdm_int.19*
  call void @_ZN8ssdm_intILi34ELb1EEC2Ev(%struct.ssdm_int.19* %4)
  %5 = load %struct.ap_int_base.16*, %struct.ap_int_base.16** %2, align 8
  %6 = bitcast %struct.ap_int_base.16* %5 to %struct.ssdm_int.17*
  %7 = getelementptr inbounds %struct.ssdm_int.17, %struct.ssdm_int.17* %6, i32 0, i32 0
  %8 = load i32, i32* %7, align 4
  %9 = zext i32 %8 to i34
  %10 = bitcast %struct.ap_int_base.18* %3 to %struct.ssdm_int.19*
  %11 = getelementptr inbounds %struct.ssdm_int.19, %struct.ssdm_int.19* %10, i32 0, i32 0
  store i34 %9, i34* %11, align 8
  ret void
}

; Function Attrs: alwaysinline inlinehint nounwind uwtable
define linkonce_odr zeroext i1 @_ZNK11ap_int_baseILi32ELb0ELb1EEltILi32ELb1EEEbRKS_IXT_EXT0_EXleT_Li64EEE(%struct.ap_int_base.16* %this, %struct.ap_int_base.22* %op2) #1 comdat align 2 {
  %1 = alloca %struct.ap_int_base.16*, align 8
  %2 = alloca %struct.ap_int_base.22*, align 8
  store %struct.ap_int_base.16* %this, %struct.ap_int_base.16** %1, align 8
  store %struct.ap_int_base.22* %op2, %struct.ap_int_base.22** %2, align 8
  %3 = load %struct.ap_int_base.16*, %struct.ap_int_base.16** %1
  %4 = bitcast %struct.ap_int_base.16* %3 to %struct.ssdm_int.17*
  %5 = getelementptr inbounds %struct.ssdm_int.17, %struct.ssdm_int.17* %4, i32 0, i32 0
  %6 = load i32, i32* %5, align 4
  %7 = load %struct.ap_int_base.22*, %struct.ap_int_base.22** %2, align 8
  %8 = bitcast %struct.ap_int_base.22* %7 to %struct.ssdm_int.23*
  %9 = getelementptr inbounds %struct.ssdm_int.23, %struct.ssdm_int.23* %8, i32 0, i32 0
  %10 = load i32, i32* %9, align 4
  %11 = icmp ult i32 %6, %10
  ret i1 %11
}

; Function Attrs: alwaysinline inlinehint nounwind uwtable
define linkonce_odr void @_ZN7ap_uintILi32EEC2Ei(%struct.ap_uint.15* %this, i32 %val) unnamed_addr #1 comdat align 2 {
  %1 = alloca %struct.ap_uint.15*, align 8
  %2 = alloca i32, align 4
  store %struct.ap_uint.15* %this, %struct.ap_uint.15** %1, align 8
  store i32 %val, i32* %2, align 4
  %3 = load %struct.ap_uint.15*, %struct.ap_uint.15** %1
  %4 = bitcast %struct.ap_uint.15* %3 to %struct.ap_int_base.16*
  call void @_ZN11ap_int_baseILi32ELb0ELb1EEC2Ev(%struct.ap_int_base.16* %4)
  %5 = load i32, i32* %2, align 4
  %6 = bitcast %struct.ap_uint.15* %3 to %struct.ap_int_base.16*
  %7 = bitcast %struct.ap_int_base.16* %6 to %struct.ssdm_int.17*
  %8 = getelementptr inbounds %struct.ssdm_int.17, %struct.ssdm_int.17* %7, i32 0, i32 0
  store i32 %5, i32* %8, align 4
  ret void
}

; Function Attrs: alwaysinline inlinehint nounwind uwtable
define linkonce_odr void @_ZN11ap_int_baseILi32ELb0ELb1EEC2Ev(%struct.ap_int_base.16* %this) unnamed_addr #1 comdat align 2 {
  %1 = alloca %struct.ap_int_base.16*, align 8
  store %struct.ap_int_base.16* %this, %struct.ap_int_base.16** %1, align 8
  %2 = load %struct.ap_int_base.16*, %struct.ap_int_base.16** %1
  %3 = bitcast %struct.ap_int_base.16* %2 to %struct.ssdm_int.17*
  call void @_ZN8ssdm_intILi32ELb0EEC2Ev(%struct.ssdm_int.17* %3)
  ret void
}

; Function Attrs: alwaysinline inlinehint nounwind uwtable
define linkonce_odr void @_ZN8ssdm_intILi32ELb0EEC2Ev(%struct.ssdm_int.17* %this) unnamed_addr #1 comdat align 2 {
  %1 = alloca %struct.ssdm_int.17*, align 8
  store %struct.ssdm_int.17* %this, %struct.ssdm_int.17** %1, align 8
  %2 = load %struct.ssdm_int.17*, %struct.ssdm_int.17** %1
  ret void
}

; Function Attrs: alwaysinline inlinehint nounwind uwtable
define linkonce_odr void @_ZN7ap_uintILi32EEC2Ev(%struct.ap_uint.15* %this) unnamed_addr #1 comdat align 2 {
  %1 = alloca %struct.ap_uint.15*, align 8
  store %struct.ap_uint.15* %this, %struct.ap_uint.15** %1, align 8
  %2 = load %struct.ap_uint.15*, %struct.ap_uint.15** %1
  %3 = bitcast %struct.ap_uint.15* %2 to %struct.ap_int_base.16*
  call void @_ZN11ap_int_baseILi32ELb0ELb1EEC2Ev(%struct.ap_int_base.16* %3)
  ret void
}

; Function Attrs: nounwind uwtable
define linkonce_odr void @_ZN3hls6WindowILi1ELi3E8tensor_tEC2Ev(%"class.hls::Window.2"* %this) unnamed_addr #2 comdat align 2 {
  %1 = alloca %"class.hls::Window.2"*, align 8
  store %"class.hls::Window.2"* %this, %"class.hls::Window.2"** %1, align 8
  %2 = load %"class.hls::Window.2"*, %"class.hls::Window.2"** %1
  %3 = getelementptr inbounds %"class.hls::Window.2", %"class.hls::Window.2"* %2, i32 0, i32 0
  %4 = getelementptr inbounds %"class.hls::Window.2", %"class.hls::Window.2"* %2, i32 0, i32 0
  %5 = getelementptr inbounds [1 x [3 x %struct.tensor_t]], [1 x [3 x %struct.tensor_t]]* %4, i32 0, i32 0
  call void (...) @_ssdm_SpecArrayPartition([3 x %struct.tensor_t]* %5, i32 1, i8* getelementptr inbounds ([9 x i8], [9 x i8]* @.str3, i32 0, i32 0), i32 0, i8* getelementptr inbounds ([1 x i8], [1 x i8]* @.str2, i32 0, i32 0)) #0
  %6 = getelementptr inbounds %"class.hls::Window.2", %"class.hls::Window.2"* %2, i32 0, i32 0
  %7 = getelementptr inbounds [1 x [3 x %struct.tensor_t]], [1 x [3 x %struct.tensor_t]]* %6, i32 0, i32 0
  call void (...) @_ssdm_SpecArrayPartition([3 x %struct.tensor_t]* %7, i32 2, i8* getelementptr inbounds ([9 x i8], [9 x i8]* @.str3, i32 0, i32 0), i32 0, i8* getelementptr inbounds ([1 x i8], [1 x i8]* @.str2, i32 0, i32 0)) #0
  ret void
}

; Function Attrs: nounwind uwtable
define linkonce_odr void @_ZN3hls10LineBufferILi3ELi1024E7outer_tLi0EEC2Ev(%"class.hls::LineBuffer.1"* %this) unnamed_addr #2 comdat align 2 {
  %1 = alloca %"class.hls::LineBuffer.1"*, align 8
  store %"class.hls::LineBuffer.1"* %this, %"class.hls::LineBuffer.1"** %1, align 8
  %2 = load %"class.hls::LineBuffer.1"*, %"class.hls::LineBuffer.1"** %1
  %3 = getelementptr inbounds %"class.hls::LineBuffer.1", %"class.hls::LineBuffer.1"* %2, i32 0, i32 0
  %4 = getelementptr inbounds %"class.hls::LineBuffer.1", %"class.hls::LineBuffer.1"* %2, i32 0, i32 0
  %5 = getelementptr inbounds [3 x [1024 x %struct.outer_t]], [3 x [1024 x %struct.outer_t]]* %4, i32 0, i32 0
  call void (...) @_ssdm_SpecArrayPartition([1024 x %struct.outer_t]* %5, i32 1, i8* getelementptr inbounds ([9 x i8], [9 x i8]* @.str3, i32 0, i32 0), i32 0, i8* getelementptr inbounds ([1 x i8], [1 x i8]* @.str2, i32 0, i32 0)) #0
  %6 = getelementptr inbounds %"class.hls::LineBuffer.1", %"class.hls::LineBuffer.1"* %2, i32 0, i32 0
  %7 = getelementptr inbounds [3 x [1024 x %struct.outer_t]], [3 x [1024 x %struct.outer_t]]* %6, i32 0, i32 0
  call void (...) @_ssdm_SpecDependence([1024 x %struct.outer_t]* %7, i32 0, i32 0, i32 -1, i32 0, i32 1) #0
  %8 = getelementptr inbounds %"class.hls::LineBuffer.1", %"class.hls::LineBuffer.1"* %2, i32 0, i32 0
  %9 = getelementptr inbounds [3 x [1024 x %struct.outer_t]], [3 x [1024 x %struct.outer_t]]* %8, i32 0, i32 0
  call void (...) @_ssdm_SpecDependence([1024 x %struct.outer_t]* %9, i32 0, i32 0, i32 -1, i32 0, i32 0) #0
  ret void
}

; Function Attrs: nounwind
declare void @_ssdm_SpecDependence(...) #0

; Function Attrs: nounwind uwtable
define linkonce_odr void @_ZN3hls6WindowILi1ELi7E10gradient_tEC2Ev(%"class.hls::Window.0"* %this) unnamed_addr #2 comdat align 2 {
  %1 = alloca %"class.hls::Window.0"*, align 8
  store %"class.hls::Window.0"* %this, %"class.hls::Window.0"** %1, align 8
  %2 = load %"class.hls::Window.0"*, %"class.hls::Window.0"** %1
  %3 = getelementptr inbounds %"class.hls::Window.0", %"class.hls::Window.0"* %2, i32 0, i32 0
  %4 = getelementptr inbounds %"class.hls::Window.0", %"class.hls::Window.0"* %2, i32 0, i32 0
  %5 = getelementptr inbounds [1 x [7 x %struct.gradient_t]], [1 x [7 x %struct.gradient_t]]* %4, i32 0, i32 0
  call void (...) @_ssdm_SpecArrayPartition([7 x %struct.gradient_t]* %5, i32 1, i8* getelementptr inbounds ([9 x i8], [9 x i8]* @.str3, i32 0, i32 0), i32 0, i8* getelementptr inbounds ([1 x i8], [1 x i8]* @.str2, i32 0, i32 0)) #0
  %6 = getelementptr inbounds %"class.hls::Window.0", %"class.hls::Window.0"* %2, i32 0, i32 0
  %7 = getelementptr inbounds [1 x [7 x %struct.gradient_t]], [1 x [7 x %struct.gradient_t]]* %6, i32 0, i32 0
  call void (...) @_ssdm_SpecArrayPartition([7 x %struct.gradient_t]* %7, i32 2, i8* getelementptr inbounds ([9 x i8], [9 x i8]* @.str3, i32 0, i32 0), i32 0, i8* getelementptr inbounds ([1 x i8], [1 x i8]* @.str2, i32 0, i32 0)) #0
  ret void
}

; Function Attrs: nounwind uwtable
define linkonce_odr void @_ZN3hls10LineBufferILi7ELi1024E10gradient_tLi0EEC2Ev(%"class.hls::LineBuffer"* %this) unnamed_addr #2 comdat align 2 {
  %1 = alloca %"class.hls::LineBuffer"*, align 8
  store %"class.hls::LineBuffer"* %this, %"class.hls::LineBuffer"** %1, align 8
  %2 = load %"class.hls::LineBuffer"*, %"class.hls::LineBuffer"** %1
  %3 = getelementptr inbounds %"class.hls::LineBuffer", %"class.hls::LineBuffer"* %2, i32 0, i32 0
  %4 = getelementptr inbounds %"class.hls::LineBuffer", %"class.hls::LineBuffer"* %2, i32 0, i32 0
  %5 = getelementptr inbounds [7 x [1024 x %struct.gradient_t]], [7 x [1024 x %struct.gradient_t]]* %4, i32 0, i32 0
  call void (...) @_ssdm_SpecArrayPartition([1024 x %struct.gradient_t]* %5, i32 1, i8* getelementptr inbounds ([9 x i8], [9 x i8]* @.str3, i32 0, i32 0), i32 0, i8* getelementptr inbounds ([1 x i8], [1 x i8]* @.str2, i32 0, i32 0)) #0
  %6 = getelementptr inbounds %"class.hls::LineBuffer", %"class.hls::LineBuffer"* %2, i32 0, i32 0
  %7 = getelementptr inbounds [7 x [1024 x %struct.gradient_t]], [7 x [1024 x %struct.gradient_t]]* %6, i32 0, i32 0
  call void (...) @_ssdm_SpecDependence([1024 x %struct.gradient_t]* %7, i32 0, i32 0, i32 -1, i32 0, i32 1) #0
  %8 = getelementptr inbounds %"class.hls::LineBuffer", %"class.hls::LineBuffer"* %2, i32 0, i32 0
  %9 = getelementptr inbounds [7 x [1024 x %struct.gradient_t]], [7 x [1024 x %struct.gradient_t]]* %8, i32 0, i32 0
  call void (...) @_ssdm_SpecDependence([1024 x %struct.gradient_t]* %9, i32 0, i32 0, i32 -1, i32 0, i32 0) #0
  ret void
}

; Function Attrs: nounwind uwtable
define linkonce_odr void @_ZN3hls6WindowILi5ELi5EfEC2Ev(%"class.hls::Window"* %this) unnamed_addr #2 comdat align 2 {
  %1 = alloca %"class.hls::Window"*, align 8
  store %"class.hls::Window"* %this, %"class.hls::Window"** %1, align 8
  %2 = load %"class.hls::Window"*, %"class.hls::Window"** %1
  %3 = getelementptr inbounds %"class.hls::Window", %"class.hls::Window"* %2, i32 0, i32 0
  %4 = getelementptr inbounds [5 x [5 x float]], [5 x [5 x float]]* %3, i32 0, i32 0
  call void (...) @_ssdm_SpecArrayPartition([5 x float]* %4, i32 1, i8* getelementptr inbounds ([9 x i8], [9 x i8]* @.str3, i32 0, i32 0), i32 0, i8* getelementptr inbounds ([1 x i8], [1 x i8]* @.str2, i32 0, i32 0)) #0
  %5 = getelementptr inbounds %"class.hls::Window", %"class.hls::Window"* %2, i32 0, i32 0
  %6 = getelementptr inbounds [5 x [5 x float]], [5 x [5 x float]]* %5, i32 0, i32 0
  call void (...) @_ssdm_SpecArrayPartition([5 x float]* %6, i32 2, i8* getelementptr inbounds ([9 x i8], [9 x i8]* @.str3, i32 0, i32 0), i32 0, i8* getelementptr inbounds ([1 x i8], [1 x i8]* @.str2, i32 0, i32 0)) #0
  ret void
}

; Function Attrs: alwaysinline inlinehint nounwind uwtable
define linkonce_odr void @_ZN7ap_uintILi64EEC2Ev(%struct.ap_uint.3* %this) unnamed_addr #1 comdat align 2 {
  %1 = alloca %struct.ap_uint.3*, align 8
  store %struct.ap_uint.3* %this, %struct.ap_uint.3** %1, align 8
  %2 = load %struct.ap_uint.3*, %struct.ap_uint.3** %1
  %3 = bitcast %struct.ap_uint.3* %2 to %struct.ap_int_base.4*
  call void @_ZN11ap_int_baseILi64ELb0ELb1EEC2Ev(%struct.ap_int_base.4* %3)
  ret void
}

; Function Attrs: alwaysinline inlinehint nounwind uwtable
define linkonce_odr void @_ZN11ap_int_baseILi64ELb0ELb1EEC2Ev(%struct.ap_int_base.4* %this) unnamed_addr #1 comdat align 2 {
  %1 = alloca %struct.ap_int_base.4*, align 8
  store %struct.ap_int_base.4* %this, %struct.ap_int_base.4** %1, align 8
  %2 = load %struct.ap_int_base.4*, %struct.ap_int_base.4** %1
  %3 = bitcast %struct.ap_int_base.4* %2 to %struct.ssdm_int.5*
  call void @_ZN8ssdm_intILi64ELb0EEC2Ev(%struct.ssdm_int.5* %3)
  ret void
}

; Function Attrs: alwaysinline inlinehint nounwind uwtable
define linkonce_odr void @_ZN8ssdm_intILi64ELb0EEC2Ev(%struct.ssdm_int.5* %this) unnamed_addr #1 comdat align 2 {
  %1 = alloca %struct.ssdm_int.5*, align 8
  store %struct.ssdm_int.5* %this, %struct.ssdm_int.5** %1, align 8
  %2 = load %struct.ssdm_int.5*, %struct.ssdm_int.5** %1
  ret void
}

; Function Attrs: alwaysinline inlinehint nounwind uwtable
define linkonce_odr void @_ZN12ap_range_refILi64ELb0EEC1EP11ap_int_baseILi64ELb0ELb1EEii(%struct.ap_range_ref* %this, %struct.ap_int_base.4* %bv, i32 %h, i32 %l) unnamed_addr #1 comdat align 2 {
  %1 = alloca %struct.ap_range_ref*, align 8
  %2 = alloca %struct.ap_int_base.4*, align 8
  %3 = alloca i32, align 4
  %4 = alloca i32, align 4
  store %struct.ap_range_ref* %this, %struct.ap_range_ref** %1, align 8
  store %struct.ap_int_base.4* %bv, %struct.ap_int_base.4** %2, align 8
  store i32 %h, i32* %3, align 4
  store i32 %l, i32* %4, align 4
  %5 = load %struct.ap_range_ref*, %struct.ap_range_ref** %1
  %6 = load %struct.ap_int_base.4*, %struct.ap_int_base.4** %2, align 8
  %7 = load i32, i32* %3, align 4
  %8 = load i32, i32* %4, align 4
  call void @_ZN12ap_range_refILi64ELb0EEC2EP11ap_int_baseILi64ELb0ELb1EEii(%struct.ap_range_ref* %5, %struct.ap_int_base.4* %6, i32 %7, i32 %8)
  ret void
}

; Function Attrs: alwaysinline inlinehint nounwind uwtable
define linkonce_odr void @_ZN12ap_range_refILi64ELb0EEC2EP11ap_int_baseILi64ELb0ELb1EEii(%struct.ap_range_ref* %this, %struct.ap_int_base.4* %bv, i32 %h, i32 %l) unnamed_addr #1 comdat align 2 {
  %1 = alloca %struct.ap_range_ref*, align 8
  %2 = alloca %struct.ap_int_base.4*, align 8
  %3 = alloca i32, align 4
  %4 = alloca i32, align 4
  store %struct.ap_range_ref* %this, %struct.ap_range_ref** %1, align 8
  store %struct.ap_int_base.4* %bv, %struct.ap_int_base.4** %2, align 8
  store i32 %h, i32* %3, align 4
  store i32 %l, i32* %4, align 4
  %5 = load %struct.ap_range_ref*, %struct.ap_range_ref** %1
  %6 = getelementptr inbounds %struct.ap_range_ref, %struct.ap_range_ref* %5, i32 0, i32 0
  %7 = load %struct.ap_int_base.4*, %struct.ap_int_base.4** %2, align 8
  store %struct.ap_int_base.4* %7, %struct.ap_int_base.4** %6, align 8
  %8 = getelementptr inbounds %struct.ap_range_ref, %struct.ap_range_ref* %5, i32 0, i32 1
  %9 = load i32, i32* %4, align 4
  store i32 %9, i32* %8, align 4
  %10 = getelementptr inbounds %struct.ap_range_ref, %struct.ap_range_ref* %5, i32 0, i32 2
  %11 = load i32, i32* %3, align 4
  store i32 %11, i32* %10, align 4
  ret void
}

; Function Attrs: alwaysinline inlinehint nounwind uwtable
define linkonce_odr void @_ZN7ap_uintILi8EEC2Ev(%struct.ap_uint* %this) unnamed_addr #1 comdat align 2 {
  %1 = alloca %struct.ap_uint*, align 8
  store %struct.ap_uint* %this, %struct.ap_uint** %1, align 8
  %2 = load %struct.ap_uint*, %struct.ap_uint** %1
  %3 = bitcast %struct.ap_uint* %2 to %struct.ap_int_base*
  call void @_ZN11ap_int_baseILi8ELb0ELb1EEC2Ev(%struct.ap_int_base* %3)
  ret void
}

; Function Attrs: alwaysinline inlinehint nounwind uwtable
define linkonce_odr void @_ZN11ap_int_baseILi8ELb0ELb1EEC2Ev(%struct.ap_int_base* %this) unnamed_addr #1 comdat align 2 {
  %1 = alloca %struct.ap_int_base*, align 8
  store %struct.ap_int_base* %this, %struct.ap_int_base** %1, align 8
  %2 = load %struct.ap_int_base*, %struct.ap_int_base** %1
  %3 = bitcast %struct.ap_int_base* %2 to %struct.ssdm_int*
  call void @_ZN8ssdm_intILi8ELb0EEC2Ev(%struct.ssdm_int* %3)
  ret void
}

; Function Attrs: alwaysinline inlinehint nounwind uwtable
define linkonce_odr void @_ZN8ssdm_intILi8ELb0EEC2Ev(%struct.ssdm_int* %this) unnamed_addr #1 comdat align 2 {
  %1 = alloca %struct.ssdm_int*, align 8
  store %struct.ssdm_int* %this, %struct.ssdm_int** %1, align 8
  %2 = load %struct.ssdm_int*, %struct.ssdm_int** %1
  ret void
}

; Function Attrs: nounwind
define internal void @_GLOBAL__I_a() #0 section ".text.startup" {
  call void @__cxx_global_var_init()
  call void @__cxx_global_var_init1()
  ret void
}

attributes #0 = { nounwind }
attributes #1 = { alwaysinline inlinehint nounwind uwtable }
attributes #2 = { nounwind uwtable }
attributes #3 = { nounwind readnone speculatable }
attributes #4 = { nounwind readnone }
attributes #5 = { noreturn nounwind }

!opencl.kernels = !{!0, !7, !13, !19, !25, !28, !31, !34, !37, !40, !46, !47, !53, !57, !61, !62, !65, !67, !68, !69, !71, !72, !75, !76, !77, !78, !81, !82, !83, !84, !86, !87, !89, !90, !91, !92, !93, !95, !96, !97, !98, !100, !101, !102, !103, !104, !105, !108, !110, !111, !112, !113, !115, !116, !117, !118, !119, !121, !122, !123, !124, !125, !127, !128, !129, !130, !131, !132, !133, !134, !136, !140, !141, !142, !143, !144}
!hls.encrypted.func = !{}

!0 = !{void ([1024 x float]*, [1024 x float]*, [1024 x float]*)* @_Z16gradient_xy_calcPA1024_fS0_S0_, !1, !2, !3, !4, !5, !6}
!1 = !{!"kernel_arg_addr_space", i32 1, i32 1, i32 1}
!2 = !{!"kernel_arg_access_qual", !"none", !"none", !"none"}
!3 = !{!"kernel_arg_type", !"pixel_t [1024]*", !"pixel_t [1024]*", !"pixel_t [1024]*"}
!4 = !{!"kernel_arg_type_qual", !"", !"", !""}
!5 = !{!"kernel_arg_name", !"frame", !"gradient_x", !"gradient_y"}
!6 = !{!"reqd_work_group_size", i32 1, i32 1, i32 1}
!7 = !{void ([1024 x float]*, [1024 x float]*, [1024 x float]*, [1024 x float]*, [1024 x float]*, [1024 x float]*)* @_Z15gradient_z_calcPA1024_fS0_S0_S0_S0_S0_, !8, !9, !10, !11, !12, !6}
!8 = !{!"kernel_arg_addr_space", i32 1, i32 1, i32 1, i32 1, i32 1, i32 1}
!9 = !{!"kernel_arg_access_qual", !"none", !"none", !"none", !"none", !"none", !"none"}
!10 = !{!"kernel_arg_type", !"pixel_t [1024]*", !"pixel_t [1024]*", !"pixel_t [1024]*", !"pixel_t [1024]*", !"pixel_t [1024]*", !"pixel_t [1024]*"}
!11 = !{!"kernel_arg_type_qual", !"", !"", !"", !"", !"", !""}
!12 = !{!"kernel_arg_name", !"frame1", !"frame2", !"frame3", !"frame4", !"frame5", !"gradient_z"}
!13 = !{void ([1024 x float]*, [1024 x float]*, [1024 x float]*, [1024 x %struct.gradient_t]*)* @_Z17gradient_weight_yPA1024_fS0_S0_PA1024_10gradient_t, !14, !15, !16, !17, !18, !6}
!14 = !{!"kernel_arg_addr_space", i32 1, i32 1, i32 1, i32 1}
!15 = !{!"kernel_arg_access_qual", !"none", !"none", !"none", !"none"}
!16 = !{!"kernel_arg_type", !"pixel_t [1024]*", !"pixel_t [1024]*", !"pixel_t [1024]*", !"gradient_t [1024]*"}
!17 = !{!"kernel_arg_type_qual", !"", !"", !"", !""}
!18 = !{!"kernel_arg_name", !"gradient_x", !"gradient_y", !"gradient_z", !"filt_grad"}
!19 = !{void ([1024 x %struct.gradient_t]*, [1024 x %struct.gradient_t]*)* @_Z17gradient_weight_xPA1024_10gradient_tS1_, !20, !21, !22, !23, !24, !6}
!20 = !{!"kernel_arg_addr_space", i32 1, i32 1}
!21 = !{!"kernel_arg_access_qual", !"none", !"none"}
!22 = !{!"kernel_arg_type", !"gradient_t [1024]*", !"gradient_t [1024]*"}
!23 = !{!"kernel_arg_type_qual", !"", !""}
!24 = !{!"kernel_arg_name", !"y_filt", !"filt_grad"}
!25 = !{void ([1024 x %struct.gradient_t]*, [1024 x %struct.outer_t]*)* @_Z13outer_productPA1024_10gradient_tPA1024_7outer_t, !20, !21, !26, !23, !27, !6}
!26 = !{!"kernel_arg_type", !"gradient_t [1024]*", !"outer_t [1024]*"}
!27 = !{!"kernel_arg_name", !"gradient", !"outer_product"}
!28 = !{void ([1024 x %struct.outer_t]*, [1024 x %struct.tensor_t]*)* @_Z15tensor_weight_yPA1024_7outer_tPA1024_8tensor_t, !20, !21, !29, !23, !30, !6}
!29 = !{!"kernel_arg_type", !"outer_t [1024]*", !"tensor_t [1024]*"}
!30 = !{!"kernel_arg_name", !"outer", !"tensor_y"}
!31 = !{void ([1024 x %struct.tensor_t]*, [1024 x %struct.tensor_t]*)* @_Z15tensor_weight_xPA1024_8tensor_tS1_, !20, !21, !32, !23, !33, !6}
!32 = !{!"kernel_arg_type", !"tensor_t [1024]*", !"tensor_t [1024]*"}
!33 = !{!"kernel_arg_name", !"tensor_y", !"tensor"}
!34 = !{void ([1024 x %struct.tensor_t]*, [1024 x %struct.velocity_t]*)* @_Z9flow_calcPA1024_8tensor_tPA1024_10velocity_t, !20, !21, !35, !23, !36, !6}
!35 = !{!"kernel_arg_type", !"tensor_t [1024]*", !"velocity_t [1024]*"}
!36 = !{!"kernel_arg_name", !"tensors", !"outputs"}
!37 = !{void ([1024 x %struct.ap_uint.3]*, [1024 x %struct.velocity_t]*)* @_Z12optical_flowPA1024_7ap_uintILi64EEPA1024_10velocity_t, !20, !21, !38, !23, !39, !6}
!38 = !{!"kernel_arg_type", !"frames_t [1024]*", !"velocity_t [1024]*"}
!39 = !{!"kernel_arg_name", !"frames", !"outputs"}
!40 = !{i64 (%struct.ap_range_ref*)* @_ZNK12ap_range_refILi64ELb0EEcvyEv, !41, !42, !43, !44, !45, !6}
!41 = !{!"kernel_arg_addr_space"}
!42 = !{!"kernel_arg_access_qual"}
!43 = !{!"kernel_arg_type"}
!44 = !{!"kernel_arg_type_qual"}
!45 = !{!"kernel_arg_name"}
!46 = !{i64 (%struct.ap_range_ref*)* @_ZNK12ap_range_refILi64ELb0EE9to_uint64Ev, !41, !42, !43, !44, !45, !6}
!47 = !{%struct.ap_uint.3* (%struct.ap_uint.3*, %struct.ap_uint.3*)* @_ZN7ap_uintILi64EEaSERKS0_, !48, !49, !50, !51, !52, !6}
!48 = !{!"kernel_arg_addr_space", i32 0}
!49 = !{!"kernel_arg_access_qual", !"none"}
!50 = !{!"kernel_arg_type", !"const ap_uint<64> &"}
!51 = !{!"kernel_arg_type_qual", !""}
!52 = !{!"kernel_arg_name", !"op2"}
!53 = !{%struct.tensor_t* (%"class.hls::Window.2"*, i32, i32)* @_ZN3hls6WindowILi1ELi3E8tensor_tE6getvalEii, !54, !21, !55, !23, !56, !6}
!54 = !{!"kernel_arg_addr_space", i32 0, i32 0}
!55 = !{!"kernel_arg_type", !"int", !"int"}
!56 = !{!"kernel_arg_name", !"row", !"col"}
!57 = !{void (%"class.hls::Window.2"*, %struct.tensor_t*, i32, i32)* @_ZN3hls6WindowILi1ELi3E8tensor_tE12insert_pixelES1_ii, !58, !2, !59, !4, !60, !6}
!58 = !{!"kernel_arg_addr_space", i32 0, i32 0, i32 0}
!59 = !{!"kernel_arg_type", !"tensor_t", !"int", !"int"}
!60 = !{!"kernel_arg_name", !"value", !"row", !"col"}
!61 = !{void (%"class.hls::Window.2"*)* @_ZN3hls6WindowILi1ELi3E8tensor_tE17shift_pixels_leftEv, !41, !42, !43, !44, !45, !6}
!62 = !{void (%struct.ap_int_base.16*, %struct.ap_int_base.16*, i32)* @_ZN11ap_int_baseILi32ELb0ELb1EEppEi, !48, !49, !63, !51, !64, !6}
!63 = !{!"kernel_arg_type", !"int"}
!64 = !{!"kernel_arg_name", !""}
!65 = !{void (%struct.ap_int_base.20*, i32)* @_ZN11ap_int_baseILi1ELb0ELb1EEC1Ei, !48, !49, !63, !51, !66, !6}
!66 = !{!"kernel_arg_name", !"op"}
!67 = !{void (%struct.ap_int_base.20*, i32)* @_ZN11ap_int_baseILi1ELb0ELb1EEC2Ei, !48, !49, !63, !51, !66, !6}
!68 = !{void (%struct.ssdm_int.21*)* @_ZN8ssdm_intILi1ELb0EEC2Ev, !41, !42, !43, !44, !45, !6}
!69 = !{%struct.ap_int_base.16* (%struct.ap_int_base.16*, %struct.ap_int_base.20*)* @_ZN11ap_int_baseILi32ELb0ELb1EEpLILi1ELb0EEERS0_RKS_IXT_EXT0_EXleT_Li64EEE, !48, !49, !70, !51, !52, !6}
!70 = !{!"kernel_arg_type", !"const ap_int_base<1, false> &"}
!71 = !{i64 (%struct.ap_int_base.18*)* @_ZNK11ap_int_baseILi34ELb1ELb1EEcvxEv, !41, !42, !43, !44, !45, !6}
!72 = !{void (%struct.ap_int_base.18*, %struct.ap_int_base.16*, i32)* @_ZplILi32ELb0EEN11ap_int_baseIXT_EXT0_EXleT_Li64EEE5RTypeIXLi32EEXLb1EEE4plusERKS1_i, !54, !21, !73, !23, !74, !6}
!73 = !{!"kernel_arg_type", !"const ap_int_base<32, false> &", !"int"}
!74 = !{!"kernel_arg_name", !"op", !"i_op"}
!75 = !{void (%struct.ap_int_base.22*, i32)* @_ZN11ap_int_baseILi32ELb1ELb1EEC1Ei, !48, !49, !63, !51, !66, !6}
!76 = !{void (%struct.ap_int_base.22*, i32)* @_ZN11ap_int_baseILi32ELb1ELb1EEC2Ei, !48, !49, !63, !51, !66, !6}
!77 = !{void (%struct.ssdm_int.23*)* @_ZN8ssdm_intILi32ELb1EEC2Ev, !41, !42, !43, !44, !45, !6}
!78 = !{void (%struct.ap_int_base.18*, %struct.ap_int_base.16*, %struct.ap_int_base.22*)* @_ZplILi32ELb0ELi32ELb1EEN11ap_int_baseIXT_EXT0_EXleT_Li64EEE5RTypeIXT1_EXT2_EE4plusERKS1_RKS0_IXT1_EXT2_EXleT1_Li64EEE, !54, !21, !79, !23, !80, !6}
!79 = !{!"kernel_arg_type", !"const ap_int_base<32, false> &", !"const ap_int_base<32, true> &"}
!80 = !{!"kernel_arg_name", !"op", !"op2"}
!81 = !{void (%struct.ap_int_base.18*)* @_ZN11ap_int_baseILi34ELb1ELb1EEC1Ev, !41, !42, !43, !44, !45, !6}
!82 = !{void (%struct.ap_int_base.18*)* @_ZN11ap_int_baseILi34ELb1ELb1EEC2Ev, !41, !42, !43, !44, !45, !6}
!83 = !{void (%struct.ssdm_int.19*)* @_ZN8ssdm_intILi34ELb1EEC2Ev, !41, !42, !43, !44, !45, !6}
!84 = !{void (%struct.ap_int_base.18*, %struct.ap_int_base.22*)* @_ZN11ap_int_baseILi34ELb1ELb1EEC1ILi32ELb1EEERKS_IXT_EXT0_EXleT_Li64EEE, !48, !49, !85, !51, !66, !6}
!85 = !{!"kernel_arg_type", !"const ap_int_base<32, true> &"}
!86 = !{void (%struct.ap_int_base.18*, %struct.ap_int_base.22*)* @_ZN11ap_int_baseILi34ELb1ELb1EEC2ILi32ELb1EEERKS_IXT_EXT0_EXleT_Li64EEE, !48, !49, !85, !51, !66, !6}
!87 = !{void (%struct.ap_int_base.18*, %struct.ap_int_base.16*)* @_ZN11ap_int_baseILi34ELb1ELb1EEC1ILi32ELb0EEERKS_IXT_EXT0_EXleT_Li64EEE, !48, !49, !88, !51, !66, !6}
!88 = !{!"kernel_arg_type", !"const ap_int_base<32, false> &"}
!89 = !{void (%struct.ap_int_base.18*, %struct.ap_int_base.16*)* @_ZN11ap_int_baseILi34ELb1ELb1EEC2ILi32ELb0EEERKS_IXT_EXT0_EXleT_Li64EEE, !48, !49, !88, !51, !66, !6}
!90 = !{i32 (%struct.ap_int_base.16*)* @_ZNK11ap_int_baseILi32ELb0ELb1EEcvjEv, !41, !42, !43, !44, !45, !6}
!91 = !{i1 (%struct.ap_int_base.16*, i32)* @_ZltILi32ELb0EEbRK11ap_int_baseIXT_EXT0_EXleT_Li64EEEi, !54, !21, !73, !23, !80, !6}
!92 = !{i1 (%struct.ap_int_base.16*, %struct.ap_int_base.22*)* @_ZNK11ap_int_baseILi32ELb0ELb1EEltILi32ELb1EEEbRKS_IXT_EXT0_EXleT_Li64EEE, !48, !49, !85, !51, !52, !6}
!93 = !{void (%struct.ap_uint.15*, i32)* @_ZN7ap_uintILi32EEC1Ei, !48, !49, !63, !51, !94, !6}
!94 = !{!"kernel_arg_name", !"val"}
!95 = !{void (%struct.ap_uint.15*, i32)* @_ZN7ap_uintILi32EEC2Ei, !48, !49, !63, !51, !94, !6}
!96 = !{void (%struct.ap_int_base.16*)* @_ZN11ap_int_baseILi32ELb0ELb1EEC2Ev, !41, !42, !43, !44, !45, !6}
!97 = !{void (%struct.ssdm_int.17*)* @_ZN8ssdm_intILi32ELb0EEC2Ev, !41, !42, !43, !44, !45, !6}
!98 = !{%struct.ap_uint.15* (%struct.ap_uint.15*, %struct.ap_uint.15*)* @_ZN7ap_uintILi32EEaSERKS0_, !48, !49, !99, !51, !52, !6}
!99 = !{!"kernel_arg_type", !"const ap_uint<32> &"}
!100 = !{void (%struct.ap_uint.15*)* @_ZN7ap_uintILi32EEC1Ev, !41, !42, !43, !44, !45, !6}
!101 = !{void (%struct.ap_uint.15*)* @_ZN7ap_uintILi32EEC2Ev, !41, !42, !43, !44, !45, !6}
!102 = !{void (%"class.hls::Window.2"*)* @_ZN3hls6WindowILi1ELi3E8tensor_tEC1Ev, !41, !42, !43, !44, !45, !6}
!103 = !{void (%"class.hls::Window.2"*)* @_ZN3hls6WindowILi1ELi3E8tensor_tEC2Ev, !41, !42, !43, !44, !45, !6}
!104 = !{%struct.outer_t* (%"class.hls::LineBuffer.1"*, i32, i32)* @_ZN3hls10LineBufferILi3ELi1024E7outer_tLi0EE6getvalEii, !54, !21, !55, !23, !56, !6}
!105 = !{void (%"class.hls::LineBuffer.1"*, %struct.outer_t*, i32)* @_ZN3hls10LineBufferILi3ELi1024E7outer_tLi0EE17insert_bottom_rowES1_i, !54, !21, !106, !23, !107, !6}
!106 = !{!"kernel_arg_type", !"outer_t", !"int"}
!107 = !{!"kernel_arg_name", !"value", !"col"}
!108 = !{void (%"class.hls::LineBuffer.1"*, i32)* @_ZN3hls10LineBufferILi3ELi1024E7outer_tLi0EE15shift_pixels_upEi, !48, !49, !63, !51, !109, !6}
!109 = !{!"kernel_arg_name", !"col"}
!110 = !{void (%"class.hls::LineBuffer.1"*)* @_ZN3hls10LineBufferILi3ELi1024E7outer_tLi0EEC1Ev, !41, !42, !43, !44, !45, !6}
!111 = !{void (%"class.hls::LineBuffer.1"*)* @_ZN3hls10LineBufferILi3ELi1024E7outer_tLi0EEC2Ev, !41, !42, !43, !44, !45, !6}
!112 = !{%struct.gradient_t* (%"class.hls::Window.0"*, i32, i32)* @_ZN3hls6WindowILi1ELi7E10gradient_tE6getvalEii, !54, !21, !55, !23, !56, !6}
!113 = !{void (%"class.hls::Window.0"*, %struct.gradient_t*, i32, i32)* @_ZN3hls6WindowILi1ELi7E10gradient_tE12insert_pixelES1_ii, !58, !2, !114, !4, !60, !6}
!114 = !{!"kernel_arg_type", !"gradient_t", !"int", !"int"}
!115 = !{void (%"class.hls::Window.0"*)* @_ZN3hls6WindowILi1ELi7E10gradient_tE17shift_pixels_leftEv, !41, !42, !43, !44, !45, !6}
!116 = !{void (%"class.hls::Window.0"*)* @_ZN3hls6WindowILi1ELi7E10gradient_tEC1Ev, !41, !42, !43, !44, !45, !6}
!117 = !{void (%"class.hls::Window.0"*)* @_ZN3hls6WindowILi1ELi7E10gradient_tEC2Ev, !41, !42, !43, !44, !45, !6}
!118 = !{%struct.gradient_t* (%"class.hls::LineBuffer"*, i32, i32)* @_ZN3hls10LineBufferILi7ELi1024E10gradient_tLi0EE6getvalEii, !54, !21, !55, !23, !56, !6}
!119 = !{void (%"class.hls::LineBuffer"*, %struct.gradient_t*, i32)* @_ZN3hls10LineBufferILi7ELi1024E10gradient_tLi0EE17insert_bottom_rowES1_i, !54, !21, !120, !23, !107, !6}
!120 = !{!"kernel_arg_type", !"gradient_t", !"int"}
!121 = !{void (%"class.hls::LineBuffer"*, i32)* @_ZN3hls10LineBufferILi7ELi1024E10gradient_tLi0EE15shift_pixels_upEi, !48, !49, !63, !51, !109, !6}
!122 = !{void (%"class.hls::LineBuffer"*)* @_ZN3hls10LineBufferILi7ELi1024E10gradient_tLi0EEC1Ev, !41, !42, !43, !44, !45, !6}
!123 = !{void (%"class.hls::LineBuffer"*)* @_ZN3hls10LineBufferILi7ELi1024E10gradient_tLi0EEC2Ev, !41, !42, !43, !44, !45, !6}
!124 = !{float* (%"class.hls::Window"*, i32, i32)* @_ZN3hls6WindowILi5ELi5EfE6getvalEii, !54, !21, !55, !23, !56, !6}
!125 = !{void (%"class.hls::Window"*, float, i32, i32)* @_ZN3hls6WindowILi5ELi5EfE12insert_pixelEfii, !58, !2, !126, !4, !60, !6}
!126 = !{!"kernel_arg_type", !"float", !"int", !"int"}
!127 = !{void (%"class.hls::Window"*)* @_ZN3hls6WindowILi5ELi5EfE17shift_pixels_leftEv, !41, !42, !43, !44, !45, !6}
!128 = !{void (%"class.hls::Window"*)* @_ZN3hls6WindowILi5ELi5EfEC1Ev, !41, !42, !43, !44, !45, !6}
!129 = !{void (%"class.hls::Window"*)* @_ZN3hls6WindowILi5ELi5EfEC2Ev, !41, !42, !43, !44, !45, !6}
!130 = !{void (%struct.ap_uint.3*)* @_ZN7ap_uintILi64EEC1Ev, !41, !42, !43, !44, !45, !6}
!131 = !{void (%struct.ap_uint.3*)* @_ZN7ap_uintILi64EEC2Ev, !41, !42, !43, !44, !45, !6}
!132 = !{void (%struct.ap_int_base.4*)* @_ZN11ap_int_baseILi64ELb0ELb1EEC2Ev, !41, !42, !43, !44, !45, !6}
!133 = !{void (%struct.ssdm_int.5*)* @_ZN8ssdm_intILi64ELb0EEC2Ev, !41, !42, !43, !44, !45, !6}
!134 = !{void (%struct.ap_range_ref*, %struct.ap_int_base.4*, i32, i32)* @_ZN11ap_int_baseILi64ELb0ELb1EEclEii, !54, !21, !55, !23, !135, !6}
!135 = !{!"kernel_arg_name", !"Hi", !"Lo"}
!136 = !{void (%struct.ap_range_ref*, %struct.ap_int_base.4*, i32, i32)* @_ZN12ap_range_refILi64ELb0EEC1EP11ap_int_baseILi64ELb0ELb1EEii, !137, !2, !138, !4, !139, !6}
!137 = !{!"kernel_arg_addr_space", i32 1, i32 0, i32 0}
!138 = !{!"kernel_arg_type", !"ap_int_base<64, false>*", !"int", !"int"}
!139 = !{!"kernel_arg_name", !"bv", !"h", !"l"}
!140 = !{void (%struct.ap_range_ref*, %struct.ap_int_base.4*, i32, i32)* @_ZN12ap_range_refILi64ELb0EEC2EP11ap_int_baseILi64ELb0ELb1EEii, !137, !2, !138, !4, !139, !6}
!141 = !{void (%struct.ap_uint*)* @_ZN7ap_uintILi8EEC1Ev, !41, !42, !43, !44, !45, !6}
!142 = !{void (%struct.ap_uint*)* @_ZN7ap_uintILi8EEC2Ev, !41, !42, !43, !44, !45, !6}
!143 = !{void (%struct.ap_int_base*)* @_ZN11ap_int_baseILi8ELb0ELb1EEC2Ev, !41, !42, !43, !44, !45, !6}
!144 = !{void (%struct.ssdm_int*)* @_ZN8ssdm_intILi8ELb0EEC2Ev, !41, !42, !43, !44, !45, !6}

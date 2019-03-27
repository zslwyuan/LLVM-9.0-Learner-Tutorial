; ModuleID = 'top.bc'
source_filename = "top.bc"
target datalayout = "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v64:64:64-v128:128:128-a0:0:64-s0:64:64-f80:128:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@.str = private unnamed_addr constant [1 x i8] zeroinitializer, align 1
@.str1 = private unnamed_addr constant [13 x i8] c"RAM_T2P_BRAM\00", align 1

; Function Attrs: nounwind uwtable
define void @_Z1fPA100_iiiiiPi([100 x i32]* %A, i32 %a, i32 %b, i32 %c, i32 %d, i32* %e) #0 {
  %1 = alloca [100 x i32]*, align 8
  %2 = alloca i32, align 4
  %3 = alloca i32, align 4
  %4 = alloca i32, align 4
  %5 = alloca i32, align 4
  %6 = alloca i32*, align 8
  %N = alloca i32, align 4
  %M = alloca i32, align 4
  %j = alloca i32, align 4
  %i = alloca i32, align 4
  %j1 = alloca i32, align 4
  %i2 = alloca i32, align 4
  store [100 x i32]* %A, [100 x i32]** %1, align 8
  store i32 %a, i32* %2, align 4
  store i32 %b, i32* %3, align 4
  store i32 %c, i32* %4, align 4
  store i32 %d, i32* %5, align 4
  store i32* %e, i32** %6, align 8
  %7 = load [100 x i32]*, [100 x i32]** %1, align 8
  call void (...) @_ssdm_SpecArrayDimSize([100 x i32]* %7, i32 50) #2
  %8 = load [100 x i32]*, [100 x i32]** %1, align 8
  call void (...) @_ssdm_op_SpecResource([100 x i32]* %8, i8* getelementptr inbounds ([1 x i8], [1 x i8]* @.str, i32 0, i32 0), i8* getelementptr inbounds ([13 x i8], [13 x i8]* @.str1, i32 0, i32 0), i8* getelementptr inbounds ([1 x i8], [1 x i8]* @.str, i32 0, i32 0), i32 -1, i8* getelementptr inbounds ([1 x i8], [1 x i8]* @.str, i32 0, i32 0), i8* getelementptr inbounds ([1 x i8], [1 x i8]* @.str, i32 0, i32 0), i8* getelementptr inbounds ([1 x i8], [1 x i8]* @.str, i32 0, i32 0), i8* getelementptr inbounds ([1 x i8], [1 x i8]* @.str, i32 0, i32 0), i8* getelementptr inbounds ([1 x i8], [1 x i8]* @.str, i32 0, i32 0)) #2
  store i32 100, i32* %N, align 4
  store i32 50, i32* %M, align 4
  store i32 1, i32* %j, align 4
  br label %9

; <label>:9:                                      ; preds = %62, %0
  %10 = load i32, i32* %j, align 4
  %11 = load i32, i32* %N, align 4
  %12 = icmp slt i32 %10, %11
  br i1 %12, label %13, label %65

; <label>:13:                                     ; preds = %9
  %14 = load i32, i32* %M, align 4
  %15 = sub nsw i32 %14, 1
  store i32 %15, i32* %i, align 4
  br label %16

; <label>:16:                                     ; preds = %58, %13
  %17 = load i32, i32* %i, align 4
  %18 = icmp sge i32 %17, 1
  br i1 %18, label %19, label %61

; <label>:19:                                     ; preds = %16
  %20 = load i32, i32* %j, align 4
  %21 = sub nsw i32 %20, 1
  %22 = sext i32 %21 to i64
  %23 = load i32, i32* %i, align 4
  %24 = sub nsw i32 %23, 1
  %25 = sext i32 %24 to i64
  %26 = load [100 x i32]*, [100 x i32]** %1, align 8
  %27 = getelementptr inbounds [100 x i32], [100 x i32]* %26, i64 %25
  %28 = getelementptr inbounds [100 x i32], [100 x i32]* %27, i32 0, i64 %22
  %29 = load i32, i32* %28, align 4
  %30 = load i32, i32* %j, align 4
  %31 = sub nsw i32 %30, 1
  %32 = sext i32 %31 to i64
  %33 = load i32, i32* %i, align 4
  %34 = sext i32 %33 to i64
  %35 = load [100 x i32]*, [100 x i32]** %1, align 8
  %36 = getelementptr inbounds [100 x i32], [100 x i32]* %35, i64 %34
  %37 = getelementptr inbounds [100 x i32], [100 x i32]* %36, i32 0, i64 %32
  %38 = load i32, i32* %37, align 4
  %39 = add nsw i32 %29, %38
  %40 = load i32, i32* %j, align 4
  %41 = sext i32 %40 to i64
  %42 = load i32, i32* %i, align 4
  %43 = sub nsw i32 %42, 1
  %44 = sext i32 %43 to i64
  %45 = load [100 x i32]*, [100 x i32]** %1, align 8
  %46 = getelementptr inbounds [100 x i32], [100 x i32]* %45, i64 %44
  %47 = getelementptr inbounds [100 x i32], [100 x i32]* %46, i32 0, i64 %41
  %48 = load i32, i32* %47, align 4
  %49 = add nsw i32 %39, %48
  %50 = add nsw i32 %49, 1
  %51 = load i32, i32* %j, align 4
  %52 = sext i32 %51 to i64
  %53 = load i32, i32* %i, align 4
  %54 = sext i32 %53 to i64
  %55 = load [100 x i32]*, [100 x i32]** %1, align 8
  %56 = getelementptr inbounds [100 x i32], [100 x i32]* %55, i64 %54
  %57 = getelementptr inbounds [100 x i32], [100 x i32]* %56, i32 0, i64 %52
  store i32 %50, i32* %57, align 4
  br label %58

; <label>:58:                                     ; preds = %19
  %59 = load i32, i32* %i, align 4
  %60 = add nsw i32 %59, -1
  store i32 %60, i32* %i, align 4
  br label %16

; <label>:61:                                     ; preds = %16
  br label %62

; <label>:62:                                     ; preds = %61
  %63 = load i32, i32* %j, align 4
  %64 = add nsw i32 %63, 1
  store i32 %64, i32* %j, align 4
  br label %9

; <label>:65:                                     ; preds = %9
  store i32 1, i32* %j1, align 4
  br label %66

; <label>:66:                                     ; preds = %118, %65
  %67 = load i32, i32* %j1, align 4
  %68 = load i32, i32* %N, align 4
  %69 = icmp slt i32 %67, %68
  br i1 %69, label %70, label %121

; <label>:70:                                     ; preds = %66
  store i32 1, i32* %i2, align 4
  br label %71

; <label>:71:                                     ; preds = %114, %70
  %72 = load i32, i32* %i2, align 4
  %73 = load i32, i32* %M, align 4
  %74 = icmp slt i32 %72, %73
  br i1 %74, label %75, label %117

; <label>:75:                                     ; preds = %71
  %76 = load i32, i32* %j1, align 4
  %77 = sub nsw i32 %76, 1
  %78 = sext i32 %77 to i64
  %79 = load i32, i32* %i2, align 4
  %80 = sub nsw i32 %79, 1
  %81 = sext i32 %80 to i64
  %82 = load [100 x i32]*, [100 x i32]** %1, align 8
  %83 = getelementptr inbounds [100 x i32], [100 x i32]* %82, i64 %81
  %84 = getelementptr inbounds [100 x i32], [100 x i32]* %83, i32 0, i64 %78
  %85 = load i32, i32* %84, align 4
  %86 = load i32, i32* %j1, align 4
  %87 = sub nsw i32 %86, 1
  %88 = sext i32 %87 to i64
  %89 = load i32, i32* %i2, align 4
  %90 = sext i32 %89 to i64
  %91 = load [100 x i32]*, [100 x i32]** %1, align 8
  %92 = getelementptr inbounds [100 x i32], [100 x i32]* %91, i64 %90
  %93 = getelementptr inbounds [100 x i32], [100 x i32]* %92, i32 0, i64 %88
  %94 = load i32, i32* %93, align 4
  %95 = add nsw i32 %85, %94
  %96 = load i32, i32* %j1, align 4
  %97 = sext i32 %96 to i64
  %98 = load i32, i32* %i2, align 4
  %99 = sub nsw i32 %98, 1
  %100 = sext i32 %99 to i64
  %101 = load [100 x i32]*, [100 x i32]** %1, align 8
  %102 = getelementptr inbounds [100 x i32], [100 x i32]* %101, i64 %100
  %103 = getelementptr inbounds [100 x i32], [100 x i32]* %102, i32 0, i64 %97
  %104 = load i32, i32* %103, align 4
  %105 = add nsw i32 %95, %104
  %106 = add nsw i32 %105, 1
  %107 = load i32, i32* %j1, align 4
  %108 = sext i32 %107 to i64
  %109 = load i32, i32* %i2, align 4
  %110 = sext i32 %109 to i64
  %111 = load [100 x i32]*, [100 x i32]** %1, align 8
  %112 = getelementptr inbounds [100 x i32], [100 x i32]* %111, i64 %110
  %113 = getelementptr inbounds [100 x i32], [100 x i32]* %112, i32 0, i64 %108
  store i32 %106, i32* %113, align 4
  br label %114

; <label>:114:                                    ; preds = %75
  %115 = load i32, i32* %i2, align 4
  %116 = add nsw i32 %115, 1
  store i32 %116, i32* %i2, align 4
  br label %71

; <label>:117:                                    ; preds = %71
  br label %118

; <label>:118:                                    ; preds = %117
  %119 = load i32, i32* %j1, align 4
  %120 = add nsw i32 %119, 1
  store i32 %120, i32* %j1, align 4
  br label %66

; <label>:121:                                    ; preds = %66
  %122 = load i32, i32* %2, align 4
  %123 = load i32, i32* %3, align 4
  %124 = add nsw i32 %122, %123
  %125 = load i32, i32* %4, align 4
  %126 = add nsw i32 %124, %125
  %127 = load i32, i32* %5, align 4
  %128 = add nsw i32 %126, %127
  %129 = load i32*, i32** %6, align 8
  store i32 %128, i32* %129, align 4
  ret void
}

; Function Attrs: nounwind readnone speculatable
declare void @llvm.dbg.declare(metadata, metadata) #1

; Function Attrs: nounwind
declare void @_ssdm_SpecArrayDimSize(...) #2

; Function Attrs: nounwind
declare void @_ssdm_op_SpecResource(...) #2

attributes #0 = { nounwind uwtable }
attributes #1 = { nounwind readnone speculatable }
attributes #2 = { nounwind }

!opencl.kernels = !{!0}
!hls.encrypted.func = !{}

!0 = !{void ([100 x i32]*, i32, i32, i32, i32, i32*)* @_Z1fPA100_iiiiiPi, !1, !2, !3, !4, !5, !6}
!1 = !{!"kernel_arg_addr_space", i32 1, i32 0, i32 0, i32 0, i32 0, i32 1}
!2 = !{!"kernel_arg_access_qual", !"none", !"none", !"none", !"none", !"none", !"none"}
!3 = !{!"kernel_arg_type", !"int [100]*", !"int", !"int", !"int", !"int", !"int*"}
!4 = !{!"kernel_arg_type_qual", !"", !"", !"", !"", !"", !""}
!5 = !{!"kernel_arg_name", !"A", !"a", !"b", !"c", !"d", !"e"}
!6 = !{!"reqd_work_group_size", i32 1, i32 1, i32 1}

; ModuleID = 'arb_prec_int.cpp'
source_filename = "arb_prec_int.cpp"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%class.Derived = type { %class.Base }
%class.Base = type { i65 }

@_ZL8an_array = internal constant [3 x i65] [i65 1, i65 2, i65 3], align 16
@_ZL9a_derived = internal constant { i65 } { i65 17 }, align 8

; Function Attrs: noinline nounwind optnone
define void @_Z9NewDeletev() #0 {
entry:
  %p = alloca i37*, align 8
  %p2 = alloca i37*, align 8
  %call = call i8* @_Znwm(i64 8) #4
  %0 = bitcast i8* %call to i37*
  store i37* %0, i37** %p, align 8
  %call1 = call i8* @_Znam(i64 80) #4
  %1 = bitcast i8* %call1 to i37*
  store i37* %1, i37** %p2, align 8
  %2 = load i37*, i37** %p2, align 8
  %isnull = icmp eq i37* %2, null
  br i1 %isnull, label %delete.end, label %delete.notnull

delete.notnull:                                   ; preds = %entry
  %3 = bitcast i37* %2 to i8*
  call void @_ZdaPv(i8* %3) #5
  br label %delete.end

delete.end:                                       ; preds = %delete.notnull, %entry
  %4 = load i37*, i37** %p, align 8
  %isnull2 = icmp eq i37* %4, null
  br i1 %isnull2, label %delete.end4, label %delete.notnull3

delete.notnull3:                                  ; preds = %delete.end
  %5 = bitcast i37* %4 to i8*
  call void @_ZdlPv(i8* %5) #5
  br label %delete.end4

delete.end4:                                      ; preds = %delete.notnull3, %delete.end
  ret void
}

; Function Attrs: nobuiltin
declare noalias i8* @_Znwm(i64) #1

; Function Attrs: nobuiltin
declare noalias i8* @_Znam(i64) #1

; Function Attrs: nobuiltin nounwind
declare void @_ZdaPv(i8*) #2

; Function Attrs: nobuiltin nounwind
declare void @_ZdlPv(i8*) #2

; Function Attrs: noinline nounwind optnone
define zeroext i1 @_Z19SignedConvertToBoolv() #0 {
entry:
  %an_ap_int = alloca i65, align 8
  %b = alloca i8, align 1
  %the_5th_bit = alloca i8, align 1
  store i65 10, i65* %an_ap_int, align 8
  %0 = load i65, i65* %an_ap_int, align 8
  %tobool = icmp ne i65 %0, 0
  %frombool = zext i1 %tobool to i8
  store i8 %frombool, i8* %b, align 1
  %1 = load i65, i65* %an_ap_int, align 8
  %shr = ashr i65 %1, 5
  %and = and i65 %shr, 1
  %tobool1 = icmp ne i65 %and, 0
  %frombool2 = zext i1 %tobool1 to i8
  store i8 %frombool2, i8* %the_5th_bit, align 1
  %2 = load i65, i65* %an_ap_int, align 8
  %tobool3 = icmp ne i65 %2, 0
  ret i1 %tobool3
}

; Function Attrs: noinline nounwind optnone
define zeroext i1 @_Z21UnsignedConvertToBoolv() #0 {
entry:
  %an_ap_int = alloca i65, align 8
  %b = alloca i8, align 1
  %the_5th_bit = alloca i8, align 1
  store i65 10, i65* %an_ap_int, align 8
  %0 = load i65, i65* %an_ap_int, align 8
  %tobool = icmp ne i65 %0, 0
  %frombool = zext i1 %tobool to i8
  store i8 %frombool, i8* %b, align 1
  %1 = load i65, i65* %an_ap_int, align 8
  %shr = lshr i65 %1, 5
  %and = and i65 %shr, 1
  %tobool1 = icmp ne i65 %and, 0
  %frombool2 = zext i1 %tobool1 to i8
  store i8 %frombool2, i8* %the_5th_bit, align 1
  %2 = load i65, i65* %an_ap_int, align 8
  %tobool3 = icmp ne i65 %2, 0
  ret i1 %tobool3
}

; Function Attrs: noinline nounwind optnone
define void @_Z17OtherBoolConvertsv() #0 {
entry:
  %s = alloca i5, align 1
  %t1 = alloca i5, align 1
  %t2 = alloca i5, align 1
  %b = alloca i8, align 1
  %s1 = alloca i2, align 1
  %t3 = alloca i2, align 1
  %t4 = alloca i2, align 1
  %b1 = alloca i8, align 1
  store i5 0, i5* %s, align 1
  %0 = load i5, i5* %s, align 1
  %add = add nsw i5 %0, 1
  store i5 %add, i5* %t1, align 1
  %1 = load i5, i5* %s, align 1
  %add1 = add nsw i5 1, %1
  store i5 %add1, i5* %t2, align 1
  %2 = load i5, i5* %s, align 1
  %add2 = add nsw i5 %2, 1
  store i5 %add2, i5* %s, align 1
  store i8 1, i8* %b, align 1
  %3 = load i5, i5* %s, align 1
  %4 = load i8, i8* %b, align 1
  %tobool = trunc i8 %4 to i1
  %conv = zext i1 %tobool to i5
  %add3 = add nsw i5 %conv, %3
  %tobool4 = icmp ne i5 %add3, 0
  %frombool = zext i1 %tobool4 to i8
  store i8 %frombool, i8* %b, align 1
  store i2 0, i2* %s1, align 1
  %5 = load i2, i2* %s1, align 1
  %add5 = add nsw i2 %5, 1
  store i2 %add5, i2* %t3, align 1
  %6 = load i2, i2* %s1, align 1
  %add6 = add nsw i2 1, %6
  store i2 %add6, i2* %t4, align 1
  %7 = load i2, i2* %s1, align 1
  %add7 = add nsw i2 %7, 1
  store i2 %add7, i2* %s1, align 1
  store i8 1, i8* %b1, align 1
  %8 = load i2, i2* %s1, align 1
  %9 = load i8, i8* %b1, align 1
  %tobool8 = trunc i8 %9 to i1
  %conv9 = zext i1 %tobool8 to i2
  %add10 = add nsw i2 %conv9, %8
  %tobool11 = icmp ne i2 %add10, 0
  %frombool12 = zext i1 %tobool11 to i8
  store i8 %frombool12, i8* %b1, align 1
  ret void
}

; Function Attrs: noinline nounwind optnone
define i65 @_Z9int_usagev() #0 {
entry:
  ret i65 17
}

; Function Attrs: noinline nounwind optnone
define i77 @_Z15unsigned_usagesi(i32 %i) #0 {
entry:
  %retval = alloca i77, align 8
  %i.addr = alloca i32, align 4
  store i32 %i, i32* %i.addr, align 4
  %0 = load i32, i32* %i.addr, align 4
  switch i32 %0, label %sw.epilog [
    i32 1, label %sw.bb
    i32 2, label %sw.bb1
    i32 3, label %sw.bb2
    i32 4, label %sw.bb3
    i32 5, label %sw.bb4
    i32 6, label %sw.bb5
    i32 7, label %sw.bb6
    i32 8, label %sw.bb7
    i32 9, label %sw.bb8
    i32 0, label %sw.bb9
    i32 10, label %sw.bb10
  ]

sw.bb:                                            ; preds = %entry
  store i77 15, i77* %retval, align 8
  br label %return

sw.bb1:                                           ; preds = %entry
  store i77 4, i77* %retval, align 8
  br label %return

sw.bb2:                                           ; preds = %entry
  store i77 19, i77* %retval, align 8
  br label %return

sw.bb3:                                           ; preds = %entry
  store i77 11, i77* %retval, align 8
  br label %return

sw.bb4:                                           ; preds = %entry
  store i77 3, i77* %retval, align 8
  br label %return

sw.bb5:                                           ; preds = %entry
  store i77 60, i77* %retval, align 8
  br label %return

sw.bb6:                                           ; preds = %entry
  store i77 3, i77* %retval, align 8
  br label %return

sw.bb7:                                           ; preds = %entry
  store i77 15, i77* %retval, align 8
  br label %return

sw.bb8:                                           ; preds = %entry
  store i77 4, i77* %retval, align 8
  br label %return

sw.bb9:                                           ; preds = %entry
  store i77 11, i77* %retval, align 8
  br label %return

sw.bb10:                                          ; preds = %entry
  store i77 -16, i77* %retval, align 8
  br label %return

sw.epilog:                                        ; preds = %entry
  store i77 0, i77* %retval, align 8
  br label %return

return:                                           ; preds = %sw.epilog, %sw.bb10, %sw.bb9, %sw.bb8, %sw.bb7, %sw.bb6, %sw.bb5, %sw.bb4, %sw.bb3, %sw.bb2, %sw.bb1, %sw.bb
  %1 = load i77, i77* %retval, align 8
  ret i77 %1
}

; Function Attrs: noinline nounwind optnone
define i77 @_Z13signed_usagesi(i32 %i) #0 {
entry:
  %retval = alloca i77, align 8
  %i.addr = alloca i32, align 4
  store i32 %i, i32* %i.addr, align 4
  %0 = load i32, i32* %i.addr, align 4
  switch i32 %0, label %sw.epilog [
    i32 1, label %sw.bb
    i32 2, label %sw.bb1
    i32 3, label %sw.bb2
    i32 4, label %sw.bb3
    i32 5, label %sw.bb4
    i32 6, label %sw.bb5
    i32 7, label %sw.bb6
    i32 8, label %sw.bb7
    i32 9, label %sw.bb8
    i32 0, label %sw.bb9
    i32 10, label %sw.bb10
  ]

sw.bb:                                            ; preds = %entry
  store i77 15, i77* %retval, align 8
  br label %return

sw.bb1:                                           ; preds = %entry
  store i77 4, i77* %retval, align 8
  br label %return

sw.bb2:                                           ; preds = %entry
  store i77 19, i77* %retval, align 8
  br label %return

sw.bb3:                                           ; preds = %entry
  store i77 11, i77* %retval, align 8
  br label %return

sw.bb4:                                           ; preds = %entry
  store i77 3, i77* %retval, align 8
  br label %return

sw.bb5:                                           ; preds = %entry
  store i77 60, i77* %retval, align 8
  br label %return

sw.bb6:                                           ; preds = %entry
  store i77 3, i77* %retval, align 8
  br label %return

sw.bb7:                                           ; preds = %entry
  store i77 15, i77* %retval, align 8
  br label %return

sw.bb8:                                           ; preds = %entry
  store i77 4, i77* %retval, align 8
  br label %return

sw.bb9:                                           ; preds = %entry
  store i77 11, i77* %retval, align 8
  br label %return

sw.bb10:                                          ; preds = %entry
  store i77 -16, i77* %retval, align 8
  br label %return

sw.epilog:                                        ; preds = %entry
  store i77 0, i77* %retval, align 8
  br label %return

return:                                           ; preds = %sw.epilog, %sw.bb10, %sw.bb9, %sw.bb8, %sw.bb7, %sw.bb6, %sw.bb5, %sw.bb4, %sw.bb3, %sw.bb2, %sw.bb1, %sw.bb
  %1 = load i77, i77* %retval, align 8
  ret i77 %1
}

; Function Attrs: noinline nounwind optnone
define void @_Z14derrived_usagev(%class.Derived* noalias sret %agg.result) #0 {
entry:
  %0 = bitcast %class.Derived* %agg.result to i8*
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* align 8 %0, i8* align 8 bitcast ({ i65 }* @_ZL9a_derived to i8*), i64 16, i1 false)
  ret void
}

; Function Attrs: argmemonly nounwind
declare void @llvm.memcpy.p0i8.p0i8.i64(i8* nocapture writeonly, i8* nocapture readonly, i64, i1) #3

; Function Attrs: noinline nounwind optnone
define i65 @_Z11array_usagei(i32 %idx) #0 {
entry:
  %idx.addr = alloca i32, align 4
  store i32 %idx, i32* %idx.addr, align 4
  %0 = load i32, i32* %idx.addr, align 4
  %idxprom = sext i32 %0 to i64
  %arrayidx = getelementptr inbounds [3 x i65], [3 x i65]* @_ZL8an_array, i64 0, i64 %idxprom
  %1 = load i65, i65* %arrayidx, align 1
  ret i65 %1
}

attributes #0 = { noinline nounwind optnone "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-features"="+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nobuiltin "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-features"="+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #2 = { nobuiltin nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-features"="+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #3 = { argmemonly nounwind }
attributes #4 = { builtin }
attributes #5 = { builtin nounwind }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 9.0.0 "}

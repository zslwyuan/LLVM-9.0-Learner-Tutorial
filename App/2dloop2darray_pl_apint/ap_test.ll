; ModuleID = 'ap_test.cc'
source_filename = "ap_test.cc"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

$_Z10partSelectI11clang_apint_17_iET_S1_ii = comdat any

; Function Attrs: norecurse nounwind
define i32 @_Z1f11clang_apint_317_i11clang_apint_17_i(i317 %a, i17 %b) local_unnamed_addr #0 {
entry:
  %add = add nsw i317 %a, 1
  %0 = trunc i317 %add to i17
  %conv2 = add i17 %0, %b
  %call = tail call i17 @_Z10partSelectI11clang_apint_17_iET_S1_ii(i17 %conv2, i32 4, i32 1)
  %conv3 = sext i17 %call to i317
  %add4 = add nsw i317 %add, %conv3
  %1 = trunc i317 %add4 to i32
  %conv6 = add i32 %1, 1
  ret i32 %conv6
}

; Function Attrs: inlinehint norecurse nounwind
define linkonce_odr i17 @_Z10partSelectI11clang_apint_17_iET_S1_ii(i17 %input, i32 %l, i32 %r) local_unnamed_addr #1 comdat {
entry:
  %conv = sext i17 %input to i32
  %add = add nsw i32 %l, 1
  %notmask = shl nsw i32 -1, %add
  %sub = xor i32 %notmask, -1
  %and = and i32 %sub, %conv
  %shr = ashr i32 %and, %r
  %conv1 = trunc i32 %shr to i17
  ret i17 %conv1
}

attributes #0 = { norecurse nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-features"="+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { inlinehint norecurse nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-features"="+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 9.0.0 "}

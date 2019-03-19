; ModuleID = 'arb_prec_array.cpp'
source_filename = "arb_prec_array.cpp"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: norecurse nounwind
define void @_Z1fPA15_11clang_apint_13_i([15 x i13]* nocapture %a) local_unnamed_addr #0 {
entry:
  br label %for.cond1.preheader

for.cond1.preheader:                              ; preds = %for.cond1.preheader, %entry
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.cond1.preheader ]
  %arrayidx5 = getelementptr inbounds [15 x i13], [15 x i13]* %a, i64 %indvars.iv, i64 0
  %0 = load i13, i13* %arrayidx5, align 2, !tbaa !2
  %inc = add nsw i13 %0, 1
  store i13 %inc, i13* %arrayidx5, align 2, !tbaa !2
  %arrayidx5.1 = getelementptr inbounds [15 x i13], [15 x i13]* %a, i64 %indvars.iv, i64 1
  %1 = load i13, i13* %arrayidx5.1, align 2, !tbaa !2
  %inc.1 = add nsw i13 %1, 1
  store i13 %inc.1, i13* %arrayidx5.1, align 2, !tbaa !2
  %arrayidx5.2 = getelementptr inbounds [15 x i13], [15 x i13]* %a, i64 %indvars.iv, i64 2
  %2 = load i13, i13* %arrayidx5.2, align 2, !tbaa !2
  %inc.2 = add nsw i13 %2, 1
  store i13 %inc.2, i13* %arrayidx5.2, align 2, !tbaa !2
  %arrayidx5.3 = getelementptr inbounds [15 x i13], [15 x i13]* %a, i64 %indvars.iv, i64 3
  %3 = load i13, i13* %arrayidx5.3, align 2, !tbaa !2
  %inc.3 = add nsw i13 %3, 1
  store i13 %inc.3, i13* %arrayidx5.3, align 2, !tbaa !2
  %arrayidx5.4 = getelementptr inbounds [15 x i13], [15 x i13]* %a, i64 %indvars.iv, i64 4
  %4 = load i13, i13* %arrayidx5.4, align 2, !tbaa !2
  %inc.4 = add nsw i13 %4, 1
  store i13 %inc.4, i13* %arrayidx5.4, align 2, !tbaa !2
  %arrayidx5.5 = getelementptr inbounds [15 x i13], [15 x i13]* %a, i64 %indvars.iv, i64 5
  %5 = load i13, i13* %arrayidx5.5, align 2, !tbaa !2
  %inc.5 = add nsw i13 %5, 1
  store i13 %inc.5, i13* %arrayidx5.5, align 2, !tbaa !2
  %arrayidx5.6 = getelementptr inbounds [15 x i13], [15 x i13]* %a, i64 %indvars.iv, i64 6
  %6 = load i13, i13* %arrayidx5.6, align 2, !tbaa !2
  %inc.6 = add nsw i13 %6, 1
  store i13 %inc.6, i13* %arrayidx5.6, align 2, !tbaa !2
  %arrayidx5.7 = getelementptr inbounds [15 x i13], [15 x i13]* %a, i64 %indvars.iv, i64 7
  %7 = load i13, i13* %arrayidx5.7, align 2, !tbaa !2
  %inc.7 = add nsw i13 %7, 1
  store i13 %inc.7, i13* %arrayidx5.7, align 2, !tbaa !2
  %arrayidx5.8 = getelementptr inbounds [15 x i13], [15 x i13]* %a, i64 %indvars.iv, i64 8
  %8 = load i13, i13* %arrayidx5.8, align 2, !tbaa !2
  %inc.8 = add nsw i13 %8, 1
  store i13 %inc.8, i13* %arrayidx5.8, align 2, !tbaa !2
  %arrayidx5.9 = getelementptr inbounds [15 x i13], [15 x i13]* %a, i64 %indvars.iv, i64 9
  %9 = load i13, i13* %arrayidx5.9, align 2, !tbaa !2
  %inc.9 = add nsw i13 %9, 1
  store i13 %inc.9, i13* %arrayidx5.9, align 2, !tbaa !2
  %arrayidx5.10 = getelementptr inbounds [15 x i13], [15 x i13]* %a, i64 %indvars.iv, i64 10
  %10 = load i13, i13* %arrayidx5.10, align 2, !tbaa !2
  %inc.10 = add nsw i13 %10, 1
  store i13 %inc.10, i13* %arrayidx5.10, align 2, !tbaa !2
  %arrayidx5.11 = getelementptr inbounds [15 x i13], [15 x i13]* %a, i64 %indvars.iv, i64 11
  %11 = load i13, i13* %arrayidx5.11, align 2, !tbaa !2
  %inc.11 = add nsw i13 %11, 1
  store i13 %inc.11, i13* %arrayidx5.11, align 2, !tbaa !2
  %arrayidx5.12 = getelementptr inbounds [15 x i13], [15 x i13]* %a, i64 %indvars.iv, i64 12
  %12 = load i13, i13* %arrayidx5.12, align 2, !tbaa !2
  %inc.12 = add nsw i13 %12, 1
  store i13 %inc.12, i13* %arrayidx5.12, align 2, !tbaa !2
  %arrayidx5.13 = getelementptr inbounds [15 x i13], [15 x i13]* %a, i64 %indvars.iv, i64 13
  %13 = load i13, i13* %arrayidx5.13, align 2, !tbaa !2
  %inc.13 = add nsw i13 %13, 1
  store i13 %inc.13, i13* %arrayidx5.13, align 2, !tbaa !2
  %arrayidx5.14 = getelementptr inbounds [15 x i13], [15 x i13]* %a, i64 %indvars.iv, i64 14
  %14 = load i13, i13* %arrayidx5.14, align 2, !tbaa !2
  %inc.14 = add nsw i13 %14, 1
  store i13 %inc.14, i13* %arrayidx5.14, align 2, !tbaa !2
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 15
  br i1 %exitcond, label %for.end9, label %for.cond1.preheader

for.end9:                                         ; preds = %for.cond1.preheader
  ret void
}

attributes #0 = { norecurse nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-features"="+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 9.0.0 "}
!2 = !{!3, !3, i64 0}
!3 = !{!"omnipotent char", !4, i64 0}
!4 = !{!"Simple C++ TBAA"}

; ModuleID = 'kernel.ll'
source_filename = "kernel.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nounwind uwtable
define dso_local void @matmul(double*, double*, double*) #0 {
  br label %5

4:                                                ; preds = %27
  br label %28

5:                                                ; preds = %3, %27
  %indvars.iv11 = phi i64 [ 0, %3 ], [ %indvars.iv.next12, %27 ]
  br label %7

6:                                                ; preds = %25
  br label %26

7:                                                ; preds = %5, %25
  %indvars.iv7 = phi i64 [ 0, %5 ], [ %indvars.iv.next8, %25 ]
  br label %9

8:                                                ; preds = %20
  %.01.lcssa = phi double [ %19, %20 ]
  br label %21

9:                                                ; preds = %7, %20
  %indvars.iv = phi i64 [ 0, %7 ], [ %indvars.iv.next, %20 ]
  %.014 = phi double [ 0.000000e+00, %7 ], [ %19, %20 ]
  %10 = mul nuw nsw i64 %indvars.iv11, 128
  %11 = add nuw nsw i64 %10, %indvars.iv
  %12 = getelementptr inbounds double, double* %0, i64 %11
  %13 = load double, double* %12, align 8, !tbaa !2
  %14 = mul nuw nsw i64 %indvars.iv, 128
  %15 = add nuw nsw i64 %14, %indvars.iv7
  %16 = getelementptr inbounds double, double* %1, i64 %15
  %17 = load double, double* %16, align 8, !tbaa !2
  %18 = fmul double %13, %17
  %19 = fadd double %.014, %18
  br label %20

20:                                               ; preds = %9
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp ne i64 %indvars.iv.next, 128
  br i1 %exitcond, label %9, label %8

21:                                               ; preds = %8
  %22 = mul nuw nsw i64 %indvars.iv11, 128
  %23 = add nuw nsw i64 %22, %indvars.iv7
  %24 = getelementptr inbounds double, double* %2, i64 %23
  store double %.01.lcssa, double* %24, align 8, !tbaa !2
  br label %25

25:                                               ; preds = %21
  %indvars.iv.next8 = add nuw nsw i64 %indvars.iv7, 1
  %exitcond9 = icmp ne i64 %indvars.iv.next8, 128
  br i1 %exitcond9, label %7, label %6

26:                                               ; preds = %6
  br label %27

27:                                               ; preds = %26
  %indvars.iv.next12 = add nuw nsw i64 %indvars.iv11, 1
  %exitcond13 = icmp ne i64 %indvars.iv.next12, 128
  br i1 %exitcond13, label %5, label %4

28:                                               ; preds = %4
  ret void
}

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.start.p0i8(i64 immarg, i8* nocapture) #1

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.end.p0i8(i64 immarg, i8* nocapture) #1

; Function Attrs: nounwind uwtable
define dso_local void @interchange_demo(double* noalias, double* noalias) #0 {
  br label %4

3:                                                ; preds = %21
  br label %22

4:                                                ; preds = %2, %21
  %indvars.iv4 = phi i64 [ 0, %2 ], [ %indvars.iv.next5, %21 ]
  br label %6

5:                                                ; preds = %19
  br label %20

6:                                                ; preds = %4, %19
  %indvars.iv = phi i64 [ 0, %4 ], [ %indvars.iv.next, %19 ]
  %7 = mul nuw nsw i64 %indvars.iv, 128
  %8 = add nuw nsw i64 %7, %indvars.iv4
  %9 = getelementptr inbounds double, double* %0, i64 %8
  %10 = load double, double* %9, align 8, !tbaa !2
  %11 = mul nuw nsw i64 %indvars.iv, 128
  %12 = add nuw nsw i64 %11, %indvars.iv4
  %13 = getelementptr inbounds double, double* %1, i64 %12
  %14 = load double, double* %13, align 8, !tbaa !2
  %15 = fadd double %10, %14
  %16 = mul nuw nsw i64 %indvars.iv, 128
  %17 = add nuw nsw i64 %16, %indvars.iv4
  %18 = getelementptr inbounds double, double* %0, i64 %17
  store double %15, double* %18, align 8, !tbaa !2
  br label %19

19:                                               ; preds = %6
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp ne i64 %indvars.iv.next, 128
  br i1 %exitcond, label %6, label %5

20:                                               ; preds = %5
  br label %21

21:                                               ; preds = %20
  %indvars.iv.next5 = add nuw nsw i64 %indvars.iv4, 1
  %exitcond6 = icmp ne i64 %indvars.iv.next5, 128
  br i1 %exitcond6, label %4, label %3

22:                                               ; preds = %3
  ret void
}

attributes #0 = { nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { argmemonly nounwind }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 9.0.1 (https://github.com/llvm/llvm-project.git c1a0a213378a458fbea1a5c77b315c7dce08fd05)"}
!2 = !{!3, !3, i64 0}
!3 = !{!"double", !4, i64 0}
!4 = !{!"omnipotent char", !5, i64 0}
!5 = !{!"Simple C/C++ TBAA"}

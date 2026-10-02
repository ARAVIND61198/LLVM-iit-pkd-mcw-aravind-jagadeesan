; ModuleID = 'kernel.c'
source_filename = "kernel.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nounwind uwtable
define dso_local void @matmul(double*, double*, double*) #0 {
  %4 = alloca double*, align 8
  %5 = alloca double*, align 8
  %6 = alloca double*, align 8
  %7 = alloca i32, align 4
  %8 = alloca i32, align 4
  %9 = alloca i32, align 4
  %10 = alloca double, align 8
  %11 = alloca i32, align 4
  store double* %0, double** %4, align 8, !tbaa !2
  store double* %1, double** %5, align 8, !tbaa !2
  store double* %2, double** %6, align 8, !tbaa !2
  %12 = bitcast i32* %7 to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %12) #2
  store i32 0, i32* %7, align 4, !tbaa !6
  br label %13

13:                                               ; preds = %70, %3
  %14 = load i32, i32* %7, align 4, !tbaa !6
  %15 = icmp slt i32 %14, 128
  br i1 %15, label %18, label %16

16:                                               ; preds = %13
  store i32 2, i32* %8, align 4
  %17 = bitcast i32* %7 to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %17) #2
  br label %73

18:                                               ; preds = %13
  %19 = bitcast i32* %9 to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %19) #2
  store i32 0, i32* %9, align 4, !tbaa !6
  br label %20

20:                                               ; preds = %66, %18
  %21 = load i32, i32* %9, align 4, !tbaa !6
  %22 = icmp slt i32 %21, 128
  br i1 %22, label %25, label %23

23:                                               ; preds = %20
  store i32 5, i32* %8, align 4
  %24 = bitcast i32* %9 to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %24) #2
  br label %69

25:                                               ; preds = %20
  %26 = bitcast double* %10 to i8*
  call void @llvm.lifetime.start.p0i8(i64 8, i8* %26) #2
  store double 0.000000e+00, double* %10, align 8, !tbaa !8
  %27 = bitcast i32* %11 to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %27) #2
  store i32 0, i32* %11, align 4, !tbaa !6
  br label %28

28:                                               ; preds = %53, %25
  %29 = load i32, i32* %11, align 4, !tbaa !6
  %30 = icmp slt i32 %29, 128
  br i1 %30, label %33, label %31

31:                                               ; preds = %28
  store i32 8, i32* %8, align 4
  %32 = bitcast i32* %11 to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %32) #2
  br label %56

33:                                               ; preds = %28
  %34 = load double*, double** %4, align 8, !tbaa !2
  %35 = load i32, i32* %7, align 4, !tbaa !6
  %36 = mul nsw i32 %35, 128
  %37 = load i32, i32* %11, align 4, !tbaa !6
  %38 = add nsw i32 %36, %37
  %39 = sext i32 %38 to i64
  %40 = getelementptr inbounds double, double* %34, i64 %39
  %41 = load double, double* %40, align 8, !tbaa !8
  %42 = load double*, double** %5, align 8, !tbaa !2
  %43 = load i32, i32* %11, align 4, !tbaa !6
  %44 = mul nsw i32 %43, 128
  %45 = load i32, i32* %9, align 4, !tbaa !6
  %46 = add nsw i32 %44, %45
  %47 = sext i32 %46 to i64
  %48 = getelementptr inbounds double, double* %42, i64 %47
  %49 = load double, double* %48, align 8, !tbaa !8
  %50 = fmul double %41, %49
  %51 = load double, double* %10, align 8, !tbaa !8
  %52 = fadd double %51, %50
  store double %52, double* %10, align 8, !tbaa !8
  br label %53

53:                                               ; preds = %33
  %54 = load i32, i32* %11, align 4, !tbaa !6
  %55 = add nsw i32 %54, 1
  store i32 %55, i32* %11, align 4, !tbaa !6
  br label %28

56:                                               ; preds = %31
  %57 = load double, double* %10, align 8, !tbaa !8
  %58 = load double*, double** %6, align 8, !tbaa !2
  %59 = load i32, i32* %7, align 4, !tbaa !6
  %60 = mul nsw i32 %59, 128
  %61 = load i32, i32* %9, align 4, !tbaa !6
  %62 = add nsw i32 %60, %61
  %63 = sext i32 %62 to i64
  %64 = getelementptr inbounds double, double* %58, i64 %63
  store double %57, double* %64, align 8, !tbaa !8
  %65 = bitcast double* %10 to i8*
  call void @llvm.lifetime.end.p0i8(i64 8, i8* %65) #2
  br label %66

66:                                               ; preds = %56
  %67 = load i32, i32* %9, align 4, !tbaa !6
  %68 = add nsw i32 %67, 1
  store i32 %68, i32* %9, align 4, !tbaa !6
  br label %20

69:                                               ; preds = %23
  br label %70

70:                                               ; preds = %69
  %71 = load i32, i32* %7, align 4, !tbaa !6
  %72 = add nsw i32 %71, 1
  store i32 %72, i32* %7, align 4, !tbaa !6
  br label %13

73:                                               ; preds = %16
  ret void
}

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.start.p0i8(i64 immarg, i8* nocapture) #1

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.end.p0i8(i64 immarg, i8* nocapture) #1

; Function Attrs: nounwind uwtable
define dso_local void @interchange_demo(double* noalias, double* noalias) #0 {
  %3 = alloca double*, align 8
  %4 = alloca double*, align 8
  %5 = alloca i32, align 4
  %6 = alloca i32, align 4
  %7 = alloca i32, align 4
  store double* %0, double** %3, align 8, !tbaa !2
  store double* %1, double** %4, align 8, !tbaa !2
  %8 = bitcast i32* %5 to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %8) #2
  store i32 0, i32* %5, align 4, !tbaa !6
  br label %9

9:                                                ; preds = %50, %2
  %10 = load i32, i32* %5, align 4, !tbaa !6
  %11 = icmp slt i32 %10, 128
  br i1 %11, label %14, label %12

12:                                               ; preds = %9
  store i32 2, i32* %6, align 4
  %13 = bitcast i32* %5 to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %13) #2
  br label %53

14:                                               ; preds = %9
  %15 = bitcast i32* %7 to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %15) #2
  store i32 0, i32* %7, align 4, !tbaa !6
  br label %16

16:                                               ; preds = %46, %14
  %17 = load i32, i32* %7, align 4, !tbaa !6
  %18 = icmp slt i32 %17, 128
  br i1 %18, label %21, label %19

19:                                               ; preds = %16
  store i32 5, i32* %6, align 4
  %20 = bitcast i32* %7 to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %20) #2
  br label %49

21:                                               ; preds = %16
  %22 = load double*, double** %3, align 8, !tbaa !2
  %23 = load i32, i32* %7, align 4, !tbaa !6
  %24 = mul nsw i32 %23, 128
  %25 = load i32, i32* %5, align 4, !tbaa !6
  %26 = add nsw i32 %24, %25
  %27 = sext i32 %26 to i64
  %28 = getelementptr inbounds double, double* %22, i64 %27
  %29 = load double, double* %28, align 8, !tbaa !8
  %30 = load double*, double** %4, align 8, !tbaa !2
  %31 = load i32, i32* %7, align 4, !tbaa !6
  %32 = mul nsw i32 %31, 128
  %33 = load i32, i32* %5, align 4, !tbaa !6
  %34 = add nsw i32 %32, %33
  %35 = sext i32 %34 to i64
  %36 = getelementptr inbounds double, double* %30, i64 %35
  %37 = load double, double* %36, align 8, !tbaa !8
  %38 = fadd double %29, %37
  %39 = load double*, double** %3, align 8, !tbaa !2
  %40 = load i32, i32* %7, align 4, !tbaa !6
  %41 = mul nsw i32 %40, 128
  %42 = load i32, i32* %5, align 4, !tbaa !6
  %43 = add nsw i32 %41, %42
  %44 = sext i32 %43 to i64
  %45 = getelementptr inbounds double, double* %39, i64 %44
  store double %38, double* %45, align 8, !tbaa !8
  br label %46

46:                                               ; preds = %21
  %47 = load i32, i32* %7, align 4, !tbaa !6
  %48 = add nsw i32 %47, 1
  store i32 %48, i32* %7, align 4, !tbaa !6
  br label %16

49:                                               ; preds = %19
  br label %50

50:                                               ; preds = %49
  %51 = load i32, i32* %5, align 4, !tbaa !6
  %52 = add nsw i32 %51, 1
  store i32 %52, i32* %5, align 4, !tbaa !6
  br label %9

53:                                               ; preds = %12
  ret void
}

attributes #0 = { nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { argmemonly nounwind }
attributes #2 = { nounwind }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 9.0.1 (https://github.com/llvm/llvm-project.git c1a0a213378a458fbea1a5c77b315c7dce08fd05)"}
!2 = !{!3, !3, i64 0}
!3 = !{!"any pointer", !4, i64 0}
!4 = !{!"omnipotent char", !5, i64 0}
!5 = !{!"Simple C/C++ TBAA"}
!6 = !{!7, !7, i64 0}
!7 = !{!"int", !4, i64 0}
!8 = !{!9, !9, i64 0}
!9 = !{!"double", !4, i64 0}

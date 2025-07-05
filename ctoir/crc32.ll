; ModuleID = 'crc32.c'
source_filename = "crc32.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-i128:128-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux-gnu"

; Function Attrs: noinline nounwind optnone uwtable
define dso_local i32 @crc32(ptr noundef %0, i32 noundef %1) #0 {
  %3 = alloca ptr, align 8
  %4 = alloca i32, align 4
  %5 = alloca i32, align 4
  %6 = alloca i32, align 4
  %7 = alloca i32, align 4
  %8 = alloca i32, align 4
  %9 = alloca i32, align 4
  store ptr %0, ptr %3, align 8
  store i32 %1, ptr %4, align 4
  store i32 0, ptr %5, align 4
  store i32 -1, ptr %8, align 4
  br label %10

10:                                               ; preds = %39, %2
  %11 = load i32, ptr %5, align 4
  %12 = load i32, ptr %4, align 4
  %13 = icmp ult i32 %11, %12
  br i1 %13, label %14, label %42

14:                                               ; preds = %10
  %15 = load ptr, ptr %3, align 8
  %16 = load i32, ptr %5, align 4
  %17 = sext i32 %16 to i64
  %18 = getelementptr inbounds i8, ptr %15, i64 %17
  %19 = load i8, ptr %18, align 1
  %20 = zext i8 %19 to i32
  store i32 %20, ptr %7, align 4
  %21 = load i32, ptr %8, align 4
  %22 = load i32, ptr %7, align 4
  %23 = xor i32 %21, %22
  store i32 %23, ptr %8, align 4
  store i32 7, ptr %6, align 4
  br label %24

24:                                               ; preds = %36, %14
  %25 = load i32, ptr %6, align 4
  %26 = icmp sge i32 %25, 0
  br i1 %26, label %27, label %39

27:                                               ; preds = %24
  %28 = load i32, ptr %8, align 4
  %29 = and i32 %28, 1
  %30 = sub i32 0, %29
  store i32 %30, ptr %9, align 4
  %31 = load i32, ptr %8, align 4
  %32 = lshr i32 %31, 1
  %33 = load i32, ptr %9, align 4
  %34 = and i32 -306674912, %33
  %35 = xor i32 %32, %34
  store i32 %35, ptr %8, align 4
  br label %36

36:                                               ; preds = %27
  %37 = load i32, ptr %6, align 4
  %38 = add nsw i32 %37, -1
  store i32 %38, ptr %6, align 4
  br label %24, !llvm.loop !6

39:                                               ; preds = %24
  %40 = load i32, ptr %5, align 4
  %41 = add nsw i32 %40, 1
  store i32 %41, ptr %5, align 4
  br label %10, !llvm.loop !8

42:                                               ; preds = %10
  %43 = load i32, ptr %8, align 4
  %44 = xor i32 %43, -1
  ret i32 %44
}

attributes #0 = { noinline nounwind optnone uwtable "frame-pointer"="all" "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }

!llvm.module.flags = !{!0, !1, !2, !3, !4}
!llvm.ident = !{!5}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 8, !"PIC Level", i32 2}
!2 = !{i32 7, !"PIE Level", i32 2}
!3 = !{i32 7, !"uwtable", i32 2}
!4 = !{i32 7, !"frame-pointer", i32 2}
!5 = !{!"Debian clang version 19.1.7 (3)"}
!6 = distinct !{!6, !7}
!7 = !{!"llvm.loop.mustprogress"}
!8 = distinct !{!8, !7}

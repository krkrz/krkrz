//---------------------------------------------------------------------------
/*
	TVP2 ( T Visual Presenter 2 )  A script authoring tool
	Copyright (C) 2000 W.Dee <dee@kikyou.info> and contributors

	See details of license at "license.txt"
*/
//---------------------------------------------------------------------------
// CPU idetification / features detection routine
//---------------------------------------------------------------------------
#ifndef DetectCPUH
#define DetectCPUH
//---------------------------------------------------------------------------

#include <cpu-features.h>

extern "C"
{
	extern tjs_uint32 TVPCPUType;
}

#define TVP_CPU_FAMILY_ARM      ANDROID_CPU_FAMILY_ARM
#define TVP_CPU_FAMILY_X86      ANDROID_CPU_FAMILY_X86
#define TVP_CPU_FAMILY_MIPS     ANDROID_CPU_FAMILY_MIPS
#define TVP_CPU_FAMILY_ARM64    ANDROID_CPU_FAMILY_ARM64
#define TVP_CPU_FAMILY_X86_64   ANDROID_CPU_FAMILY_X86_64
#define TVP_CPU_FAMILY_MIPS64   ANDROID_CPU_FAMILY_MIPS64
#define TVP_CPU_FAMILY_MASK     0x0000000f


#define TVP_CPU_HAS_SSSE3    (ANDROID_CPU_X86_FEATURE_SSSE3 << 8)
#define TVP_CPU_HAS_POPCNT   (ANDROID_CPU_X86_FEATURE_POPCNT << 8)
#define TVP_CPU_HAS_MOVBE    (ANDROID_CPU_X86_FEATURE_MOVBE << 8)
#define TVP_CPU_HAS_CMOV     0x00001000		// always support for android
#define TVP_CPU_HAS_MMX      0x00002000		// always support for android
#define TVP_CPU_HAS_SSE      0x00004000		// always support for android
#define TVP_CPU_HAS_SSE2     0x00008000		// always support for android
#define TVP_CPU_HAS_AVX2     0x00010000		// not used on android

#define TVP_CPU_HAS_ARM_ARMv7        (ANDROID_CPU_ARM_FEATURE_ARMv7 << 8)
#define TVP_CPU_HAS_ARM_VFPv3        (ANDROID_CPU_ARM_FEATURE_VFPv3 << 8)
#define TVP_CPU_HAS_ARM_NEON         (ANDROID_CPU_ARM_FEATURE_NEON << 8)
#define TVP_CPU_HAS_ARM_LDREX_STREX  (ANDROID_CPU_ARM_FEATURE_LDREX_STREX << 8)
#define TVP_CPU_HAS_ARM_VPFv2        (ANDROID_CPU_ARM_FEATURE_VFPv2 << 8)
#define TVP_CPU_HAS_ARM_VFPD32       (ANDROID_CPU_ARM_FEATURE_VFP_D32 << 8)
#define TVP_CPU_HAS_ARM_VFP_FP16     (ANDROID_CPU_ARM_FEATURE_VFP_FP16 << 8)
#define TVP_CPU_HAS_ARM_VFP_FMA      (ANDROID_CPU_ARM_FEATURE_VFP_FMA << 8)
#define TVP_CPU_HAS_ARM_NEON_FMA     (ANDROID_CPU_ARM_FEATURE_NEON_FMA << 8)
#define TVP_CPU_HAS_ARM_IDIV_ARM     (ANDROID_CPU_ARM_FEATURE_IDIV_ARM << 8)
#define TVP_CPU_HAS_ARM_IDIV_THUM2   (ANDROID_CPU_ARM_FEATURE_IDIV_THUMB2 << 8)
#define TVP_CPU_HAS_ARM_iWMMXt       (ANDROID_CPU_ARM_FEATURE_iWMMXt << 8)
#define TVP_CPU_HAS_ARM_AES          (ANDROID_CPU_ARM_FEATURE_AES << 8)
#define TVP_CPU_HAS_ARM_PMULL        (ANDROID_CPU_ARM_FEATURE_PMULL << 8)
#define TVP_CPU_HAS_ARM_SHA1         (ANDROID_CPU_ARM_FEATURE_SHA1 << 8)
#define TVP_CPU_HAS_ARM_SHA2         (ANDROID_CPU_ARM_FEATURE_SHA2 << 8)
#define TVP_CPU_HAS_ARM_CRC32        (ANDROID_CPU_ARM_FEATURE_CRC32 << 8)

#define TVP_CPU_HAS_ARM64_FP         (ANDROID_CPU_ARM64_FEATURE_FP << 8)
#define TVP_CPU_HAS_ARM64_ASIMD      (ANDROID_CPU_ARM64_FEATURE_ASIMD << 8)
#define TVP_CPU_HAS_ARM64_AES        (ANDROID_CPU_ARM64_FEATURE_AES << 8)
#define TVP_CPU_HAS_ARM64_PMULL      (ANDROID_CPU_ARM64_FEATURE_PMULL << 8)
#define TVP_CPU_HAS_ARM64_SHA1       (ANDROID_CPU_ARM64_FEATURE_SHA1 << 8)
#define TVP_CPU_HAS_ARM64_SHA2       (ANDROID_CPU_ARM64_FEATURE_SHA2 << 8)
#define TVP_CPU_HAS_ARM64_CRC32      (ANDROID_CPU_ARM64_FEATURE_CRC32 << 8)

#define TVP_CPU_HAS_MIPS_R6     (ANDROID_CPU_MIPS_FEATURE_R6 << 8)
#define TVP_CPU_HAS_MIPS_MSA    (ANDROID_CPU_MIPS_FEATURE_MSA << 8)

#define TVP_CPU_FEATURE_MASK 0xffffff00


extern void TVPDetectCPU();
TJS_EXP_FUNC_DEF(tjs_uint32, TVPGetCPUType, ());
extern tjs_int TVPGetCPUFamily();

//---------------------------------------------------------------------------
#endif

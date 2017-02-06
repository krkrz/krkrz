//---------------------------------------------------------------------------
/*
	TVP2 ( T Visual Presenter 2 )  A script authoring tool
	Copyright (C) 2000 W.Dee <dee@kikyou.info> and contributors

	See details of license at "license.txt"
*/
//---------------------------------------------------------------------------
// CPU idetification / features detection routine
//---------------------------------------------------------------------------
#include "tjsCommHead.h"

#include "DetectCPU.h"
#include "DebugIntf.h"
#include "SysInitIntf.h"

#include "ThreadIntf.h"

#include "Exception.h"
#include "MsgIntf.h"

/*
	Note: CPU clock measuring routine is in EmergencyExit.cpp, reusing
	hot-key watching thread.
*/

//---------------------------------------------------------------------------
extern "C"
{
	tjs_uint32 TVPCPUType = 0; // CPU type
	tjs_uint32 TVPCPUFeatures = 0;
}

static bool TVPCPUChecked = false;
//---------------------------------------------------------------------------





//---------------------------------------------------------------------------
// TVPGetCPUTypeForOne
//---------------------------------------------------------------------------
static void TVPGetCPUTypeForOne()
{
	AndroidCpuFamily family = android_getCpuFamily();
	uint64_t features = android_getCpuFeatures();
	TVPCPUFeatures = static_cast<tjs_uint32>((features << 8) & 0xffffff00) | static_cast<tjs_uint32>(family);
	if( family == TVP_CPU_FAMILY_X86 || family == TVP_CPU_FAMILY_X86_64 ) {
		TVPCPUFeatures |= TVP_CPU_HAS_CMOV | TVP_CPU_HAS_MMX | TVP_CPU_HAS_SSE | TVP_CPU_HAS_SSE2;
	}
}
//---------------------------------------------------------------------------




//---------------------------------------------------------------------------
// tTVPCPUCheckThread
//---------------------------------------------------------------------------
class tTVPCPUCheckThread : public tTVPThread
{
	bool Succeeded;
public:
	tTVPCPUCheckThread(tjs_uint32 tam)
	{
//		Succeeded = true;
//		SetThreadAffinityMask(GetHandle(), tam);

		StartTread();
	}

	~tTVPCPUCheckThread()
	{
	}

	void WaitEnd()
	{
		Terminate();
		WaitFor();
	}

	bool GetSucceeded() const { return Succeeded; }

	void Execute(void);
} ;
//---------------------------------------------------------------------------
void tTVPCPUCheckThread::Execute(void)
{
	try
	{
		TVPGetCPUTypeForOne();
	}
	catch(...)
	{
		Succeeded = false;
	}
}
//---------------------------------------------------------------------------





//---------------------------------------------------------------------------
static ttstr TVPDumpCPUFeatures(tjs_uint32 features)
{
	ttstr ret;
#define TVP_DUMP_CPU(x, n) { ret += TJS_W("  ") TJS_W(n);  \
	if(features & x) ret += TJS_W(":yes"); else ret += TJS_W(":no"); }

	tjs_uint32 family = TVPCPUFeatures & TVP_CPU_FAMILY_MASK;
	switch( family ) {
	case TVP_CPU_FAMILY_X86:
	case TVP_CPU_FAMILY_X86_64:
		TVP_DUMP_CPU(TVP_CPU_HAS_MMX, "MMX");
		TVP_DUMP_CPU(TVP_CPU_HAS_SSE, "SSE");
		TVP_DUMP_CPU(TVP_CPU_HAS_CMOV, "CMOVcc");
		TVP_DUMP_CPU(TVP_CPU_HAS_SSE2, "SSE2");
		TVP_DUMP_CPU(TVP_CPU_HAS_SSSE3, "SSSE3");
		TVP_DUMP_CPU(TVP_CPU_HAS_POPCNT, "POPCNT");
		TVP_DUMP_CPU(TVP_CPU_HAS_MOVBE, "MOVBE");
		break;
	case TVP_CPU_FAMILY_ARM:
		TVP_DUMP_CPU(TVP_CPU_HAS_ARM_ARMv7, "ARVv7");
		TVP_DUMP_CPU(TVP_CPU_HAS_ARM_VFPv3, "VFPv3");
		TVP_DUMP_CPU(TVP_CPU_HAS_ARM_NEON, "NEON");
		TVP_DUMP_CPU(TVP_CPU_HAS_ARM_LDREX_STREX, "LDREX STREX");
		TVP_DUMP_CPU(TVP_CPU_HAS_ARM_VPFv2, "VPFv2");
		TVP_DUMP_CPU(TVP_CPU_HAS_ARM_VFPD32, "VFPD32");
		TVP_DUMP_CPU(TVP_CPU_HAS_ARM_VFP_FP16, "VFP FP16");
		TVP_DUMP_CPU(TVP_CPU_HAS_ARM_VFP_FMA, "VFP FMA");
		TVP_DUMP_CPU(TVP_CPU_HAS_ARM_NEON_FMA, "NEON FMA");
		TVP_DUMP_CPU(TVP_CPU_HAS_ARM_IDIV_ARM, "IDIV ARM");
		TVP_DUMP_CPU(TVP_CPU_HAS_ARM_IDIV_THUM2, "IDIV THUM2");
		TVP_DUMP_CPU(TVP_CPU_HAS_ARM_iWMMXt, "iWMMXt");
		TVP_DUMP_CPU(TVP_CPU_HAS_ARM_AES, "AES");
		TVP_DUMP_CPU(TVP_CPU_HAS_ARM_PMULL, "PMULL");
		TVP_DUMP_CPU(TVP_CPU_HAS_ARM_SHA1, "SHA1");
		TVP_DUMP_CPU(TVP_CPU_HAS_ARM_SHA2, "SHA2");
		TVP_DUMP_CPU(TVP_CPU_HAS_ARM_CRC32, "CRC32");
		break;
	case TVP_CPU_FAMILY_ARM64:
		TVP_DUMP_CPU(TVP_CPU_HAS_ARM64_FP, "FP");
		TVP_DUMP_CPU(TVP_CPU_HAS_ARM64_ASIMD, "ASIMD");
		TVP_DUMP_CPU(TVP_CPU_HAS_ARM64_AES, "AES");
		TVP_DUMP_CPU(TVP_CPU_HAS_ARM64_PMULL, "PMULL");
		TVP_DUMP_CPU(TVP_CPU_HAS_ARM64_SHA1, "SHA1");
		TVP_DUMP_CPU(TVP_CPU_HAS_ARM64_SHA2, "SHA2");
		TVP_DUMP_CPU(TVP_CPU_HAS_ARM64_CRC32, "CRC32");
		break;
	case TVP_CPU_FAMILY_MIPS:
	case TVP_CPU_FAMILY_MIPS64:
		TVP_DUMP_CPU(TVP_CPU_HAS_MIPS_R6, "R6");
		TVP_DUMP_CPU(TVP_CPU_HAS_MIPS_MSA, "MSA");
		break;
	}

	return ret;
}
//---------------------------------------------------------------------------
static ttstr TVPDumpCPUInfo(tjs_int cpu_num)
{
	// dump detected cpu type
	ttstr features( TVPFormatMessage(TVPInfoCpuNumber,ttstr(cpu_num)) );

	features += TVPDumpCPUFeatures(TVPCPUFeatures);

	tjs_uint32 family = TVPCPUFeatures & TVP_CPU_FAMILY_MASK;

#undef TVP_DUMP_CPU
#define TVP_DUMP_CPU(x, n) { \
	if(family == x) features += TJS_W("  ") TJS_W(n); }

	TVP_DUMP_CPU(TVP_CPU_FAMILY_ARM, "ARM");
	TVP_DUMP_CPU(TVP_CPU_FAMILY_X86, "x86");
	TVP_DUMP_CPU(TVP_CPU_FAMILY_MIPS, "MIPS");
	TVP_DUMP_CPU(TVP_CPU_FAMILY_ARM64, "ARM64");
	TVP_DUMP_CPU(TVP_CPU_FAMILY_X86_64, "x64");
	TVP_DUMP_CPU(TVP_CPU_FAMILY_MIPS64, "MIPS64");

#undef TVP_DUMP_CPU

	TVPAddImportantLog(features);
	return features;
}
//---------------------------------------------------------------------------





//---------------------------------------------------------------------------
// TVPDetectCPU
//---------------------------------------------------------------------------
static void TVPDisableCPU(tjs_uint32 featurebit, const tjs_char *name)
{
	tTJSVariant val;
	ttstr str;
	if(TVPGetCommandLine(name, &val))
	{
		str = val;
		if(str == TJS_W("no"))
			TVPCPUType &=~ featurebit;
		else if(str == TJS_W("force"))
			TVPCPUType |= featurebit;
	}
}
//---------------------------------------------------------------------------
void TVPDetectCPU()
{
	if(TVPCPUChecked) return;
	TVPCPUChecked = true;

	// cannot use affinity mask on android

	// for each CPU...
	ttstr cpuinfo;
	tjs_uint32 features = 0;

	tTVPCPUCheckThread * thread = new tTVPCPUCheckThread(1);
	thread->WaitEnd();
	bool succeeded = thread->GetSucceeded();
	delete thread;
	if(!succeeded) throw Exception(TJS_W("CPU check failure"));
	cpuinfo += TVPDumpCPUInfo(0) + TJS_W("\r\n");
	features =  (TVPCPUFeatures & TVP_CPU_FEATURE_MASK);
	TVPCPUType = TVPCPUFeatures;

	TVPCPUType &= ~ TVP_CPU_FEATURE_MASK;
	TVPCPUType |= features;

	// Disable or enable cpu features by option
	tjs_uint32 family = TVPCPUFeatures & TVP_CPU_FAMILY_MASK;
	switch( family ) {
	case TVP_CPU_FAMILY_X86:
	case TVP_CPU_FAMILY_X86_64:
		TVPDisableCPU(TVP_CPU_HAS_MMX,  TJS_W("-cpummx"));
		TVPDisableCPU(TVP_CPU_HAS_SSE,  TJS_W("-cpusse"));
		TVPDisableCPU(TVP_CPU_HAS_CMOV, TJS_W("-cpucmov"));
		TVPDisableCPU(TVP_CPU_HAS_SSE2, TJS_W("-cpusse2"));
		TVPDisableCPU(TVP_CPU_HAS_SSSE3, TJS_W("-cpussse3"));
		TVPDisableCPU(TVP_CPU_HAS_POPCNT, TJS_W("-cpupopcnt"));
		TVPDisableCPU(TVP_CPU_HAS_MOVBE, TJS_W("-cpumovbe"));
		break;
	case TVP_CPU_FAMILY_ARM:
		TVPDisableCPU(TVP_CPU_HAS_ARM_ARMv7, TJS_W("-cpuarvv7"));
		TVPDisableCPU(TVP_CPU_HAS_ARM_VFPv3, TJS_W("-cpuvfpv3"));
		TVPDisableCPU(TVP_CPU_HAS_ARM_NEON, TJS_W("-cpuneon"));
		TVPDisableCPU(TVP_CPU_HAS_ARM_LDREX_STREX, TJS_W("-cpuldrex_strex"));
		TVPDisableCPU(TVP_CPU_HAS_ARM_VPFv2, TJS_W("-cpuvpfv2"));
		TVPDisableCPU(TVP_CPU_HAS_ARM_VFPD32, TJS_W("-cpuvfpd32"));
		TVPDisableCPU(TVP_CPU_HAS_ARM_VFP_FP16, TJS_W("-cpuvfp_fp16"));
		TVPDisableCPU(TVP_CPU_HAS_ARM_VFP_FMA, TJS_W("-cpuvfp_fma"));
		TVPDisableCPU(TVP_CPU_HAS_ARM_NEON_FMA, TJS_W("-cpuneon_fma"));
		TVPDisableCPU(TVP_CPU_HAS_ARM_IDIV_ARM, TJS_W("-cpuidiv_arm"));
		TVPDisableCPU(TVP_CPU_HAS_ARM_IDIV_THUM2, TJS_W("-cpuidiv_thum2"));
		TVPDisableCPU(TVP_CPU_HAS_ARM_iWMMXt, TJS_W("-cpuiwmmxt"));
		TVPDisableCPU(TVP_CPU_HAS_ARM_AES, TJS_W("-cpuaes"));
		TVPDisableCPU(TVP_CPU_HAS_ARM_PMULL, TJS_W("-cpupmull"));
		TVPDisableCPU(TVP_CPU_HAS_ARM_SHA1, TJS_W("-cpusha1"));
		TVPDisableCPU(TVP_CPU_HAS_ARM_SHA2, TJS_W("-cpusha2"));
		TVPDisableCPU(TVP_CPU_HAS_ARM_CRC32, TJS_W("-cpucrc32"));
		break;
	case TVP_CPU_FAMILY_ARM64:
		TVPDisableCPU(TVP_CPU_HAS_ARM64_FP, TJS_W("-cpufp"));
		TVPDisableCPU(TVP_CPU_HAS_ARM64_ASIMD, TJS_W("-cpuasimd"));
		TVPDisableCPU(TVP_CPU_HAS_ARM64_AES, TJS_W("-cpuaes"));
		TVPDisableCPU(TVP_CPU_HAS_ARM64_PMULL, TJS_W("-cpupmull"));
		TVPDisableCPU(TVP_CPU_HAS_ARM64_SHA1, TJS_W("-cpusha1"));
		TVPDisableCPU(TVP_CPU_HAS_ARM64_SHA2, TJS_W("-cpusha2"));
		TVPDisableCPU(TVP_CPU_HAS_ARM64_CRC32, TJS_W("-cpucrc32"));
		break;
	case TVP_CPU_FAMILY_MIPS:
	case TVP_CPU_FAMILY_MIPS64:
		TVPDisableCPU(TVP_CPU_HAS_MIPS_R6, TJS_W("-cpur6"));
		TVPDisableCPU(TVP_CPU_HAS_MIPS_MSA, TJS_W("-cpumsa"));
		break;
	}

	if(TVPCPUType == 0)
		throw Exception( TVPFormatMessage(TVPCpuCheckFailureNotSupprtedCpu, cpuinfo).c_str() );

	TVPAddImportantLog( TVPFormatMessage(TVPInfoFinallyDetectedCpuFeatures,TVPDumpCPUFeatures(TVPCPUType)) );
}
//---------------------------------------------------------------------------




//---------------------------------------------------------------------------
// TVPGetCPUType
//---------------------------------------------------------------------------
tjs_uint32 TVPGetCPUType()
{
	TVPDetectCPU();
	return TVPCPUType;
}
//---------------------------------------------------------------------------


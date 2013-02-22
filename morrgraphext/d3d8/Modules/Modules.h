
#pragma once

#undef MODULENAME
#include "Modules\MiscPatches\ExtensionMod.h"
#undef MODULENAME
#include "Modules\MWTalkyMod\ExtensionMod.h"
#undef MODULENAME
#include "Modules\MorrTextMod\ExtensionMod.h"
#undef MODULENAME
#include "Modules\MWCodePatch\ExtensionMod.h"
#undef MODULENAME

//====================================================================================
// Redefinable
#undef  FMODHIGHPRI
#undef  MOD_VERSION
#undef  MOD_PARAMETERS
#undef  MOD_NAME
#define FMODHIGHPRI      0x80000000
#define	MOD_VERSION	     0x0001010A
#define	MOD_PARAMETERS   FMODHIGHPRI
#define	MOD_NAME         "Morrowind Graphics Extender"  
//---------------------------------------------------------------------------  

#ifndef MODULENAME
#define DEFLOGPROC Logger::LogToFile
#define MODULENAME MorrGraphExt   // No spaces allowed
#define LogMsg(msg,...) _LogLineM(MODULENAME::THISMODNAME,msg,__VA_ARGS__) 
#else
#define DEFLOGPROC Logger::LogToProc
#define LogMsg(msg,...) _LogLineM(NULL,msg,__VA_ARGS__) 
#endif

//--------------------------------------------------------------------------- 
static void _stdcall LoadModules(HMODULE hDllMod)
{
 TroubleFixerMod::CreateModule(hDllMod);
// MorrTextMod::CreateModule(hDllMod);
 MWCodePatchMod::CreateModule(hDllMod);
 TalkyMorrowind::CreateModule(hDllMod);
}
//---------------------------------------------------------------------------  


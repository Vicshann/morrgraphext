
#pragma once

#include "CommonFuncs.h"
#include "MemWork.h"
#include "HookTools.h"

//====================================================================================
// Redefinable
#undef  FMODHIGHPRI
#undef  MOD_VERSION
#undef  MOD_PARAMETERS
#undef  MOD_NAME
#define FMODHIGHPRI      0x80000000
#define	MOD_VERSION		 0x0001010A
#define	MOD_PARAMETERS   0
#define	MOD_NAME         "Morrowind Code Patch"  // Spaces allowed - used in MLDR
//---------------------------------------------------------------------------  
  
#ifndef MODULENAME
#define DLLMAINTYPE static
#define DEFLOGPROC Logger::LogToFile
#define MODULENAME MWCodePatchMod   // No spaces allowed
#define LogMsg(msg,...) _LogLineM(MODULENAME::THISMODNAME,msg,__VA_ARGS__) 
#else
#define DLLMAINTYPE
#define DEFLOGPROC Logger::LogToProc
#define LogMsg(msg,...) _LogLineM(NULL,msg,__VA_ARGS__) 
#endif

namespace MODULENAME
{  
const LPSTR THISMODNAME    = _STRNG(MODULENAME);    
const LPSTR MLDRLIBNAME    = "MLDRLib.dll";
const LPSTR MLDRLOGPROC    = "WriteLogFile";
const LPSTR MLDRPROCNAME   = "IsModuleLoader";
const LPSTR SHELLCFGNAME   = "MWCPCfg.exe";		// For call from Moduleloader
const LPSTR PATCHFILENAME  = "patch";
const LPSTR CONFIGINIFILE  = "MWCodePatch.ini";
const LPSTR SIGNATUREFILE  = "MWCPSignatures.ini";
const LPSTR SINGERRORMSGRU = "Œ‰Ì‡ ËÁ ÒË„Ì‡ÚÛ ÌÂ ÒÓ‚Ô‡‰‡ÂÚ.\r\nÕÂ Ú‡ ‚ÂÒËˇ 'Morrowind.exe'? (ÕÛÊÌ‡: 1.6.1820).\r\nœ–ŒƒŒÀ∆≈Õ»≈ –¿¡Œ“€ Õ≈¬Œ«ÃŒ∆ÕŒ!";
const LPSTR SINGERRORMSGEN = "One of a signatures is differs.\r\nWrong version of 'Morrowind.exe'? (Need: 1.6.1820).\r\nCONTINUE IS IMPOSSIBLE!";

//--------------------------------------------------------------------------- 
// Standart export
//
DWORD _stdcall ModGetVersion(void);
DWORD _stdcall ModGetParameters(void); 
LPSTR _stdcall ModGetModuleName(void);        
DWORD _stdcall ModEventOnHostExit(void);      
DWORD _stdcall ModEventOnHostStart(void);
LPSTR _stdcall ModGetConfigShellName(void);
DWORD _stdcall ModRunConfigShell(DWORD Reserved);
void  _stdcall ModWriteLogFile(LPCTSTR LogMessage);

//--------------------------------------------------------------------------- 
// Module specific
//
void  _stdcall ApplyPatches(LPSTR CfgFileName, LPSTR SigFileName, LPSTR PatFileName);
bool  _stdcall IsPatchSignaturesOk(LPSTR SigFileName, PVOID PatchBlock, DWORD BlockSize, DWORD PatchId);
DWORD _stdcall WritePatches(PVOID PatchBlock, DWORD BlockSize, DWORD PatchId);
DWORD _stdcall GetEnabledPatches(LPSTR CfgFile, PDWORD PatchTable);
DWORD _stdcall GetPatchesCount(PVOID PatchBlock, DWORD BlockSize);

//--------------------------------------------------------------------------- 
struct MWPATCH
{
 DWORD PatchID;
 DWORD PatchOffset;
 DWORD PatchLength;
 BYTE  PatchData[];
};
//--------------------------------------------------------------------------- 
struct SModData  
{
 HMODULE      ExeModuleBase;
 HMODULE      ThisLibBase;
 HANDLE       DefHeapHandle;
 BOOL         ModuleLoader;
 BYTE         GameCurDir[MAX_PATH];
 BYTE         CurrentDir[MAX_PATH];
 BYTE         ConfigAppName[64];
//---------------------------------
 
 void* operator new(size_t size){return MEMWORK::GetMemoryBlock(sizeof(SModData));}  // Sets memory to ZERO
 void  operator delete(void* mem){MEMWORK::FreeMemoryBlock(mem);}    
//---------------------------------

 SModData(void)     
  {
   
  }
//---------------------------------
 ~SModData(void) 
  {

  }

};
typedef SModData*  PSModData;
//------------------------------------------------------------------------------------

PSModData _stdcall CreateModule(HMODULE hDllMod);  
PSModData _stdcall DestroyModule(void);            

//====================================================================================
}

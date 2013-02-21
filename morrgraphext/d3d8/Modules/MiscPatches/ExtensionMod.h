
#pragma once

#include "CommonFuncs.h"
#include "MemWork.h"
#include "HookTools.h"
#include "Containers.h"
#include "Morrowind.h"

//====================================================================================
// Redefinable
#undef  FMODHIGHPRI
#undef  MOD_VERSION
#undef  MOD_PARAMETERS
#undef  MOD_NAME
#define FMODHIGHPRI      0x80000000
#define	MOD_VERSION		 0x0001010A
#define	MOD_PARAMETERS   FMODHIGHPRI
#define	MOD_NAME         "Trouble Fixer"  // Spaces allowed - used in MLDR
//---------------------------------------------------------------------------  

#ifndef MODULENAME
#define DLLMAINTYPE static
#define DEFLOGPROC Logger::LogToFile
#define MODULENAME TroubleFixerMod   // No spaces allowed
#define LogMsg(msg,...) _LogLineM(MODULENAME::THISMODNAME,msg,__VA_ARGS__) 
#else
#define DLLMAINTYPE
#define DEFLOGPROC Logger::LogToProc
#define LogMsg(msg,...) _LogLineM(NULL,msg,__VA_ARGS__) 
#endif

//---------------------------------------------------------------------------  
namespace MODULENAME
{
const LPSTR THISMODNAME    = _STRNG(MODULENAME);
const LPSTR RUSTEXTMODN    = "Text.dll";
const LPSTR INISECCONFIG   = "BaseConfig";
const LPSTR INISECFIXES	   = "UsedFixes";
const LPSTR MLDRLIBNAME    = "MLDRLib.dll";
const LPSTR MLDRLOGPROC    = "WriteLogFile";
const LPSTR MLDRPROCNAME   = "IsModuleLoader";
const LPSTR CONFIGINIFILE  = "TrFixerCfg.ini";
const LPSTR CRASHLOGFILE   = "CrashLog.txt";
const LPSTR SHELLCFGNAME   = "TrFixerCfg.exe";		// For call from Moduleloader
const LPSTR SINGERRORMSGRU = "Îäíà èç ñèãíàòóð íå ñîâïàäàåò.\r\nÍå òà âåðñèÿ 'Morrowind.exe'? (Íóæíà: 1.6.1820).\r\nÏÐÎÄÎËÆÅÍÈÅ ÐÀÁÎÒÛ ÍÅÂÎÇÌÎÆÍÎ!";
const LPSTR SINGERRORMSGEN = "One of a signatures is differs.\r\nWrong version of 'Morrowind.exe'? (Need: 1.6.1820).\r\nCONTINUE IS IMPOSSIBLE!";

//------------------------------------------------------------------------------------
// Standart export
//
DWORD _stdcall ModGetVersion(void);
DWORD _stdcall ModGetParameters(void);  
LPSTR _stdcall ModGetModuleName(void);      
DWORD _stdcall ModEventOnHostExit(void);       // Call this internally from DLL_PROCESS_DETACH
DWORD _stdcall ModEventOnHostStart(void);
LPSTR _stdcall ModGetConfigShellName(void);
DWORD _stdcall ModRunConfigShell(DWORD Reserved);
void  _stdcall ModWriteLogFile(LPCTSTR LogMessage);

//------------------------------------------------------------------------------------
// Module specific
//
void  _stdcall LoadConfiguration(LPSTR CfgFileName);

void  _stdcall FixMgeFOV(void);
void  _stdcall HookEndDrawGeometry(void);
void  _stdcall HookBeginDrawGeometry(void);
DWORD _stdcall ProcHookBinkOpen(LPSTR FilePath, DWORD Unknown);
DWORD _stdcall WriteExceptionLog(LPSTR FilePath, PEXCEPTION_RECORD ExcepRec, PCONTEXT ContextRec);
void  _stdcall SetProcPriority(DWORD prior);
BOOL  _stdcall ForceProcessSingleCore(HANDLE hProcess);
BOOL  _cdecl   __PickMenuMode(DWORD Reg_EIP,DWORD Reg_EFL,MORROWIND::PCMenu Menu,MORROWIND::PCCursor Cursor,DWORD Reg_EBP,DWORD Reg_ESP,MORROWIND::PCGameScene GameSceneIn,DWORD Reg_EDX,MORROWIND::PCGameScene GameSceneOut,DWORD Reg_EAX,...);
BOOL  _cdecl   __AMMO_dmg_STUB(HVL DWORD Reg_EIP,HVL DWORD Reg_EFL,HVL DWORD Reg_EDI,HVL DWORD Reg_ESI,HVL DWORD Reg_EBP,HVL DWORD Reg_ESP,HVL DWORD Reg_EBX,HVL DWORD Reg_EDX,HVL DWORD Reg_ECX,HVL DWORD FormID,...);
BOOL  _cdecl   __EnterMenu_STUB(HVL DWORD Reg_EIP,HVL DWORD Reg_EFL,HVL DWORD Reg_EDI,HVL DWORD Reg_ESI,HVL DWORD Reg_EBP,HVL DWORD Reg_ESP,HVL DWORD Reg_EBX,HVL DWORD Reg_EDX,HVL DWORD Reg_ECX,HVL DWORD Reg_EAX,...);
BOOL  _cdecl   __LeaveMenu_STUB(HVL DWORD Reg_EIP,HVL DWORD Reg_EFL,HVL DWORD Reg_EDI,HVL DWORD Reg_ESI,HVL DWORD Reg_EBP,HVL DWORD Reg_ESP,HVL DWORD Reg_EBX,HVL DWORD Reg_EDX,HVL DWORD Reg_ECX,HVL DWORD Reg_EAX,...);

void  _cdecl   ProcHookRenderWorld(void);
LONG  WINAPI   VecCrashHandler(PEXCEPTION_POINTERS ExceptionInfo);

//====================================================================================
struct SModData  
{
 MORROWIND::GLOBALS MWGlobals;

 // Ini file Options
 DWORD        LangIndex;
 DWORD        CrashIgnoreMax;
 DWORD        OptProcPrior;
 DWORD        FixUnarmoredBug;  // From TextDLL	 
 BOOL         FixRequestCD;
 //BOOL         FixRusTexDll;
 BOOL         FixLevUpText;
 BOOL         FixBikSearch;
 BOOL         FixMGraphExt;
 BOOL         FixMulticore;
 BOOL         FixMouseCapt;
 BOOL         FixLostFocus;
 BOOL		  FixItemInfo;	    // From TextDLL	 
 BOOL         FixCrashes;
 BOOL         ExWrCrashLog;
 BOOL         ExPassKern32;
 BOOL         ExTryIgnore;
 BOOL         ExStackDump;
 BOOL         ExBeepExcept;

 HMODULE      ExeModuleBase;
 HMODULE      ThisLibBase;
 HMODULE      Kernel32Base;
 HMODULE      KernelBaseBase;
 HANDLE       DefHeapHandle;
 BOOL         InMenu;           // For TextDll
 BOOL         ModuleLoader;
 PVOID        ProcBinkOpen;
 PVOID        pVecExHandler;
 DWORD        LastExCounter;
 DWORD        LastExAddress;
 DWORD        LastExTime;

 CODEHOOK32   MgeFixHook;
 CODEHOOK32   ItemInfoHook;
 CODEHOOK32   EnterMenuHook;
 CODEHOOK32   LeaveMenuHook;

 IMPORTHOOK   HookBinkOpen;

 BYTE         AngelIniPath[MAX_PATH];
 BYTE         GameCurDir[MAX_PATH];
 BYTE         CurrentDir[MAX_PATH];
 BYTE         CfgFilePath[MAX_PATH];
 BYTE         ExcFilePath[MAX_PATH];
 BYTE         ConfigAppName[64];
//---------------------------------

 void* operator new(size_t size){return MEMWORK::GetMemoryBlock(sizeof(SModData));}   // Sets memory to ZERO
 void  operator delete(void* mem){MEMWORK::FreeMemoryBlock(mem);}    
//---------------------------------

 SModData(void)     
  {
   this->MWGlobals.Initialize();
   this->LangIndex        = 1;
   this->CrashIgnoreMax   = 20;
   this->OptProcPrior     = NORMAL_PRIORITY_CLASS;
   this->FixUnarmoredBug  = 1;
   this->FixItemInfo      = TRUE;
   this->FixRequestCD     = TRUE;
   //this->FixRusTexDll     = TRUE;
   this->FixLevUpText     = TRUE;
   this->FixBikSearch     = TRUE;
   this->FixMGraphExt     = TRUE;
   this->FixMulticore     = TRUE;
   this->FixMouseCapt     = FALSE;
   this->FixLostFocus     = FALSE;					 
   this->FixCrashes       = TRUE;
   this->ExWrCrashLog     = TRUE;
   this->ExPassKern32     = TRUE;
   this->ExTryIgnore      = TRUE; 
   this->ExStackDump      = FALSE;
   this->ExBeepExcept     = FALSE;
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

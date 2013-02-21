
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
#define	MOD_PARAMETERS   0
#define	MOD_NAME         "MorrText"  // Spaces allowed - used in MLDR
//---------------------------------------------------------------------------  

#ifndef MODULENAME
#define DLLMAINTYPE static
#define DEFLOGPROC Logger::LogToFile
#define MODULENAME MorrTextMod   // No spaces allowed
#define LogMsg(msg,...) _LogLineM(MODULENAME::THISMODNAME,msg,__VA_ARGS__) 
#else
#define DLLMAINTYPE
#define DEFLOGPROC Logger::LogToProc
#define LogMsg(msg,...) _LogLineM(NULL,msg,__VA_ARGS__) 
#endif

//--------------------------------------------------------------------------- 
// From additional files
//
void  _stdcall UnloadDialogEngine(void);
int   _stdcall InitDialogEngine(void);
int   _stdcall InitMWKeyboard(void);
int   _stdcall InitConsole(void);
//------------------------------------------------------------------------------------
 
namespace MODULENAME
{
const LPSTR THISMODNAME    = _STRNG(MODULENAME);
const LPSTR	INISECCONFIG   = "BaseConfig";
const LPSTR MLDRLIBNAME    = "MLDRLib.dll";
const LPSTR MLDRLOGPROC    = "WriteLogFile";
const LPSTR MLDRPROCNAME   = "IsModuleLoader";
const LPSTR CONFIGINIFILE  = "MorrTextCfg.ini";
const LPSTR SHELLCFGNAME   = "MorrTextCfg.exe";		// For call from Moduleloader
const LPSTR SINGERRORMSGRU = "Îäíà èç ñèãíàòóð íå ñîâïàäàåò.\r\nÍå òà âåðñèÿ 'Morrowind.exe'? (Íóæíà: 1.6.1820).\r\nÏÐÎÄÎËÆÅÍÈÅ ÐÀÁÎÒÛ ÍÅÂÎÇÌÎÆÍÎ!";
const LPSTR SINGERRORMSGEN = "One of a signatures is differs.\r\nWrong version of 'Morrowind.exe'? (Need: 1.6.1820).\r\nIMPOSSIBLE TO CONTINUE!";

#define GameConOut(msg,...) ModInfo->MWGlobals.OutConsoleMessage(*ModInfo->MWGlobals.ppGameScene,msg,__VA_ARGS__)

//------------------------------------------------------------------------------------
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
//------------------------------------------------------------------------------------
// Module specific
//
void  _stdcall LoadConfiguration(LPSTR CfgFileName);
int   _stdcall FixLoadBsa(void);
//====================================================================================

class CConsoleInput : public MORROWIND::CConsoleInput
{
public:
  bool MWTHISCALL __ParseInputString(void);
};

//====================================================================================
struct SModData  
{
 MORROWIND::GLOBALS MWGlobals;

 CStringMap cells;
 CStringMap top_data;
 CStringMap marktopic;

 // Config
 bool  LoadMarkTopicForPlugins;
 bool  LoadTopicsForPlugins;
 bool  LoadCellsForPlugins;
 bool  EnableConsole;
 bool  EnglishInput;
 bool  LogTxtSubst; 
 SLOpt StrLstCase;     // 1 = case insensitive

 HMODULE      ExeModuleBase;
 HMODULE      ThisLibBase;
 HANDLE       DefHeapHandle;
 UINT         HookIndex;
 PBYTE        PatchBase;
 BOOL         ModuleLoader;
 CODEHOOK32   CodeHooks[32];

 BYTE         GameCurDir[MAX_PATH];
 BYTE         CurrentDir[MAX_PATH];
 BYTE         CfgFilePath[MAX_PATH];
 BYTE         ConfigAppName[64];
//---------------------------------

 void* operator new(size_t size){return MEMWORK::GetMemoryBlock(sizeof(SModData));}  // Sets memory to ZERO
 void  operator delete(void* mem){MEMWORK::FreeMemoryBlock(mem);}    
//---------------------------------

 SModData(void)     
  {
   this->MWGlobals.Initialize();

   this->StrLstCase              = soNone;
   this->LogTxtSubst             = false;
   this->EnglishInput            = true;
   this->EnableConsole           = false;
   this->LoadCellsForPlugins     = false;
   this->LoadTopicsForPlugins    = false;
   this->LoadMarkTopicForPlugins = false;
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
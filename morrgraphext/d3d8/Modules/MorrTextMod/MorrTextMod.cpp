
#include "ExtensionMod.h"


using namespace MODULENAME;


MODULENAME::PSModData  ModInfo;
//====================================================================================
//
//====================================================================================
DLLMAINTYPE BOOL APIENTRY DLLMain(HMODULE hModule, DWORD ReasonCall, LPVOID lpReserved)
{				           //  Only one thread will execute within DLLMAIN
 switch(ReasonCall)	    
  {
   case DLL_PROCESS_ATTACH:
     CreateModule(hModule);
     break;									

   case DLL_THREAD_ATTACH:
     break;

   case DLL_THREAD_DETACH:
     break;

   case DLL_PROCESS_DETACH: 
	 DestroyModule();
	 break;
   default : return false;  // WRONG REASON CODE !!!!!!
  }
 return true;
}
//====================================================================================

namespace MODULENAME
{
//====================================================================================
//                                   MODULE
//====================================================================================
PSModData _stdcall DestroyModule(void)
{
 if(!ModInfo)return NULL;
 LogMsg("Executing...");
 UnloadDialogEngine();
 delete(ModInfo);
 ModInfo = NULL;
 return ModInfo;
}
//------------------------------------------------------------------------------------
PSModData _stdcall CreateModule(HMODULE hDllMod)
{
 if(ModInfo)return ModInfo;
 Logger::LogProc = DEFLOGPROC;
 Logger::MsgProc = ModWriteLogFile;

 ModInfo = new SModData();  
 ModInfo->ThisLibBase      = hDllMod;
 ModInfo->DefHeapHandle    = GetProcessHeap();
 ModInfo->ExeModuleBase    = GetModuleHandle(NULL);
 GetModuleFileName(hDllMod,(LPSTR)&ModInfo->CurrentDir,sizeof(ModInfo->CurrentDir));                // DLL path
 GetModuleFileName(ModInfo->ExeModuleBase,(LPSTR)&ModInfo->GameCurDir,sizeof(ModInfo->GameCurDir));	// EXE path    

 TrimFilePath((LPSTR)&ModInfo->CurrentDir);
 TrimFilePath((LPSTR)&ModInfo->GameCurDir);
 if(lstrcmpi((LPSTR)&ModInfo->CurrentDir,(LPSTR)&ModInfo->GameCurDir)==0)lstrcat((LPSTR)&ModInfo->CurrentDir,"mge3\\");   // Work with MGE

 lstrcpy((LPSTR)&ModInfo->CfgFilePath, (LPSTR)&ModInfo->CurrentDir);
 lstrcat((LPSTR)&ModInfo->CfgFilePath, CONFIGINIFILE);

 LoadConfiguration((LPSTR)&ModInfo->CfgFilePath);

 LogMsg("Main EXE module loaded at %08X.", (DWORD)ModInfo->ExeModuleBase); 
 if(GetProcAddress(ModInfo->ExeModuleBase,MLDRPROCNAME)){ModInfo->ModuleLoader=true;return NULL;}   // Just if the function present     

 LogMsg("Starting %s...",MOD_NAME); 
 ModInfo->PatchBase = (PBYTE)ModInfo->ExeModuleBase;
   
 UINT result = 0;
 result += InitDialogEngine();
 result += FixLoadBsa();   
 result += InitMWKeyboard();
 if(ModInfo->EnableConsole)result += InitConsole();
 if(result)LogMsg("Errors while initializing: %u", result);
 return ModInfo;
}
//====================================================================================
DWORD _stdcall ModEventOnHostExit(void)
{
 LogMsg("Unloading...");
 DestroyModule();
 return 0;
}
//------------------------------------------------------------------------------------
DWORD _stdcall ModEventOnHostStart(void)
{
 LogMsg("Initialized.");
 return 0;
}
//------------------------------------------------------------------------------------
DWORD _stdcall ModGetVersion(void)
{
 return MOD_VERSION;
}
//------------------------------------------------------------------------------------
DWORD _stdcall ModGetParameters(void)
{
 return MOD_PARAMETERS;
}
//------------------------------------------------------------------------------------
LPSTR _stdcall ModGetModuleName(void)
{
 return MOD_NAME;
}
//------------------------------------------------------------------------------------
LPSTR _stdcall ModGetConfigShellName(void)
{
 return SHELLCFGNAME;
}
//------------------------------------------------------------------------------------
DWORD _stdcall ModRunConfigShell(DWORD Reserved)
{
 BYTE ShellPath[MAX_PATH];

 lstrcpy((LPSTR)&ShellPath, (LPSTR)&ModInfo->CurrentDir);
 lstrcat((LPSTR)&ShellPath, SHELLCFGNAME);
 if(WinExec((LPSTR)&ShellPath,SW_SHOWNORMAL) > 31)return 0;
 return 1;
}
//------------------------------------------------------------------------------------
void  _stdcall ModWriteLogFile(LPCTSTR LogMessage)
{
 static bool  NoMLDR      = false;
 static LPSTR LogFilePath = NULL;
 
 if(!LogFilePath)
  {
   if(NoMLDR)return;
   PVOID MLDRLogProc = GetProcAddress(GetModuleHandle(MLDRLIBNAME),MLDRLOGPROC);
   if(!MLDRLogProc){NoMLDR = true;return;}
   LogFilePath = ((LPSTR (_stdcall*)(LPSTR,LPSTR))MLDRLogProc)(NULL,NULL);
   if(!LogFilePath)return;
  }
 BYTE DateTimeBuf[64]; 
 FormatCurDateTime((LPSTR)&DateTimeBuf);
 for(int ctr=0;LogMessage[ctr];ctr++){if((LogMessage[ctr] == 0x0D)&&(LogMessage[ctr+1] == 0x0A)){*((PBYTE)&LogMessage[ctr]) = 0;break;}}   // Remove excess line endings (TODO: Use own INI files support)
 WritePrivateProfileString(THISMODNAME,(LPSTR)&DateTimeBuf,LogMessage,LogFilePath);  
}
//====================================================================================
//					                  OTHER FUNCTIONS
//====================================================================================
void _stdcall LoadConfiguration(LPSTR CfgFileName)
{
 ModInfo->StrLstCase              = (SLOpt)RefreshINIValueInt(INISECCONFIG,"StrLstCase",       ModInfo->StrLstCase,              CfgFileName);
 ModInfo->LogTxtSubst             = RefreshINIValueInt(INISECCONFIG,"LogTxtSubst",             ModInfo->LogTxtSubst,             CfgFileName);
 ModInfo->EnglishInput            = RefreshINIValueInt(INISECCONFIG,"EnglishInput",            ModInfo->EnglishInput,            CfgFileName);
 ModInfo->EnableConsole           = RefreshINIValueInt(INISECCONFIG,"EnableConsole",           ModInfo->EnableConsole,           CfgFileName);
 ModInfo->LoadCellsForPlugins     = RefreshINIValueInt(INISECCONFIG,"LoadCellsForPlugins",     ModInfo->LoadCellsForPlugins,     CfgFileName);
 ModInfo->LoadTopicsForPlugins    = RefreshINIValueInt(INISECCONFIG,"LoadTopicsForPlugins",    ModInfo->LoadTopicsForPlugins,    CfgFileName);
 ModInfo->LoadMarkTopicForPlugins = RefreshINIValueInt(INISECCONFIG,"LoadMarkTopicForPlugins", ModInfo->LoadMarkTopicForPlugins, CfgFileName);
}
//====================================================================================
//
//====================================================================================
// картинки в книгах сначала пытаются загрузится из "Data Files\BookArt" (решает проблему с английскими картинками в книгах, появившуюся в Bloodmoon)<br>
int _stdcall FixLoadBsa(void)  // Bloodmoon
{
 int Result = 0;
 Result += MemoryPatchMaskLocal(&ModInfo->PatchBase[0x0007AA36], "??38", "741A");
 Result += MemoryPatchMaskLocal(&ModInfo->PatchBase[0x0007AA5A], "??DC909090906A00????????????E980000000", "74146A008D44241C50"); 
 Result += MemoryPatchMaskLocal(&ModInfo->PatchBase[0x0007AAE4], "??44????E965FF", "8D4C24186A0051");
 Result += MemoryPatchMaskLocal(&ModInfo->PatchBase[0x0007AAEB], "??FF84C0752CE942FFFFFF909090", "FFD383C40883F8FF");
 if(Result)LogMsg("Errors while initializing: %u", Result);
 return Result;
}
//====================================================================================
}

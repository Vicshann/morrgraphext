
#include <windows.h>
#include "ExtensionMod.h"

using namespace MODULENAME;

MODULENAME::PSModData  ModInfo;

PBYTE  MenuFlagMGE = NULL; // MGE hack :)
BYTE   LastMFlag   = 0;
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
 if(ModInfo->FixCrashes && ModInfo->pVecExHandler)RemoveVectoredExceptionHandler(ModInfo->pVecExHandler);
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
 ModInfo->Kernel32Base     = GetModuleHandle("Kernel32.dll");
 ModInfo->KernelBaseBase   = GetModuleHandle("KERNELBASE.dll");
 GetModuleFileName(hDllMod,(LPSTR)&ModInfo->CurrentDir,sizeof(ModInfo->CurrentDir));
 GetModuleFileName(ModInfo->ExeModuleBase,(LPSTR)&ModInfo->GameCurDir,sizeof(ModInfo->GameCurDir));	     

 TrimFilePath((LPSTR)&ModInfo->CurrentDir);
 TrimFilePath((LPSTR)&ModInfo->GameCurDir);
 if(lstrcmpi((LPSTR)&ModInfo->CurrentDir,(LPSTR)&ModInfo->GameCurDir)==0)lstrcat((LPSTR)&ModInfo->CurrentDir,"mge3\\");   // Work with MGE

 lstrcpy((LPSTR)&ModInfo->ExcFilePath, (LPSTR)&ModInfo->GameCurDir);
 lstrcpy((LPSTR)&ModInfo->CfgFilePath, (LPSTR)&ModInfo->CurrentDir);
 lstrcat((LPSTR)&ModInfo->CfgFilePath, CONFIGINIFILE);
 lstrcat((LPSTR)&ModInfo->ExcFilePath, CRASHLOGFILE);

 LoadConfiguration((LPSTR)&ModInfo->CfgFilePath);
 SetProcPriority(ModInfo->OptProcPrior);

 LogMsg("Main EXE module loaded at %08X.", (DWORD)ModInfo->ExeModuleBase);     
 if(GetProcAddress(ModInfo->ExeModuleBase,MLDRPROCNAME)){ModInfo->ModuleLoader=true;return NULL;}   // Just if the function present     
   
 LogMsg("Starting %s...",MOD_NAME);     
 //================================================================================ 
 if(ModInfo->FixMulticore){LogMsg("Setting the process` affinity mask to single core.");
    if(!ForceProcessSingleCore(GetCurrentProcess()))LogMsg("Setting affinity mask failed!");}
 //================================================================================ 
 if(ModInfo->FixRequestCD){LogMsg("Disabling a game CD request.");     
    if(SetFunctionPatch(ModInfo->ExeModuleBase,0x00016E6B, &((PBYTE)ModInfo->ExeModuleBase)[0x00017041],0xE9,0,"745DA0????????84C075"))LogMsg("Patch failed!");}   
 //================================================================================ 
 if(ModInfo->FixLevUpText){LogMsg("Restoring Level Up commentaries in russian version.");   
    if(SetFunctionPatch(ModInfo->ExeModuleBase,0x001D9281,&((PBYTE)ModInfo->ExeModuleBase)[0x003A0E38],0xB8,5,"68AB020000E8A566E3FF")||
    SetFunctionPatch(ModInfo->ExeModuleBase,0x001D929D,&((PBYTE)ModInfo->ExeModuleBase)[0x0039357C],0xB8,5,"689D030000E88966E3FF"))LogMsg("Patch failed!");}
 //================================================================================   
 if(ModInfo->FixBikSearch){LogMsg("Setting hook on opening a BIK file to search it properly.");        
    if(ModInfo->HookBinkOpen.SetHook(&ProcHookBinkOpen,"binkw32.dll","_BinkOpen@8",ModInfo->ExeModuleBase,TRUE))LogMsg("Setting a hook FAILED!");} 	
 //================================================================================ 
 if(ModInfo->FixMGraphExt){LogMsg("Setting hook on state of menu to fix MGE."); 
    DWORD ProcA = (DWORD)&HookBeginDrawGeometry;  // Original is not saved!
	DWORD ProcB = (DWORD)&HookEndDrawGeometry;	  // Original is not saved!
    if(ModInfo->MgeFixHook.SetCodeHook(&((PBYTE)ModInfo->ExeModuleBase)[0x0018E1FB],GetAddress<CODEHOOK32::CODEHOOKPROC32>(__PickMenuMode),"C6475A018B")||
	   SetFunctionPatch(ModInfo->ExeModuleBase,0x0001C08E,&ProcHookRenderWorld,0xE8,0,"E86D0300008B8F080100")
	   /*MemoryPatchLocal(&((PBYTE)ExeModuleBase)[0x0034F57C], &ProcA, sizeof(DWORD), NULL)||
	   MemoryPatchLocal(&((PBYTE)ExeModuleBase)[0x0034F580], &ProcB, sizeof(DWORD), NULL)*/)LogMsg("Setting a hook FAILED!");
    LogMsg("Fixing FOV bug in MGE.");
    FixMgeFOV();} 	
 //================================================================================
 if(ModInfo->FixMouseCapt){LogMsg("Fixing DirectInput cooperative mode.");  // Fix cursor jitter   
    if(SetFunctionPatch(ModInfo->ExeModuleBase,0x0000E4FA,(PVOID)0x0F,0x6A,0,"6A07518BC8E8CC7CFFFF"))LogMsg("Patch failed!");}                                                                         	
 //================================================================================																		 
 if(ModInfo->FixLostFocus){LogMsg("Disabling game freezing, when window is lose focus.");  // TODO: Unfreeze also static game menu // DO NOT WORKS!
    if(SetFunctionPatch(ModInfo->ExeModuleBase,0x00016C74,NULL,0xEB,0,"75148B15DC6C7C008B82")||   // Allow Bink playing without focus
       SetFunctionPatch(ModInfo->ExeModuleBase,0x0001AB7D,NULL,0x90,5,"FF15746374008B0DDC67")||   // Allow rendering without focus
       SetFunctionPatch(ModInfo->ExeModuleBase,0x0001AB89,NULL,0x90,27,"3B81C800000074148B15"))LogMsg("Patch failed!");}                                                                         																	  		 		
 //================================================================================
 if(ModInfo->FixItemInfo){LogMsg("Applying Item Info fix (from TexDll)."); 
    if(ModInfo->ItemInfoHook.SetCodeHook(&((PBYTE)ModInfo->ExeModuleBase)[0x00191219],GetAddress<CODEHOOK32::CODEHOOKPROC32>(__AMMO_dmg_STUB),"3D41524D4F"))LogMsg("Setting a hook FAILED!");}
 //================================================================================
 if(ModInfo->FixUnarmoredBug){LogMsg("Applying Unarmored Bug fix (from TexDll)."); 
    if(ModInfo->EnterMenuHook.SetCodeHook(&((PBYTE)ModInfo->ExeModuleBase)[0x00195020],GetAddress<CODEHOOK32::CODEHOOKPROC32>(__EnterMenu_STUB),"8B15????????C705")||  // A hook for same purpose already exist in TalkyMorrowind module
        ModInfo->LeaveMenuHook.SetCodeHook(&((PBYTE)ModInfo->ExeModuleBase)[0x001951B0],GetAddress<CODEHOOK32::CODEHOOKPROC32>(__LeaveMenu_STUB),"8B0D????????C681"))LogMsg("Setting a hook FAILED!");  // A hook for same purpose already exist in TalkyMorrowind module
     if((ModInfo->FixUnarmoredBug > 1) && SetFunctionPatch(ModInfo->ExeModuleBase,0x0014D87D,NULL,0x90,5,"0F846B020000D8442420"))LogMsg("Patch failed!");} // Set to >1 to skip this patch                                                                                               
 //================================================================================

 // Add a new fixes here
             
 //================================================================================
 //if(ModInfo->FixRusTexDll){LogMsg("Setting hook to catch loading of a 'text.dll'");  
 //   ModInfo->HookGetVersionExA.SetHook(&ProcHookGetVersionExA, "Kernel32.dll", "GetVersionExA", TRUE);}

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
 if(ModInfo->FixCrashes){DeleteFile((LPSTR)&ModInfo->ExcFilePath);ModInfo->pVecExHandler=AddVectoredExceptionHandler(true,VecCrashHandler);if(!ModInfo->pVecExHandler)LogMsg("Failed to set an exception handler.");}
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
//
//					                  OTHER FUNCTIONS
//
//====================================================================================
void _stdcall LoadConfiguration(LPSTR CfgFileName)
{
 ModInfo->LangIndex       = RefreshINIValueInt(INISECCONFIG,"LangIndex",ModInfo->LangIndex,CfgFileName);
 ModInfo->OptProcPrior    = RefreshINIValueInt(INISECCONFIG,"ProcessPriority",ModInfo->OptProcPrior,CfgFileName);
 ModInfo->FixRequestCD    = RefreshINIValueInt(INISECFIXES,"FixRequestCD",ModInfo->FixRequestCD,CfgFileName);
 ModInfo->FixUnarmoredBug = RefreshINIValueInt(INISECFIXES,"FixUnarmoredBug",ModInfo->FixUnarmoredBug,CfgFileName);
 ModInfo->FixItemInfo     = RefreshINIValueInt(INISECFIXES,"FixItemInfo",ModInfo->FixItemInfo,CfgFileName);
 //ModInfo->FixRusTexDll    = RefreshINIValueInt(INISECFIXES,"FixRusTexDll",ModInfo->FixRusTexDll,CfgFileName);
 ModInfo->FixLevUpText    = RefreshINIValueInt(INISECFIXES,"FixLevUpText",ModInfo->FixLevUpText,CfgFileName); 
 ModInfo->FixBikSearch    = RefreshINIValueInt(INISECFIXES,"FixBikSearch",ModInfo->FixBikSearch,CfgFileName);
 ModInfo->FixMGraphExt    = RefreshINIValueInt(INISECFIXES,"FixMGraphExt",ModInfo->FixMGraphExt,CfgFileName);  
 ModInfo->FixMulticore    = RefreshINIValueInt(INISECFIXES,"FixMulticore",ModInfo->FixMulticore,CfgFileName);  

 ModInfo->FixMouseCapt    = RefreshINIValueInt(INISECFIXES,"FixMouseCapt",ModInfo->FixMouseCapt,CfgFileName);  
 ModInfo->FixLostFocus    = RefreshINIValueInt(INISECFIXES,"FixLostFocus",ModInfo->FixLostFocus,CfgFileName);  

 ModInfo->FixCrashes      = RefreshINIValueInt(INISECFIXES,"FixCrashes",  ModInfo->FixCrashes,CfgFileName);
 ModInfo->ExPassKern32    = RefreshINIValueInt(INISECFIXES,"ExPassKern32",ModInfo->ExPassKern32,CfgFileName);
 ModInfo->ExWrCrashLog    = RefreshINIValueInt(INISECFIXES,"ExWrCrashLog",ModInfo->ExWrCrashLog,CfgFileName);
 ModInfo->ExTryIgnore     = RefreshINIValueInt(INISECFIXES,"ExTryIgnore", ModInfo->ExTryIgnore,CfgFileName);
 ModInfo->ExStackDump     = RefreshINIValueInt(INISECFIXES,"ExStackDump", ModInfo->ExStackDump,CfgFileName);
 ModInfo->ExBeepExcept    = RefreshINIValueInt(INISECFIXES,"ExBeepExcept",ModInfo->ExBeepExcept,CfgFileName);  
}
//====================================================================================
//
//					                   HOOKS
//
//====================================================================================
/*DWORD _stdcall ProcHookGetVersionExA(LPOSVERSIONINFO lpVersionInfo)
{
 HMODULE RusTextDll;
 DWORD   Result = 0;

 if((RusTextDll = GetModuleHandle(RUSTEXTMODN)) && ModInfo->FixRusTexDll)
  {
   ModInfo->FixRusTexDll = FALSE;    // Make this only once
   LogMsg("Russian game version detected...");
   LogMsg("Removing integrity check of 'Morrowind.exe' in 'Text.dll' ...");
   // Trying to patch 'Text.dll 3.000, version 2.050 do not supported anymore. Use a patched file, if needed.'
   Result += (bool)SetFunctionPatch(RusTextDll,0x000071BC,(PVOID)0x00000001,0xB8,0,"E86F9FFFFF84C074F4E8");  // Remove CD check       0x00d071bc B8 01 00 00 00        L5
   Result += (bool)SetFunctionPatch(RusTextDll,0x00007B00,(PVOID)0x385688AF,0xB8,0,"E84BFFFFFF3DAF885638");  // Set 'RIGHT' checksum [1.6.0.1820] and fix problem with MGE-MWSE      0x00d07b00 B8 AF 88 56 38        L5
   Result += (bool)SetFunctionPatch(RusTextDll,0x00007B05,&((PBYTE)RusTextDll)[0x0001DAA0],0xA3,2,"3DAF8856387515E80FFE");  // Set 'RIGHT' checksum [1.6.0.1820] and fix problem with MGE-MWSE       0x00d07b05 A3 A0 DA D1 00 90 90  L7
   if(Result)LogMsg("Failed to patch 'Text.dll' - a signature not found!");  
   	 else ((PBYTE)RusTextDll)[0x0001D600] = ModInfo->CurrentDir[0]; // Set valid CD drive letter	// Check1:+1982,Check2:+5B10,CrcChek:+590E
   LogMsg("Setting hook to work with INI file of a 'text.dll' on full path.");  
   if(UpdateModuleImports(GetProcAddress(GetModuleHandle("Kernel32.dll"),"GetPrivateProfileIntA"), &ProcHookGetPrivateProfileIntA, RusTextDll))LogMsg("Setting a hook FAILED!"); 	
   
   GetModuleFileName(RusTextDll,(LPSTR)&ModInfo->AngelIniPath,sizeof(ModInfo->AngelIniPath));
   ModInfo->AngelIniPath[CharPosFromRight((LPSTR)&ModInfo->AngelIniPath, '\\', 0, 0)+1] = 0;
   lstrcat((LPSTR)&ModInfo->AngelIniPath,"Angel.ini");
  }
 return GetVersionEx(lpVersionInfo);
}
//====================================================================================
DWORD _stdcall ProcHookGetPrivateProfileIntA(LPSTR lpAppName, LPSTR lpKeyName, DWORD nDefault, LPSTR lpFileName)
{
 return GetPrivateProfileIntA(lpAppName, lpKeyName, nDefault, (LPSTR)&ModInfo->AngelIniPath);
}*/
//====================================================================================
DWORD _stdcall ProcHookBinkOpen(LPSTR FilePath, DWORD Unknown)
{
 int   Offset;
 LPSTR FileName = FilePath;
 BYTE  BikFilePath[MAX_PATH];
  
 LogMsg("Trying to open a BIK file: %s", FilePath);  
 if((Offset = CharPosFromRight(FilePath, '\\', 0, 0)) >=0 )FileName += (Offset+1);
 lstrcpy((LPSTR)&BikFilePath,(LPSTR)&ModInfo->GameCurDir);
 lstrcat((LPSTR)&BikFilePath,"Data Files\\Video\\");
 lstrcat((LPSTR)&BikFilePath, FileName);
 if(GetFileAttributes((LPSTR)&BikFilePath)==INVALID_FILE_ATTRIBUTES) // If file on hard drive
  {
   lstrcpy((LPSTR)&BikFilePath,"Z:\\Video\\"); 
   lstrcat((LPSTR)&BikFilePath, FileName);
   for(int ctr=0;ctr < 26;ctr++)
    {
     if(GetFileAttributes((LPSTR)&BikFilePath)!=INVALID_FILE_ATTRIBUTES){FileName = (LPSTR)&BikFilePath;break;} // File on CD
     BikFilePath[0]--;
    }            
  }
   else FileName = (LPSTR)&BikFilePath;   
 return ((DWORD (_stdcall*)(LPSTR,DWORD))ModInfo->HookBinkOpen.OriginalAddr)(FileName, Unknown);
}
//====================================================================================
void _stdcall FixMgeFOV(void)
{
 HMODULE hDllMGE = GetModuleHandle("d3d8.dll");
 if(!hDllMGE)return;

 DOS_HEADER *DosHdr = (DOS_HEADER*)hDllMGE;
 WIN_HEADER *WinHdr = (WIN_HEADER*)&((PBYTE)hDllMGE)[DosHdr->OffsetHeaderPE];
 PDWORD DataSec      = (PDWORD)&((PBYTE)hDllMGE)[WinHdr->OptionalHeader.BaseOfData];
 DWORD  DataSize     = WinHdr->OptionalHeader.InitDataSize;

 LogMsg("Searching for 'ScreenFOV' address");
 for(UINT ctr=0;ctr<(DataSize/sizeof(DWORD));ctr++) 
  {
   if((DataSec[ctr+1]==8)&&(DataSec[ctr+2]==1)&&(DataSec[ctr+3]==0)&&!IsBadReadPtr((PVOID)(DataSec[ctr+4]),20)&&(lstrcmpi((LPSTR)(DataSec[ctr+4]),"Horizontal Screen FOV")==0))
	 {
	  LogMsg("Address of 'ScreenFOV' is %08X", DataSec[ctr]);
	  // Set to Zero Min and Max values [double]
	  DataSec[ctr+8] = DataSec[ctr+9] = DataSec[ctr+10] = DataSec[ctr+11] = 0;
      return;
	 }
  }
 LogMsg("Address of 'ScreenFOV' not found!");
}
//====================================================================================
BOOL _stdcall IsInterior(void)
{
#define eMaster 0x007C67DC
#define eEnviro (eMaster + 0x04)
 register DWORD addr = *reinterpret_cast<DWORD*>(eEnviro);
 return (addr && *reinterpret_cast<DWORD*>(addr+0xAC)); // IsInterior
}
//====================================================================================
__declspec(naked) void _stdcall HookBeginDrawGeometry(void)
{
 __asm
  {
   call IsInterior
   test EAX, EAX
   jz   OCALL
   mov EAX, [MenuFlagMGE] 
   test EAX, EAX
   jz   OCALL
   mov DL, [EAX]
  // mov byte ptr [EAX], 1
   mov [LastMFlag], DL
OCALL:
   mov EAX, 0x006AC520
   jmp EAX
  }
}
//====================================================================================
__declspec(naked) void _stdcall HookEndDrawGeometry(void)
{
 __asm	// *MenuFlagMGE = LastMFlag;
 {
  mov  EAX, [MenuFlagMGE] 
  test EAX,EAX
  jz   OCALL
  mov  DL, [LastMFlag]
  //mov  [EAX], DL
  mov  [MenuFlagMGE], 0
OCALL:
  mov EAX, 0x006AC590
  jmp EAX
 }
}
//====================================================================================
// Make full initialization of MORROWIND:GLOBALS in future and use it here
void _cdecl ProcHookRenderWorld(void)
{
#define eMaster 0x007C67DC
#define eEnviro (eMaster + 0x04)
register DWORD addr = *reinterpret_cast<DWORD*>(eEnviro);
if(addr && *reinterpret_cast<DWORD*>(addr+0xAC)) // IsInterior
 {
  BYTE LastMFlag = *MenuFlagMGE;
  *MenuFlagMGE = true; 
  ((void  (__cdecl *)(void))&((PBYTE)ModInfo->ExeModuleBase)[0x0001C400])();
  *MenuFlagMGE = LastMFlag;
 }
  else ((void  (__cdecl *)(void))&((PBYTE)ModInfo->ExeModuleBase)[0x0001C400])();   
}
//====================================================================================
BOOL _cdecl __PickMenuMode(DWORD Reg_EIP,DWORD Reg_EFL,MORROWIND::PCMenu Menu,MORROWIND::PCCursor Cursor,DWORD Reg_EBP,DWORD Reg_ESP,MORROWIND::PCGameScene GameSceneIn,DWORD Reg_EDX,MORROWIND::PCGameScene GameSceneOut,DWORD Reg_EAX,...)
{
 Menu->Activated = false;        // Always reset - MGE have a water problems, when paused 
 MenuFlagMGE     = &Menu->Activated;
 GameSceneOut    = GameSceneIn;  // Assigning, removed by this hook
 return false;
}
//====================================================================================
//                            Item Info fix from TextDll
//------------------------------------------------------------------------------------
BOOL _cdecl __AMMO_dmg_STUB(HVL DWORD Reg_EIP,HVL DWORD Reg_EFL,HVL DWORD Reg_EDI,HVL DWORD Reg_ESI,HVL DWORD Reg_EBP,HVL DWORD Reg_ESP,HVL DWORD Reg_EBX,HVL DWORD Reg_EDX,HVL DWORD Reg_ECX,HVL DWORD FormID,...)
{
 if(FormID == _REV_DW('AMMO')){Reg_EIP = 0x59146B;return FALSE;}
 return TRUE;
}
//====================================================================================
//                            Unarmored fix from TextDll
//------------------------------------------------------------------------------------
BOOL _cdecl __EnterMenu_STUB(HVL DWORD Reg_EIP,HVL DWORD Reg_EFL,HVL DWORD Reg_EDI,HVL DWORD Reg_ESI,HVL DWORD Reg_EBP,HVL DWORD Reg_ESP,HVL DWORD Reg_EBX,HVL DWORD Reg_EDX,HVL DWORD Reg_ECX,HVL DWORD Reg_EAX,...)
{
 ModInfo->InMenu = true;
 ModInfo->MWGlobals.UpdateAC();
 return TRUE;
}
//------------------------------------------------------------------------------------
BOOL _cdecl __LeaveMenu_STUB(HVL DWORD Reg_EIP,HVL DWORD Reg_EFL,HVL DWORD Reg_EDI,HVL DWORD Reg_ESI,HVL DWORD Reg_EBP,HVL DWORD Reg_ESP,HVL DWORD Reg_EBX,HVL DWORD Reg_EDX,HVL DWORD Reg_ECX,HVL DWORD Reg_EAX,...)
{
 ModInfo->InMenu = false;
 return TRUE;
}
//====================================================================================
//                                 Crash Processing
//====================================================================================
DWORD _stdcall WriteExceptionLog(LPSTR FilePath, PEXCEPTION_RECORD ExcepRec, PCONTEXT ContextRec)
{
 DWORD Result;
 LPSTR ExName;
 BYTE  ExText[1024];
 BYTE  Buffer[512];
 BYTE  DateTimeBuf[64];

 LogMsg("Exception %s", FilePath);  
 HANDLE hFile = CreateFile(FilePath,GENERIC_READ|GENERIC_WRITE,FILE_SHARE_READ|FILE_SHARE_WRITE,NULL,OPEN_ALWAYS,FILE_ATTRIBUTE_NORMAL,NULL);
 if(hFile == INVALID_HANDLE_VALUE)return 1;
 SetFilePointer(hFile,0,NULL,FILE_END);
 FormatCurDateTime((LPSTR)&DateTimeBuf);
 ExText[0] = Buffer[0] = 0;
 switch(ExcepRec->ExceptionCode)
  {
   case EXCEPTION_ACCESS_VIOLATION:         ExName = "EXCEPTION_ACCESS_VIOLATION";break;
   case EXCEPTION_ARRAY_BOUNDS_EXCEEDED:    ExName = "EXCEPTION_ARRAY_BOUNDS_EXCEEDED";break;
   case EXCEPTION_BREAKPOINT:               ExName = "EXCEPTION_BREAKPOINT"; break;
   case EXCEPTION_DATATYPE_MISALIGNMENT:    ExName = "EXCEPTION_DATATYPE_MISALIGNMENT"; break;
   case EXCEPTION_FLT_DENORMAL_OPERAND:     ExName = "EXCEPTION_FLT_DENORMAL_OPERAND"; break;
   case EXCEPTION_FLT_DIVIDE_BY_ZERO:       ExName = "EXCEPTION_FLT_DIVIDE_BY_ZERO"; break;
   case EXCEPTION_FLT_INEXACT_RESULT:       ExName = "EXCEPTION_FLT_INEXACT_RESULT"; break;
   case EXCEPTION_FLT_INVALID_OPERATION:    ExName = "EXCEPTION_FLT_INVALID_OPERATION"; break;
   case EXCEPTION_FLT_OVERFLOW:             ExName = "EXCEPTION_FLT_OVERFLOW"; break;
   case EXCEPTION_FLT_STACK_CHECK:          ExName = "EXCEPTION_FLT_STACK_CHECK"; break;
   case EXCEPTION_FLT_UNDERFLOW:            ExName = "EXCEPTION_FLT_UNDERFLOW"; break;
   case EXCEPTION_ILLEGAL_INSTRUCTION:      ExName = "EXCEPTION_ILLEGAL_INSTRUCTION"; break;
   case EXCEPTION_IN_PAGE_ERROR:            ExName = "EXCEPTION_IN_PAGE_ERROR"; break;
   case EXCEPTION_INT_DIVIDE_BY_ZERO:       ExName = "EXCEPTION_INT_DIVIDE_BY_ZERO"; break;
   case EXCEPTION_INT_OVERFLOW:             ExName = "EXCEPTION_INT_OVERFLOW"; break;
   case EXCEPTION_INVALID_DISPOSITION:      ExName = "EXCEPTION_INVALID_DISPOSITION"; break;
   case EXCEPTION_NONCONTINUABLE_EXCEPTION: ExName = "EXCEPTION_NONCONTINUABLE_EXCEPTION"; break;
   case EXCEPTION_PRIV_INSTRUCTION:         ExName = "EXCEPTION_PRIV_INSTRUCTION"; break;
   case EXCEPTION_SINGLE_STEP:              ExName = "EXCEPTION_SINGLE_STEP"; break;
   case EXCEPTION_STACK_OVERFLOW:           ExName = "EXCEPTION_STACK_OVERFLOW"; break;
   default: ExName = "EXCEPTION_UNKNOW_EXCEPTION";
  }
 
 wsprintf((LPSTR)&Buffer,"%s - An exception has occured: %s (0x%08X), Address=%08X%s\r\n\r\n",(LPSTR)&DateTimeBuf,ExName,ExcepRec->ExceptionCode,ExcepRec->ExceptionAddress,(ExcepRec->ExceptionFlags)?(", NONCONTINUABLE"):(", CONTINUABLE"));
 lstrcat((LPSTR)&ExText,(LPSTR)&Buffer);
 if(ExcepRec->ExceptionCode == EXCEPTION_ACCESS_VIOLATION)
  {
   if(ExcepRec->ExceptionInformation[0])wsprintf((LPSTR)&Buffer,"Information: Attempt to write to an inaccessible address [0x%08X]\r\n\r\n",ExcepRec->ExceptionInformation[1]);
     else wsprintf((LPSTR)&Buffer,"Information: Attempt to read from an inaccessible address [0x%08X]\r\n\r\n",ExcepRec->ExceptionInformation[1]);
   lstrcat((LPSTR)&ExText,(LPSTR)&Buffer);
  }
 wsprintf((LPSTR)&Buffer,"Registers: EAX=%08X,ECX=%08X,EDX=%08X,EBX=%08X,ESI=%08X,EDI=%08X,EBP=%08X,ESP=%08X,EIP=%08X,EFL=%08X\r\n\r\n",ContextRec->Eax,ContextRec->Ecx,ContextRec->Edx,ContextRec->Ebx,ContextRec->Esi,ContextRec->Edi,ContextRec->Ebp,ContextRec->Esp,ContextRec->Eip,ContextRec->EFlags);
 lstrcat((LPSTR)&ExText,(LPSTR)&Buffer);
 WriteFile(hFile,&ExText,lstrlen((LPSTR)&ExText),&Result,NULL);
 ExText[0] = Buffer[0] = 0;

 // Can cause exception if ESP is wrong or stack is empty [Stack is in ex handler?]
 if(ModInfo->ExStackDump)
  {
   lstrcat((LPSTR)&ExText,"Stack dump:\r\n");
   for(int ctr=0;ctr<32;ctr++)
    {
     PDWORD Stack = (PDWORD)ContextRec->Esp; ;
     wsprintf((LPSTR)&Buffer,"\t%08X  %08X\r\n",&Stack[ctr],Stack[ctr]);  //   \t\t%08X  %08X\t\t%08X  %08X\t\t\r\n",&Stack[ctr],Stack[ctr], &Stack[ctr+32],Stack[ctr+32], &Stack[ctr+64],Stack[ctr+64]);
     lstrcat((LPSTR)&ExText,(LPSTR)&Buffer);
    }
  }
 lstrcat((LPSTR)&ExText,"\r\nLoaded modules:\r\nName\t\t\tBase\tSize\tPath \r\n");
 WriteFile(hFile,&ExText,lstrlen((LPSTR)&ExText),&Result,NULL);
 ExText[0] = Buffer[0] = 0;

 MODULEENTRY32 ment32;
 ment32.dwSize = sizeof(MODULEENTRY32);
 HANDLE hModulesSnap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, 0);
 if(Module32First(hModulesSnap, &ment32))
  {
   do
    {
     wsprintf((LPSTR)&ExText,"%s\t\t\t%08X\t%08X\t%s\r\n",(LPSTR)&ment32.szModule,ment32.modBaseAddr,ment32.modBaseSize,(LPSTR)&ment32.szExePath);
     WriteFile(hFile,&ExText,lstrlen((LPSTR)&ExText),&Result,NULL);
    }
     while(Module32Next(hModulesSnap, &ment32));
  }
 CloseHandle(hModulesSnap);
 ExText[0] = Buffer[0] = 0;

 lstrcat((LPSTR)&ExText,"-----------------------------------------------------------------------------\r\n");
 WriteFile(hFile,&ExText,lstrlen((LPSTR)&ExText),&Result,NULL);
 CloseHandle(hFile);
 return 0;
}
//====================================================================================
BOOL _stdcall ForceProcessSingleCore(HANDLE hProcess)
{
 DWORD ProcAffMask = 0;
 DWORD SystAffMask = 0;

 if(!GetProcessAffinityMask(hProcess,&ProcAffMask,&SystAffMask))return false;
 if(SystAffMask & 0xFFFFFFFE)  // Zero bit is always set for first core
  {
   for(UINT ctr=31;ctr > 0;ctr--) // Leave first core for something else and find next
	{
	 if(SystAffMask >> ctr)return SetProcessAffinityMask(hProcess, (((DWORD)1) << ctr));
	}
  }
 return false;
}
//====================================================================================
void _stdcall SetProcPriority(DWORD prior)
{
 switch (prior)
  {
   case 1:
	 prior = NORMAL_PRIORITY_CLASS;
	break;
   case 2:
	 prior = HIGH_PRIORITY_CLASS;
	break;
   case 3:
	 prior = REALTIME_PRIORITY_CLASS;
	break;
   default: prior = NORMAL_PRIORITY_CLASS;		
  }
 SetPriorityClass(GetCurrentProcess(), prior);
}
//====================================================================================
LONG WINAPI VecCrashHandler(PEXCEPTION_POINTERS ExceptionInfo)
{
 if(ModInfo->ExBeepExcept)Beep(1000,50);
 if(ModInfo->ExWrCrashLog)WriteExceptionLog((LPSTR)&ModInfo->ExcFilePath,ExceptionInfo->ExceptionRecord,ExceptionInfo->ContextRecord);
 switch(ExceptionInfo->ExceptionRecord->ExceptionCode)
  {
   case EXCEPTION_ACCESS_VIOLATION:
   case EXCEPTION_INT_DIVIDE_BY_ZERO:
   case EXCEPTION_FLT_DIVIDE_BY_ZERO:
    if(ModInfo->ExTryIgnore)
     {
      HDE_STRUCT hde;
      DWORD CurExTime  = GetTickCount()/1000; // To seconds
      PVOID ModuleBase = (PVOID)ExceptionInfo->ContextRecord->Eip; 
      if((ModInfo->LastExAddress == ExceptionInfo->ContextRecord->Eip)&&((CurExTime-ModInfo->LastExTime) < 4))
       {
        ModInfo->LastExCounter++;
        if(ModInfo->LastExCounter > ModInfo->CrashIgnoreMax)break;
       }
        else ModInfo->LastExCounter = 0;
       
      ModInfo->LastExTime    = CurExTime;
      ModInfo->LastExAddress = ExceptionInfo->ContextRecord->Eip;
	  //PVOID LocMod  = FindLocalModule(&ModuleBase,NULL);
      if(!ModInfo->ExPassKern32 || (FindLocalModule(&ModuleBase,NULL) != ModInfo->Kernel32Base))  //	((LocMod != Kernel32Base)&&(LocMod != KernelBaseBase))
       {
        hde_disasm((PVOID)ExceptionInfo->ContextRecord->Eip, &hde);
        ExceptionInfo->ContextRecord->Eip += hde.Len;
       }
      return EXCEPTION_CONTINUE_EXECUTION;
     }   
  }
 return EXCEPTION_CONTINUE_SEARCH;
}
//====================================================================================
}


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
 GetModuleFileName(hDllMod,(LPSTR)&ModInfo->CurrentDir,sizeof(ModInfo->CurrentDir));
 GetModuleFileName(ModInfo->ExeModuleBase,(LPSTR)&ModInfo->GameCurDir,sizeof(ModInfo->GameCurDir));	     	
	 
 TrimFilePath((LPSTR)&ModInfo->CurrentDir);
 TrimFilePath((LPSTR)&ModInfo->GameCurDir);
 if(lstrcmpi((LPSTR)&ModInfo->CurrentDir,(LPSTR)&ModInfo->GameCurDir)==0)lstrcat((LPSTR)&ModInfo->CurrentDir,"mge3\\");   // Work with MGE

 LogMsg("Main EXE module loaded at %08X.", (DWORD)ModInfo->ExeModuleBase); 
 if(GetProcAddress(ModInfo->ExeModuleBase,MLDRPROCNAME)){ModInfo->ModuleLoader=true;return NULL;}   // Just if the function present     
  
 LogMsg("Starting %s...",MOD_NAME);   
 ApplyPatches(CONFIGINIFILE,SIGNATUREFILE,PATCHFILENAME);
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
DWORD  _stdcall ModRunConfigShell(DWORD Reserved)
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
//					               OTHER FUNCTIONS
//====================================================================================
void _stdcall ApplyPatches(LPSTR CfgFileName, LPSTR SigFileName, LPSTR PatFileName)
{
 HANDLE  hDataFile;
 PVOID   PatchData;
 DWORD   PatchSize;
 PVOID   ConfigData;
 DWORD   ConfigSize;
 DWORD   PatchCount;
 PDWORD  PatchTable;
 BYTE    CfgFilePath[MAX_PATH];
 BYTE    SigFilePath[MAX_PATH];
 BYTE    PatFilePath[MAX_PATH];

 LogMsg("Beginning apply patches..."); 
 if(CfgFileName[1]!= ':'){lstrcpy((LPSTR)&CfgFilePath,(LPSTR)&ModInfo->CurrentDir);lstrcat((LPSTR)&CfgFilePath, CfgFileName);CfgFileName = (LPSTR)&CfgFilePath;}
 if(SigFileName[1]!= ':'){lstrcpy((LPSTR)&SigFilePath,(LPSTR)&ModInfo->CurrentDir);lstrcat((LPSTR)&SigFilePath, SigFileName);SigFileName = (LPSTR)&SigFilePath;}
 if(PatFileName[1]!= ':'){lstrcpy((LPSTR)&PatFilePath,(LPSTR)&ModInfo->CurrentDir);lstrcat((LPSTR)&PatFilePath, PatFileName);PatFileName = (LPSTR)&PatFilePath;}

 hDataFile = CreateFile(PatFileName,GENERIC_READ,FILE_SHARE_READ|FILE_SHARE_WRITE,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL|FILE_FLAG_SEQUENTIAL_SCAN,NULL);
 if(hDataFile != INVALID_HANDLE_VALUE)
  {
   PatchSize = GetFileSize(hDataFile,NULL);
   PatchData = HeapAlloc(ModInfo->DefHeapHandle,0,PatchSize);
   if(PatchData)ReadFile(hDataFile,PatchData,PatchSize,&PatchSize,NULL);
   CloseHandle(hDataFile);
  }
   else LogMsg("Cannot open a patch data file!"); 

 hDataFile   = CreateFile(CfgFileName,GENERIC_READ,FILE_SHARE_READ|FILE_SHARE_WRITE,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL|FILE_FLAG_SEQUENTIAL_SCAN,NULL);
 if(hDataFile != INVALID_HANDLE_VALUE)
  {
   ConfigSize  = GetFileSize(hDataFile,NULL);
   ConfigData  = HeapAlloc(ModInfo->DefHeapHandle,0,ConfigSize);
   if(ConfigData)ReadFile(hDataFile,ConfigData,ConfigSize,&ConfigSize,NULL);
   CloseHandle(hDataFile);
  }
   else LogMsg("Cannot open a configuration file!"); 

 PatchCount    = GetPatchesCount(PatchData, PatchSize);
 PatchTable    = (PDWORD)HeapAlloc(ModInfo->DefHeapHandle,0,(sizeof(DWORD)*PatchCount));
 PatchCount    = GetEnabledPatches((LPSTR)ConfigData, &PatchTable[1])+1;
 PatchTable[0] = 0;  // Always inject control procedures  (need for some patches)
 for(int ctr=0;(ctr < (int)PatchCount)&&(PatchCount > 1);ctr++)
  { 
   if(!IsPatchSignaturesOk(SigFileName, PatchData, PatchSize, PatchTable[ctr])){LogMsg("Signature not match for '%08X' patch!",PatchTable[ctr]);continue;}
   LogMsg("Applying patch '%08X'.", PatchTable[ctr]);
   if(WritePatches(PatchData, PatchSize, PatchTable[ctr]))LogMsg("Failed to apply patch '%08X'!",PatchTable[ctr]);
 //    else LogMsg("Patch '%08X' OK.",PatchTable[ctr]);
  }

 LogMsg("Patching finished."); 
 HeapFree(ModInfo->DefHeapHandle,0,PatchData);
 HeapFree(ModInfo->DefHeapHandle,0,ConfigData);
 HeapFree(ModInfo->DefHeapHandle,0,PatchTable);
}
//====================================================================================
DWORD _stdcall GetPatchesCount(PVOID PatchBlock, DWORD BlockSize)
{
 MWPATCH *CurPatch;
 DWORD   PatchCount  = 0;
 DWORD   PatchOffset = 0;
 
 while(PatchOffset < BlockSize)
  {
   CurPatch     = (MWPATCH*)&((PBYTE)PatchBlock)[PatchOffset];
   PatchOffset += (CurPatch->PatchLength+sizeof(MWPATCH));
   PatchCount++;
  }
 return PatchCount;
}
//====================================================================================
bool _stdcall IsPatchSignaturesOk(LPSTR SigFileName, PVOID PatchBlock, DWORD BlockSize, DWORD PatchId)
{
 MWPATCH *CurPatch;
 PVOID   PatchAddress; 
 LPSTR   CurSignature;
 DWORD   PatchOffset = 0;
 BOOL    Result      = true;
 BYTE    PatchNumber[10];
 BYTE    PatchOffsEt[10];   // Compiler has a BUG: This name cannot be 'PatchOffset' !!! 
 
 while(Result && (PatchOffset < BlockSize))
  {
   CurPatch     = (MWPATCH*)&((PBYTE)PatchBlock)[PatchOffset];
   PatchOffset += (CurPatch->PatchLength+sizeof(MWPATCH));
   if(CurPatch->PatchID != PatchId)continue;
   PatchAddress = &((PBYTE)ModInfo->ExeModuleBase)[FileOffsetToRva(ModInfo->ExeModuleBase,CurPatch->PatchOffset,NULL)];  
   CurSignature = (LPSTR)HeapAlloc(ModInfo->DefHeapHandle,0,(CurPatch->PatchLength*2)+32);
   ConvertToHexDW(CurPatch->PatchID, 8, (LPSTR)&PatchNumber,true);
   ConvertToHexDW(CurPatch->PatchOffset, 8, (LPSTR)&PatchOffsEt,true);
   GetPrivateProfileString((LPSTR)&PatchNumber,(LPSTR)&PatchOffsEt,"",CurSignature,(CurPatch->PatchLength*2)+16,SigFileName);
   Result = IsMemSignatureMatch(PatchAddress, CurSignature, lstrlen(CurSignature));
   HeapFree(ModInfo->DefHeapHandle,0,CurSignature);
  } 
 return Result;
}
//====================================================================================
DWORD _stdcall WritePatches(PVOID PatchBlock, DWORD BlockSize, DWORD PatchId)
{
 MWPATCH *CurPatch;
 PVOID   PatchAddress; 
 DWORD   PatchOffset = 0;

 while(PatchOffset < BlockSize)
  {
   CurPatch     = (MWPATCH*)&((PBYTE)PatchBlock)[PatchOffset];
   PatchOffset += (CurPatch->PatchLength+sizeof(MWPATCH));
   if(CurPatch->PatchID != PatchId)continue;
   PatchAddress = &((PBYTE)ModInfo->ExeModuleBase)[FileOffsetToRva(ModInfo->ExeModuleBase,CurPatch->PatchOffset,NULL)];  
   if(WriteLocalProtectedMemory(PatchAddress, CurPatch->PatchData, CurPatch->PatchLength, true) < CurPatch->PatchLength)return 1;  
  }
 return 0;
}
//====================================================================================
DWORD _stdcall GetEnabledPatches(LPSTR CfgFile, PDWORD PatchTable)
{
 int   CurPos;
 int   PatchCtr = 0;
 DWORD CurPatchID;
 
 while((CurPos=CharPosFromLeft(CfgFile, '\n', 0, 0)) >= 0)
  {
   CfgFile   += CurPos+1;
   if(CfgFile[0] == '[')continue;
   CurPatchID = HexStrToDW(CfgFile);
   if((CurPos = CharPosFromLeft(CfgFile, '=', 0, 0)) < 0)continue;
   CfgFile   += CurPos;
   if(CfgFile[1] == '0')continue;   // Patch disabled
   PatchTable[PatchCtr] = CurPatchID;
   PatchCtr++;
  }
 return PatchCtr;
}
//====================================================================================
}

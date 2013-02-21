
#include "ExtensionMod.h"

using namespace MWDerived;
using namespace MODULENAME;

MODULENAME::PSModData  ModInfo;
//====================================================================================
//
//====================================================================================
DLLMAINTYPE BOOL APIENTRY DLLMain(HMODULE hModule, DWORD ReasonCall, LPVOID lpReserved)
{
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

 ModInfo->VoiceInf[VoiceMale].Voice.UnInitialize();
 ModInfo->VoiceInf[VoiceFemale].Voice.UnInitialize();
 ModInfo->VoiceInf[VoiceCreature].Voice.UnInitialize();
 ModInfo->VoiceInf[VoiceDocument].Voice.UnInitialize();

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

 DWORD  Index;
 DWORD  Result;
      
 ModInfo = new SModData();  
 ModInfo->ThisLibBase      = hDllMod; 
 ModInfo->hDefHeap         = GetProcessHeap();
 ModInfo->ExeModuleBase    = GetModuleHandle(NULL);
 PBYTE PatchBase           = (PBYTE)ModInfo->ExeModuleBase;
 GetModuleFileName(ModInfo->ThisLibBase,(LPSTR)&ModInfo->CurrentDir,sizeof(ModInfo->CurrentDir));    
 GetModuleFileName(ModInfo->ExeModuleBase,(LPSTR)&ModInfo->GameCurDir,sizeof(ModInfo->GameCurDir));	// EXE path 
      
 TrimFilePath((LPSTR)&ModInfo->CurrentDir);
 TrimFilePath((LPSTR)&ModInfo->GameCurDir);
 if(lstrcmpi((LPSTR)&ModInfo->CurrentDir,(LPSTR)&ModInfo->GameCurDir)==0)lstrcat((LPSTR)&ModInfo->CurrentDir,"mge3\\");   // Work with MGE

 lstrcpy((LPSTR)&ModInfo->CfgFilePath, (LPSTR)&ModInfo->CurrentDir);
 lstrcat((LPSTR)&ModInfo->CfgFilePath, CONFIGINIFILE);
 lstrcpy((LPSTR)&ModInfo->SpeechDirectory, (LPSTR)&ModInfo->GameCurDir); 
 lstrcat((LPSTR)&ModInfo->SpeechDirectory, "Data Files\\Speech\\"); 
                                                    
 LoadConfiguration((LPSTR)&ModInfo->CfgFilePath);
 
 LogMsg("Main EXE module loaded at %08X.", (DWORD)ModInfo->ExeModuleBase);     
 if(GetProcAddress(ModInfo->ExeModuleBase,MLDRPROCNAME)){ModInfo->ModuleLoader=true;return NULL;}   // Just if the function present     
  
 LogMsg("Starting %s...",MOD_NAME);   	           
 Result    = Index = 0; 
 PatchBase = (PBYTE)ModInfo->ExeModuleBase;
 //================================================================================
 // TODO: Place hooks into a linked list
 // For making a hook chains in modiles, better DO NOT USE 'SetCmdValue' ans 'SetAddressInVFT'.  TODO: Make fast chaining hooks for 'CALL XXXX' instead of directly setting address
 LogMsg("Setting hook on checking 'MouseLookDisabled' flag.");
 Result += ModInfo->CodeHooks[Index++].SetCodeHook(&PatchBase[0x00169264],GetAddress<CODEHOOK32::CODEHOOKPROC32>(__CamLookDisabling),"8A85B20500");   

 //-------------------------------------------------------------------    
 LogMsg("Setting hook on camera`s vertical delta writing.");
 Result += ModInfo->CodeHooks[Index++].SetCodeHook(&PatchBase[0x00169148],GetAddress<CODEHOOK32::CODEHOOKPROC32>(__CamLookVertDelta),"84C0895C24");  

 //-------------------------------------------------------------------    
 LogMsg("Setting hook on camera`s horizontal delta writing.");
 Result += ModInfo->CodeHooks[Index++].SetCodeHook(&PatchBase[0x00169120],GetAddress<CODEHOOK32::CODEHOOKPROC32>(__CamLookHortDelta),"894C241C8B");  
  	 	  	
 //-------------------------------------------------------------------                                      
 LogMsg("Setting hook on locking Player`s sneaking in menu mode.");
 Result += ModInfo->CodeHooks[Index++].SetCodeHook(&PatchBase[0x00169781],GetAddress<CODEHOOK32::CODEHOOKPROC32>(__PlayerSneakLocking),"8A85B00500");  
 	  	          
 //-------------------------------------------------------------------                                      
 LogMsg("Setting hook on checking 'GoToJail'.");
 Result += ModInfo->CodeHooks[Index++].SetCmdValue(&PatchBase[0x00167F9D],GetAddress<PVOID>(&CObjMACP::__GoToJailChecking),"E87E260000"); 
      	      
 //-------------------------------------------------------------------                                      
 LogMsg("Setting hook on 'AIDoActorAttack' - suspend hostile NPCs while PC in dialogue.");
 Result += ModInfo->CodeHooks[Index++].SetCmdValue(&PatchBase[0x0012EDF8],GetAddress<PVOID>(&CObjMACH::__AIDoActorAttack),"E823A30200"); 

 //-------------------------------------------------------------------  
 LogMsg("Setting hook for enabling hints in inventory when no an object focused.");
 Result += ModInfo->CodeHooks[Index++].SetCmdValue(&PatchBase[0x0001B3D6],GetAddress<PVOID>(&CRenderer::__ProcessPlayerUseAction),"E875160000");  
   
 //-------------------------------------------------------------------  
 LogMsg("Setting hook on exiting from menu mode.");
 Result += ModInfo->CodeHooks[Index++].SetCodeHook(&PatchBase[0x001951AB],GetAddress<CODEHOOK32::CODEHOOKPROC32>(__ExitMenuMode),"E8A0020000");  // 0x001951B0 patched by the 'Text.dll' 
          
 //-------------------------------------------------------------------	
 LogMsg("Setting hook on AI action changing.");
 Result += ModInfo->CodeHooks[Index++].SetCodeHook(&PatchBase[0x0012EAB5],GetAddress<CODEHOOK32::CODEHOOKPROC32>(__ActorAiActionChanging),"8B86C80000");  
      
 //-------------------------------------------------------------------          
 LogMsg("Setting hook to changing voice pitch.");
 Result += ModInfo->CodeHooks[Index++].SetCmdValue(&PatchBase[0x0008C6B6],GetAddress<PVOID>(&CSoundBase::__SetDSBufFrequency),"E8A563F7FF");   
  
 //-------------------------------------------------------------------
 LogMsg("Setting hook for pausing water riffles in menu mode.");
 Result += ModInfo->CodeHooks[Index++].SetCodeHook(&PatchBase[0x0011C896],GetAddress<CODEHOOK32::CODEHOOKPROC32>(__MenuFreezingWaterRiffles),"8A88D60000");  
                      
 //-------------------------------------------------------------------
 LogMsg("Setting hook for pausing doors moving in menu mode.");
 Result += ModInfo->CodeHooks[Index++].SetCodeHook(&PatchBase[0x000E721C],GetAddress<CODEHOOK32::CODEHOOKPROC32>(__MenuFreezingDoors),"8A88D60000");   

 //-------------------------------------------------------------------
 LogMsg("Setting hook for pausing shadows in menu mode.");
 Result += ModInfo->CodeHooks[Index++].SetCodeHook(&PatchBase[0x000388D0],GetAddress<CODEHOOK32::CODEHOOKPROC32>(__MenuFreezingShadows),"8A88D60000");   

 //-------------------------------------------------------------------
 LogMsg("Setting hook for pausing compass in menu mode.");
 Result += ModInfo->CodeHooks[Index++].SetCodeHook(&PatchBase[0x001EB7BC],GetAddress<CODEHOOK32::CODEHOOKPROC32>(__MenuFreezingCompass),"8A88D60000");  

 //-------------------------------------------------------------------
 LogMsg("Setting hook for pausing an active objects in menu mode.");   // WATCH FOR IT!
 Result += ModInfo->CodeHooks[Index++].SetCodeHook(&PatchBase[0x0000F68D],GetAddress<CODEHOOK32::CODEHOOKPROC32>(__MenuFreezingActiveObjects),"8A86D60000");  
                                                        
 //-------------------------------------------------------------------
 LogMsg("Setting hook for allowing head tracking in menu mode.");
 Result += ModInfo->CodeHooks[Index++].SetCodeHook(&PatchBase[0x0006F420],GetAddress<CODEHOOK32::CODEHOOKPROC32>(__DlgAllowingHeadTrack),"8A81D60000");  
                                                                                                     
 //-------------------------------------------------------------------  
 LogMsg("Setting hook for DIALOGS and JOURNAL.");
 Result += ModInfo->CodeHooks[Index++].SetCmdValue(&PatchBase[0x0000BE32],GetAddress<PVOID>(&CTESFont::__PrepareTextDlgJnl),"E8C9F6FFFF");  
            
 //-------------------------------------------------------------------
 LogMsg("Setting hook for opening books.");
 Result += ModInfo->CodeHooks[Index++].SetCmdValue(&PatchBase[0x000A2A15],GetAddress<PVOID>(__OpenBook),"E886981000");  
         
 //-------------------------------------------------------------------	              
 LogMsg("Setting hook for opening documents.");
 Result += ModInfo->CodeHooks[Index++].SetCmdValue(&PatchBase[0x002138FB],GetAddress<PVOID>(__OpenDocument),"E860F7FFFF");   
                                                                                                 
 //-------------------------------------------------------------------	 
 LogMsg("Setting hook for book page changing.");
 Result += ModInfo->CodeHooks[Index++].SetCmdValue(&PatchBase[0x001AC36B],GetAddress<PVOID>(__ChangeBookPageForward),"6870C95A00"); 
 Result += ModInfo->CodeHooks[Index++].SetCmdValue(&PatchBase[0x001ABE9C],GetAddress<PVOID>(__ChangeBookPageForward),"6870C95A00");   	                      
 Result += ModInfo->CodeHooks[Index++].SetCmdValue(&PatchBase[0x001AC358],GetAddress<PVOID>(__ChangeBookPageBackward),"68C0C85A00");   
 Result += ModInfo->CodeHooks[Index++].SetCmdValue(&PatchBase[0x001ABCBF],GetAddress<PVOID>(__ChangeBookPageBackward),"68C0C85A00");  
     
 //-------------------------------------------------------------------	   	 	  	   
 LogMsg("Setting hook on 'SendTextLine' for documents.");
 Result += ModInfo->CodeHooks[Index++].SetCmdValue(&PatchBase[0x0021375E],GetAddress<PVOID>(&CWindow::__DrawTextLine),"E87D54F7FF");        
 Result += ModInfo->CodeHooks[Index++].SetCmdValue(&PatchBase[0x0021378C],GetAddress<PVOID>(&CWindow::__DrawTextLine),"E84F54F7FF");    
                       
 //-------------------------------------------------------------------
 LogMsg("Setting hook for parsing book lines (Hook 'DrawTextLine').");
 Result += ModInfo->CodeHooks[Index++].SetCmdValue(&PatchBase[0x001AD9AF],GetAddress<PVOID>(&CWindow::__DrawTextLine),"E82CB2FDFF");   
 Result += ModInfo->CodeHooks[Index++].SetCmdValue(&PatchBase[0x001ADAB3],GetAddress<PVOID>(&CWindow::__DrawTextLine),"E828B1FDFF");   
     	   	     	     	   	     
 //-------------------------------------------------------------------
 LogMsg("Setting hook for finishing journal page.");
 Result += ModInfo->CodeHooks[Index++].SetCmdValue(&PatchBase[0x001D644E],GetAddress<PVOID>(__MakeJournalPage),"6860725D00"); 
 Result += ModInfo->CodeHooks[Index++].SetCmdValue(&PatchBase[0x001D6377],GetAddress<PVOID>(__MakeJournalPage),"6860725D00");  // Possible this is do not used!
 
 //-------------------------------------------------------------------	 
 LogMsg("Setting hook for Menu mode animation freezing control.");
 Result += ModInfo->CodeHooks[Index++].SetCodeHook(&PatchBase[0x0001B3AF],GetAddress<CODEHOOK32::CODEHOOKPROC32>(__MenuModeAnimationFreezing),"8A81D60000");   
     	   	     	     
 //-------------------------------------------------------------------	 
 LogMsg("Setting hook on dialogue begin.");
 Result += ModInfo->CodeHooks[Index++].SetCodeHook(&PatchBase[0x0012935A],GetAddress<CODEHOOK32::CODEHOOKPROC32>(__StartDialogue),"668B154234");  

 //-------------------------------------------------------------------
 LogMsg("Setting hook to loading voice file.");
 Result += ModInfo->CodeHooks[Index++].SetCmdValue(&PatchBase[0x0008C369],GetAddress<PVOID>(&CSoundBase::__OpenVoiceFile),"E8425AF7FF");  // See 'vadefs.h' (Variable argument lists) for more useful code examples           
                     
 //-------------------------------------------------------------------
 LogMsg("Setting hook on head tracking of NPC.");
 Result += ModInfo->CodeHooks[Index++].SetCmdValue(&PatchBase[0x0016F68B],GetAddress<PVOID>(&CAnimation::__HeadTrack),"E88002F0FF");  
   
 //-------------------------------------------------------------------
 LogMsg("Setting hook on AI state check.");  
 Result += ModInfo->CodeHooks[Index++].SetCodeHook(&PatchBase[0x00121D83],GetAddress<CODEHOOK32::CODEHOOKPROC32>(__ControlCurDlgActorAI),"F641240874"); 

 //-------------------------------------------------------------------	  	 
 LogMsg("Setting hook on changing view mode (1st - 3rd Person switch).");
 Result += ModInfo->CodeHooks[Index++].SetCmdValue(&PatchBase[0x00143A17],GetAddress<PVOID>(&CCamera::__SwitchViewMode),"E854EBFFFF"); 
                              
 //-------------------------------------------------------------------	
 LogMsg("Setting hook on changing dialogue topic.");
 Result += ModInfo->CodeHooks[Index++].SetCodeHook(&PatchBase[0x001C03F9],GetAddress<CODEHOOK32::CODEHOOKPROC32>(__ChangeDlgTopic),"75108B4770");    

 //-------------------------------------------------------------------	
 LogMsg("Setting hook on open_close any text document.");
 Result += ModInfo->CodeHooks[Index++].SetFuncHook(&PatchBase[0x001AD630],GetAddress<CODEHOOK32::FUNCHOOKPROC32>(__OpenCloseBookAndDoc),"8B0D??????");   
                
 //-------------------------------------------------------------------	
 LogMsg("Setting hook on 'ToggleGodMode'.");
 Result += ModInfo->CodeHooks[Index++].SetCodeHook(&PatchBase[0x00103D66],GetAddress<CODEHOOK32::CODEHOOKPROC32>(__ToggleGM),"8B89E40200");    

 //-------------------------------------------------------------------	
 LogMsg("Setting hook on journal key press checking.");
 Result += ModInfo->CodeHooks[Index++].SetCodeHook(&PatchBase[0x0001AECB],GetAddress<CODEHOOK32::CODEHOOKPROC32>(__IsJournalAllowed),"85C00F84BA");    

 //-------------------------------------------------------------------	  	 
 //LogMsg("Setting hook on changing anim group.");  // NO USE FOR THIS !
 //Result += ModInfo->CodeHooks[Index++].SetCmdValue(&PatchBase[0x00140EF9],GetAddress<PVOID>(&CAnimation::__ChangeAnimGroup),"E8E2FBF2FF"); 
                              
 //-------------------------------------------------------------------
 LogMsg("Setting hook on Player`s scene updating.");	                                                           
 Result += SetAddressInVFT(&PatchBase[0x0034AE30],1,&CCamera::__UpdatePlayerPositions);
                                                                                                                                                                                                                
 //================================================================================
 if(Result)
  {
   MessageBox(NULL,(ModInfo->Config.LangIndex)?(SINGERRORMSGEN):SINGERRORMSGRU,THISMODNAME,MB_ICONSTOP|MB_SYSTEMMODAL|MB_OK);
   FreeLibraryAndExitThread(ModInfo->ThisLibBase, 0);  // Finish process
  }
 //================================================================================

 if(!(ModInfo->hWorkerThread = CreateThread(NULL,0,&WorkerThread,NULL,CREATE_SUSPENDED,&ModInfo->WorkerThreadID))){LogMsg("Failed to create worker thread!");return false;}	      
 LogMsg("Worker thread are created successfully.");
 ResumeThread(ModInfo->hWorkerThread);     
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
//					          THE MAIN INTERNAL FUNCTIONS
//------------------------------------------------------------------------------------
DWORD WINAPI WorkerThread(LPVOID lpParameter)
{
 MSG        ThNewMsg;
 PVOICEINFO VoiceInf;
                              
 LogMsg("Entering worker thread...");
 if((VoiceInf=&ModInfo->VoiceInf[VoiceMale])->VoiceEnabled)
  {
   LogMsg("Trying to initialize 'VoiceMale'.");
   VoiceInf->Voice.Initialize(SPKTOMEMB, FALSE);
   VoiceInf->Voice.SetOutputFormat(SOAF44kHz16BitM);
   VoiceInf->Voice.SetVoiceConfig((LPSTR)&VoiceInf->VoiceConfig);
   if(VoiceInf->Voice.SetVoice((LPSTR)&VoiceInf->VoiceName))LogMsg("'VoiceMale' is failed to initialize.");
     else LogMsg("'VoiceMale' is initialized.");
  }

 if((VoiceInf=&ModInfo->VoiceInf[VoiceFemale])->VoiceEnabled)
  {
   LogMsg("Trying to initialize 'VoiceFemale'.");  
   VoiceInf->Voice.Initialize(SPKTOMEMB, FALSE);
   VoiceInf->Voice.SetOutputFormat(SOAF44kHz16BitM);
   VoiceInf->Voice.SetVoiceConfig((LPSTR)&VoiceInf->VoiceConfig);
   if(VoiceInf->Voice.SetVoice((LPSTR)&VoiceInf->VoiceName))LogMsg("'VoiceFemale' is failed to initialize.");
     else LogMsg("'VoiceFemale' is initialized.");
  }

 if((VoiceInf=&ModInfo->VoiceInf[VoiceCreature])->VoiceEnabled)
  {
   LogMsg("Trying to initialize 'VoiceCreature'.");  
   VoiceInf->Voice.Initialize(SPKTOMEMB, FALSE);
   VoiceInf->Voice.SetOutputFormat(SOAF44kHz16BitM);
   VoiceInf->Voice.SetVoiceConfig((LPSTR)&VoiceInf->VoiceConfig);
   if(VoiceInf->Voice.SetVoice((LPSTR)&VoiceInf->VoiceName))LogMsg("'VoiceCreature' is failed to initialize.");
     else LogMsg("'VoiceCreature' is initialized.");
  }

 if((VoiceInf=&ModInfo->VoiceInf[VoiceDocument])->VoiceEnabled && !ModInfo->Config.DocVoiceAsDlg)
  {
   LogMsg("Trying to initialize 'VoiceDocument'.");      
   VoiceInf->Voice.Initialize(SPKTOMEMB, FALSE);
   VoiceInf->Voice.SetOutputFormat(SOAF44kHz16BitM);
   VoiceInf->Voice.SetVoiceConfig((LPSTR)&VoiceInf->VoiceConfig);
   if(VoiceInf->Voice.SetVoice((LPSTR)&VoiceInf->VoiceName))LogMsg("'VoiceDocument' is failed to initialize.");
     else LogMsg("'VoiceDocument' is initialized.");
  }
   
 LogMsg("Entering a message loop.");   
 PeekMessage(&ThNewMsg, NULL, 0, MAXDWORD, PM_REMOVE);
 while(1)
  {
   if(ModInfo->GameInMenuMode || ModInfo->CurrentTalker->IsPlaying())PeekMessage(&ThNewMsg, NULL, MSG_START_SPEAK, MSG_THREADEXIT, PM_REMOVE);
     else GetMessage(&ThNewMsg, NULL, MSG_START_SPEAK, MSG_THREADEXIT);
//   LogMsg("Message received: %08X.",ThNewMsg.message);
   if(ThNewMsg.message == MSG_START_SPEAK){(reinterpret_cast<PCTalker>(ThNewMsg.wParam))->SpeakToBuffer();}                     
   if(ThNewMsg.message == MSG_THREADEXIT)break;
   ThNewMsg.message = NULL; 
   ThNewMsg.lParam  = NULL;
   ThNewMsg.wParam  = NULL; 
   Sleep(KEYWAITDEL);
 
   if(ModInfo->GameInMenuMode  && IsKeyCombinationPressed(ModInfo->Config.SpeakKeyCode) && !ModInfo->CurrentTalker->IsPlaying()){ModInfo->CurrentTalker->SendWavDataToObject();continue;}
   if(ModInfo->GameInMenuMode  && IsKeyCombinationPressed(ModInfo->Config.SilenceKeyCode)){ModInfo->CurrentTalker->Silence();continue;}        
   if(!ModInfo->GameInMenuMode && IsKeyCombinationPressed(ModInfo->Config.SilenceKeyCode)){ModInfo->CurrentTalker->SilenceAllTalkers();continue;}        
  }

 // When exiting from a game menu, this code will never be reached! 
 LogMsg("Exiting TalkyMorrowind...");
 DestroyModule();
 FreeLibraryAndExitThread(ModInfo->ThisLibBase,1);   
 return 0; 
}
//====================================================================================
void _stdcall LoadConfiguration(LPSTR CfgFilePath)    
{
 ModInfo->Config.SpeakKeyCode     = RefreshINIValueInt("BaseConfig","SpeakKeyCode",ModInfo->Config.SpeakKeyCode,CfgFilePath);
 ModInfo->Config.SilenceKeyCode   = RefreshINIValueInt("BaseConfig","SilenceKeyCode",ModInfo->Config.SilenceKeyCode,CfgFilePath);
 ModInfo->Config.DocVoiceAsDlg    = RefreshINIValueInt("BaseConfig","DocVoiceAsDlg",ModInfo->Config.DocVoiceAsDlg,CfgFilePath);
 ModInfo->Config.VoiceFxByRace    = RefreshINIValueInt("BaseConfig","VoiceFxByRace",ModInfo->Config.VoiceFxByRace,CfgFilePath);
 ModInfo->Config.SearchSpeechFile = RefreshINIValueInt("BaseConfig","SearchSpeechFile",ModInfo->Config.SearchSpeechFile,CfgFilePath);
 ModInfo->Config.FilterSpeechFile = RefreshINIValueInt("BaseConfig","FilterSpeechFile",ModInfo->Config.FilterSpeechFile,CfgFilePath);
 ModInfo->Config.TTSBoundPitchDS  = RefreshINIValueInt("BaseConfig","TTSBoundPitchDS",ModInfo->Config.TTSBoundPitchDS,CfgFilePath);
 ModInfo->Config.AlwReadOpenJnl   = RefreshINIValueInt("BaseConfig","AlwReadOpenJnl",ModInfo->Config.AlwReadOpenJnl,CfgFilePath);
 ModInfo->Config.StopSpkOnClose	  = RefreshINIValueInt("BaseConfig","StopSpkOnClose",ModInfo->Config.StopSpkOnClose,CfgFilePath);
 ModInfo->Config.PickPocketMode	  = RefreshINIValueInt("BaseConfig","PickPocketMode",ModInfo->Config.PickPocketMode,CfgFilePath);
 ModInfo->Config.AttackSuspMode	  = RefreshINIValueInt("BaseConfig","AttackSuspMode",ModInfo->Config.AttackSuspMode,CfgFilePath);		
 ModInfo->Config.DlgSwitchToFP	  = RefreshINIValueInt("BaseConfig","DlgSwitchToFP",ModInfo->Config.DlgSwitchToFP,CfgFilePath);
 ModInfo->Config.DlgEnableZoom    = RefreshINIValueInt("BaseConfig","DlgEnableZoom",ModInfo->Config.DlgEnableZoom,CfgFilePath);
 ModInfo->Config.SmoothZooming    = RefreshINIValueInt("BaseConfig","SmoothZooming",ModInfo->Config.SmoothZooming,CfgFilePath);                  
 ModInfo->Config.SpkAfterFocusNpc = RefreshINIValueInt("BaseConfig","SpkAfterFocusNpc",ModInfo->Config.SpkAfterFocusNpc,CfgFilePath);
 ModInfo->Config.ZoomDistNPC      = RefreshINIValueInt("BaseConfig","ZoomDistNPC",ModInfo->Config.ZoomDistNPC,CfgFilePath);
 ModInfo->Config.LangIndex	      = RefreshINIValueInt("BaseConfig","LangIndex",ModInfo->Config.LangIndex,CfgFilePath);
 ModInfo->Config.VPBaseMale	      = RefreshINIValueInt("BaseConfig","VPBaseMale",ModInfo->Config.VPBaseMale,CfgFilePath);
 ModInfo->Config.VPBaseFemale	  = RefreshINIValueInt("BaseConfig","VPBaseFemale",ModInfo->Config.VPBaseFemale,CfgFilePath);
 ModInfo->Config.VPBaseCreature	  = RefreshINIValueInt("BaseConfig","VPBaseCreature",ModInfo->Config.VPBaseCreature,CfgFilePath);

 ModInfo->Config.MPauseChargen      = RefreshINIValueInt("MenuConfig","MPauseChargen",ModInfo->Config.MPauseChargen,CfgFilePath);
 ModInfo->Config.MPauseService      = RefreshINIValueInt("MenuConfig","MPauseService",ModInfo->Config.MPauseService,CfgFilePath);
 ModInfo->Config.MPauseResting      = RefreshINIValueInt("MenuConfig","MPauseResting",ModInfo->Config.MPauseResting,CfgFilePath);
 ModInfo->Config.MPauseJournal      = RefreshINIValueInt("MenuConfig","MPauseJournal",ModInfo->Config.MPauseJournal,CfgFilePath);
 ModInfo->Config.MPauseDialogue     = RefreshINIValueInt("MenuConfig","MPauseDialogue",ModInfo->Config.MPauseDialogue,CfgFilePath);
 ModInfo->Config.MPauseContainer    = RefreshINIValueInt("MenuConfig","MPauseContainer",ModInfo->Config.MPauseContainer,CfgFilePath);   
 ModInfo->Config.MPauseInventory    = RefreshINIValueInt("MenuConfig","MPauseInventory",ModInfo->Config.MPauseInventory,CfgFilePath);
 ModInfo->Config.MPauseDocuments    = RefreshINIValueInt("MenuConfig","MPauseDocuments",ModInfo->Config.MPauseDocuments,CfgFilePath);
 ModInfo->Config.MPauseMessages     = RefreshINIValueInt("MenuConfig","MPauseMessages",ModInfo->Config.MPauseMessages,CfgFilePath); 
 ModInfo->Config.MPauseOptions      = RefreshINIValueInt("MenuConfig","MPauseOptions",ModInfo->Config.MPauseOptions,CfgFilePath);
 ModInfo->Config.MKeepAiChargen     = RefreshINIValueInt("MenuConfig","MKeepAiChargen",ModInfo->Config.MKeepAiChargen,CfgFilePath);
 ModInfo->Config.MKeepAiService	    = RefreshINIValueInt("MenuConfig","MKeepAiService",ModInfo->Config.MKeepAiService,CfgFilePath);
 ModInfo->Config.MKeepAiResting	    = RefreshINIValueInt("MenuConfig","MKeepAiResting",ModInfo->Config.MKeepAiResting,CfgFilePath);
 ModInfo->Config.MKeepAiJournal	    = RefreshINIValueInt("MenuConfig","MKeepAiJournal",ModInfo->Config.MKeepAiJournal,CfgFilePath);
 ModInfo->Config.MKeepAiDialogue    = RefreshINIValueInt("MenuConfig","MKeepAiDialogue",ModInfo->Config.MKeepAiDialogue,CfgFilePath);
 ModInfo->Config.MKeepAiContainer   = RefreshINIValueInt("MenuConfig","MKeepAiContainer",ModInfo->Config.MKeepAiContainer,CfgFilePath);  
 ModInfo->Config.MKeepAiInventory   = RefreshINIValueInt("MenuConfig","MKeepAiInventory",ModInfo->Config.MKeepAiInventory,CfgFilePath);
 ModInfo->Config.MKeepAiDocuments   = RefreshINIValueInt("MenuConfig","MKeepAiDocuments",ModInfo->Config.MKeepAiDocuments,CfgFilePath);
 ModInfo->Config.MKeepAiMessages    = RefreshINIValueInt("MenuConfig","MKeepAiMessages",ModInfo->Config.MKeepAiMessages,CfgFilePath); 
 ModInfo->Config.MKeepAiOptions     = RefreshINIValueInt("MenuConfig","MKeepAiOptions",ModInfo->Config.MKeepAiOptions,CfgFilePath);
 ModInfo->Config.MEnableGmChargen   = RefreshINIValueInt("MenuConfig","MEnableGmChargen",ModInfo->Config.MEnableGmChargen,CfgFilePath);
 ModInfo->Config.MEnableGmService   = RefreshINIValueInt("MenuConfig","MEnableGmService",ModInfo->Config.MEnableGmService,CfgFilePath);
 ModInfo->Config.MEnableGmResting   = RefreshINIValueInt("MenuConfig","MEnableGmResting",ModInfo->Config.MEnableGmResting,CfgFilePath);
 ModInfo->Config.MEnableGmJournal   = RefreshINIValueInt("MenuConfig","MEnableGmJournal",ModInfo->Config.MEnableGmJournal,CfgFilePath);
 ModInfo->Config.MEnableGmDialogue  = RefreshINIValueInt("MenuConfig","MEnableGmDialogue",ModInfo->Config.MEnableGmDialogue,CfgFilePath);
 ModInfo->Config.MEnableGmContainer = RefreshINIValueInt("MenuConfig","MEnableGmContainer",ModInfo->Config.MEnableGmContainer,CfgFilePath);  
 ModInfo->Config.MEnableGmInventory = RefreshINIValueInt("MenuConfig","MEnableGmInventory",ModInfo->Config.MEnableGmInventory,CfgFilePath);
 ModInfo->Config.MEnableGmDocuments = RefreshINIValueInt("MenuConfig","MEnableGmDocuments",ModInfo->Config.MEnableGmDocuments,CfgFilePath);
 ModInfo->Config.MEnableGmMessages  = RefreshINIValueInt("MenuConfig","MEnableGmMessages",ModInfo->Config.MEnableGmMessages,CfgFilePath); 
 ModInfo->Config.MEnableGmOptions   = RefreshINIValueInt("MenuConfig","MEnableGmOptions",ModInfo->Config.MEnableGmOptions,CfgFilePath);

 ModInfo->Config.VPOrc      = RefreshINIValueInt("RaceConfig","VPOrc",ModInfo->Config.VPOrc,CfgFilePath);
 ModInfo->Config.VPNord     = RefreshINIValueInt("RaceConfig","VPNord",ModInfo->Config.VPNord,CfgFilePath);
 ModInfo->Config.VPBreton   = RefreshINIValueInt("RaceConfig","VPBreton",ModInfo->Config.VPBreton,CfgFilePath);
 ModInfo->Config.VPKhajiit  = RefreshINIValueInt("RaceConfig","VPKhajiit",ModInfo->Config.VPKhajiit,CfgFilePath);
 ModInfo->Config.VPDarkElf  = RefreshINIValueInt("RaceConfig","VPDarkElf",ModInfo->Config.VPDarkElf,CfgFilePath);
 ModInfo->Config.VPHighElf  = RefreshINIValueInt("RaceConfig","VPHighElf",ModInfo->Config.VPHighElf,CfgFilePath);
 ModInfo->Config.VPWoodElf  = RefreshINIValueInt("RaceConfig","VPWoodElf",ModInfo->Config.VPWoodElf,CfgFilePath);
 ModInfo->Config.VPArgonian = RefreshINIValueInt("RaceConfig","VPArgonian",ModInfo->Config.VPArgonian,CfgFilePath);
 ModInfo->Config.VPImperial = RefreshINIValueInt("RaceConfig","VPImperial",ModInfo->Config.VPImperial,CfgFilePath);
 ModInfo->Config.VPRedguard = RefreshINIValueInt("RaceConfig","VPRedguard",ModInfo->Config.VPRedguard,CfgFilePath);
  
 ModInfo->VoiceInf[VoiceMale].VoiceEnabled = ModInfo->VoiceInf[VoiceFemale].VoiceEnabled = ModInfo->VoiceInf[VoiceCreature].VoiceEnabled = RefreshINIValueInt("TalkyDlg","VoiceEnable",1,CfgFilePath);
 ModInfo->VoiceInf[VoiceMale].AutoSpeak    = ModInfo->VoiceInf[VoiceFemale].AutoSpeak    = ModInfo->VoiceInf[VoiceCreature].AutoSpeak    = RefreshINIValueInt("TalkyDlg","AutoSpeak",1,CfgFilePath);
 RefreshINIValueStr("TalkyDlg","VoiceMaleName","?",(LPSTR)&ModInfo->VoiceInf[VoiceMale].VoiceName,sizeof(ModInfo->VoiceInf[VoiceMale].VoiceName),CfgFilePath);
 RefreshINIValueStr("TalkyDlg","VoiceFemaleName","?",(LPSTR)&ModInfo->VoiceInf[VoiceFemale].VoiceName,sizeof(ModInfo->VoiceInf[VoiceFemale].VoiceName),CfgFilePath);
 RefreshINIValueStr("TalkyDlg","VoiceCreatureName","?",(LPSTR)&ModInfo->VoiceInf[VoiceCreature].VoiceName,sizeof(ModInfo->VoiceInf[VoiceCreature].VoiceName),CfgFilePath);
 RefreshINIValueStr("TalkyDlg","VoiceMaleConfig","",(LPSTR)&ModInfo->VoiceInf[VoiceMale].VoiceConfig,sizeof(ModInfo->VoiceInf[VoiceMale].VoiceConfig),CfgFilePath);
 RefreshINIValueStr("TalkyDlg","VoiceFemaleConfig","",(LPSTR)&ModInfo->VoiceInf[VoiceFemale].VoiceConfig,sizeof(ModInfo->VoiceInf[VoiceFemale].VoiceConfig),CfgFilePath);
 RefreshINIValueStr("TalkyDlg","VoiceCreatureConfig","",(LPSTR)&ModInfo->VoiceInf[VoiceCreature].VoiceConfig,sizeof(ModInfo->VoiceInf[VoiceCreature].VoiceConfig),CfgFilePath);

 ModInfo->VoiceInf[VoiceDocument].VoiceEnabled = RefreshINIValueInt("TalkyDoc","VoiceEnable",1,CfgFilePath);
 ModInfo->VoiceInf[VoiceDocument].AutoSpeak    = RefreshINIValueInt("TalkyDoc","AutoSpeak",1,CfgFilePath);
 RefreshINIValueStr("TalkyDoc","VoiceName","?",(LPSTR)&ModInfo->VoiceInf[VoiceDocument].VoiceName,sizeof(ModInfo->VoiceInf[VoiceDocument].VoiceName),CfgFilePath);
 RefreshINIValueStr("TalkyDoc","VoiceConfig","",(LPSTR)&ModInfo->VoiceInf[VoiceDocument].VoiceConfig,sizeof(ModInfo->VoiceInf[VoiceDocument].VoiceConfig),CfgFilePath);
}
//====================================================================================
}
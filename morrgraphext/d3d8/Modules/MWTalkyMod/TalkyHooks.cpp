
#include "ExtensionMod.h"
#include "TalkyHooks.h"

using namespace MODULENAME;

extern MODULENAME::PSModData  ModInfo;
//====================================================================================
//                                      HOOKS
//------------------------------------------------------------------------------------
// All checkers of the 'ModInfo->PausedMenuMode' and 'GameClass->MenuMode' may be  
// called before '__MenuModeAnimationFreezing'.	Always keep 'ModInfo->PausedMenuMode' 
// in TRUE state until selecting a menu.
//------------------------------------------------------------------------------------
void _cdecl __OpenCloseBookAndDoc(void)  // Books, Scrolls, Notes and Journal
{
 BOOL DocOpened = ((PDWORD)&((PBYTE)ModInfo->ExeModuleBase)[0x003D2B10])[0];  // Unknow pointer
 if(DocOpened){if(ModInfo->Config.StopSpkOnClose)ModInfo->CurrentTalker->Silence();}
   else {ModInfo->CurrentTalker = ModInfo->CurrentTalker->PrepareNew((*ModInfo->MWGlobals.ppGameScene)->GetPlayerActor());ModInfo->GameInMenuMode = true;} 
}
//====================================================================================
void _cdecl __OpenBook(HVL LPSTR BookText)
{
 ModInfo->MWGlobals.OpenBook(BookText);
 ModInfo->CurrentTalker->InitDialogueText(); 
}
//====================================================================================
void _cdecl __OpenDocument(HVL MORROWIND::PCWindow TxtClass, HVL LPSTR DocText)
{
 ModInfo->MWGlobals.OpenDocument(TxtClass, DocText); 
 ModInfo->CurrentTalker->InitDialogueText(); 
}
//====================================================================================
BOOL _cdecl __ChangeBookPageForward(HVL MORROWIND::PCWindow TxtClass00,HVL short TgtWndID00,HVL DWORD UnkRndTime,HVL short TgtWndID01,HVL MORROWIND::PCWindow TxtClass01)
{
 ModInfo->CurrentTalker->ClearTextLines(); 
 BOOL Result = ModInfo->MWGlobals.BookPageForward(TxtClass00, TgtWndID00, UnkRndTime, TgtWndID01, TxtClass01);
 ModInfo->CurrentTalker->InitDialogueText(); 
 return Result;
}
//====================================================================================
BOOL _cdecl __ChangeBookPageBackward(HVL MORROWIND::PCWindow TxtClass00,HVL short TgtWndID00,HVL DWORD UnkRndTime,HVL short TgtWndID01,HVL MORROWIND::PCWindow TxtClass01)
{
 ModInfo->CurrentTalker->ClearTextLines(); 
 BOOL Result = ModInfo->MWGlobals.BookPageBackward(TxtClass00, TgtWndID00, UnkRndTime, TgtWndID01, TxtClass01);
 ModInfo->CurrentTalker->InitDialogueText(); 
 return Result;
}
//====================================================================================
BOOL _cdecl __MakeJournalPage(HVL MORROWIND::PCWindow TxtClass00,HVL short TgtWndID00,HVL DWORD UnkRndTime,HVL short TgtWndID01,HVL MORROWIND::PCWindow TxtClass01)
{
 ModInfo->CurrentTalker->ClearTextLines(); 
 BOOL Result = ModInfo->MWGlobals.MakeJournalPage(TxtClass00, TgtWndID00, UnkRndTime, TgtWndID01, TxtClass01);
 if(!ModInfo->Config.AlwReadOpenJnl)ModInfo->CurrentTalker->SetFinalCRC(ModInfo->LastJnlCS);
 if(!ModInfo->CurrentTalker->IsEmpty()&&*(ModInfo->MWGlobals.PreparedText)){ModInfo->CurrentTalker->InitDialogueText();ModInfo->LastJnlCS = ModInfo->CurrentTalker->GetFinalCRC();}  
 return Result;
}
//====================================================================================
BOOL _cdecl __ChangeDlgTopic(HVL DWORD Reg_EIP,HVL DWORD Reg_EFL,HVL PVOID CUnkCopyA,HVL PVOID CUnkCopyB,HVL MORROWIND::PCObjDIAL Dialogue,HVL DWORD Reg_ESP,HVL MORROWIND::PCObjINFO Info,HVL DWORD Reg_EDX,HVL PVOID CUnkCopyC,HVL DWORD Reg_EAX,HVL long Und0,HVL long Und1,HVL PVOID RetAddr,HVL PVOID Unkn00,HVL LPSTR Text,HVL DWORD Type,HVL DWORD Unkn01,HVL MORROWIND::PCObjMACT Actor,...)
{
 switch(Type)
  {
   case tTopicName:
     break;
   case tGreeting:     
   case tTopicText:
     ModInfo->CurrentTalker->SetFinalCRC(-1);
     ModInfo->CurrentTalker->ClearTextLines();
     ModInfo->CurrentTalker->UpdateTextLine(*(ModInfo->MWGlobals.PreparedText));  // Prepared unduplicated text
     ModInfo->CurrentTalker->UpdateRawTextLine(Text);
     ModInfo->CurrentTalker->InitDialogueText(); 
     break;
  } 
 return true;
}
//====================================================================================
BOOL _cdecl __StartDialogue(HVL DWORD Reg_EIP,HVL DWORD Reg_EFL,HVL DWORD Reg_EDI,HVL MORROWIND::PCObjMACT Actor,HVL MORROWIND::PCObjDIAL Dialogue,HVL DWORD Reg_ESP,HVL MORROWIND::PCObjINFO DlgInfo,HVL DWORD Reg_EDX,HVL DWORD Reg_ECX,HVL DWORD Reg_EAX,...)
{
 ModInfo->GameInMenuMode = true;   // called earlier than '__MenuModeAnimationFreezing' function
 ModInfo->CurrentTalker  = ModInfo->CurrentTalker->PrepareNew(Actor); 
 ModInfo->TgtObjectRef   = (Actor)?(Actor->GetReference()):(NULL);
 ForceFaceToPlayer(Actor);
 if(ModInfo->Config.DlgEnableZoom)
  {
   ZoomDialogueActor(Actor);   
   SetCameraToDlgActor(Actor);
  }
 return true;
}
//====================================================================================
BOOL _cdecl __ControlCurDlgActorAI(HVL DWORD Reg_EIP,HVL DWORD Reg_EFL,HVL PVOID UnkClass,HVL MORROWIND::PCObjMACT Actor,HVL DWORD Reg_EBP,HVL DWORD Reg_ESP,HVL DWORD Reg_EBX,HVL MORROWIND::PCObjREFR ActorRef,HVL DWORD Reg_ECX,HVL DWORD Reg_EAX,...)
{
 if(!ModInfo->GameInMenuMode||ModInfo->PausedMenuMode)return true;  
 if(ModInfo->CurrentTalker->IsValid()&&Actor&&(ModInfo->CurrentTalker->GetActor() == Actor))  // For current DLG NPC AI always enabled but anim group changing is disabled, so he can`t walk away while in dialogue
  {
   if((!ModInfo->CurrentTalker->GetActor()->CombatTarget)&&(miContainer != ModInfo->LastMenuID)){Reg_EIP += 39; return false;}        // TODO: Control directly an Anim group changing
     else return false;
  }
 if(ModInfo->EnabledAiMenuM)Reg_EIP += 39;
 return false;
}
//====================================================================================
BOOL _cdecl __CamLookDisabling(HVL DWORD Reg_EIP,HVL DWORD Reg_EFL,HVL DWORD Reg_EDI,HVL DWORD Reg_ESI,HVL MORROWIND::PCObjMACP Player,HVL DWORD Reg_ESP,HVL DWORD Reg_EBX,HVL DWORD Reg_EDX,HVL DWORD Reg_ECX,HVL DWORD Reg_EAX,...)
{
 Reg_EAX = ((bool)Player->MouseDisabled ^ (bool)ModInfo->CamTrackingActive);  
 return false;    
} 
//====================================================================================
BOOL _cdecl __CamLookVertDelta(HVL DWORD Reg_EIP,HVL DWORD Reg_EFL,HVL DWORD Reg_EDI,HVL DWORD Reg_ESI,HVL MORROWIND::PCObjMACP Player,HVL DWORD Reg_ESP,HVL DWORD Reg_EBX,HVL DWORD Reg_EDX,HVL DWORD Reg_ECX,HVL DWORD Reg_EAX,...)
{
 if(ModInfo->CamTrackingActive)Reg_EBX = ModInfo->CamRotDeltaY;      // Override vertical user`s control
 return true;
} 
//====================================================================================
BOOL _cdecl __CamLookHortDelta(HVL DWORD Reg_EIP,HVL DWORD Reg_EFL,HVL DWORD Reg_EDI,HVL DWORD Reg_ESI,HVL MORROWIND::PCObjMACP Player,HVL DWORD Reg_ESP,HVL DWORD Reg_EBX,HVL DWORD Reg_EDX,HVL DWORD Reg_ECX,HVL DWORD Reg_EAX,...)
{
 if(ModInfo->CamTrackingActive)Reg_ECX = ModInfo->CamRotDeltaX;      // Override horizontal user`s control
 return true;   
} 
//====================================================================================
BOOL _cdecl __PlayerSneakLocking(HVL DWORD Reg_EIP,HVL DWORD Reg_EFL,HVL DWORD Reg_EDI,HVL DWORD Reg_ESI,HVL MORROWIND::PCObjMACP Player,HVL DWORD Reg_ESP,HVL DWORD Reg_EBX,HVL DWORD Reg_EDX,HVL DWORD Reg_ECX,HVL DWORD Reg_EAX,...)
{
 if(((bool)ModInfo->GameInMenuMode & (bool)ModInfo->PCSneakBefore))Player->SetActorSneaking(true);
 return true;
}
//====================================================================================
BOOL _cdecl __ActorAiActionChanging(HVL DWORD Reg_EIP,HVL DWORD Reg_EFL,HVL DWORD Reg_EDI,HVL MORROWIND::PCObjMACT Actor,HVL DWORD Reg_EBP,HVL DWORD Reg_ESP,HVL DWORD Reg_EBX,HVL DWORD Reg_EDX,HVL DWORD Reg_ECX,HVL DWORD Reg_EAX,...)
{
 if(ModInfo->GameInMenuMode&&ModInfo->CurrentTalker->IsValid()&&(ModInfo->CurrentTalker->GetActor() == Actor)){Reg_EAX = 0;return false;}        // Remove animation flicking for dialogue NPC
 return true;
}
//====================================================================================
BOOL _cdecl __MenuFreezingWaterRiffles(HVL DWORD Reg_EIP,HVL DWORD Reg_EFL,HVL DWORD Reg_EDI,HVL PVOID UnkClass,HVL DWORD Reg_EBP,HVL DWORD Reg_ESP,HVL DWORD Reg_EBX,HVL DWORD Reg_EDX,HVL DWORD Reg_ECX,HVL MORROWIND::PCGameScene GameClass,...)
{
 Reg_ECX = (bool)((bool)ModInfo->PausedMenuMode & (bool)GameClass->MenuMode); 
 return false;
}
//====================================================================================
BOOL _cdecl __MenuFreezingDoors(HVL DWORD Reg_EIP,HVL DWORD Reg_EFL,HVL MORROWIND::PCObjREFR ObjectA,HVL MORROWIND::PCObjREFR ObjectB,HVL MORROWIND::PCObjCELL Cell,HVL DWORD Reg_ESP,HVL DWORD Reg_EBX,HVL DWORD Reg_EDX,HVL DWORD Reg_ECX,HVL MORROWIND::PCGameScene GameClass,...)
{
 Reg_ECX = (bool)((bool)ModInfo->PausedMenuMode & (bool)GameClass->MenuMode); 
 return false;
}
//====================================================================================
BOOL _cdecl __MenuFreezingShadows(HVL DWORD Reg_EIP,HVL DWORD Reg_EFL,HVL PVOID UnkClass,HVL DWORD Reg_ESI,HVL DWORD Reg_EBP,HVL DWORD Reg_ESP,HVL DWORD Reg_EBX,HVL DWORD Reg_EDX,HVL DWORD Reg_ECX,HVL MORROWIND::PCGameScene GameClass,...)
{
 Reg_ECX = (bool)((bool)ModInfo->PausedMenuMode & (bool)GameClass->MenuMode); 
 return false;
}
//====================================================================================
BOOL _cdecl __MenuFreezingCompass(HVL DWORD Reg_EIP,HVL DWORD Reg_EFL,HVL DWORD Reg_EDI,HVL PVOID UnkClass,HVL DWORD Reg_EBP,HVL DWORD Reg_ESP,HVL DWORD Reg_EBX,HVL DWORD Reg_EDX,HVL DWORD Reg_ECX,HVL MORROWIND::PCGameScene GameClass,...)
{
 Reg_ECX = (bool)((bool)ModInfo->PausedMenuMode & (bool)GameClass->MenuMode); 
 return false;
}
//====================================================================================
BOOL _cdecl __MenuFreezingActiveObjects(HVL DWORD Reg_EIP,HVL DWORD Reg_EFL,HVL DWORD Reg_EDI,HVL MORROWIND::PCGameScene GameClass,HVL DWORD Reg_EBP,HVL DWORD Reg_ESP,HVL DWORD Reg_EBX,HVL DWORD Reg_EDX,HVL PVOID UnkClass,HVL DWORD Reg_EAX,...)
{
 Reg_EAX = (bool)((bool)ModInfo->PausedMenuMode & (bool)GameClass->MenuMode);   // Freeze activators if = 1
 return false;   // The game will crash with some plugins, if activators are enabled in title menu.
}
//====================================================================================
BOOL _cdecl __DlgAllowingHeadTrack(HVL DWORD Reg_EIP,HVL DWORD Reg_EFL,HVL DWORD UnkValue,HVL PVOID UnkClass,HVL DWORD Reg_EBP,HVL DWORD Reg_ESP,HVL DWORD Reg_EBX,HVL DWORD Reg_EDX,HVL MORROWIND::PCGameScene GameClass,HVL DWORD Reg_EAX,...)
{
 Reg_EAX = (bool)((bool)ModInfo->PausedMenuMode & (bool)GameClass->MenuMode); 
 return false;
}
//====================================================================================
BOOL _cdecl __ToggleGM(HVL DWORD Reg_EIP,HVL DWORD Reg_EFL,HVL DWORD Reg_EDI,HVL DWORD Reg_ESI,HVL DWORD Reg_EBP,HVL DWORD Reg_ESP,HVL DWORD Reg_EBX,HVL DWORD Reg_EDX,HVL DWORD Reg_ECX,HVL DWORD Reg_EAX,...) 
{
 ModInfo->LastGodMode = !(*ModInfo->MWGlobals.ppGameScene)->GetGodMode();
 return true;
}
//====================================================================================
BOOL _cdecl __IsJournalAllowed(HVL DWORD Reg_EIP,HVL DWORD Reg_EFL,HVL DWORD Reg_EDI,HVL DWORD Reg_ESI,HVL DWORD Reg_EBP,HVL DWORD Reg_ESP,HVL DWORD Reg_EBX,HVL DWORD Reg_EDX,HVL DWORD Reg_ECX,HVL DWORD JnlKeyState,...)
{
 if(ModInfo->GameInMenuMode && (miJournal != (*ModInfo->MWGlobals.ppGameScene)->GetMenuID()))JnlKeyState = 0;  // Allow the 'J' button only in journal
 return true;
}
//====================================================================================
BOOL _cdecl __ExitMenuMode(HVL DWORD Reg_EIP,HVL DWORD Reg_EFL,HVL DWORD Reg_EDI,HVL DWORD Reg_ESI,HVL DWORD Reg_EBP,HVL DWORD Reg_ESP,HVL DWORD Reg_EBX,HVL DWORD Reg_EDX,HVL DWORD Reg_ECX,HVL DWORD Reg_EAX,...)
{
 if(ModInfo->GameInMenuMode)   // Keep also initial menu ID
  {
   if(ModInfo->Config.StopSpkOnClose)ModInfo->CurrentTalker->SilenceAllTalkers();   // Free CPU from reading first
   if(ModInfo->LastFOV >= 0)(*ModInfo->MWGlobals.ppGameScene)->SetCurrentFOV(ModInfo->LastFOV);
   (*ModInfo->MWGlobals.ppGameScene)->SetViewMode(ModInfo->PrevPViewMode);  
   (*ModInfo->MWGlobals.ppGameScene)->ShowCrosshair(TRUE);
   if(ModInfo->LastMenuID == miDialogue)
    {
     (*ModInfo->MWGlobals.ppGameScene)->CamMovMultX = ModInfo->CamMovMPrevX;
     (*ModInfo->MWGlobals.ppGameScene)->CamMovMultY = ModInfo->CamMovMPrevY;
	}
   ModInfo->LastGodMode = (*ModInfo->MWGlobals.ppGameScene)->SetGodMode(ModInfo->LastGodMode);
   ModInfo->TgtObjectRef        = NULL;
   ModInfo->PausedMenuMode      = true;	  // Exiting menu mode and leaving it in PAUSED state
   ModInfo->GameInMenuMode      = false;
   ModInfo->ZoomingInProces     = false;
   ModInfo->CamTrackingActive   = false; 
   ModInfo->MenuModeResult      = 2;
   ModInfo->CamRotDeltaX        = 0;
   ModInfo->CamRotDeltaY        = 0;
   ModInfo->StartingFOV         = -1;
   ModInfo->CurrentFOV          = -1;
   ModInfo->LastFOV             = -1; 
   ModInfo->LastMenuID          = 0;
  }
 return true;
} 
//====================================================================================
// WARNING: This function will be called VERY FRIQUENTLY while the game in menu mode
BOOL _cdecl __MenuModeAnimationFreezing(HVL DWORD Reg_EIP,HVL DWORD Reg_EFL,HVL MORROWIND::PCRenderer Renderer,HVL DWORD Reg_ESI,HVL DWORD Reg_EBP,HVL DWORD Reg_ESP,HVL BOOL PauseAnims2,HVL MORROWIND::PCObjREFR SomeRef,HVL MORROWIND::PCGameScene GameClass,HVL BOOL PauseAnims,...) 
{
 bool EnableGM  = true;
 //MORROWIND::PCGameScene GameClass = (*ModInfo->MWGlobals.ppGameScene);

 if(!GameClass->MenuMode || !GameClass->GetPlayerActor())return true;  // Checking MACP Detects buggy call of an inventory on the title screen after start (with some bad plugins)   
 if(ModInfo->ZoomingInProces && ModInfo->LastMenuID)UpdateCameraZooming();      
 if(ModInfo->CamTrackingActive && ModInfo->LastMenuID)UpdateCameraTracking();    
 if((ModInfo->Config.PickPocketMode == ppmCloseMenu)&&(ModInfo->LastMenuID == miContainer))if(TestTargetMoveDist(ModInfo->TgtObjectRef,&ModInfo->PlayerTgtPos,TgtObjMoveDist))ModInfo->MWGlobals.CloseAllMenus(1);  // 'GameClass->GetMenuID()' replaced with 'ModInfo->LastMenuID' to fix a glitch with a companion`s inventory.
 if(ModInfo->MenuModeResult < 2){PauseAnims = (GameClass->GetMenuID() == miRestHours)?(1):(ModInfo->MenuModeResult);return false;}  // Allways pause the game when resting hours counted. The Game will make reposition of a MOBs.
   
 ModInfo->GameInMenuMode = true;
 ModInfo->MenuModeResult = 1; 
 ModInfo->EnabledAiMenuM = false;
 ModInfo->LastCamCtrl    = GameClass->GetPlayerActor()->GetLooking();
 ModInfo->LastKeyCtrl    = GameClass->GetPlayerActor()->GetMovement();
 ModInfo->LastGodMode    = GameClass->GetGodMode();
 ModInfo->PrevPViewMode  = GameClass->GetViewTPM();
 ModInfo->PCSneakBefore  = GameClass->GetPlayerActor()->IsActorSneaking();
 ModInfo->LastMenuID     = GameClass->GetMenuID();

 switch(GameClass->GetMenuID())
  {
   case miTitle:		  // Title menu (No game loaded yet) - Game will CRASH if from here return '0'	
     break; 
   case miOptions:		  // Options menu  (Will be overriden by other menu values)
	 if(!ModInfo->Config.MPauseOptions){ModInfo->EnabledAiMenuM = ModInfo->Config.MKeepAiOptions;EnableGM = (ModInfo->Config.MEnableGmOptions);goto FixMenu;}
	 break;	 
   case miResting:		  // Resting menu
	 if(!ModInfo->Config.MPauseResting){ModInfo->EnabledAiMenuM = ModInfo->Config.MKeepAiResting;EnableGM = (ModInfo->Config.MEnableGmResting);goto FixMenu;}
     break;
   case miInventory:	  // Inventory  menu	  
	 if(!ModInfo->Config.MPauseInventory){ModInfo->EnabledAiMenuM = ModInfo->Config.MKeepAiInventory;EnableGM = (ModInfo->Config.MEnableGmInventory);goto FixMenu;}   
     break;  
   case miContainer:	  // Container  menu
     if((ModInfo->Config.PickPocketMode == ppmLockActor)||(ModInfo->Config.PickPocketMode == ppmCloseMenu))
      {
       BYTE Flag = 0;
       ModInfo->TgtObjectRef = (*ModInfo->MWGlobals.ppMWRenderer)->_GetFocusObject(GameClass->GetPlayerActor()->GetReference()->_GetActiveModel(),200,0,&Flag);   	
   	   ModInfo->TgtObjectRef->GetPosition(&ModInfo->PlayerTgtPos.x, &ModInfo->PlayerTgtPos.y, &ModInfo->PlayerTgtPos.z);
   	   if((ModInfo->Config.PickPocketMode == ppmLockActor)&&(ModInfo->TgtObjectRef->GetObjAsNPC_()||ModInfo->TgtObjectRef->GetObjAsCREA()))ModInfo->CurrentTalker = ModInfo->CurrentTalker->PrepareNew(ModInfo->TgtObjectRef->_GetActorItem());  // Simulate pickpocket target actor as new dlg actor
	  }
	 if(!ModInfo->Config.MPauseContainer){ModInfo->EnabledAiMenuM = ModInfo->Config.MKeepAiContainer;EnableGM = (ModInfo->Config.MEnableGmContainer);goto FixMenu;}   
     break;               
   case miDialogue:		  // Dialogue menu 
	 if(!ModInfo->Config.MPauseDialogue)
	  {
	   EnableGM  = (ModInfo->Config.MEnableGmDialogue);
	   ModInfo->EnabledAiMenuM = ModInfo->Config.MKeepAiDialogue;
	   GameClass->ShowCrosshair(false); 
       ModInfo->SmoothFaceTrackingX = false;   // Another NPC can initiate a dialogue (TODO: Recalculate FOV in this case also)
       ModInfo->SmoothFaceTrackingY = false;
       ModInfo->CamMovMPrevX = (*ModInfo->MWGlobals.ppGameScene)->CamMovMultX;
       ModInfo->CamMovMPrevY = (*ModInfo->MWGlobals.ppGameScene)->CamMovMultY;
	   if(ModInfo->Config.DlgSwitchToFP)GameClass->SetViewMode(0);
       goto FixMenu;
	  }
     break;  
   case miJournal:		  // Journal
	 if(!ModInfo->Config.MPauseJournal){ModInfo->EnabledAiMenuM = ModInfo->Config.MKeepAiJournal;EnableGM = (ModInfo->Config.MEnableGmJournal);goto FixMenu;} 
     break;
   case miBook:		      // Book reading from world  
   case miScroll:		  // Scroll or note reading from world  
	 if(!ModInfo->Config.MPauseDocuments){ModInfo->EnabledAiMenuM = ModInfo->Config.MKeepAiDocuments;EnableGM = (ModInfo->Config.MEnableGmDocuments);goto FixMenu;}
	 break;
   case miHotKeys:        // Hotkeys menu (Temporary bounded with 'miMessageBox') !!!!!!!!!!!
   case miMessageBox:     // Messagebox 
	 if(!ModInfo->Config.MPauseMessages){ModInfo->EnabledAiMenuM = ModInfo->Config.MKeepAiMessages;EnableGM = (ModInfo->Config.MEnableGmMessages);goto FixMenu;}
	 break;
  /* case 0x              // Find service menu ID
   case 0x 
	 if(!ModInfo->Config.MPauseService){ModInfo->EnabledAiMenuM = ModInfo->Config.MKeepAiService;EnableGM = (ModInfo->Config.MEnableGmService);goto FixMenu;}  // Currently, there in no a script commands to show these menus
	 break;  */
   case miCGClass:        // Class selection mode menu - full system of menu (chargen)		  
   case miCGCharSel:      // Character select (chargen)
   case miCGSummary:      // Total preview (chargen)	  		  
   case miCGCharName:     // Name enter box (chargen)		   
   case miCGBirthSign:    // Sign select (chargen)	  
	 if(!ModInfo->Config.MPauseChargen){ModInfo->EnabledAiMenuM = ModInfo->Config.MKeepAiChargen;EnableGM = (ModInfo->Config.MEnableGmChargen);goto FixMenu;}	  
	 break; 
  }                         
 PauseAnims = ModInfo->MenuModeResult;   
 return false;   
FixMenu:
 if(EnableGM)ModInfo->LastGodMode     = GameClass->SetGodMode(true);	  
 ModInfo->MenuModeResult = PauseAnims = 0;
 ModInfo->PausedMenuMode = false;	// Set current menu mode as UNPAUSED
 return false;   
} 
//====================================================================================



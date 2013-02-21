
#pragma once

#include "TalkyFuncs.h"
#include "TalkyHooks.h"
//====================================================================================
// Redefinable
#undef  FMODHIGHPRI
#undef  MOD_VERSION
#undef  MOD_PARAMETERS
#undef  MOD_NAME
#define FMODHIGHPRI      0x80000000
#define	MOD_VERSION		 0x0001020B
#define	MOD_PARAMETERS   0
#define	MOD_NAME         "Talky Morrowind"  // Spaces allowed - used in MLDR
//---------------------------------------------------------------------------  

#define MSG_START_SPEAK  0x00000400
#define MSG_THREADEXIT   0x00000409
//---------------------------------------------------------------------------  

#ifndef MODULENAME
#define DLLMAINTYPE static
#define DEFLOGPROC Logger::LogToFile
#define MODULENAME TalkyMorrowind   // No spaces allowed
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
const LPSTR MLDRLIBNAME    = "MLDRLib.dll";
const LPSTR MLDRLOGPROC    = "WriteLogFile";
const LPSTR MLDRPROCNAME   = "IsModuleLoader";
const LPSTR CONFIGINIFILE  = "TalkyConfig.ini"; 
const LPSTR SHELLCFGNAME   = "MWTalkyCfg.exe";  // For call from Moduleloader
const LPSTR SINGERRORMSGRU = "Œ‰Ì‡ ËÁ ÒË„Ì‡ÚÛ ÌÂ ÒÓ‚Ô‡‰‡ÂÚ.\r\nÕÂ Ú‡ ‚ÂÒËˇ 'Morrowind.exe'? (ÕÛÊÌ‡: 1.6.1820).\r\nœ–ŒƒŒÀ∆≈Õ»≈ –¿¡Œ“€ Õ≈¬Œ«ÃŒ∆ÕŒ!";
const LPSTR SINGERRORMSGEN = "One of a signatures is differs.\r\nWrong version of 'Morrowind.exe'? (Need: 1.6.1820).\r\nCONTINUE IS IMPOSSIBLE!";

const float TgtObjMoveDist = 15.0;  // If target Actor will move to dist more than this, his menus must be closed  // TODO: Move to config

//---------------------------------------------------------------------------
struct  MODCONFIG;
class   CModData;
typedef CModData*  PCModData;
typedef MODCONFIG* PMODCONFIG;

enum AttackSP {asNone,asAll,asHostile,asSmart};
enum PPModes  {ppmDoNothing, ppmLockActor, ppmCloseMenu};
enum Voices   {VoiceMale, VoiceFemale, VoiceCreature, VoiceDocument};

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
void  _stdcall LoadConfiguration(LPSTR CfgFilePath);
DWORD WINAPI   WorkerThread(LPVOID lpParameter);

//------------------------------------------------------------------------------------
struct MODCONFIG   // Readed from the configuration file
{
 short VPOrc;
 short VPNord;
 short VPBreton;
 short VPKhajiit;
 short VPDarkElf;
 short VPHighElf;
 short VPWoodElf;
 short VPArgonian;
 short VPImperial;
 short VPRedguard;

 short VPBaseMale;
 short VPBaseFemale;
 short VPBaseCreature;

 bool  MPauseChargen;
 bool  MPauseService;
 bool  MPauseResting;
 bool  MPauseJournal;
 bool  MPauseDialogue;
 bool  MPauseContainer;
 bool  MPauseInventory;
 bool  MPauseDocuments;
 bool  MPauseMessages;
 bool  MPauseOptions;
 bool  MKeepAiChargen;
 bool  MKeepAiService;
 bool  MKeepAiResting;
 bool  MKeepAiJournal;
 bool  MKeepAiDialogue;
 bool  MKeepAiContainer;
 bool  MKeepAiInventory;
 bool  MKeepAiDocuments;
 bool  MKeepAiMessages;
 bool  MKeepAiOptions;
 bool  MEnableGmChargen;
 bool  MEnableGmService;
 bool  MEnableGmResting;
 bool  MEnableGmJournal;
 bool  MEnableGmDialogue;
 bool  MEnableGmContainer;
 bool  MEnableGmInventory;
 bool  MEnableGmDocuments;
 bool  MEnableGmMessages;
 bool  MEnableGmOptions;
 
 bool  AlwReadOpenJnl;   // Always read text in appeared journal,even if it is was readed before journal was closed
 bool  DocVoiceAsDlg;    // Read journal and documents by Player`s gender voice
 bool  VoiceFxByRace;
 bool  StopSpkOnClose;   // Stop speaking when menu mode closed
 bool  DlgSwitchToFP;
 bool  DlgEnableZoom;
 bool  SmoothZooming;
 bool  SpkAfterFocusNpc;
 bool  TTSBoundPitchDS;
 bool  SearchSpeechFile;
 bool  FilterSpeechFile;
 
 long  AttackSuspMode;
 long  PickPocketMode;
 long  ZoomDistNPC;
 DWORD LangIndex;     
 DWORD SpeakKeyCode;     // Speak last sended text (F1) (Four bytes, each = ONE virtual key code; 0 - Not requested)
 DWORD SilenceKeyCode;   // Stop all speaking	  (F2) (Four bytes, each = ONE virtual key code; 0 - Not requested)
 
 PMODCONFIG SetDefaults(void)
  {
   VPOrc      = -8000;
   VPNord     = -6000;
   VPBreton   = -2000;
   VPKhajiit  = 4000;
   VPDarkElf  = 0;
   VPHighElf  = 1000;
   VPWoodElf  = 8000;
   VPArgonian = -1000;
   VPImperial = -3000;
   VPRedguard = -4000;

   VPBaseMale     = 0;
   VPBaseFemale   = 0;
   VPBaseCreature = 0;

   MPauseChargen    = true;
   MPauseService    = true;
   MPauseResting    = true;
   MPauseJournal    = true;
   MPauseDialogue   = false;
   MPauseContainer  = false;
   MPauseInventory  = false;
   MPauseDocuments  = true;
   MPauseMessages   = true;
   MPauseOptions    = true;
   MKeepAiChargen   = false;
   MKeepAiService   = false;
   MKeepAiResting   = false;
   MKeepAiJournal   = false;
   MKeepAiDialogue  = false;
   MKeepAiContainer = false;
   MKeepAiInventory = false;
   MKeepAiDocuments = false;
   MKeepAiMessages  = false;
   MKeepAiOptions   = false;
   MEnableGmChargen   = true;
   MEnableGmService   = true;
   MEnableGmResting   = true;
   MEnableGmJournal   = false;
   MEnableGmDialogue  = true;
   MEnableGmContainer = false;
   MEnableGmInventory = false;
   MEnableGmDocuments = false;
   MEnableGmMessages  = true;
   MEnableGmOptions   = true;
    
   AlwReadOpenJnl   = false;  
   DocVoiceAsDlg    = true;   
   VoiceFxByRace    = true;
   StopSpkOnClose   = true;   
   DlgSwitchToFP    = true;
   DlgEnableZoom    = true;
   SmoothZooming    = true;
   SpkAfterFocusNpc = false; 
   TTSBoundPitchDS  = true;
   SearchSpeechFile = true;
   FilterSpeechFile = false;
 
   LangIndex      = 1;
   ZoomDistNPC	  = 60;
   AttackSuspMode = asAll;
   PickPocketMode = ppmLockActor;
   SpeakKeyCode   = VK_F3;    // Speak last sended text (F1) (Four bytes, each = ONE virtual key code; 0 - Not requested)
   SilenceKeyCode = VK_F4;	  // Stop all speaking	    (F2) (Four bytes, each = ONE virtual key code; 0 - Not requested)
   return this;
  }
};
//---------------------------------------------------------------------------
struct SModData
{
 MODCONFIG          Config;
 MORROWIND::GLOBALS MWGlobals;
       
 float FltCamBIG;        // Taked from Morrowind.exe [9.9999997e-6]
 float FltCamSMALL;      // Taked from Morrowind.exe [1.0]
 float CamRotDeltaX;    
 float CamRotDeltaY;  
 float CamMovMPrevX;
 float CamMovMPrevY;
 float LastFOV;        
 float CurrentFOV;     
 float StartingFOV; 
 bool  ModuleLoader;     // True, if module loaded into ModuleLoader`s space
 bool  LastCamCtrl;   
 bool  LastKeyCtrl; 
 bool  LastGodMode; 
 bool  PrevPViewMode;    // First\Third person view
 bool  GameInMenuMode;   // For title screen menu state initial=TRUE
 bool  PausedMenuMode;  
 bool  EnabledAiMenuM;
 bool  PCSneakBefore;   
 bool  ZoomingInProces; 
 bool  CamTrackingActive;
 bool  SmoothFaceTrackingX;	 
 bool  SmoothFaceTrackingY;	 

 BYTE       MenuModeResult;   // For initial menu mode=1
 WORD       LastMenuID;
 DWORD      LastJnlCS;    
 DWORD      WorkerThreadID; 
 HANDLE     hWorkerThread;   
 HANDLE     hDefHeap;       
 HMODULE    ThisLibBase;     
 HMODULE    ExeModuleBase;  
 PCTalker   CurrentTalker;      
 VECTOR3    PlayerTgtPos;

 MORROWIND::PNiNode     pTrkTarget;  
 MORROWIND::PNiCamera   pTrkCamera;
 MORROWIND::PCObjREFR   TgtObjectRef; 

 BYTE        CurrentDir[MAX_PATH];
 BYTE        CfgFilePath[MAX_PATH];
 BYTE        GameCurDir[MAX_PATH];
 BYTE        SpeechDirectory[MAX_PATH];

 QWORD       SinCosTable[360];
 VOICEINFO   VoiceInf[4];              // Male, Female, Creature, Document 
 CODEHOOK32  CodeHooks[40];   // Max hooks
 //---------------------------------

 void* operator new(size_t size){return MEMWORK::GetMemoryBlock(sizeof(SModData));}   // Sets memory to ZERO
 void  operator delete(void* mem){MEMWORK::FreeMemoryBlock(mem);}    
//---------------------------------

 SModData(void)     
  {
   Config.SetDefaults();            //  construct !!!!!!!!!!!!!!
   MWGlobals.Initialize();
   FillSinCosTable((QWORD*)SinCosTable);
   for(int ctr=0;ctr<(sizeof(VoiceInf)/sizeof(VOICEINFO));ctr++)VoiceInf[ctr].VoiceEnabled = VoiceInf[ctr].AutoSpeak = true;
        
   FltCamBIG           = 9.9999997e-6;  
   FltCamSMALL         = 1.0;           
   CamRotDeltaX        = 0;
   CamRotDeltaY        = 0;
   CamMovMPrevX        = -1;
   CamMovMPrevX        = -1;
   LastFOV             = -1;
   CurrentFOV          = -1;
   StartingFOV         = -1;
   ModuleLoader        = true;   
   LastCamCtrl         = false;
   LastKeyCtrl         = false;
   LastGodMode         = false;
   PrevPViewMode       = false;   
   GameInMenuMode      = false;   
   PausedMenuMode      = true;	// Game may crash on title screen if this is not TRUE;
   EnabledAiMenuM      = false;
   PCSneakBefore       = false;
   ZoomingInProces     = false;
   CamTrackingActive   = false;
   SmoothFaceTrackingX = false;
   SmoothFaceTrackingY = false;
   MenuModeResult      = 2;
   LastMenuID		   = 0;	     
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


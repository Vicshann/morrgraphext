
#include "MWDerived.h"
#include "ExtensionMod.h"

// TODO: Share this unit between all modules
using namespace MODULENAME;

extern MODULENAME::PSModData  ModInfo; 

namespace MWDerived
{
//====================================================================================
void MWTHISCALL CObjMACH::__AIDoActorAttack(void)
{
 // Also exclude from attacks current DLG actor, and allow others to fight back
 if(ModInfo->GameInMenuMode)
  {
   MORROWIND::PCObjREFR CombatObjRef = (this->CombatTarget)?(this->CombatTarget->GetReference()):((MORROWIND::PCObjREFR)-1);
   MORROWIND::PCObjREFR UnknowObjRef = (this->UnknowTarget)?(this->UnknowTarget->GetReference()):((MORROWIND::PCObjREFR)-1);
   MORROWIND::PCObjREFR PlayerObjRef = (*ModInfo->MWGlobals.ppGameScene)->GetPlayerActor()->GetReference();
   switch(ModInfo->Config.AttackSuspMode)
    {
	 case asSmart:
	   if((CombatObjRef == PlayerObjRef)||(UnknowObjRef == PlayerObjRef)||(CombatObjRef == ModInfo->TgtObjectRef)||(UnknowObjRef == ModInfo->TgtObjectRef)){this->AIState = -1;return;}  // Exclude player and DLG target (TODO: Companion)
	   if(this->CombatTarget && this->CombatTarget->CombatTarget && ((this->CombatTarget->CombatTarget->GetReference() == PlayerObjRef)||(this->CombatTarget->CombatTarget->GetReference() == ModInfo->TgtObjectRef)))this->CombatTarget->CombatTarget = this;	// Allow hostile NPC defend themselves against PC allies and monsters
	   if(this->CombatTarget && this->CombatTarget->UnknowTarget && ((this->CombatTarget->UnknowTarget->GetReference() == PlayerObjRef)||(this->CombatTarget->UnknowTarget->GetReference() == ModInfo->TgtObjectRef)))this->CombatTarget->UnknowTarget = this;	// Allow hostile NPC defend themselves against PC allies and monsters
	   if(this->UnknowTarget && this->UnknowTarget->CombatTarget && ((this->UnknowTarget->CombatTarget->GetReference() == PlayerObjRef)||(this->UnknowTarget->CombatTarget->GetReference() == ModInfo->TgtObjectRef)))this->UnknowTarget->CombatTarget = this;	// Allow hostile NPC defend themselves against PC allies and monsters
	   if(this->UnknowTarget && this->UnknowTarget->UnknowTarget && ((this->UnknowTarget->UnknowTarget->GetReference() == PlayerObjRef)||(this->UnknowTarget->UnknowTarget->GetReference() == ModInfo->TgtObjectRef)))this->UnknowTarget->UnknowTarget = this;	// Allow hostile NPC defend themselves against PC allies and monsters
	   break;
	 case asHostile:
	  if((CombatObjRef != PlayerObjRef)&&(UnknowObjRef != PlayerObjRef)&&(CombatObjRef != ModInfo->TgtObjectRef)&&(UnknowObjRef != ModInfo->TgtObjectRef))break;
	 case asAll:	  // The best solution
	   this->AIState = -1;
	   return;
	}
  }
 this->_AIDoActorAttack();
}  
//====================================================================================
void MWTHISCALL CObjMACP::__GoToJailChecking(void)
{
 if(!ModInfo->GameInMenuMode)this->_GoToJailChecking();
}  
//====================================================================================
void MWTHISCALL CRenderer::__ProcessPlayerUseAction(void)
{                          
 if(!ModInfo->GameInMenuMode)this->_ProcessPlayerUseAction();
}
//====================================================================================
PVOID MWTHISCALL CTESFont::__PrepareTextDlgJnl(PVOID Class_20,MORROWIND::PCObjMACT Actor,DWORD Zero_00,DWORD Zero_01,DWORD Zero_02,LPSTR TopicText,DWORD Zero_03,PVOID Unknow00,PVOID Unknow01,DWORD UnkIndex)
{
 ModInfo->CurrentTalker->UpdateTextLine(TopicText);   // Don`t used for dialogue - duplicated!
 return this->_PrepareTextFmt(Class_20,Actor,Zero_00,Zero_01,Zero_02,TopicText,Zero_03,Unknow00,Unknow01,UnkIndex);
}
//====================================================================================
MORROWIND::PCWindow MWTHISCALL CWindow::__DrawTextLine(short TgtWndID, LPSTR TextLine, BOOL UnkFlag00, BOOL UnkFlag01)
{
 ModInfo->CurrentTalker->UpdateTextLine(TextLine);
 return this->_DrawTextLine(TgtWndID, TextLine, UnkFlag00, UnkFlag01);
}
//====================================================================================
void MWTHISCALL CAnimation::__HeadTrack(MORROWIND::PCObjREFR Tracker, MORROWIND::PCObjREFR Target)
{
 if(ModInfo->GameInMenuMode&&ModInfo->CurrentTalker->IsValid()&&(ModInfo->CurrentTalker->GetActor()->GetReference() == Tracker))Target = (*ModInfo->MWGlobals.ppGameScene)->GetPlayerActor()->GetReference();
 this->_HeadTrack(Tracker,Target);
}
//====================================================================================
void MWTHISCALL CAnimation::__ChangeAnimGroup(BYTE AnimGroup, long AnimType, long Unkn00, long Unkn01)
{
 MORROWIND::PCObjREFR ObjectRef = ((MORROWIND::PCObjREFR*)(((char*)_ADDRESSOF(Unkn01))+_INTSIZEOF(Unkn01)))[0]; 
 this->_ChangeAnimGroup(AnimGroup, AnimType, Unkn00, Unkn01);
}        
//====================================================================================
void MWTHISCALL CCamera::__SwitchViewMode(void)
{ 
 float LastFOV = (*ModInfo->MWGlobals.ppGameScene)->GetCurrentFOV(); 
 this->_SwitchViewMode();
 if(LastFOV != (*ModInfo->MWGlobals.ppGameScene)->GetCurrentFOV())(*ModInfo->MWGlobals.ppGameScene)->SetCurrentFOV(LastFOV);
}
//====================================================================================
void MWTHISCALL CCamera::__UpdatePlayerPositions(DWORD Unkn)
{
 if(ModInfo->GameInMenuMode)
  {
   ModInfo->LastCamCtrl = (*ModInfo->MWGlobals.ppGameScene)->GetPlayerActor()->EnableLooking(false);
   ModInfo->LastKeyCtrl = (*ModInfo->MWGlobals.ppGameScene)->GetPlayerActor()->EnableMovement(false);
   this->_UpdatePlayerPositions(Unkn);
   (*ModInfo->MWGlobals.ppGameScene)->GetPlayerActor()->EnableLooking(ModInfo->LastCamCtrl);
   (*ModInfo->MWGlobals.ppGameScene)->GetPlayerActor()->EnableMovement(ModInfo->LastKeyCtrl);  
  }
   else this->_UpdatePlayerPositions(Unkn);
}
//====================================================================================
void MWTHISCALL CSoundBase::__SetDSBufFrequency(MORROWIND::PCSndInfo Sound, float FreqMult) 
{
 if(!ModInfo->CurrentTalker->IsSameDSBuf(Sound->pDSBuffer))this->_SetDSBufFrequency(Sound, FreqMult);   // Don` call this for TTS speech 
   else if(!ModInfo->CurrentTalker->IsRealVoice() || ModInfo->Config.FilterSpeechFile)ModInfo->CurrentTalker->SetVoicePitch();    // Set pitch for current voice
}   
//====================================================================================
MORROWIND::PCSndInfo MWTHISCALL CSoundBase::__OpenVoiceFile(LPSTR VoiceFile, DWORD Flag) 
{
 short      CurSample;
 short      MinSample;
 short      MaxSample;
 DWORD      Result;
 DWORD      WavDataSize;
 DWORD      SBufSize;
 PVOID      SBufPtr;
 PWORD      WavData;
 PCTalker   Talker;
 MORROWIND::PCSndInfo SndInfoBlock; 
  
 if(((PDWORD)&VoiceFile[17])[0] != TLKYFLAG)return this->_OpenVoiceFile(VoiceFile, Flag);
 if(this->SoundDisabled)return NULL;      // Sound disabled ?
 Talker = (PCTalker)HexStrToDW((LPSTR)&((PDWORD)&VoiceFile[17])[1]);    // "data files\\sound\\" - constant path prefix
 Talker->Silence();   // No overlapping voice from same NPC (Watch for AV here)
 if(Talker->GetWavSpeechBuffer((PVOID*)&WavData, &WavDataSize))return NULL;

 SndInfoBlock = (MORROWIND::PCSndInfo)ModInfo->MWGlobals.MWGetMem(sizeof(MORROWIND::CSndInfo));               
 FastZeroMemory(SndInfoBlock, sizeof(MORROWIND::CSndInfo));
 SndInfoBlock->pLipSBlock = (PWORD)ModInfo->MWGlobals.MWGetMem(WavDataSize);    // Was '((WavDataSize/2304)+1)' but it causes some internal AccessViolation;
 Talker->FillWavFmtStruct(&SndInfoBlock->WaveFmtx);

 // Create Lipsync to 'LipSBlock' ONE SAMPLE (2 bytes) from each chunk (2304 bytes, 1152 samples)
 for(DWORD cctr=0,smpl=0;cctr < (WavDataSize/2304);cctr++)       
  {
   MinSample = MaxSample = 0;
   for(DWORD sctr=1152;sctr > 0;sctr--,smpl++)
	{
	 CurSample = WavData[smpl];
	 if(CurSample < MinSample)MinSample = CurSample; 
       else {if(CurSample > MaxSample)MaxSample = CurSample;}
	}
   if(MaxSample > -MinSample)MinSample = MaxSample;
   SndInfoBlock->pLipSBlock[cctr] = MinSample;  
  }

 if(this->UnkByte01 > 1)SndInfoBlock->DsBufDesc.dwFlags = (DSBCAPS_GETCURRENTPOSITION2 | DSBCAPS_MUTE3DATMAXDISTANCE | DSBCAPS_CTRL3D | DSBCAPS_CTRLFREQUENCY | DSBCAPS_CTRLVOLUME);  // 0x000300B0; 
   else SndInfoBlock->DsBufDesc.dwFlags = (DSBCAPS_CTRLFREQUENCY | DSBCAPS_CTRLPAN | DSBCAPS_CTRLVOLUME);  // 0x000000E0; 
 if(this->UnkByte00 & 0x04)SndInfoBlock->DsBufDesc.dwFlags |= DSBCAPS_LOCDEFER;  // 0x00040000;

 SndInfoBlock->DsBufDesc.dwSize        = sizeof(DSBUFFERDESC);
 SndInfoBlock->DsBufDesc.lpwfxFormat   = &SndInfoBlock->WaveFmtx;   
 SndInfoBlock->DsBufDesc.dwBufferBytes = (WavDataSize > DSBSIZE_MAX)?(DSBSIZE_MAX):((WavDataSize < DSBSIZE_MIN)?DSBSIZE_MIN:WavDataSize);

 Result = this->pDSnd8->CreateSoundBuffer(&SndInfoBlock->DsBufDesc, &SndInfoBlock->pDSBuffer, 0);
 if(Result)LogMsg("Creating DirectSound buffer FAILED!");

 Result = SndInfoBlock->pDSBuffer->Lock(0, 0, &SBufPtr, &SBufSize, 0, 0, 2);
 if(!Result && SBufPtr)FastMoveMemory(SBufPtr, WavData, SBufSize);
 Result = SndInfoBlock->pDSBuffer->Unlock(SBufPtr, SBufSize, 0, 0);

 if(this->UnkByte01 > 1)Result = SndInfoBlock->pDSBuffer->QueryInterface(IID_IDirectSound3DBuffer8, (LPVOID*)&SndInfoBlock->pDS3DBuffer);
 Result = SndInfoBlock->pDSBuffer->SetVolume(0); 
 Talker->SetDSBuffer(SndInfoBlock->pDSBuffer);  
 return SndInfoBlock; 
}
//====================================================================================
}
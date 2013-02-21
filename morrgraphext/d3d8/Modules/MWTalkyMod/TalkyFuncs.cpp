
#include "ExtensionMod.h"
#include "TalkyFuncs.h"

using namespace MODULENAME;

extern MODULENAME::PSModData  ModInfo;
//====================================================================================
//                                 SPEAKING FUNCTIONS
//------------------------------------------------------------------------------------
PCTalker CTalker::PrepareNew(MORROWIND::PCObjMACT Actor)
{
 PCTalker Empty     = this;
 PCTalker NewTalker = NULL;
 
 while(Empty)             
  {
   if(!IsValidDSBuffer(Empty->DSBuffer))break;  // Place in already allocated memory (unused item)
   Empty = Empty->Prev;
  }
 if(Empty)
  {
   NewTalker = Empty; 
   NewTalker->VPlayer = NewTalker->RealVoice = false;
   WChannels = WSamplesPerSec = WBitsPerSample = TxtCrcBase = TxtCrcFinal = SpBufSize = 0;
   FastZeroMemory(&NewTalker->CurVoice,sizeof(VOICEINFO));
  }
   else 
    {
     Empty     = this;
     NewTalker = (PCTalker)MEMWORK::GetHeapMemory(sizeof(CTalker));
     while(Empty && Empty->Next)Empty = Empty->Next;   
     if(Empty)Empty->Next = NewTalker;
     NewTalker->Prev = Empty;
    }
 NewTalker->Actor = Actor;   
 return NewTalker;
}
//------------------------------------------------------------------------------------
DWORD CTalker::SilenceActor(MORROWIND::PCObjMACT Actor)
{
 if(!this)return -1;
 PCTalker Talker = this;
 while(Talker->Prev)Talker = Talker->Prev;  // Go to First item
 while(Talker)
  {
   if(Talker->Actor == Actor)
    {
	 if(IsValidDSBuffer(Talker->DSBuffer))Talker->DSBuffer->Stop();		   
	 Talker->DSBuffer = NULL;  
	 return 0;
	}
   Talker = Talker->Next;
  }   
 return 2;
}  
//------------------------------------------------------------------------------------
void CTalker::SilenceAllTalkers(void)
{
 if(!this)return;
 PCTalker Talker = this;
 while(Talker->Prev)Talker = Talker->Prev;  // Go to First item
 while(Talker)  
  {
   if(IsValidDSBuffer(Talker->DSBuffer))Talker->DSBuffer->Stop();   
   Talker->DSBuffer = NULL;
   Talker->Actor    = NULL; 
   Talker = Talker->Next; 
  }
}    
//------------------------------------------------------------------------------------
void CTalker::UpdateTextLine(LPSTR Text)
{
 if(!this)return;
 DWORD FullSize = lstrlen(this->TextLine);
 DWORD LineSize = lstrlen(Text);
 if(this->TextLine)this->TextLine = (LPSTR)MEMWORK::ReAllocHeapMem(this->TextLine,(FullSize+LineSize+32)); 
   else this->TextLine = (LPSTR)MEMWORK::GetHeapMemory(FullSize+LineSize+32);
 lstrcat((LPSTR)&this->TextLine[FullSize], Text);        
 FullSize += LineSize;
 if((Text[LineSize+3] == '>')||(Text[LineSize+1] == 0)){this->TextLine[FullSize++]='.';this->TextLine[FullSize++]=0x20;this->TextLine[FullSize]=0;}  // Detect a New Paragraph '<BR>'
   else {this->TextLine[FullSize++]=0x20;this->TextLine[FullSize]=0;}  	// Add space after a new line (Game removes spaces in text on end of line)	    
}
//------------------------------------------------------------------------------------
// Remove spaces and replacers from text, calculate checksum
void CTalker::UpdateRawTextLine(LPSTR Text)
{
 if(!this)return;
 DWORD FullSize = lstrlen(this->RawTextLine);
 DWORD LineSize = lstrlen(Text);
 if(this->RawTextLine)this->RawTextLine = (LPSTR)MEMWORK::ReAllocHeapMem(this->RawTextLine,(FullSize+LineSize+32));
   else this->RawTextLine = (LPSTR)MEMWORK::GetHeapMemory(FullSize+LineSize+32);
 lstrcpy((LPSTR)&this->RawTextLine[FullSize], Text); 
 this->TxtCrcBase = MakeRawTextAndCRC(this->RawTextLine);
}
//------------------------------------------------------------------------------------
void CTalker::InitDialogueText(void)
{
 if(!this || !this->Actor || !this->TextLine[0])return;
 if(MORROWIND::PCObjNPC_ Npc = this->Actor->GetObjectAsNPC())
  {
   if(Npc->Base->IsFemale()){this->SetVoice(&ModInfo->VoiceInf[VoiceFemale]);LogMsg("Detected a Female NPC: %s, [ %s ]",Npc->Base->GetName(),Npc->Base->GetObjectID());}
     else {this->SetVoice(&ModInfo->VoiceInf[VoiceMale]);LogMsg("Detected a Male NPC: %s, [ %s ]",Npc->Base->GetName(),Npc->Base->GetObjectID());} 
  }
   else {this->SetVoice(&ModInfo->VoiceInf[VoiceCreature]);LogMsg("Detected a Creature: %s, [ %s ]",this->Actor->GetActorBaseName(), this->Actor->GetActorBaseID());} 
 
 if((this->VPlayer = ((*ModInfo->MWGlobals.ppGameScene)->GetPlayerActor() == this->Actor)))   // Player talks to himself in journal or reads a document
  {
   LogMsg("Preparing document text.");
   if(ModInfo->Config.DocVoiceAsDlg)SetVoiceFlags(ModInfo->VoiceInf[VoiceDocument].VoiceEnabled,ModInfo->VoiceInf[VoiceDocument].AutoSpeak);
     else {this->SetVoice(&ModInfo->VoiceInf[VoiceDocument]);LogMsg("Used DOCUMENT voice.");}  
  }
   else LogMsg("Preparing a dialogue text.");   // Player talks to some NPC in dialogue  
   this->SendTextToSpeaking();      
}
//------------------------------------------------------------------------------------
void CTalker::SendTextToSpeaking(void)
{
 if(this->PrepareSpeechText())PostThreadMessage(ModInfo->WorkerThreadID, MSG_START_SPEAK, reinterpret_cast<DWORD>(this), NULL); 
 this->ClearTextLines();
}
//------------------------------------------------------------------------------------
bool CTalker::PrepareSpeechText(void)
{
 if(!this)return false;
 DWORD TextLen     = lstrlen(this->TextLine);
 if(this->TextToSpeak)MEMWORK::FreeHeapMemory(this->TextToSpeak);
 this->TextToSpeak = (LPSTR)MEMWORK::GetHeapMemory(TextLen+8);
 NormalizeTextString(this->TextToSpeak, this->TextLine);
 DWORD CurChSum    = ComputeCRC32(this->TextToSpeak, lstrlen(this->TextToSpeak));   // No repeats 
 bool  Result      = (CurChSum != this->TxtCrcFinal);
 this->TxtCrcFinal = CurChSum;
 //
 // TODO: Make lexical analysis and dictionary search here.
 //
 return Result;
}
//------------------------------------------------------------------------------------
DWORD CTalker::GetWavSpeechBuffer(PVOID *Buffer, DWORD *BufSize)
{
 if(!this || !this->SpBuffer || !this->SpBufSize)return 1;
 if(Buffer)(*Buffer)   = this->SpBuffer;
 if(BufSize)(*BufSize) = this->SpBufSize;
 return 0;
}
//------------------------------------------------------------------------------------
// For now - changes only SPEED of speech
DWORD CTalker::SetVoicePitch(void)
{
 DWORD CurFreq   = 0;
 long  FreqShift = 0;
 long  RacePitch;

 if(!this || !this->DSBuffer || !this->Actor)return 1;
 this->DSBuffer->GetFrequency(&CurFreq);			 
 
 if(MORROWIND::PCObjNPC_ Npc = this->GetActor()->GetObjectAsNPC())
  {
   if(Npc->Base->IsFemale())FreqShift = ModInfo->Config.VPBaseFemale;
     else FreqShift = ModInfo->Config.VPBaseMale;
   if(ModInfo->Config.VoiceFxByRace)
    {
     RacePitch  = GetRaceVoicePitch(Npc);
     if(ModInfo->Config.TTSBoundPitchDS)ChangeNumberInXml(GetPitchTTSRelativeToDS(RacePitch), "PITCH ABSMIDDLE", (LPSTR)&this->CurVoice.VoiceConfig);
     FreqShift += RacePitch;
    }
  }
   else FreqShift = ModInfo->Config.VPBaseCreature; 
 CurFreq = ((int)CurFreq) + FreqShift;   // + or - results
 if(CurFreq < 0x00000064)CurFreq = 0x00000064;
   else {if(CurFreq > 0x000186A0)CurFreq = 0x000186A0;}   
 this->DSBuffer->SetFrequency(CurFreq);
 return 0;
}
//------------------------------------------------------------------------------------
DWORD CTalker::SpeakToBuffer(void)
{ 
 PVOID TmpSpBuf    = NULL;
 UINT  TmpSpBufLen = 0; 

 if(!this || !this->TextToSpeak || !this->TextToSpeak[0] || !this->CurVoice.VoiceEnabled || !this->Actor)return 3;
 LogMsg("Trying to speak: %s",this->TextToSpeak);
  
 if(this->SpBuffer)MEMWORK::FreeMemoryBlock(this->SpBuffer);
 this->SpBuffer  = NULL;
 this->SpBufSize = 0;
 this->RealVoice = false; 
 
 if(ModInfo->Config.SearchSpeechFile && this->TxtCrcBase)  
  {
   PBYTE FilePath[MAX_PATH];    
   if(!FindActorSpeechFile((LPSTR)&ModInfo->SpeechDirectory, (LPSTR)&FilePath, this->Actor->GetActorRaceID(), this->Actor->GetActorBaseID(), this->Actor->IsActorFemale(), this->Actor->IsActorCreature(), this->TxtCrcBase))
    {
     MORROWIND::PCMP3File  Mp3File  = new MORROWIND::CMP3File((LPSTR)&FilePath);   // Causes AV if the file the path is wrong!
     MORROWIND::PCMP3Chunk Mp3Chunk = new MORROWIND::CMP3Chunk(Mp3File);  
     int   WavSize   = (Mp3File->ChunksCount*MP3WAVCHUNKSIZE);
     PBYTE WavData   = (PBYTE)MEMWORK::GetMemoryBlock(WavSize);
     PWORD WavChunk  = NULL;  
     for(DWORD Offset=0;(WavChunk=Mp3Chunk->_DecodeChunk());Offset+=MP3WAVCHUNKSIZE)FastCopyMemory(&WavData[Offset],WavChunk,MP3WAVCHUNKSIZE);
     this->WChannels      = 1;    // TODO: Get this from 'Mp3Chunk' !
     this->WSamplesPerSec = Mp3Chunk->pMP3File->pMP3Info->SamplPerSec;
     this->WBitsPerSample = 16;   // TODO: Get this from 'Mp3Chunk' !
     this->RealVoice = WavSize;
     this->SpBuffer  = WavData;
     this->SpBufSize = WavSize;
     delete(Mp3Chunk);
     delete(Mp3File);
    }                                                                          
  } 
 if(!this->SpBuffer && this->TxtCrcFinal) 
  {
   if(this->CurVoice.Voice.SpeakText(this->TextToSpeak) || this->CurVoice.Voice.GetSpeechBuffer(&TmpSpBuf, &TmpSpBufLen))return 4;
   if(TmpSpBuf && TmpSpBufLen)
    {
     this->CurVoice.Voice.GetOutputFormat(&this->WSamplesPerSec, &this->WBitsPerSample, &this->WChannels);
     this->SpBuffer  = MEMWORK::GetMemoryBlock(TmpSpBufLen);
     FastCopyMemory(this->SpBuffer, TmpSpBuf, TmpSpBufLen);
     this->SpBufSize = TmpSpBufLen;   
    }
  } 
 if(!this->SpBuffer){LogMsg("Failed to get speech WAV data for Actor '%s'!",this->Actor->GetActorID());return 5;}
 if(this->CurVoice.AutoSpeak || this->RealVoice)return this->SendWavDataToObject();
 return 0;
}
//------------------------------------------------------------------------------------
DWORD CTalker::SendWavDataToObject(void)
{
 if(!this || !this->SpBuffer || !this->SpBufSize || !this->CurVoice.VoiceEnabled || !this->Actor)return 1;
 if(ForceActorSpeaking(this)){LogMsg("Failed to add speech to Actor '%s'!",this->Actor->GetActorID());return 2;}
   else LogMsg("Speech data sended to Actor '%s'.",this->Actor->GetActorID());  // ERROR: Once 'this' was ZERO - HOW IT IS POSSIBLE ?!
 return 0;
}
//------------------------------------------------------------------------------------
DWORD  CTalker::FillWavFmtStruct(WAVEFORMATEX *WaveFmtx)
{
 if(!this || !WaveFmtx)return 1;
 if(!this->SpBuffer  || !this->SpBufSize)return 2;
 if(!this->WChannels || !this->WSamplesPerSec || !this->WBitsPerSample)return 3;

 WaveFmtx->wFormatTag      = WAVE_FORMAT_PCM;
 WaveFmtx->nChannels       = this->WChannels;
 WaveFmtx->nSamplesPerSec  = this->WSamplesPerSec;
 WaveFmtx->wBitsPerSample  = this->WBitsPerSample;
 WaveFmtx->nBlockAlign     = (WORD)((WaveFmtx->wBitsPerSample/8)*WaveFmtx->nChannels);
 WaveFmtx->nAvgBytesPerSec = (DWORD)(WaveFmtx->nSamplesPerSec*WaveFmtx->nBlockAlign);
 WaveFmtx->cbSize          = 0;
 return 0;
} 
//====================================================================================
void _stdcall NormalizeTextString(LPSTR DstTextStr, LPSTR SrcTextStr) 
{ 
 bool SecSpace  = false;
 bool SkipChars = false;
 while(SrcTextStr[0])
  {
   if(((BYTE)SrcTextStr[0]) == '>'){SkipChars = false;SrcTextStr++;continue;}
   if(((BYTE)SrcTextStr[0]) == '<'){SkipChars = true;SrcTextStr++;continue;}
   if(SkipChars){SrcTextStr++;continue;}
   if((((BYTE)SrcTextStr[0] == '@')||((BYTE)SrcTextStr[0] == '#')||((BYTE)SrcTextStr[0] == 0x7F))&&SrcTextStr[0]){SrcTextStr++;continue;}    
   if((BYTE)SrcTextStr[0] == 0x22){DstTextStr[0]='.';DstTextStr[1]=0x22;DstTextStr+=2;SrcTextStr++;continue;}   // Delay before quotes
   if((BYTE)SrcTextStr[0] <= 0x20){if(SecSpace){SrcTextStr++;continue;}DstTextStr[0] = 0x20;SecSpace=true;}  // Replace all special symbols with space  
     else {DstTextStr[0] = SrcTextStr[0];SecSpace=false;} 
   SrcTextStr++;  
   DstTextStr++;
  } 
 DstTextStr[0] = 0;
}
//------------------------------------------------------------------------------------
DWORD _stdcall MakeRawTextAndCRC(LPSTR TextString)
{
 bool  SkipChars  = false;
 LPSTR SrcPointer = TextString;
 LPSTR DstPointer = TextString;
 while(SrcPointer[0])
  {
   if(((BYTE)SrcPointer[0]) == '>'){SkipChars = false;SrcPointer++;continue;}
   if(((BYTE)SrcPointer[0]) == '<'){SkipChars = true;SrcPointer++;continue;}
   if(SkipChars){SrcPointer++;continue;}
   if((((BYTE)SrcPointer[0]) < 0x21)||(((BYTE)SrcPointer[0]) == 0x7F)||(((BYTE)SrcPointer[0]) == '#')||(((BYTE)SrcPointer[0]) == '@')){SrcPointer++;continue;}   // Remove spaces and special symbols
   if(SrcPointer[0] == '%')
    {                                                                    
     SrcPointer++;
     for(int ctr=0;ctr<(sizeof(DlgReplacers)/sizeof(LPSTR));ctr++)
      {
       int SLen = lstrlen(DlgReplacers[ctr]);
       if(CompareString(LOCALE_USER_DEFAULT,NORM_IGNORECASE,SrcPointer,SLen,DlgReplacers[ctr],SLen)==CSTR_EQUAL){SrcPointer += SLen;break;}
      }   
     continue;
    }
   DstPointer[0] = SrcPointer[0];
   DstPointer++;
   SrcPointer++; 
  }
 DstPointer[0] = 0; 
 return ComputeCRC32(TextString, lstrlen(TextString));  
}
//------------------------------------------------------------------------------------
DWORD _stdcall ForceActorSpeaking(PCTalker Talker)  
{
 MORROWIND::PCSoundRef SndRef;
 DWORD BaseVolume;
 DWORD SndFilePath[4];  // 1-TLKYFLAG, 2-3 For Address of 'Talker', 4-Ending zero

 if(!Talker || !Talker->IsValid() || !ModInfo->GameInMenuMode)return 1;
 SndFilePath[3] = 0;
 SndFilePath[0] = TLKYFLAG;
 ConvertToHexDW(reinterpret_cast<DWORD>(Talker), 8, (LPSTR)&SndFilePath[1], true);
 while(ModInfo->Config.SpkAfterFocusNpc && ModInfo->ZoomingInProces)Sleep(ZOOMSPEAKDEL);         // Finish zooming and then speak
 BaseVolume = (*ModInfo->MWGlobals.ppGameScene)->pDSndBase->_GetBaseVolume(false);   // Get BASE_VOLUME of voice (0x00 - 0xFF)  - Voice volume setting in game menu 
 if(ModInfo->CurrentFOV > 0)BaseVolume += FPUDiv32((ModInfo->LastFOV - ModInfo->CurrentFOV), ZOOMVOLDIV);  
 if(BaseVolume > BASEVOLUMEMAX)BaseVolume = BASEVOLUMEMAX; 
 
 //TODO: Need to set an exception handler, for safety. (Stop playing or wait until it`s finished ?)  // Works only HERE!!!
 for(int ctr=0;(SndRef=(*ModInfo->MWGlobals.ppMWGame)->FindSoundForObject(Talker->GetActor()->GetReference(), &ctr));){if(!IsBadReadPtr(SndRef,sizeof(MORROWIND::CSoundRef))&&!IsBadReadPtr(SndRef->Sound,sizeof(MORROWIND::CSndInfo))&&IsValidDSBuffer(SndRef->Sound->pDSBuffer))SndRef->Sound->pDSBuffer->Stop();}
 (*ModInfo->MWGlobals.ppMWGame)->_AddSoundToObject((LPSTR)&SndFilePath,Talker->GetActor()->GetReference(),0,BaseVolume,1,1,0);   // Find subtitles string in params
 return 0;
}
//------------------------------------------------------------------------------------
// DO NOT CHANGE THIS FUNCTION - FILE SEARCH MUST BE STANDARTIZED !
DWORD _stdcall FindActorSpeechFile(LPSTR RootDir, LPSTR PathOut, LPSTR RaceID, LPSTR ActorID, bool Female, bool Creature, DWORD TextCRC)
{
 BYTE  AType[32];
 BYTE  ARace[32];
 BYTE  AGend[32];
 BYTE  AIden[32];
 BYTE  RootPath[MAX_PATH];
 BYTE  FullPath[MAX_PATH]; 
 
 lstrcpy((LPSTR)&RootPath, RootDir);
 int Length = lstrlen((LPSTR)&RootPath);     
 if((RootPath[Length-1]!=PATHDLML)&&(RootPath[Length-1]!=PATHDLMR)){RootPath[Length-1]=PATHDLML;Length++;} 

 AType[0]=ARace[0]=AGend[0]=AIden[0]=0;
 if(ActorID && ActorID[0]){lstrcpy((LPSTR)&AIden, ActorID);lstrcat((LPSTR)&AIden,"\\");} 
 if(RaceID  && RaceID[0]){lstrcpy((LPSTR)&ARace,"__");lstrcat((LPSTR)&ARace,RaceID);lstrcat((LPSTR)&ARace,"__\\");}        
 if(Creature)lstrcpy((LPSTR)&AType,"__CREATURE__\\");
   else 
    {
     lstrcpy((LPSTR)&AType,"__NPC__\\");
     if(Female)lstrcpy((LPSTR)&AGend,"__FEMALE__\\");
       else lstrcpy((LPSTR)&AGend,"__MALE__\\");     
    }
       
 if(!Creature)   
  {
   wsprintf((LPSTR)&FullPath,"%s%s%s%s%s%08X.*",&RootPath,&AType,&ARace,&AGend,&AIden,TextCRC);  // Search in 'Speech|TYPE|RACE|GENDER|ID'    
   if(FindFile((LPSTR)&FullPath, PathOut))return 0;   
   wsprintf((LPSTR)&FullPath,"%s%s%s%s%08X.*",&RootPath,&AType,&AGend,&AIden,TextCRC);  // Search in 'Speech|TYPE|GENDER|ID'     
   if(FindFile((LPSTR)&FullPath, PathOut))return 0;   
   wsprintf((LPSTR)&FullPath,"%s%s%s%08X.*",&RootPath,&AGend,&AIden,TextCRC);  // Search in 'Speech|GENDER|ID'    
   if(FindFile((LPSTR)&FullPath, PathOut))return 0;   
  }  
       
 wsprintf((LPSTR)&FullPath,"%s%s%s%s%08X.*",&RootPath,&AType,&ARace,&AIden,TextCRC);  // Search in 'Speech|TYPE|RACE|ID'    
 if(FindFile((LPSTR)&FullPath, PathOut))return 0;   
 wsprintf((LPSTR)&FullPath,"%s%s%s%08X.*",&RootPath,&AType,&AIden,TextCRC);  // Search in 'Speech|TYPE|ID'     
 if(FindFile((LPSTR)&FullPath, PathOut))return 0;   
 wsprintf((LPSTR)&FullPath,"%s%s%08X.*",&RootPath,&AIden,TextCRC);  // Search in 'Speech|ID'    
 if(FindFile((LPSTR)&FullPath, PathOut))return 0;   
      
 if(!Creature)   
  {
   wsprintf((LPSTR)&FullPath,"%s%s%s%s%08X.*",&RootPath,&AType,&ARace,&AGend,TextCRC);  // Search in 'Speech|TYPE|RACE|GENDER'    
   if(FindFile((LPSTR)&FullPath, PathOut))return 0;   
   wsprintf((LPSTR)&FullPath,"%s%s%s%08X.*",&RootPath,&AType,&AGend,TextCRC);  // Search in 'Speech|TYPE|GENDER'     
   if(FindFile((LPSTR)&FullPath, PathOut))return 0;   
   wsprintf((LPSTR)&FullPath,"%s%s%08X.*",&RootPath,&AGend,TextCRC);  // Search in 'Speech|GENDER'    
   if(FindFile((LPSTR)&FullPath, PathOut))return 0;   
  } 
      
 wsprintf((LPSTR)&FullPath,"%s%s%s%08X.*",&RootPath,&AType,&ARace,TextCRC);  // Search in 'Speech|TYPE|RACE'
 if(FindFile((LPSTR)&FullPath, PathOut))return 0;   
 wsprintf((LPSTR)&FullPath,"%s%s%08X.*",&RootPath,&AType,TextCRC);  // Search in 'Speech|TYPE'   
 if(FindFile((LPSTR)&FullPath, PathOut))return 0;   
 wsprintf((LPSTR)&FullPath,"%s%08X.*",&RootPath,TextCRC);  // Search in 'Speech' root
 if(FindFile((LPSTR)&FullPath, PathOut))return 0;   
       
 return 2;
}
//------------------------------------------------------------------------------------
long _stdcall GetPitchTTSRelativeToDS(long PitchDS)
{
 if(PitchDS >= (PITCHMAX/2))return 10;
 if(PitchDS <= (PITCHMIN/2))return -10;
 return (PitchDS / ((PITCHMAX/2)/10));
}
//------------------------------------------------------------------------------------
// Extend in future to work with any race [use list of races]
long _stdcall GetRaceVoicePitch(MORROWIND::PCObjNPC_ Npc)
{
 if(!Npc)return 0;
 long  FreqShift = 0;
 LPSTR RaceID    = Npc->Base->Race->GetID();
 if(!lstrcmp(RaceID,"Orc"))FreqShift = ModInfo->Config.VPOrc;		
   else
    {
 if(!lstrcmp(RaceID,"Nord"))FreqShift = ModInfo->Config.VPNord;		
   else
    {
 if(!lstrcmp(RaceID,"Breton"))FreqShift = ModInfo->Config.VPBreton;		
   else
    {
 if(!lstrcmp(RaceID,"Khajiit"))FreqShift = ModInfo->Config.VPKhajiit;		
   else
    {
 if(!lstrcmp(RaceID,"Dark Elf"))FreqShift = ModInfo->Config.VPDarkElf;		
   else
    {
 if(!lstrcmp(RaceID,"High Elf"))FreqShift = ModInfo->Config.VPHighElf;		
   else
    {
 if(!lstrcmp(RaceID,"Wood Elf"))FreqShift = ModInfo->Config.VPWoodElf;		
   else
    {
 if(!lstrcmp(RaceID,"Argonian"))FreqShift = ModInfo->Config.VPArgonian;		
   else
    {
 if(!lstrcmp(RaceID,"Imperial"))FreqShift = ModInfo->Config.VPImperial;		
   else
    {
 if(!lstrcmp(RaceID,"Redguard"))FreqShift = ModInfo->Config.VPRedguard;		  
    } } } } } } } } }
 return FreqShift;
}
//====================================================================================
//                                 SERVICE FUNCTIONS
//------------------------------------------------------------------------------------
long  _stdcall GetDirectionAxisY(void)
{
 return (*(ModInfo->MWGlobals.MouseInvAxisY))?(-1):(1);
}
//------------------------------------------------------------------------------------
void _stdcall ForceFaceToPlayer(MORROWIND::PCObjMACT Actor)
{
 if(Actor->GetReference() != (*ModInfo->MWGlobals.ppGameScene)->GetPlayerActor()->GetReference())
  {
   ((PWORD)Actor)[438]    = 4;  // Analyze this !
   ((PBYTE)Actor)[220]    = 0;  // Sets trigger - NOT ALWAYS WORKS !!!
  }
}
//------------------------------------------------------------------------------------
void _stdcall ZoomDialogueActor(MORROWIND::PCObjMACT Actor) 
{
 float DistPerc;
 float DistNpc;
                  
 if(Actor->GetReference() != (*ModInfo->MWGlobals.ppGameScene)->GetPlayerActor()->GetReference())
  {
   DistNpc = Actor->GetReference()->GetDistance((*ModInfo->MWGlobals.ppGameScene)->GetPlayerActor()->GetReference());  // MaxDist=225, MinDist=60
   ModInfo->LastFOV = (*ModInfo->MWGlobals.ppGameScene)->GetCurrentFOV();  // Default=75, MinFov=20 for 225 DIST, MaxFov=75 for 60 DIST
   if(DistNpc > ModInfo->Config.ZoomDistNPC)
    {
     DistPerc             = NumToPerc(ModInfo->Config.ZoomDistNPC, DistNpc);
     ModInfo->CurrentFOV  = PercToNum(DistPerc, ModInfo->LastFOV);   
     ModInfo->StartingFOV = ModInfo->LastFOV;
     if(ModInfo->CurrentFOV <= 0)ModInfo->CurrentFOV = 1;
     if(!ModInfo->Config.SmoothZooming)(*ModInfo->MWGlobals.ppGameScene)->SetCurrentFOV(ModInfo->CurrentFOV);     // Removed for smooth camera
       else ModInfo->ZoomingInProces = true;
    } 
  }
}     
//------------------------------------------------------------------------------------
//  Do not forget: Y axis may be inverted by a user
//    CW  -> + 
//    ACW -> -
//
void _stdcall SetCameraToDlgActor(MORROWIND::PCObjMACT Actor) 
{
 if(Actor->GetReference() != (*ModInfo->MWGlobals.ppGameScene)->GetPlayerActor()->GetReference())
  { 
   ModInfo->pTrkTarget = NULL;  // Do not take 'Base->BPHead->Model' for NPCs - sometimes sets focus to a wrong NPC!
   if(!ModInfo->pTrkTarget)ModInfo->pTrkTarget = ModInfo->CurrentTalker->GetActor()->GetReference()->FindModelByName("Bip01 Head");           
   if(!ModInfo->pTrkTarget)ModInfo->pTrkTarget = ModInfo->CurrentTalker->GetActor()->GetReference()->FindModelByName("Bip01 Neck");      
   if(!ModInfo->pTrkTarget)ModInfo->pTrkTarget = ModInfo->CurrentTalker->GetActor()->GetReference()->FindModelByName("Head");  // Wrong coordinates, when used some model replacers  
   if(!ModInfo->pTrkTarget)ModInfo->pTrkTarget = ModInfo->CurrentTalker->GetActor()->GetReference()->FindModelByName("Neck");  // Wrong coordinates, when used some model replacers          
     
   ModInfo->pTrkCamera    = (*ModInfo->MWGlobals.ppGameScene)->WorldSceneGraph.Camera.pNiCameraNode;  
   ModInfo->CamRotDeltaX  = 0;
   ModInfo->CamRotDeltaY  = 0;
   ModInfo->CamTrackingActive = (ModInfo->pTrkTarget&&ModInfo->pTrkCamera);
  }    
}
//------------------------------------------------------------------------------------
void _stdcall UpdateCameraTracking(void)  
{
 long  DirecX, DirecY; 
 float Distance; 
 QWORD TgtSinCosX, TgtSinCosY;      
   
 if(!ModInfo->CamTrackingActive||!ModInfo->pTrkTarget||!ModInfo->pTrkCamera)return;      
                  
 float DiffX = (ModInfo->pTrkTarget->kWorldBound.x-ModInfo->pTrkCamera->kWorldBound.x);     
 float DiffY = (ModInfo->pTrkTarget->kWorldBound.y-ModInfo->pTrkCamera->kWorldBound.y);      
 float DiffZ = (ModInfo->pTrkTarget->kWorldBound.z-ModInfo->pTrkCamera->kWorldBound.z);             
 if((Distance = sqrt((DiffX*DiffX)+(DiffY*DiffY)+(DiffZ*DiffZ))))
  {
   TgtSinCosX = MakeSinCos((DiffX / Distance), (DiffY / Distance));
   TgtSinCosY = ModInfo->SinCosTable[FindDegreeByCos((QWORD*)&ModInfo->SinCosTable, (DiffZ / Distance))];
  }
   else
    {
     TgtSinCosX = MakeSinCos(0, 0);
     TgtSinCosY = ModInfo->SinCosTable[FindDegreeByCos((QWORD*)&ModInfo->SinCosTable, 0)];
    } 
                            
 // Track a Actor`s horizontal movements  (Camera X axis)                                
 long ViewAngleX = FindNearestDegree((QWORD*)&ModInfo->SinCosTable, MakeSinCos(ModInfo->pTrkCamera->kWorldDir.x, ModInfo->pTrkCamera->kWorldDir.y)); 
 long TargAngleX = FindNearestDegree((QWORD*)&ModInfo->SinCosTable, TgtSinCosX); 
 long DistAngleX = GetNearestAngle((QWORD*)&ModInfo->SinCosTable, ViewAngleX, TargAngleX, &DirecX);
 ModInfo->CamRotDeltaX = (CAMROTATESTEP*(float)DirecX);
       
 // Track a Actor`s vertical movements  (Camera Y axis)
 long ViewAngleY = FindDegreeByCos((QWORD*)&ModInfo->SinCosTable, ModInfo->pTrkCamera->kWorldDir.z); // 0-90 = Upper; 90-180 = Lower                                              
 long TargAngleY = FindNearestDegree((QWORD*)&ModInfo->SinCosTable, TgtSinCosY); 
 long DistAngleY = GetNearestAngle((QWORD*)&ModInfo->SinCosTable, ViewAngleY, TargAngleY, &DirecY);
 ModInfo->CamRotDeltaY = (CAMROTATESTEP*(float)DirecY)*(float)GetDirectionAxisY();
     
 // Controlling camera`s movement smoothness 0.002-0.0002 / FAST-SLOW
 if(ModInfo->CamRotDeltaX == 0)ModInfo->SmoothFaceTrackingX = true;
 if(ModInfo->CamRotDeltaY == 0)ModInfo->SmoothFaceTrackingY = true;
 (*ModInfo->MWGlobals.ppGameScene)->CamMovMultX = CAMMOVMULTMIN + PercToNum(NumToPerc((float)DistAngleX,180.0),(ModInfo->SmoothFaceTrackingX)?(CAMMOVMULTMIN):(CAMMOVMULTMAX));
 (*ModInfo->MWGlobals.ppGameScene)->CamMovMultY = CAMMOVMULTMIN + PercToNum(NumToPerc((float)DistAngleY,90.0),(ModInfo->SmoothFaceTrackingY)?(CAMMOVMULTMIN):(CAMMOVMULTMAX));

 /*BYTE MessStr[512]; 
 wsprintf((LPSTR)&MessStr,"DeltaX: %i, DeltaY: %i",(long)ModInfo->CamRotDeltaX,(long)ModInfo->CamRotDeltaY);
 if((ModInfo->CamRotDeltaX != 0)||(ModInfo->CamRotDeltaY != 0))
  ModInfo->CamRotDeltaX = ModInfo->CamRotDeltaX;
 //wsprintf((LPSTR)&MessStr,"VANGLE_X: %i, TANGLE_X: %i, DANGLE_X: %i, VANGLE_Y: %i, TANGLE_Y: %i, DANGLE_Y: %i,",ViewAngleX,TargAngleX,DistAngleX,ViewAngleY,TargAngleY,DistAngleY); 
 SetWindowText(GetForegroundWindow(),(LPSTR)&MessStr);*/ 
}
//------------------------------------------------------------------------------------
void _stdcall UpdateCameraZooming(void)
{
 if(ModInfo->StartingFOV > ModInfo->CurrentFOV){ModInfo->StartingFOV -= CAMZOOMSPEED;(*ModInfo->MWGlobals.ppGameScene)->SetCurrentFOV(ModInfo->StartingFOV);} 
  else ModInfo->ZoomingInProces = false;
}
//------------------------------------------------------------------------------------
bool _stdcall TestTargetMoveDist(MORROWIND::PCObjREFR Target, VECTOR3 *TgtOriginPos, float Distance)     
{
 float PosX,PosY,PosZ;
 if(!Target)return false;
 Target->GetPosition(&PosX, &PosY, &PosZ);
 PosX -= TgtOriginPos->x;
 PosY -= TgtOriginPos->y;
 PosZ -= TgtOriginPos->z;
 return (!IsInRange<float>(PosX,Distance,-Distance)||!IsInRange<float>(PosY,Distance,-Distance)||!IsInRange<float>(PosZ,Distance,-Distance));
}
//------------------------------------------------------------------------------------
MORROWIND::PCObjREFR _stdcall GetConsoleObjectRef(void)
{
 MORROWIND::PCObjREFR ObjRefr = NULL;
 MORROWIND::PCWindow  ConWin  = ModInfo->MWGlobals.FindWindowByID(miConsole);
 if(ConWin)ConWin->_GetData((PVOID)&ObjRefr,0xFF5C,8,0,0);
 return ObjRefr;
}
//====================================================================================
//                              ADDITIONAL FUNCTIONS
//------------------------------------------------------------------------------------
DWORD _stdcall ChangeNumberInXml(long Value, LPSTR ValueName, LPSTR XmlCfgStr)
{
 int   SPos;
 int   ValueLen;
 int   XmlStrLen;
 BYTE  SPartA[128];
 BYTE  SPartB[128];
 BYTE  XmlStr[256];
 LPSTR SPartAPtr;
 LPSTR SPartBPtr;
 LPSTR XmlStrPtr;

 ValueLen  = lstrlen(ValueName);
 XmlStrLen = lstrlen(XmlCfgStr);
 SPartAPtr = (LPSTR)&SPartA;
 SPartBPtr = (LPSTR)&SPartB;
 XmlStrPtr = (LPSTR)&XmlStr;
 lstrcpyn(XmlStrPtr,XmlCfgStr,sizeof(XmlStr));
 if(XmlStrLen < ValueLen)return 1;
 SPos = -1;
 for(int ctr=XmlStrLen; ctr > ValueLen;ctr--,XmlStrPtr++)if(CompareString(LOCALE_SYSTEM_DEFAULT,NORM_IGNORECASE,XmlStrPtr,ValueLen,ValueName,ValueLen)==CSTR_EQUAL){SPos=(XmlStrLen-ctr);break;}
 if(SPos < 0)return 2;
 XmlStrPtr += ValueLen;
 while((XmlStrPtr[0] != 0)&&(XmlStrPtr[0] != '\"'))XmlStrPtr++;
 if(XmlStrPtr[0] == 0)return 3;
 XmlStrPtr++;
 XmlStrPtr[0] = 0;
 lstrcpyn(SPartAPtr,(LPSTR)&XmlStr,sizeof(SPartA));
 XmlStrPtr++;
 while((XmlStrPtr[0] != 0)&&(XmlStrPtr[0] != '\"'))XmlStrPtr++;
 if(XmlStrPtr[0] == 0)return 3;
 lstrcpyn(SPartBPtr,XmlStrPtr,sizeof(SPartB));
 wsprintf((LPSTR)&XmlStr,"%s%i%s",SPartAPtr,Value,SPartBPtr);
 lstrcpy(XmlCfgStr,(LPSTR)&XmlStr);
 return 0;
}
//====================================================================================


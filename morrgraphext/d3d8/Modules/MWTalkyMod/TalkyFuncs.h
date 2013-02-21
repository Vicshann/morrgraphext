#pragma once

#ifndef TalkyFuncsH
#define TalkyFuncsH

//===========================================================================
#include "MWDerived.h"
#include "MemWork.h"

//===========================================================================
#define BASEVOLUMEMAX   255
#define PITCHMAX        10000
#define PITCHMIN        -10000 
#define CAMZOOMSPEED    1.0
#define CAMROTATESTEP   10 		
#define CAMMOVMULTMIN   0.00002		   // Slowest speed of camera
#define CAMMOVMULTMAX   0.0200         // Fastest speed of camera
#define ZOOMSPEAKDEL    500
#define ZOOMVOLDIV      1.5            // Opposite to DS distance volume reducing  (Leaved a little redusing for better expirience)

#define TLKYFLAG        0x594B4C54	   // TLKY
#define KEYWAITDEL	    1000
                        
class   CTalker;
struct  VOICEINFO;
typedef CTalker*   PCTalker;
typedef VOICEINFO* PVOICEINFO;

//---------------------------------------------------------------------------
struct VOICEINFO
{
 CTalky Voice;
 bool   AutoSpeak;
 bool   VoiceEnabled; 
 BYTE   VoiceName[128];
 BYTE   VoiceConfig[128];
};
//---------------------------------------------------------------------------
void  _stdcall NormalizeTextString(LPSTR DstTextStr, LPSTR SrcTextStr);
long  _stdcall GetPitchTTSRelativeToDS(long PitchDS);
long  _stdcall GetRaceVoicePitch(MORROWIND::PCObjNPC_ Npc);
DWORD _stdcall MakeRawTextAndCRC(LPSTR TextString);
DWORD _stdcall ForceActorSpeaking(PCTalker Talker); 
DWORD _stdcall FindActorSpeechFile(LPSTR RootDir, LPSTR PathOut, LPSTR RaceID, LPSTR ActorID, bool Female, bool Creature, DWORD TextCRC);

void  _stdcall UpdateCameraZooming(void);
void  _stdcall UpdateCameraTracking(void);
void  _stdcall ForceFaceToPlayer(MORROWIND::PCObjMACT Actor);
void  _stdcall ZoomDialogueActor(MORROWIND::PCObjMACT Actor); 
void  _stdcall SetCameraToDlgActor(MORROWIND::PCObjMACT Actor); 
bool  _stdcall TestTargetMoveDist(MORROWIND::PCObjREFR Target, VECTOR3 *TgtOriginPos, float Distance);

long  _stdcall GetDirectionAxisY(void);
DWORD _stdcall ChangeNumberInXml(long Value, LPSTR ValueName, LPSTR XmlCfgStr);
MORROWIND::PCObjREFR _stdcall GetConsoleObjectRef(void);

//---------------------------------------------------------------------------
// WARNINS: Self initialized class - make all possible memory checks!
//
// Talker is free if 'DSBuffer' are invalid.
//
class CTalker
{
private:
 PCTalker Prev;
 PCTalker Next;
 bool     VPlayer;
 bool     RealVoice;       // Voice is a MP3 file
 UINT     WChannels;
 UINT     WSamplesPerSec;
 UINT     WBitsPerSample;
 DWORD    TxtCrcBase;      // Text as in editor
 DWORD    TxtCrcFinal;     // Text with a resolved replacers
 DWORD    SpBufSize;
 PVOID    SpBuffer;
 LPSTR    TextLine;
 LPSTR    RawTextLine; 
 LPSTR    TextToSpeak;
 LPDIRECTSOUNDBUFFER  DSBuffer;
 MORROWIND::PCObjMACT Actor;
 VOICEINFO            CurVoice;

public:
 PCTalker PrepareNew(MORROWIND::PCObjMACT Actor);
 DWORD    GetWavSpeechBuffer(PVOID *Buffer, DWORD *BufSize);
 DWORD    SendWavDataToObject(void);
 DWORD    SpeakToBuffer(void);
 DWORD    SetVoicePitch(void);
 DWORD    FillWavFmtStruct(WAVEFORMATEX *WaveFmtx);
 DWORD    SilenceActor(MORROWIND::PCObjMACT Actor);
 void     SilenceAllTalkers(void);
 void     UpdateTextLine(LPSTR Text);
 void     UpdateRawTextLine(LPSTR Text);
 void     InitDialogueText(void);
 void     SendTextToSpeaking(void);
 bool     PrepareSpeechText(void);
 
 DWORD    Silence(void){return (this)?(this->SilenceActor(this->Actor)):(-1);}
 DWORD    GetFinalCRC(void){return (this)?(this->TxtCrcFinal):(0);}
 void     SetFinalCRC(DWORD TextCrc){if(this)this->TxtCrcFinal = TextCrc;}
 void     SetDSBuffer(LPDIRECTSOUNDBUFFER DSBuf){if(this)this->DSBuffer = DSBuf;}
 void     Reset(void){if(this)this->TxtCrcFinal = -1;this->ClearTextLines();}
 void     ClearTextLines(void){if(this){if(this->TextLine)TextLine[0]=0;if(this->RawTextLine)RawTextLine[0]=0;}}
 void     SpeakText(LPSTR Text){ClearTextLines();UpdateTextLine(Text);PrepareSpeechText();SpeakToBuffer();}
 void     SetVoice(PVOICEINFO BaseVoice){if(this)FastMoveMemory(&this->CurVoice,BaseVoice,sizeof(VOICEINFO));}
 void     SetVoiceFlags(bool Enabled, bool AutoSpeak){if(this){this->CurVoice.VoiceEnabled=Enabled;this->CurVoice.AutoSpeak=AutoSpeak;}}
 bool     IsSameDSBuf(LPDIRECTSOUNDBUFFER DSBuf){return (this && (DSBuf == this->DSBuffer));}
 bool     IsRealVoice(void){return (this && this->RealVoice);}
 bool     IsPlaying(void){return (this && IsValidDSBuffer(this->DSBuffer));}
 bool     IsValid(void){return (this && this->Actor);}
 bool     IsEmpty(void){return (!this || !this->TextLine || !this->TextLine[0]);}
 bool     IsActive(void){return (this && !this->IsEmpty() && (this->TxtCrcFinal != -1));}
 MORROWIND::PCObjMACT GetActor(void){return (this)?(this->Actor):(NULL);}
};
//---------------------------------------------------------------------------
#endif


#pragma once

#ifndef MWDerivedH
#define MWDerivedH

//===========================================================================
#include "CommonFuncs.h"
#include "Morrowind.h"
#include "MathTools.h"
#include "SinCos.h"
#include "Talky.h"


namespace MWDerived
{
//===========================================================================
class CSoundBase : public MORROWIND::CSoundBase      // Size = 0x02D8 (728)
{
public:
  void                 MWTHISCALL __SetDSBufFrequency(MORROWIND::PCSndInfo Sound, float FreqMult); 
  MORROWIND::PCSndInfo MWTHISCALL __OpenVoiceFile(LPSTR VoiceFile, DWORD Flag);
};
//===========================================================================
class CCamera : public MORROWIND::CCamera
{
public:
  void MWTHISCALL __SwitchViewMode(void);
  void MWTHISCALL __UpdatePlayerPositions(DWORD Unkn); 
};
//===========================================================================
class CAnimation : public MORROWIND::CAnimation
{
public:
   void MWTHISCALL __HeadTrack(MORROWIND::PCObjREFR Tracker, MORROWIND::PCObjREFR Target);
   void MWTHISCALL __ChangeAnimGroup(BYTE AnimGroup, long AnimType, long Unkn00, long Unkn01);      
};
//===========================================================================
class CRenderer : public MORROWIND::CRenderer
{
public:
  void MWTHISCALL __ProcessPlayerUseAction(void);
};
//===========================================================================
class CWindow : public MORROWIND::CWindow
{
public:
  MORROWIND::PCWindow MWTHISCALL __DrawTextLine(short TgtWndID, LPSTR TextLine, BOOL UnkFlag00, BOOL UnkFlag01);  // 'TextLine' points to part of text in whitch char '<' replaced with 'ZERO'
};
//===========================================================================
class CTESFont : public MORROWIND::CTESFont
{
public:
PVOID MWTHISCALL __PrepareTextDlgJnl(PVOID Class_20,MORROWIND::PCObjMACT Actor,DWORD Zero_00,DWORD Zero_01,DWORD Zero_02,LPSTR TopicText,DWORD Zero_03,PVOID Unknow00,PVOID Unknow01,DWORD UnkIndex);
};
//===========================================================================
class CObjMACP : public MORROWIND::CObjMACP
{
public:
  void MWTHISCALL __GoToJailChecking(void);
};
//===========================================================================
class CObjMACH : public MORROWIND::CObjMACH
{
public:
  void MWTHISCALL __AIDoActorAttack(void);
};
//===========================================================================
}
#endif

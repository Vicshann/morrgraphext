
#pragma once

#ifndef CommonFuncsH
#define CommonFuncsH

#define WIN32_LEAN_AND_MEAN		 // Exclude rarely-used stuff from Windows headers
#define _WIN32_WINNT  0x0500     // For CoInitializeEx support

#include <Windows.h>
#include <mmsystem.h> 
#include <dsound.h>

//---------------------------------------------------------------------------
//#define sqrt             FPUSqrt32
//#define MWTHISCALL              // Class member function call type (empty for '__thiscall')
//#ifndef DEFCALL
//#define DEFCALL  //__fastcall
//#endif

#define CRC32POLYNOME 0xEDB88320
#define IUNKFLAG      0x4B4E5549
#define DSBFFLAG      0x46425344

//---------------------------------------------------------------------------
//#define ZeroStruct(a) { FastZeroMemory(&a,sizeof(a)); }
//#define ZeroPointer(a) { FastZeroMemory(a,sizeof(*a)); }
//#define InitPointer(a,b) { a=new b(); ZeroPointer(a); }

typedef unsigned __int64 QWORD;
typedef QWORD* PQWORD;
//---------------------------------------------------------------------------
#define FOREGROUND_YELLOW (FOREGROUND_RED|FOREGROUND_GREEN)
#define _MAKEVKEYCOMB(VKeyA,VKeyB,VKeyC,VKeyD) ((((DWORD)((BYTE)(VKeyA)))<<24)|(((DWORD)((BYTE)(VKeyB)))<<16)|(((DWORD)((BYTE)(VKeyC)))<<8)|((DWORD)((BYTE)(VKeyD))))

#define _STRNGZ(arg) #arg
#define _STRNG(arg) _STRNGZ(arg) 
  
#define _MERGE(a,b) a####b
#define _MAKENAME(pref,val,post) pref####val####post
#define _REV_DW(Value)                (((DWORD)(Value) << 24)|((DWORD)(Value) >> 24)|(((DWORD)(Value) << 8)&0x00FF0000)|(((DWORD)(Value) >> 8)&0x0000FF00))
#define _MINIMAL(ValueA, ValueB)      (((ValueA) < (ValueB))?(ValueA):(ValueB))
#define _PERCTOMAX(Perc, Num)         (((Num)*100)/((Perc)?(Perc):1))
#define _NUMTOPERC(Num, MaxVal)       (((Num)*100)/(MaxVal))
#define _PERCTONUM(Perc, MaxVal)      (((Perc)*(MaxVal))/100)
#define _SWAPNUMBERS(A,B)             ((A) ^= (B) ^= (A) ^= (B))
#define _MIN(a, b)                    (((a) < (b)) ? (a) : (b))
#define _MAX(a, b)                    (((a) > (b)) ? (a) : (b))
#define _DELTA(a,b)                   (((a) > (b)) ? ((a) - (b)) : ((b) - (a)))
#define _BYTELIMIT(B)                 ((B) < 0)?(0):(((B) > 255)?(255):(B));
#define _LODWORD(Value)               ((DWORD)(Value))
#define _HIDWORD(Value)               ((DWORD)((Value)>>32))
#define _ALIGN_FRWRD(Value,Alignment) ((((Value)/(Alignment))+((bool)((Value)%(Alignment))))*(Alignment))
#define _ALIGN_BKWRD(Value,Alignment) (((Value)/(Alignment))*(Alignment))

//---------------------------------------------------------------------------
#define _DO_ONCE_VOID   {static bool DoOnce = false;if(DoOnce)return;DoOnce = true;}

#define UNSAFELOOP      __asm{xzc: jmp xzc}
#define SAFELOOP(Delay) __asm Label: push Delay      \
                        __asm call dword ptr [Sleep] \
                        __asm jmp Label                                                                                                                      

//---------------------------------------------------------------------------
template<class N, class M> __inline M NumToPerc(N Num, M MaxVal){return (((Num)*100)/(MaxVal));}
template<class P, class M> __inline M PercToNum(P Per, M MaxVal){return (((Per)*(MaxVal))/100);}                         
template<class T> __inline T    Min(T ValueA, T ValueB){return (ValueA > ValueB)?(ValueB):(ValueA);} 
template<class T> __inline T    Max(T ValueA, T ValueB){return (ValueA > ValueB)?(ValueA):(ValueB);}                      
template<class T> __inline bool IsInRange(T Value, T Minv, T Maxv){return ((Value >= Min(Minv,Maxv))&&(Value <= Max(Minv,Maxv)));}
                                   
template<class R, class T> __inline R GetAddress(T Src){return reinterpret_cast<R>(reinterpret_cast<void* &>(Src));}
//template<class T> __inline void ZeroStruct(T &Stru){FastZeroMemory(&Stru,sizeof(Stru));}  
//template<class T> __inline void ZeroPointer(T Stru){FastZeroMemory(Stru,sizeof(*Stru));}
template<class T> __inline T*   AddressOf(T Src){return reinterpret_cast<T*>(reinterpret_cast<void* &>(Src));}
template<class T> __inline int  SetAddressInVFT(PVOID VFT, int Index, T Proc, LPSTR Signature=NULL){PVOID Addr=reinterpret_cast<PVOID>(reinterpret_cast<void* &>(Proc));return MemoryPatchLocal(&((PVOID*)VFT)[Index], &Addr, sizeof(PVOID), Signature);}
template<class T> __inline bool SetPtrAddress(PVOID Src, T &Dst, LPSTR Signature=NULL)
{
 if(IsMemSignatureMatch(Src, Signature)){Dst = reinterpret_cast<T>(Src);return false;}
   else return true;
}

__inline void __cdecl SetAddress(PVOID SrcAddr,...){((PVOID*)(((PVOID*)&SrcAddr)[1]))[0] = SrcAddr;}   // Hack :)  

//====================================================================================		
DWORD   _stdcall ComputeCRC32(PVOID Buffer, DWORD BufferSize);
WORD    _stdcall ComputeCRC16(PVOID Buffer, DWORD BufferSize);
BOOL    _stdcall IsKeyCombinationPressed(DWORD Combination);
BOOL    _stdcall SetWinConsoleSizes(DWORD WndWidth, DWORD WndHeight, DWORD BufWidth, DWORD BufHeight);
LPSTR   _stdcall FormatCurDateTime(LPSTR DateTimeStr);
int     _stdcall GetDesktopRefreshRate(void);
int     _stdcall RefreshINIValueInt(LPSTR SectionName, LPSTR ValueName, int Default, LPSTR FileName);
int     _stdcall RefreshINIValueStr(LPSTR SectionName, LPSTR ValueName, LPSTR Default, LPSTR RetString, DWORD Size, LPSTR FileName);
bool    _stdcall IsValidDSBuffer(LPDIRECTSOUNDBUFFER pDSBuffer);

//===========================================================================
#endif

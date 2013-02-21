// AUTHOR: Vicshann

#include "ConvertTools.h"

//====================================================================================
long _stdcall CharToHex(BYTE CharValue) 
{
 if((CharValue >= 0x30)&&(CharValue <= 0x39))return (CharValue - 0x30);		 // 0 - 9
 if((CharValue >= 0x41)&&(CharValue <= 0x46))return (CharValue - (0x41-10)); // A - F
 if((CharValue >= 0x61)&&(CharValue <= 0x66))return (CharValue - (0x41-10)); // a - f
 return -1;
}
//---------------------------------------------------------------------------
WORD _stdcall HexToChar(BYTE Value, bool UpCase)
{
 WORD Result = 0;
 for(int ctr=1;ctr >= 0;ctr--)
  {
   BYTE Tmp = (Value & 0x0F);
   if(Tmp < 0x0A)((PBYTE)&Result)[ctr] |= 0x30+Tmp;
	 else ((PBYTE)&Result)[ctr] |= 0x37+Tmp;  // Uppercase
   Value  >>= 4;
  }
 return Result;
}
//---------------------------------------------------------------------------
LPSTR _stdcall ConvertToHexDW(DWORD Value, DWORD MaxDigits, LPSTR Number, bool UpCase)
{
 char  HexNums[] = "0123456789ABCDEF0123456789abcdef";
 DWORD DgCnt;
 DWORD Case;

 for(int ctr=0;ctr < (int)MaxDigits;ctr++){Number[ctr]='0';Number[ctr+1]=0;}
 if(UpCase)Case = 0;
   else Case = 16;
 for(DgCnt = 8;DgCnt > 0;DgCnt--)
  {
   ((char *)Number)[DgCnt-1] = HexNums[((Value & 0x0000000F)+Case)];
   Value = Value >> 4;
  }
 for(DgCnt = 0;Number[DgCnt] == '0';DgCnt++);
 if((MaxDigits > 8) || (MaxDigits == 0))MaxDigits = 8;
 if((8-DgCnt) < MaxDigits)DgCnt = 8 - MaxDigits;
 return (Number + DgCnt);
}
//---------------------------------------------------------------------------
LPSTR _stdcall ConvertToDecDW(DWORD Value, LPSTR Number)
{
 char  DecNums[] = "0123456789";
 DWORD DgCnt;
 DWORD TmpValue;
 DWORD Divid;
 
 Divid  = 1000000000;
 for(DgCnt = 0;DgCnt < 10;DgCnt++)
  {
   TmpValue = Value / Divid;
   Number[DgCnt] = DecNums[TmpValue];  
   Value -= TmpValue * Divid;
   Divid  = Divid / 10;
  }
 for(DgCnt = 0;(DgCnt < 9) && (Number[DgCnt] == '0');DgCnt++);
 return (LPSTR)(((DWORD)Number) + DgCnt);
}
//---------------------------------------------------------------------------
DWORD _stdcall HexStrToDW(LPSTR String)   // Fast, but do not safe
{
 long  StrLength = 0;
 DWORD Result    = 0;
 DWORD DgtPow    = 1;
 BYTE  Symbol;

 for(int ctr=0;CharToHex(String[ctr]) >= 0;ctr++)StrLength++;     // Break on any non hex digit
 for(long ctr=1;(ctr<=8)&&(ctr<=StrLength);ctr++)
  {
   Symbol = (String[StrLength-ctr]-0x30);
   if(Symbol > 9)Symbol  -= 7;
   if(Symbol > 15)Symbol -= 32;
   Result += (DgtPow*Symbol);
   DgtPow  = 1;
   for(long num = 0;num < ctr;num++)DgtPow = DgtPow*16;
  }
 return Result;
}
//---------------------------------------------------------------------------
DWORD _stdcall DecStrToDW(LPSTR String)   // Fast, but do not safe
{
 long  StrLength = 0;
 DWORD Result    = 0;
 DWORD DgtPow    = 1;
 BYTE  Symbol;

 for(int ctr=0;((BYTE)(String[ctr]-0x30)) <= 9;ctr++)StrLength++;     // Break on any non digit
 for(long ctr=1;ctr<=StrLength;ctr++)
  {
   Symbol  = (String[StrLength-ctr]-0x30);
   Result += (DgtPow*Symbol);
   DgtPow  = 1;
   for(long num = 0;num < ctr;num++)DgtPow = DgtPow*10;
  }
 return Result;
}
//---------------------------------------------------------------------------
//  CmdAddr [ESP+4]
//  CmdLen  [ESP+8]
//  JmpAddr [ESP+12]
//
_declspec( naked )DWORD _stdcall AddrToRelAddr(PVOID CmdAddr, DWORD CmdLen, PVOID TgtAddr)
{
 __asm
 {
  MOV  EAX,[ESP+4]
  ADD  EAX,[ESP+8]
  SUB  EAX,[ESP+12]
  NEG  EAX
  RET 12
 } 
}
//===========================================================================
//  CmdAddr   [ESP+4]
//  CmdLen    [ESP+8]
//  JmpOffset [ESP+12]
//
// WRONG !!!!!!! How to know difference bet. 8-bit negative and 32-bit positive
_declspec( naked )PVOID _stdcall RelAddrToAddr(PVOID CmdAddr, DWORD CmdLen, DWORD TgtOffset)
{
 __asm
 {
  MOV   EAX,[ESP+12]
  CMP   EAX,0x000000FF
  MOVSX EAX,AL
  JBE   RATA1         // ; Rel8
  MOV   EAX,[ESP+12]
  CMP   EAX,0x0000FFFF
  MOVSX EAX,AX
  JBE   RATA1         // ; Rel16
  MOV   EAX,[ESP+12]  // ; Rel32
RATA1:
  ADD   EAX,[ESP+8]
  ADD   EAX,[ESP+4]
  RET   12
 }
}
//---------------------------------------------------------------------------



//====================================================================================


// AUTHOR: Vicshann

#include "StringTools.h"

//====================================================================================
int _stdcall CharPosFromLeft(LPSTR String, char Letter, int Index, int Offset)
{
 int Counter = 0;
 int Num     = 0;

 if(Offset > lstrlen(String))return -1;
 if(Index  < 0)Index = -Index;    // For all possible index manipulations
 if(Index == 0)Index = 1;
 Counter += Offset;
 for(;;)
  {
   if(String[Counter] == Letter)Num++;
   if(Num == Index)return Counter;
   if(String[Counter] == 0)return -1;
   Counter++;
  }
}
//---------------------------------------------------------------------------
int _stdcall CharPosFromRight(LPSTR String, char Letter, int Index, int Offset)
{
 int Counter = lstrlen(String)-1;
 int Num     = 0;

 if(Offset > Counter)return -1;
 if(Index  < 0)Index = -Index;    // For all possible index manipulations
 if(Index == 0)Index = 1;
 Counter -= Offset;
 for(;;)
  {
   if(String[Counter] == Letter)Num++;
   if(Num == Index)return Counter;
   if(Counter == 0)return -1;
   Counter--;
  }
}
//---------------------------------------------------------------------------
int _stdcall ReplaceChar(LPSTR str, BYTE what, BYTE with)
{
 int ctr = 0;
 for(;*str;str++){if(*str == what){*str = with;ctr++;}}
 return ctr;
}
//---------------------------------------------------------------------------
int _stdcall FindEndOfLine(LPSTR Line, int* NextLine)
{
 int res;
 int ctr = 0;
 for(;Line[ctr];ctr++){if((Line[ctr] == 0x0D)||(Line[ctr] == 0x0A))break;}  // a guess :)
 res = ctr;
 if(NextLine)
  {   
   while(Line[ctr] && (Line[++ctr] == 0x0A));
   if(Line[ctr])*NextLine = ctr;
    else *NextLine = -1;
  }
 return res; 
}
//---------------------------------------------------------------------------
int _stdcall GetCharCount(LPSTR String, char Par)
{
 int Counter = 0;
 int Num     = 0;

 for(;;)
  {
   if(String[Counter] == Par)Num++;
   if(String[Counter] == 0)return Num;
   Counter++;
  }
}
//---------------------------------------------------------------------------
bool _stdcall IsUnicodeString(PVOID String)
{
 if((((BYTE*)String)[0]!=0)&&(((BYTE*)String)[1]==0)&&(((BYTE*)String)[2]!=0)&&(((BYTE*)String)[3]==0))return true;
 return false;
}
//---------------------------------------------------------------------------


//====================================================================================



#include "CommonFuncs.h"
//---------------------------------------------------------------------------
void _stdcall ConMessageOut(LPSTR Message, DWORD TxtAttr)
{
 DWORD  Result;
 HANDLE hConOutput;

 hConOutput = GetStdHandle(STD_OUTPUT_HANDLE);
 if(TxtAttr)SetConsoleTextAttribute(hConOutput,TxtAttr);
 WriteConsole(hConOutput,Message,lstrlen(Message),&Result,NULL);
 WriteConsole(hConOutput,"\n\r",2,&Result,NULL);
}	
//---------------------------------------------------------------------------
BOOL _stdcall SetWinConsoleSizes(DWORD WndWidth, DWORD WndHeight, DWORD BufWidth, DWORD BufHeight)
{
 HANDLE hConOutput;  
 CONSOLE_SCREEN_BUFFER_INFO ConBufInfo;

 hConOutput = GetStdHandle(STD_OUTPUT_HANDLE);
 if(!GetConsoleScreenBufferInfo(hConOutput,&ConBufInfo))return false;
 // 
 // Need do some a VERY HARD calculations !!!
 //
 
 ConBufInfo.dwSize.X = BufWidth;
 ConBufInfo.dwSize.Y = BufHeight;
  
 if(!SetConsoleWindowInfo(hConOutput,true,&ConBufInfo.srWindow))return false;
 if(!SetConsoleScreenBufferSize(hConOutput,ConBufInfo.dwSize))return false; 
 return true;
}
//===========================================================================
// Suspicious... Taked from 'WinApiOverride32' source
WORD _stdcall ComputeCRC16(PVOID Buffer, DWORD BufferSize)
{
 if(!Buffer || !BufferSize)return 0;
 DWORD Checksum = 0;
 for(DWORD ctr=0;ctr < (BufferSize/2);ctr++)Checksum += ((PWORD)Buffer)[ctr];  
 if(BufferSize % 2)Checksum += ((PBYTE)Buffer)[BufferSize-1];
 Checksum  = (Checksum >> 16) + (Checksum & 0xffff);
 Checksum += (Checksum >> 16);
 Checksum =~ Checksum;
 return Checksum;
}
//---------------------------------------------------------------------------
// Something from USB spec.
DWORD _stdcall ComputeCRC32(PVOID Buffer, DWORD BufferSize)
{
 if(!Buffer || !BufferSize)return 0;
 DWORD Checksum = 0xFFFFFFFF;
 for(int bctr=0;bctr < BufferSize;bctr++)
  {
   Checksum ^= ((PBYTE)Buffer)[bctr];
   for(int ctr=0;ctr < 8;ctr++)Checksum = ((Checksum >> 1) ^ (CRC32POLYNOME & ~((Checksum & 1) - 1))); // {if((Checksum = Checksum >> 1) & 1)Checksum ^= 0xA001A001;}
  }
 return Checksum;
}
//---------------------------------------------------------------------------
BOOL _stdcall IsKeyCombinationPressed(DWORD Combination)
{
 BYTE KeyCode;
 WORD KeyState;

 for(DWORD ctr=4;ctr > 0;ctr--)
  {
   if((KeyCode = (Combination & 0x000000FF)))
	{
     KeyState = GetAsyncKeyState(KeyCode);	// 1 - key is DOWN; 0 - key is UP
	 if(!(KeyState & 0x8000))return FALSE;  // If one of keys in combination is up - no combination pressed 
	}
   Combination = (Combination >> 8);
  }
 return TRUE;
}
//---------------------------------------------------------------------------
LPSTR _stdcall FormatCurDateTime(LPSTR DateTimeStr)
{
 static DWORD MsgNum = 0;
 SYSTEMTIME   CurDateTime;
   
 GetLocalTime(&CurDateTime);
 wsprintf(DateTimeStr,"%u_%u.%u.%u_%u:%u:%u:%u",MsgNum,CurDateTime.wDay,CurDateTime.wMonth,CurDateTime.wYear,CurDateTime.wHour,CurDateTime.wMinute,CurDateTime.wSecond,CurDateTime.wMilliseconds);
 MsgNum++;
 return DateTimeStr;
}
//====================================================================================
int _stdcall GetDesktopRefreshRate(void)
{
 HDC hDCScreen = GetDC(NULL);
 int Refresh   = GetDeviceCaps(hDCScreen, VREFRESH);
 ReleaseDC(NULL, hDCScreen);
 return Refresh;
}
//====================================================================================
int _stdcall RefreshINIValueInt(LPSTR SectionName, LPSTR ValueName, int Default, LPSTR FileName)
{
 BYTE Buffer[128];
 int  Result = GetPrivateProfileInt(SectionName,ValueName,Default,FileName);
 wsprintf((LPSTR)&Buffer,"%i",Result);
 WritePrivateProfileString(SectionName,ValueName,(LPSTR)&Buffer,FileName);
 return Result;
}
//---------------------------------------------------------------------------
int _stdcall RefreshINIValueStr(LPSTR SectionName, LPSTR ValueName, LPSTR Default, LPSTR RetString, DWORD Size, LPSTR FileName)
{
 int  Result = GetPrivateProfileString(SectionName, ValueName, Default, RetString, Size, FileName);
 WritePrivateProfileString(SectionName,ValueName,RetString,FileName);
 return Result;
}
//---------------------------------------------------------------------------
bool _stdcall IsValidDSBuffer(LPDIRECTSOUNDBUFFER pDSBuffer)
{
 return (!IsBadReadPtr(pDSBuffer,32)&&!IsBadReadPtr((PVOID)((PDWORD)pDSBuffer)[0],4)&&(((PDWORD)pDSBuffer)[2] == IUNKFLAG)&&(((PDWORD)pDSBuffer)[6] == DSBFFLAG));
}
//------------------------------------------------------------------------------------

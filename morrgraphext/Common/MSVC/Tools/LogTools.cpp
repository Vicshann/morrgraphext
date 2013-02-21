// AUTHOR: Vicshann

#include "LogTools.h"


namespace Logger
{
HANDLE       hLogFile = INVALID_HANDLE_VALUE;
LOG_PROC     LogProc  = LogToNull;
MSG_LOG_PROC MsgProc  = OutputDebugString;
LOG_BIN_PROC BinProc  = LogBinToNull;
//====================================================================================
int _stdcall MakeLogMsg(LPSTR OutBuffer, UINT Type, LPSTR ModuleName, LPSTR ProcName, LPSTR Message, va_list arglist)
{
 BYTE FmtBuffer[1025] = {'\0'};  //  > 4k creates __chkstk   // 1024 is MAX for wsprintf
   
 if(ModuleName && ModuleName[0])      // Optimize
  {
   lstrcat((LPSTR)&FmtBuffer,"<");
   lstrcat((LPSTR)&FmtBuffer,ModuleName);
   lstrcat((LPSTR)&FmtBuffer,"> "); 
  }
 if(ProcName && ProcName[0])       // Optimize
  {
   lstrcat((LPSTR)&FmtBuffer,ProcName);
   lstrcat((LPSTR)&FmtBuffer," : ");   
  }
 if(FmtBuffer[0] || (Type & lmtLine))
  {
   lstrcat((LPSTR)&FmtBuffer,Message);
   if(Type & lmtLine)lstrcat((LPSTR)&FmtBuffer,"\r\n");
   Message = (LPSTR)&FmtBuffer; 
  }
 return wvsprintf(OutBuffer,Message,arglist);
}
//---------------------------------------------------------------------------
int _stdcall MakeBinaryLine16(LPSTR DstLine, PBYTE Addr, UINT Size)
{
 DstLine[0] = '\t'; 
 PBYTE CPtr = (PBYTE)&DstLine[1];
 if(Size > 16)Size = 16;
 for(UINT ctr = 0;ctr < Size;ctr++,CPtr+=3)
  {
   WORD chr = HexToChar(Addr[ctr]);
   CPtr[0]  = ((PBYTE)&chr)[0];
   CPtr[1]  = ((PBYTE)&chr)[1];
   CPtr[2]  = 0x20;
  }
 CPtr[0] = '\t';
 for(UINT ctr = 0;ctr < Size;ctr++,CPtr++)
  {
   if(Addr[ctr] >= 0x20)CPtr[0] = Addr[ctr];
     else CPtr[0] = '.';
  }  
 CPtr[0] = '\r';
 CPtr[1] = '\n';
 CPtr[2] = 0;
 return ((PBYTE)DstLine - CPtr)+2; 
}
//---------------------------------------------------------------------------
bool _stdcall BackupLogFile(LPSTR BackupDir, LPSTR LogFile)
{
 SYSTEMTIME stime;
 WIN32_FILE_ATTRIBUTE_DATA finf;
 BYTE TmpName[128];
 BYTE NewPath[MAX_PATH];

 CreateDirectoryPath(BackupDir);
 if(!GetFileAttributesEx(LogFile,GetFileExInfoStandard,&finf))return false;
 if(!FileTimeToSystemTime(&finf.ftLastWriteTime,&stime))return false;
 LPSTR Ext  = GetFileExt(LogFile)-1;
 LPSTR Name = GetFileName(LogFile);
 LPSTR Deli = "";
 lstrcpyn((LPSTR)&TmpName,Name,Ext-Name+1);
 wsprintf((LPSTR)&NewPath,"%s%s%s_%.4u%.2u%.2u-%.2u%.2u%.2u%s",BackupDir,Deli,(LPSTR)&TmpName,stime.wYear,stime.wMonth,stime.wDay,stime.wHour,stime.wMinute,stime.wSecond,Ext);                                   
 return (bool)MoveFile(LogFile,(LPSTR)&NewPath);
}
//------------------------------------------------------------------------------------
void  _cdecl LogToDebug(UINT Type, LPSTR ModuleName, LPSTR ProcName, LPSTR Message, ...)
{
 va_list args;
 BYTE Buffer[1025];     //  > 4k creates __chkstk   // 1024 is MAX for wsprintf

 va_start(args,Message);
 if(MakeLogMsg((LPSTR)&Buffer,Type,ModuleName,ProcName,Message,args) > 0)OutputDebugString((LPSTR)&Buffer);
 va_end(args); 
}
//------------------------------------------------------------------------------------
void  _cdecl LogToProc(UINT Type, LPSTR ModuleName, LPSTR ProcName, LPSTR Message, ...)
{
 if(!MsgProc)return;    // Was set to NULL on purpose
 va_list args;
 BYTE Buffer[1025];     //  > 4k creates __chkstk   // 1024 is MAX for wsprintf

 va_start(args,Message);
 if(MakeLogMsg((LPSTR)&Buffer,Type,ModuleName,ProcName,Message,args) > 0)MsgProc((LPSTR)&Buffer);
 va_end(args); 
}
//------------------------------------------------------------------------------------
void  _cdecl LogToFile(UINT Type, LPSTR ModuleName, LPSTR ProcName, LPSTR Message, ...)
{
 DWORD Result;
 va_list args;
 BYTE Buffer[1025];     //  > 4k creates __chkstk   // 1024 is MAX for wsprintf

 va_start(args,Message);
 if(int len = MakeLogMsg((LPSTR)&Buffer,Type,ModuleName,ProcName,Message,args))
  {
   if(hLogFile == INVALID_HANDLE_VALUE);//OutputDebugString((LPSTR)&Buffer);
     else if(!WriteFile(hLogFile,&Buffer,len,&Result,NULL));//OutputDebugString((LPSTR)&Buffer);
      //  else FlushFileBuffers(hLogFile);   // Slow and mostly useless
  }
 va_end(args); 
}
//------------------------------------------------------------------------------------
void _cdecl LogToNull(UINT Type, LPSTR ModuleName, LPSTR ProcName, LPSTR Message, ...)
{
 // Null :)
}
//------------------------------------------------------------------------------------
void _stdcall LogBinToDebug(PVOID Addr, UINT Size)
{
 PBYTE Ptr = (PBYTE)Addr;
 BYTE Buffer[256];
 for(int len = Size;len > 0;len-=16,Ptr+=16)
  {
   MakeBinaryLine16((LPSTR)&Buffer,Ptr,len);
   OutputDebugString((LPSTR)&Buffer);
  }
}
//------------------------------------------------------------------------------------
void _stdcall LogBinToProc(PVOID Addr, UINT Size)
{
 PBYTE Ptr = (PBYTE)Addr;
 BYTE Buffer[256];
 for(int len = Size;len > 0;len-=16,Ptr+=16)
  {
   MakeBinaryLine16((LPSTR)&Buffer,Ptr,len);
   MsgProc((LPSTR)&Buffer);
  }
}
//------------------------------------------------------------------------------------
void _stdcall LogBinToFile(PVOID Addr, UINT Size)
{
 DWORD Result;
 PBYTE Ptr = (PBYTE)Addr;
 BYTE Buffer[256];
 for(int len = Size;len > 0;len-=16,Ptr+=16)
  {
   int llen = MakeBinaryLine16((LPSTR)&Buffer,Ptr,len);
   if(hLogFile == INVALID_HANDLE_VALUE);//OutputDebugString((LPSTR)&Buffer);
     else if(!WriteFile(hLogFile,&Buffer,llen,&Result,NULL));//OutputDebugString((LPSTR)&Buffer);
      //  else FlushFileBuffers(hLogFile);   // Slow and mostly useless
  }
}
//------------------------------------------------------------------------------------
void _stdcall LogBinToNull(PVOID Addr, UINT Size)
{
 // Null :)
}
//------------------------------------------------------------------------------------
bool _stdcall CreateLogFile(LPSTR LogFile, bool NewFile, LPSTR LogHistoryDir)
{
 if(LogHistoryDir && NewFile)BackupLogFile(LogHistoryDir, LogFile);
 DWORD fattr = ((NewFile)?(CREATE_ALWAYS):(OPEN_ALWAYS));  // TRUNCATE_EXISTING
 hLogFile = CreateFile(LogFile,GENERIC_WRITE,FILE_SHARE_READ|FILE_SHARE_WRITE,NULL,fattr,FILE_ATTRIBUTE_NORMAL|FILE_FLAG_SEQUENTIAL_SCAN,NULL);
 return (hLogFile != INVALID_HANDLE_VALUE);
}
//------------------------------------------------------------------------------------
void _stdcall CloseLogFile(void)
{
 if(hLogFile == INVALID_HANDLE_VALUE)return;
 CloseHandle(hLogFile);
 hLogFile = INVALID_HANDLE_VALUE;
}
//------------------------------------------------------------------------------------
void _stdcall SetLogFile(HANDLE hLog, bool ClosePrev)
{
 if(ClosePrev)CloseLogFile();
 hLogFile = hLog;
}
//------------------------------------------------------------------------------------


//====================================================================================
}

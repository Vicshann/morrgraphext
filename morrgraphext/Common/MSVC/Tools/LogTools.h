// AUTHOR: Vicshann

#pragma once

#ifndef LogToolsH
#define LogToolsH

#include "ToolsBase.h"
#include "FileTools.h"
#include "ConvertTools.h"
//===========================================================================	

#define _LogTextM(mod,msg,...) Logger::LogProc(Logger::lmtNone,mod,__FUNCTION__,msg,__VA_ARGS__)     // Start line
#define _LogTextS(msg,...) Logger::LogProc(Logger::lmtNone,__FILE__,__FUNCTION__,msg,__VA_ARGS__)    // Start line
#define _LogTextE(msg,...) Logger::LogProc(Logger::lmtLine,NULL,NULL,msg,__VA_ARGS__)                // End of line 
#define _LogText(msg,...) Logger::LogProc(Logger::lmtNone,NULL,NULL,msg,__VA_ARGS__)                 // Log block of text

#define _LogLine(msg,...) Logger::LogProc(Logger::lmtLine,__FILE__,__FUNCTION__,msg,__VA_ARGS__)
#define _LogLineM(mod,msg,...) Logger::LogProc(Logger::lmtLine,mod,__FUNCTION__,msg,__VA_ARGS__)

#define _LogInfLine(msg,...) Logger::LogProc(Logger::lmtLine,__FILE__,__FUNCTION__,msg,__VA_ARGS__)
#define _LogErrLine(msg,...) Logger::LogProc(Logger::lmtLine|Logger::lmtError,__FILE__,__FUNCTION__,msg,__VA_ARGS__)
#define _LogWrnLine(msg,...) Logger::LogProc(Logger::lmtLine|Logger::lmtWarning,__FILE__,__FUNCTION__,msg,__VA_ARGS__)    

#define _LogBinary(addr,len) Logger::BinProc(addr,len)
  
//---------------------------------------------------------------------------  

namespace Logger
{
enum ELogMsgType {lmtNone=0,lmtMaskOff=0x00FFFFFF,lmtLine=0x80000000,lmtError=0x40000000,lmtWarning=0x20000000};

typedef void (_stdcall *MSG_LOG_PROC)(LPCTSTR Message);   // Compatible with OutputDebugString
typedef void (_stdcall *LOG_BIN_PROC)(PVOID Addr, UINT Size);
typedef void (_cdecl *LOG_PROC)(UINT Type, LPSTR ModuleName, LPSTR ProcName, LPSTR Message, ...);

extern "C" Logger::LOG_PROC     LogProc;   // Set this to any of LogToXXX functions or your own of LOG_PROC type
extern "C" Logger::MSG_LOG_PROC MsgProc;   // Used for LogToProc
extern "C" Logger::LOG_BIN_PROC BinProc;   // Set this to any of LogBinToXXX functions or your own of LOG_BIN_PROC type
//---------------------------------------------------------------------------

bool  _stdcall BackupLogFile(LPSTR BackupDir, LPSTR LogFile);
int   _stdcall MakeLogMsg(LPSTR OutBuffer, UINT Type, LPSTR ModuleName, LPSTR ProcName, LPSTR Message, va_list arglist);
int   _stdcall MakeBinaryLine16(LPSTR DstLine, PBYTE Addr, UINT Size);

void    _cdecl LogToDebug(UINT Type, LPSTR ModuleName, LPSTR ProcName, LPSTR Message, ...);
void    _cdecl LogToFile(UINT Type, LPSTR ModuleName, LPSTR ProcName, LPSTR Message, ...);
void    _cdecl LogToProc(UINT Type, LPSTR ModuleName, LPSTR ProcName, LPSTR Message, ...);
void    _cdecl LogToNull(UINT Type, LPSTR ModuleName, LPSTR ProcName, LPSTR Message, ...);

void  _stdcall LogBinToDebug(PVOID Addr, UINT Size);
void  _stdcall LogBinToProc(PVOID Addr, UINT Size);
void  _stdcall LogBinToFile(PVOID Addr, UINT Size);
void  _stdcall LogBinToNull(PVOID Addr, UINT Size);

void  _stdcall SetMsgProc(MSG_LOG_PROC Proc);
void _stdcall SetLogFile(HANDLE hLog, bool ClosePrev=false);
bool  _stdcall CreateLogFile(LPSTR LogFile, bool NewFile=true, LPSTR LogHistoryDir=NULL);
void  _stdcall CloseLogFile(void);

}

//===========================================================================	
#endif

// AUTHOR: Vicshann

#include "FileTools.h"

//====================================================================================
void _stdcall CreateDirectoryPath(LPSTR Path)
{
 BYTE FullPath[MAX_PATH];

 lstrcpy((LPSTR)&FullPath, Path);
 for(int Count=0;FullPath[Count] != 0;Count++)
  {
   if((FullPath[Count]==PATHDLML)||(FullPath[Count]==PATHDLMR))
    {
     FullPath[Count] = 0;
     CreateDirectory((LPSTR)&FullPath, NULL); // Faster Always create or Test it`s existance first ?
     FullPath[Count] = PATHDLML;
    } 
  }
}
//---------------------------------------------------------------------------
UINT _stdcall TrimFilePath(LPSTR FullPath)
{
 for(int ctr = lstrlen(FullPath)-1;ctr > 0;ctr--){if(IsFilePathDelim(FullPath[ctr])){FullPath[ctr+1] = 0;return ctr+1;}}  
 return 0;
}
//---------------------------------------------------------------------------
LPSTR _stdcall GetFileName(LPSTR FullPath)
{
 for(int ctr = lstrlen(FullPath)-1;ctr > 0;ctr--){if(IsFilePathDelim(FullPath[ctr]))return (LPSTR)&FullPath[ctr+1];}  
 return FullPath;
}
//---------------------------------------------------------------------------
LPSTR _stdcall GetFileExt(LPSTR FullPath)  // Skips '.'
{ 
 for(int ctr = lstrlen(FullPath)-1;ctr > 0;ctr--){if(FullPath[ctr] == '.')return (LPSTR)&FullPath[ctr+1];}  
 return FullPath;
}
//---------------------------------------------------------------------------
DWORD _stdcall GetFileNameByHandle(HANDLE hFile, LPSTR Name)
{
 PVOID  Proc;
 DWORD  Result;
 PWSTR  NamePtr;
 BYTE   Status[8];
 struct UNI_NAME
  {
   DWORD   Length;
   wchar_t Data[MAX_PATH];
  }String;

 if(Name)Name[0] = 0;
 if(!(Proc = GetProcAddress(GetModuleHandle("ntdll.dll"),"NtQueryInformationFile")))return -1;
 Result = ((DWORD (_stdcall *)(PVOID,PVOID,PVOID,DWORD,DWORD))Proc)(hFile,(PVOID)&Status,(PVOID)&String,sizeof(String),9); // FILE_NAME_INFORMATION  // Only path, no drive letter
 if(Result)return Result;
 Result  = (String.Length/sizeof(wchar_t));
 NamePtr = (PWSTR)&String.Data;
 NamePtr[Result] = 0;
 for(int ctr=Result-1;ctr >= 0;ctr--)if((String.Data[ctr]==PATHDLMR)||(String.Data[ctr]==PATHDLML)){NamePtr = (PWSTR)&String.Data[ctr+1];break;}
 WideCharToMultiByte(CP_ACP,0,NamePtr,-1,Name,MAX_PATH,NULL,NULL);
 return 0;
}
//---------------------------------------------------------------------------
bool _stdcall FindFile(LPSTR FilePath, LPSTR OutBuffer)
{
 int    Index = -1;
 DWORD  Attrs;
 HANDLE hSearch;
 WIN32_FIND_DATA fdat;

 if((hSearch = FindFirstFile(FilePath,&fdat)) == INVALID_HANDLE_VALUE)return false;
 if(!OutBuffer)OutBuffer = FilePath; 
   else lstrcpy(OutBuffer, FilePath);
 FindClose(hSearch);  
 for(int ctr=0;OutBuffer[ctr];ctr++){if((OutBuffer[ctr] == PATHDLML)||(OutBuffer[ctr] == PATHDLMR))Index=ctr;}
 if(Index >= 0)OutBuffer[Index+1] = 0;
 lstrcat(OutBuffer, (LPSTR)&fdat.cFileName); 
 Attrs = GetFileAttributes(OutBuffer);
 if((Attrs != INVALID_FILE_ATTRIBUTES)&&!(Attrs & FILE_ATTRIBUTE_DIRECTORY))return true;  
 return false;
}
//====================================================================================


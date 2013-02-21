// AUTHOR: Vicshann

#include "ProcessTools.h"

//====================================================================================
DWORD _stdcall WriteLocalProtectedMemory(PVOID Address, PVOID Data, DWORD DataSize, bool RestoreProt)
{
 DWORD                    Result;
 DWORD                    Offset;
 DWORD                    PrevProt;
 DWORD                    ProtSize;
 DWORD                    BlockSize;
 PVOID                    CurProtBase;
 MEMORY_BASIC_INFORMATION MemInf;

 Offset      = 0;
 ProtSize    = 0;
 BlockSize   = DataSize;
 while(BlockSize)               // WARNING  BlockSize must be COUNTED TO ZERO !!!
  {
   CurProtBase = &((BYTE*)Address)[Offset];
   if(!VirtualQuery(CurProtBase,&MemInf,sizeof(MEMORY_BASIC_INFORMATION)))break;    // Rounded to begin of page
   if(MemInf.RegionSize > BlockSize)ProtSize = BlockSize;   // No Protection loop
	 else ProtSize = MemInf.RegionSize;
   if((MemInf.Protect==PAGE_READWRITE)||(MemInf.Protect==PAGE_EXECUTE_READWRITE)) // WRITECOPY  changed to READWRITE by writeprocessmemory - DO NOT ALLOW THIS !!!
	 {   
	  MoveMemory(CurProtBase,&((BYTE*)Data)[Offset],ProtSize);    // FastMoveMemory
	  Result = ProtSize;
	 }
	  else
	   {
		if(!VirtualProtect(CurProtBase,ProtSize,PAGE_EXECUTE_READWRITE,&PrevProt))break;   // Allow writing
		MoveMemory(CurProtBase,&((BYTE*)Data)[Offset],ProtSize);     // FastMoveMemory
		Result = ProtSize;
		if(RestoreProt){if(!VirtualProtect(CurProtBase,ProtSize,PrevProt,&PrevProt))break;}  // Restore protection
	   }
   if(Result  != ProtSize)break;
   Offset     += ProtSize;
   BlockSize  -= ProtSize;
  }
 return (DataSize-BlockSize); // Bytes written
}
//===========================================================================	   
DWORD _stdcall WriteProtectedMemory(DWORD ProcessID, HANDLE hProcess, PVOID Address, PVOID Data, DWORD DataSize, bool RestoreProt)
{
 bool                     CurProc;
 bool                     CloseP;
 DWORD                    Result;
 DWORD                    Offset;
 DWORD                    PrevProt;
 DWORD                    ProtSize;
 DWORD                    BlockSize;
 PVOID                    CurProtBase;
 MEMORY_BASIC_INFORMATION MemInf;

 CloseP      = 0;
 Offset      = 0;
 CurProc     = 0;
 ProtSize    = 0;
 BlockSize   = DataSize;
 if(!hProcess){hProcess = OpenProcess(PROCESS_VM_READ|PROCESS_VM_WRITE|PROCESS_VM_OPERATION|PROCESS_QUERY_INFORMATION,false,ProcessID);CloseP=true;}
 if(hProcess == GetCurrentProcess())CurProc = true;
 while(BlockSize)               // WARNING  BlockSize must be COUNTED TO ZERO !!!
  {
   CurProtBase = &((BYTE*)Address)[Offset];
   if(!VirtualQueryEx(hProcess,CurProtBase,&MemInf,sizeof(MEMORY_BASIC_INFORMATION)))break;    // Rounded to begin of page
   if(MemInf.RegionSize > BlockSize)ProtSize = BlockSize;   // No Protection loop
	 else ProtSize = MemInf.RegionSize;
   if((MemInf.Protect==PAGE_READWRITE)||(MemInf.Protect==PAGE_EXECUTE_READWRITE)) // WRITECOPY  changed to READWRITE by writeprocessmemory - DO NOT ALLOW THIS !!!
	 {
	  if(CurProc){MoveMemory(CurProtBase,&((BYTE*)Data)[Offset],ProtSize);Result = ProtSize;}      // FastMoveMemory
		else {if(!WriteProcessMemory(hProcess,CurProtBase,&((BYTE*)Data)[Offset],ProtSize,&Result))break;} // Size of Type VOID do not converted to DWORD - compatible with WIN32 And WIN64
	 }
	  else
	   {
		if(!VirtualProtectEx(hProcess,CurProtBase,ProtSize,PAGE_EXECUTE_READWRITE,&PrevProt))break;   // Allow writing
		if(CurProc){MoveMemory(CurProtBase,&((BYTE*)Data)[Offset],ProtSize);Result = ProtSize;}   // FastMoveMemory
		  else {if(!WriteProcessMemory(hProcess,CurProtBase,&((BYTE*)Data)[Offset],ProtSize,&Result))break;} // Size of Type VOID do not converted to DWORD - compatible with WIN32 And WIN64
		if(RestoreProt){if(!VirtualProtectEx(hProcess,CurProtBase,ProtSize,PrevProt,&PrevProt))break;}  // Restore protection
	   }
   if(Result  != ProtSize)break;
   Offset     += ProtSize;
   BlockSize  -= ProtSize;
  }
 if(CloseP)CloseHandle(hProcess);
 return (DataSize-BlockSize); // Bytes written
}
//===========================================================================	   
PVOID _stdcall FindProcAddress(HMODULE Module, LPSTR ProcName)
{
 return &((BYTE*)Module)[(((DWORD*)FindProcEntry(Module,ProcName))[0])];    
}
//===========================================================================	   
PVOID _stdcall FindProcEntry(HMODULE Module, LPSTR ProcName)
{
 DWORD          Value;
 LPSTR          FuncName;
 DOS_HEADER     *DosHdr;
 WIN_HEADER     *WinHdr;
 EXPORT_DIR     *Export;
 DATA_DIRECTORY *ExportDir;

 DosHdr    = (DOS_HEADER*)Module;
 WinHdr    = (WIN_HEADER*)&((BYTE*)Module)[DosHdr->OffsetHeaderPE];
 ExportDir = &WinHdr->OptionalHeader.DataDirectories.ExportTable;
 Export    = (EXPORT_DIR*)&((BYTE*)Module)[ExportDir->DirectoryRVA];
 
 for(DWORD ctr=0;ctr < Export->NamePointersNumber;ctr++)
  {
   FuncName = (LPSTR)&((BYTE*)Module)[(((DWORD*)&((BYTE*)Module)[Export->NamePointersRVA])[ctr])]; 
   if(lstrcmp(FuncName,ProcName) == 0)
    {     
	 Value = ((WORD*)&((BYTE*)Module)[Export->OrdinalTableRVA])[ctr];
	 return &((DWORD*)&((BYTE*)Module)[Export->AddressTableRVA])[Value];
    }  
  }
 return NULL;
}
//===========================================================================	   
PVOID _stdcall FindLocalModule(PVOID *ModuleBase, DWORD *ModuleSize)
{
 DWORD Type     = NULL;
 DWORD Size     = NULL;
 PVOID Base     = NULL;
 PVOID BaseAddr = (*ModuleBase);
 MEMORY_BASIC_INFORMATION MemInf;

 // Set initial region by allocation
 while(VirtualQuery(BaseAddr,&MemInf,sizeof(MEMORY_BASIC_INFORMATION)))
  {
   BaseAddr = (MemInf.AllocationBase)?(MemInf.AllocationBase):(MemInf.BaseAddress);     // Initial base
   Base     = &((BYTE*)BaseAddr)[MemInf.RegionSize];   // Next region
   Size     = MemInf.RegionSize;
   Type     = MemInf.Type;
   while(VirtualQuery(Base,&MemInf,sizeof(MEMORY_BASIC_INFORMATION)))
	{
	 if(MemInf.AllocationBase != BaseAddr)break;      // End of allocated region
     Base  = &((BYTE*)Base)[MemInf.RegionSize];   
     Size += MemInf.RegionSize;      
     Type  = MemInf.Type;
    }
   if(Type == MEM_IMAGE)
    {
     (*ModuleBase) = Base;  // Set next region base
     if(ModuleSize)(*ModuleSize) = Size;  // Set current module size
     return BaseAddr;    // Return module base
    }
   BaseAddr = &((BYTE*)BaseAddr)[Size]; 
  }
 return NULL;
}
//===========================================================================	   
HMODULE _stdcall GetOwnerModule(PVOID Address)
{
 MEMORY_BASIC_INFORMATION MemInf;

 if(!VirtualQuery(Address,&MemInf,sizeof(MEMORY_BASIC_INFORMATION)))return NULL;
 if(MemInf.Type != MEM_IMAGE)return NULL;
 return (HMODULE)MemInf.AllocationBase;
}
//===========================================================================	   
DWORD _stdcall UpdateProcessImports(PVOID OldProcAddr, PVOID NewProcAddr, BOOL OnlyLocalModules)
{
 PVOID BaseAddr    = NULL;
 PVOID ModuleAddr  = NULL;
 PVOID OwnerModule = NULL;
 PDWORD         Table;
 DOS_HEADER     *DosHdr;
 WIN_HEADER     *WinHdr;
 IMPORT_DESC    *Import;
 DATA_DIRECTORY *ImportDir; 
 BYTE           LocalPath[MAX_PATH];
 BYTE           ModulePath[MAX_PATH];
     
 OwnerModule = (PVOID)GetOwnerModule(&UpdateProcessImports); 
 GetModuleFileName((HMODULE)OwnerModule,(LPSTR)&LocalPath,sizeof(LocalPath));
 LocalPath[CharPosFromRight((LPSTR)&LocalPath, '\\', 0, 0)+1] = 0;
 DBGMSGINF("Current directory: '%s'.",(LPSTR)&LocalPath);

 while((ModuleAddr = FindLocalModule(&BaseAddr, NULL)))
  {  
   GetModuleFileName((HMODULE)ModuleAddr,(LPSTR)&ModulePath,sizeof(ModulePath));
   DBGMSGINF("Found module '%s' at %08X.",(LPSTR)&ModulePath,(DWORD)ModuleAddr); 
   ModulePath[CharPosFromRight((LPSTR)&ModulePath, '\\', 0, 0)+1] = 0;
   if(OnlyLocalModules && (lstrcmpi((LPSTR)&ModulePath,(LPSTR)&LocalPath) != 0))continue;
   if(ModuleAddr == OwnerModule)continue;    // Do not hook this CRKLIB
   if(!IsValidPEHeader(ModuleAddr))continue; // If some strange module
     
   DosHdr    = (DOS_HEADER*)ModuleAddr;
   WinHdr    = (WIN_HEADER*)&((BYTE*)ModuleAddr)[DosHdr->OffsetHeaderPE];
   ImportDir = &WinHdr->OptionalHeader.DataDirectories.ImportTable;
   if(!ImportDir->DirectoryRVA)continue;
   Import    = (IMPORT_DESC*)&((BYTE*)ModuleAddr)[ImportDir->DirectoryRVA];
    
   for(DWORD tctr=0;Import[tctr].AddressTabRVA;tctr++)
    {
     Table = (PDWORD)&((BYTE*)ModuleAddr)[Import[tctr].AddressTabRVA];
     for(DWORD actr=0;Table[actr];actr++)
      {
       if(Table[actr] == (DWORD)OldProcAddr)
        {
         if(WriteLocalProtectedMemory(&Table[actr], &NewProcAddr, 4, TRUE))DBGMSGINF("Import replaced at %08X.",(DWORD)&Table[actr]);          
           else DBGMSGERR("Replacing import at %08X - FAILED!",(DWORD)&Table[actr]); 			  
        } 
      } 
    }
  }
 DBGMSGINF("Finished."); 
 return 0;
}
//===========================================================================
DWORD _stdcall UpdateModuleImports(PVOID OldProcAddr, PVOID NewProcAddr, HMODULE hTgtModule)
{
 PVOID ModuleAddr  = NULL;
 PVOID OwnerModule = NULL;
 PDWORD         Table;
 DOS_HEADER     *DosHdr;
 WIN_HEADER     *WinHdr;
 IMPORT_DESC    *Import;
 DATA_DIRECTORY *ImportDir; 
 BYTE           ModulePath[MAX_PATH];
     
 ModuleAddr  = (PVOID)hTgtModule;    
 OwnerModule = (PVOID)GetOwnerModule(&UpdateProcessImports); 
 GetModuleFileName((HMODULE)ModuleAddr,(LPSTR)&ModulePath,sizeof(ModulePath));
 DBGMSGINF("For module '%s' at %08X.",(LPSTR)&ModulePath,(DWORD)ModuleAddr); 
 if(ModuleAddr == OwnerModule)return 1;    // Do not hook this CRKLIB
 if(!IsValidPEHeader(ModuleAddr))return 2; // If some strange module
     
 DosHdr    = (DOS_HEADER*)ModuleAddr;
 WinHdr    = (WIN_HEADER*)&((BYTE*)ModuleAddr)[DosHdr->OffsetHeaderPE];
 ImportDir = &WinHdr->OptionalHeader.DataDirectories.ImportTable;
 if(!ImportDir->DirectoryRVA)return 3;
 Import    = (IMPORT_DESC*)&((BYTE*)ModuleAddr)[ImportDir->DirectoryRVA];
    
 for(DWORD tctr=0;Import[tctr].AddressTabRVA;tctr++)
  {
   Table = (PDWORD)&((BYTE*)ModuleAddr)[Import[tctr].AddressTabRVA];
   for(DWORD actr=0;Table[actr];actr++)
    {
     if(Table[actr] == (DWORD)OldProcAddr)
      {
       if(WriteLocalProtectedMemory(&Table[actr], &NewProcAddr, 4, TRUE))DBGMSGINF("Import replaced at %08X.",(DWORD)&Table[actr]);          
         else DBGMSGERR("Replacing import at %08X - FAILED!",(DWORD)&Table[actr]); 			  
      } 
    } 
  }
 DBGMSGINF("Finished."); 
 return 0;
}
//===========================================================================	   
BOOL _stdcall SetDbgFlag(BOOL Flag)
{
 BOOL  OldFlag;
 PVOID Address;

 __asm { MOV EAX,DWORD PTR FS:[0x18] 
         MOV Address,EAX }   
 DBGMSGINF("Value of 'FS:[18]' = %08X.",(DWORD)Address);           
 if(Address)
  {
   Address = (PVOID)((DWORD*)Address)[12];
   DBGMSGINF("Value of '(FS:[18])+30' = %08X.",(DWORD)Address);           
   if(Address)
    {
     OldFlag = ((BYTE*)Address)[2];
     ((BYTE*)Address)[2] = Flag;
     DBGMSGINF("Old DBG flag value = %02X.",OldFlag);           
    } 
  }
 return OldFlag;
}
//====================================================================================
long  _stdcall GetProcessPathByID(DWORD ProcessID, LPSTR PathBuffer, long BufferLngth)
{
 PROCESSENTRY32 pent32;
 MODULEENTRY32  ment32;
 HANDLE         hProcessSnap;
 HANDLE         hModulesSnap;

 PathBuffer[0] = 0;
 hProcessSnap  = INVALID_HANDLE_VALUE;
 hModulesSnap  = INVALID_HANDLE_VALUE;
 ment32.dwSize = sizeof(MODULEENTRY32);
 pent32.dwSize = sizeof(PROCESSENTRY32);
 hProcessSnap  = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);
 if(Process32First(hProcessSnap, &pent32))
  {
   do
	{
	 if(pent32.th32ProcessID == ProcessID)
	  {
	   hModulesSnap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, pent32.th32ProcessID);
	   if(Module32First(hModulesSnap, &ment32))
		{
         do
          {
           if(lstrcmpi(pent32.szExeFile, ment32.szModule)==0)
            {
             lstrcpyn(PathBuffer, ment32.szExePath,BufferLngth);
             CloseHandle(hModulesSnap);
             return lstrlen(PathBuffer)+1;  // Excess calculations !!!
            }
          }
		   while(Module32Next(hModulesSnap, &ment32));
		}
       CloseHandle(hModulesSnap);
       break;
	  }
	}
   while(Process32Next(hProcessSnap, &pent32));
  }
 CloseHandle(hProcessSnap);
 return 0;
}
//====================================================================================
long _stdcall GetProcessPathByHandle(HANDLE hProcess, LPSTR PathBuffer, long BufferLngth)
{
 PVOID         GetProcInfo;
 PEBL          ProcBlock;
 PROCPARAMS    ProcPars;
 PROCBASICINFO ProcInfo;
 wchar_t       UImagePath[MAX_PATH];      // Better allocate buffer in heap

 ZeroMemory(&ProcPars,sizeof(ProcPars));
 ZeroMemory(&ProcInfo,sizeof(ProcInfo));
 ZeroMemory(&ProcBlock,sizeof(ProcBlock));
 ZeroMemory(&UImagePath,sizeof(UImagePath));
 if( (GetProcInfo = GetProcAddress(GetModuleHandle("ntdll.dll"),"NtQueryInformationProcess")) &&
     !((DWORD (_stdcall *)(HANDLE,DWORD,PVOID,DWORD,PDWORD))GetProcInfo) (hProcess, 0, &ProcInfo, sizeof(ProcInfo), NULL) &&
     ReadProcessMemory(hProcess, ProcInfo.PebBaseAddress,&ProcBlock,sizeof(ProcBlock),NULL) &&
     ReadProcessMemory(hProcess, ProcBlock.ProcessParameters,&ProcPars,sizeof(ProcPars),NULL) &&
     ReadProcessMemory(hProcess, ProcPars.ApplicationName.Buffer,&UImagePath,ProcPars.ApplicationName.Length,NULL))
           return WideCharToMultiByte(CP_ACP,0,(PWSTR)&UImagePath,-1,PathBuffer,BufferLngth,NULL,NULL);
 return GetProcessPathByID(GetProcessId(hProcess), PathBuffer, BufferLngth);
}
//---------------------------------------------------------------------------
DWORD _stdcall GetRealModuleSize(HANDLE hProcess, DWORD ModuleBase)
{
 MEMORY_BASIC_INFORMATION meminfo;
 DWORD                    ModuleAddr;
 DWORD                    ModuleSize;

 ModuleSize = 0;
 ModuleAddr = ModuleBase;
 ZeroMemory(&meminfo, sizeof(MEMORY_BASIC_INFORMATION));
 while(VirtualQueryEx(hProcess, (LPCVOID)ModuleAddr, &meminfo, sizeof(MEMORY_BASIC_INFORMATION)))
  {
   ModuleAddr += meminfo.RegionSize;
   if((DWORD)meminfo.AllocationBase == ModuleBase)ModuleSize += meminfo.RegionSize;
	 else break;
  }
 return ModuleSize;
}
//---------------------------------------------------------------------------


//====================================================================================


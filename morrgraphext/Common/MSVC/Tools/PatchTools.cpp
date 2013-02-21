// AUTHOR: Vicshann

#include "PatchTools.h"

//====================================================================================
//   Signature Format: '123464??????5636'
//
PVOID _stdcall FindMemSignatureLocal(PVOID Address, LPSTR Signature)
{
 DWORD BlockSize;
 DWORD SigLen;
 PBYTE BytesBuf;
 MEMORY_BASIC_INFORMATION MemInf;

 if((SigLen = lstrlen(Signature)) < 2)return NULL;
 SigLen = (SigLen/2);
 while(VirtualQuery(Address,&MemInf, sizeof(MEMORY_BASIC_INFORMATION)))
  {
   BytesBuf  = (PBYTE)Address;
   Address   = &((BYTE*)Address)[MemInf.RegionSize]; 
   BlockSize = MemInf.RegionSize;
   if((!(MemInf.State & MEM_COMMIT))||(MemInf.State & PAGE_NOACCESS)||(MemInf.State & PAGE_GUARD))continue;  	
   while (BlockSize >= SigLen)
   	{				  	 	 	    
	 if(IsMemSignatureMatch(BytesBuf,Signature,SigLen))return (PVOID)BytesBuf;		
	 BytesBuf++;
	 BlockSize--;
	}
  }
 return NULL;
}
//===========================================================================
// SigLen - number of bytes in signature
bool _stdcall IsMemSignatureMatch(PVOID Address, LPSTR Signature, int SigLen)
{
 BYTE  Value   = NULL;
 long  ValueH  = NULL;
 long  ValueL  = NULL;
 int   Counter = 0;
 int   Offset  = 0; 
 int   SigMult = 1;

 if(!Signature || (SigLen <= 0))return false;
 if('R'==(BYTE)Signature[0]){Signature++;Offset--;SigMult=-1;}
 // Scan by Half byte
 while(Signature[0] && !Value)
  {
   while(Signature[0] == 0x20)Signature++;   // Skip spaces
   if(Signature[0] == '*')
    {
     Signature++;
     Counter = DecStrToDW(Signature);
     while(Signature[0] != '*')Signature++;
     Signature++;
     Offset += (Counter*SigMult);   // Skip N bytes
     continue;
    }
       
   Value  = ((PBYTE)Address)[Offset];
   ValueH = CharToHex(Signature[0]);
   ValueL = CharToHex(Signature[1]);
   if(ValueH < 0)ValueH = (Value >> 4);
   if(ValueL < 0)ValueL = (Value & 0x0F);
   Value  = (BYTE)(((ValueH << 4) | ValueL) ^ Value);
   Signature += 2;
   Offset += SigMult;
  }
 if(!Value && !Signature[0])return true;
 return false;
}
//===========================================================================	   
bool _stdcall PatchBySignature(PBYTE Buffer, UINT BufSize, LPSTR OriginalSig, LPSTR PatchSig, bool PatchAll)
{
 bool Found   = false;
 UINT PSigLen = (lstrlen(PatchSig)/2);
 UINT OSigLen = (lstrlen(OriginalSig)/2);

 for(UINT Offset=0;Offset < BufSize;Offset++)
  {
   PBYTE CurBuf = &Buffer[Offset];
   if(IsMemSignatureMatch(&Buffer[Offset],OriginalSig,OSigLen))
	{
     // --Make memory writable
	 for(UINT ctr=0,inx=0;ctr<PSigLen;ctr++,inx+=2)
	  {
	   long ValueH = CharToHex(PatchSig[inx+0]);
	   long ValueL = CharToHex(PatchSig[inx+1]);
	   if(ValueH < 0)ValueH = (CurBuf[ctr] >> 4);    // Keep original Hi
	   if(ValueL < 0)ValueL = (CurBuf[ctr] & 0x0F);  // Keep original Lo
	   CurBuf[ctr] = (BYTE)((ValueH << 4) | ValueL);
	  }
	  // --Restore memory protection
	 Found = true;
	 if(!PatchAll)break;
	}
  }
 return Found;
}
//---------------------------------------------------------------------------
DWORD _stdcall MemoryPatchLocal(PVOID PatchAddr, PVOID PatchData, DWORD PatchSize, LPSTR Signature)
{
 // Uncomment to take a REVERSE signatures
 //BYTE Buffer[256];
 //wsprintf((LPSTR)&Buffer,"R %08X: %02X%02X%02X%02X%02X%02X%02X%02X%02X%02X",(DWORD)PatchAddr,((PBYTE)PatchAddr)[-1],((PBYTE)PatchAddr)[-2],((PBYTE)PatchAddr)[-3],((PBYTE)PatchAddr)[-4],((PBYTE)PatchAddr)[-5],((PBYTE)PatchAddr)[-6],((PBYTE)PatchAddr)[-7],((PBYTE)PatchAddr)[-8],((PBYTE)PatchAddr)[-9],((PBYTE)PatchAddr)[-10]);
 //OutputDebugString((LPSTR)&Buffer);

 if(IsBadReadPtr(PatchAddr,PatchSize))return 1;
 if(IsBadReadPtr(PatchData,PatchSize))return 2;
 if(Signature && !IsMemSignatureMatch(PatchAddr,Signature,(lstrlen(Signature)/2)))return 3;  //Signature not match
 if(!WriteLocalProtectedMemory(PatchAddr, PatchData, PatchSize, TRUE))return 4;   // Patching failed
 return 0;
}
//---------------------------------------------------------------------------
DWORD _stdcall MemoryPatchMaskLocal(PVOID PatchAddr, LPSTR PatchMask, LPSTR Signature)
{
 DWORD PrevProt;
 PBYTE CurBuf  = (PBYTE)PatchAddr;
 UINT  PSigLen = (lstrlen(PatchMask)/2);

 if(IsBadReadPtr(PatchAddr,PSigLen))return 1;
 if(Signature && !IsMemSignatureMatch(PatchAddr,Signature,(lstrlen(Signature)/2)))return 2;  //Signature not match
 if(!VirtualProtect(PatchAddr,PSigLen,PAGE_EXECUTE_READWRITE,&PrevProt))return 3; 

 for(UINT ctr=0,inx=0;ctr<PSigLen;ctr++,inx+=2)
  {
   long ValueH = CharToHex(PatchMask[inx+0]);
   long ValueL = CharToHex(PatchMask[inx+1]);
   if(ValueH < 0)ValueH = (CurBuf[ctr] >> 4);    // Keep original Hi
   if(ValueL < 0)ValueL = (CurBuf[ctr] & 0x0F);  // Keep original Lo
   CurBuf[ctr] = (BYTE)((ValueH << 4) | ValueL);
  }
 VirtualProtect(PatchAddr,PSigLen,PrevProt,&PrevProt);
 return 0;
}
//---------------------------------------------------------------------------



//====================================================================================


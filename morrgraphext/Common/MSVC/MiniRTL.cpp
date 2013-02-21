
#include "MiniRTL.h"
#include "MemWork.h"
//====================================================================================
extern "C" int _fltused = 0;

extern "C" __declspec(naked) float _cdecl _CIsqrt(float &Value)
{
 __asm FSQRT
 __asm RET
}
//---------------------------------------------------------------------------
extern "C" __declspec(naked) long _cdecl _ftol2(float &Value)
{
 __asm PUSH EAX
 __asm FISTP DWORD PTR [ESP]
 __asm POP  EAX
 __asm RET
}
//---------------------------------------------------------------------------
#ifdef __CRTDECL

void* __CRTDECL operator new(size_t size){return HeapAlloc(GetProcessHeap(),HEAP_ZERO_MEMORY,size);}
//void* __CRTDECL operator new[](size_t width, size_t size){return HeapAlloc(GetProcessHeap(),HEAP_ZERO_MEMORY,(size*width));}
void  __CRTDECL operator delete(void* mem){HeapFree(GetProcessHeap(),0,mem);}
//void  __CRTDECL operator delete[](void* mem){HeapFree(GetProcessHeap(),0,mem);}
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------


extern "C" int __cdecl atexit(void (__cdecl *pf)())
{
 return 0;
}
#endif

#pragma function(memmove,memcpy,memset,memcmp)
 
void*  __cdecl memmove(void* _Dst, const void* _Src, size_t _Size)
{
 return MEMWORK:: mwMoveMemory(_Dst, (PVOID)_Src, _Size); 
}									
void*  __cdecl memcpy(void* _Dst, const void* _Src, size_t _Size)
{
 return MEMWORK::mwCopyMemory(_Dst, (PVOID)_Src, _Size);
} 
void*  __cdecl memset(void* _Dst, int _Val, size_t _Size)
{
 return  MEMWORK::mwFillMemory(_Dst, _Size, _Val);
}
int    __cdecl memcmp(const void* _Buf1, const void* _Buf2,  size_t _Size)
{
 return MEMWORK::mwCompareMemory((PVOID)_Buf1, (PVOID)_Buf2, _Size);
}
//---------------------------------------------------------------------------
// WCRT  -  Win32API CRT   [http://www.ibsensoftware.com/]
//
double __cdecl strtod(const char *s, char **endp)
{
 const unsigned char *p = (unsigned char *)s;
 double res = 0;
 int negative = 0;
 unsigned int val;
 
 while(*p && *p <= 0x20) ++p; // skip initial whitespaces

 if(!p[0]){if (endp) *endp = (char *)p;return 0.0;}

 if((p[0] == '+') || (p[0] == '-')){negative = ((p[0] == '-') ? (1) : (0)); ++p;}   // check sign  

 for( ; (val = p[0] - '0') < 10; ++p) res = 10.0*res + val;  // get non-fractional part    

 if(p[0] == '.')  // get fractional part
  {
   double factor = 0.1;
   for (++p; (val = p[0] - '0') < 10; ++p)
    {
     res += val*factor;
     factor *= 0.1;
    }
  }
  
 if((p[0] == 'e') || (p[0] == 'E'))    // get exponent
  {
   int exponent = 0;
   double factor = 10.0;
   ++p;       
   if ((p[0] == '+') || (p[0] == '-')){if (p[0] == '-') factor = 0.1; ++p;}    // check sign        
   for(; (val = p[0] - '0') < 10; ++p) exponent = 10*exponent + val;  // get exponent value
   for( ; exponent; exponent >>= 1)   // perform exponentiation
    {
     if (exponent & 0x01) res *= factor;
     // TODO: possible overflow?
     factor *= factor;
    }
  }

 if (endp) *endp = (char *)p;
 return negative ? -res : res;
}
//---------------------------------------------------------------------------
double __cdecl atof(const char *s)
{
 return strtod(s, NULL);
}
//---------------------------------------------------------------------------




//====================================================================================


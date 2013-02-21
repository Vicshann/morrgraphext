// AUTHOR: Vicshann

#pragma once

#ifndef ContainersH
#define ContainersH

#include "CommonFuncs.h"
#include "StringTools.h"
#include "LogTools.h"

//---------------------------------------------------------------------------
template <class T> class CLinkedList
{
 struct SEntry
  {
   T  Element;
   SEntry *Prev;
   SEntry *Next;   

   void* operator new(size_t size/*, void *ptr*/){return HeapAlloc(GetProcessHeap(),HEAP_ZERO_MEMORY,size);}  // malloc(size);
   void  operator delete(void* mem){HeapFree(GetProcessHeap(),0,mem);}   // free(mem); // No normal placement delete are possible
//----------------------
  SEntry(void){this->Prev = this->Next = NULL;}
  ~SEntry(){this->Prev = this->Next = NULL;}
  }*List;

 UINT   ECount;
 //HANDLE hHeap;
 static __inline SEntry* EntryFromElement(T* Element){return (SEntry*)Element;}     //{return (SEntry*)(((PBYTE)Element)-(PBYTE)&((SEntry*)(0))->Element);}

public:
 CLinkedList(void){this->ECount=0;this->List=NULL;/*this->hHeap=GetProcessHeap();*/}
 ~CLinkedList(){this->Clear();}
//----------------------
 SEntry* AllocEntry(void){return new /*((void*)this->hHeap)*/ SEntry();}   //(SEntry*)HeapAlloc(this->hHeap,HEAP_ZERO_MEMORY,sizeof(SEntry));}
//----------------------
 void FreeEntry(SEntry* Entry){delete(Entry);}
//----------------------
 UINT Count(void){return this->ECount;}
//----------------------
 void Clear(void){while(this->List && this->Remove(&this->List->Element));}
//----------------------
 T*   GetFirst(void){return (this->List)?(&this->List->Element):(NULL);}          // Don`t do checks?
 T*   GetLast(void){return (this->List)?(&this->List->Prev->Element):(NULL);}     // Don`t do checks?
 //static T*   _GetNext(T* Element){return &this->EntryFromElement(Element)->Next->Element;}   // Works for ANY CLinkedList instance, but no check of end of the list
 //static T*   _GetPrev(T* Element){return &this->EntryFromElement(Element)->Prev->Element;}   // Works for ANY CLinkedList instance, but no check of end of the list
//----------------------
 T*   GetNext(T* Element)            // Check List to NULL or GetHead is enough?
  {
   SEntry* entry = this->EntryFromElement(Element)->Next;  // Last is Tail
   if(entry == this->List)return NULL;    // Head again
   return &entry->Element;
  }
//----------------------
 T*   GetPrev(T* Element)            // Check List to NULL or GetTail is enough?
  {
   SEntry* entry = this->EntryFromElement(Element)->Prev; // Last is Head
   if(entry == this->List->Prev)return NULL;    // Tail again
   return &entry->Element;
  }
//----------------------
 template<typename A> T* Add(A Compatible, PUINT Index=NULL){return this->Insert(Compatible,NULL,Index);}
//----------------------
 bool MoveFirst(UINT Index)
  {
   if(!this->MoveAfter(Index, 0))return false;
   this->List = this->List->Next; // Set new entry as list head	 // The list is cycled
   return true;
  }
//----------------------
 bool MoveAfter(UINT Index, UINT IndexAfter)
  {
   SEntry* After   = NULL;
   SEntry* Current = NULL;

   T* NE1 = this->GetEntry(IndexAfter, &After);
   if(!NE1)return false;
   T* NE2 = this->GetEntry(Index, &Current);
   if(!NE2)return false;

   Current->Prev->Next = Current->Next;
   Current->Next->Prev = Current->Prev;

   Current->Prev = After;
   Current->Next = After->Next;
   After->Next->Prev = Current;
   After->Next   = Current;
   return true;
  }
//----------------------
 T* InsertFirst(PUINT Index=NULL)
  {
   T* NEl = this->Insert(NULL,Index);
   if(NEl)this->List = this->List->Prev; // Set new entry as list head	 // The list is cycled
   return NEl;
  }
//----------------------
 template<typename A> T* InsertFirst(A Compatible, PUINT Index)   // What if need to add a simple structure without an assignment operator defined?
  {
   T* NEl = this->Insert(NULL,Index);
   if(NEl)
	{
	 this->List = this->List->Prev; // Set new entry as list head	 // The list is cycled
	 *NEl = Compatible; // Used assignment operator (copy) - must be defined  //memcpy(NEl,Element,sizeof(T));    // Object copy!!!!!!!!!!!!!  What if the address of a local object? !!!!!
	}
   return NEl;
  }
//----------------------
 template<typename A> T* Insert(A Compatible, T* After, PUINT Index=NULL)
  {
   T* NEl = this->Insert(After,Index);
   if(NEl)*NEl = Compatible; // Used assignment operator (copy) - must be defined  //if(NEl)memcpy(NEl,Element,sizeof(T));    // Object copy!!!!!!!!!!!!!  What if the address of a local object? !!!!!
   return NEl;
  }
//----------------------
 T* InsertBefore(T* Before, PUINT Index=NULL)
  {
  // SEntry * ent1 = this->List;

   T* NEl = this->Insert(this->EntryFromElement(Before)->Prev,Index);
  // SEntry * ent2 = this->List;

   if(this->List == Before)this->List = ((SEntry*)Before)->Prev;
   return NEl;
  }
//----------------------
 T* Insert(T* After, PUINT Index=NULL)
  {
   SEntry *NewEntry = this->AllocEntry();
   if(Index)*Index  = this->ECount;
   if(!this->List)
	{
	 this->List        = NewEntry;
	 this->List->Prev  = NewEntry;   // Set end of list
	 this->List->Next  = NewEntry;
	 this->ECount      = 1;
	}
	 else
	  {
	   SEntry *AfterEn = (After)?(this->EntryFromElement(After)):(this->List->Prev); // After=NULL - Add to end of list
	   AfterEn->Next->Prev = NewEntry;
	   NewEntry->Next  = AfterEn->Next;
	   NewEntry->Prev  = AfterEn;
	   AfterEn->Next   = NewEntry;
	   this->ECount++;
	  }
   return &NewEntry->Element;
  }
//----------------------
 bool Remove(UINT Index){return this->Remove(this->GetEntry(Index));}
//----------------------
 bool Remove(T* Element)
  {
   if(!this->List || !Element)return false;
   SEntry* Removing     = (SEntry*)(((PBYTE)Element)-(PBYTE)&((SEntry*)(0))->Element);
   Removing->Prev->Next = Removing->Next;
   Removing->Next->Prev = Removing->Prev;
   if(Removing == this->List)
	{
	 if(Removing->Next == this->List)this->List = NULL;  // Removing one last entry (head)
	   else this->List = Removing->Next; // Set new head
	}
   this->FreeEntry(Removing);
   this->ECount--;
   return true;
  }
//----------------------
 T* FindEntry(T* Element, PUINT Index=NULL)
  {
   SEntry* CurEn = this->List;
   for(UINT ctr=0;CurEn;ctr++)
	{
	 if(RtlCompareMemory(&CurEn->Element,Element,sizeof(T))==sizeof(T)){if(Index)*Index=ctr;return CurEn->Element;}  // Use operator '=='?
	 CurEn = CurEn->Next;
	 if(CurEn == this->List)break;
	}
   return NULL;
  }
//----------------------
 T* GetEntry(UINT Index)
  {
   SEntry* CurEn = this->List;
   for(UINT ctr=0;CurEn;ctr++)
	{
	 if(ctr == Index)return &CurEn->Element;
	 CurEn = CurEn->Next;
	 if(CurEn == this->List)break;
	}
   return NULL;
  }

};
//===========================================================================================================
class CMiniStr
{
 HANDLE hHeap;
 LPSTR  Data;
 UINT   SLength;     // Not including terminating 0
 UINT   Allocated;

//----------------------
 void Initialize(void)
 {
  this->Data      = NULL;
  this->SLength   = 0;
  this->Allocated = 0;
  this->hHeap     = GetProcessHeap();
 }
//----------------------
public:
 UINT  BufSize(void) const {return this->Allocated;}
//----------------------
 LPSTR ResizeFor(UINT Len)
 {
  if(Len >= this->Allocated)
   {
	this->Allocated = Len + ((this->Allocated)?(this->Allocated):(256));  // Reserve some space, initial prealloc 256
	if(!this->Data)this->Data = (LPSTR)HeapAlloc(this->hHeap,HEAP_ZERO_MEMORY,this->Allocated);
	  else this->Data = (LPSTR)HeapReAlloc(this->hHeap,HEAP_ZERO_MEMORY,this->Data,this->Allocated);
	if(!this->Data)this->Allocated = 0;
   }
  return this->Data;
 }
//----------------------
 LPSTR Assign(const CMiniStr *str)
 {
  if(!str)return NULL;
  if(!this->ResizeFor(str->Length()+1))return NULL;
  memcpy(this->Data,str->c_str(),str->Length()+1);         // Copy string including terminating Null
  this->SLength = str->Length();         // Not include terminating Null
  return this->Data;
 }
//----------------------
 LPSTR Assign(LPSTR str)    // Assign a Null terminated string
 {
  if(!str)return NULL;
  UINT Len = lstrlen(str);        // Not includes terminating Null   // Beware a not Null terminated strings                        
  if(!this->ResizeFor(Len+1))return NULL;     // +1 for terminating Null
  memcpy(this->Data,str,Len+1);             // Copy string including terminating Null
  this->SLength = Len;    
  return this->Data;
 }
//----------------------
 LPSTR Assign(LPSTR str, UINT Len)
 { 
  if(!str)return NULL;                         
  if(!this->ResizeFor(Len+1))return NULL;    // +1 for terminating Null
  memcpy(this->Data,str,Len);          // Copy string, no terminating Null
  this->Data[Len] = 0;          // Add terminating Null
  this->SLength = Len;        // Not include terminating Null
  return this->Data;
 }
//----------------------
 LPSTR Append(const CMiniStr *str)
 {
  if(!str)return NULL;
  if(!this->ResizeFor(str->Length()+this->SLength+1))return NULL;     // +1 for terminating Null
  memcpy(&this->Data[this->SLength],str->c_str(),str->Length()+1);    // Copy string including terminating Null
  this->SLength += str->Length();
  return this->Data;
 }
//----------------------
 LPSTR Append(LPSTR str)
 {
  if(!str)return NULL;
  UINT Len = lstrlen(str);     // Not includes terminating Null   // Beware a not Null terminated strings
  if(!this->ResizeFor(1+Len+this->SLength))return NULL;  // 1 for terminating Null
  memcpy(&this->Data[this->SLength],str,Len+1);           // Copy string including terminating Null
  this->SLength += Len;
  return this->Data;
 }
//----------------------
 LPSTR Append(LPSTR str, UINT Len)
 {
  if(!str)return NULL;
  if(!this->ResizeFor(1+Len+this->SLength))return NULL;   // +1 for terminating Null
  memcpy(&this->Data[this->SLength],str,Len);            // Copy string, no terminating Null
  this->SLength += Len;
  this->Data[this->SLength] = 0;          // Add terminating Null
  return this->Data;
 }
//----------------------
 bool ChangeEncoding(UINT FromCP, UINT ToCP)
 {
  UINT  TmpLen = (this->Length()*sizeof(WCHAR));   // twice as size of current string
  PBYTE TmpBuf = (PBYTE)HeapAlloc(this->hHeap,HEAP_ZERO_MEMORY,(TmpLen*2)+64);
  if(!TmpBuf)return false;
  int wlen = MultiByteToWideChar(FromCP,0,this->Data,this->SLength,(PWSTR)TmpBuf,(TmpLen/2));
  if(wlen <= 0)return false;
  int ulen = WideCharToMultiByte(ToCP,0,(PWSTR)TmpBuf,wlen,(LPSTR)&TmpBuf[TmpLen],TmpLen,NULL,NULL); 
  if(ulen <= 0)return false;
  this->ResizeFor(ulen);
  this->SLength = ulen-1; // Skip Null terminator
  lstrcpy(this->Data,(LPSTR)&TmpBuf[TmpLen]);    
  HeapFree(this->hHeap,0,TmpBuf);
  return true;
 }
//----------------------

public:
 CMiniStr(void){this->Initialize();}
 CMiniStr(LPSTR str){this->Initialize();this->Append(str);}
 //CMiniStr(const char* str){this->Initialize();this->Append((LPSTR)str);}
 //CMiniStr(const unsigned char* str){this->Initialize();this->Append((LPSTR)str);}
 CMiniStr(const CMiniStr &str){this->Initialize();this->Append(&str);}
//----------------------
 ~CMiniStr()
 {
  if(this->Data)HeapFree(this->hHeap,0,Data);
 }
//----------------------
 CMiniStr& Utf8ToAnsi(void)
 {
  this->ChangeEncoding(CP_UTF8, CP_ACP);
  return *this;
 }
//----------------------
 CMiniStr& AnsiToUtf8(void)
 {
  this->ChangeEncoding(CP_ACP, CP_UTF8);
  return *this;
 }
//----------------------
 UINT  Length(void) const {return this->SLength;}
//----------------------
 LPSTR c_str(void) const {if(this->Data)return this->Data;return "";}
//----------------------
 PBYTE c_data(void) const {if(this->SLength)return (PBYTE)this->Data;return NULL;}  // String is empty if SLength=0, but the buffer may contain something
//----------------------
 void  operator = (const CMiniStr &str){this->Assign(&str);}
//----------------------
 void  operator = (LPSTR str){this->Assign(str);}
//----------------------
 CMiniStr& operator += (const CMiniStr &str){this->Append(&str);return *this;}
//----------------------
 CMiniStr& operator += (LPSTR str){this->Append(str);return *this;}

};
//===========================================================================================================
enum SLOpt {soNone=0,soInsCase=1,soNoDups=2};
class CStringMap 
{
public:
 struct SEntry
  {
   CMiniStr key;
   CMiniStr val;   
  };
private:
 CLinkedList<SEntry> map;

public:
 void Clear(void){map.Clear();}
 UINT Count(void){return map.Count();}
 SEntry* First(void){return this->map.GetFirst();}
 SEntry* Next(SEntry* ent){return this->map.GetNext(ent);}
//----------------------
 CMiniStr* FindKey(LPSTR key, SLOpt flags=soNone, SEntry** ent=NULL)       // Returns VALUE string
  {
   UINT keyLen  = lstrlen(key);
   SEntry* cent = ((ent)?(this->map.GetNext(*ent)):(this->map.GetFirst()));
   int (_stdcall *CmpProc)(LPCSTR lpString1, LPCSTR lpString2) = ((flags & soInsCase)?(&lstrcmpi):(&lstrcmp));
   for(;cent && (cent->key.Length() <  keyLen);cent = this->map.GetNext(cent));   // Skip all shorter strings
   for(;cent && (cent->key.Length() == keyLen);cent = this->map.GetNext(cent))    // Serach only among the equals - If sorting is wrong, some keys will never be found!
    {
     int res = CmpProc(key,cent->key.c_str());
     if(res < 0)break;  // Out of sorted order - If sorting is wrong, some keys will never be found!        //_LogTextE("Searching res %i for '%s': %s",res,key,cent->key.c_str());
     if(res == 0){if(ent)*ent = cent;return &cent->val;}
    }
   //_LogTextE("---NOT FOUND");
   return NULL;
  }
//----------------------
 CMiniStr* FindValue(LPSTR value, SLOpt flags=soNone, SEntry** ent=NULL)   // Returns KEY string
  {
   UINT  ValLen = lstrlen(value);
   int (_stdcall *CmpProc)(LPCSTR lpString1, LPCSTR lpString2) = ((flags & soInsCase)?(&lstrcmpi):(&lstrcmp));
   for(SEntry* cent = ((ent)?(this->map.GetNext(*ent)):(this->map.GetFirst()));cent;cent = this->map.GetNext(cent))   // All Strings must be already sorted by length
    {
     if(ValLen != cent->val.Length())continue; 
     int res = CmpProc(value,cent->val.c_str());
     if(res == 0){if(ent)*ent = cent;return &cent->key;}
    }
   return NULL;
  }
//----------------------
 CMiniStr* GetValue(SEntry* key_ent)
  {
   return &key_ent->val;     // No checks needed?
  }
//----------------------
 CMiniStr* GetValue(LPSTR key, SLOpt flags=soNone)     // Returns only first found match
  {
   return this->FindKey(key,flags); 
  }
//----------------------
 void SetValue(SEntry* key_ent, LPSTR Value)
  {
   key_ent->val = Value;
  }
//----------------------
 void SetValue(LPSTR key, LPSTR Value, SLOpt flags=soNone)
  {
   CMiniStr* str = GetValue(key,flags);
   if(str)*str = Value;
  }
//----------------------
 SEntry* AddPair(LPSTR key, LPSTR Value, SLOpt flags=soNone)  // Sort by KEY string length, so search can be made mostly by comparing lengths of two strings  (Shortest at top, longest at bottom)
  {
   UINT keyLen  = lstrlen(key);
   SEntry* cent = this->map.GetFirst();
   SEntry* prev = NULL;
   if(cent && (keyLen >= cent->key.Length()))     // If not less than first
    {
     for(;cent && (keyLen > cent->key.Length());cent = this->map.GetNext(cent))prev = cent;  // Search for a first equal in size string
     if(cent)     // Equal or more is found
      {
       int (_stdcall *CmpProc)(LPCSTR lpString1, LPCSTR lpString2) = ((flags & soInsCase)?(&lstrcmpi):(&lstrcmp));
       for(;cent && (keyLen == cent->key.Length());cent = this->map.GetNext(cent))
        {     
         int res = CmpProc(key,cent->key.c_str());      // Keep alphabetical order among an equal in size strings
         if((res == 0) && (flags & soNoDups))return NULL;   // Already in the list
         if(res < 0)break;   // A position found, insert before
         prev = cent;
        }    
      }
    } 
   cent = ((prev)?(this->map.Insert(prev)):(this->map.InsertFirst()));  // InsertAfter   
   cent->key = key;
   cent->val = Value;
   return cent;
  }
//----------------------
 bool SaveToFile(LPSTR FileName, BYTE Sep)
  {
   HANDLE hFile = CreateFile(FileName,GENERIC_WRITE,FILE_SHARE_READ|FILE_SHARE_WRITE,NULL,CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL|FILE_FLAG_SEQUENTIAL_SCAN,NULL);
   if(hFile == INVALID_HANDLE_VALUE)return false;
   BYTE Separator[] = {Sep,0};
   CMiniStr str;
   for(SEntry* cent = this->map.GetFirst();cent;cent = this->map.GetNext(cent))
    {
     DWORD Result;
     str  = cent->key;
     str += (LPSTR)&Separator;
     str += cent->val;
     str += "\r\n";
     WriteFile(hFile,str.c_str(),str.Length(),&Result,NULL);
    }
   CloseHandle(hFile);  
   return true;
  }
//----------------------
 bool LoadFromFile(LPSTR FileName, BYTE Sep, SLOpt flags=soNone, bool Append=false)
  {   
   HANDLE hFile   = CreateFile(FileName,GENERIC_READ,FILE_SHARE_READ|FILE_SHARE_WRITE,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);
   if(hFile == INVALID_HANDLE_VALUE)return false;
   if(!Append)this->Clear();
   DWORD FSize    = GetFileSize(hFile,NULL);
   LPSTR StrList  = (LPSTR)HeapAlloc(GetProcessHeap(),HEAP_ZERO_MEMORY,FSize+32);  // (LPSTR)malloc(FSize+32);
   ReadFile(hFile,StrList,FSize,&FSize,NULL);
   StrList[FSize] = 0;
   int Next = 0;
   for(int lctr=0;Next >= 0;lctr++,StrList = &StrList[Next])
    {
     int Curr = FindEndOfLine(StrList, &Next);  // \r or \0
     StrList[Curr] = 0;
     if(!StrList[0])continue;  // An empty line!
     int TPos = CharPosFromLeft(StrList, Sep, 0, 0);
     if(TPos < 0){/*LogMsg("Separator is missing in file '%s' for line '%s'", FileName, StrList);*/continue;}    // TODO: Test and remove  -  vicshann
     StrList[TPos] = 0;
     LPSTR KeyStr  = &StrList[0];
     LPSTR ValStr  = &StrList[TPos+1];
     //if(CharPosFromLeft(TopicStr, '\t', 0, 0) >= 0)continue;  // ???
     if(flags & soNoDups){if(this->FindKey(KeyStr,flags)){/*LogMsg("Key '%s' already exist. File '%s'.", KeyStr, FileName);*/continue;}}
     this->AddPair(KeyStr,ValStr);
    }
   HeapFree(GetProcessHeap(),0,StrList);  // free(StrList);  
   CloseHandle(hFile);  
   return true;
  }
//----------------------

};
//===========================================================================================================
#endif

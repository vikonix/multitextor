#if 0
#ifndef __DIR_H__
#define __DIR_H__

#ifndef _WIN32
  #include <dirent.h>
  #include <limits.h>
#else
  #include <windows.h>
#endif
#include <time.h>


#ifndef MAX_PATH
  #define MAX_PATH PATH_MAX
#endif


#define F_ATTR_DIRECTORY 0x1000
#define F_ATTR_READONLY  0x2000
#define F_ATTR_LINK      0x4000

#ifdef _WIN32
  #define F_SLASH  "\\"
#else
  #define F_SLASH  "/"
#endif


/////////////////////////////////////////////////////////////////////////////
struct FInfo {
  int          nFileAttr;
  long long    nSize;
  time_t       time;
  void*        p;
  char         sName[1];
};

struct FileInfo : public FInfo {
  char         sNameExt[MAX_PATH];
};


/////////////////////////////////////////////////////////////////////////////
class SDir
{
  static char           m_SelfPath[];
  static char           m_RunPath[];
  static const char*    m_pName;

  char*                 m_pPath;

#ifdef _WIN32
  HANDLE                m_hFind;
#else
  DIR*                  m_pDir;
#endif
  int                   m_nFound;

public:
  SDir(const char* pPath = NULL);
  ~SDir();

  int                   FindFirst(FileInfo* pInfo);
  int                   FindNext(FileInfo* pInfo);
  int                   FindClose();

  char*                 GetPath() {return m_pPath;}

  static int            SetCurDir(char* pPath);

  static int            SetArg0(char* pArg0, const char* pName = "");
  static const char*    GetSelfPath() {return m_SelfPath;}
  static const char*    GetRunPath()  {return m_RunPath;}
  static const char*    GetCurPath();
  static const char*    GetTmpPath();
  static const char*    GetCfgPath();
  static const char*    GetSysCfgPath();
  static const char*    GetUserName();
  static const char*    FixPath(char* pPath);

  static int            MkDir(const char* pPath, const char* pName, char* pBuff);

  static int            CutMultiMask(char* pPath, char* pMask);
  static int            CutMask(char* pPath, char* pMask);
  static char*          FindLastName(char* pPath);
  static int            CutLastName(char* pPath, char* pName);
  static int            GetNextName(char** pPath, char* pName);
  static int            MakeFullPath(char* pPath, const char* pName, char* pBuff);
  static int            AppendName(char* pPath, const char* pName, char* pBuff);
  static int            CutPath(const char* pPath, char* pBuff, int len);
  static int            CutName(const char* pName, char* pBuff, int len);
  static int            GetRelPath(const char* pPath, char* pBuff, int len);
};


/////////////////////////////////////////////////////////////////////////////
#define MAX_FILES_NUM 0x8000

#define INDEX_NUMBER     256

struct FileIndex {
  struct FileIndex* pNext;
  FileInfo*         dIndex[INDEX_NUMBER];
};


enum SortOrder {
  byLast,   byName,   byExt,   bySize,   byDate,
  byLastIn, byNameIn, byExtIn, bySizeIn, byDateIn //inverce order
};


class FileList
{
  unsigned int  m_nItems;
  FileIndex*    m_pFirstIndex;

private:
  FileInfo**    GetFreeItemPos();
  FileInfo**    GetItemPos(unsigned int ItemNo);

  int           CmpItems(FileInfo* p1, FileInfo* p2, int order);

public:
  FileList();
  ~FileList();

  int           Clear();
  int           GetItemsNumber() {return m_nItems;}

  int           AddItem(FileInfo* pInfo);
  FileInfo*     GetItem(unsigned int ItemNo);

  int           Sort(int order = byLast);

  static int    CmpNames(const char* pName1, const char* pName2);
};


/////////////////////////////////////////////////////////////////////////////
#define MASK_NUMBER 32

class FileMask
{
protected:
  char      m_dMask[MAX_PATH + 1];
  size_t    m_nOffset1[MASK_NUMBER];
  size_t    m_nOffset2[MASK_NUMBER];

protected:
  int   Match(char* pName, char* pMask);

public:
  FileMask();

  int   SetMMask(const char* pMask);

  int   CheckFileByMask(const char* pName);
  int   CheckFileByMask(const char* pName, const char* pMask);
};


class DirList : public FileMask
{
  SDir*         m_pDir;

  FileList*     m_pDrvList;
  FileList*     m_pDirList;
  FileList*     m_pFileList;

  char          m_sPath[MAX_PATH + 1];
  char          m_mMask[MAX_PATH + 1];
  char          m_sMask[MAX_PATH + 1];

private:
  int SetPath(char* pPath = NULL);

public:
  DirList();
  ~DirList();

  FileInfo* GetDrvInfo(unsigned int nItem);
  FileInfo* GetDirInfo(unsigned int nItem);
  FileInfo* GetFileInfo(unsigned int nItem);

  int       ReadDir();
  int       ScanDir();

  size_t    SetFullPath(char* pMask = NULL);

  char*     GetPath()  {return m_sPath;}
  char*     GetMask()  {if(m_sMask[0]) return m_sMask; else return m_mMask;}
  char*     GetMMask() {return m_mMask;}

  int       GetFilesCount(const char* pPath, const char* pMask);
  int       ControlFilesCount(const char* pPath, const char* pMask, int number);
};

#endif //__DIR_H__
#endif
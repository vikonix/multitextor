#pragma once
#include <filesystem>

/////////////////////////////////////////////////////////////////////////////
class Directory
{
    static std::string      m_runPath;
    static std::string      m_name;
    
    std::filesystem::path   m_path;

public:
    Directory(const std::string& );
    ~Directory();

    std::string         GetPath();

    static bool         SetCurDir(const std::string& path);
    static bool         SetProjectName(const std::string& name);
    
    static std::string  GetRunPath() { return m_runPath; }
    static std::string  GetCurPath();
    static std::string  GetTmpPath();
    static std::string  GetCfgPath();
    static std::string  GetSysCfgPath();
    static std::string  GetUserName();
};

#if 0
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

#endif
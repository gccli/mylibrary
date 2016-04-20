// Client7z.cpp

#include "StdAfx.h"

#include <stdio.h>

#include "Common/MyWindows.h"

#include "Common/Defs.h"
#include "Common/MyInitGuid.h"

#include "Common/IntToString.h"
#include "Common/StringConvert.h"

#include "Windows/DLL.h"
#include "Windows/FileDir.h"
#include "Windows/FileFind.h"
#include "Windows/FileName.h"
#include "Windows/NtCheck.h"
#include "Windows/PropVariant.h"
#include "Windows/PropVariantConv.h"

#include "7zip/Common/FileStreams.h"
#include "7zip/Archive/IArchive.h"
#include "7zip/IPassword.h"

#include "libzip.h"

DEFINE_GUID(CLSID_CFormatZIP,
	    0x23170F69, 0x40C1, 0x278A, 0x10, 0x00, 0x00, 0x01, 0x10, 0x01, 0x00, 0x00);
DEFINE_GUID(CLSID_CFormat7z,
            0x23170F69, 0x40C1, 0x278A, 0x10, 0x00, 0x00, 0x01, 0x10, 0x07, 0x00, 0x00);
#define CLSID_Format CLSID_CFormatZIP

Func_CreateObject createObjectFunc = NULL;

using namespace NWindows;
using namespace NFile;
using namespace NDir;

#define kDllName "7z.dll"

static inline AString FStringToCString(const FString &s)
{
  return GetOemString(fs2us(s));
}

static inline FString CStringToFString(const char *s)
{
  return us2fs(GetUnicodeString(s));
}

static inline void PrintString(const UString &s)
{
  printf("%s", (LPCSTR)GetOemString(s));
}

static inline void PrintString(const AString &s)
{
  printf("%s", (LPCSTR)s);
}

static void PrintError(const char *message, const FString &name)
{
  printf("Error: %s", (LPCSTR)message);
  putchar(0x0a);
  PrintString(FStringToCString(name));
  putchar(0x0a);
}

static void PrintError(const AString &s)
{
  putchar(0x0a);
  PrintString(s);
  putchar(0x0a);
}

static HRESULT IsArchiveItemProp(IInArchive *archive, UInt32 index, PROPID propID, bool &result)
{
  NCOM::CPropVariant prop;
  RINOK(archive->GetProperty(index, propID, &prop));
  if (prop.vt == VT_BOOL)
    result = VARIANT_BOOLToBool(prop.boolVal);
  else if (prop.vt == VT_EMPTY)
    result = false;
  else
    return E_FAIL;
  return S_OK;
}

static HRESULT IsArchiveItemFolder(IInArchive *archive, UInt32 index, bool &result)
{
  return IsArchiveItemProp(archive, index, kpidIsDir, result);
}


static const wchar_t *kEmptyFileAlias = L"[Content]";


//////////////////////////////////////////////////////////////
// Archive Open callback class


class CArchiveOpenCallback:
  public IArchiveOpenCallback,
  public ICryptoGetTextPassword,
  public CMyUnknownImp
{
public:
  MY_UNKNOWN_IMP1(ICryptoGetTextPassword)

  STDMETHOD(SetTotal)(const UInt64 *files, const UInt64 *bytes);
  STDMETHOD(SetCompleted)(const UInt64 *files, const UInt64 *bytes);

  STDMETHOD(CryptoGetTextPassword)(BSTR *password);

  UString Password;

  CArchiveOpenCallback() {}
};

STDMETHODIMP CArchiveOpenCallback::SetTotal(const UInt64 * /* files */, const UInt64 * /* bytes */)
{
  return S_OK;
}

STDMETHODIMP CArchiveOpenCallback::SetCompleted(const UInt64 * /* files */, const UInt64 * /* bytes */)
{
  return S_OK;
}

STDMETHODIMP CArchiveOpenCallback::CryptoGetTextPassword(BSTR *password)
{
  return StringToBstr(Password, password);
}


//////////////////////////////////////////////////////////////
// Archive Extracting callback class

static const char *kTestingString    =  "Testing     ";
static const char *kExtractingString =  "Extracting  ";
static const char *kSkippingString   =  "Skipping    ";

static const char *kUnsupportedMethod = "Unsupported Method";
static const char *kCRCFailed = "CRC Failed";
static const char *kDataError = "Data Error";
static const char *kUnavailableData = "Unavailable data";
static const char *kUnexpectedEnd = "Unexpected end of data";
static const char *kDataAfterEnd = "There are some data after the end of the payload data";
static const char *kIsNotArc = "Is not archive";
static const char *kHeadersError = "Headers Error";

class CArchiveExtractCallback:
  public IArchiveExtractCallback,
  public ICryptoGetTextPassword,
  public CMyUnknownImp
{
public:
  MY_UNKNOWN_IMP1(ICryptoGetTextPassword)

  // IProgress
  STDMETHOD(SetTotal)(UInt64 size);
  STDMETHOD(SetCompleted)(const UInt64 *completeValue);

  // IArchiveExtractCallback
  STDMETHOD(GetStream)(UInt32 index, ISequentialOutStream **outStream, Int32 askExtractMode);
  STDMETHOD(PrepareOperation)(Int32 askExtractMode);
  STDMETHOD(SetOperationResult)(Int32 resultEOperationResult);

  // ICryptoGetTextPassword
  STDMETHOD(CryptoGetTextPassword)(BSTR *aPassword);

private:
  CMyComPtr<IInArchive> _archiveHandler;
  FString _directoryPath;  // Output directory
  UString _filePath;       // name inside arcvhive
  FString _diskFilePath;   // full path to file on disk
  bool _extractMode;
  struct CProcessedFileInfo
  {
    FILETIME MTime;
    UInt32 Attrib;
    bool isDir;
    bool AttribDefined;
    bool MTimeDefined;
  } _processedFileInfo;

  COutFileStream *_outFileStreamSpec;
  CMyComPtr<ISequentialOutStream> _outFileStream;

  CObjectVector<NWindows::NFile::NDir::CDelayedSymLink> _delayedSymLinks;

public:
  void Init(IInArchive *archiveHandler, const FString &directoryPath);

  HRESULT SetFinalAttribs();

  UInt64 NumErrors;
  UString Password;

  CArchiveExtractCallback() {}
};

void CArchiveExtractCallback::Init(IInArchive *archiveHandler, const FString &directoryPath)
{
  NumErrors = 0;
  _archiveHandler = archiveHandler;
  _directoryPath = directoryPath;
  NName::NormalizeDirPathPrefix(_directoryPath);
}

STDMETHODIMP CArchiveExtractCallback::SetTotal(UInt64 /* size */)
{
  return S_OK;
}

STDMETHODIMP CArchiveExtractCallback::SetCompleted(const UInt64 * /* completeValue */)
{
  return S_OK;
}

STDMETHODIMP CArchiveExtractCallback::GetStream(UInt32 index,
    ISequentialOutStream **outStream, Int32 askExtractMode)
{
  *outStream = 0;
  _outFileStream.Release();

  {
    // Get Name
    NCOM::CPropVariant prop;
    RINOK(_archiveHandler->GetProperty(index, kpidPath, &prop));

    UString fullPath;
    if (prop.vt == VT_EMPTY)
      fullPath = kEmptyFileAlias;
    else
    {
      if (prop.vt != VT_BSTR)
        return E_FAIL;
      fullPath = prop.bstrVal;
    }
    _filePath = fullPath;
  }

  if (askExtractMode != NArchive::NExtract::NAskMode::kExtract)
    return S_OK;

  {
    // Get Attrib
    NCOM::CPropVariant prop;
    RINOK(_archiveHandler->GetProperty(index, kpidAttrib, &prop));
    if (prop.vt == VT_EMPTY)
    {
      _processedFileInfo.Attrib = 0;
      _processedFileInfo.AttribDefined = false;
    }
    else
    {
      if (prop.vt != VT_UI4)
        return E_FAIL;
      _processedFileInfo.Attrib = prop.ulVal;
      _processedFileInfo.AttribDefined = true;
    }
  }

  RINOK(IsArchiveItemFolder(_archiveHandler, index, _processedFileInfo.isDir));

  {
    // Get Modified Time
    NCOM::CPropVariant prop;
    RINOK(_archiveHandler->GetProperty(index, kpidMTime, &prop));
    _processedFileInfo.MTimeDefined = false;
    switch (prop.vt)
    {
      case VT_EMPTY:
        // _processedFileInfo.MTime = _utcMTimeDefault;
        break;
      case VT_FILETIME:
        _processedFileInfo.MTime = prop.filetime;
        _processedFileInfo.MTimeDefined = true;
        break;
      default:
        return E_FAIL;
    }

  }
  {
    // Get Size
    NCOM::CPropVariant prop;
    RINOK(_archiveHandler->GetProperty(index, kpidSize, &prop));
    UInt64 newFileSize;
    /* bool newFileSizeDefined = */ ConvertPropVariantToUInt64(prop, newFileSize);
  }


  {
    // Create folders for file
    int slashPos = _filePath.ReverseFind_PathSepar();
    if (slashPos >= 0)
      CreateComplexDir(_directoryPath + us2fs(_filePath.Left(slashPos)));
  }

  FString fullProcessedPath = _directoryPath + us2fs(_filePath);
  _diskFilePath = fullProcessedPath;

  if (_processedFileInfo.isDir)
  {
    CreateComplexDir(fullProcessedPath);
  }
  else
  {
    NFind::CFileInfo fi;
    if (fi.Find(fullProcessedPath))
    {
      if (!DeleteFileAlways(fullProcessedPath))
      {
        PrintError("Can not delete output file", fullProcessedPath);
        return E_ABORT;
      }
    }

    _outFileStreamSpec = new COutFileStream;
    CMyComPtr<ISequentialOutStream> outStreamLoc(_outFileStreamSpec);
    if (!_outFileStreamSpec->Open(fullProcessedPath, CREATE_ALWAYS))
    {
      PrintError("Can not open output file", fullProcessedPath);
      return E_ABORT;
    }
    _outFileStream = outStreamLoc;
    *outStream = outStreamLoc.Detach();
  }
  return S_OK;
}

STDMETHODIMP CArchiveExtractCallback::PrepareOperation(Int32 askExtractMode)
{
  _extractMode = false;
  switch (askExtractMode)
  {
    case NArchive::NExtract::NAskMode::kExtract:  _extractMode = true; break;
  };
  switch (askExtractMode)
  {
    case NArchive::NExtract::NAskMode::kExtract:  PrintString(kExtractingString); break;
    case NArchive::NExtract::NAskMode::kTest:  PrintString(kTestingString); break;
    case NArchive::NExtract::NAskMode::kSkip:  PrintString(kSkippingString); break;
  };
  PrintString(_filePath);
  return S_OK;
}

STDMETHODIMP CArchiveExtractCallback::SetOperationResult(Int32 operationResult)
{
  switch (operationResult)
  {
    case NArchive::NExtract::NOperationResult::kOK:
      break;
    default:
    {
      NumErrors++;
      PrintString("  :  ");
      const char *s = NULL;
      switch (operationResult)
      {
        case NArchive::NExtract::NOperationResult::kUnsupportedMethod:
          s = kUnsupportedMethod;
          break;
        case NArchive::NExtract::NOperationResult::kCRCError:
          s = kCRCFailed;
          break;
        case NArchive::NExtract::NOperationResult::kDataError:
          s = kDataError;
          break;
        case NArchive::NExtract::NOperationResult::kUnavailable:
          s = kUnavailableData;
          break;
        case NArchive::NExtract::NOperationResult::kUnexpectedEnd:
          s = kUnexpectedEnd;
          break;
        case NArchive::NExtract::NOperationResult::kDataAfterEnd:
          s = kDataAfterEnd;
          break;
        case NArchive::NExtract::NOperationResult::kIsNotArc:
          s = kIsNotArc;
          break;
        case NArchive::NExtract::NOperationResult::kHeadersError:
          s = kHeadersError;
          break;
      }
      if (s) {
          printf("Error: %s", s);
      } else {
          char temp[16];
          ConvertUInt32ToString(operationResult, temp);
          printf("Error #%s", temp);
      }
    }
  }

  if (_outFileStream)
  {
    if (_processedFileInfo.MTimeDefined)
      _outFileStreamSpec->SetMTime(&_processedFileInfo.MTime);
    RINOK(_outFileStreamSpec->Close());
  }
  _outFileStream.Release();
  if (_extractMode && _processedFileInfo.AttribDefined)
    SetFileAttrib(_diskFilePath, _processedFileInfo.Attrib, &_delayedSymLinks);

  putchar(0x0a);
  return S_OK;
}

HRESULT CArchiveExtractCallback::SetFinalAttribs()
{
  HRESULT result = S_OK;

  for (int i = 0; i != _delayedSymLinks.Size(); ++i)
    if (!_delayedSymLinks[i].Create())
      result = E_FAIL;

  _delayedSymLinks.Clear();

  return result;
}

STDMETHODIMP CArchiveExtractCallback::CryptoGetTextPassword(BSTR *password)
{
  return StringToBstr(Password, password);
}

//////////////////////////////////////////////////////////////
// Archive Creating callback class

struct CDirItem
{
  UInt64 Size;
  FILETIME CTime;
  FILETIME ATime;
  FILETIME MTime;
  UString Name;
  FString FullPath;
  UInt32 Attrib;

  bool isDir() const { return (Attrib & FILE_ATTRIBUTE_DIRECTORY) != 0 ; }
};

class CArchiveUpdateCallback:
  public IArchiveUpdateCallback2,
  public ICryptoGetTextPassword2,
  public CMyUnknownImp
{
public:
  MY_UNKNOWN_IMP2(IArchiveUpdateCallback2, ICryptoGetTextPassword2)

  // IProgress
  STDMETHOD(SetTotal)(UInt64 size);
  STDMETHOD(SetCompleted)(const UInt64 *completeValue);

  // IUpdateCallback2
  STDMETHOD(GetUpdateItemInfo)(UInt32 index,
      Int32 *newData, Int32 *newProperties, UInt32 *indexInArchive);
  STDMETHOD(GetProperty)(UInt32 index, PROPID propID, PROPVARIANT *value);
  STDMETHOD(GetStream)(UInt32 index, ISequentialInStream **inStream);
  STDMETHOD(SetOperationResult)(Int32 operationResult);
  STDMETHOD(GetVolumeSize)(UInt32 index, UInt64 *size);
  STDMETHOD(GetVolumeStream)(UInt32 index, ISequentialOutStream **volumeStream);

  STDMETHOD(CryptoGetTextPassword2)(Int32 *passwordIsDefined, BSTR *password);

public:
  CRecordVector<UInt64> VolumesSizes;
  UString VolName;
  UString VolExt;
  UString Password;

  FString DirPrefix;
  const CObjectVector<CDirItem> *DirItems;

  bool m_NeedBeClosed;

  FStringVector FailedFiles;
  CRecordVector<HRESULT> FailedCodes;

    CArchiveUpdateCallback(): DirItems(0) {};
    CArchiveUpdateCallback(const FString &dir): DirItems(0), DirPrefix(dir) {};

  ~CArchiveUpdateCallback() { Finilize(); }
  HRESULT Finilize();

  void Init(const CObjectVector<CDirItem> *dirItems)
  {
    DirItems = dirItems;
    m_NeedBeClosed = false;
    FailedFiles.Clear();
    FailedCodes.Clear();
  }
};

STDMETHODIMP CArchiveUpdateCallback::SetTotal(UInt64 /* size */)
{
  return S_OK;
}

STDMETHODIMP CArchiveUpdateCallback::SetCompleted(const UInt64 * /* completeValue */)
{
  return S_OK;
}

STDMETHODIMP CArchiveUpdateCallback::GetUpdateItemInfo(UInt32 /* index */,
      Int32 *newData, Int32 *newProperties, UInt32 *indexInArchive)
{
  if (newData)
    *newData = BoolToInt(true);
  if (newProperties)
    *newProperties = BoolToInt(true);
  if (indexInArchive)
    *indexInArchive = (UInt32)(Int32)-1;
  return S_OK;
}

STDMETHODIMP CArchiveUpdateCallback::GetProperty(UInt32 index, PROPID propID, PROPVARIANT *value)
{
  NCOM::CPropVariant prop;

  if (propID == kpidIsAnti)
  {
    prop = false;
    prop.Detach(value);
    return S_OK;
  }

  {
    const CDirItem &dirItem = (*DirItems)[index];
    switch (propID)
    {
      case kpidPath:  prop = dirItem.Name;
          printf("filename: %s\n", (LPCSTR) GetOemString(dirItem.Name));
          break;
      case kpidIsDir:  prop = dirItem.isDir(); break;
      case kpidSize:  prop = dirItem.Size; break;
      case kpidAttrib:  prop = dirItem.Attrib; break;
      case kpidCTime:  prop = dirItem.CTime; break;
      case kpidATime:  prop = dirItem.ATime; break;
      case kpidMTime:  prop = dirItem.MTime; break;
    }
  }
  prop.Detach(value);
  return S_OK;
}

HRESULT CArchiveUpdateCallback::Finilize()
{
  if (m_NeedBeClosed)
  {
      putchar(0x0a);

      m_NeedBeClosed = false;
  }
  return S_OK;
}

static void GetStream2(const wchar_t *name)
{
    PrintString("Compressing  ");
    if (name[0] == 0)
        name = kEmptyFileAlias;
    PrintString(name);
}

STDMETHODIMP CArchiveUpdateCallback::GetStream(UInt32 index, ISequentialInStream **inStream)
{
  RINOK(Finilize());

  const CDirItem &dirItem = (*DirItems)[index];
  GetStream2(dirItem.Name);

  if (dirItem.isDir())
    return S_OK;

  {
    CInFileStream *inStreamSpec = new CInFileStream;
    CMyComPtr<ISequentialInStream> inStreamLoc(inStreamSpec);
    FString path = DirPrefix + dirItem.FullPath;
    if (!inStreamSpec->Open(path))
    {
      DWORD sysError = ::GetLastError();
      FailedCodes.Add(sysError);
      FailedFiles.Add(path);
      // if (systemError == ERROR_SHARING_VIOLATION)
      {
        putchar(0x0a);
        PrintError("WARNING: can't open file", path);
        // PrintString(NError::MyFormatMessageW(systemError));
        return S_FALSE;
      }
      // return sysError;
    }
    *inStream = inStreamLoc.Detach();
  }
  return S_OK;
}

STDMETHODIMP CArchiveUpdateCallback::SetOperationResult(Int32 /* operationResult */)
{
  m_NeedBeClosed = true;
  return S_OK;
}

STDMETHODIMP CArchiveUpdateCallback::GetVolumeSize(UInt32 index, UInt64 *size)
{
  if (VolumesSizes.Size() == 0)
    return S_FALSE;
  if (index >= (UInt32)VolumesSizes.Size())
    index = VolumesSizes.Size() - 1;
  *size = VolumesSizes[index];
  return S_OK;
}

STDMETHODIMP CArchiveUpdateCallback::GetVolumeStream(UInt32 index, ISequentialOutStream **volumeStream)
{
  wchar_t temp[16];
  ConvertUInt32ToString(index + 1, temp);
  UString res = temp;
  while (res.Len() < 2)
    res.InsertAtFront(L'0');
  UString fileName = VolName;
  fileName += L'.';
  fileName += res;
  fileName += VolExt;
  COutFileStream *streamSpec = new COutFileStream;
  CMyComPtr<ISequentialOutStream> streamLoc(streamSpec);
  if (!streamSpec->Create(us2fs(fileName), false))
    return ::GetLastError();
  *volumeStream = streamLoc.Detach();
  return S_OK;
}

STDMETHODIMP CArchiveUpdateCallback::CryptoGetTextPassword2(Int32 *passwordIsDefined, BSTR *password)
{
    return StringToBstr(Password, password);
}

// Main function
static int selector(const struct dirent *unused)
{
  if ((strcmp(unused->d_name, ".") == 0) || (strcmp(unused->d_name, "..") == 0))
    return 0;
  return 1;
}

static int PrepareDirItems(const char *root, const char *relative, CObjectVector<CDirItem> &dirItems)
{
    struct dirent **dp;
    char path[256];
    int n = scandir(root, &dp, selector, alphasort);

    if (n <= 0) {
        if (n == -1) {
            printf("failed to open dir: %s %s\n", root, strerror(errno));
        }
        return n;
    }
    while (n--) {
        if (dp[n]->d_type == DT_DIR) {
            snprintf(path, sizeof(path), "%s/%s", root, dp[n]->d_name);
            if (PrepareDirItems(path, dp[n]->d_name, dirItems) != 0)
                break;

        } else {
            CDirItem di;
            FString name, absolute;
            if (relative && relative[0]) {
                snprintf(path, sizeof(path), "%s/%s", relative, dp[n]->d_name);
                name = CStringToFString(path);
            } else {
                name = CStringToFString(dp[n]->d_name);
            }

            snprintf(path, sizeof(path), "%s/%s", root, dp[n]->d_name);
            absolute = CStringToFString(path);

            NFind::CFileInfo fi;
            if (!fi.Find(absolute)) {
                printf("failed to get file info %s\n", path);
                return 1;
            }

            di.Attrib = fi.Attrib;
            di.Size = fi.Size;
            di.CTime = fi.CTime;
            di.ATime = fi.ATime;
            di.MTime = fi.MTime;
            di.Name = fs2us(name);
            di.FullPath = name;
            dirItems.Add(di);
        }
        free(dp[n]);
    }
    free(dp);
    return 0;
}

extern "C" int CompressDirectory(const char *file, const char *directory)
{
    CObjectVector<CDirItem> dirItems;
    if (PrepareDirItems(directory, "", dirItems) != 0)
        return 1;

    FString archiveName = CStringToFString(file);
    FString dirname = CStringToFString(directory);
    COutFileStream *outFileStreamSpec = new COutFileStream;
    CMyComPtr<IOutStream> outFileStream = outFileStreamSpec;
    if (!outFileStreamSpec->Create(archiveName, false))
    {
        PrintError("can't create archive file");
        return 1;
    }

    CMyComPtr<IOutArchive> outArchive;
    if (createObjectFunc(&CLSID_Format, &IID_IOutArchive, (void **)&outArchive) != S_OK)
    {
      PrintError("Can not get class object");
      return 1;
    }

    if (directory[0] && directory[strlen(directory)-1] != '/')
        dirname += CStringToFString("/");
    CArchiveUpdateCallback *ucb_spec = new CArchiveUpdateCallback(dirname);
    CMyComPtr<IArchiveUpdateCallback2> updateCallback(ucb_spec);
    ucb_spec->Init(&dirItems);

    HRESULT result = outArchive->UpdateItems(outFileStream, dirItems.Size(), updateCallback);

    ucb_spec->Finilize();

    if (result != S_OK)
    {
      PrintError("Update Error");
      return 1;
    }

    FOR_VECTOR (i, ucb_spec->FailedFiles)
    {
      putchar(0x0a);
      PrintError("Error for file", ucb_spec->FailedFiles[i]);
    }

    if (ucb_spec->FailedFiles.Size() != 0)
      return 1;

    return 0;
}

extern "C" int DecompressArchive(const char *zipfile, const char *directory)
{
    CMyComPtr<IInArchive> archive;
    if (createObjectFunc(&CLSID_Format, &IID_IInArchive, (void **)&archive) != S_OK)
    {
        printf("Can not get class object\n");
        return 1;
    }

    CInFileStream *fileSpec = new CInFileStream;
    CMyComPtr<IInStream> file = fileSpec;

    FString archiveName = CStringToFString(zipfile);
    FString outdir = CStringToFString(directory);
    if (!fileSpec->Open(archiveName))
    {
        printf("Can not open archive file %s\n", zipfile);
        return 1;
    }

    {
        CArchiveOpenCallback *openCallbackSpec = new CArchiveOpenCallback;
        CMyComPtr<IArchiveOpenCallback> openCallback(openCallbackSpec);

        const UInt64 scanSize = 1 << 23;
        if (archive->Open(file, &scanSize, openCallback) != S_OK)
        {
            printf("Can not open file as archive %s\n", zipfile);
            return 1;
        }
    }

    CArchiveExtractCallback *extractCallbackSpec = new CArchiveExtractCallback;
    CMyComPtr<IArchiveExtractCallback> extractCallback(extractCallbackSpec);
    extractCallbackSpec->Init(archive, outdir);


    HRESULT result = archive->Extract(NULL, (UInt32)(Int32)(-1), false, extractCallback);
    if (result == S_OK)
        result = extractCallbackSpec->SetFinalAttribs();
    if (result != S_OK)
    {
        printf("Extract Error\n");
        return 1;
    }

    return 0;
}

int ListArchive(const char *zipfile)
{
    CMyComPtr<IInArchive> archive;
    if (createObjectFunc(&CLSID_Format, &IID_IInArchive, (void **)&archive) != S_OK)
    {
        printf("Can not get class object\n");
        return 1;
    }

    CInFileStream *fileSpec = new CInFileStream;
    CMyComPtr<IInStream> file = fileSpec;

    FString archiveName = CStringToFString(zipfile);

    if (!fileSpec->Open(archiveName))
    {
      printf("Can not open archive file %s\n", zipfile);
      return 1;
    }

    {
        CArchiveOpenCallback *openCallbackSpec = new CArchiveOpenCallback;
        CMyComPtr<IArchiveOpenCallback> openCallback(openCallbackSpec);

        const UInt64 scanSize = 1 << 23;
        if (archive->Open(file, &scanSize, openCallback) != S_OK)
        {
            printf("Can not open file as archive %s\n", zipfile);
            return 1;
        }
    }

    UInt32 numItems = 0;
    archive->GetNumberOfItems(&numItems);
    for (UInt32 i = 0; i < numItems; i++)
    {
        {
            // Get uncompressed size of file
            NCOM::CPropVariant prop;
            archive->GetProperty(i, kpidSize, &prop);
            char s[32];
            ConvertPropVariantToShortString(prop, s);
            printf("%s\t", s);
        }
        {
            // Get name of file
            NCOM::CPropVariant prop;
            archive->GetProperty(i, kpidPath, &prop);
            if (prop.vt == VT_BSTR)
                printf("%s", (LPCSTR) GetOemString(prop.bstrVal));
            else if (prop.vt != VT_EMPTY)
                printf("ERROR!");
        }
        printf("\n");
    }

    return 0;
}

int main(int argc, const char *args[])
{
    char c = args[1][0];

    NDLL::CLibrary lib;
    if (!lib.Load(NDLL::GetModuleDirPrefix() + FTEXT(kDllName)))
    {
        printf("Can not load 7-zip library\n");
        return 1;
    }

    createObjectFunc = (Func_CreateObject)lib.GetProc("CreateObject");
    if (!createObjectFunc)
    {
        printf("Can not get CreateObject\n");
        return 1;
    }

    if (c == 'a' && argc == 4) {
        CompressDirectory(args[2], args[3]);
    } else if (c == 'x' && argc == 4) {
        DecompressArchive(args[2], args[3]);
    } else if (c == 'l' && argc == 3) {
        ListArchive(args[2]);
    }

    return 0;
}

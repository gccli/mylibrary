#include <stdint.h>
#include <string.h>
#include <errno.h>
#include <dlfcn.h>

#include "StdAfx.h"
#include "Common/MyInitGuid.h"

#include "7zip/Common/FileStreams.h"
#include "7zip/Archive/IArchive.h"
#include "7zip/IPassword.h"
#include "7zip/MyVersion.h"

// use another CLSIDs, if you want to support other formats (zip, rar, ...).
// {23170F69-40C1-278A-1000-000110070000}
DEFINE_GUID(CLSID_CFormat7z,
  0x23170F69, 0x40C1, 0x278A, 0x10, 0x00, 0x00, 0x01, 0x10, 0x07, 0x00, 0x00);
DEFINE_GUID(CLSID_CFormatZIP,
	    0x23170F69, 0x40C1, 0x278A, 0x10, 0x00, 0x00, 0x01, 0x10, 0x01, 0x00, 0x00);

#include "libp7zip.h"
#include "p7zip_callback.h"
//#include "Common/MyInitGuid.h"

typedef UInt32 (*CreateObjectFunc)(const GUID *clsID, const GUID *interfaceID, void **outObject);

static CreateObjectFunc CreateObject = NULL;
static bool P7ZipInit()
{
	static void *p7zlib = NULL;
	
    if (!p7zlib)
    {
		if ((p7zlib = dlopen("7z.so", RTLD_LAZY)) == NULL) {
		    printf("dlopen %s\n", dlerror());
		    return false;
		}
		CreateObject = (CreateObjectFunc )dlsym(p7zlib, "CreateObject");
		if (CreateObject == NULL) {
		    printf("dlsym %s\n", dlerror());
		    return false;
		}
    }

	return true;
}

static void ListArchives(CMyComPtr<IInArchive> archive)
{
	// List command
	UInt32 numItems = 0;
	archive->GetNumberOfItems(&numItems);
	for (UInt32 i = 0; i < numItems; i++)
	{
	  {
		// Get uncompressed size of file
		NWindows::NCOM::CPropVariant prop;
		archive->GetProperty(i, kpidSize, &prop);
		UString s = ConvertPropVariantToString(prop);
		PrintString(s);
		PrintString("  ");
	  }
	  {
		// Get name of file
		NWindows::NCOM::CPropVariant prop;
		archive->GetProperty(i, kpidPath, &prop);
		UString s = ConvertPropVariantToString(prop);
		PrintString(s);
	  }
	  PrintString("\n");
	}
}


int Compress(const char *outname, const char **filelist)
{    
	P7ZipInit();
	int i;

	UString archiveName = GetUnicodeString(outname);
	CObjectVector<CDirItem> dirItems;
    for (i = 0; filelist[i] != NULL; i++)
    {
      CDirItem di;
      UString name = GetUnicodeString(filelist[i]);
      
      NFile::NFind::CFileInfoW fi;
      if (!fi.Find(name))
      {
        PrintString(UString(L"Can't find file") + name);
        return 1;
      }

      di.Attrib = fi.Attrib;
      di.Size = fi.Size;
      di.CTime = fi.CTime;
      di.ATime = fi.ATime;
      di.MTime = fi.MTime;
      di.Name = name;
      di.FullPath = name;
      dirItems.Add(di);
    }
    COutFileStream *outFileStreamSpec = new COutFileStream;
    CMyComPtr<IOutStream> outFileStream = outFileStreamSpec;
    if (!outFileStreamSpec->Create(archiveName, false))
    {
      PrintError("can't create archive file");
      return 1;
    }

    CMyComPtr<IOutArchive> outArchive;
    if (CreateObject(&CLSID_CFormatZIP, &IID_IOutArchive, (void **)&outArchive) != S_OK)
    {
      PrintError("Can not get class object");
      return 1;
    }

    P7ZipArchiveUpdateCallback *updateCallbackSpec = new P7ZipArchiveUpdateCallback;
    CMyComPtr<IArchiveUpdateCallback2> updateCallback(updateCallbackSpec);
    updateCallbackSpec->Init(&dirItems);
    
    HRESULT result = outArchive->UpdateItems(outFileStream, dirItems.Size(), updateCallback);
    updateCallbackSpec->Finilize();
    if (result != S_OK)
    {
      PrintError("Update Error");
      return 1;
    }
    for (i = 0; i < updateCallbackSpec->FailedFiles.Size(); i++)
    {
      PrintNewLine();
      PrintString((UString)L"Error for file: " + updateCallbackSpec->FailedFiles[i]);
    }
    if (updateCallbackSpec->FailedFiles.Size() != 0)
      return 1;

    return 0;
}

int Decompress(const char *inname, const char *outdir, int flags)
{    
	P7ZipInit();
	CMyComPtr<IInArchive> archive;
    if (CreateObject(&CLSID_CFormatZIP, &IID_IInArchive, (void **)&archive) != S_OK)
    {
      PrintError("Can not get class object");
      return 1;
    }

	UString archiveName = GetUnicodeString(inname);
	UString destdirName = GetUnicodeString(outdir);

	CInFileStream *fileSpec = new CInFileStream;
	CMyComPtr<IInStream> file = fileSpec;

	if (!fileSpec->Open(archiveName))
	{
		PrintError("Can not open archive file");
		return 1;
	}

	P7ZipArchiveOpenCallback *openCallbackSpec = new P7ZipArchiveOpenCallback;
	CMyComPtr<IArchiveOpenCallback> openCallback(openCallbackSpec);
	openCallbackSpec->PasswordIsDefined = false;
	if (archive->Open(file, 0, openCallback) != S_OK)
	{
		PrintError("Can not open archive");
		return 1;
	}



	P7ZipArchiveExtractCallback *extractCallbackSpec = new P7ZipArchiveExtractCallback;
	CMyComPtr<IArchiveExtractCallback> extractCallback(extractCallbackSpec);
	extractCallbackSpec->Init(archive, destdirName); // second parameter is output folder path
	extractCallbackSpec->PasswordIsDefined = false;
	HRESULT result = archive->Extract(NULL, (UInt32)(Int32)(-1), false, extractCallback);
	if (result != S_OK)
	{
		PrintError("Extract Error");
		return 1;
	}

    return 0;
}


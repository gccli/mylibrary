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

#include "p7libzip.h"
#include "p7zip_callback.h"

P7libzip::P7libzip():mLib(NULL){}
P7libzip::~P7libzip()
{
    if(mLib != NULL) {
	dlclose(mLib);
	mLib = NULL;
    }
}

bool P7libzip::Load(const char * filename)
{
    if(NULL == filename)
	return false;
    mLib = dlopen(filename,RTLD_LAZY);
    return mLib!=NULL;
}

FunProc P7libzip::GetFunAddress(const char * funcname)const
{
    void* pfn = dlsym(mLib,funcname);
    return (FunProc)pfn;
}

//////////////////////////////////////////
/////////////// Factory class ////////////


std::auto_ptr<P7ZipFactory> P7ZipFactory::mInstance;
P7ZipFactory* P7ZipFactory::GetInstance()
{
//    P7ZipMutex initMutex(mInitMutex);
    if (mInstance.get() == NULL) 
    {
	mInstance = std::auto_ptr<P7ZipFactory>(new P7ZipFactory()); 
    }

    return mInstance.get();
}

P7ZipFactory::P7ZipFactory()
    :mP7zCreateObj(NULL)
    ,mRarCreateObj(NULL)
{
    char *errstr = NULL;   
            
    if(!mP7zlib.Load("7z.so"))
    {
	errstr = dlerror();
	if(errstr != NULL)
	{
	    Mylog("A dynamic linking error occurred: (%s)", errstr);
	}
	abort();
    }

    if(!mRarlib.Load("Rar29.so"))
    {
	errstr = dlerror();
	if(errstr != NULL)
	{
	    Mylog("A dynamic linking error occurred: (%s)", errstr);
	}
	abort();
    }
    mP7zCreateObj = (CreateObjectFunc)mP7zlib.GetFunAddress("CreateObject");
    mRarCreateObj = (CreateObjectFunc)mRarlib.GetFunAddress("CreateObject");
    if(NULL == mP7zCreateObj || NULL == mRarCreateObj)
    {
	errstr = dlerror();
	if(errstr != NULL)
	{
	    Mylog("A dynamic linking error occurred: (%s)", errstr);
	}
	abort();
    }

    Mylog("%s", __FUNCTION__);
}

P7ZipFactory::~P7ZipFactory()
{
    Mylog("%s", __FUNCTION__);
}


int P7ZDecompress(const char *inname, const char *outdir, int flags)
{    
    P7ZipFactory* p7Factory = P7ZipFactory::GetInstance();

    CMyComPtr<IInArchive> archive;

    if (p7Factory->mP7zCreateObj(&CLSID_CFormatZIP, &IID_IInArchive, (void **)&archive) != S_OK)
    {
	//PrintError("Can not get class object");
      return 1;
    }

    UString archiveName = GetUnicodeString(inname);
    UString destdirName = GetUnicodeString(outdir);

    CInFileStream *fileSpec = new CInFileStream;
    CMyComPtr<IInStream> file = fileSpec;

    if (!fileSpec->Open(archiveName))
    {
	//PrintError("Can not open archive file");
	return 1;
    }

    P7ZipArchiveOpenCallback *openCallbackSpec = new P7ZipArchiveOpenCallback;
    CMyComPtr<IArchiveOpenCallback> openCallback(openCallbackSpec);
    openCallbackSpec->PasswordIsDefined = false;
    if (archive->Open(file, 0, openCallback) != S_OK)
    {
	//PrintError("Can not open archive");
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

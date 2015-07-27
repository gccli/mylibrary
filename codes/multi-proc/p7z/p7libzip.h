#ifndef __P7_LIBZLIB_H__
#define __P7_LIBZLIB_H__

#include "comm.h"
typedef int (*FunProc)();
typedef UInt32 (*CreateObjectFunc)(const GUID *clsID, const GUID *interfaceID, void **outObject);

class P7libzip
{
public:
    P7libzip();
    ~P7libzip();
    bool Load(const char* fileName);
    FunProc GetFunAddress(const char* procName)const;

private:
    void* mLib;
};

class P7ZipFactory
{
public:
    ~P7ZipFactory();
    static P7ZipFactory* GetInstance();
    CreateObjectFunc mP7zCreateObj;
    CreateObjectFunc mRarCreateObj;

private:
    P7ZipFactory();
    static std::auto_ptr<P7ZipFactory> mInstance;

    P7libzip mP7zlib;
    P7libzip mRarlib;
};


#endif

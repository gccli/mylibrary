#ifndef LIBZIP_H__
#define LIBZIP_H__

#ifdef __cplusplus
extern "C" {
#endif

    int CompressDirectory(const char *file, const char *directory);
    int DecompressArchive(const char *zipfile, const char *directory);

#ifdef __cplusplus
}
#endif

#endif

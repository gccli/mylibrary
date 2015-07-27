#ifndef _LIBP7ZIP_WRAPPER_H__
#define _LIBP7ZIP_WRAPPER_H__

extern int Compress(const char *outname, const char **filelist);
extern int Decompress(const char *inname, const char *outdir, int flags);


#endif


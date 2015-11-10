#include "utilfile.h"

#ifdef __cpluplus
extern "C" {
#endif


const char *file_size(const char *file)
{
    static char filesz[64];
    const char *suf[] = {"Bytes", "KB", "MB", "GB", "TB"};
    int i;
    size_t sz, unit = 1024;
    sz = get_file_size(file);

    for(i=0; (sz/unit) && i<5; unit <<= 10, i++)
        ;
    if (i > 0) {
        unit >>= 10;
        snprintf(filesz, sizeof(filesz), "%.2f %s", 1.0*sz/unit, suf[i]);
    } else {
        snprintf(filesz, sizeof(filesz), "%zu Bytes", sz);
    }

    return filesz;
}

#ifdef __cpluplus
}
#endif

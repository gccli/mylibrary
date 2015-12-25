#ifndef PROGRESS_BAR_H__
#define PROGRESS_BAR_H__

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Print @width bytes progress-bar to stdout, only update @resolution times,
 * auto detect if @resolution set to 0
 */
void progressbar(size_t n, size_t total, size_t resolution, size_t width,
                 const char *fmt, ...);


#ifdef __cplusplus
}
#endif


#endif

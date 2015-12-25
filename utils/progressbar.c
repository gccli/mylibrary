#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <signal.h>
#include <sys/ioctl.h>

#include "progressbar.h"

#ifdef __cplusplus
extern "C" {
#endif

// http://en.wikipedia.org/wiki/ANSI_escape_code
#define TCKL "\033[0G\033[0K"

static void terminal_width_get();
static void window_changed(int signo)
{
    terminal_width_get();
}

static size_t terminal_width = 0;
static void terminal_width_get()
{
    struct winsize ws;
    ioctl(0, TIOCGWINSZ, &ws);
    terminal_width = ws.ws_col;
    signal(SIGWINCH, window_changed);
}

void progressbar(size_t n, size_t total, size_t resolution, size_t width,
                 const char *fmt, ...)
{
    if (total < resolution) resolution = total;
    if (resolution == 0) resolution = total/10;
    if (n%(total/resolution)!=0 || n==0) return ;

    size_t len;
    char buffer[256] = {0};
    va_list args;
    va_start(args, fmt);
    len = vsnprintf(buffer, sizeof(buffer), fmt, args);
    len += 32;
    va_end(args);

    if (terminal_width == 0) {
        terminal_width_get();
    }
    if (len + 32 > terminal_width) {
        printf("%s\n", buffer);
        return ;
    }
    if (width + len > terminal_width) {
        width = terminal_width - len - 16;
    }

    // calculuate the ratio of complete-to-incomplete.
    float ratio = n/(float)total;
    size_t    c = ratio * width;

    printf("%s [", buffer);
    for (n=0; n<c; n++)
        printf("=");
    printf(">");
    for (n=c+1; n<width; n++)
        printf(" ");
    printf("] %.2f%% ", ratio*100);
    fflush(stdout);  printf(TCKL"");
}

#ifdef __cplusplus
}
#endif


#ifdef _TEST
int main(int argc, char *argv[])
{
    int i,total=0;
    struct winsize w;
    ioctl(0, TIOCGWINSZ, &w);
    printf ("lines %d columns %d\n",w.ws_row, w.ws_col);



    total = 4*1024*1024;
    total = total + random()%total;
    for (i=0; i<total; i += random()%4096) {
        progressbar(i, total, total/10, 80, "%30s %d", "xsh...", i);
        usleep(500);
    }

    return 0;
}
#endif

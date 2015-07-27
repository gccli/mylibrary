#ifndef GLOBAL_DEFINES_H__
#define GLOBAL_DEFINES_H__

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>

/* TERMINAL DISPLAY CONTROL */

// Colour Control
#define FMTEND        "\033[0m"
#define FMTBLACK      "\033[30m"
#define FMTRED        "\033[31m"
#define FMTGREEN      "\033[32m"
#define FMTYELLOW     "\033[33m"
#define FMTBLUE       "\033[34m"
#define FMTMAGENTA    "\033[35m"
#define FMTCYAN       "\033[36m"
#define FMTWRITE      "\033[37m"

// Clear Screen, and moves cursor to upper left
#define TCCLS         "\033[2J\033[1;1H"

// Moves the cursor to Begining Of Line
#define TCBOL         "\033[0G"

// Kill Line, Moves the cursor to Begining Of Line
#define TCKL          "\033[0G\033[0K"

// Kill Line, Cursor position does not change
#define TCKL0         "\033[0K"
#define TCKL1         "\033[1K"
#define TCKL2         "\033[2K"

/* END TERMINAL DISPLAY CONTROL */

#define     SECOND_OF_DAY   86400
#define     NSEC_PER_SECOND 1000000000

enum
{
    KBytes = 1024,
    MBytes = 1048576,
    GBytes = 1073741824
};

// GET LWP ID
#define gettid() syscall(__NR_gettid)

// Thread Func
typedef void* (*thread_func)(void *);
#endif

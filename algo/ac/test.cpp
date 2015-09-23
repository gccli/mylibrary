#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <stdarg.h>

#include <list>
#include <string>
#include <getopt.h>
#include <sys/resource.h>
#include "acstd.h"

extern "C"
{
#include "utils/utiltime.h"
#include <sfutil/mpse.h>
#include <sfutil/acsmx2.h>

void FatalError(const char *format,...)
{
    va_list ap;
    va_start(ap, format);
    vfprintf(stderr, format, ap);
    va_end(ap);
}
void LogMessage(const char *format,...)
{
    va_list ap;
    va_start(ap, format);
    vfprintf(stderr, format, ap);
    va_end(ap);
}
typedef struct _TestConfig {
    unsigned nocase;
    int verbose;
    int compress;
} TestConfig;

static TestConfig sc;
TestConfig *snort_conf = &sc;
}

#define MAX_KEYWORD 300
typedef std::list<std::string> kset_t;
static kset_t keywords;
static void *engine;
static unsigned eng_method = 1;
static unsigned keyword_max_len = 0;
struct stats {
    long file_count;
    long file_bytes;
    long pattern_count;
    long match_total;
    long match_count[MAX_KEYWORD];
    double t_compile;
    double t_match;
    long vms[2]; // 0:before create engine, 1:after compile engine, 2: usage diff
    long rss[2]; // ditto
    long alloc_total;
} st;

#define trim(start,end) while(end >= start && isspace(*end)) { *end-- = 0; }
static void get_memusage(long *vmsize, long *rss) {
    FILE *fp;
    char *line = NULL;
    size_t len = 0;
    ssize_t sz, i, k;

    char number[16] = {0};
    fp = fopen("/proc/self/status", "r");
    if (fp == NULL) {
        return ;
    }
    while ((sz = getline(&line, &len, fp)) != -1) {
        if (memcmp("VmSize:", line, 7) == 0) {
            if (sc.verbose > 1) printf("%s", line);
            for (k=0, i=7; i<sz; ++i)
                if (isdigit(line[i]))
                    number[k++] = line[i];
            *vmsize = atoi(number);
        } else if (memcmp("VmRSS:", line, 6) == 0) {
            if (sc.verbose > 1) printf("%s", line);
            for (k=0, i=6; i<sz; ++i)
                if (isdigit(line[i]))
                    number[k++] = line[i];
            *vmsize = atoi(number);
            break;
        }
    }

    free(line);
    fclose(fp);
}

static void load_keywords(const char *filename)
{
    FILE *fp;
    char *line, *pend;
    size_t len = 0, cnt = 0;
    ssize_t sz;
    if ((fp = fopen(filename, "rb")) == NULL) {
        fprintf (stderr, "failed to open '%s': %s\n",
                 filename, strerror(errno));
        exit(errno);
    }

    line = NULL;
    while ((sz = getline(&line, &len, fp)) != -1) {
        pend = line+sz-1;
        trim(line, pend);
        if (line >= pend) continue;
        if (++cnt > MAX_KEYWORD) {
            fprintf(stderr, "exceed max keyword size %d\n", MAX_KEYWORD);
            exit(0);
        }
        len = pend - line + 1;
        if (keyword_max_len < len) keyword_max_len = len;

        keywords.push_back(line);
    }
    if (keywords.size() == 0) exit(0);

    free(line);
    fclose(fp);
}

static int callback1 (int id, int index, void *data)
{
    if (sc.verbose > 1) {
        fprintf (stdout, "match %-8d keyword id %d\n", index, id);
    }
    st.match_count[id]++;

    return 0;
}

static int callback (void *id, void *tree,
                     int index, void *data, void *neg_list)
{
    if (sc.verbose > 1) {
        fprintf (stdout, "match %-8d keyword id %ld\n", index, (long)id);
    }
    st.match_count[(long)id]++;

    return 0;
}

static int EngineSearch(const char *text, int len, int *state)
{
    double start;
    int s = 0, c = 0;
    unsigned char *tx = (unsigned char *)text;

    start = timing_start();
    switch(eng_method) {
    case 1:
        c = mp_search((mp_struct_t *)engine, tx, len,  callback1, &s);
        break;
    case MPSE_ACF:
    case MPSE_ACF_Q:
    case MPSE_ACS:
    case MPSE_ACB:
    case MPSE_ACSB:
        c = mpseSearchAll(engine, tx, len, callback, NULL, state);
        break;
    default:
        c = mpseSearch(engine, tx, len, callback, NULL, state);
        break;
    }
    st.t_match += timing_cost(start);
    return c;
}

static void EngineCompile()
{
    long id = 0;
    double start;

    foreach(keywords) {
        start = timing_start();
        if (eng_method == 1) {
            mp_add_pattern((mp_struct_t *)engine, (unsigned char *)ii->c_str(),
                           ii->length(), id);
        } else {
            mpseAddPattern(engine, (void *)ii->c_str(), ii->length(), sc.nocase,
                           0, 0, 0, (void *)id, id);
            // unsigned offset, unsigned depth, unsigned negative
        }

        st.t_compile += timing_cost(start);
        id++;
        if (sc.verbose > 1) {
            printf("add pattern [%s]\n", ii->c_str());
        }
    }

    start = timing_start();
    if (eng_method == 1) {
        mp_compile((mp_struct_t *)engine);
        st.alloc_total = mp_alloc_total;
    } else {
        mpsePrepPatterns(engine, NULL, NULL);
        st.alloc_total = acsm2_total_memory;
        mpsePrintInfo(engine);
    }

    st.t_compile += timing_cost(start);
    st.pattern_count = id;

    usleep(1000);
    get_memusage(&st.vms[1], &st.rss[1]);
}

void *EngineCreate()
{
    get_memusage(&st.vms[0], &st.rss[0]);

    switch(eng_method) {
    case 1:
        engine = calloc(sizeof(mp_struct_t), 1);
        break;
    default:
        engine = mpseNew(eng_method, 0, NULL, NULL, NULL);
        if (sc.compress) {
            mpseSetOpt(engine, 1);
        }

        break;
    }
    if (engine == NULL) {
        printf("can not create match engine, maybe method incorrectly\n");
        exit(1);
    }

    return engine;
}

void EngineFree()
{
    switch(eng_method) {
    case 1:
        free(engine);
        break;
    default:
        mpseFree(engine);

        break;
    }
}

static char buffer[8192];
int ScanFile(const char *filename)
{
    FILE *fp;
    size_t len, filesz = 0;

    int state = 0, c = 0;
    if ((fp = fopen(filename, "rb")) == NULL) {
        fprintf (stderr, "failed to open '%s': %s\n",
                 filename, strerror(errno));
        exit(errno);
    }

    while (!feof(fp)) {
        len = fread(buffer, 1, sizeof(buffer), fp);
        c += EngineSearch(buffer, len, &state);

        filesz += len;
    }

    fclose(fp);

    st.file_count++;
    st.file_bytes += filesz;
    st.match_total += c;

    return 0;
}

int ScanDir(const char *dirp)
{
    struct dirent **namelist;
    int n;
    char path[1024];
    n = scandir(dirp, &namelist, 0, alphasort);
    if (n < 0) {
	printf("failed to scandir '%s'\n", dirp);
        exit(1);
    }

    while (n--) {
	snprintf(path, sizeof(path), "%s/%s", dirp, namelist[n]->d_name);
	if (namelist[n]->d_type == DT_REG) {
            if (sc.verbose > 1) printf("%s\n", path);
            ScanFile(path);
	}
	free(namelist[n]);
    }
    free(namelist);
    return 0;
}

const char *methods[] = {
    "",
    "AC_STD, implemented by me (default)",
    "",
    "",
    "LOWMEM, Basic Keyword Search Trie - uses linked lists to build the finite automat",
    "",
    "AC_FULL, acsmx2.c, Full matrix",
    "AC_SPARSE, acsmx2.c, Sparse matrix",
    "AC_BANDED, acsmx2.c, Banded matrix",
    "AC_SPARSEBANDS, acsmx2.c, Sparse-Banded matrix",
    "AC_BNFA, bnfa_search.c, Basic NFA based AC, compacted sparse array storage",
    "AC_BNFA_Q, bnfa_search.c, Basic NFA based AC, compacted sparse array storage",
    "AC_FULLQ, acsmx2.c, matching states are queued",
    NULL
};
void usage()
{
    fprintf (stderr, "Usage: METHOD=method ./a.out keyword_file dir\n");
    fprintf (stderr, "Valid match method as follow:\n");
    fprintf (stderr, " 1:  %s\n", methods[1]);
    fprintf (stderr, " 4:  %s\n", methods[4]);
    fprintf (stderr, " 6:  %s\n", methods[6]);
    fprintf (stderr, " 7:  %s\n", methods[7]);
    fprintf (stderr, " 8:  %s\n", methods[8]);
    fprintf (stderr, " 9:  %s\n", methods[9]);
    fprintf (stderr, " 10: %s\n", methods[10]);
    fprintf (stderr, " 11: %s\n", methods[11]);
    fprintf (stderr, " 12: %s\n", methods[12]);
    fprintf (stderr, "\nExample:\n");
    fprintf (stderr, "  METHOD=10 ./a.out /tmp/keyword /tmp/data\n");
    exit(0);
}
int main(int argc, char *argv[])
{
    int i = 0, opt;
    char *e;
    memset(&st, 0, sizeof(st));
    memset(&sc, 0, sizeof(sc));

    if (argc < 3) usage();
    while ((opt = getopt(argc, argv, "vncm:")) != -1) {
        switch (opt) {
        case 'n':
            sc.nocase = 1;
            break;
        case 'c':
            sc.compress = 1;
            break;
        case 'm':
            eng_method = atoi(optarg);
            break;
        case 'v':
            sc.verbose++;
            break;
        default:
            usage();
            break;
        }
    }

    load_keywords(argv[optind]);
    if ((e = getenv("METHOD"))) eng_method = atoi(e);

    engine = EngineCreate();
    EngineCompile();
    ScanDir(argv[optind+1]);

    printf("--------------------------------\n");
    printf("Method          : %s\n", methods[eng_method]);
    printf("pattern count   : %ld\n", st.pattern_count);
    printf("file count      : %ld\n", st.file_count);
    printf("file bytes      : %ld kB\n", st.file_bytes/1024);
    printf("Compile time(s) : \x1b[33m%lf\x1b[0m\n", st.t_compile);

    printf("\x1b[1m\x1b[31mMatch time(s)   : %lf\x1b[0m\n", st.t_match);
    printf("\x1b[1m\x1b[32mMemory(kB)      : %.2f\x1b[0m\n", (float)st.alloc_total/1024);
    printf("VmSize(kB)      : %ld\n", st.vms[1]-st.vms[0]);
    printf("Match total     : %ld\n", st.match_total);
    if (sc.verbose) {
        printf("Match count for each keyword :\n");
        i=0;
        foreach(keywords) {
            printf("  %-*s\t%ld\t\t",
                   keyword_max_len, ii->c_str(), st.match_count[i++]);
            if (i%2 == 0) printf("\n");
        }
        printf("\n");
    }

    EngineFree();
    return 0;
}

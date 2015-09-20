#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <stdarg.h>

#include <list>
#include <string>
#include "mpse.h"

//#include <dynamic_preproc/str_search.h>


extern "C"
{
#include "../../../utils/utiltime.h"
#include <dynamic_preproc/str_search.h>
//#include "acsmx.h"

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
    int verbose = 0;
} TestConfig;

static TestConfig sc;
TestConfig *snort_conf = &sc;
}

#define MAX_KEYWORD    100
typedef std::list<std::string> kset_t;
static kset_t keywords;
static void *engine;
static unsigned eng_method = 1;

struct stats {
    long file_count;
    long file_bytes;
    long pattern_count;
    long match_total;
    long match_count[MAX_KEYWORD];
    double t_compile;
    double t_match;
} st;

#define trim(start,end) while(end >= start && isspace(*end)) { *end-- = 0; }

static void load_keywords(const char *filename)
{
    FILE *fp;
    char *line, *pend;
    size_t len = 0;
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
        keywords.push_back(line);
    }
    if (keywords.size() == 0) exit(0);

    free(line);
    fclose(fp);
}

static char *load_file(const char *filename, int *n)
{
    FILE *fp;
    char *buff;
    if ((fp = fopen(filename, "rb")) == NULL) {
        fprintf (stderr, "failed to open '%s': %s\n",
                 filename, strerror(errno));
        exit(errno);
    }
    fseek (fp, 0L, SEEK_END);
    *n = ftell(fp);
    rewind(fp);
    buff = (char *)calloc(*n, 1);
    if (fread(buff, 1, *n, fp) != (size_t) *n) {
        printf("failed to read file %s\n", filename);
        exit(1);
    }
    fclose(fp);

    return buff;
}

static int callback1 (int id, int index, void *data)
{
//    fprintf (stdout, "match %-8d keyword id %d\n", index, id);
    st.match_count[id]++;

    return 0;
}

static int callback (void *id, void *tree,
                     int index, void *data, void *neg_list)
{
//    fprintf (stdout, "match %-8d keyword id %ld\n", index, (long)id);
    st.match_count[(long)id]++;

    return 0;
}

static int EngineSearch(const char *text, int len)
{
    double start = timing_start();
    int s = 0,c;
    if (eng_method == 1) {
    c = mp_search((mp_struct_t *)engine, (unsigned char *)text, len,
                  callback1, &s);
    } else {
        c = SearchInstanceFindString(engine, text, len, 0, callback);
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
            SearchInstanceAddEx(engine, ii->c_str(), ii->length(), (void *)id, sc.nocase);
        }

        st.t_compile += timing_cost(start);
        id++;
    }

    start = timing_start();
    if (eng_method == 1) {
        mp_compile((mp_struct_t *)engine);
    } else {
        SearchInstancePrepPatterns(engine);
    }
    st.t_compile += timing_cost(start);
    st.pattern_count = id;
}

void *EngineCreate()
{
    switch(eng_method) {
    case 1:
        engine = calloc(sizeof(mp_struct_t), 1);
        break;
    default:
        engine = SearchInstanceNewEx(eng_method);
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
        break;
    default:
        SearchInstanceFree(engine);
        break;
    }
}

int ScanDir(const char *dirp)
{
    struct dirent **namelist;
    int n, c, len;
    char path[1024], *text;
    n = scandir(dirp, &namelist, 0, alphasort);
    if (n < 0) {
	printf("failed to scandir '%s'\n", dirp);
        exit(1);
    }

    while (n--) {
	snprintf(path, sizeof(path), "%s/%s", dirp, namelist[n]->d_name);
	if (namelist[n]->d_type == DT_REG) {
            text = load_file(path, &len);
            c = EngineSearch(text, len);

            st.file_count++;
            st.file_bytes += len;
            st.match_total += c;
            free(text);
	}
	free(namelist[n]);
    }
    free(namelist);
    return 0;
}

const char *methods[] = {
    "",
    "AC, implemented by me (default)",
    "",
    "",
    "LOWMEM, Basic Keyword Search Trie - uses linked lists to build the finite automat",
    "",
    "ACF_FULL, Full matrix",
    "ACF_SPARSE, Sparse matrix",
    "ACF_BANDED, Banded matrix",
    "ACF_SPARSEBANDS, Sparse-Banded matrix",
    "AC_BNFA, ",
    "AC_BNFA_Q",
    NULL
};

int main(int argc, char *argv[])
{
    int i = 0;
    char *e;
    memset(&st, 0, sizeof(st));
    sc.nocase = 1;
    sc.verbose= 1;
    if (argc < 3) {
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
        fprintf (stderr, "\nExample:\n");
        fprintf (stderr, "  METHOD=10 ./a.out keyword /usr/include\n");
        return 1;
    }
    load_keywords(argv[1]);

    if ((e = getenv("METHOD"))) eng_method = atoi(e);

    engine = EngineCreate();
    EngineCompile();
    ScanDir(argv[2]);

    printf("\n--------------------------------\n");
    printf("Method        : %s\n", methods[eng_method]);
    printf("pattern count : %ld\n", st.pattern_count);
    printf("file count    : %ld\n", st.file_count);
    printf("file bytes    : %ld\n", st.file_bytes);
    printf("compile time  : %lf(s)\n", st.t_compile);
    printf("\x1b[1m\x1b[31mmatch time    : %lf(s)\x1b[0m\n", st.t_match);
    printf("match total   : %ld\n", st.match_total);
    if (sc.verbose) {
    printf("match count for each keyword :\n");
    foreach(keywords) {
        printf("  %-20s    %-4ld\n", ii->c_str(), st.match_count[i++]);
    }
    }

    EngineFree();
    return 0;
}

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>

#include <list>
#include <string>
#include <typeinfo>

extern "C"
{
#include "../../../utils/utiltime.h"
#include "acsmx.h"
}

using namespace std;

#define foreach(c) for(decltype(c)::iterator ii = c.begin(); ii != c.end(); ++ii)
#define MAX_KEYWORD    100

typedef std::list<string>    kset_t;
static kset_t keywords;
static ACSM_STRUCT *engine;
static int nocase = 0;

struct stats {
    long file_count;
    long file_bytes;
    long pattern_count;
    long match_total;
    long match_count[MAX_KEYWORD];
    double t_compile;
    double t_match;
} st;

#define trim(end) while(*end && isspace(*end)) { *end-- = 0; }

static void load_keywords(const char *filename)
{
    FILE *fp;
    char *line, *pend;
    size_t len = 0;
    ssize_t sz;
    if ((fp = fopen(filename, "rb")) == NULL) {
        fprintf (stderr, "failed to open '%s': %s\n", filename, strerror(errno));
        exit(errno);
    }

    while ((sz = getline(&line, &len, fp)) != -1) {
        pend = line+sz-1;
        trim(pend);
        if (line > pend) continue;
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
        fprintf (stderr, "failed to open '%s': %s\n", filename, strerror(errno));
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

    return buff;
}

static int callback (void *id, void *tree, int index, void *data, void *neg_list)
{
    fprintf (stdout, "match %-8d keyword id %ld\n", index, (long)id);
    st.match_count[(long)id]++;

    return 0;
}

static int pattern_search(const char *text, int len)
{
    double start = timing_start();
    int s = 0;
    int c = acsmSearch(engine, (unsigned char *)text, len,
                       callback, NULL, &s);

    st.t_match += timing_cost(start);
    return c;
}

static void pattern_compile()
{
    long id = 0;
    double start;

    foreach(keywords) {
        printf("add keyword [%s]\n", ii->c_str());
        start = timing_start();
        acsmAddPattern (engine, (unsigned char *)ii->c_str(),
                        ii->length(), nocase,
                        0, 0, 0, (void *)id, id);
        st.t_compile += timing_cost(start);
        id++;
    }

    start = timing_start();
    acsmCompile(engine, NULL, NULL);
    st.t_compile += timing_cost(start);
    st.pattern_count = id;
}


int myscandir(const char *dirp)
{
    struct dirent **namelist;
    int n, c, len;
    char path[1024], *text;
    n = scandir(dirp, &namelist, 0, alphasort);
    if (n < 0) {
	printf("failed to scandir '%s'\n", dirp);
	return -1;
    }

    while (n--) {
	snprintf(path, sizeof(path), "%s/%s", dirp, namelist[n]->d_name);
	if (namelist[n]->d_type == DT_DIR) {
	    if (namelist[n]->d_name[0] == '.') continue;
	    printf("%s\n", path);
	    myscandir(path);
	} else if (namelist[n]->d_type == DT_REG) {
            text = load_file(path, &len);
            c = pattern_search(text, len);
	    //printf("%3d match:  %s\n", c, path);
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



/**
 * Usage: ./a.out he she his hers
 * Compile: g++ -g -Wall -std=c++0x main.cpp
 */
int main(int argc, char *argv[])
{
    int i=0;
    memset(&st, 0, sizeof(st));

    if (argc < 3) {
        fprintf (stderr, "Usage: ./a.out keyword_file dir [options]\n");
        return 1;
    }
    load_keywords(argv[1]);

    engine = acsmNew(NULL, NULL, NULL);
    pattern_compile();
    myscandir(argv[2]);

    printf("pattern count : %ld\n", st.pattern_count);
    printf("file count    : %ld\n", st.file_count);
    printf("file bytes    : %ld\n", st.file_bytes);
    printf("compile time  : %lf(s)\n", st.t_compile);
    printf("match time    : %lf(s)\n", st.t_match);
    printf("match total   : %ld\n", st.match_total);
    printf("match count for each keyword :\n");
    foreach(keywords) {
        printf("  %-20s    %-4ld\n", ii->c_str(), st.match_count[i++]);
    }

    acsmFree (engine);
    return 0;
}

#include <sys/types.h>
#include <regex.h>
#include <string>
#include <dirent.h>
#include <assert.h>
#include <boost/regex.hpp>
#include "utiltime.h"
using namespace std;


#define FUNC_COUNT 3
bool  debug = false;
FILE *fpout = NULL;

struct regex_stat {
    size_t n;
    size_t match;
    double timecost;
} statistics;


typedef void (*SCANFILE_FUNC)(const char*); 

void scanfile_0(const char *filename)
{
    int err;
    FILE *fp;
    char *line = NULL;
    size_t len = 0;
    ssize_t read;

    fp = fopen(filename, "r");
    if (fp == NULL) {
	exit(EXIT_FAILURE);
    }

    const char *useragent_pattern = "^User-Agent:\\s*([^\r\n]*)";
    regex_t pattern;
    if ((err = regcomp(&pattern, useragent_pattern, REG_EXTENDED)) != 0) {
	char error[256] = {0};
	regerror(err, &pattern, error, sizeof(error));
	printf("compiling regex - %s\n", error);
	exit(EXIT_FAILURE);
    }

    string useragent;

    while ((read = getline(&line, &len, fp)) != -1) {
	if (read < 1) {
	    continue;
	}

	regmatch_t match[2];
	size_t nmatch = 2;
	err = regexec(&pattern, line, nmatch, match, 0);
	if (err == 0) {
	    size_t k;
	    statistics.match++;
	    for(k=0; k<nmatch && match[k].rm_so != -1; ++k) {
		if (k == 1) {
		    line[match[k].rm_eo]=0;
		    useragent.assign(line+match[k].rm_so, match[k].rm_eo-match[k].rm_so);
		}
	    }
	    assert (k == 2);
	}
    }

    regfree(&pattern);

    free(line);
    fclose(fp);
    if (useragent.length() > 0) {
	if(debug)printf("%s %s - %s\n", __FUNCTION__, filename, useragent.c_str());
    }
}

void scanfile_1(const char *filename)
{
    int err;

    FILE *fp;
    char *line = NULL;
    size_t len = 0;
    ssize_t read;

    fp = fopen(filename, "r");
    if (fp == NULL) {
	exit(EXIT_FAILURE);
    }

    const char *useragent_pattern = "^User-Agent:[^\r\n]*";
    string useragent;
    regex_t pattern;
    if ((err = regcomp(&pattern, useragent_pattern, REG_EXTENDED)) != 0) {
	char error[256] = {0};
	regerror(err, &pattern, error, sizeof(error));
	printf("compiling regex - %s\n", error);
	exit(EXIT_FAILURE);
    }

    while ((read = getline(&line, &len, fp)) != -1) {
	if (read < 1) {
	    continue;
	}

	regmatch_t match[2];
	size_t nmatch = 2;
	err = regexec(&pattern, line, nmatch, match, 0);
	if (err == 0) {
	    statistics.match++;
	    useragent.assign(line, match[0].rm_eo);
	}

    }
    regfree(&pattern);
    free(line);
    fclose(fp);
    if (useragent.length() > 0) {
	string::size_type n;
	if ((n = useragent.find(": ")) != string::npos) {
	    useragent = useragent.substr(n+2, string::npos);
	} else if((n = useragent.find(":")) != string::npos) {
	    useragent = useragent.substr(n+1, string::npos);
	} 
	if(debug)printf("%s %s - %s\n", __FUNCTION__, filename, useragent.c_str());
    }
}

void scanfile_2(const char *filename)
{
    FILE *fp;
    char *line = NULL;
    size_t len = 0;
    ssize_t read;

    fp = fopen(filename, "r");
    if (fp == NULL) {
	exit(EXIT_FAILURE);
    }

    const char *useragent_pattern = "^User-Agent:\\s*([^\r\n]*)";
    boost::regex pattern(useragent_pattern);

    string useragent;
    while ((read = getline(&line, &len, fp)) != -1) {
	if (read < 1) {
	    continue;
	}

	boost::smatch what;
	if (regex_search(string(line), what, pattern)) {
	    statistics.match++;
	    useragent = std::string(what[1].first, what[1].second);
	}
    }

    free(line);
    fclose(fp);
    if (useragent.length() > 0) {
	if(debug)printf("%s %s - %s\n", __FUNCTION__, filename, useragent.c_str());
    }
}

void scandir(const char *dir, SCANFILE_FUNC pscan)
{
    struct dirent **namelist;
    int n;
    char path[128] = {0};

    memset(&statistics, 0, sizeof(statistics));

    n = scandir(dir, &namelist, NULL, alphasort);
    if (n < 0) {
	perror("scandir");
	exit(EXIT_FAILURE);
    }
    statistics.n = n;
    while (n--) {
	if (namelist[n]->d_type != DT_REG) {
	    free(namelist[n]);
	    continue;
	}
	snprintf(path, sizeof(path), "%s/%s", dir, namelist[n]->d_name);
	pscan(path);
	free(namelist[n]);
    }
    free(namelist);

}

int main(int argc, char *argv[])
{
    double start = 0.0;
    int select = 0;

    if (argc < 2) {
	printf("Usage: %s directory [ select=0|1|2 ] [debug=0|1] \n", argv[0]);
	return 1;
    }
    
    char dir[128] = {0};
    strcpy(dir, argv[1]);

    if (argc >= 3) {
	select = atoi(argv[2]);
	if (argc > 3) {
	    debug = true;
	}
    }


    SCANFILE_FUNC scanfile[FUNC_COUNT] = {scanfile_0, scanfile_1, scanfile_2};
    fpout = fopen("/root/report.csv", "w");
    for(int i=0; i<FUNC_COUNT*30; ++i) {
	start = timing_start();
	//scandir(dir, scanfile[i%FUNC_COUNT]);

	scandir(dir, scanfile[select]);
	statistics.timecost = timing_cost(start);
	fprintf(fpout, "%d %zu, %zu, %lf\n", (i%FUNC_COUNT)+1,statistics.n, statistics.match, statistics.timecost);
	fflush(fpout);
	usleep(1000+random()%100000);
    }
    fclose(fpout);

    return 0;
}

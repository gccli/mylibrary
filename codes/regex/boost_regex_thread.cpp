#include <boost/regex.hpp>
#include <string>
#include <set>
#include <pthread.h>
#include "utiltime.h"
using namespace std;

int totalscantimes=10*20*24*32;
int scantimes;
double *array;

// g++ -I ../../inc main.cpp -lpthread -lboost_regex
void *regex_match_thread(void *param)
{
    double *cost = (double *)param;
    double start = 0.0;

    string regex="^My Muppets Show|^My%20Muppets%20Show|^Mozilla";
    string data = "Mozilla/5.0 (Windows NT 6.1; WOW64; rv:25.0) Gecko/20100101 Firefox/25.0";
    start = timing_start();

    printf("Thread scan number %d\n", scantimes);

    for(int i=0; i<scantimes; ++i) {
	try {
	    boost::regex regexp(regex);
	    boost::smatch what;
	    if (regex_search(string(data), what, regexp)) {
		//printf("Match regexp - %s\n", std::string(what[0].first, what[0].second).c_str());
	    } 
	} catch (boost::regex_error &e) {
	    printf("failed search section - %s\n", e.what());
	}

    }

    *cost = timing_cost(start);

    return NULL;
}

int main(int argc, char *argv[])
{
    set<int> accept_num;
    int thnum = 1;
    if (argc > 1) thnum = atoi(argv[1]);
    if (thnum > 32) thnum = 32;

    accept_num.insert(1);
    accept_num.insert(2);
    accept_num.insert(4);
    accept_num.insert(8);
    accept_num.insert(10);
    accept_num.insert(16);
    accept_num.insert(20);
    accept_num.insert(24);
    accept_num.insert(32);

    if (!accept_num.count(thnum)) {
	printf("Accept thread number: ");
	for(set<int>::iterator it = accept_num.begin(); 
	    it != accept_num.end(); ++it) {
	    printf("%d ", *it);
	}
	printf("\n");
	return 1;
    }

    array = new double[thnum];
    double total = 0.0;
    pthread_t th[50];

    scantimes = totalscantimes/thnum;

    for(int i=0; i<thnum; ++i) {
	pthread_create(&th[i], NULL, regex_match_thread, (void *)&array[i]);
    }

    for(int i=0; i<thnum; ++i) {
	pthread_join(th[i], NULL);
	total += array[i];
    }

    printf ("Thread number %d, Total %d for each thread %d.\nTotal time cost %lf\n", 
	    thnum, totalscantimes, scantimes, total);


    return 0;
}

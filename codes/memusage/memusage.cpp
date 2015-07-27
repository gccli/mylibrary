#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <regex.h>
#include <errno.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <libgen.h>

#include <string>
#include <set>
#include <mcheck.h>

#include "memory-usage.h"

using namespace std;

static size_t pagesize = 0;
static long sleep_time = 100*1000; // 0.1 second
FILE *report_file = NULL;

static void free_list(char **list, size_t n)
{
   for(size_t i=0; i<n; ++i)
      free(list[i]);
   free(list);
}

static char **gen_list(const char *str, size_t *n, const char *delim = " ")
{
   char *running, *q;
   char **result = NULL;
   int i=0;
   running = strdupa(str);

   while((q = strsep(&running, delim)) != NULL)
   {
      if (q[0] == 0) continue;
      result = (char **)realloc(result, (i+2)*sizeof(char *));
      result[i++] = strdup(q);
      result[i] = 0;
   }
   *n = i;

   return result;
}

bool regex_match(const string &pattern, const char *text, string *result, bool icase=false)
{  
   regex_t r;
   int cflags, status;

   cflags = REG_EXTENDED|REG_NEWLINE;
   if (icase)
      cflags |= REG_ICASE;
   if ((status = regcomp(&r, pattern.c_str(), cflags)) != 0)
   {
      char errorstr[256] = {0};
      regerror(status, &r, errorstr, sizeof(errorstr));
      printf("Regex error compiling '%s': %s", pattern.c_str(), errorstr);
      return false;
   }

   bool match=false;
   const int n = 2;
   regmatch_t m[n];

   while (1)
   {
      int nomatch = regexec(&r, text, n, m, 0);
      if (nomatch) {
	 break;
      }
      match = true;
      for (int i = 0; i < n; i++) {
	 if (m[i].rm_so == -1) {
	    break;
	 }
	 if (result)
	    result->assign(text+m[i].rm_so, m[i].rm_eo-m[i].rm_so);
	 break;// only match the first one
      }
      break;
   }

   regfree(&r);
   return match;
}

static char *load_proc_file(const char *filename, size_t *len)
{
   FILE *fp;
   char *buffp;
   long  allocsz=0, n=0;
   if (!(fp = fopen(filename, "r"))) {
      printf("Failed to open '%s' - %s", filename, strerror(errno));
      return NULL;
   }

   buffp = NULL;
   while(!feof(fp)) {
      allocsz += 1024;
      buffp = (char *) realloc(buffp, allocsz);
      memset(buffp+n, 0, 1024);
      n += fread(buffp, 1024, 1, fp);      
   }
   *len = n;
   fclose(fp);

   return buffp;
}

static bool analyze_proc_stat(int pid, string &outstr)
{
   size_t len, rss_bytes, n=0;

   char temp[128] = {0};
   char *fbuff;
   char **stat_list;

   sprintf(temp, "/proc/%d/stat", pid);
   fbuff = load_proc_file(temp, &len);
   stat_list = gen_list(fbuff, &n);

   rss_bytes = pagesize*atoi(stat_list[stat_rss]);
   sprintf(temp, "%s, %s, %zu, %s", stat_list[stat_pid], stat_list[stat_ppid], rss_bytes, stat_list[stat_vsize]);
   outstr.append(temp);

   free_list(stat_list, n);
   free(fbuff);
   return true;   
}

static bool analyze_proc_status(int pid, string &outstr)
{
   size_t len;
   size_t count=0;

   char status_file[128] = {0};
   char *fbuff;
   char **result;
   sprintf(status_file, "/proc/%d/status", pid);

   fbuff = load_proc_file(status_file, &len);
 
   string rss, vsz;
   string pattern = "^VmRSS:\\s*[0-9]+";
   regex_match(pattern, fbuff, &rss);
   result = gen_list(rss.c_str(), &count, " \t");
   outstr.append(", ");   outstr.append(result[1]);
   free_list(result, count);

   pattern = "^VmSize:\\s*[0-9]+";
   regex_match(pattern, fbuff, &vsz);
   result = gen_list(vsz.c_str(), &count, " \t");
   outstr.append(", ");   outstr.append(result[1]);
   free_list(result, count);

   free(fbuff);
   return true;
}

struct rss_usage {
   char shared_clean[12];
   char shared_dirty[12];
   char private_clean[12];
   char private_dirty[12];
};

static bool analyze_proc_smaps(int pid, string &outstr)
{
   char smaps_file[512] = {0};
   sprintf(smaps_file, "/proc/%d/smaps", pid);

   FILE *fp;
   char *line = NULL;
   size_t len = 0,n;
   ssize_t read;
   char **addr_maps;


   if ((fp = fopen(smaps_file, "r")) == NULL) {
      printf("failed to open '%s'\n", smaps_file);
      return false;
   }

   struct rss_usage rss;
   long shared_clean=0,shared_dirty=0,private_clean=0,private_dirty=0,total=0;

   string match;
   string addr_line;
   string addr_prog;
   string pattern;
   int num=0;
   int num_maps = 0;
   int error=0;
   while ((read = getline(&line, &len, fp)) != -1) {
      if (read-1 < 0) continue;
      if (len == 0 || line == NULL) {
	 error=1;
	 goto Error;
      }
      line[read-1] = 0;

      pattern = "^[0-9a-f]+-[0-9a-f]+";
      if (regex_match(pattern, line, &match)) {
	 if (num_maps > 0) {
	    //printf("%-32s %-21s Shared_Clean:%4s Shared_Dirty:%4s Private_Clean:%4s Private_Dirty:%4s\n",
	    //addr_line.c_str(), addr_prog.substr(0,12).c_str(), rss.shared_clean,rss.shared_dirty,rss.private_clean,rss.private_dirty);
	    shared_clean+=atoi(rss.shared_clean);
	    shared_dirty+=atoi(rss.shared_dirty);
	    private_clean+=atoi(rss.private_clean);
	    private_dirty+=atoi(rss.private_dirty);
	 }
	 memset(&rss, 0, sizeof(rss));
	 n=0;
	 addr_maps = gen_list(line, &n, " \t");
	 addr_line = match;
	 addr_prog = basename(addr_maps[n-1]);
	 free_list(addr_maps, n);
	 num_maps++;
      }

      pattern = "^Shared_Clean.*";
      if (regex_match(pattern, line, &match)) {
	 for(size_t i=0,j=0; i<match.length(); ++i)
	    if(isdigit(match[i]))
	       rss.shared_clean[j++] = match[i];
      }

      pattern = "^Shared_Dirty.*";
      if (regex_match(pattern, line, &match)) {
	 for(size_t i=0,j=0; i<match.length(); ++i)
	    if(isdigit(match[i]))
	       rss.shared_dirty[j++] = match[i];
      }

      pattern = "^Private_Clean.*";
      if (regex_match(pattern, line, &match)) {
	 for(size_t i=0,j=0; i<match.length(); ++i)
	    if(isdigit(match[i]))
	       rss.private_clean[j++] = match[i];
      }

      pattern = "^Private_Dirty.*";
      if (regex_match(pattern, line, &match)) {
	 for(size_t i=0,j=0; i<match.length(); ++i)
	    if(isdigit(match[i]))
	       rss.private_dirty[j++] = match[i];
      }
      ++num;
   }

   total=shared_clean+shared_dirty+private_clean+private_dirty;
   sprintf(smaps_file, ", %ld, %ld, %ld, %ld, %ld", total, shared_clean,shared_dirty,private_clean,private_dirty);
   outstr.append(smaps_file);

Error:
   if (line)
      free(line);
   fclose(fp);
   return true;
}

static bool get_memory_usage(string &outstr)
{
   pid_t pid = getpid();

   analyze_proc_stat(pid, outstr);
   analyze_proc_status(pid, outstr);
   analyze_proc_smaps(pid, outstr);

   return true;
}

void create_child_proc(int n, int loop)
{
   pid_t child,cpid;
   int status;
   static set<int> childs;
   string report;

   set<int>::iterator it = childs.begin();
   for (; it != childs.end(); ++it) {
      int pid = *it;
      kill(pid, SIGTERM);
      cpid = waitpid(cpid, &status, 0);
      childs.erase(pid);
   }

   for (int kk = 0; kk < n; ++kk) {
      if ((child = fork()) == 0) { // in child process
	 while(--loop && getppid() > 1) {
	    report.clear();
	    get_memory_usage(report);
	    fprintf(report_file, "%s\n", report.c_str());
	    fflush(report_file);
	    usleep(sleep_time);
	 }
	 exit(0);
      }

      // parent process in here
      childs.insert(child);
   }

   //printf("PARENT(%d) %d child created\n",getpid(), childs.size());
}

void sighandle(int signo)
{
   int status;
   if (signo == SIGCHLD) {
      wait(&status);
   }
}

int main(int argc, char *argv[])
{
   mtrace();

   int count = 50;
   int alloc_sz = 10240;
   string report;

   char temp[1024] = {0};
   char *p[8];

   pagesize = sysconf(_SC_PAGESIZE);
   report_file = fopen("/root/report.csv", "w");

   if (argc > 1)
      count = atoi(argv[1]);

   for(int i=0; i<8; ++i) {
      p[i] = (char *) malloc(alloc_sz);
      memset(p[i], 'B', alloc_sz);
   }
   memset(temp, 0, sizeof(temp));
   fprintf(report_file, "PID   PPID   RSS(stat) VmSize(stat) RSS(kB) VmSzie(kB) TotalRSS Shared_Clean Shared_Dirty Private_Clean Private_Dirty\n");
   fflush(report_file);

   for(int i=0; ; i++) {
      if (i%count == 0) {// create child process
	 create_child_proc(2, count);
	 //alloc_sz += 4000;
      }
      signal(SIGCHLD, sighandle);
      for(int k=0; (i%2==0) && k<8; ++k)
	 free(p[k]);

      long tmps = (random() % 10240)+4000;


      report.clear();
      get_memory_usage(report);
      fprintf(report_file, "%s\n", report.c_str());
      fflush(report_file);
      usleep(sleep_time);

      for(int k=0; (i%2==1) && k<8; ++k) {
	 void *tmpp = malloc(tmps);
	 p[k] = (char *)malloc(alloc_sz);
	 memset(p[k], 'A', alloc_sz);
	 free(tmpp);
      }
   }   

   return 0;
}

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <set>
#include <map>
#include <queue>
#include <string>
#include <typeinfo>

#include <iostream>

using namespace std;

#define FAIL      -1

static int newstate = 0;
typedef std::set<char>              cset_t;   // all char set
typedef std::set<string>            kset_t;   // keyword output set
typedef std::pair<int, char>        scpair_t; // state char pair
typedef std::map<scpair_t, int>     goto_t;
typedef std::map<int, int>          fail_t;
typedef std::map<int, kset_t>       output_t;

cset_t   all_a;
goto_t   g_st; // goto storage
fail_t   f_st; // failure storage
output_t o_st; // output storage

#define foreach(cont) for(decltype(cont)::iterator ia = cont.begin(); ia != cont.end(); ++ia)

int g(int s, char a)
{
    goto_t::iterator it = g_st.find(scpair_t(s,a));
    if (it == g_st.end())
        return FAIL;
    return it->second;
}

int f(int s)
{
    fail_t::iterator it = f_st.find(s);
    if (it == f_st.end())
        abort();
    return it->second;
}

void enter(const char *key, int len)
{
    printf("add %s...\n", key);

    int j, p;
    int state = 0;
    j = 0;
    while(g(state, key[j]) != FAIL) {
        state = g(state, key[j]);
        ++j;
    }
    for (p=j; p<len; ++p) {
        newstate += 1;
        g_st[scpair_t(state, key[p])] = newstate;
        state = newstate;
    }
    o_st[state].insert(key);
    for(j=0; j<len; ++j)
        all_a.insert(key[j]);
}




void init(char *keys[])
{
    goto_t::iterator it;
    while(keys[0]) {
        enter(keys[0], strlen(keys[0]));
        keys++;
    }
    foreach(all_a) {
        decltype(all_a)::value_type a = *ia;
        if (g(0, a) == FAIL) g_st[scpair_t(0, a)] = 0;
    }

    int s, r, ss, state;
    queue<int> q;

    foreach(all_a) {
        decltype(all_a)::value_type a = *ia;
        if ((s = g(0, a)) != 0) {
            q.push(s);
            f_st[s] = 0;
        }
    }

    printf("done qsize: %zu\n", q.size());
    while(!q.empty()) {
        r = q.front(); q.pop();
        foreach(all_a) {
            decltype(all_a)::value_type a = *ia;
            if ((s = g(r, a)) == FAIL) continue;
            q.push(s);

            state = f(r);
            while(g(state, a) == FAIL)
                state = f(state);
            f_st[s] = g(state, a);
            ss = f(s);
            if (o_st.find(s) != o_st.end()) {
                if (o_st.find(ss) != o_st.end()) {
                    o_st[s].insert(o_st[ss].begin(),o_st[ss].end());
                }
            }
        }
    }
}

void show()
{
    scpair_t k;
    kset_t ks;
    goto_t::iterator it;
    fail_t::iterator iif;
    output_t::iterator io;
    for(it = g_st.begin(); it != g_st.end(); ++it) {
        k = it->first;
        printf("g(%d,%c) = %d\n", k.first, k.second, g(k.first, k.second));
    }
    for(iif = f_st.begin(); iif != f_st.end(); ++iif) {
        printf("f(%d) = %d\n", iif->first, f(iif->first));
    }

    for(io = o_st.begin(); io != o_st.end(); ++io) {
        ks = io->second;
        printf("%d = {", io->first);
        kset_t::iterator i = ks.begin();
        printf("%s", i->c_str());
        ++i;
        for(;i != ks.end(); ++i) printf(",%s", i->c_str());
        printf("}\n");
    }
}

/**
 * Usage: ./a.out he she his hers
 * Compile: g++ -g -Wall -std=c++0x main.cpp
 */
int main(int argc, char *argv[])
{
    argv++;
    init(argv);
    show();
    return 0;
}

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
    if (s == 0) return 0;

    fail_t::iterator it = f_st.find(s);
    if (it == f_st.end())
        abort();
    return it->second;
}

void enter(const char *key, int len)
{
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
    printf("Keywords: ");
    goto_t::iterator it;
    while(keys[0]) {
        printf("\"%s\", ", keys[0]);
        enter(keys[0], strlen(keys[0]));
        keys++;
    }

    printf("\n");
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

bool search(const char *s, int len)
{
    bool found = false;
    int i, state = 0;
    output_t::iterator it;
    for(i=0; i<len;) {
        while(g(state, s[i++]) == FAIL)
            state = f(state);
        state = g(state, s[i-1]);
        if ((it = o_st.find(state)) != o_st.end()) {
            foreach(it->second) {
                printf("%-6d %s\n", i, ia->c_str());
            }
            found = true;
        }
    }
    return found;
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
    const char *test = "If expression is a function call which returns a prvalue of\n"
        "class type or is a comma expression whose right operand is such a function call,\n"
        "a temporary object is not introduced for that prvalue.";
    argv++;
    init(argv);
    //show();

    printf("----------------------------------------------------------------\n");
    printf("Search\n%s\n", test);
    printf("----------------------------------------------------------------\n\nResult:\n");

    printf("Match: %s\n", search(test, strlen(test))? "Yes": "No");
    return 0;
}

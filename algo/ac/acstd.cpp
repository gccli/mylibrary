#include "acstd.h"

#include <queue>
#include <typeinfo>

long mp_alloc_total = 0;

static mp_pattern_t *mp_pattern_dup(mp_pattern_t *pt)
{
    mp_pattern_t *p = (mp_pattern_t *)calloc(sizeof(*p), 1);
    memcpy(p, pt, sizeof(*p));
    p->next = NULL;

    mp_alloc_total += sizeof(*p);

    return p;
}

int mp_add_pattern(mp_struct_t *p, unsigned char *pat, int n, long id)
{
    mp_pattern_t *plist;

    plist = (mp_pattern_t *) calloc(sizeof(mp_pattern_t), 1);
    mp_alloc_total += sizeof(*plist);

    plist->pattern = (unsigned char *) calloc(n+1, 1); // add a terminater
    mp_alloc_total += (n+1);

    memcpy(plist->pattern, pat, n);
    plist->n = n;
    plist->id = id;
    plist->next = p->patterns;

    p->patterns = plist;
    p->num_patterns++;

    return 0;
}

static void mp_add_state(mp_struct_t *mp, mp_pattern_t *pt)
{
    int state = 0;
    unsigned char *pat;
    mp_pattern_t *newp;

    pat = pt->pattern;
    while(*pat && mp->stable[state].next[*pat] != FAIL_STATE) {
        state = mp->stable[state].next[*pat];
        pat++;
    }

    for(; *pat; ++pat) {
        mp->num_states++;
        mp->stable[state].next[ *pat ] = mp->num_states;
        state = mp->num_states;
    }

    // associate state with match list
    newp = (mp_pattern_t *) calloc(sizeof(*newp), 1);
    memcpy(newp, pt, sizeof(*pt));
    newp->next = mp->stable[state].match_list;
    mp->stable[state].match_list = newp;

    mp_alloc_total += sizeof(*newp);
}

static void mp_build_nfa(mp_struct_t *mp)
{
    int i,s,r,state,next;
    std::queue<int> q;
    mp_pattern_t *pt, *plist;

    for(i=0; i<ALPHABET_SIZE; ++i) {
        if ((s = mp->stable[0].next[i]) != 0) {
            q.push(s);
            mp->stable[s].fail = 0;
        }
    }

    while(!q.empty()) {
        r = q.front(); q.pop();
        for(i=0; i<ALPHABET_SIZE; ++i) {
            if ((s = mp->stable[r].next[i]) != FAIL_STATE) {
                q.push(s);
                state = mp->stable[r].fail;
                while((next = mp->stable[state].next[i]) == FAIL_STATE)
                    state = mp->stable[state].fail;
                mp->stable[s].fail = next;

                // merge next states matchlist to s
                for(plist = mp->stable[next].match_list; plist != NULL;
                    plist = plist->next) {
                    pt = mp_pattern_dup(plist);
                    pt->next = mp->stable[s].match_list;
                    mp->stable[s].match_list = pt;
                }
            }
        }

    }
}

static void mp_nfa_to_nfa(mp_struct_t *mp)
{
    int i,s,r,state;
    std::queue<int> q;

    for(i=0; i<ALPHABET_SIZE; ++i) {
        if ((s = mp->stable[0].next[i]) != 0) {
            q.push(s);
        }
    }

    while(!q.empty()) {
        r = q.front(); q.pop();
        for(i=0; i<ALPHABET_SIZE; ++i) {
            if ((s = mp->stable[r].next[i]) != FAIL_STATE) {
                q.push(s);
            } else {
                state = mp->stable[r].fail;
                mp->stable[r].next[i] = mp->stable[state].next[i];
            }
        }
    }
}

int mp_compile(mp_struct_t *mp)
{
    int i,k;
    mp_pattern_t *plist;
    mp->max_states = 1;
    for(plist = mp->patterns; plist; plist = plist->next) {
        mp->max_states += plist->n;
    }
    mp->stable = (mp_statetable_t *) calloc(sizeof(mp_statetable_t),
                                            mp->max_states);
    mp->num_states = 0;
    mp_alloc_total += mp->max_states * sizeof(mp_statetable_t);

    for (k=0; k<mp->max_states; ++k) {
        for(i=0; i<ALPHABET_SIZE; ++i)
            mp->stable[k].next[i] = FAIL_STATE;
    }

    // add single pattern
    for(plist = mp->patterns; plist; plist = plist->next) {
        mp_add_state(mp, plist);
    }

    for(i=0; i<ALPHABET_SIZE; ++i) {
        if (mp->stable[0].next[i] == FAIL_STATE)
            mp->stable[0].next[i] = 0;
    }
    mp_build_nfa(mp);
    mp_nfa_to_nfa(mp);

    return 0;
}

int mp_search(mp_struct_t *mp, unsigned char *txt, int n,
              match_callback callback, int *state)
{
    int index, next, count = 0;
    unsigned char *t, *tend;
    mp_pattern_t *m; // pattern match list

    t = txt;
    tend = txt + n;

    next = *state;
    for(; t < tend; ++t) {
        next = mp->stable[next].next[*t];
        for (m = mp->stable[next].match_list; m; m = m->next) {
            index = t - m->n + 1 - txt;
            count++;
            if (callback(m->id, index, m) > 0) {
                break;
            }
        }
    }

    *state = next;
    return count;
}

#ifndef AC_STD_IMPL_H__
#define AC_STD_IMPL_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define foreach(c) \
    for(std::decltype(c)::iterator ii = c.begin(); ii != c.end(); ++ii)

#define FAIL_STATE         -1
#define ALPHABET_SIZE     256

typedef int (*match_callback)(int id, int index, void *data);

typedef struct _mp_pattern {
    struct _mp_pattern *next;
    unsigned char      *pattern;
    int                 n;
    long                id;
} mp_pattern_t;

typedef struct _mp_statetable {
    int           next[ALPHABET_SIZE];// next state, next move function
    int           fail;               // fail state
    mp_pattern_t *match_list;
} mp_statetable_t;

typedef struct _mp_struct {
    int max_states;
    int num_states;
    int num_patterns;

    mp_pattern_t     *patterns;
    mp_statetable_t  *stable; // state table
} mp_struct_t;

extern long mp_alloc_total;
int mp_add_pattern(mp_struct_t *p, unsigned char *pat, int n, long id);
int mp_compile(mp_struct_t *mp);
int mp_search(mp_struct_t *mp, unsigned char *txt, int n,
              match_callback callback, int *state);

#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define INSERT_MODE 1   /* Indicate the insertion mode */
#define SEARCH_MODE 2   /* Indicate the search, or retrieval, mode */
#define DELETE_MODE 3   /* Indicate the deletion mode */
#define DUMP_MODE   4   /* Indicate the dump mode */
#define END_MODE    5   /* Indicate the end of program */
#define MIN_CODE    1   /* Minimum numerical code */
#define MAX_CODE    255 /* Maximum numerical code */
#define BC_INC      10  /* Increment of the double-array */
#define TAIL_INC    10  /* Increment of TAIL */
#define KEY_INC     5   /* Increment of the double-array */
#define TEMP_INC    5   /* Increment of TAIL */
#define TRUE       -1
#define FALSE       0
#define NILL       -1



FILE *KEY_FILE; /* Key dictionary file */
char *KEY;      /* Key */
char *TAIL;     /* TAIL */
char *TEMP ;    /* Buffer */
int  *BC;       /* BASE and CHECK*/
int   MODE ;    /* Flag indicating insertion, search,
                   int deletion, dump and end */
int   BC_POS ;  /* Current maximum index of the double-array */
int   TAIL_POS; /* The current maximum index of TAIL */
int   BC_MAX ;  /* Maximum size of BASE and CHECK */
int   TAIL_MAX; /* Maximum size of TAIL */
int   KEY_MAX;  /* Maximum size of KEY */
int   TEMP_MAX; /* Maximum size of TEMP */

void BC_INSERT(), TAIL_INSERT(), SEPARATE(), W_BASE(), W_CHECK();
char *MEM_STR() ;


int BASE(int n) {
    if (n > BC_POS) return 0;
    return BC[2*n];
}

int CHECK(int n) {
    if (n > BC_POS) return 0;
    return BC[2*n+1];
}

void REALLOC_BC() {
    int i, pre_bc;
    pre_bc = BC_MAX;
    BC_MAX += BC_INC;

    BC = (int *)realloc(BC, sizeof(int) * 2 * BC_MAX);
    for(i=2*pre_bc; i<2*BC_MAX;++i) BC[i] = 0;
}
char *REALLOC_STR(char *area, int *max, int inc) {
    int i, pre_size = *max;
    *max += inc;
    area = realloc(area, *max);
    for(i=pre_size; i<*max; ++i) area[i] = 0;
    return area;
}

void W_BASE(int n, int node) {
    while (n >= BC_MAX) REALLOC_BC();
    if (n > BC_POS) BC_POS = n;
    BC[2*n] = node;
}

void W_CHECK(int n, int node) {
    while (n >= BC_MAX) REALLOC_BC();
    if (n > BC_POS) BC_POS = n;
    BC[2*n+1] = node;
}

char *MEM_STR(char *area_name, int *max) {
    char *area;

    area = (char *)malloc(*max);
    memset(area, 0, *max);
    return area;
}

void READ_TAIL(int p) {
    int i=0;
    while(TAIL[p] != '#') TEMP[i++] = TAIL[p++];
    TEMP[i++] = '#'; TEMP[i] = 0;
}

void WRITE_TAIL(char *temp, int p) {
    int i=0, tail_index = p;
    while((p+strlen(temp)) >TAIL_MAX-1) {
        TAIL = REALLOC_STR(TAIL, &TAIL_MAX, TAIL_INC);
    }
    while(*(temp+i) != 0) {
        TAIL[tail_index++] = temp[i++];
    }
    if ((p+i+1) > TAIL_POS) TAIL_POS = p+i;
}


int SEARCH(char *KEY) {
    unsigned char ch;
    int h=-1, s=-1, t;

    do {
        ++h;
        ch = KEY[h];
        t = BASE(s) + ch;
        if (CHECK(t) != s) {
            return FALSE;
        }
        if (BASE(t) <0) break;
        s = t;
    } while(1);

    if (*(KEY+h) != '#') READ_TAIL((-1)*BASE(t));
    if (*(KEY+h) != '#' || strcmp(TEMP, KEY+h+1) == 0) {
        return TRUE;
    }
    return FALSE;
}



char *SET_LIST(int s) {
    static char list[256];
    int i, j=0, t;
    for(i=0; i<255; ++i)  {
        t = BASE(s) + i;
        if (CHECK(t) == s) list[j++] = (char)i;
    }
    list[j] = 0;
    return list;
}

int X_CHECK(char *list) {
    int i, base_pos = 1, check_pos;
    unsigned char sch;
    i=0;
    do {
        sch = list[i++];
        check_pos = base_pos + sch;
        if (CHECK(check_pos) != 0) {
            base_pos++; i = 0;
            continue;
        }
    } while(list[i] != 0);

    return base_pos;
}

int CHANGE_BC(int current, int s, char *list, char ch) {
    int i, k, old_node, new_node, old_base;
    char a_list[256] = {0};

    old_base = BASE(s);
    if (ch != 0) {
        strcpy(a_list, list); i = strlen(a_list);
        a_list[i] = ch; a_list[i+1] = 0;
    }
    W_BASE(s, X_CHECK(a_list));
    i=0;
    do {
        old_node = old_base + (unsigned char) (*list);
        new_node = BASE(s) + (unsigned char) (*list);
        W_BASE(new_node, BASE(old_node));
        W_CHECK(new_node, s);
        if (BASE(old_node) > 0) {
            k = BASE(old_node)+1;
            while (k-BASE(old_node) <MAX_CODE-MIN_CODE||k<BC_POS) {
                if(CHECK(k) == old_node) W_CHECK(k, new_node);
                ++k;
            }
        }
        if (current != s && old_node == current) current = new_node;
        W_BASE(old_node, 0); W_CHECK(old_node, 0) ; list++;
    } while(*list != 0);

    return current;
}

void SEPARATE (int s, char *b, int tail_pos)
{
    int t;
    t = BASE(s) + (unsigned char) *b; b++;
    W_CHECK (t , s ) ;
    W_BASE (t, (-1) *tail_pos) ;
    WRITE_TAIL (b, tail_pos) ;
}

void BC_INSERT(int s, char *b) {
    int t;
    char list_s[256], list_t[256];

    t = BASE(s) + (unsigned char) *b;
    if (CHECK(t) != 0) {
        strcpy(list_s, SET_LIST(s));
        strcpy(list_t, SET_LIST(CHECK(t)));
        if (strlen(list_s) +1<strlen(list_t))
            s = CHANGE_BC(s, s, list_s, *b);
        else
            s = CHANGE_BC(s, CHECK(t), list_t, '\0');
    }

    SEPARATE(s, b, TAIL_POS);
}


void SELECTION()
{
    char key_name[30];
    printf(" 1. Insert 2. Search 3. Delete 4. Dump 5. End \n");
    scanf("%d%*c", &MODE) ;
    if(MODE == END_MODE) exit(0) ;
    if(MODE != DUMP_MODE) {
        printf("key_file = ");
        scanf("%s%*c", key_name) ;
        KEY_FILE = fopen(key_name, "r") ;
        if(KEY_FILE == NULL){
            printf("\nkey_dic can’t open\n");
            exit(0);
        }
    }
}

int main(int argc, char *argv[])
{
//    INSERT("hello");
//    INSERT("hero");
//    INSERT("elasticsearch");
    printf ("%d\n", SEARCH("elasticsearch"));
    return 0;
}

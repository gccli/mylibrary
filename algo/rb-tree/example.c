#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <sys/time.h>
#include <rb_tree.h>
#include <rb_tree_printer.h>

typedef struct st_entry {
    rb_node_t node;
    int       key;
    void     *data;
} entry_t;

static entry_t *create(int key)
{
    entry_t *node = calloc(sizeof(*node), 1);
    node->key = key;
    return node;
}

static int get_key(rb_node_t *z)
{
    entry_t *e = rb_entry(z, entry_t, node);
    return e->key;
}

static void shuffle(int *a, int length)
{
    struct timeval tv;
    gettimeofday(&tv, NULL);

    srand(1000*tv.tv_sec + 1000*tv.tv_usec);
    int i,j,k,tmp;

    for(i=0; i<length; ++i) {
        j = random()%(length-2)+1;
        k = (i%2) ? length-1 : 0;
        tmp = a[j];
        a[j] = a[k];
        a[k] = tmp;
    }
}

static void clear_screen()
{
    printf("\033[1;1H\033[0J");
}

void test_rbtree(int length)
{
    int i,h;
    entry_t *e;
    rb_node_t *p;
    rb_tree_t rbtree = {&rb_nil, get_key};

    int a[] = {3,7,10,12,14,15,16,17,19,20,21,23,26,28,30,35,38,39,41,47};
    length = sizeof(a)/sizeof(int);
    shuffle(a, length);

    // insert
    for(i=0; i<length; ++i) {
        e = create(a[i]);
        rb_insert(&rbtree, &e->node);
    }

    // traverse & print tree
    clear_screen();
    h = rb_height(rbtree.root);
    printf("Number of node %d, height %d\n", length, h);
    for(p = rb_first(&rbtree); p; p = rb_next(p)) {
        printf("%d  ", get_key(p));
    }
    printf("\n");

    rb_pretty_print(&rbtree, 3);
    rb_print(&rbtree);

    // search & delete
    shuffle(a, length);
    for(i=0; i<length; ++i) {
        p = rb_search(&rbtree, a[i]);
        e = rb_entry(p, entry_t, node);
        rb_delete(&rbtree, p);
        free(e);
    }
}

int main(int argc, char* argv[])
{
    test_rbtree(10);

    return 0;
}

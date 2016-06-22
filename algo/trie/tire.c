#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "tire.h"

static size_t g_allocate_size = 0;

tire_node *tree;


void *my_malloc(size_t size)
{
    g_allocate_size += size;
    return calloc(1, size);
}

void my_free(void *data)
{
    return free(data);
}

int tire_init()
{
    tree = my_malloc(sizeof(tire_node));
    return 0;
}

int tire_insert(const char *str, int len, void *data)
{
    int i;
    unsigned char c;
    tire_node *p_tree;

    for (p_tree = tree, i=0; i<len; ++i) {
        c = str[i];
        if (p_tree->children[c] == NULL) {
            p_tree->children[c] = my_malloc(sizeof(tire_node));
        }
        p_tree = p_tree->children[c];
    }

    if (p_tree->data != NULL) {
        return 1; // already exists
    }
    p_tree->data = data;

    printf("Memory allocate %.2f KB\n", 1.0*g_allocate_size/1024.0);
    return 0;
}

tire_node *tire_search(const char *str, int len)
{
    int i;
    unsigned char c;
    tire_node *p_tree;

    for (p_tree = tree, i=0; i<len; ++i) {
        c = str[i];
        if (p_tree->children[c] == NULL) {
            return NULL;
        }
        p_tree = p_tree->children[c];
    }

    return  (p_tree->data == NULL) ? NULL : p_tree;

    return p_tree;
}

int main(int argc, char *argv[])
{
    int i=0;
    const char *keys[] = {
        "morphological",
        "mathematics",
        "method",
        "commutative",
        "currency",
        "tokenization",
        "lexicon",
        "proportional",
        NULL
    };

    tire_init();

    for(i=0; i<sizeof(keys)/sizeof(char *); ++i) {
        if (keys[i] == NULL) break;
        tire_insert(keys[i], strlen(keys[i]), (void *)keys[i]);
    }

    printf("Memory allocate %.2f KB\n", 1.0*g_allocate_size/1024.0);
    for (i=1; i<argc; ++i) {
        printf("search %-16s %s\n", argv[i],
               tire_search(argv[i], strlen(argv[i]))?"found":"not found");
    }

    return 0;
}

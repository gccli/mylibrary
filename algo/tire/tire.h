#ifndef TIRE_TREE_H__
#define TIRE_TREE_H__

typedef struct tire_node {
    void             *data;
    struct tire_node *children[256];
} tire_node;


#endif

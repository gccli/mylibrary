# rb-tree
Implementation of red-black tree

## Features
* This code based on the book "Introduction to Algorithms 3rd".
* Add pretty printer for rb-tree.
* Elaborate example


## Limit
This implementation assume that the type of key is int


## Usage

``` c
#include <rb_tree.h>
#include <rb_tree_printer.h>

typedef struct st_entry {
    rb_node_t node;
    int       key;
    void     *data;
} entry_t;

// get_key_callback
static int get_key(rb_node_t *z)
{
    entry_t *e = rb_entry(z, entry_t, node);
    return e->key;
}

// create rb-tree
entry_t *e;
rb_node_t *p;
rb_tree_t rbtree = {&rb_nil, get_key};

// insert
e = create(key);
rb_insert(&rbtree, &e->node);

// traverse & print tree
for(p = rb_first(&rbtree); p; p = rb_next(p)) {
    printf("%d  ", get_key(p));
}
rb_pretty_print(&rbtree, 3);
rb_print(&rbtree);

// search
p = rb_search(&rbtree, a[i]);
e = rb_entry(p, entry_t, node);

// delete
rb_delete(&rbtree, p);

```


## Run example
``` shell
make
./rbtee
```

After run the example:
![eveny time the result is different](example.png "snashot)

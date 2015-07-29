#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "list.h"
#include "rbtree.h"

typedef struct mytype
{
  char             *key;
  char             *value;
  char             *block;
  struct list_head list;
  struct rb_node   node;
} mytype_t;

void my_free(mytype_t *data)
{
  if (data) {
    if(data->key) free(data->key);
    if(data->value) free(data->value);
    if (data->block) free(data->block);
    free(data);
  }
}


struct mytype *my_search(struct rb_root *root, char *string)
{
  struct rb_node *node = root->rb_node;
  while (node) {
    struct mytype *data = container_of(node, struct mytype, node);
    int result;
    result = strcmp(string, data->key);
    if (result < 0)
      node = node->rb_left;
    else if (result > 0)
      node = node->rb_right;
    else
      return data;
  }
  return NULL;
}


int my_insert(struct rb_root *root, struct mytype *data)
{
  struct rb_node **new = &(root->rb_node), *parent = NULL;
  /* Figure out where to put new node */
  while (*new) {
    struct mytype *this = container_of(*new, struct mytype, node);
    int result = strcmp(data->key, this->key);

    parent = *new;
    if (result < 0)
      new = &((*new)->rb_left);
    else if (result > 0)
      new = &((*new)->rb_right);
    else
      return false;
  }

  /* Add new node and rebalance tree. */
  rb_link_node(&data->node, parent, new);
  rb_insert_color(&data->node, root);

  return true;
}


int test_rbtree()
{
  int i=0;
  char str[128];
  struct rb_node *node;
  mytype_t *data, *next;
  struct rb_root mytree = RB_ROOT;

  for(i=0; i<10; ++i) {
    mytype_t *a = calloc(sizeof(*a), 1);
    a->block = malloc(1024);

    sprintf(str, "%ld", random());
    a->key = strdup(str);
    sprintf(str, "%d_%ld", i, random());
    a->value = strdup(str);
    if (my_search(&mytree, a->key) == NULL)
      my_insert(&mytree, a);
    else printf("key '%s' exists\n", a->key);
  }

  // Iterating through the elements stored in an rbtree (in sort order)
  for (node = rb_first(&mytree); node; node = rb_next(node)) {
    data = rb_entry(node, mytype_t, node);
    printf("key=%-12s value=%-12s %p\n", data->key, data->value, &data->node);
  }

  printf("--------------------------------\n");
  printf("delete %s\n", data->key);
  rb_erase(&data->node, &mytree);
  my_free(data);

  rbtree_postorder_for_each_entry_safe(data, next, &mytree, node) {
    printf("key=%-12s value=%s\n", data->key, data->value);
    rb_erase(&data->node, &mytree);
    my_free(data);
  }
  printf("--------------------------------\n");
  for (node = rb_first(&mytree); node; node = rb_next(node)) {
    data = rb_entry(node, mytype_t, node);
    printf("key=%-12s value=%s\n", data->key, data->value);
  }
  data = rb_entry(rb_first(&mytree), mytype_t, node);
  rb_erase(&data->node, &mytree);
  my_free(data);

  return 0;
}


int test_list()
{
  int i=0;
  char str[128];
  mytype_t myroot, *pos, *next;
  INIT_LIST_HEAD(&myroot.list);

  for(i=0; i<10; ++i) {
    mytype_t *a = calloc(sizeof(*a), 1);
    sprintf(str, "%ld", random());
    a->key = strdup(str);
    sprintf(str, "%d_%ld", i, random());
    a->value = strdup(str);

    list_add_tail(&a->list, &myroot.list);
  }

  list_for_each_entry(pos, &myroot.list, list) {
    printf("key=%-12s value=%s\n", pos->key, pos->value);
  }

  list_for_each_entry_safe(pos, next, &myroot.list, list) {
    list_del_init(&pos->list);
    my_free(pos);
  }
  printf("--------------------------------\n");
  list_for_each_entry(pos, &myroot.list, list) {
    printf("key=%-12s value=%s\n", pos->key, pos->value);
  }

  return 0;
}


int main(int argc, char *argv[])
{

  printf("\nLIST\n");
  test_list();
  printf("\nRBTREE\n");
  test_rbtree();

  return 0;
}

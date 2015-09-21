#ifndef _RB_TREE_PRINTER_H__
#define _RB_TREE_PRINTER_H__

#include "rb_tree.h"

void rb_print(rb_tree_t *);
/**
 * @where: vertical position of cursor
 */
void rb_pretty_print(rb_tree_t *tree, int where);

#endif

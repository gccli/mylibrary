#include <stdio.h>
#include <math.h>
#include <string.h>

#include "rb_tree_printer.h"

#define bt_left(i)        2*((i)+1)-1
#define bt_right(i)       bt_left(i)+1
#define bt_parent(i)      ((i)-1)/2
#define coordinate        "\033[%d;%dH"

/**
 * rbtree simple printer
 */
static void set_btree_buffer(rb_tree_t *tree,
                             rb_node_t *root, char *buff, long *offs)
{
    rb_node_t *p = root;
    if(p != &rb_nil) {
        *offs += sprintf(buff + *offs, "%d%s",
                         tree->get_key_callback(p), p->color==RED?"R":"B");
        if (p->left == &rb_nil && p->right == &rb_nil) {
            return;
        }
        *offs += sprintf(buff + *offs, "[");
        if (p->left) {
            set_btree_buffer(tree, p->left, buff, offs);
            *offs += sprintf(buff + *offs, ",");
        }

        if (p->right){
            set_btree_buffer(tree, p->right, buff, offs);
            *offs += sprintf(buff + *offs, "]");
        }
    }
}

static int indent = 8;
static void print_line(char *line, int size, int *len,
                       int width)
{
    int s, e, color;
    char fmtstr[32] = {0};
    if (*len == 0 || line[0] == 0) return ;

    if ((strchr(line, 'R'))) {
        color = 101;  line[*len-1] = 0;
    } else if ((strchr(line, 'B'))) {
        color = 100;  line[*len-1] = 0;
    } else {
        color = 0;
    }
    sprintf(fmtstr, "\033[%dG\033[%dm%s\033[0m", width, color, line);

    s = width - indent;
    e = width;
    if (s >= 0) {
        printf("\033[%dG|\033[0m", s);
        for(s++; s < e-2; s++)
            printf("\033[%dG-\033[0m", s);
        printf("\033[%dG>\033[0m", s);
    }

    printf("%s\n", fmtstr);
    memset(line, 0, size);
    *len = 0;
}

void rb_print(rb_tree_t *tree)
{
    int i,j,w;
    char line[32] = {0};
    char buff[1024];
    long offs = 0;

    set_btree_buffer(tree, tree->root, buff, &offs);
    printf("TREE:  %s\n", buff);

    w = 0;
    for(i=0,j=0; i<offs; ) {
        char c = buff[i];
        switch(c) {
        case '[':
            print_line(line, sizeof(line), &j, w);
            w += indent;
            if (buff[i+1] == ',') {
                strcpy(line, "NIL"); // left is nil, right not nil
                j+=3;
            }
            break;
        case ']':
            print_line(line, sizeof(line), &j, w);
            w -= indent;
            break;
        case ',':
            print_line(line, sizeof(line), &j, w);
            break;
        default:
            line[j++] = c;
            break;
        }
        i++;
    }
}

/**
 * rbtree finer printer
 */
static void set_complete_btree(rb_node_t **nodes, int i, const rb_node_t *node)
{
    if (node != &rb_nil) {
        nodes[i] = (rb_node_t *)node;
        set_complete_btree(nodes, bt_left(i), node->left);
        set_complete_btree(nodes, bt_right(i), node->right);
    }
}

void rb_pretty_print(rb_tree_t *tree, int where)
{
    int height = rb_height(tree->root);
    int n = (int) pow(2, height) - 1; // max number of nodes in total
    int min_width = 4; // the min width of each node
    int max_width = ((int) pow(2, height-1)) * min_width;

    int i,j,k;
    int s,e,w;
    int c,row,col,color;
    rb_node_t *nodes[n];
    for(i=0; i<n; ++i) nodes[i] = NULL;
    set_complete_btree(nodes, 0, tree->root);

    w = max_width;
    for(j=1; j<=height; ++j) {
        s = (int)pow(2, j-1);
        e = (int)pow(2, j);
        w = max_width/s; // the width of every node
        row = 4*(j-1) + where;
        if(j==height) row-=2;
        for(i=0, k=s-1; k<e-1; ++i, ++k) {
            col = i*w + w/2;
            if (nodes[k]) {
                color = (nodes[k]->color == RED) ? 101 : 100;
                printf(coordinate"\033[%dm%2d\033[0m",
                       row, col, color, tree->get_key_callback(nodes[k]));
                if (nodes[k]->left != &rb_nil) {
                    printf(coordinate"/", row+1, col);
                }
                if (nodes[k]->right != &rb_nil) {
                    printf(coordinate"\\", row+1, col+1);
                }

                if (j==height) continue;
                if (nodes[k]->p != &rb_nil && nodes[k]->p->left == nodes[k]) {
                    printf(coordinate"/", row-1, col+1);
                    for(c=col+2; c<col+w/2;++c)
                        printf(coordinate"%c", row-2, c, '-');
                }
                if (nodes[k]->p != &rb_nil && nodes[k]->p->right == nodes[k]) {
                    printf(coordinate"\\", row-1, col);
                    for(c=col-1; c>col-w/2+1;--c)
                        printf(coordinate"%c", row-2, c, '-');
                }
            }
        }
        printf("\n");
    }
}

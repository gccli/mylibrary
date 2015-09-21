#include <math.h>
#include <string.h>

#include "btree.h"

#define bt_left(i)        2*((i)+1)-1
#define bt_right(i)       bt_left(i)+1
#define bt_parent(i)      ((i)-1)/2
#define coordinate        "\033[%d;%dH"

static void set_complete_btree(rb_node_t **nodes, int i, const rb_node_t *node)
{
    if (node != &sentry) {
        nodes[i] = (rb_node_t *)node;
        set_complete_btree(nodes, bt_left(i), node->left);
        set_complete_btree(nodes, bt_right(i), node->right);
    }
}

void pretty_print_rb_tree(const rb_node_t *root, int height)
{
    int n = (int) pow(2, height) - 1; // max number of nodes in total
    int min_width = 4; // the min width of each node
    int max_width = ((int) pow(2, height-1)) * min_width;

    int i,j,k;
    int s,e,w;
    int c,row,col,color;
    rb_node_t *nodes[n];
    for(i=0; i<n; ++i) nodes[i] = NULL;
    set_complete_btree(nodes, 0, root);

    printf("\033[1;1H\033[0J"); // clear screen

    w = max_width;
    for(j=1; j<=height; ++j) {
        s = (int)pow(2, j-1);
        e = (int)pow(2, j);
        w = max_width/s; // the width of every node
        row = 4*(j-1)+1;
        if(j==height) row-=2;
        for(i=0, k=s-1; k<e-1; ++i, ++k) {
            col = i*w + w/2;
            if (nodes[k]) {
                color = (nodes[k]->color == RED) ? 101 : 100;
                printf(coordinate"\033[%dm%2d\033[0m",
                       row, col, color, nodes[k]->key);
                if (nodes[k]->left != &sentry) {
                    printf(coordinate"/", row+1, col);
                }
                if (nodes[k]->right != &sentry) {
                    printf(coordinate"\\", row+1, col+1);
                }

                if (j==height) continue;
                if (nodes[k]->p != &sentry && nodes[k]->p->left == nodes[k]) {
                    printf(coordinate"/", row-1, col+1);
                    for(c=col+2; c<col+w/2;++c)
                        printf(coordinate"%c", row-2, c, '-');
                }
                if (nodes[k]->p != &sentry && nodes[k]->p->right == nodes[k]) {
                    printf(coordinate"\\", row-1, col);
                    for(c=col-1; c>col-w/2+1;--c)
                        printf(coordinate"%c", row-2, c, '-');
                }
            }
        }
        printf("\n");
    }
}
/**
 * printer version 2
 */
static void set_btree_buffer(rb_node_t *root, char *buff, long& offs)
{
    rb_node_t *p = root;

    if(p != &sentry) {
        offs += sprintf(buff+offs, "%d(%s)", p->key, p->color==RED?"R":"B");
        if (p->left == &sentry && p->right == &sentry) {
            //offs += sprintf(buff+offs, "]");
            return;
        }
        offs += sprintf(buff+offs, "[");
        if (p->left) {
            set_btree_buffer(p->left, buff, offs);
            offs += sprintf(buff+offs, ",");
        }

        if (p->right){
            set_btree_buffer(p->right, buff, offs);
            offs += sprintf(buff+offs, "]");
        }
    }
}

void print_rb_tree(rb_node_t *root)
{
    int i,j;
    int indent = 0, level = 0;
    char node[32] = {0};
    char buff[1024];
    long offs = 0;

    set_btree_buffer(root, buff, offs);
    printf("TREE:  %s\n", buff);

    for(i=0,j=0; i<offs; ) {
        char c = buff[i];
        switch(c) {
        case '[':
            level++;
            node[j++] = c;
            if (buff[i+1] == ',') {
                node[j++] = ',';
                i++;
            }

            printf("%*s%s\n", indent, "", node);
            memset(node, 0, sizeof(node));
            j=0;
            indent += 4;
            break;
        case ']':
            printf("%*s%s\n", indent, "", node);

            memset(node, 0, sizeof(node));
            j=0;
            indent -= 4;
            printf("%*s%c", indent, "", c);
            break;
        case ',':
            printf("%*s%s\n", indent, "", node);

            memset(node, 0, sizeof(node));
            j=0;
            break;
        default:
            node[j++] = c;
            break;
        }
        i++;
    }
}

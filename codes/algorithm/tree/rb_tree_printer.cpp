#include <math.h>
#include <string.h>

#include "btree.h"

#define bt_left(i)        2*((i)+1)-1
#define bt_right(i)       bt_left(i)+1
#define bt_parent(i)      ((i)-1)/2

static void build_info_tree(rb_node_t **array, int i, const rb_node_t *node)
{
    if (node != &sentry) {
        array[i] = (rb_node_t *)node;
        build_info_tree(array, bt_left(i), node->left);
        build_info_tree(array, bt_right(i), node->right);
    }
}

void print_complete_bt(const rb_node_t *root, int height)
{
#define coordinate "\033[%d;%dH"

    const int n = (int) pow(2, height) - 1;

    int min_width = 4; // the min width of each node
    int max_width = ((int) pow(2, height-1)) * min_width;

    int i,j,k;
    int s,e,w;
    int c,row,col,color;
    rb_node_t *array[n];
    for(i=0; i<n; ++i) array[i] = NULL;
    build_info_tree(array, 0, root);

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
            if (array[k]) {
                color = (array[k]->color == RED) ? 101 : 100;
                printf(coordinate"\033[%dm%2d\033[0m",
                       row, col, color, array[k]->key);
                if (array[k]->left != &sentry) {
                    printf(coordinate"/", row+1, col);
                }
                if (array[k]->right != &sentry) {
                    printf(coordinate"\\", row+1, col+1);
                }

                if (j==height) continue;
                if (array[k]->p != &sentry && array[k]->p->left == array[k]) {
                    printf(coordinate"/", row-1, col+1);
                    for(c=col+2; c<col+w/2;++c)
                        printf(coordinate"%c", row-2, c, '-');
                }
                if (array[k]->p != &sentry && array[k]->p->right == array[k]) {
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
static void print_tree(rb_node_t *root, char *buff, long& offs)
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
            print_tree(p->left, buff, offs);
            offs += sprintf(buff+offs, ",");
        }

        if (p->right){
            print_tree(p->right, buff, offs);
            offs += sprintf(buff+offs, "]");
        }
    }
}

void print_rb_tree(rb_node_t *root)
{
    int i,j;
    int indent = 0, level = 0;

    char buff[1024];
    long offs = 0;

    char node[32] = {0};
    print_tree(root, buff, offs);
    printf("TREE:  %s\n", buff);

    for(i=0,j=0; i<offs; ++i) {
        char c = buff[i];
        switch(c) {
        case '[':
            level++;
            if (buff[i+1] == ',') {
                node[j++] = c;
                node[j++] = ',';
            }
            else if (buff[i+1] != ']') {
                node[j++] = c;
            }

            printf("%*s%s\n", indent, "", node);
            memset(node, 0, sizeof(node));
            j=0;
            indent += 4;
            break;
        case ']':
            indent -= 4;
            if (buff[i-1] != '[')
                printf("%*s%c\n", indent, "", c);
            break;
        case ',':
            break;
        default:
            node[j++] = c;
            break;
        }

    }
}

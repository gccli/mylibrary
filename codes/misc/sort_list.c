#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct ListNode {
    int val;
    struct ListNode *next;
};

void show( struct ListNode *head)
{
    struct ListNode *x;
    for(x=head; x; x=x->next) {
        printf("%d%s", x->val, x->next?"->":"");
    }
    printf("\n");
}

struct ListNode* merge(struct ListNode* l, struct ListNode* r)
{
    struct ListNode head;
    struct ListNode* p = &head;

    while(l && r) {
        if(l->val <= r->val) {//collect left first when left and right are equal;
            p->next = l;
            l = l->next;
        }  else   {
            p->next = r;
            r = r->next;
        }
        p = p->next;
    }
    p->next = (l == NULL ? r : l);

    return head.next; //return without the fake head;
}
struct ListNode* sortList(struct ListNode* head)
{
    if(head == NULL || head->next == NULL)
        return head;

    struct ListNode *s1 = head, *s2 = head->next;
    while(s2 && s2->next) //split the list into two halves;
    {
        s1 = s1->next;
        s2 = s2->next->next;
    }
    s2 = s1->next;
    s1->next = NULL;
    return merge(sortList(head), sortList(s2)); //merge two parts by invoking recursive method;
}

void create(struct ListNode **p, int val)
{
    struct ListNode *x = calloc(1, sizeof(*x));
    x->val = val;
    *p = x;
}

int main(int argc, char *argv[])
{
    int i;

    struct ListNode *head, *x;
    struct ListNode **p;
    create(&head, 1);

    x = head;
    p = &x->next;  create(p, 3); x = x->next;
    p = &x->next;  create(p, 7); x = x->next;
    p = &x->next;  create(p, 5); x = x->next;
    p = &x->next;  create(p, 4); x = x->next;
    p = &x->next;  create(p, 0); x = x->next;

    show(head);


    head = sortList(head);

    for(x=head; x; x=x->next) {
        printf("%d%s", x->val, x->next?"->":"");
    }
    printf("\n");
    return 0;
}

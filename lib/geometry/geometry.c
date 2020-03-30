#include "geometry.h"

DLNode *___dl_add(DLList *list, DLNode *node)
{
    if (list->first == NULL) {
        list->first = node;
        list->last = node;
        return node;
    }
    node->prev = list->last;
    node->next = NULL;
    list->last->next = node;
    list->last = node;
    return node;
}
void ___dl_remove(DLList *list, DLNode *node)
{
    if (node == list->first && node == list->last) {
        list->first = NULL;
        list->last = NULL;
    } else if (node == list->first) {
        list->first = list->first->next;
        list->first->prev = NULL;
    } else if (node == list->last) {
        list->last = list->last->prev;
        list->last->next = NULL;
    } else {
        node->prev->next = node->next;
        node->next->prev = node->prev;
    }
    free(node);
}

float tetrahedron_6_times_volume(vec3 a, vec3 b, vec3 c, vec3 d)
{
    float a1,a2,a3,a4;
    a1 = a.vals[0]; a2 = b.vals[0]; a3 = c.vals[0]; a4 = d.vals[0];
    float b1,b2,b3,b4;
    b1 = a.vals[1]; b2 = b.vals[1]; b3 = c.vals[1]; b4 = d.vals[1];
    float c1,c2,c3,c4;
    c1 = a.vals[2]; c2 = b.vals[2]; c3 = c.vals[2]; c4 = d.vals[2];
    float d1,d2,d3,d4;
    d1 = a.vals[3]; d2 = b.vals[3]; d3 = c.vals[3]; d4 = d.vals[3];

    return a1*(b2*(c3-c4) - b3*(c2-c4) + b4*(c2-c3))
         - a2*(b1*(c3-c4) - b3*(c1-c4) + b4*(c1-c3))
         + a3*(b1*(c2-c4) - b2*(c1-c4) + b4*(c1-c2))
         - a4*(b1*(c2-c3) - b2*(c1-c3) + b3*(c1-c2));

}

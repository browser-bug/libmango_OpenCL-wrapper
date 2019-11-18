#include "utils.h"
#include <string.h>
#include <assert.h>

void list_node_insert_before(struct list_node *node, struct list_node *the_new)
{
  list_node *before_node = node->p;
  the_new->p = before_node;
  the_new->n = node;
  node->p = the_new;
  before_node->n = the_new;
}

void list_node_insert_after(struct list_node *node, struct list_node *the_new)
{
  list_node *after_node = node->n;
  the_new->n = after_node;
  the_new->p = node;
  node->n = the_new;
  after_node->p = the_new;
}

void list_move(struct list_head *the_old, struct list_head *the_new)
{
  assert(list_empty(the_new));
  if (list_empty(the_old))
  {
    return;
  }

  memcpy(&the_new->head_node, &the_old->head_node, sizeof(list_node));
  the_new->head_node.n->p = &the_new->head_node;
  the_new->head_node.p->n = &the_new->head_node;
  list_init(the_old);
}

void list_merge(struct list_head *head, struct list_head *to_merge)
{
  if (list_empty(to_merge))
    return;

  list_node *merge_last_node = to_merge->head_node.p;
  list_node *merge_first_node = to_merge->head_node.n;

  merge_last_node->n = &head->head_node;
  merge_first_node->p = head->head_node.p;
  head->head_node.p->n = merge_first_node;
  head->head_node.p = merge_last_node;
  list_init(to_merge);
}

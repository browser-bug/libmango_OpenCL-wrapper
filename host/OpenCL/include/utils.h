#ifndef __UTILS_H__
#define __UTILS_H__

#include <stdio.h>

#include "cl.h"

/* Taken from beignet OpenCl implementation by Intel */

/* Define one list node. */
typedef struct list_node
{
  struct list_node *n;
  struct list_node *p;
} list_node;
typedef struct list_head
{
  list_node head_node;
} list_head;

static inline void list_node_init(list_node *node)
{
  node->n = node;
  node->p = node;
}
static inline int list_node_out_of_list(const struct list_node *node)
{
  return node->n == node;
}
static inline void list_init(list_head *head)
{
  head->head_node.n = &head->head_node;
  head->head_node.p = &head->head_node;
}
void list_node_insert_before(list_node *node, list_node *the_new);
void list_node_insert_after(list_node *node, list_node *the_new);
static inline void list_node_del(struct list_node *node)
{
  node->n->p = node->p;
  node->p->n = node->n;
  /* And all point to self for safe. */
  node->p = node;
  node->n = node;
}
static inline void list_add(list_head *head, list_node *the_new)
{
  list_node_insert_after(&head->head_node, the_new);
}
static inline void list_add_tail(list_head *head, list_node *the_new)
{
  list_node_insert_before(&head->head_node, the_new);
}
static inline int list_empty(const struct list_head *head)
{
  return head->head_node.n == &head->head_node;
}
/* Move the content from one head to another. */
void list_move(struct list_head *the_old, struct list_head *the_new);
/* Merge the content of the two lists to one head. */
void list_merge(struct list_head *head, struct list_head *to_merge);

#undef offsetof
#ifdef __compiler_offsetof
#define offsetof(TYPE, MEMBER) __compiler_offsetof(TYPE, MEMBER)
#else
#define offsetof(TYPE, MEMBER) ((size_t) & ((TYPE *)0)->MEMBER)
#endif
#define list_entry(ptr, type, member) ({                      \
      const typeof( ((type *)0)->member ) *__mptr = (ptr);    \
      (type *)( (char *)__mptr - offsetof(type,member) ); })

#define list_for_each(pos, head) \
  for (pos = (head)->head_node.n; pos != &((head)->head_node); pos = pos->n)

#define list_for_each_safe(pos, ne, head)                                   \
  for (pos = (head)->head_node.n, ne = pos->n; pos != &((head)->head_node); \
       pos = ne, ne = pos->n)

#endif /* __UTILS_H__ */
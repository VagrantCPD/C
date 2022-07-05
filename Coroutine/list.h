#pragma once

#include <type.h>

#define container_of(ptr, type, field) \
	((type *)((u64)(ptr) - (u64)(&(((type *)(0))->field))))

struct list_head {
	struct list_head *prev;
	struct list_head *next;
};

static inline void init_list_head(struct list_head *list)
{
	list->next = list;
	list->prev = list;
}

static inline void list_add(struct list_head *new, struct list_head *head)
{
	new->next = head->next;
	new->prev = head;
	head->next->prev = new;
	head->next = new;
}

static inline void list_append(struct list_head *new, struct list_head *head)
{
	struct list_head *tail = head->prev;
	return list_add(new, tail);
}

static inline void list_del(struct list_head *node)
{
	node->prev->next = node->next;
	node->next->prev = node->prev;
}

static inline bool list_empty(struct list_head *head)
{
	return (head->prev == head && head->next == head);
}

#define for_each_in_list(elem, type, field, head) \
	for (elem = container_of((head)->next, type, field); \
	     &((elem)->field) != (head); \
	     elem = container_of(((elem)->field).next, type, field))

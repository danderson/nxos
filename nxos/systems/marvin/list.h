/** @file list.h
 *  @brief Circularly linked list library.
 */

/* Copyright (C) 2007 the NxOS developers
 *
 * See AUTHORS for a full list of the developers.
 *
 * Redistribution of this file is permitted under
 * the terms of the GNU Public License (GPL) version 2.
 */

#ifndef __NXOS_MARVIN_LIST_H__
#define __NXOS_MARVIN_LIST_H__

#include "base/types.h"

/** Internal macro to insert an item before another item.
 *
 * This assumes that item is part of a working list.
 */
#define __mv_list_insert_before(before_this, item) ({ \
  (before_this)->prev->next = (item); \
  (item)->prev = (before_this)->prev; \
  (before_this)->prev = (item); \
  (item)->next = (before_this); \
})

/** Internal macro to insert an item after another item.
 *
 * This assumes that item is part of a working list.
 */
#define __mv_list_insert_after(after_this, item) ({ \
  (after_this)->next->prev = (item); \
  (item)->next = (after_this)->next; \
  (after_this)->next = (item); \
  (item)->prev = (after_this); \
})

/** Initialize an empty list. */
#define mv_list_init(list) \
  ((list) = NULL)

/** Initialize a single-item list. */
#define mv_list_init_singleton(list, item) ({ \
  (item)->next = (item)->prev = (item); \
  (list) = (item); \
})

/** Check if the given list is empty. */
#define mv_list_is_empty(list) \
  ((list) == NULL)

/** Return the head of the given list, or NULL if the list is empty. */
#define mv_list_get_head(list) \
  (list)

/** Return the tail of the given list, or NULL if the list is empty. */
#define mv_list_get_tail(list) \
  ((list) ? ((list)->prev) : NULL)

/** Insert @a item at the head of @list */
#define mv_list_add_head(list, item) ({ \
  if (list) \
    __mv_list_insert_before(list, item); \
  else \
    mv_list_init_singleton(list, item); \
  (list) = (item); \
})

/** Insert @a item at the tail of @list */
#define mv_list_add_tail(list, item) ({ \
  if (list) \
    __mv_list_insert_after(list, item); \
  else \
    mv_list_init_singleton(list, item); \
})

/** Remove @a item from @a list */
#define mv_list_remove(list, item) ({ \
  if (((item)->next == (item)) && ((item)->prev == (item))) { \
    (item)->next = (item)->prev = (list) = NULL; \
  } else {					 \
    (item)->prev->next = (item)->next; \
    (item)->next->prev = (item)->prev; \
    if ((item) == (list)) \
      (list) = (item)->next; \
    (item)->prev = (item)->next = NULL; \
  } \
})

/** Remove and return @a item from @a list */
#define mv_list_pop(list, item) ({ \
  typeof(list) __ret_elt = (item); \
  mv_list_remove(list, item); \
  __ret_elt; \
})

/** Remove and return the head of @a list */
#define mv_list_pop_head(list) ({ \
  typeof(list) __ret_elt; \
  if (list) \
    __ret_elt = mv_list_pop(list, list); \
  else \
    __ret_elt = NULL; \
  __ret_elt; \
})

/** Remove and return the tail of @a list */
#define mv_list_pop_tail(list) ({ \
  typeof(list) __ret_elt; \
  if (list) \
    __ret_elt = mv_list_pop(list, (list)->prev);        \
  else \
    __ret_elt = NULL; \
  __ret_elt; \
})

/** Rotate @a list forward. */
#define mv_list_rotate_forward(list) ({ \
  if (list) \
    (list) = (list)->next; \
})

#endif /* __NXOS_MARVIN_LIST_H__ */

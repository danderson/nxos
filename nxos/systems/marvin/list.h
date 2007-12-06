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
  if (((item)->next == (item)) && ((item)->prev == (item))) \
    (item)->next = (item)->prev = (list) = NULL; \
  else { \
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
  if (list) \
    mv_list_pop(list, list); \
  else \
    

/** Remove and return the tail of @a list */

#endif /* __NXOS_MARVIN_LIST_H__ */

Fall 2007 (indicative, no results yet):
TX52 (Personal Project - NxOS embedded operating system)
RE51 (Distributed Systems)
IA52 (Natural Language Processing)
IA54 (Multi-Agent Systems)
AG51 (Advanced Algorithms)
TR52 (Realtime Operating Systems)
HE09 (History of Science)

Spring 2007:
TX52 - A (Personal Project - NxOS embedded operating system)
LO51 - A (Systems and Database Administration)
LO52 - A (Embedded and mobile computing)
TR54 - A (Applications of Realtime Systems)
RE56 - A (Mobile Networking)
DR05 - A (Intellectual Property Law)
LE06 - A (American Civilization)

Fall 2006: (intern at Google)

Spring 2006:
MI43 - A (Microcontrollers and Embedded Programming)
LO41 - A (Operating System Fundamentals)
IN41 - B (Digital Signal Processing)
RE41 - B (Networking Fundamentals)
AG41 - A (Heuristics and Operational Research)
GO01 - C (Current Geopolitics)

Fall 2005:
IA41 - A (Artificial Intelligence and Data Representation)
BD40 - D (Introduction to databases)
MI41 - A (Digital Logic and Microprocessors)
LO43 - A (Introduction to Object-Oriented Programming)
AR01 - A (Art and Society)
LE04 - A (British History)

Spring 2005:
PH01 - C (Introduction to philosophy)
PS26 - C (Electromagnetism)
SY20 - E (Introduction to automatism)
TX20 - A (Personal project - a highly distributed online world)
MT26 - F (Series and functions of complex variables)

Fall 2004:
CE01 - A (Communication skills in English)
LO21 - B (Algorithmics and programming II)
LO22 - A (Introduction to Unix and Unix C programming)
MT25 - C (Applications of algebra and analysis to geometry)
PS27 - B (Thermodynamics)
SQ20 - B (Statistics and probability)

Spring 2004:
EC01 - B (Introduction to economy)
GE01 - C (Introduction to management and accounting)
LO10 - A (Introduction to computing)
LO11 - A (Algorithmics and programming I)
MT12 - C (Integration, linear algebra and functions of multiple variables)
PS11 - B (Point mechanics and geometric optics)

Fall 2003
MT11 - D (Basic algebra and analysis)
PS12 - C (Measurements and Electricity)
CM11 - C (General Chemistry)
TN10 - C (Mechanical Construction)
MP10 - B (Introduction to scientific reasoning)
LS00 - B (Introduction to Spanish)

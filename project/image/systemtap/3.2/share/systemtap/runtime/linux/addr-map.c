/* -*- linux-c -*- 
 * Map of addresses to disallow.
 * Copyright (C) 2005-2016 Red Hat Inc.
 *
 * This file is part of systemtap, and is free software.  You can
 * redistribute it and/or modify it under the terms of the GNU General
 * Public License (GPL); either version 2, or (at your option) any
 * later version.
 */

#ifndef _STAPLINUX_ADDR_MAP_C_
#define _STAPLINUX_ADDR_MAP_C_ 1

/** @file addr-map
 * @brief Implements functions used by the deref macro to blacklist
 * certain addresses.
 */

// PR12970 (populate runtime/bad-addr database) hasn't been
// fixed. Until we're ready to populate the bad address database,
// let's provide a "junior" version of lookup_bad_addr(), since that
// will allow us to skip grabbing the lock and searching the empty
// list.
//
// See <https://sourceware.org/bugzilla/show_bug.cgi?id=12970> for
// more details.

#include <linux/uaccess.h>

#ifndef PR12970

#if ! STP_PRIVILEGE_CONTAINS (STP_PRIVILEGE, STP_PR_STAPDEV) && \
    ! STP_PRIVILEGE_CONTAINS (STP_PRIVILEGE, STP_PR_STAPSYS)
#include <asm/processor.h> /* For TASK_SIZE */
#endif

static int
lookup_bad_addr(const int type, const unsigned long addr, const size_t size)
{
  /* Is this a valid memory access?
   * 
   * Note we're using access_ok() here. This is only supposed to be
   * called when we're in task context. Occasionally lookup_bad_addr()
   * gets called when not in task context. If in_task() exists, only
   * call access_ok() when we're in task context (otherwise we'll get
   * a kernel warning). If we aren't in task context, we'll just do a
   * range check.
   */
#if !defined(in_task)
  if (size == 0 || ULONG_MAX - addr < size - 1
      || !access_ok(type, (void *)addr, size))
    return 1;
#else
  if (size == 0 || ULONG_MAX - addr < size - 1
      || (in_task() && !access_ok(type, (void *)addr, size))
      || (!in_task()
#if defined(user_addr_max)
	  && ((user_addr_max() - size) < addr)
#endif
	      ))
    return 1;
#endif

#if ! STP_PRIVILEGE_CONTAINS (STP_PRIVILEGE, STP_PR_STAPDEV) && \
    ! STP_PRIVILEGE_CONTAINS (STP_PRIVILEGE, STP_PR_STAPSYS)
  /* Unprivileged users must not access memory while the context
     does not refer to their own process.  */
  if (! is_myproc ())
    return 1;
  /* Unprivileged users must not access kernel space memory.  */
  if (addr + size > TASK_SIZE)
    return 1;
#endif

  return 0;
}

#else /* PR12970 */

struct addr_map_entry
{
  unsigned long min;
  unsigned long max;
};

struct addr_map
{
  size_t size;
  struct addr_map_entry entries[0];
};

static STP_DEFINE_RWLOCK(addr_map_lock);
static struct addr_map* blackmap;

/* Find address of entry where we can insert a new one. */
static size_t
upper_bound(unsigned long addr, struct addr_map* map)
{
  size_t start = 0;
  size_t end = map->size;
  struct addr_map_entry *entry = 0;

  if (end == 0)
    return 0;
  do
    {
      size_t new_idx;
      if (addr < map->entries[start].min)
        return start;
      if (addr >= map->entries[end-1].max)
        return end;
      new_idx = (end + start) / 2;
      entry = &map->entries[new_idx];
      if (addr < entry->min)
        end = new_idx;
      else
        start = new_idx + 1;
    } while (end != start);
  return entry - &map->entries[0];
}

/* Search for an entry in the given map which overlaps the range specified by
   addr and size.  */
static struct addr_map_entry*
lookup_addr_aux(unsigned long addr, size_t size, struct addr_map* map)
{
  size_t start = 0;
  size_t end;

  /* No map? No matching entry. */
  if (!map)
    return 0;
  /* Empty map? No matching entry.  */
  if (map->size == 0)
    return 0;

  /* Use binary search on the sorted map entries.  */
  end = map->size;
  do
    {
      int entry_idx;
      struct addr_map_entry *entry = 0;
      /* If the target range is beyond those of the remaining entries in the
	 map, then a matching entry will not be found  */
      if (addr + size <= map->entries[start].min ||
	  addr >= map->entries[end - 1].max)
        return 0;
      /* Choose the map entry at the mid point of the remaining entries.  */
      entry_idx = (end + start) / 2;
      entry = &map->entries[entry_idx];
      /* If our range overlaps the range of this entry, then we have a match. */
      if (addr + size > entry->min && addr < entry->max)
        return entry;
      /* If our range is below the range of this entry, then cull the entries
	 above this entry, otherwise cull the entries below it.  */
      if (addr + size <= entry->min)
        end = entry_idx;
      else
        start = entry_idx + 1;
    } while (start < end);
  return 0;
}

#if ! STP_PRIVILEGE_CONTAINS (STP_PRIVILEGE, STP_PR_STAPDEV) && \
    ! STP_PRIVILEGE_CONTAINS (STP_PRIVILEGE, STP_PR_STAPSYS)
#include <asm/processor.h> /* For TASK_SIZE */
#endif

static int
lookup_bad_addr(const int type, const unsigned long addr, const size_t size)
{
  struct addr_map_entry* result = 0;
  unsigned long flags;

  /* Is this a valid memory access?  */
  if (size == 0 || ULONG_MAX - addr < size - 1
      || !access_ok(type, (void *)addr, size))
    return 1;

#if ! STP_PRIVILEGE_CONTAINS (STP_PRIVILEGE, STP_PR_STAPDEV) && \
    ! STP_PRIVILEGE_CONTAINS (STP_PRIVILEGE, STP_PR_STAPSYS)
  /* Unprivileged users must not access memory while the context
     does not refer to their own process.  */
  if (! is_myproc ())
    return 1;
  /* Unprivileged users must not access kernel space memory.  */
  if (addr + size > TASK_SIZE)
    return 1;
#endif

  /* Search for the given range in the black-listed map.  */
  stp_read_lock_irqsave(&addr_map_lock, flags);
  result = lookup_addr_aux(addr, size, blackmap);
  stp_read_unlock_irqrestore(&addr_map_lock, flags);
  if (result)
    return 1;
  else
    return 0;
}


static int
add_bad_addr_entry(unsigned long min_addr, unsigned long max_addr,
                   struct addr_map_entry** existing_min,
                   struct addr_map_entry** existing_max)
{
  struct addr_map* new_map = 0;
  struct addr_map* old_map = 0;
  struct addr_map_entry* min_entry = 0;
  struct addr_map_entry* max_entry = 0;
  struct addr_map_entry* new_entry = 0;
  size_t existing = 0;
  unsigned long flags;

  /* Loop allocating memory for a new entry in the map. */
  while (1)
    {
      size_t old_size = 0;
      stp_write_lock_irqsave(&addr_map_lock, flags);
      old_map = blackmap;
      if (old_map)
        old_size = old_map->size;
      /* Either this is the first time through the loop, or we
         allocated a map previous time, but someone has come in and
         added an entry while we were sleeping. */
      if (!new_map || (new_map && new_map->size < old_size + 1))
        {
          stp_write_unlock_irqrestore(&addr_map_lock, flags);
          if (new_map)
            {
	      _stp_kfree(new_map);
              new_map = 0;
            }
          new_map = _stp_kmalloc(sizeof(*new_map)
				 + sizeof(*new_entry) * (old_size + 1));
          if (!new_map)
            return -ENOMEM;
          new_map->size = old_size + 1;
        }
      else
        break;
    }
  if (!blackmap)
    {
      existing = 0;
    }
  else
    {
      min_entry = lookup_addr_aux(min_addr, 1, blackmap);
      max_entry = lookup_addr_aux(max_addr, 1, blackmap);
      if (min_entry || max_entry)
        {
          if (existing_min)
            *existing_min = min_entry;
          if (existing_max)
            *existing_max = max_entry;
          stp_write_unlock_irqrestore(&addr_map_lock, flags);
          _stp_kfree(new_map);
          return 1;
        }
      existing = upper_bound(min_addr, old_map);
    }
  new_entry = &new_map->entries[existing];
  new_entry->min = min_addr;
  new_entry->max = max_addr;
  if (old_map)
    {
      memcpy(&new_map->entries, old_map->entries,
             existing * sizeof(*new_entry));
      if (old_map->size > existing)
        memcpy(new_entry + 1, &old_map->entries[existing],
               (old_map->size - existing) * sizeof(*new_entry));
    }
  blackmap = new_map;
  stp_write_unlock_irqrestore(&addr_map_lock, flags);
  if (old_map)
    _stp_kfree(old_map);
  return 0;
}

static void
delete_bad_addr_entry(struct addr_map_entry* entry)
{
}

#endif /* PR12970 */

#endif /* _STAPLINUX_ADDR_MAP_C_ */

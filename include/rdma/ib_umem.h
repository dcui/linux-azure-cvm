/*
 * Copyright (c) 2007 Cisco Systems.  All rights reserved.
 *
 * This software is available to you under a choice of one of two
 * licenses.  You may choose to be licensed under the terms of the GNU
 * General Public License (GPL) Version 2, available from the file
 * COPYING in the main directory of this source tree, or the
 * OpenIB.org BSD license below:
 *
 *     Redistribution and use in source and binary forms, with or
 *     without modification, are permitted provided that the following
 *     conditions are met:
 *
 *      - Redistributions of source code must retain the above
 *        copyright notice, this list of conditions and the following
 *        disclaimer.
 *
 *      - Redistributions in binary form must reproduce the above
 *        copyright notice, this list of conditions and the following
 *        disclaimer in the documentation and/or other materials
 *        provided with the distribution.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef IB_UMEM_H
#define IB_UMEM_H

#include <linux/list.h>
#include <linux/scatterlist.h>
#include <linux/workqueue.h>
#include <rdma/ib_verbs.h>

struct ib_ucontext;
struct ib_umem_odp;

struct ib_umem {
	struct ib_device       *ibdev;
	struct mm_struct       *owning_mm;
	size_t			length;
	unsigned long		address;
	u32 writable : 1;
	u32 is_odp : 1;
	/* Placing at the end of the bitfield list is ABI preserving on LE */
	u32 is_peer : 1;
	struct work_struct	work;
	struct sg_table sg_head;
	int             nmap;
	unsigned int    sg_nents;
	unsigned int    page_shift;
};

typedef void (*umem_invalidate_func_t)(struct ib_umem *umem, void *priv);
enum ib_peer_mem_flags {
	IB_PEER_MEM_ALLOW = 1 << 0,
	IB_PEER_MEM_INVAL_SUPP = 1 << 1,
};

/* Returns the offset of the umem start relative to the first page. */
static inline int ib_umem_offset(struct ib_umem *umem)
{
	return umem->address & ~PAGE_MASK;
}

static inline size_t ib_umem_num_pages(struct ib_umem *umem)
{
	return (ALIGN(umem->address + umem->length, PAGE_SIZE) -
		ALIGN_DOWN(umem->address, PAGE_SIZE)) >>
	       PAGE_SHIFT;
}

#ifdef CONFIG_INFINIBAND_USER_MEM

struct ib_umem *ib_umem_get(struct ib_device *device, unsigned long addr,
			    size_t size, int access);
void ib_umem_release(struct ib_umem *umem);
int ib_umem_page_count(struct ib_umem *umem);
int ib_umem_copy_from(void *dst, struct ib_umem *umem, size_t offset,
		      size_t length);
unsigned long ib_umem_find_best_pgsz(struct ib_umem *umem,
				     unsigned long pgsz_bitmap,
				     unsigned long virt);

struct ib_umem *ib_umem_get_peer(struct ib_device *device, unsigned long addr,
				 size_t size, int access,
				 unsigned long peer_mem_flags);
void ib_umem_activate_invalidation_notifier(struct ib_umem *umem,
					    umem_invalidate_func_t func,
					    void *cookie);
void ib_umem_stop_invalidation_notifier(struct ib_umem *umem);

#else /* CONFIG_INFINIBAND_USER_MEM */

#include <linux/err.h>

static inline struct ib_umem *ib_umem_get(struct ib_device *device,
					  unsigned long addr, size_t size,
					  int access)
{
	return ERR_PTR(-EINVAL);
}
static inline void ib_umem_release(struct ib_umem *umem) { }
static inline int ib_umem_page_count(struct ib_umem *umem) { return 0; }
static inline int ib_umem_copy_from(void *dst, struct ib_umem *umem, size_t offset,
		      		    size_t length) {
	return -EINVAL;
}
static inline unsigned long ib_umem_find_best_pgsz(struct ib_umem *umem,
						   unsigned long pgsz_bitmap,
						   unsigned long virt)
{
	return 0;
}

static inline struct ib_umem *ib_umem_get_peer(struct ib_device *device,
					       unsigned long addr, size_t size,
					       int access,
					       unsigned long peer_mem_flags)
{
	return ERR_PTR(-EINVAL);
}

static inline void ib_umem_activate_invalidation_notifier(
	struct ib_umem *umem, umem_invalidate_func_t func, void *cookie)
{
}

static inline void ib_umem_stop_invalidation_notifier(struct ib_umem *umem)
{
}

#endif /* CONFIG_INFINIBAND_USER_MEM */

#endif /* IB_UMEM_H */

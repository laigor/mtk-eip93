// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2019
 *
 * Richard van Schagen <vschagen@cs.com>
 */

#include "eip93-common.h"
#include "eip93-core.h"


inline int mtk_ring_first_cdr_index(struct mtk_device *mtk)
{
	struct mtk_desc_ring *cdr  = &mtk->ring[0].cdr;

	return (cdr->read - cdr->base) / cdr->offset;
}

inline int mtk_ring_curr_wptr_index(struct mtk_device *mtk)
{
	struct mtk_desc_ring *cdr  = &mtk->ring[0].cdr;

	return (cdr->write - cdr->base) / cdr->offset;
}

inline int mtk_ring_curr_rptr_index(struct mtk_device *mtk)
{
	struct mtk_desc_ring *rdr  = &mtk->ring[0].rdr;

	return (rdr->read - rdr->base) / rdr->offset;
}

inline int mtk_ring_cdr_index(struct mtk_device *mtk,
				struct eip93_descriptor_s *cdesc)
{
	struct mtk_desc_ring *cdr = &mtk->ring[0].cdr;

	return ((void *)cdesc - cdr->base) / cdr->offset;
}

void *mtk_ring_next_wptr(struct mtk_device *mtk, struct mtk_desc_ring *ring)
{
	void *ptr = ring->write;

	if ((ring->write == ring->read - ring->offset) ||
		(ring->read == ring->base && ring->write == ring->base_end))
		return ERR_PTR(-ENOMEM);
 
	if (ring->write == ring->base_end)
		ring->write = ring->base;
	else
		ring->write += ring->offset;
	
	return ptr;
}

void *mtk_ring_next_rptr(struct mtk_device *mtk, struct mtk_desc_ring *ring)
{
	void *ptr = ring->read;

	if (ring->write == ring->read)
		return ERR_PTR(-ENOENT);

	if (ring->read == ring->base_end)
		ring->read = ring->base;
	else
		ring->read += ring->offset;

	return ptr;
}

void mtk_ring_rollback_wptr(struct mtk_device *mtk,
				 struct mtk_desc_ring *ring)
{
	if (ring->write == ring->read)
		return;

	if (ring->write == ring->base)
		ring->write = ring->base_end - ring->offset;
	else
		ring->write -= ring->offset;
}

inline void *mtk_ring_curr_wptr(struct mtk_device *mtk)
{
	struct mtk_desc_ring *cdr  = &mtk->ring[0].cdr;

	return cdr->write;
}

inline void *mtk_ring_curr_rptr(struct mtk_device *mtk)
{
	struct mtk_desc_ring *rdr  = &mtk->ring[0].rdr;

	return rdr->read;
}

struct eip93_descriptor_s *mtk_add_cdesc(struct mtk_device *mtk,
					struct mtk_dma_rec *rec, dma_addr_t saRecord_base,
					dma_addr_t saState_base)
{
	struct eip93_descriptor_s *cdesc;

	cdesc = mtk_ring_next_wptr(mtk, &mtk->ring[0].cdr);
	if (IS_ERR(cdesc))
		return cdesc;

	memset(cdesc, 0, sizeof(struct eip93_descriptor_s));

	cdesc->peCrtlStat.bits.hostReady = 1;
	cdesc->peCrtlStat.bits.hashFinal = 0;
	cdesc->peCrtlStat.bits.padCrtlStat = 0; //padCrtlStat; pad boundary
	cdesc->peCrtlStat.bits.peReady = 0;
	cdesc->srcAddr = rec->srcDma;
	cdesc->dstAddr = rec->dstDma;
	cdesc->saAddr = saRecord_base;
	cdesc->stateAddr = saState_base;
	cdesc->arc4Addr = saState_base;
	cdesc->peLength.bits.length = (rec->dmaLen) & GENMASK(20, 0);
	cdesc->peLength.bits.hostReady = 1;

	return cdesc;
}

struct eip93_descriptor_s *mtk_add_rdesc(struct mtk_device *mtk)
{
	struct eip93_descriptor_s *rdesc;

	rdesc = mtk_ring_next_wptr(mtk, &mtk->ring[0].rdr);
	if (IS_ERR(rdesc))
		return rdesc;

	memset(rdesc, 0, sizeof(struct eip93_descriptor_s));

	return rdesc;
}

/*
 * This file is subject to the terms of the GFX License. If a copy of
 * the license was not distributed with this file, you can obtain one at:
 *
 *              http://chibios-gfx.com/license.html
 */

/**
 * @file    src/gqueue/gqueue.c
 * @brief   GQUEUE source file.
 */
#if GFX_USE_GQUEUE

#if GQUEUE_NEED_ASYNC
	void gfxQueueASyncInit(gfxQueueASync *pqueue) {
		pqueue->head = pqueue->tail = 0;
	}
	gfxQueueASyncItem *gfxQueueASyncGet(gfxQueueASync *pqueue) {
		gfxQueueASyncItem	*pi;

		if (!pqueue->head) return 0;
		gfxSystemLock();
		if ((pi = pqueue->head))
			pqueue->head = pi->next;
		gfxSytemUnlock();
		return pi;
	}
	void gfxQueueASyncPut(gfxQueueASync *pqueue, gfxQueueASyncItem *pitem) {
		pitem->next = 0;

		gfxSystemLock();
		if (!pqueue->head) {
			pqueue->head = pqueue->tail = pitem;
		} else {
			pqueue->tail->next = pitem;
			pqueue->tail = pitem;
		}
		gfxSystemUnlock();
	}
	void gfxQueueASyncPush(gfxQueueASync *pqueue, gfxQueueASyncItem *pitem) {
		gfxSystemLock();
		pitem->next = pqueue->head;
		pqueue->head = pitem;
		if (!pitem->next)
			pqueue->tail = pitem;
		gfxSystemUnlock();
	}
	void gfxQueueASyncRemove(gfxQueueASync *pqueue, gfxQueueASyncItem *pitem) {
		if (!pitem) return;
		gfxSystemLock();
		if (pqueue->head) {
			if (pqueue->head == pitem) {
				pqueue->head = pitem->next;
			} else {
				for(gfxQueueASyncItem *pi = pqueue->head; pi->next; pi = pi->next) {
					if (pi->next == pitem) {
						pi->next = pitem->next;
						if (pqueue->tail == pitem)
							pqueue->tail = pi;
						break;
					}
				}
			}
		}
		gfxSystemUnlock();
	}
	bool_t gfxQueueASyncIsEmpty(gfxQueueASync *pqueue) {
		return pqueue->head == NULL;
	}
	bool_t gfxQueueASyncIsIn(gfxQueueASync *pqueue, gfxQueueASyncItem *pitem) {
		gfxSystemLock();
		for(gfxQueueASyncItem *pi = pqueue->head; pi; pi = pi->next) {
			if (pi == pitem) {
				gfxSystemUnlock();
				return TRUE;
			}
		}
		gfxSystemUnlock();
		return FALSE;
	}
#endif

#if GQUEUE_NEED_GSYNC
	void gfxQueueGSyncInit(gfxQueueGSync *pqueue) {
		pqueue->head = pqueue->tail = 0;
		gfxSemInit(&pqueue->sem, 0, MAX_SEMAPHORE_COUNT);
	}
	gfxQueueGSyncItem *gfxQueueGSyncGet(gfxQueueGSync *pqueue, delaytime_t ms) {
		gfxQueueGSyncItem	*pi;

		if (!gfxSemWait(&pqueue->sem, ms)) return 0;
		gfxSystemLock();
		pi = pqueue->head;
		pqueue->head = pi->next;
		gfxSytemUnlock();
		return pi;
	}
	void gfxQueueGSyncPut(gfxQueueGSync *pqueue, gfxQueueGSyncItem *pitem) {
		pitem->next = 0;

		gfxSystemLock();
		if (!pqueue->head) {
			pqueue->head = pqueue->tail = pitem;
		} else {
			pqueue->tail->next = pitem;
			pqueue->tail = pitem;
		}
		gfxSystemUnlock();

		gfxSemSignal(&pqueue->sem);
	}
	void gfxQueueGSyncPush(gfxQueueGSync *pqueue, gfxQueueGSyncItem *pitem) {
		gfxSystemLock();
		pitem->next = pqueue->head;
		pqueue->head = pitem;
		if (!pitem->next)
			pqueue->tail = pitem;
		gfxSystemUnlock();

		gfxSemSignal(&pqueue->sem);
	}
	void gfxQueueGSyncRemove(gfxQueueGSync *pqueue, gfxQueueGSyncItem *pitem) {
		if (!pitem) return;
		gfxSystemLock();
		if (pqueue->head) {
			if (pqueue->head == pitem) {
				pqueue->head = pitem->next;
			} else {
				for(gfxQueueGSyncItem *pi = pqueue->head; pi->next; pi = pi->next) {
					if (pi->next == pitem) {
						pi->next = pitem->next;
						if (pqueue->tail == pitem)
							pqueue->tail = pi;
						break;
					}
				}
			}
		}
		gfxSystemUnlock();
	}
	bool_t gfxQueueGSyncIsEmpty(gfxQueueGSync *pqueue) {
		return pqueue->head == NULL;
	}
	bool_t gfxQueueGSyncIsIn(gfxQueueGSync *pqueue, gfxQueueGSyncItem *pitem) {
		gfxSystemLock();
		for(gfxQueueGSyncItem *pi = pqueue->head; pi; pi = pi->next) {
			if (pi == pitem) {
				gfxSystemUnlock();
				return TRUE;
			}
		}
		gfxSystemUnlock();
		return FALSE;
	}
#endif

#if GQUEUE_NEED_FSYNC
	void gfxQueueFSyncInit(gfxQueueFSync *pqueue) {
		pqueue->head = pqueue->tail = 0;
		gfxSemInit(&pqueue->sem, 0, MAX_SEMAPHORE_COUNT);
	}
	gfxQueueFSyncItem *gfxQueueFSyncGet(gfxQueueFSync *pqueue, delaytime_t ms) {
		gfxQueueFSyncItem	*pi;

		if (!gfxSemWait(&pqueue->sem, ms)) return 0;
		gfxSystemLock();
		pi = pqueue->head;
		pqueue->head = pi->next;
		gfxSytemUnlock();

		gfxSemSignalI(&pi->sem);
		gfxSemDestroy(&pi->sem);
		return pi;
	}
	bool_t gfxQueueFSyncPut(gfxQueueFSync *pqueue, gfxQueueFSyncItem *pitem, delaytime_t ms) {
		gfxSemInit(&pitem->sem, 0, 1);
		pitem->next = 0;

		gfxSystemLock();
		if (!pqueue->head) {
			pqueue->head = pqueue->tail = pitem;
		} else {
			pqueue->tail->next = pitem;
			pqueue->tail = pitem;
		}
		gfxSystemUnlock();

		gfxSemSignal(&pqueue->sem);
		return gfxSemWait(&pitem->sem, ms);
	}
	bool_t gfxQueueFSyncPush(gfxQueueFSync *pqueue, gfxQueueFSyncItem *pitem, delaytime_t ms) {
		gfxSemInit(&pitem->sem, 0, 1);

		gfxSystemLock();
		pitem->next = pqueue->head;
		pqueue->head = pitem;
		if (!pitem->next)
			pqueue->tail = pitem;
		gfxSystemUnlock();

		gfxSemSignal(&pqueue->sem);
		return gfxSemWait(&pitem->sem, ms);
	}
	void gfxQueueFSyncRemove(gfxQueueFSync *pqueue, gfxQueueFSyncItem *pitem) {
		if (!pitem) return;
		gfxSystemLock();
		if (pqueue->head) {
			if (pqueue->head == pitem) {
				pqueue->head = pitem->next;
			found:
				gfxSystemUnlock();
				gfxSemSignal(&pitem->sem);
				gfxSemDestroy(&pitem->sem);
				return;
			}
			for(gfxQueueFSyncItem *pi = pqueue->head; pi->next; pi = pi->next) {
				if (pi->next == pitem) {
					pi->next = pitem->next;
					if (pqueue->tail == pitem)
						pqueue->tail = pi;
					goto found;
				}
			}
		}
		gfxSystemUnlock();
	}
	bool_t gfxQueueFSyncIsEmpty(gfxQueueFSync *pqueue) {
		return pqueue->head == NULL;
	}
	bool_t gfxQueueFSyncIsIn(gfxQueueFSync *pqueue, gfxQueueFSyncItem *pitem) {
		gfxSystemLock();
		for(gfxQueueFSyncItem *pi = pqueue->head; pi; pi = pi->next) {
			if (pi == pitem) {
				gfxSystemUnlock();
				return TRUE;
			}
		}
		gfxSystemUnlock();
		return FALSE;
	}
#endif

#endif /* GFX_USE_GQUEUE */

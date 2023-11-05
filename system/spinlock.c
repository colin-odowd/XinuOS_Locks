/* spinlock.c - spinlock variant */

#include <xinu.h>

/*------------------------------------------------------
 *  spinlock  -  Implementation of the spinlock variant
 *------------------------------------------------------
 */
syscall sl_initlock(sl_lock_t *l)
{
    l->flag = 0;
    return OK;
}

syscall sl_lock(sl_lock_t *l)
{
    while (test_and_set(&l->flag, 1) == 1);
    l->owner = currpid;
    return OK;
}

syscall sl_unlock(sl_lock_t *l)
{
    if (l->owner == currpid)
    {
        l->flag = 0;
        return OK;
    }
    else
    {
        return SYSERR;
    }
}


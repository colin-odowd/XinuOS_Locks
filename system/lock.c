/* lock.c - sleep&guard lock variant */

#include <xinu.h>

qid16	locklist;			/* Index of lock list */

/*----------------------------------------------------------
 *  lock  -  Implementation of the sleep&guard lock variant
 *----------------------------------------------------------
 */
syscall initlock(lock_t *l)
{
    l->flag = 0;
    l->guard = 0;
    l->owner = NO_LOCK;
    l->q = queuehead(locklist);
    return OK;
}

syscall lock(lock_t *l)
{
    struct	procent *prptr;	

    while (test_and_set(&l->guard, 1) == 1) sleepms(QUANTUM);
    if (l->flag == 0) 
    {
        l->flag = 1; 
        l->owner = currpid;
        l->guard = 0;
    }
    else
    {
        enqueue(currpid, l->q);
        prptr = &proctab[currpid];
	    prptr->prlockqueue = 1;
        setpark();
        l->guard = 0;
        park();
        l->owner = currpid;
    }
    return OK;
}

syscall unlock(lock_t *l)
{
    struct	procent *prptr;	

    while (test_and_set(&l->guard, 1) == 1) sleepms(QUANTUM);
    if (l->owner == currpid)
    {
        if (isempty(l->q))
        {
            l->flag = 0;
        }
        else 
        {
            prptr = &proctab[firstid(l->q)];
            unpark(dequeue(l->q));
            prptr->prlockqueue = 0;
        }
        l->guard = 0;
        return OK;
    }
    else 
    {
        l->guard = 0;
        return SYSERR;
    }
}


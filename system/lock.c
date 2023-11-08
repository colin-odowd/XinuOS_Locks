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
    l->owner = NO_OWNER;
    l->q = queuehead(locklist);
    return OK;
}

syscall lock(lock_t *l)
{
    while (test_and_set(&l->guard, 1) == 1) sleepms(QUANTUM);
    if (l->flag == 0) 
    {
        //kprintf("L %d\n", currpid);
        l->flag = 1; 
        l->guard = 0;
        l->owner = currpid;
    }
    else
    {
        print_lock_list(l->q);
        insert_lock(currpid, l->q, 0);
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
        if (isempty(l->q) == 0)
        {
            //kprintf("UE %d\n", currpid);
            l->flag = 0;
        }
        else 
        {
            //kprintf("UU %d\n", currpid);
            unpark(dequeue(l->q));

            prptr = &proctab[currpid];
            prptr->prlockqueue = 0;

        }
        l->guard = 0;
        return OK;
    }
    else 
    {
        //kprintf("UF %d %d\n", currpid, l->owner);
        l->guard = 0;
        return SYSERR;
    }
}


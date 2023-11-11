/* pi_lock.c - priority inversion lock variant */

#include <xinu.h>

intmask	mask; 

/*-----------------------------------------------------------------
 *  pi_initlock  -  Initialize the priority inversion lock variant 
 *-----------------------------------------------------------------
 */
syscall pi_initlock(pi_lock_t *l)
{
    static uint32 pi_lock_count = 0;
    static int32 lock_id = 0;
    
    if (++pi_lock_count >= NPILOCKS) return SYSERR;

    l->flag = 0;
    l->guard = 0;
    l->owner = NO_LOCK;
    l->id = lock_id++;
    l->q = queuehead(locklist);

    mask = disable();
    pi_lock_array[pi_lock_count] = l;
    restore(mask);

    return OK;
}

/*----------------------------------------------------------
 *  pi_lock  -  Lock the priority inversion lock variant 
 *----------------------------------------------------------
 */
syscall pi_lock(pi_lock_t *l)
{
    #define NO_PID (-1)

    struct	procent *prptr;	
    struct	procent *owner;	

    prptr = &proctab[currpid];
	
    qid16	curr;
    uint32  lock_owner;
    pri16   prev_prio;

    while (test_and_set(&l->guard, 1) == 1) sleepms(QUANTUM);
    if (l->flag == 0) 
    {
        l->flag = 1; 
        l->guard = 0;
        l->owner = currpid;
        mask = disable();
        pi_lock_array[l->id] = l;
        restore(mask);
        prptr->prlockid_waiting = NO_LOCK;
    }
    else
    {
        enqueue(currpid, l->q);
        prptr = &proctab[currpid];
	    prptr->prlockqueue = 1;
        prptr->prlockid_waiting = l->id;

        lock_owner = pi_lock_array[l->id]->owner;
        owner = &proctab[lock_owner];
        prev_prio = owner->prprio;

        curr = firstid(l->q);
        while (curr != queuetail(l->q))
        {
            prptr = &proctab[curr];
            if (prptr->prprio > owner->prprio) 
            {
                owner->prprio = prptr->prprio;
            }
            curr = queuetab[curr].qnext;
        }

        if (owner->prprio != prev_prio)
        {
            kprintf("priority_change=P%d::%d-%d\n", currpid, owner->prprio_base, owner->prprio);
        }

        mask = disable();
        setpark();
        l->guard = 0;
        park();
        restore(mask);
    }
    return OK;
}

/*----------------------------------------------------------
 *  pi_unlock  -  Unlock the priority inversion lock variant 
 *----------------------------------------------------------
 */
syscall pi_unlock(pi_lock_t *l)
{
    struct	procent *prptr;
    prptr = &proctab[currpid];

    while (test_and_set(&l->guard, 1) == 1) sleepms(QUANTUM);
    if (l->owner == currpid)
    {
        if (isempty(l->q))
        {
            l->flag = 0;
            l->owner = NO_LOCK;
            mask = disable();
            pi_lock_array[l->id] = l;
            restore(mask);
            prptr->prlockqueue = 0;
            prptr->prlockid_waiting = NO_LOCK;
            
        }
        else 
        {
            prptr->prlockqueue = 0;
            prptr->prlockid_waiting = NO_LOCK;  
            prptr = &proctab[firstid(l->q)];
            l->owner = currpid;
            mask = disable();
            pi_lock_array[l->id] = l;
            restore(mask);
            unpark(dequeue(l->q));
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

/* active_lock.c - active lock variant */

#include <xinu.h>

/*----------------------------------------------------------
 *  al_initlock  -  Initialize the active lock variant 
 *----------------------------------------------------------
 */
syscall al_initlock(al_lock_t *l)
{
    static uint32 active_lock_count = 0;
    static uint32 lock_id = 1;
    
    if (++active_lock_count >= NALOCKS) return SYSERR;

    l->flag = 0;
    l->guard = 0;
    l->owner = NO_OWNER;
    l->id = lock_id++;
    l->q = queuehead(locklist);

    active_lock_array[active_lock_count] = l;

    return OK;
}

/*----------------------------------------------------------
 *  al_lock  -  Lock the active lock variant 
 *----------------------------------------------------------
 */
syscall al_lock(al_lock_t *l)
{
    struct	procent *prptr;	
    struct	procent *curr;	

    prptr = &proctab[currpid];

    uint32  i;
    pid32 deadlock_pids[NPROC]; 
    uint32 deadlock_count = 0; 

    while (test_and_set(&l->guard, 1) == 1) sleepms(QUANTUM);
    if (l->flag == 0) 
    {
        l->flag = 1; 
        l->guard = 0;
        l->owner = currpid;
        active_lock_array[l->id] = l;

        prptr->prlockid_holding = l->id;
        prptr->prlockid_waiting = NO_LOCK;
    }
    else
    {
        /* This PID is going to sleep at this point, need to check queues of other locks to see if
        they are waiting on this PID */

        enqueue(currpid, l->q);
        prptr = &proctab[currpid];
	    prptr->prlockqueue = 1;
        prptr->prlockid_waiting = l->id;

        if (prptr->prlockid_holding != NO_LOCK)
        {
            
            for (i = 0; i < NPROC; i++)
            {
                curr = &proctab[i];
                if ((curr->prlockid_waiting == prptr->prlockid_holding) &&  
                    (i != currpid))
                {
                    deadlock_pids[deadlock_count++] = i; 
                }
            }

            if (deadlock_count > 0) 
            {
                kprintf("deadlock_detected=");
                for (i = 0; i < deadlock_count; i++)
                {
                    if (i == (deadlock_count-1))
                    {
                        kprintf("P%d", deadlock_pids[i]);
                    }
                    else 
                    {
                        kprintf("P%d-", deadlock_pids[i]);
                    }
                    
                    kprintf("-P%d", currpid);

                }
                kprintf("\n", deadlock_pids[i]);
                return OK; 
            }
        }
        setpark();
        l->guard = 0;
        park();
        l->owner = currpid;
        active_lock_array[(l->id)-1] = l;
    }
    return OK;
}

/*----------------------------------------------------------
 *  al_unlock  -  Unlock the active lock variant 
 *----------------------------------------------------------
 */
syscall al_unlock(al_lock_t *l)
{
    uint32  i;
    struct	procent *prptr;
    prptr = &proctab[currpid];

    while (test_and_set(&l->guard, 1) == 1) sleepms(QUANTUM);
    if (l->owner == currpid)
    {
        for (i = NALOCKS; i > 0; i--)
        {
            if ((active_lock_array[i]->owner == currpid) &&
                (active_lock_array[i] != NULL))
            {
                prptr->prlockqueue = 0;
                prptr->prlockid_holding = i;
                prptr->prlockid_waiting = NO_LOCK;
            }
        }

        if (isempty(l->q))
        {
            l->flag = 0;
            l->owner = NO_LOCK;
            active_lock_array[l->id] = l;

            prptr->prlockid_holding = NO_LOCK;
            prptr->prlockqueue = 0;
            prptr->prlockid_waiting = NO_LOCK;
        }
        else 
        {
            prptr = &proctab[firstid(l->q)];
            unpark(dequeue(l->q));
            prptr->prlockqueue = 0;
            prptr->prlockid_holding = l->id;
            prptr->prlockid_waiting = NO_LOCK;
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

/*----------------------------------------------------------
 *  al_trylock  -  Try to obtain the active lock variant 
 *----------------------------------------------------------
 */
bool8 al_trylock(al_lock_t *l)
{
    while (test_and_set(&l->guard, 1) == 1) sleepms(QUANTUM);
    
    if (l->flag == 1) 
    {
        l->guard = 0;
        return FALSE;    
    }
    else 
    {
        l->flag = 1; 
        l->guard = 0;
        l->owner = currpid;        
        return TRUE;
    }
}
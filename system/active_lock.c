/* active_lock.c - active lock variant */

#include <xinu.h>

intmask	mask; 
uint32 deadlock_detected = 0;    

/*----------------------------------------------------------
 *  al_initlock  -  Initialize the active lock variant 
 *----------------------------------------------------------
 */
syscall al_initlock(al_lock_t *l)
{
    static uint32 active_lock_count = 0;
    static int32 lock_id = 0;
    
    if (++active_lock_count >= NALOCKS) return SYSERR;

    l->flag = 0;
    l->guard = 0;
    l->owner = NO_LOCK;
    l->id = lock_id++;
    l->q = queuehead(locklist);

    mask = disable();
    active_lock_array[l->id] = l;
    restore(mask);

    return OK;
}

/*----------------------------------------------------------
 *  al_lock  -  Lock the active lock variant 
 *----------------------------------------------------------
 */
syscall al_lock(al_lock_t *l)
{
    #define NO_PID (-1)

    struct	procent *prptr;	
    prptr = &proctab[currpid];
    
    uint32  i;
    bool8   currpid_first = TRUE;
    pid32   lock_id;
    pid32   temp_pid = NO_PID;
    pid32   deadlock_pids[NPROC]; 
    uint32  deadlock_count = 0; 

    while (test_and_set(&l->guard, 1) == 1) sleepms(QUANTUM);
    if (l->flag == 0) 
    {
        l->flag = 1; 
        l->owner = currpid;
        mask = disable();
        active_lock_array[l->id] = l;
        restore(mask);
        prptr->prlockid_waiting = NO_LOCK;
        l->guard = 0;
    }
    else
    {
        enqueue(currpid, l->q);
        prptr = &proctab[currpid];
	    prptr->prlockqueue = 1;
        prptr->prlockid_waiting = l->id;

        mask = disable();
        while (((prptr->prlockid_waiting != NO_LOCK) && 
                (temp_pid != currpid)) &&
                (temp_pid != (NPROC+1)))
        {   
            lock_id =  prptr->prlockid_waiting;
            temp_pid = active_lock_array[lock_id]->owner;

            for(i = 0; i < deadlock_count; i++) 
            {
                if (temp_pid == deadlock_pids[i]) 
                {
                    temp_pid = NPROC+1;
                }
            }

            prptr = &proctab[temp_pid]; 
            if (temp_pid != currpid) 
            {
                deadlock_pids[deadlock_count++] = temp_pid;
            }
        }
        
        if (temp_pid == currpid)
        {
            if (currpid > deadlock_pids[deadlock_count-1])
            {
                currpid_first = FALSE;
            }

            kprintf("deadlock_detected=");
            if (currpid_first) kprintf("P%d-", currpid);
            for (i = 0; i < deadlock_count; i++) 
            {
                kprintf("P%d", deadlock_pids[i]);
                if (i != (deadlock_count-1)) 
                {
                    kprintf("-");
                }
                else if (!currpid_first)
                {
                    kprintf("-");
                }
            }
            if (!currpid_first) kprintf("P%d", currpid);
            kprintf("\n");
        }

        restore(mask);
        setpark();
        l->guard = 0;
        park();

    }
    return OK;
}

/*----------------------------------------------------------
 *  al_unlock  -  Unlock the active lock variant 
 *----------------------------------------------------------
 */
syscall al_unlock(al_lock_t *l)
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
            active_lock_array[l->id] = l;
            restore(mask);
            prptr->prlockqueue = 0;
        }
        else 
        {
            // if (currpid >= 22)
            // {
            //     int i;
            //     for (i=0; i<NALOCKS; i++)
            //     {
            //         kprintf("Lock: %d %d %d\n", active_lock_array[i]->id, active_lock_array[i]->owner, currpid);
            //     }
            // }

            prptr->prlockqueue = 0;
            prptr->prlockid_waiting = NO_LOCK;  
            prptr = &proctab[firstid(l->q)];
            l->owner = firstid(l->q);
            mask = disable();
            active_lock_array[l->id] = l;
            unpark(dequeue(l->q));
            restore(mask);
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
        l->owner = currpid;
        mask = disable();
        active_lock_array[l->id] = l;
        restore(mask);
        l->guard = 0;
        return TRUE;
    }
}

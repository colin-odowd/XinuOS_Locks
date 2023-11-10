/* active_lock.c - active lock variant */

#include <xinu.h>

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

    active_lock_array[active_lock_count] = l;

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
        l->guard = 0;
        l->owner = currpid;
        active_lock_array[l->id] = l;

        prptr->prlockid_holding = l->id;
        prptr->prlockid_waiting = NO_LOCK;
    }
    else
    {
        enqueue(currpid, l->q);
        prptr = &proctab[currpid];
	    prptr->prlockqueue = 1;
        prptr->prlockid_waiting = l->id;

        while ((prptr->prlockid_waiting != NO_LOCK) && 
               (temp_pid != currpid))
        {   
            lock_id =  prptr->prlockid_waiting;
            temp_pid = active_lock_array[lock_id]->owner;
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

        setpark();
        l->guard = 0;
        park();
        l->owner = currpid;
        active_lock_array[l->id] = l;
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
            active_lock_array[l->id] = l;
            prptr->prlockqueue = 0;
            prptr->prlockid_holding = NO_LOCK;
            prptr->prlockid_waiting = NO_LOCK;
        }
        else 
        {
            prptr->prlockqueue = 0;
            prptr->prlockid_waiting = NO_LOCK;  

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
        l->owner = currpid;
        active_lock_array[l->id] = l;
        l->guard = 0;
        return TRUE;
    }
}

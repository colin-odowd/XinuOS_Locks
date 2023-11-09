/* active_lock.c - active lock variant */

#include <xinu.h>

/*----------------------------------------------------------
 *  al_initlock  -  Initialize the active lock variant 
 *----------------------------------------------------------
 */
syscall al_initlock(al_lock_t *l)
{
    static uint32 lock_id = 1;

    l->flag = 0;
    l->guard = 0;
    l->owner = NO_OWNER;
    l->id = lock_id++;
    l->q = queuehead(locklist);
    return OK;
}

/*----------------------------------------------------------
 *  al_lock  -  Lock the active lock variant 
 *----------------------------------------------------------
 */
syscall al_lock(al_lock_t *l)
{
    struct	procent *prptr;	
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
        prptr->prlockid = NO_LOCK;
    }
    else
    {
        prptr->prlockid = l->id;
        for (i = 0; i < NPROC; i++)
        {
            prptr = &proctab[i];
            if (prptr->prlockid == currpid)
            {
                deadlock_pids[deadlock_count++] = i; 
            }
        }

        if (deadlock_count > 0) 
        {
            kprintf("deadlock_detected=");
            for (i = 0; i < deadlock_count; i++)
            {
                if (currpid < deadlock_pids[i])
                {
                    kprintf("P%d-", currpid);
                }
                else if (i == (deadlock_count-1))
                {
                    kprintf("P%d", deadlock_pids[i]);
                }
                else 
                {
                    kprintf("P%d-", deadlock_pids[i]);
                }
            }
            kprintf("\n", deadlock_pids[i]);
        }

        //print_lock_list(l->q);
        insert_lock(currpid, l->q, 0);        
        setpark();
        l->guard = 0;
        park();
        l->owner = currpid;
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

    while (test_and_set(&l->guard, 1) == 1) sleepms(QUANTUM);
    if (l->owner == currpid)
    {
        if (isempty(l->q) == 0)
        {
            l->flag = 0;
        }
        else 
        {
            unpark(dequeue(l->q));

            prptr = &proctab[currpid];
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
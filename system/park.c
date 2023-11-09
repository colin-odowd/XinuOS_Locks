/* park.c - park / unpark a process */

#include <xinu.h>

struct	procent *prptr;	

/*------------------------------------------------------
 *  park  -  Park a process
 *------------------------------------------------------
 */
void park()
{
    intmask		mask;		/* Saved interrupt mask		*/
	mask = disable();

    if (prptr->prparkflag == 1)
    {
        prptr = &proctab[currpid];
        prptr->prstate = PR_WAIT;
        prptr->prparkflag = 0;
        resched();
    }

    restore(mask);
}

/*------------------------------------------------------
 *  unpark  -  Unpark a process
 *------------------------------------------------------
 */
void unpark(pid32 pid)
{
    intmask		mask;		/* Saved interrupt mask		*/
	mask = disable();

    prptr = &proctab[pid];
    prptr->prparkflag = 0;
    prptr->prstate = PR_READY;
    insert(pid, readylist, prptr->prprio);    
    resched();
    
    restore(mask);

}

/*------------------------------------------------------
 *  setpark  -  setpark
 *------------------------------------------------------
 */
void setpark()
{
    intmask		mask;		/* Saved interrupt mask		*/
	mask = disable();

    prptr = &proctab[currpid];
    prptr->prparkflag = 1;
    
    restore(mask);
}
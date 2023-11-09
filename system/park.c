/* park.c - park / unpark a process */

#include <xinu.h>

struct	procent *prptr;	

/*------------------------------------------------------
 *  park  -  Park a process
 *------------------------------------------------------
 */
void park()
{
    prptr = &proctab[currpid];
    if (prptr->prparkflag == 1)
    {
        resched();
    }
}

/*------------------------------------------------------
 *  unpark  -  Unpark a process
 *------------------------------------------------------
 */
void unpark(pid32 pid)
{
    prptr = &proctab[pid];
    prptr->prparkflag = 0;
    prptr->prstate = PR_READY;
    insert(pid, readylist, prptr->prprio);
    resched();
}

/*------------------------------------------------------
 *  setpark  -  setpark
 *------------------------------------------------------
 */
void setpark()
{
    prptr = &proctab[currpid];
    prptr->prparkflag = 1;
}
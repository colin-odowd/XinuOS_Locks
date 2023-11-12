// /*  main-deadlock.c  - main-deadlock */

// #include <xinu.h>

// #define N 3
// #define PART_1 1
// #define PART_2 0
// #define PART_3 0
// #define PART_4 1

// pid32 pid[N];		    // threads 
// al_lock_t mutex[N];	    // mutexes
// al_lock_t master;       // master mutex lock

// syscall sync_printf(char *fmt, ...)
// {
//         intmask mask = disable();
//         void *arg = __builtin_apply_args();
//         __builtin_apply((void*)kprintf, arg, 100);
//         restore(mask);
//         return OK;
// }

// void run_for_ms(uint32 time){
// 	uint32 start = proctab[currpid].runtime;
// 	while ((proctab[currpid].runtime-start) < time);
// }

// process p2(al_lock_t *l1, al_lock_t *l2){
// 	sync_printf("P%d:: acquiring: l1=%d l2=%d\n", currpid, l1->id, l2->id);	
// 	al_lock(l1);
//     if (currpid == 6) kprintf("A\n");
// 	run_for_ms(1000);
// 	al_lock(l2);	
//     if (currpid == 6) kprintf("B\n");
// 	run_for_ms(1000);
// 	al_unlock(l1);
//     if (currpid == 6) kprintf("C\n");
// 	run_for_ms(1000);
// 	al_unlock(l2);	
//     if (currpid == 6) kprintf("D\n");
// 	run_for_ms(1000);
// 	return OK;
// }

// process	main_deadlock(void)
// {

// 	uint32 i;
	
//     if (PART_1)
//     { 
//         /* initialize al_locks */
//         for (i=0; i<N; i++) al_initlock(&mutex[i]);

//         kprintf("\n\n=========== TEST for deadlocks ===================\n\n");

//         pid[0] = create((void *)p2, INITSTK, 5, "p2", 2, &mutex[0], &mutex[1]);
//         pid[1] = create((void *)p2, INITSTK, 5, "p2", 2, &mutex[1], &mutex[2]);
//         pid[2] = create((void *)p2, INITSTK, 5, "p2", 2, &mutex[2], &mutex[0]);
        
//         for (i = 0; i < N; i++) 
//         {
//             sleepms(100);
//             resume(pid[i]);
//             sleepms(100);
//         }

//         al_unlock(&mutex[0]);
//         sleepms(100);
//         al_unlock(&mutex[1]);
//         sleepms(100);
//         al_unlock(&mutex[2]);
//         kprintf("P%d completed\n", receive());

//     }
//     if (PART_2)
//     {
//         /* initialize al_locks */
//         for (i=0; i < N; i++) al_initlock(&mutex[i]);
//         al_initlock(&master);

//         kprintf("\n\n=========== TEST for deadlocks, avoiding hold-and-wait ===================\n\n");

//         pid[0] = create((void *)p2, INITSTK, 5, "p2", 2, &mutex[0], &mutex[1]);
//         pid[1] = create((void *)p2, INITSTK, 5, "p2", 2, &mutex[1], &mutex[2]);
//         pid[2] = create((void *)p2, INITSTK, 5, "p2", 2, &mutex[2], &mutex[0]);

//         for (i = 0; i < N; i++) 
//         {
//             sleepms(100);
//             al_lock(&master);
//             resume(pid[i]);
//             al_unlock(&master);
//             sleepms(100);
//         }

//         kprintf("P%d completed\n", receive());
//     }
//     if (PART_3)
//     {
//         // /* initialize al_locks */
//         // for (i=0; i<N; i++) al_initlock(&mutex[i]);

//         // kprintf("\n\n=========== TEST for deadlocks, allowing preemption ===================\n\n");

//         // /* first deadlock: 2 threads */	
//         // pid[0] = create((void *)p2, INITSTK, 5, "p2", 2, &mutex[0], &mutex[1]);
//         // pid[1] = create((void *)p2, INITSTK, 5, "p2", 2, &mutex[1], &mutex[2]);
//         // pid[2] = create((void *)p2, INITSTK, 5, "p2", 2, &mutex[2], &mutex[0]);

//         // for (i = 0; i < N; i++) 
//         // {
//         //     sleepms(100);
//         //     resume(pid[i]);
//         //     sleepms(100);
//         // }

//         // kprintf("P%d completed\n", receive());
//     }
//     if (PART_4)
//     {
//         /* initialize al_locks */
//         for (i=0; i<N; i++) al_initlock(&mutex[i]);

//         kprintf("\n\n=========== TEST for deadlocks, avoiding circular wait ===================\n\n");

//         pid[0] = create((void *)p2, INITSTK, 5, "p2", 2, &mutex[0], &mutex[1]);
//         pid[1] = create((void *)p2, INITSTK, 5, "p2", 2, &mutex[1], &mutex[2]);
//         pid[2] = create((void *)p2, INITSTK, 5, "p2", 2, &mutex[0], &mutex[2]);

//         for (i = 0; i < N; i++) 
//         {
//             sleepms(100);
//             resume(pid[i]);
//             sleepms(100);
//         }

//         kprintf("P%d completed\n", receive());
//     }

// 	return OK;
// }

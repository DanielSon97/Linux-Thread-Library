/*
 * Test 1 of 2: Join
 *
 * Test 2 of 2: Join and Yield
 * 
 * See main() for more details.
 */

#include <stdio.h>
#include <stdlib.h>

#include <uthread.h>

/***** Begin Join-only test *****/

int thread6(void *arg)
{
    return (2 + *(int *)arg);
}

int thread5(void* arg) {
    return (2000 + *(int *)arg);
}

int thread4(void* arg) {
    return (100 + *(int *)arg); // 901
}

int thread3(void* arg) {
    int math = 0;
    math = *(int *)arg + 1; // 801

    int retval;
    uthread_join(uthread_create(thread4, &math), &retval);

    int math2 = retval + 2; // 903
    int retval2 = 0;
    uthread_join(uthread_create(thread5, &math2), &retval2); // 903+2000=2903

    return retval2; // 2903
}

int thread2(void* arg)
{
	return 776;
}

int thread1(void* arg)
{	
    int thread2_returnVal = 0;

    uthread_join(uthread_create(thread2, NULL), &thread2_returnVal);

    if (thread2_returnVal == 0) { 
        return -1; 
    }
    else
    {
        thread2_returnVal = thread2_returnVal + 24;
        return thread2_returnVal; // 800
    }
}

/***** End Join()-only test *****/

/***** Begin Join() & Yield test *****/

int thread33(void* arg)
{
	//printf("Entering thread3\n"); // 5; ready: thread1; blocked: main(), thread2
	//printf("Exiting thread3\n"); // 6
	uthread_yield(); 
	//printf("Entering thread3\n"); // 9; ready: thread1; blocked: main(), thread2
	printf("thread3\n");
	//printf("Exiting thread3\n"); // 10
	return 0; // unblocks thread2. ready: thread1, thread2; blocked: main()
}

int thread22(void* arg)
{
	//printf("Entering thread2\n"); // 3; ready: thread1; blocked: main()
	//printf("Exiting thread2\n"); // 4
	uthread_join(uthread_create(thread33, NULL), NULL);
	//printf("Entering thread2\n"); // 13
	printf("thread2\n");
	//printf("Exiting thread2\n"); // 14; ready: empty, blocked: main()
	return 0;					// 15; unblocks main(). ready: main()
}

int thread11(void* arg)
{
	//printf("Entering thread1\n"); // 1; ready: empty; blocked: main()
	uthread_create(thread22, NULL);
	//printf("Exiting thread1\n"); // 2; ready: thread2, blocked: main()
	uthread_yield();
	//printf("Entering thread1\n"); // 7; ready: thread3; blocked: main(), thread2
	printf("\nthread1\n");
	//printf("Exiting thread1\n"); // 8
	uthread_yield();
	//printf("Entering thread1\n"); // 11; ready: thread2; blocked: main()
	//printf("Exiting thread1\n"); // 12
	return 0;
}

/***** End Join() & Yield test *****/

int main(void)
{
	int returnValue = 0;
    int returnValue2 = 0;
    int returnValue3 = 0;

    /*
    Test 1 of 2:

    uthread_join() test without uthread_yield(): have threads other than main()
    thread call join. When control returns back to main() thread, use the 
    return value as the argument for the next uthread_join(uthread_create()) call.
    */

	uthread_join(uthread_create(thread1, NULL), &returnValue);
	if (returnValue == 0) {
        printf("\nMain() -- FAIL: Did not capture thread1()'s return value.");
    } else {
        printf("\nMain() -- SUCCESS: Captured thread1()'s return value = %d\n", returnValue); // 800
    }

    uthread_join(uthread_create(thread3, &returnValue), &returnValue2);
    if (returnValue2 == 0) {
        printf("\nMain() -- FAIL: Did not capture thread_?()'s return value.");
    } else {
        printf("\nMain() -- SUCCESS: Captured thread3()'s return value = %d\n", returnValue2); // 2903
    }

    uthread_join(uthread_create(thread6, &returnValue2), &returnValue3);
    if (returnValue3 == 0) {
        printf("\nMain() -- FAIL: Did not capture thread_?()'s return value.");
    } else {
        printf("\nMain() -- SUCCESS: Captured thread6()'s return value = %d\n", returnValue3); // 2905
    }

    /*
    Test 2 of 2:

    Combine uthread_join() and uthread_yield(). Same as original code in uthread_yield.c, 
    but with a join() statement added in thread2. Output:

    thread1
    thread3
    thread2

    If we removed the join statement, it would be: 

    thread1
    thread2
    thread3
    */

    uthread_join(uthread_create(thread11, NULL), NULL);

    return 0;
}
/* fork.c */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>

int main()
{
  /* create a new process */
  pid_t p = fork();

  if ( p == -1 )
  {
    perror( "fork() failed" );
    return EXIT_FAILURE;
  }

  if ( p == 0 )
  {
    /* CHILD PROCESS */
    printf( "CHILD: happy birthday to me!  My PID is %d.\n", getpid() );
    printf( "CHILD: my parent's PID is %d.\n", getppid() );
  }
  else /* p > 0 */
  {
    usleep( 30 );
    /* PARENT PROCESS */
    printf( "PARENT: my child process PID is %d.\n", p );
    printf( "PARENT: my PID is %d.\n", getpid() );
  }

  /* This one line of output appears twice, once in the parent,
   *  once in the child process....
   *
   * TO DO: revise the diagram below with this new line of output...
   */
  printf( "My PID is %d and I'm terminating...\n", getpid() );

  usleep( 10000 );  /* <== add this so that the bash prompt delays printing */

  return EXIT_SUCCESS;
}


/*
 * goldsd@linux:~/u22/csci4210$ ./a.out
 * PARENT: my child process PID is 1606230.
 * PARENT: my PID is 1606229.
 * CHILD: happy birthday to me!  My PID is 1606230.
 * CHILD: my parent's PID is 1606229.
 * goldsd@linux:~/u22/csci4210$ ./a.out
 * CHILD: happy birthday to me!  My PID is 1606234.
 * PARENT: my child process PID is 1606234.
 * PARENT: my PID is 1606233.
 * CHILD: my parent's PID is 1606233.
 * goldsd@linux:~/u22/csci4210$ ./a.out
 *
 *
 * What are all possible outputs for this code?
 *
 *                               p = fork()
 *                                /     \
 *                        p > 0  /       \  p == 0
 *                              /         \
 *                             /           \
 *                     <PARENT>             <CHILD>
 * PARENT: my child process PID...          CHILD: happy birthday to me!...
 * PARENT: my PID is ...                    CHILD: my parent's PID is ...
 *
 * (1) lines shown above in the <PARENT> section interleave with
 *      the lines shown in the <CHILD> section
 *
 * (2) lines shown above in the <PARENT> section occur in that given order;
 *      same for <CHILD> section
 *
 * TO DO: in addition to the diagram, write out ALL possible outputs
 *
 */

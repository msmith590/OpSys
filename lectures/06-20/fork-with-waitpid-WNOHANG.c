
/* fork-with-waitpid-WNOHANG.c */

/* TO DO: run this via gcc -E and figure out what the
 *         preprocessor macros are doing with the exit status variable...
 *
 *        bash$ gcc -E fork-with-waitpid.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

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

    sleep( 5 );

    return EXIT_FAILURE;   /* TO DO: try other values here... */
  }
  else /* p > 0 */
  {
    usleep( 30 );
    /* PARENT PROCESS */
    printf( "PARENT: my child process PID is %d.\n", p );
    printf( "PARENT: my PID is %d.\n", getpid() );

    /* TO DO: rewrite this code to handle the SIGCHLD signal,
     *         essentially running the code below when the
     *          SIGCHLD signal is received...
     */

    /* Wait for my child process to complete/terminate */
    int status;
#if 0
    pid_t child_pid = waitpid( p, &status, 0 );   /* BLOCKING */
#endif

#if 1
    pid_t child_pid;
    do
    {
      child_pid = waitpid( p, &status, WNOHANG );   /* NON-BLOCKING */

      if ( child_pid == 0 )
      {
        printf( "PARENT: still waiting for my child process to terminate\n" );
        sleep( 1 );
      }
    }
    while ( child_pid == 0 );
#endif

    printf( "PARENT: child process %d terminated...\n", child_pid );

    if ( WIFSIGNALED( status ) )  /* child process was terminated   */
    {                             /*  by a signal (e.g., seg fault) */
      printf( "PARENT: ...abnormally (killed by a signal)\n" );
    }
    else if ( WIFEXITED( status ) )
    {
      int exit_status = WEXITSTATUS( status );
      printf( "PARENT: ...successfully with exit status %d\n", exit_status );
    }

  }

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

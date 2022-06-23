/* fork-with-waitpid.c */

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

#if 0
    char * x = NULL;
    *x = 'A';
#endif

    sleep( 3 );
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
    pid_t child_pid = waitpid( p, &status, 0 );   /* BLOCKING */

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
/* fork-with-exec.c */

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

                       /* argv[0], argv[1], argv[2] */
    execl( "/usr/bin/ls", "ls",    "-l",    "fork-with-waitpid.c", NULL );

#if 0
                       /* argv[0], argv[1], argv[2] */
    execl( "/usr/bin/xyz", "ls",    "-l",    "fork-with-waitpid.c", NULL );

    /* Modify this to match your Homework 2 code... */
    execl( "/cs/goldsd/u22/csci4210/assignments/hw1/code/a.out", "a.out",    "37",    "/cs/goldsd/u22/csci4210/assignments/hw1/code/hw1-input10.txt", NULL );

#endif

    printf( "CHILD: returned from execl(), which means an error occurred\n" );
    perror( "execl() failed" );
    return EXIT_FAILURE;
  }
  else /* p > 0 */
  {
    usleep( 30 );
    /* PARENT PROCESS */
    printf( "PARENT: my child process PID is %d.\n", p );
    printf( "PARENT: my PID is %d.\n", getpid() );

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
/* fork-octuplets-impatient.c */

#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>

#define CHILDREN 8

/* function prototypes */
int child();
void parent( int children, pid_t * pids );


int main()
{
  int i, children = CHILDREN;

  pid_t * pids = calloc( CHILDREN, sizeof( pid_t ) );

  for ( i = 0 ; i < children ; i++ )
  {
    pid_t p = fork();

    if ( p == -1 )
    {
      perror( "fork() failed" );
      return EXIT_FAILURE;
    }

    if ( p == 0 )
    {
      free( pids );    /* child process does not use this pids array */
      int rc = child();
      exit( rc );      /* be sure to exit the child process! */
    }

    /* at this point, we are only running in the parent process */
    pids[i] = p;
  }

  parent( children, pids );

  free( pids );

  return EXIT_SUCCESS;
}

/* each child process will sleep for t second,    */
/*  then return t as exit status when it wakes up */
int child()
{
  /* seed the pseudo-random number generator (in EACH child process) */
  srand( time( NULL ) * getpid() * getpid() );

  int t = 10 + ( rand() % 21 );  /* [10,30] */

  printf( "CHILD %d: I'm gonna nap for %d seconds\n", getpid(), t );
  sleep( t );
  printf( "CHILD %d: I'm awake!\n", getpid() );

  return t;
}

void parent( int children, pid_t * pids )
{
  int status;  /* exit status from each child process */

  pid_t child_pid;

  printf( "PARENT: I'm waiting for my children to wake up\n" );

  while ( children > 0 )
  {
    /* TO DO: use values from the pids array for waitpid() below... */

    /* wait until a child process exits */
    child_pid = waitpid( -1, &status, 0 );   /* BLOCKING CALL */

    children--;

    printf( "PARENT: child process %d terminated...", child_pid );

    if ( WIFSIGNALED( status ) )
    {
      printf( "abnormally\n" );
    }
    else if ( WIFEXITED( status ) )
    {
      int rc = WEXITSTATUS( status );
      printf( "successfully with exit status %d\n", rc );
    }

    printf( "PARENT: %d child processes to go...\n", children );

    if ( children == 4 )
    {
      printf( "PARENT: Wake up already!\n" );

      /* TO DO: avoid the "No such process" error by updating the
       *         pids array after waitpid(), i.e., zeroing out the
       *          child process PID that has terminated...
       *
       * TO DO: replace the loop below with exactly one call to kill()
       *         with 0 (or -1?) as the first argument...
       */
      for ( int i = 0 ; i < CHILDREN; i++ )
      {
        int rc = kill( pids[i], SIGTERM );

        if ( rc == -1 )
        {
          perror( "kill() failed" );
          /* return EXIT_FAILURE; */
        }
      }
    }
  }

  printf( "PARENT: All done!\n" );
}
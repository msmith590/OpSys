/* fork-with-variables.c */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>

int main()
{
  int x = 5;
  int * y = &x;
  int * z = calloc( 20, sizeof( int ) );

  /* When we call fork(), all memory gets duplicated and
   *  copied in the child process memory image
   *
   * fork() will attempt to create a new process by
   *  duplicating/copying the memory image of the
   *   existing running process (parent)
   *
   * this includes all statically and dynamically allocated memory,
   *  i.e., x, y, z, p, and whatever z points to on the heap
   */


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
    x += 100;
    printf( "CHILD: x is now %d and *y is now %d\n", x, *y );
    z[9] = 88;
    printf( "CHILD: z[9] is %d\n", z[9] );
  }
  else /* p > 0 */
  {
    usleep( 30 );
    /* PARENT PROCESS */
    printf( "PARENT: my child process PID is %d.\n", p );
    printf( "PARENT: my PID is %d.\n", getpid() );
    x += 12;
    printf( "PARENT: x is now %d and *y is now %d\n", x, *y );
    z[9] = 33;
    printf( "PARENT: z[9] is %d\n", z[9] );
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
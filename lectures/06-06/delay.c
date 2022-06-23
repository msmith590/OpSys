/* delay.c */

/* run this in the background:
 *
 * bash$ ./a.out &
 * [1] 1485218             <== what is this telling?
 * bash$
 *
 * how do we know the background process has terminated?
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main( int argc, char ** argv )
{
  printf( "PID %d: argc is %d\n", getpid(), argc );

  printf( "PID %d: My parent process is PID %d\n", getpid(), getppid() );
  printf( "PID %d: Calculating something very important...\n", getpid() );

  sleep( 30 );

  printf( "PID %d: All done --- terminating...\n", getpid() );

  return EXIT_SUCCESS;
}
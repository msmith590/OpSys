/* rlimit.c */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/resource.h>

/* to look at all of the resource limit values:
 *
 * bash$ ulimit -a
 *
 */

int main()
{
  /* TO DO: capture and check the return values from get/setrlimit() */
  struct rlimit rl;
  getrlimit( RLIMIT_NPROC, &rl );
  printf( "RLIMIT_NPROC soft limit: %ld\n", rl.rlim_cur );
  printf( "RLIMIT_NPROC hard limit: %ld\n", rl.rlim_max );

  /* lower the RLIMIT_NPROC to 20 */
  rl.rlim_cur = 20;
  setrlimit( RLIMIT_NPROC, &rl );

  /* display the RLIMIT_NPROC values again */
  getrlimit( RLIMIT_NPROC, &rl );
  printf( "RLIMIT_NPROC soft limit: %ld\n", rl.rlim_cur );
  printf( "RLIMIT_NPROC hard limit: %ld\n", rl.rlim_max );

  /* STOP HERE!  Only run the above code on your platform! */

#if 0
  /* the code below is a FORK BOMB -- so be careful! */
  while ( 1 )
  {
    int p = fork();

    if ( p == -1 )
    {
      perror( "fork() failed" );
      return EXIT_FAILURE;
    }

    printf( "PID %d: fork() worked!\n", getpid() );

    sleep( 3 );  /* this gives you time to shut all the processes down! */
  }
#endif

  return EXIT_SUCCESS;
}
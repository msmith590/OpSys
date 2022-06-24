/* fork-pass-by-fork.c */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/wait.h>

void block_on_waitpid( pid_t pid );
void child_process( char * data, int i );

int main()
{
  pid_t p;   /* process id (pid) -- unsigned int */

  /*                         1111111111222222
   *        index: 01234567890123456789012345 */
  char data[27] = "abcdefghijklmnopqrstuvwxyz";

  /* this index variable i will indicate to the child process
   *  which chunk of data to convert to uppercase and display
   *   (and we'll have the child process work on a substring
   *    of length 6)
   */
  int i = 5;

  /* create the first child process */
  p = fork();  /* i == 5 */
  if ( p == -1 ) { perror( "fork() failed" ); return EXIT_FAILURE; }

  if ( p > 0 )   /* PARENT process executes here... */
  {
    usleep( 100 );
    printf( "PARENT: my first child process id is %d\n", p );

    i = 15;

    pid_t p2 = fork();  /* i == 15 */
    if ( p2 == -1 ) { perror( "fork() failed" ); return EXIT_FAILURE; }

    if ( p2 > 0 )   /* PARENT process executes here... */
    {
      usleep( 100 );
      printf( "PARENT: my second child process id is %d\n", p2 );
      block_on_waitpid( p );
      block_on_waitpid( p2 );
      printf( "PARENT: data: \"%s\"\n", data );
      /* The above printf() will always print the original data,
       *  i.e., "abcdefghijklmnopqrstuvwxyz" because the child
       *   processes are changing only their copies of this data
       *    variable on their own runtime stacks in their own memory
       */
    }
    else if ( p2 == 0 )   /* second CHILD process executes here... */
    {
      printf( "CHILD: happy birthday to me!  My pid is %d\n", getpid() );
      child_process( data, i );
    }
  }
  else if ( p == 0 )   /* first CHILD process executes here... */
  {
    printf( "CHILD: happy birthday to me!  My pid is %d\n", getpid() );
    child_process( data, i );
  }

  /* The PARENT and both CHILD processes all end up here... */

  return EXIT_SUCCESS;
}

void block_on_waitpid( pid_t pid )
{
  int status;
  pid_t child_pid;

  child_pid = waitpid( pid, &status, 0 );   /* BLOCKING */

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

void child_process( char * data, int i )
{
  for ( int j = 0 ; j < 6 ; j++ )
  {
    data[i+j] = toupper( data[i+j] );
  }
  printf( "CHILD %d: data: \"%s\"\n", getpid(), data );
}
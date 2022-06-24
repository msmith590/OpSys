/* pipe-ipc-complete.c */

/* A pipe is a unidirectional communication channel -- man 2 pipe */
/*             ^^^                                                */

/* TO DO: Draw a diagram that shows all possible outputs for this code */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main()
{
  int pipefd[2];   /* array to hold the two pipe (file) descriptors:
                    * pipefd[0] is the "read" end of the pipe
                    * pipefd[1] is the "write" end of the pipe
                    */

  int rc = pipe( pipefd );

  if ( rc == -1 )
  {
    perror( "pipe() failed" );
    return EXIT_FAILURE;
  }

  /* fd table for this process:
   *
   *  0: stdin
   *  1: stdout
   *  2: stderr                   +--------+
   *  3: pipefd[0] <======READ====| buffer | think of this buffer as a
   *  4: pipefd[1] =======WRITE==>| buffer |  temporary hidden transient file
   *                              +--------+
   */

  printf( "PARENT: Created pipe; pipefd[0] is %d and pipefd[1] is %d\n",
          pipefd[0], pipefd[1] );


  pid_t p = fork();  /* fork() will copy the fd table to the child process */

  if ( p == -1 )
  {
    perror( "fork() failed" );
    return EXIT_FAILURE;
  }

  /* fd table for this process:
   *
   *  [PARENT]                                              [CHILD]
   *  0: stdin                                              0: stdin
   *  1: stdout                                             1: stdout
   *  2: stderr                   +--------+                2: stderr
   *  3: pipefd[0] <======READ====| buffer |=====READ=====> 3: pipefd[0]
   *  4: pipefd[1] =======WRITE==>| buffer |<====WRITE===== 4: pipefd[1]
   *                              +--------+
   */

  /* fd table for this process (after we close the unused descriptors):
   *
   *  [PARENT]                                              [CHILD]
   *  0: stdin                                              0: stdin
   *  1: stdout                                             1: stdout
   *  2: stderr                   +--------+                2: stderr
   *  3: pipefd[0] <======READ====| buffer |                3:
   *  4:                          | buffer |<====WRITE===== 4: pipefd[1]
   *                              +--------+
   */
  if ( p == 0 )
  {
    close( pipefd[0] );  /* close the read end of the pipe */

    /* write data to the pipe */
    int bytes_written = write( pipefd[1], "ABCDEFGHIJKLMNOPQRSTUVWXYZ", 26 );
    printf( "CHILD: Wrote %d bytes to the pipe\n", bytes_written );

    close( pipefd[1] );  /* close the write end of the pipe */
  }
  else /* p > 0 */
  {
    usleep( 30 );
    /* TO DO: add waitpid() here; also try moving waitpid() down further... */

    close( pipefd[1] );  /* close the write end of the pipe */

    /* read data from the pipe */
    char buffer[20];
    int bytes_read = read( pipefd[0], buffer, 10 );
    buffer[bytes_read] = '\0';
    printf( "PARENT: Read %d bytes: \"%s\"\n", bytes_read, buffer );

    bytes_read = read( pipefd[0], buffer, 10 );
    buffer[bytes_read] = '\0';
    printf( "PARENT: Read %d bytes: \"%s\"\n", bytes_read, buffer );

    bytes_read = read( pipefd[0], buffer, 10 );
    buffer[bytes_read] = '\0';
    printf( "PARENT: Read %d bytes: \"%s\"\n", bytes_read, buffer );

    bytes_read = read( pipefd[0], buffer, 10 );
    buffer[bytes_read] = '\0';
    printf( "PARENT: Read %d bytes: \"%s\"\n", bytes_read, buffer );

    close( pipefd[0] );  /* close the read end of the pipe */
  }

  usleep( 10000 );

  return EXIT_SUCCESS;
}
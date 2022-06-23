/* fd-read.c */

/**
 *  file descriptor (fd)
 *
 *  -- a small non-negative integer used in a variety of system calls
 *      to refer to an open file (i.e., file stream or byte stream)
 *
 *  -- each process has a file descriptor table associated with it
 *      that keeps track of its open files
 *
 *  fd        C++   Java        C
 *  0 stdin   cin   System.in   scanf(), read(), getchar(), ...
 *  1 stdout  cout  System.out  printf(), write(), putchar(), ...
 *  2 stderr  cerr  System.err  fprintf( stderr, "ERROR: ....\n" );
 *                              perror( "open() failed" );
 *
 *     stdout and stderr (by default) are both displayed on the terminal
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

int main()
{
  char * name = "testfile.txt";

  int fd = open( name, O_RDONLY );

  printf( "fd is %d\n", fd );

  if ( fd == -1 )
  {
    perror( "open() failed" );
    return EXIT_FAILURE;
  }

  /* fd table for this running process:
   *   0  stdin
   *   1  stdout
   *   2  stderr
   *   3  testfile.txt (O_RDONLY)
   */

  char buffer[20];
  int rc = read( fd, buffer, 19 );
  buffer[rc] = '\0';   /* assume we have character data */
  printf( "read() returned %d -- read \"%s\"\n", rc, buffer );

  rc = read( fd, buffer, 19 );
  buffer[rc] = '\0';   /* assume we have character data */
  printf( "read() returned %d -- read \"%s\"\n", rc, buffer );

  rc = read( fd, buffer, 19 );
  buffer[rc] = '\0';   /* assume we have character data */
  printf( "read() returned %d -- read \"%s\"\n", rc, buffer );

  rc = read( fd, buffer, 19 );
  buffer[rc] = '\0';   /* assume we have character data */
  printf( "read() returned %d -- read \"%s\"\n", rc, buffer );

  rc = read( fd, buffer, 19 );
  buffer[rc] = '\0';   /* assume we have character data */
  printf( "read() returned %d -- read \"%s\"\n", rc, buffer );

  close( fd );

  /* fd table for this running process:
   *   0  stdin
   *   1  stdout
   *   2  stderr
   *   3
   */

#if 0
  rc = read( fd, buffer, 19 );
  buffer[rc] = '\0';   /* assume we have character data */
  printf( "read() returned %d -- read \"%s\"\n", rc, buffer );

  if ( rc == -1 )
  {
    perror( "read() failed" );
    return EXIT_FAILURE;   /* 1 */
  }
#endif

  return EXIT_SUCCESS;   /* 0 --- use echo $? to see this in the shell */
}
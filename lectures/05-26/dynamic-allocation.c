/* dynamic-allocation.c  */

/* TO DO: use valgrind with this example and fix all of the errors
 *
 *        bash$ gcc -g -Wall -Werror dynamic-allocation.c
 *
 *        bash$ valgrind ./a.out
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

int main()
{
  char * path = malloc( 40 );  /* dynamically allocate 16 bytes
                                *  on the runtime heap
                                */
  if ( path == NULL )
  {
    perror( "malloc() failed" );
    return EXIT_FAILURE;
  }

  char * path2 = malloc( 17 );

  printf( "sizeof( path ) is %lu\n", sizeof( path ) );

  strcpy( path, "/cs/goldsd/u22/" );
  printf( "path is \"%s\" (strlen is %ld)\n", path, strlen( path ) );

  strcpy( path2, "AAAAAAAAAAAAAAAA" );  /* 16 'A' characters */
  printf( "path2 is \"%s\" (strlen is %ld)\n", path2, strlen( path2 ) );

  strcpy( path, "/cs/goldsd/u22/blah/BLAH/BlAh/blaH/meme" );
  printf( "path is \"%s\" (strlen is %ld)\n", path, strlen( path ) );
  printf( "path2 is \"%s\" (strlen is %ld)\n", path2, strlen( path2 ) );

  /* TO DO: Fix the memory issues above -- use valgrind to debug... */

  free( path );
  free( path2 );

  return EXIT_SUCCESS;
}
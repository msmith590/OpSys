/* dynamic-mem-two-layer-structure.c */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

int main()
{
  /* array of strings (i.e., array of char arrays) */

  char ** names;

  /*      malloc( 62 * sizeof( char * ) );  <== UNINITIALIZED! */
  names = calloc( 62, sizeof( char * ) );

  names[2] = calloc( 7, sizeof( char ) );
  strcpy( names[2], "Lakers" );
  printf( "%s will win the title next year\n", names[2] );

#if 0
                 char*
               +------+
  names---> [0]| NULL |
               +------+
            [1]| NULL |
               +------+   +----------+
            [2]| ========>|"Lakers\0"|
               +------+   +----------+
                 ....
               +------+
           [61]| NULL |
               +------+
#endif

  /* use realloc() to change the size of names[2] */
  /* increase from 7 to 8 bytes... */
  names[2] = realloc( names[2], 8 * sizeof( char ) );
  printf( "Again, %s will win the title next year\n", names[2] );
  strcpy( names[2], "Celtics" );
  printf( "%s will win the title this year\n", names[2] );


#if 0
  /* memory leaks: */
  names[2] = NULL;  /* or   names[2] = calloc( 100, ... ); */
  /* DON'T DO THIS! */
#endif

  free( names[2] );
  free( names );

  return EXIT_SUCCESS;
}
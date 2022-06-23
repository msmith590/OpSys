/* dynamic-mem-ptr-arithmetic.c */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main()
{
  /* dynamically allocate 100 bytes (on the heap) */
  char * s = malloc( 100 );

  /* dynamically allocate 100 bytes (on the heap) */
  char * t = calloc( 100, sizeof( char ) );
     /* calloc() will initialize these 100 bytes */
     /*  to all be zero bytes '\0'               */

  printf( "\"%s\" \"%s\"\n", s, t );

  s[0] = 'A';
  s[1] = 'B';
  s[2] = 'C';
  /* s[3] is NOT guaranteed by malloc() to contain '\0' */
  s[20] = 'W';
  s[21] = 'X';
  s[22] = 'Y';

  t[0] = 'T';
  t[1] = 'V';
  /* t[2] is guaranteed by calloc() to contain '\0' */

  printf( "\"%s\" \"%s\"\n", s, t );

  /* the output will be: "ABC" "TV"
   *                         ^
   *               there could be garbage data after "ABC"
   *                because malloc() does not zero that memory out...
   */

  printf( "\"%s\" \"%s\"\n", s + 20, t );   /* <=== &(*(s+20)) */
                                            /* <=== &s[20]     */

  free( s );
  free( t );


  int * v = calloc( 1, sizeof( int ) );
  *v = 1234;
  free( v );


  /* dynamically allocate memory for an array of integers of size 62 */
  int * numbers = calloc( 62, sizeof( int ) );
               /* malloc( 62 * sizeof( int ) ); */

  numbers[26] = 1234;
  *(numbers + 26) = 1234;
 /* ^^^^^^^^^^^^ */
 /* numbers + ( 26 x sizeof( int ) )  <== this is in terms of bytes */
 /*                         <== we'd never actually write this code */

  printf( "%d\n", numbers[26] );
  printf( "%d\n", *(numbers+26) );

#if 0
  printf( "%d\n", *(numbers+400) );
  printf( "%d\n", *(numbers+4000) );
  printf( "%d\n", *(numbers+40000) );
  printf( "%d\n", *(numbers+400000) );
#endif

  free( numbers );


#if 0
  char * x = (char *)numbers;
  *(x + 104) = 'A';
#endif

  return EXIT_SUCCESS;
}
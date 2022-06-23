/* simple.c */

/* compile from terminal/shell as follows:
 * bash$ gcc -Wall -Werror simple.c -lm
 *
 * to only perform preprocessing stage of compiler:
 * bash$ gcc -E simple.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>

#define BUFFER_MAX 32

int main()
{
  /* one char == one byte  -->  go look up an ASCII table... */

  /*                      [0] [1] [2] [3]  [4]   */
  /*                     +---------------------+ */
  /* C string in memory: | H | i | . | \n | \0 | */
  /*                     +---------------------+ */
  /*                                        ^^   */
  /*                       '\0' is end-of-string */

  printf( "Hi.\n" );
  /*          ^^
   *     one single character '\n' means newline */

  printf( "BUFFER_MAX is %d\n", BUFFER_MAX );

  int i;
  for ( i = 1 ; i < BUFFER_MAX / 4 ; i++ )
  {
    printf( "%3d %f %20.15f\n", i, sqrt( i ), sqrt( i ) );
    printf( "%03d %f %-20.15f\n", i, sqrt( i ), sqrt( i ) );
    printf( "%03d %f %-20.200f\n", i, sqrt( i ), sqrt( i ) );
    printf( "%03d %f %-20.200f\n", i, sqrtf( i ), sqrtf( i ) );
    printf( "\n" );
  }

#if 0
  /* TO DO: what is the exact output of the following code? */
  float j;   /* float j = 12.345; */
  printf( "printf prints j: %f\n", j );
#endif

  return EXIT_SUCCESS;   /* 0 */
}
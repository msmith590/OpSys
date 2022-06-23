/* scanf.c */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>

int main()
{
                  /*       [0]1 2 3  ... 31  */
                  /*       +---------------+ */
  char name[32];  /* name: | | | | | ... | | */
                  /*       +---------------+ */

  /* Above, name is a pointer to the first byte of the array */

  /* note that name is uninitialized here... */
  printf( "name: %s\n", name );

#if 0
  printf( "Enter your name: " );
  scanf( "%s", name );
  printf( "Hello %s\n", name );
#endif

  printf( "Enter your name: " );
  scanf( "%[^\n]s", name );
  printf( "Hello %s\n", name );

  /* The above scanf() call will now read all characters up to
   *  but NOT including the '\n' newline character
   *
   * Since the above scanf() calls that use %s have no bound or limit,
   *  we can end up with a buffer overflow --- instead, we can use:
   *
   *   char * name;
   *   scanf( "%ms", &name );    <== dynamic memory allocation
   *    ...
   *   free( name );
   */

             /*    +----+ */
  float x;   /* x: |    | */
             /*    +----+ */

  printf( "sizeof( float ) is %lu bytes\n", sizeof( float ) );

  printf( "Enter a number: " );
  scanf( "%f", &x );   /* & is the address-of operator */
  printf( "The square root of %f is %f\n", x, sqrt( x ) );

  return EXIT_SUCCESS;
}
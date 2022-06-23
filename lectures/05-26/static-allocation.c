/* static-allocation.c */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main()
{
  int x = 5;   /* x is statically allocated (on the stack) */
               /*  (4 bytes are allocated on the stack)    */

  printf( "x is %d\n", x );
  printf( "sizeof( int ) is %lu bytes\n", sizeof( int ) );
  printf( "sizeof( x ) is %lu bytes\n", sizeof( x ) );

  int * y = NULL;  /* y is statically allocated (on the stack) */
                   /*  (8 bytes are allocated on the stack)    */

  y = &x;  /* & is the address-of operator */
  printf( "y points to %d\n", *y );

  printf( "sizeof( int* ) is %lu bytes\n", sizeof( int* ) );
  printf( "sizeof( y ) is %lu bytes\n", sizeof( y ) );


  char name[5] = "David";
  printf( "Hi, %s ???\n", name );

  char xyz[5] = "QRSTU";

  printf( "Hi again, %s\n", name );
  printf( "xyz is %s\n", xyz );

  /* the fix above is to increase the size to 6 -- in any case,
   *  there will be the implied '\0' character in memory
   */


  char * cptr = "ABBDEFGHIJKLMNOPQRSTUVWXYZ";
  printf( "cptr points to \"%s\"\n", cptr );
  cptr[2] = 'C';
  printf( "now cptr points to \"%s\"\n", cptr );

  /* TO DO (fix): try replacing char * cptr with a more specific
   *               static memory allocation ---> char cptr[27];
   *
   *              we could also look to strcpy()...
   */

  return EXIT_SUCCESS;
}
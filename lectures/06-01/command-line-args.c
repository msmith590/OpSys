/* command-line-args.c */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

                 /* char ** argv */
int main( int argc, char * argv[] )
{
  printf( "argc is %d\n", argc );  /* argument count */
  printf( "argv[0] is %s\n", argv[0] );

  if ( argc != 4 )
  {
    fprintf( stderr, "ERROR: Invalid arguments\n" );
    fprintf( stderr, "USAGE: a.out <filename> <x> <y>\n" );
    return EXIT_FAILURE;
  }

#if 0
  argv[2][1] = '!';   /* this worked! */
#endif

  printf( "argv[1] is %s\n", argv[1] );
  printf( "argv[2] is %s\n", argv[2] );
  printf( "argv[3] is %s\n", argv[3] );
  printf( "argv[4] is %s\n", argv[4] );

#if 0
                 char*                   Where is this allocated???
               +------+                   Generally on the runtime stack
  argv ---> [0]|  =======>"./a.out"
               +------+
            [1]|  =======>"abcd"
               +------+
            [2]|  =======>"1234"
               +------+
            [3]|  =======>"xyz"
               +------+
            [4]| NULL |        argv[argc] always is NULL
               +------+
#endif

  /* other ways to display all command-line arguments... */
  for ( int i = 0 ; *(argv+i) != NULL ; i++ )
  {
    printf( "*(argv+%d) is %s\n", i, *(argv+i) );
  }

  for ( char ** ptr = argv ; *ptr ; ptr++ )
  {
    printf( "next arg is %s\n", *ptr );
  }

  return EXIT_SUCCESS;
}
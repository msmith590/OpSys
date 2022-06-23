/* buffering.c */

/* When printing to the terminal via stdout (fd 1),
 *  a '\n' character will automatically "flush" the stdout buffer,
 *   i.e., output everything that has been stored in the output
 *    buffer so far...
 * ==> this is called line-based buffering
 *
 * TO DO: "fix" this by adding a '\n' at the end of each printf()
 *
 * When we instead output fd 1 to a file (e.g., fd-write-redirect.c),
 *  the '\n' character no longer flushes the output buffer
 * ==> this is called block-based buffering
 *
 * Use fflush() to flush the buffer, i.e., write the data to the file
 *
 * We can also redirect stdout (fd 1) via the shell:
 *
 *  bash$ ./a.out > output.txt
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main()
{
  printf( "HERE0" );

  int * x = NULL;

  printf( "HERE1" );

  *x = 1234;

  printf( "HERE2" );

  return EXIT_SUCCESS;
}
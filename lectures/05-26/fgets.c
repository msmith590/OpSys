/* fgets.c */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <math.h>

int main()
{
                  /*       [0]1 2 3  ... 31  */
                  /*       +---------------+ */
  char name[32];  /* name: | | | | | ... | | */
                  /*       +---------------+ */
  char email[32];
  char hometown[32];
  /* Above, name is a pointer to the first byte of the array */
  
  int c;

  /* note that name is uninitialized here... */
  printf( "name: %s\n", name );
  printf( "email: %s\n", email);
  printf( "hometown: %s\n", hometown);

  printf( "Enter your name: " );
/*  scanf( "%s", name ); */

  if ( fgets( name, 32, stdin ) == NULL )
  {
    fprintf( stderr, "ERROR: fgets() failed...?\n" );
    return EXIT_FAILURE;
  }

  printf( "Length of name is %ld bytes\n", strlen( name ) );
  
  if (strlen(name) == 31 && name[30] != '\n') {
    do
    {
      c = fgetc(stdin);
    } while (c != '\n' && c != EOF);
  } else { // Removing the newline character for strings smaller than buffer size
    name[strlen(name) - 1] = '\0';
  }


  printf("Enter your email: ");
  if ( fgets( email, 32, stdin ) == NULL )
  {
    fprintf( stderr, "ERROR: fgets() failed...?\n" );
    return EXIT_FAILURE;
  }
  
  printf( "Length of email is %ld bytes\n", strlen( email ) );
  if (strlen(email) == 31 && email[30] != '\n') {
    do
    {
      c = fgetc(stdin);
    } while (c != '\n' && c != EOF);
  } else {
    email[strlen(email) - 1] = '\0';
  }
  
  printf("Enter your hometown: ");
  if ( fgets( hometown, 32, stdin ) == NULL )
  {
    fprintf( stderr, "ERROR: fgets() failed...?\n" );
    return EXIT_FAILURE;
  }

  printf( "Length of hometown is %ld bytes\n", strlen( hometown ) );
  if (strlen(hometown) == 31 && hometown[30] != '\n') {
    do
    {
      c = fgetc(stdin);
    } while (c != '\n' && c != EOF);
  } else {
    hometown[strlen(hometown) - 1] = '\0';
  }

  /* TO DO: using strlen(), if we see that the buffer is full,
   *         and the last character is not a newline, then we know
   *          there is more data on the input stream...
   *
   *        remove the extraneous characters...
   */


  printf( "Hello %s\n", name );
  printf( "Your email is %s\n", email );
  printf( "Your hometown is %s\n", hometown );

             /*    +----+ */
  float x;   /* x: |    | */
             /*    +----+ */

  printf( "sizeof( float ) is %lu bytes\n", sizeof( float ) );

  printf( "Enter a number: " );
  scanf( "%f", &x );   /* & is the address-of operator */
  printf( "The square root of %f is %f\n", x, sqrt( x ) );

  return EXIT_SUCCESS;
}
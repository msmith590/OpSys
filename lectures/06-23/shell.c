/* shell.c */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>

#define MAXLINE 1024

int main()
{
  char * line = calloc( MAXLINE, sizeof( char ) );

  if ( line == NULL )
  {
    perror( "calloc() failed" );
    return EXIT_FAILURE;
  }

  int rc;
  int nbg = 0;  /* number of background processes */

  while ( 1 )
  {

    while ( nbg > 0 )
    {
      int status;
      pid_t child_pid = waitpid( -1, &status, WNOHANG );

      if ( child_pid == -1 )
      {
        perror( "waitpid() failed" );
        return EXIT_FAILURE;
      }

      if ( child_pid == 0 ) break;

      if ( child_pid > 0 )
      {
        nbg--;
        printf( "<background process terminated " );

        if ( WIFSIGNALED( status ) )
        {
          printf( "abnormally>\n" );
        }
        else if ( WIFEXITED( status ) )
        {
          printf( "with exit status %d>\n", WEXITSTATUS( status ) );
        }
      }

      /* TO DO: Extend the code above to also display which command
       *         was completed...
       */
    }


    /* Display the shell prompt and read in a line of input from the user */
    printf( "$ " );
    fflush( stdout );

    if ( fgets( line, MAXLINE, stdin ) == NULL )
    {
      fprintf( stderr, "fgets() failed\n" );
      break;
    }


    /* Remove trailing '\n' and/or space characters */
    int len = strlen( line );
    while ( len > 0 && isspace( *(line+len-1) ) )
    {
      *(line+len-1) = '\0';
      len--;
    }
    if ( len == 0 ) continue;

#ifdef DEBUG_MODE
    printf( "line: \"%s\"\n", line );
    fflush( stdout );
#endif


    /* Handle the special shell commands, i.e., exit and cd */
    if ( strcmp( line, "exit" ) == 0 ) break;

    if ( strncmp( line, "cd", 2 ) == 0 )
    {
      char * location = NULL;
      if ( len == 2 ) location = getenv( "HOME" ); else location = line + 3;
      rc = chdir( location );
      if ( rc == -1 ) fprintf( stderr, "cd: %s: No such file or directory\n", location );
      continue;
    }


    /* Determine if the command is to be run in the background */
    int background = *(line+len-1) == '&';

    if ( background )
    {
      nbg++;
      do { *(line+len-1) = '\0'; len--; } while ( isspace( *(line+len-1) ) );
    }


    /* Identify how many arguments this command has,     */
    /* assuming exactly one space character as delimiter */
    int anyargs = 1;
    char * space = strchr( line, ' ' );
    if ( space == NULL ) { space = line + strlen( line ); anyargs = 0; }
    *space = '\0';

#ifdef DEBUG_MODE
    printf( "command: %s\n", line );
    fflush( stdout );
#endif


    if ( background )
    {
      printf( "<running background process \"%s\">\n", line );
      fflush( stdout );
    }


    /* Create a child process to execute the command */
    pid_t p = fork();

    if ( p == -1 ) { perror( "fork() failed" ); return EXIT_FAILURE; }

    if ( p > 0 )  /* PARENT */
    {
      if ( !background )   /* run this command in the foreground */
      {
        pid_t child_pid = waitpid( p, NULL, 0 );

        if ( child_pid == -1 )
        {
          perror( "waitpid() failed" );
          return EXIT_FAILURE;
        }
      }
    }
    else  /* CHILD */
    {
      int argc = 1;

      /* TO DO: modify this code (above and below) to NOT assume only
       *         one space as a delimiter --- try using strtok()
       */

      /* Determine argument count, assuming exactly one space as delimiter */
      if ( anyargs ) for ( char * ptr = space + 1 ; ptr ; argc++ )
      {
        ptr = strchr( ptr, ' ' );
        if ( ptr ) ptr++;
      }

#ifdef DEBUG_MODE
      printf( "argc: %d\n", argc );
      fflush( stdout );
#endif

      /* Construct argument vector, assuming exactly one space as delimiter */
      char ** argv = calloc( argc + 1, sizeof( char * ) );
      *(argv+0) = calloc( strlen( line ) + 1, sizeof( char ) );
      if ( *(argv+0) == NULL ) { perror( "calloc() failed" ); return EXIT_FAILURE; }
      strcpy( *(argv+0), line );

      for ( int i = 1 ; i < argc ; i++ )
      {
        char * argument = space + 1;
        space = strchr( argument, ' ' );
        if ( space ) *space = '\0';
        *(argv+i) = calloc( strlen( argument ) + 1, sizeof( char ) );
        if ( *(argv+i) == NULL ) { perror( "calloc() failed" ); return EXIT_FAILURE; }
        strcpy( *(argv+i), argument );
      }

#ifdef DEBUG_MODE
      for ( int i = 0 ; i <= argc ; i++ ) printf( "*(argv+%d): %s\n", i, *(argv+i) );
      fflush( stdout );
#endif

      execvp( line, argv );
      perror( "execvp() failed" );
      return EXIT_FAILURE;
    }
  }

  printf( "bye\n" );

  free( line );

  return EXIT_SUCCESS;
}
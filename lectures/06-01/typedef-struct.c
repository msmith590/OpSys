/* typedef-struct.c */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

typedef struct
{
  unsigned int rin;  /* e.g., 660000001 */
  char rcsid[16];    /* e.g., "goldsd3" */
/*  char * firstname; */
}
student_t;

int main()
{
  printf( "sizeof( student_t ) is %lu\n", sizeof( student_t ) );

  /* statically allocated */
  student_t me;
  me.rin = 660000001;
  strcpy( me.rcsid, "goldsd3" );

  /* dynamically allocated */
  student_t * st;
  st = malloc( sizeof( student_t ) );

  (*st).rin = 661234567;
  strcpy( (*st).rcsid, "smithj385" );

  printf( "me: RIN %u; RCSID %s\n", me.rin, me.rcsid );
  printf( "st: RIN %u; RCSID %s\n", st->rin, st->rcsid );

  free( st );


  student_t * students = calloc( 6200, sizeof( student_t ) );
  students[500] = me;
  *(students+500) = me;
  printf( "students[500]: RIN %u; RCSID %s\n",
          students[500].rin, students[500].rcsid );

  /* TO DO: rewrite the above code using pointer arithmetic */

  free( students );

  return EXIT_SUCCESS;
}
#include <stdio.h>
#include <signal.h>
#include <setjmp.h>
int a,b,*p;
jmp_buf jump_destination;

void exception_handler (int sg)
{
  printf ("\nError dereferencing pointer and we're in interrupt handler\n");
  p=&b; /* pointer quick fix. */
  longjmp(jump_destination,1); /* long GOTO... */
}

void main (void)
{
  int i;

  signal (SIGSEGV, exception_handler);
  b=3; 
  p=NULL;    

  printf("Addr of interrupt handler -> %8.8X \n", &exception_handler);

  setjmp(jump_destination); /* ...to this position */
  printf ("Trying to dereference pointer p with value ->  ");
  printf("%8.8X .\n ", *p);
  printf ("After dereferencing pointer, its value is: %d\n", *p);
}
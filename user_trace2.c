#include "types.h"
#include "user.h"
int main()
{
	trace(1);
    printf(1, "Trace is on(1).\n");
    trace(0);
    printf(1, "Trace is off(0).\n");

    trace(1);

	printf(1, "Total system calls so far: %d\n", trace(0));
    printf(1, "Now trace is off due to previous trace(0) call");
	exit();
}
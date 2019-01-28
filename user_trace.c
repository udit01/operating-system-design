#include "types.h"
#include "user.h"


int main()
{
	trace(1);
	printf(1, "Total system calls so far: %d\n", trace(0)); //4
	exit();
	
}
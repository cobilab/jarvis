/*------------------------------------------------------------------------------

Copyright 2011 Armando J. Pinho (ap@ua.pt), All Rights Reserved.

These programs are supplied free of charge for research purposes only,
and may not be sold or incorporated into any commercial product. There is
ABSOLUTELY NO WARRANTY of any sort, nor any undertaking that they are
fit for ANY PURPOSE WHATSOEVER. Use them at your own risk. If you do
happen to find a bug, or have modifications to suggest, please report
the same to Armando J. Pinho, ap@ua.pt. The copyright notice above
and this statement of conditions must remain an integral part of each
and every copy made of these files.

------------------------------------------------------------------------------*/

#include <stdlib.h>
#include <malloc.h>
#include "mem.h"

/*----------------------------------------------------------------------------*/

static unsigned long long totalMem = 0;

/*----------------------------------------------------------------------------*/

void *Calloc(size_t nmemb, size_t size)

	{
	totalMem += nmemb * size;
	return calloc(nmemb, size);
	}

/*----------------------------------------------------------------------------*/

void *Malloc(size_t size)

	{
	totalMem += size;
	return malloc(size);
	}

/*----------------------------------------------------------------------------*/

void *Realloc(void *ptr, size_t size, size_t additionalSize)

	{
	totalMem += additionalSize;
	return realloc(ptr, size);
	}

/*----------------------------------------------------------------------------*/

unsigned long TotalMem()

	{
	return totalMem;
	}


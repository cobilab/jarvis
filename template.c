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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "defs.h"
#include "mem.h"
#include "template.h"

/*----------------------------------------------------------------------------*/

void IncreaseTemplateStorage(Template *template, int additionalSize)

	{
	if(template->size == 0)
		{
		if(!(template->position = (int *)Malloc(additionalSize * sizeof(int))))
			{
			fprintf(stderr, "Error: in memory allocation\n");
			exit(1);
			}

		}

	else
		{
		if(!(template->position = (int *)Realloc(template->position,
		  (additionalSize + template->size) * sizeof(int), additionalSize *
		  sizeof(int))))
			{
			fprintf(stderr, "Error: in memory allocation\n");
			exit(1);
			}

		}

	}

/*------------------------------------------------------------------------------

	"bases" points to the symbol being encoded / decoded

------------------------------------------------------------------------------*/

unsigned TemplateIdx(Symbol *bases, Template *template)

	{
	unsigned n, idx = 0;

	for(n = 0 ; n < template->size ; n++)
		idx += *(bases - template->position[n]) << (n << 1);

	return idx;
	}

/*------------------------------------------------------------------------------

	"bases" points to the symbol being encoded / decoded

------------------------------------------------------------------------------*/

unsigned TemplateIdxC(Symbol *bases, Template *template)

	{
	unsigned n, idx = 0;

	for(n = 0 ; n < template->size ; n++)
		idx += (3 - *(bases - template->position[n])) <<
		  ((template->size - n - 1) << 1);

	return idx;
	}


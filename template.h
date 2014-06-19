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

typedef struct
	{
	int size;
	int *position;
	int deepestPosition;
	}
Template;

void IncreaseTemplateStorage(Template *template, int additionalSize);
unsigned TemplateIdx(Symbol *bases, Template *template);
unsigned TemplateIdxC(Symbol *bases, Template *template);


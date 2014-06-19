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
#include "defs.h"
#include "mem.h"
#include "util.h"

/*----------------------------------------------------------------------------*/

int BaseToNumber(int c)

	{
	switch(c)
		{
		case 'A':
		case 'a':
			return 0;

		case 'C':
		case 'c':
			return 1;

		case 'G':
		case 'g':
			return 2;

		case 'T':
		case 't':
			return 3;

		default:
			return 4;

		}

	}

/*----------------------------------------------------------------------------*/

void ReadSequence(Sequence *seq, char *fName, int convert)

	{
	int c;
	char str[256];
	FILE *fp;

	seq->unknowns = 0;
	seq->size = 0;
	seq->convert = convert;

	/* Find out if the file can be open for reading */
	if(!(fp = fopen(fName, "r")))
		{
		fprintf(stderr, "Error: unable to open file \"%s\"\n", fName);
		exit(1);
		}

	fclose(fp);

	/* Open the file for possible decompression with gzip */
	sprintf(str, "zcat -f %s", fName);
	if((fp = popen(str, "r")) == NULL)
		{
		fprintf(stderr, "Error: unable to open file \"%s\"\n", fName);
		exit(1);
		}

	/* Get the file size */
	while((c = getc(fp)) != EOF)
		{
		if(c == '>')
			{
			while((c = fgetc(fp)) != EOF && c != '\n')
				;

			continue;
			}

		if(c == '\n')
			continue;

		seq->size++;
		if(BaseToNumber(c) == 4)
			seq->unknowns++;

		}

	fclose(fp);
	if(convert == CONV_AND_PACK_SEQUENCE)
		{
		if(!(seq->bases = (unsigned char *)Calloc((seq->size - seq->unknowns) +
		  SEQUENCE_LEFT_GUARD, sizeof(unsigned char))))
			{
			fprintf(stderr, "Error: out of memory\n");
			exit(1);
			}

		}

	else
		{
		if(!(seq->bases = (unsigned char *)Calloc(seq->size +
		  SEQUENCE_LEFT_GUARD, sizeof(unsigned char))))
			{
			fprintf(stderr, "Error: out of memory\n");
			exit(1);
			}

		}

	seq->bases += SEQUENCE_LEFT_GUARD;
	fp = popen(str, "r");

	seq->size = 0;
	while((c = getc(fp)) != EOF)
		{
		if(c == '>')
			{
			while((c = fgetc(fp)) != EOF && c != '\n')
				;

			continue;
			}

		if(c == '\n')
			continue;

		switch(convert)
			{
			case CONV_SEQUENCE:
				seq->bases[seq->size++] = BaseToNumber(c);
				break;

			case CONV_AND_PACK_SEQUENCE:
				c = BaseToNumber(c);
				if(c != 4)
					seq->bases[seq->size++] = c;

				break;

			default:
				seq->bases[seq->size++] = c;
				break;

			}

		}

	fclose(fp);
	}

/*------------------------------------------------------------------------------

	Pow function from http://martin.ankerl.com/2007/10/04/
	  optimized-pow-approximation-for-java-and-c-c/

------------------------------------------------------------------------------*/

double Pow(double a, double b)
	{
	int tmp = (*(1 + (int *)&a));
	int tmp2 = (int)(b * (tmp - 1072632447) + 1072632447);
	double p = 0.0;
	*(1 + (int * )&p) = tmp2;
	return p;
	}


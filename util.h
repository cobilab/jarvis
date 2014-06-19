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

#define RAW_SEQUENCE 0
#define CONV_SEQUENCE 1
#define CONV_AND_PACK_SEQUENCE 2

#define SEQUENCE_LEFT_GUARD 100

/*----------------------------------------------------------------------------*/

void ReadSequence(Sequence *seq, char *fName, int convert);
double Pow(double a, double b);


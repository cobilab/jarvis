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
#include <math.h>
#include <limits.h>
#include <float.h>
#include <ctype.h>
#include <time.h>
#include <malloc.h>

#include "bitio.h"
#include "arith.h"
#include "arith_aux.h"
#include "defs.h"
#include "mem.h"
#include "template.h"
#include "util.h"
#include "fcm.h"
#include "repeat.h"

unsigned hSizes[] = {9999991,19999999,29999999,39999983};

/*----------------------------------------------------------------------------*/

void ParseListOfPositions(char *str, Template *template)

	{
	int n, low, high;

	template->size = 0;
	while(1)
		{
		switch(sscanf(str, "%d:%d", &low, &high))
			{
			case 1:
				if(low == 0)
					return;

				if(low < 0)
					{
					fprintf(stderr,"Error: can't handle non-causal contexts\n");
					exit(1);
					}

				IncreaseTemplateStorage(template, 1);
				template->position[template->size++] = low;
				break;

			case 2:
				if(low <= 0)
					{
					fprintf(stderr,"Error: can't handle non-causal contexts\n");
					exit(1);
					}

				if(high - low + 1 > 0)
					{
					IncreaseTemplateStorage(template, high - low + 1);
					for(n = 0 ; n < high - low + 1 ; n++)
						template->position[template->size++] = low + n;

					}

				break;

			default:
				return;

			}

		while(*str != '\0' && *str++ != ',')
			;

		}

	}

/*----------------------------------------------------------------------------*/

void RenormalizeWeights(CModel **cModels, unsigned nCModels, FILE *fpCWeights,
  RClass **rClasses, unsigned nRClasses, FILE *fpRWeights)

	{
	unsigned cModel, rModel, rClass;
	double totalWeight = 0;

	for(cModel = 0 ; cModel < nCModels ; cModel++)
		totalWeight += cModels[cModel]->weight;

	for(rClass = 0 ; rClass < nRClasses ; rClass++)
		for(rModel = 0 ; rModel < rClasses[rClass]->nRModels ; rModel++)
			totalWeight += rClasses[rClass]->rModels[rModel].weight;

	for(cModel = 0 ; cModel < nCModels ; cModel++)
		{
		cModels[cModel]->weight /= totalWeight;
		if(fpCWeights)
			fprintf(fpCWeights, "%.3g ", cModels[cModel]->weight);

		}

	for(rClass = 0 ; rClass < nRClasses ; rClass++)
		for(rModel = 0 ; rModel < rClasses[rClass]->nRModels ; rModel++)
			{
			rClasses[rClass]->rModels[rModel].weight /= totalWeight;
			if(fpRWeights)
				fprintf(fpRWeights, "%.3g ", rClasses[rClass]->
				  rModels[rModel].weight);
			}

	if(fpCWeights)
		fprintf(fpCWeights, "\n");

	if(fpRWeights)
		fprintf(fpRWeights, "\n");

	}

/*----------------------------------------------------------------------------*/

int main(int argc, char **argv)

	{
	Symbol s, symbol, symbolC;
	char ir, cUpdateInvComps, rAllowInvComps, verbose, debug, best,
	  fNameEval[256];
	double bestCModelNats, *cModelNats, *cModelTotalNats, *bestCModelTotalNats,
	  bestTotalNats = 0, nats = 0, gamma, beta, globalGamma, rInitWeight;
	unsigned n, nCModels, nRClasses, cModel, rModel, rClass, bestCModel = 0,
	  templateIdx, *bestCModelTotalUsage;
	int deepestPosition, limit;
	FILE *fpOut = NULL, *fpBps = NULL, *fpInfo = NULL, *fpCWeights = NULL,
	  *fpRWeights = NULL, *fpEval = NULL, *fpEvalBps = NULL, *fpEvalInfo = NULL,
	    *fpConfig = NULL;
	unsigned maxCount = DEFAULT_PMODEL_MAX_COUNT, hSize = hSizes[1];
	CModel **cModels = NULL;
	RClass **rClasses = NULL;
	Template *cTemplate = NULL, *rTemplate = NULL;
	PModel **pModels = NULL, *mxPModel = NULL;
	FloatPModel *floatPModel = NULL;
	Sequence seq;
	double (* Power)(double x, double y) = Pow;
	char **xargv;
	int xargc;

	/* Needed to measure CPU time */
	clock_t tic, tac, start;
	double cpuTimeUsed;

	start = clock();

	cUpdateInvComps = 'n';
	rAllowInvComps = 'n';
	verbose = 'n';
	debug = 'n';
	globalGamma = ((int)(0.99 * 65536)) / 65536.0;
	beta = ((int)(0.90 * 65536)) / 65536.0;
	limit = 6;
	best = 'n';
	rInitWeight = 0.001;
	if(argc < 2)
		{
		fprintf(stderr, "Usage: DNAEncR1 [ -o outFile ]\n");
		fprintf(stderr, "                [ -v (print models info) ]\n");
		fprintf(stderr, "                [ -V (print debug info) ]\n");
		fprintf(stderr, "                [ -mc maxCount (def %d) ]\n",
		  DEFAULT_PMODEL_MAX_COUNT);
		fprintf(stderr, "                [ -info infoFile ]\n");
		fprintf(stderr, "                [ -bps bpsFile ]\n");
		fprintf(stderr, "                [ -cw cWeightsFile ]\n");
		fprintf(stderr, "                [ -rw rWeightsFile ]\n");
		fprintf(stderr, "                [ -riw rInitWeight (def %.3f) ]\n",
		  rInitWeight) ;
		fprintf(stderr, "                [ -a | -g gamma (def %.3f) ]\n",
		  globalGamma);
		fprintf(stderr, "                [ -best ]\n");
		fprintf(stderr, "                [ -cic (update inv comps cModels) ]\n");
		fprintf(stderr, "                [ -ric (allow inv comps rModels) ]\n");
		fprintf(stderr, "                [ -u template n/d ]\n");
		fprintf(stderr, "                [ -u template n/d ]\n");
		fprintf(stderr, "                ...\n");
		fprintf(stderr, "                [ -r template nReps beta limit n/d ]\n");
		fprintf(stderr, "                [ -r template nReps beta limit n/d ]\n");
		fprintf(stderr, "                ...\n");
		fprintf(stderr, "                [ -conf configFile ]\n");		
		fprintf(stderr, "                [ -e evalDataFile ]\n");
		fprintf(stderr, "                [ -eBps evalBpsFile ]\n");
		fprintf(stderr, "                [ -eInfo evalInfoFile ]\n");
		fprintf(stderr, "                [ -eInfoCM evalInfoCMFile ]\n");
		fprintf(stderr, "                [ -hs hashSize (def %d) ]\n", hSize);
		fprintf(stderr, "                [ -pow (standard lib pow) ]\n");
		fprintf(stderr, "                dataFile\n");
		exit(1);
		}


	if(!(xargv = (char **) malloc (argc * sizeof(char *))))
		{
		fprintf(stderr, "Error: in memory allocation\n");
		exit(1);
		}

	for (n = 0 ; n < argc - 1; n++) 
		{
		if(!(xargv[n] = (char *) malloc ((strlen(argv[n]) + 1) * sizeof(char))))
			{
			fprintf(stderr, "Error: in memory allocation\n");
			exit(1);
			}

		strcpy(xargv[n],argv[n]);
		}

	xargc = argc - 1;

	for(n = 1 ; n < argc ; n++)
		if(strcmp("-conf", argv[n]) == 0)
			{
			if(!(fpConfig = fopen(argv[n+1], "r")))
				{
				fprintf(stderr, "Error: can't open file\n");
				return 1;
				}
			else
				{
				char line[256];
				while (fgets(line, sizeof(line), fpConfig) != NULL) 
					{
					if(line[0] == '#')
						continue;

					if(line[strlen(line)-1] == '\n')
						line[strlen(line)-1] = '\0';

					char *result = NULL;
					result = strtok(line," ");
					while(result != NULL) 
						{
						if(!(xargv = (char **) realloc (xargv, (xargc+1) * sizeof(char *))))
							{
							fprintf(stderr, "Error: in memory allocation\n");
							exit(1);
							}

						if(!(xargv[xargc] = (char *) malloc ((strlen(result) + 1) * sizeof(char))))
							{
							fprintf(stderr, "Error: in memory allocation\n");
							exit(1);
							}

						strcpy(xargv[xargc++],result);
						result = strtok(NULL," ");
						}		
					}
				}

			break;
			}
	
	for(n = 1 ; n < xargc ; n++)
		if(!strcmp("-mc", xargv[n]))
			{
			maxCount = atoi(xargv[n+1]);
			if(maxCount == 0 || maxCount > DEFAULT_PMODEL_MAX_COUNT)
				fprintf(stderr, "Warning (maxCount): counters may overflow\n");

			break;
			}

	for(n = 1 ; n < xargc ; n++)
		if(strcmp("-o", xargv[n]) == 0)
			{
			if(!(fpOut = fopen(xargv[n+1], "w")))
				{
				fprintf(stderr, "Error: can't open file\n");
				return 1;
				}

			break;
			}

	for(n = 1 ; n < xargc ; n++)
		if(strcmp("-cw", xargv[n]) == 0)
			{
			if(!(fpCWeights = fopen(xargv[n+1], "w")))
				{
				fprintf(stderr, "Error: can't open file\n");
				return 1;
				}

			break;
			}

	for(n = 1 ; n < xargc ; n++)
		if(strcmp("-rw", xargv[n]) == 0)
			{
			if(!(fpRWeights = fopen(xargv[n+1], "w")))
				{
				fprintf(stderr, "Error: can't open file\n");
				return 1;
				}

			break;
			}

	for(n = 1 ; n < xargc ; n++)
		if(strcmp("-bps", xargv[n]) == 0)
			{
			if(!(fpBps = fopen(xargv[n+1], "w")))
				{
				fprintf(stderr, "Error: can't open file\n");
				return 1;
				}

			break;
			}

	for(n = 1 ; n < xargc ; n++)
		if(strcmp("-info", xargv[n]) == 0)
			{
			if(!(fpInfo = fopen(xargv[n+1], "w")))
				{
				fprintf(stderr, "Error: can't open file\n");
				return 1;
				}

			break;
			}

	for(n = 1 ; n < xargc ; n++)
		if(strcmp("-a", xargv[n]) == 0 || strcmp("-g", xargv[n]) == 0)
			{
			globalGamma = ((int)(atof(xargv[n+1]) * 65536)) / 65536.0;
			if(globalGamma >= 1 || globalGamma < 0)
				{
				fprintf(stderr, "Error: gamma should belong to [0, 1)\n");
				return 1;
				}

			break;
			}

	for(n = 1 ; n < xargc ; n++)
		if(strcmp("-riw", xargv[n]) == 0)
			{
			rInitWeight = atof(xargv[n+1]);
			break;
			}

	for(n = 1 ; n < xargc ; n++)
		if(strcmp("-hs", xargv[n]) == 0)
			{
			hSize = atoi(xargv[n+1]);
			break;
			}

	for(n = 1 ; n < xargc ; n++)
		if(strcmp("-cic", xargv[n]) == 0)
			{
			cUpdateInvComps = 'y';
			break;
			}

	for(n = 1 ; n < xargc ; n++)
		if(strcmp("-ric", xargv[n]) == 0)
			{
			rAllowInvComps = 'y';
			break;
			}

	for(n = 1 ; n < xargc ; n++)
		if(strcmp("-best", xargv[n]) == 0)
			{
			best = 'y';
			break;
			}

	for(n = 1 ; n < xargc ; n++)
		if(strcmp("-v", xargv[n]) == 0)
			{
			verbose = 'y';
			break;
			}

	for(n = 1 ; n < xargc ; n++)
		if(strcmp("-V", xargv[n]) == 0)
			{
			debug = 'y';
			break;
			}

	for(n = 1 ; n < xargc ; n++)
		if(strcmp("-pow", xargv[n]) == 0)
			{
			Power = pow;
			break;
			}

	nCModels = 0;
	for(n = 1 ; n < xargc ; n++)
		if(strcmp("-u", xargv[n]) == 0)
			nCModels++;

	if(nCModels == 0)
		{
		fprintf(stderr,"Error: at least one context model must be specified\n");
		return 1;
		}

	nRClasses = 0;
	for(n = 1 ; n < xargc ; n++)
		if(strcmp("-r", xargv[n]) == 0)
			nRClasses++;

	for(n = 1 ; n < xargc ; n++)
		if(strcmp("-e", xargv[n]) == 0)
			{
			/* Find out if the file can be open for reading */
			if(!(fpEval = fopen(xargv[n+1], "r")))
				{
				fprintf(stderr, "Error: unable to open file\n");
				return 1;
				}

			fclose(fpEval);

			/* Open the file for possible decompression with gzip */
			sprintf(fNameEval, "zcat -f %s", xargv[n+1]);
			if((fpEval = popen(fNameEval, "r")) == NULL)
				{
				fprintf(stderr, "Error: unable to open file\n");
				return 1;
				}

			break;
			}

	for(n = 1 ; n < xargc ; n++)
		if(strcmp("-eBps", xargv[n]) == 0)
			{
			if(!(fpEvalBps = fopen(xargv[n+1], "w")))
				{
				fprintf(stderr, "Error: can't open file\n");
				return 1;
				}

			break;
			}

	for(n = 1 ; n < xargc ; n++)
		if(strcmp("-eInfo", xargv[n]) == 0)
			{
			if(!(fpEvalInfo = fopen(xargv[n+1], "w")))
				{
				fprintf(stderr, "Error: can't open file\n");
				return 1;
				}

			break;
			}

	if(!(cTemplate = (Template *)Calloc(nCModels, sizeof(Template))))
		{
		fprintf(stderr, "Error: out of memory\n");
		return 1;
		}

	if(!(cModels = (CModel **)Calloc(nCModels, sizeof(CModel *))))
		{
		fprintf(stderr, "Error: out of memory\n");
		return 1;
		}

	if(nRClasses)
		{
		if(!(rTemplate = (Template *)Calloc(nRClasses, sizeof(Template))))
			{
			fprintf(stderr, "Error: out of memory\n");
			return 1;
			}

		if(!(rClasses = (RClass **)Calloc(nRClasses, sizeof(RClass *))))
			{
			fprintf(stderr, "Error: out of memory\n");
			return 1;
			}

		}

	if(!(pModels = (PModel **)Calloc(nCModels, sizeof(PModel *))))
		{
		fprintf(stderr, "Error: out of memory\n");
		return 1;
		}

	/*
	 * For a given symbol, stores the number of nats required by each cModel.
	 */
	if(!(cModelNats = (double *)Calloc(nCModels, sizeof(double))))
		{
		fprintf(stderr, "Error: out of memory\n");
		return 1;
		}

	/*
	 * Accumulates the nats associated with each cModel, i.e., gives
	 * the total performance of the cModel.
	 */
	if(!(cModelTotalNats = (double *)Calloc(nCModels, sizeof(double))))
		{
		fprintf(stderr, "Error: out of memory\n");
		return 1;
		}

	/*
	 * Accumulates the nats associated with each cModel, but only when it
	 * is the best model for that symbol. This gives the lower bound,
	 * assuming that an oracle indicates the best cModel for each symbol.
	 */
	if(!(bestCModelTotalNats = (double *)Calloc(nCModels, sizeof(double))))
		{
		fprintf(stderr, "Error: out of memory\n");
		return 1;
		}

	/*
	 * Indicates how many times each cModel was the best one for encoding
	 * a symbol.
	 */
	if(!(bestCModelTotalUsage = (unsigned *)Calloc(nCModels, sizeof(unsigned))))
		{
		fprintf(stderr, "Error: out of memory\n");
		return 1;
		}

	cModel = 0;
	for(n = 1 ; n < xargc ; n++)
		{
		if(strcmp("-u", xargv[n]) == 0)
			ParseListOfPositions(xargv[n+1], &cTemplate[cModel++]);

		}

	ReadSequence(&seq, argv[argc-1], CONV_AND_PACK_SEQUENCE);

	tic = clock();
	cpuTimeUsed = ((double) (tic-start)) / CLOCKS_PER_SEC;
    printf("Needed %g s for reading the sequence\n", cpuTimeUsed);
	printf("Sequence has a total of %u bases, %u of them unknown\n",
	  seq.size + seq.unknowns, seq.unknowns);
	printf("Compressing %u bases\n", seq.size);

	nRClasses = 0;
	for(n = 1 ; n < xargc ; n++)
		{
		if(strcmp("-r", xargv[n]) == 0)
			{
			unsigned maxRModels, alphaNum, alphaDen;

			ParseListOfPositions(xargv[n+1], &rTemplate[nRClasses]);
			if(sscanf(xargv[n+2], "%d", &maxRModels) != 1)
				{
				fprintf(stderr, "Error: invalid number of repeat models\n");
				return 1;
				}

			if(sscanf(xargv[n+3], "%lf", &beta) != 1)
				{
				fprintf(stderr, "Error: invalid beta\n");
				return 1;
				}

			beta = ((int)(beta * 65536)) / 65536.0;
			if(beta >= 1 || beta < 0)
				{
				fprintf(stderr, "Error: beta should belong to [0, 1)\n");
				return 1;
				}

			if(sscanf(xargv[n+4], "%d", &limit) != 1)
				{
				fprintf(stderr, "Error: invalid limit\n");
				return 1;
				}

			if(sscanf(xargv[n+5], "%d/%d", &alphaNum, &alphaDen) != 2)
				{
				alphaNum = DEFAULT_PMODEL_ALPHA_NUM;
				alphaDen = DEFAULT_PMODEL_ALPHA_DEN;
				}

			rClasses[nRClasses] = CreateRClass(maxRModels,
			  rTemplate[nRClasses].size, hSize, rInitWeight,
			  (double)alphaNum / alphaDen, globalGamma, beta, limit, rAllowInvComps);

			printf("Creating %d rModels (tSize: %d, alpha = %d/%d, gamma = "
			  "%.3f, beta = %.3f, limit = %d, ir = %c)\n", maxRModels, rTemplate[nRClasses].size,
			  alphaNum, alphaDen, globalGamma, beta, limit, rAllowInvComps);
			nRClasses++;
			}
		}

	deepestPosition = 0;
	for(cModel = 0 ; cModel < nCModels ; cModel++)
		{
		if(cTemplate[cModel].size == 0)
			continue;

		cTemplate[cModel].deepestPosition = cTemplate[cModel].position[0];
		for(n = 1 ; n < cTemplate[cModel].size ; n++)
			if(cTemplate[cModel].position[n] >
			  cTemplate[cModel].deepestPosition)
				cTemplate[cModel].deepestPosition =
				  cTemplate[cModel].position[n];

		if(deepestPosition < cTemplate[cModel].deepestPosition)
			deepestPosition = cTemplate[cModel].deepestPosition;

		}

	cModel = 0;
	for(n = 1 ; n < xargc ; n++)
		{
		if(strcmp("-u", xargv[n]) == 0)
			{
			unsigned alphaNum, alphaDen;

			if(sscanf(xargv[n+2], "%d/%d", &alphaNum, &alphaDen) != 2)
				{
				alphaNum = DEFAULT_PMODEL_ALPHA_NUM;
				alphaDen = DEFAULT_PMODEL_ALPHA_DEN;
				}

			if(sscanf(xargv[n+2], "g=%lf", &gamma) != 1 &&
			   (n+3 >= argc || sscanf(xargv[n+3], "g=%lf", &gamma) != 1))
				gamma = globalGamma;

			gamma = ((int)(gamma * 65536)) / 65536.0;
			if(gamma >= 1 || gamma < 0)
				{
				fprintf(stderr, "Error: gamma should belong to [0, 1)\n");
				return 1;
				}

			ir = cUpdateInvComps;
			if(strcmp("ir", xargv[n+2]) == 0 ||
			  (n+3 < xargc && strcmp("ir", xargv[n+3]) == 0) ||
			  (n+4 < xargc && strcmp("ir", xargv[n+4]) == 0 &&
			  strcmp("-u", xargv[n+2]) != 0))
				ir = 'y';

			cModels[cModel] = CreateCModel(cTemplate[cModel].size, N_SYMBOLS,
			  alphaNum, alphaDen, maxCount, hSize, gamma, ir);
			printf("Creating %llu pModels (tSize: %d, alpha "
			  "= %d/%d, gamma = %.3f, ir = %c)\n", cModels[cModel]->nPModels,
			  cModels[cModel]->ctxSize, alphaNum, alphaDen, gamma, ir);
			pModels[cModel] = CreatePModel(N_SYMBOLS);
			cModel++;
			}

		}

	mxPModel = CreatePModel(N_SYMBOLS);
	floatPModel = CreateFloatPModel(N_SYMBOLS);

	if(fpOut == NULL)
		fpOut = fopen("/dev/null", "w");

	startoutputtingbits();
	start_encode();

	WriteNBits(seq.size - 1, STORAGE_BITS_FILE_SIZE, fpOut);
	WriteNBits(maxCount, STORAGE_BITS_PMODEL_MAX_COUNT, fpOut);
	WriteNBits(cUpdateInvComps == 'y' ? 1 : 0,
	  STORAGE_BITS_INVERTED_REPEATS, fpOut);
	WriteNBits(hSize, STORAGE_BITS_HSIZE, fpOut);
	WriteNBits(nCModels - 1, STORAGE_BITS_N_CMODELS, fpOut);
	for(cModel = 0 ; cModel < nCModels ; cModel++)
		{
		WriteNBits(cTemplate[cModel].size, STORAGE_BITS_TEMPLATE_SIZE, fpOut);
		for(n = 0 ; n < cTemplate[cModel].size ; n++)
			WriteNBits(cTemplate[cModel].position[n] - 1,
			  STORAGE_BITS_TEMPLATE_POSITION, fpOut);

		WriteNBits(cModels[cModel]->alphaNum,
		  STORAGE_BITS_PMODEL_ALPHA_NUM, fpOut);
		WriteNBits(cModels[cModel]->alphaDen,
		  STORAGE_BITS_PMODEL_ALPHA_DEN, fpOut);
		}

	WriteNBits((int)(globalGamma * 65536), STORAGE_BITS_GAMMA, fpOut);
	WriteNBits(Power == pow ? 1 : 0, STORAGE_BITS_POWER_FUNCTION, fpOut);

	/*
	 * Encode
	 */
	for(n = 0 ; n < seq.size ; n++)
		{
		unsigned bestWeightCModel = 0;
		double bestWeight = 0;

		symbol = seq.bases[n];

		for(s = 0 ; s < N_SYMBOLS ; s++)
			floatPModel->freqs[s] = 0;

		/* Stop useless rModels */
		for(rClass = 0 ; rClass < nRClasses ; rClass++)
			{
			unsigned doitAgain;

			do
				{
				doitAgain = 0;
				for(rModel = 0 ; rModel < rClasses[rClass]->nRModels ; rModel++)
					{
					if((rClasses[rClass]->rModels[rModel].acting = 
					  rClasses[rClass]->beta * 
					    rClasses[rClass]->rModels[rModel].acting + 
					      rClasses[rClass]->rModels[rModel].lastHit) > 
					        rClasses[rClass]->limit * 1.0 || 
  						  rClasses[rClass]->rModels[rModel].pos == 0)
						{
						/* Stop this model */
						if(debug == 'y')
							{
							if(rClasses[rClass]->rModels[rModel].ic == 'n')
								printf("D RModel stopped: nTries = %d, "
								  "nHits: %d, nRModels: %d, pos: %d\n",
								  rClasses[rClass]->rModels[rModel].nTries,
								  rClasses[rClass]->rModels[rModel].nHits,
								  rClasses[rClass]->nRModels,
								  rClasses[rClass]->rModels[rModel].pos);
							else
								printf("R RModel stopped: nTries = %d, "
								  "nHits: %d, nRModels: %d, pos: %d\n",
								  rClasses[rClass]->rModels[rModel].nTries,
								  rClasses[rClass]->rModels[rModel].nHits,
								  rClasses[rClass]->nRModels,
								  rClasses[rClass]->rModels[rModel].pos);

							}

						/* If not last model in the list */
						if(rModel != rClasses[rClass]->nRModels - 1)
							rClasses[rClass]->rModels[rModel] =
							  rClasses[rClass]->rModels
							  [rClasses[rClass]->nRModels - 1];

						rClasses[rClass]->nRModels--;
						doitAgain = 1;
						break;
						}

					}

				}
			while(doitAgain);
			}

		/* Start new repeat models if there is room for them */
		for(rClass = 0 ; rClass < nRClasses ; rClass++)
			{
			if(rClasses[rClass]->nRModels < rClasses[rClass]->maxRModels &&
			  StartRModel(rClasses[rClass], rClasses[rClass]->nRModels,
			  TemplateIdx(seq.bases+n, &rTemplate[rClass]), 'n'))
				{
				if(debug == 'y')
					printf("D RModel started: refPos = %d, curPos: %d\n",
					  rClasses[rClass]->rModels
					  [rClasses[rClass]->nRModels].pos, n);

				rClasses[rClass]->nRModels++;
				}

			if(rClasses[rClass]->allowIC == 'y' &&
			  rClasses[rClass]->nRModels < rClasses[rClass]->maxRModels &&
			  StartRModel(rClasses[rClass], rClasses[rClass]->nRModels,
			  TemplateIdxC(seq.bases+n, &rTemplate[rClass]), 'y'))
				{
				if(debug == 'y')
					printf("R RModel started: refPos = %d, curPos: %d\n",
					  rClasses[rClass]->rModels
					  [rClasses[rClass]->nRModels].pos, n);

				rClasses[rClass]->nRModels++;
				}

			}

		for(rClass = 0 ; rClass < nRClasses ; rClass++)
			{
			templateIdx = TemplateIdx(seq.bases + n, &rTemplate[rClass]);
			InsertKmerPos(rClasses[rClass], templateIdx, n);
			}

		RenormalizeWeights(cModels, nCModels, fpCWeights, rClasses,
		  nRClasses, fpRWeights);

		bestCModelNats = DBL_MAX;
		for(cModel = 0 ; cModel < nCModels ; cModel++)
			{
			templateIdx = TemplateIdx(seq.bases + n, &cTemplate[cModel]);
			ComputePModel(cModels[cModel], pModels[cModel], templateIdx);
			nats = PModelSymbolNats(pModels[cModel], symbol);
			cModelNats[cModel] = nats;
			cModelTotalNats[cModel] += nats;
			if(nats < bestCModelNats)
				{
				bestCModelNats = nats;
				bestCModel = cModel;
				}

			if(cModels[cModel]->weight > bestWeight)
				{
				bestWeight = cModels[cModel]->weight;
				bestWeightCModel = cModel;
				}

			/*
			 * The probabilities estimated by each cModel are weighted
			 * according to the set of current weights.
			 */
			for(s = 0 ; s < N_SYMBOLS ; s++)
				floatPModel->freqs[s] += (double)pModels[cModel]->freqs[s] /
				  pModels[cModel]->sum * cModels[cModel]->weight;

			}

		for(rClass = 0 ; rClass < nRClasses ; rClass++)
			{
			for(rModel = 0 ; rModel < rClasses[rClass]->nRModels ; rModel++)
				{
				ComputeRModelProbs(&rClasses[rClass]->rModels[rModel],
				  seq.bases);

				/*
				 * The probabilities estimated by each rModel are weighted
				 * according to the set of current weights.
				 */
				for(s = 0 ; s < N_SYMBOLS ; s++)
					floatPModel->freqs[s] +=
					  rClasses[rClass]->rModels[rModel].probs[s] *
					  rClasses[rClass]->rModels[rModel].weight;

				}

			}

		bestCModelTotalUsage[bestCModel]++;
		bestCModelTotalNats[bestCModel] += bestCModelNats;
		mxPModel->sum = 0;
		for(s = 0 ; s < N_SYMBOLS ; s++)
			{
			mxPModel->freqs[s] = 1+(unsigned)(floatPModel->freqs[s] * maxCount);
			mxPModel->sum += mxPModel->freqs[s];
			}

		/*
		 * Instead of blending the models, use only the one having the
		 * largest weight.
		 */
		if(best == 'y')
			{
			templateIdx=TemplateIdx(seq.bases + n,&cTemplate[bestWeightCModel]);
			ComputePModel(cModels[bestWeightCModel], mxPModel, templateIdx);
			}

		ArithEncodeSymbol(symbol, (int *)(mxPModel->freqs),
		  (int)mxPModel->sum, fpOut);

		nats = PModelSymbolNats(mxPModel, symbol);
		bestTotalNats += bestCModelNats;
		for(cModel = 0 ; cModel < nCModels ; cModel++)
			{
			/*
			 * Update the weights according to the performance of the model.
			 */
			cModels[cModel]->weight =
			  Power(cModels[cModel]->weight, cModels[cModel]->gamma) *
			  (double)pModels[cModel]->freqs[symbol] / pModels[cModel]->sum;

			templateIdx = TemplateIdx(seq.bases+n, &cTemplate[cModel]);
			UpdateCModelCounter(cModels[cModel], templateIdx, symbol);
			if(cModels[cModel]->ir == 'y')
				{
				templateIdx = TemplateIdxC(seq.bases+n+1, &cTemplate[cModel]);
				symbolC = 3 - *(seq.bases+n-cTemplate[cModel].deepestPosition);
				UpdateCModelCounter(cModels[cModel], templateIdx, symbolC);
				}

			}

		for(rClass = 0 ; rClass < nRClasses ; rClass++)
			for(rModel = 0 ; rModel < rClasses[rClass]->nRModels ; rModel++)
				{
				/*
				 * Update the weights according to the performance of the model.
				 */
				rClasses[rClass]->rModels[rModel].weight =
				  Power(rClasses[rClass]->rModels[rModel].weight,
				  rClasses[rClass]->rModels[rModel].gamma) *
				  rClasses[rClass]->rModels[rModel].probs[symbol];

				UpdateRModel(&rClasses[rClass]->rModels[rModel],
				  seq.bases, symbol);
				}

		if(fpInfo != NULL)
			fprintf(fpInfo, "%.4f\n", nats / M_LN2);

		if(n % (seq.size / 100) == 0)
			fprintf(stderr, "%3d%%\r", (int)(n / (seq.size / 100)));

		if(fpBps != NULL)
			{
			fprintf(fpBps, "%d\t%d\t%d\t%.3f", n, bestCModel,
			  cModels[bestCModel]->ctxSize, nats / M_LN2 );
			if(nCModels > 1)
				{
				for(cModel = 0 ; cModel < nCModels ; cModel++)
					fprintf(fpBps, "\t%.3f", cModelNats[cModel] / M_LN2);

				fprintf(fpBps, "\t%.3f", cModelNats[bestCModel] / M_LN2);
				}

			fprintf(fpBps, "\n");
			}

		}

	finish_encode(fpOut);
	doneoutputtingbits(fpOut);

	tac = clock();

	if(verbose == 'y')
		{
		for(cModel = 0 ; cModel < nCModels ; cModel++)
			{
			if(cModels[cModel]->mode == FCM_HASH_TABLE_MODE)
				{
				CHashingStats(cModels[cModel]);
				printf("Keys / bases ...... %.1f\n",
				  (double)cModels[cModel]->hTable.nUsedKeys / seq.size);
				}
			else
				printf("Usage of PModels in CModel %d: %.2f%%, "
				  "used once %.2f%%\n", cModel + 1,
				  100 * FractionOfPModelsUsed(cModels[cModel]),
				  100 * FractionOfPModelsUsedOnce(cModels[cModel]));

			}

		if(nCModels > 1)
			for(cModel = 0 ; cModel < nCModels ; cModel++)
				printf("CModel %d was best %.2f%% with %.4f bpb, "
				  "global %.4f bpb\n", cModel + 1,
				  (100.0 * bestCModelTotalUsage[cModel]) / seq.size,
				  bestCModelTotalNats[cModel] / bestCModelTotalUsage[cModel] /
				  M_LN2, cModelTotalNats[cModel] / seq.size / M_LN2);

		for(rClass = 0 ; rClass < nRClasses ; rClass++)
			{
			RClassHashingStats(rClasses[rClass]);
			printf("Keys / bases ...... %.1f\n",
			  (double)rClasses[rClass]->hTable->nUsedKeys / seq.size);
			}

		}

	printf("Total number of bytes: %llu ( %.4f bpb ) ; Lower bound: %.4f bpb\n",
	  _bytes_output, _bytes_output * 8. / seq.size, bestTotalNats /
	  seq.size / M_LN2);

	/* Close the files */
	fclose(fpOut);
	if(fpConfig) fclose(fpConfig);
	if(fpBps) fclose(fpBps);
	if(fpInfo) fclose(fpInfo);
	if(fpCWeights) fclose(fpCWeights);
	if(fpRWeights) fclose(fpRWeights);

	cpuTimeUsed = ((double) (tac - tic)) / CLOCKS_PER_SEC;
    	printf("Needed %g s for compressing the bases\n", cpuTimeUsed);
	cpuTimeUsed = ((double) (tac - start)) / CLOCKS_PER_SEC;
    	printf("Total cpu time used: %g s\n", cpuTimeUsed);
	printf("Total memory allocated by (m/c/re)alloc: %.2f MB\n",
	  TotalMem() / 1024. / 1024.);

	return 0;
	}


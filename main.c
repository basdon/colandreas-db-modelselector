#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if !defined MAXMODELS
#define MAXMODELS 1000
#endif

int models[MAXMODELS];
int actualmodelcount;

#define MAX_FILENAME 50
char ifilename[MAX_FILENAME];
char ofilename[MAX_FILENAME];

FILE *ifile, *ofile;

void parseargs(int argc, char *argv[])
{
	int i;
	for (i = 0; i < argc; i++) {
		int arglen = strlen(argv[i]);
		if (arglen < 3 || argv[i][0] != '-') {
			continue;
		}
		if (arglen > MAX_FILENAME + 1) {
			printf("argument '%s' too long\n", argv[i]);
			continue;
		}
		switch (argv[i][1]) {
		case 'i':
			memcpy(ifilename, argv[i] + 2, arglen - 2);
			break;
		case 'o':
			memcpy(ofilename, argv[i] + 2, arglen - 2);
			break;
		}
	}
}

int validateargs()
{
	if (ifilename[0] == 0) {
		printf("specify the input file using -i<file> \n");
		return 1;
	}
	if (ofilename[0] == 0) {
		printf("specify the output file using -o<file> \n");
		return 1;
	}
	return 0;
}

int readmodels()
{
	actualmodelcount = 0;
	char modelid[10];
	char *result;
_1:
	result = fgets(modelid, 10, stdin);
	if (result && modelid[0] != '\r' && modelid[0] != '\n') {
		if (actualmodelcount == MAXMODELS) {
			printf("too many models specified\n");
			printf("compile with -DMAXMODELS=<number> to allow more (current: %d)\n", MAXMODELS);
			return 1;
		}
		models[actualmodelcount++] = atoi(modelid);
		if (!feof(stdin)) {
			goto _1;
		}
	}
	return 0;
}

int main(int argc, char *argv[])
{
	ifilename[0] = 0;
	ofilename[0] = 0;

	parseargs(argc, argv);
	if (validateargs()) {
		return 1;
	}
	if (readmodels()) {
		return 1;
	}

	ifile = fopen(ifilename, "r");
	if (ifile == NULL) {
		printf("could not open input file '%s' for reading\n", ifilename);
		return 1;
	}
	ofile = fopen(ofilename, "w");
	if (ifile == NULL) {
		printf("could not open output file '%s' for writing\n", ofilename);
		fclose(ifile);
		return 1;
	}

	fclose(ofile);
	fclose(ifile);

	return 0;
}


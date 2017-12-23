#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if !defined MAXMODELS
#define MAXMODELS 1000
#endif

int models[MAXMODELS];
int actualmodelcount;

#define MAX_FILENAME 50
char ifile[MAX_FILENAME];
char ofile[MAX_FILENAME];

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
			memcpy(ifile, argv[i] + 2, arglen - 2);
			break;
		case 'o':
			memcpy(ofile, argv[i] + 2, arglen - 2);
			break;
		}
	}
}

int validateargs()
{
	if (ifile[0] == 0) {
		printf("specify the input file using -i<file> \n");
		return 1;
	}
	if (ofile[0] == 0) {
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
	ifile[0] = 0;
	ofile[0] = 0;

	parseargs(argc, argv);
	if (validateargs()) {
		return 1;
	}
	if (readmodels()) {
		return 1;
	}

	return 0;
}


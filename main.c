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
int docopy;
char buf[255];

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

void copy(int bytes)
{
	int actualbytes = fread(buf, 1, bytes, ifile);
	if (actualbytes != bytes) {
		goto err;
	}
	if (!docopy) {
		return;
	}
	actualbytes = fwrite(buf, 1, bytes, ofile);
	if (actualbytes != bytes) {
		goto err;
	}
	return;
err:
	printf("(reading) possible corruption, missing %d bytes\n", bytes - actualbytes);
}

int readi(int bytes)
{
	int res = 0;
	int i;
	for (i = 0; i < bytes; i++) {
		int c = fgetc(ifile);
		if (c == EOF) {
			printf("(reading) possible corruption, missing a byte\n");
			continue;
		}
		res |= (c & 0xFF) << (i * 8);
	}
	return res;
}

void writei(int bytes, int _i)
{
	unsigned int i = (unsigned int) _i;
	while (docopy && bytes--) {
		if (fputc(i & 0xFF, ofile) == EOF) {
			printf("(writing) possible corruption, missing a byte\n");
		}
		i >>= 8;
	}
}

int copyi(int bytes)
{
	int i = readi(bytes);
	writei(bytes, i);
	return i;
}

int domodel(int modelid)
{
	int i;
	for (i = 0; i < actualmodelcount; i++) {
		if (modelid == models[i]) {
			return 1;
		}
	}
	return 0;
}

int process()
{
	docopy = 1;

	// 4 cadf
	// 2 version
	// 2 colCount
	// 4 iplCount
	//         colCount x
	//         2 modelid
	//         2 spherecount
	//         2 boxcount
	//         2 facecount
	//                 spherecount x
	//                 16 sizeof colsphere
	//                 boxcount x
	//                 24
	//                 facecount x
	//                 12 sizeof vertex
	//                 12 sizeof vertex
	//                 12 sizeof vertex
	//         iplcount x
	//                 2 modelid
	//                 12 Vertex
	//                 16 IPLRot

	copy(4); // 'cadf'
	copy(2); // version
	int colcount = copyi(2); // colcount (will be overridden)
	int iplcount = readi(4); // iplcount
	writei(4, 0); // iplcount

	printf("colcount %d\n", colcount);
	printf("iplcount %d\n", iplcount);

	int actualcols = 0;
	int i, j;

	for (i = 0; i < colcount; i++) {
		docopy = 0;
		int modelid = readi(2);
		if (domodel(modelid)) {
			docopy = 1;
			++actualcols;
			writei(2, modelid);
		}

		int spherecount = copyi(2);
		int boxcount = copyi(2);
		int facecount = copyi(2);
		for (j = 0; j < spherecount; j++) {
			copy(16);
		}
		for (j = 0; j < boxcount; j++) {
			copy(24);
		}
		for (j = 0; j < facecount; j++) {
			copy(36);
		}
	}

	printf("actual cols %d\n", actualcols);
	return actualcols;
}

void adjustcols(int actualcols)
{
	ofile = fopen(ofilename, "rb+");
	if (ofile == NULL) {
		printf("could not open output file '%s' to change actual colcount, actualcols will be wrong\n", ofilename);
		return;
	}

	if (fseek(ofile, /*offset*/ 6, /*origin*/ SEEK_SET)) {
		printf("could not reposition output stream, actualcols will be wrong\n");
		fclose(ofile);
		return;
	}

	char bytes[] = { (0xFF & actualcols), (0xFF & (actualcols>>8))};
	fwrite(bytes, 2, 1, ofile);

	fclose(ofile);
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

	ifile = fopen(ifilename, "rb");
	if (ifile == NULL) {
		printf("could not open input file '%s' for reading\n", ifilename);
		return 1;
	}
	ofile = fopen(ofilename, "wb");
	if (ofile == NULL) {
		printf("could not open output file '%s' for writing\n", ofilename);
		fclose(ifile);
		return 1;
	}

	int actualcols = process();

	fclose(ofile);
	fclose(ifile);

	adjustcols(actualcols);

	return 0;
}


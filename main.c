/*==============================================================
 * main.c 
 * written on 01-08-2015
 This main function is for testing the C code of the MEX parser package.
 Memory error causes MATLAB to crash.
 To avoid repeatedly start MATLAB, it is a good idea to test the C code in CYGWIN first.

 * Copyright 2014-2015 by Guoyong SHI (c)
 *=============================================================*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "parse_func.h"
#include "MNA_Matrix.h"
#include "Symbol_Table.h"
#include "parse.yy.c"		// generated by flex
#include "parse.tab.c"		// generated by bison

void Call_Parser(char *filename)
{
	FILE *inFile;

	printf("\nFile name: %s\n", filename);

	inFile = fopen(filename, "r");
	if (!inFile) {
		printf("Did not find the file \"%s\".\n", filename);
		return;
	}

	Init_Symbol_Tables();
	Init_parse_util();	// initialize vars for utility functions

	yyin = inFile;
	printf("Device Table:\n");
	yyparse();			// run parser
	fclose(inFile);

	Summarize();
	Nodetail->next=NULL;
	Index_All_Nodes();  //Index all nodes
	Init_MNA_System();
	Create_MNA_Matrix();
	Print_MNA_System(); //print MNA Matrix
	
}

/**
  This main function is for driving the program in pure UNIX/CYGWIN OS.
  There is a mex_function() defined in "mex_parse.c" which is used as the driver in MATLAB.
*/
int main(int argc, char **args)
{
	FILE *inFile;

	// Test data access
	double *A;
	double *b;
	int dim;
	int i, j;
        int NodeSize;

	if (argc < 2) {
		printf("Please provide an input file.\n");
		exit(0);
	}
	//run call parser
	Call_Parser( args[1] );

	return (0);
}


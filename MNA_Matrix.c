/**
	MNA_Matrix.c
	For the undergraduate EDA course offered at 
	Dept of Micro/Nano-electronics

	(c) Guoyong Shi, 
	All rights reserved, 2006-2015
	Shanghai Jiao Tong University

	If you modify the code, please keep the above copyright info.
*/

#ifdef MATLAB
#include "mex.h"
#endif

#include <stdio.h> 
#include <stdlib.h> 
#include <string.h>
#include "Symbol_Table.h"
#include "parse_func.h"
#include "MNA_Matrix.h"
#define WARNING_DIV_BY_ZERO \
		printf("\nWarning: divide by zero.");

int MatrixSize;	// size of the MNA matrix (i.e., the max dimension)
double **MNAMatrix_R = NULL;
double **MNAMatrix_I = NULL;
double *RHS = NULL;
int NodeSize = 0;
/**
	Assign indexes to all nodes in the node table.
	The ground node (with name "0") must be assigned index zero.
	The rest nodes are assigned indexes from 1, 2, 3 continuously.
*/
void Index_All_Nodes()
{
	// MatrixSize = 3;
	Node_Entry *NDScan; // Nodehead should be given as global parameter
	Nodeindex *Indexhead, *q, *Indextail;
	NDScan = Nodehead;
	//q = Indexhead;
	int index = 0;
	
	/*Initial Nodeindex*/
	Indexhead=(Nodeindex *)malloc(sizeof(Nodeindex));
	Indexhead->Hash = -1;
	Indexhead->name = NULL;
	Indexhead->index = -1;
	Indexhead->next = NULL;

	Indextail=Indexhead;
	printf("NodeTable: %d\n");
	while(NDScan != Nodetail){
		int flag = 0;
		q=Indexhead->next;
		
		while(q !=NULL){    //Scan Nodeindex link table
	    if(NameHash(NDScan->next->name,nodenum) == q->Hash){   //Compare Hash
			if(strcmp(NDScan->next->name,q->name)==0){
				NDScan->next->index = q->index;flag=1;
			//	printf("index: %d\n",index);
			} 
			//If name is same, use allocated index, break
		}
		q=q->next;
	}
		if(flag==0){	// if is new name, allocate index and add to Nodeindex link table. 
			NDScan->next->index = index;
			Nodeindex *newnode;
			newnode=(Nodeindex *)malloc(sizeof(Nodeindex));
			newnode->name = NDScan->next->name;
			newnode->Hash = NameHash(NDScan->next->name,nodenum);
			newnode->index = index;
			newnode->next = NULL;
			Indextail->next=newnode;
			Indextail=Indextail->next;
			index++;
		}
	NDScan = NDScan->next;
		printf("node:%s\t  index:%d \n",NDScan->name, NDScan->index);
	}
        NodeSize = index;
        printf("NodeSize = %d", NodeSize);
}

int Get_Matrix_Size()
{
	return MatrixSize;
}

/**
	The MATLAB matrix data are stored in column-major format.
*/
void Get_MNA_System(double **A, double **b)
{
	int i, j;

	for (j = 0; j <= MatrixSize; j++) {
		for (i = 0; i <= MatrixSize; i++) {
			(*A)[i+j*(MatrixSize+1)] = MNAMatrix_R[i][j];  // convert to column-major format
		}
		(*b)[j] = RHS[j];
	}
}


/**
	Create memory for MNAMatrix_R and RHS.
	The adopted storage is in DENSE matrix format.
	The zeroth row/col correspond to the ground.

	Must call Index_All_Nodes() before calling this function.

	(!!) Students can try SPARSE matrix format.
*/
void Init_MNA_System(){
#if 1
	int i, j;
    MatrixSize = nVsrc+NodeSize; // MNA Matrix Size 

	MNAMatrix_R = (double**) malloc( (MatrixSize+1) * sizeof(double*) );
	for (i = 0; i <= MatrixSize; i++) {
		MNAMatrix_R[i] = (double*) malloc( (MatrixSize+1) * sizeof(double) );
	}

	MNAMatrix_I = (double**) malloc( (MatrixSize+1) * sizeof(double*) );
	for (i = 0; i <= MatrixSize; i++) {
		MNAMatrix_I[i] = (double*) malloc( (MatrixSize+1) * sizeof(double) );
	}

	RHS = (double*) malloc( (MatrixSize+1) * sizeof(double) );

	// Initialize MNA matrix and RHS to zero
	for (i = 0; i < MatrixSize; i++) {
		for (j = 0; j < MatrixSize; j++) {
			MNAMatrix_R[i][j] = 0.0;
			MNAMatrix_I[i][j]=0.0;
		}
		RHS[i] = 0.0;
	}
#endif
}

/**
	Create the MNA matrix by scanning the device table.
	In principle stamping of each element should be done by the device instance.
	In this example code we do not create device objects. 
	Therefore, stamping is done inside this function for simplicity.
	
	(!!) Students are required to develop device classes to fulfil this task.
*/
void Create_MNA_Matrix()
{
  Device_Entry *Dnode;
  int i = 0;
  int j = 0;
  printf("nRes%d\n", nRes);
  printf("MatrixSize%d\n",MatrixSize);
  
  //Build MNA for Res device stamp
	Dnode = Reshead->next;
	for (i = 0; i < nRes; i++){		
   		MNAMatrix_R[Dnode->nodelist->index][Dnode->nodelist->index] += 1 / Dnode->value;
   		MNAMatrix_R[Dnode->nodelist->next->index][Dnode->nodelist->next->index] += 1 / Dnode->value;
   		MNAMatrix_R[Dnode->nodelist->index][Dnode->nodelist->next->index] += -1 / Dnode->value;
   		MNAMatrix_R[Dnode->nodelist->next->index][Dnode->nodelist->index] += -1 / Dnode->value;
		Dnode = Dnode->next;
	}
	//Build MNA for Cap device stamp
	Dnode = Caphead->next;
	for (i = 0; i < nCap; i++){		
 		MNAMatrix_I[Dnode->nodelist->index][Dnode->nodelist->index] += 1 / Dnode->value;
   		MNAMatrix_I[Dnode->nodelist->next->index][Dnode->nodelist->next->index] += 1 / Dnode->value;
   		MNAMatrix_I[Dnode->nodelist->index][Dnode->nodelist->next->index] += -1 / Dnode->value;
   		MNAMatrix_I[Dnode->nodelist->next->index][Dnode->nodelist->index] += -1 / Dnode->value;
		Dnode = Dnode->next;
	}
	//Build MNA for Ind device stamp
	Dnode = Indhead->next;
	for (i = 0; i < nInd; i++){		
  		MNAMatrix_I[Dnode->nodelist->index][Dnode->nodelist->index] +=  Dnode->value;
  		MNAMatrix_I[Dnode->nodelist->next->index][Dnode->nodelist->next->index] +=  Dnode->value;
   		MNAMatrix_I[Dnode->nodelist->index][Dnode->nodelist->next->index] += -Dnode->value;
  		MNAMatrix_I[Dnode->nodelist->next->index][Dnode->nodelist->index] += -Dnode->value;
		Dnode = Dnode->next;
	}
	//Build MNA for Vsrc device stamp
	Dnode = Vsrchead->next;
	for (i = 0; i < nVsrc; i++){
		MNAMatrix_R[Dnode->nodelist->index][NodeSize + i] += 1;
		MNAMatrix_R[Dnode->nodelist->next->index][NodeSize + i] += -1;
		MNAMatrix_R[NodeSize + i][Dnode->nodelist->next->index] += -1;
		MNAMatrix_R[NodeSize + i][Dnode->nodelist->index] += 1;
		RHS[NodeSize + i] += Dnode->value;
		Dnode = Dnode->next;
	}
	//Build MNA for Isrc device stamp
	Dnode = Isrchead->next;
	for (i = 0; i < nIsrc; i++){
		RHS[Dnode->nodelist->index] += - Dnode->value;
		RHS[Dnode->nodelist->next->index] += Dnode->value;
		Dnode = Dnode->next;
	}
	//Build MNA for VCCShead device stamp
	Dnode = VCCShead->next;
	for (i = 0; i < nVCCS; i++){
		MNAMatrix_R[Dnode->nodelist->index][Dnode->nodelist->next->next->index] += Dnode->value;
		MNAMatrix_R[Dnode->nodelist->index][Dnode->nodelist->next->next->next->index] += - Dnode->value;
		MNAMatrix_R[Dnode->nodelist->next->index][Dnode->nodelist->next->next->index] += - Dnode->value;
		MNAMatrix_R[Dnode->nodelist->next->index][Dnode->nodelist->next->next->next->index] += Dnode->value;
		Dnode = Dnode->next;

	}
}


void Print_MNA_System()
{
	int i, j;

	printf("\n\n");
	for (j = 0; j <= MatrixSize; j++) {
		printf("\t%-24d", j);
	}
	printf("\tRHS");
	
	for (i = 0; i <= MatrixSize; i++) {
		printf("\n[%-3d]", i);
		for (j = 0; j <= MatrixSize; j++) {
			printf("\t%-6f+s*%-18f", MNAMatrix_R[i][j],MNAMatrix_I[i][j]);
		}
		printf("\t%-12f", RHS[i]);
	}
}



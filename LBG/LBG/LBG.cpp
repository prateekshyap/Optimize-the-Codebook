// LBG.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <stdio.h>
#include <conio.h>
#include <stdlib.h>
#include <ctype.h>
#include <math.h>
#include <string.h>
#include <time.h>

double ** readUniverseFile(char * fileName, int universeSize, int p, int rowSize)
{
	char * line;
	
	FILE * file = NULL;
	char * buffer = new char[rowSize];
	int i = 0, row = 0, column = 0;

	double ** universe = new double*[universeSize];
	for (i = 0; i < universeSize; ++i)
		universe[i] = new double[p];

	file = fopen(fileName,"r");
	while (fgets(buffer,rowSize,file)) //till there are more rows
	{
		line = strtok(buffer,","); //split the row by delimiter ","
		column = 0;
		while (line != NULL) //till there are more numbers
		{
			universe[row][column] = atof(line); //store the number in universe
			++column;
			line = strtok(NULL,","); //proceed for the next number
		}
		++row;
	}
	return universe;
}

double * getInitialCodebook(double ** universe, int universeSize, int p)
{
	int i = 0, j = 0;
	double * codebook = new double[p];

	//store 0 in the entire codebook
	for (j = 0; j < p; ++j)
		codebook[j] = 0;

	//add all the vectors
	for (i = 0; i < universeSize; ++i)
		for (j = 0; j < p; ++j)
			codebook[j] += universe[i][j];

	//take the average
	for (j = 0; j < p; ++j)
		codebook[j] /= universeSize;

	//uncomment to print the codebook
	/*
	for (j = 0; j < p; ++j)
		printf("%lf ",codebook[j]);
	printf("\n");
	*/
	return codebook;
}

double ** KMeans(double ** universe, int universeSize, double ** codebook, int K, int p, double delta, double w[], FILE * consoleFile)
{
	int M = 0, minIndex = 9999999; //minIndex stores the index of the code vector with minimum distortion
	/*previous distortion stores the distortion of iteration m-1
	current distortion stores the distortion of iteration m
	min stores the distortion value of the code vector with minimum distortion
	totalDistortion stores the total distortion of the universe*/
	double previousDistortion = 0.0, currentDistortion = 1000.0, min = 1000000000.0, totalDistortion = 0.0;
	long long int iterationCount = 0; //total number of iterations in kmeans
	/*bucketNumber stores values from 0 to N-1 included i.e. the respective bucket numbers of all the vectors in the universe
	bucketSize stores the number of x vectors of each bucket*/
	int * bucketNumber = new int[universeSize], * bucketSize = new int[K]; //should lie between 0 to K-1 included
	double * distances = new double[K]; //stores the tokhura distances
	double ** bucketSum = new double*[K]; //stores the sum of buckets
	int i = 0, j = 0, k = 0, m = 0; //loop variables

	//initialize everything to 0
	for (i = 0; i < K; ++i)
		bucketSum[i] = new double[p];
	for (i = 0; i < K; ++i)
		bucketSize[i] = 0;
	for (i = 0; i < K; ++i)
		for (j = 0; j < p; ++j)
			bucketSum[i][j] = 0;

	//store the iteration details to a text file	
	fprintf(consoleFile,"Initial Codebook - \n");
	for (i = 0; i < K; ++i)
	{
		for (j = 0; j < p; ++j)
			fprintf(consoleFile,"%lf ",codebook[i][j]);
		fprintf(consoleFile,"\n");
	}
	fprintf(consoleFile,"\n");
	
	i = 0;
	j = 0;
	while (abs(currentDistortion-previousDistortion) >= delta) //till the difference is more than the threshold
	{
	++iterationCount;
	totalDistortion = 0;

	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/*classification*/
	for (i = 0; i < universeSize; ++i) //for each vector in the universe
	{
		min = 1000000000.0; //reset the minimum

		//find out the tokhura distances from each vector in the codebook
		for (j = 0; j < K; ++j) //for each vector in the codebook
		{
			distances[j] = 0;
			for (k = 0; k < p; ++k) //for each p value
			{
				distances[j] += w[k]*(universe[i][k]-codebook[j][k])*(universe[i][k]-codebook[j][k]); //find out the distance
			}
		}

		//find out the minimum
		for (j = 0; j < K; ++j)
		{
			if (min > distances[j])
			{
				min = distances[j];
				minIndex = j;
			}
		}

		//store the minIndex in the bucketNumber array
		bucketNumber[i] = minIndex;

		//increase the corresponding bucket size
		++bucketSize[minIndex];

		//update the maximum bucket size
		if (bucketSize[minIndex] > M)
			M = bucketSize[minIndex];

		//add the current vector to the bucket sum
		for (j = 0; j < p; ++j)
			bucketSum[minIndex][j] += universe[i][j];

		//add the current distortion to the distortion sum
		totalDistortion += min;

		//uncomment to print the bucket sizes
		/*
		for (j = 0; j < K; ++j)
			printf("%d ",bucketSize[j]);
		printf("\n");
		*/

		//uncomment to print the respective bucket numbers
		/*
		printf("%d: %d\n",i,bucketNumber[i]);
		*/
	}

	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/*code vector updation*/

	//taking the average and updating the codebook
	for (i = 0; i < K; ++i)
		for (j = 0; j < p; ++j)
			codebook[i][j] = bucketSum[i][j]/bucketSize[i];

	//uncomment to print the updated codebook
	/*
	printf("\n\n");
	for (i = 0; i < K; ++i)
	{
		for (j = 0; j < p; ++j)
			printf("%lf ",codebook[i][j]);
		printf("\n");
	}
	*/

	previousDistortion = currentDistortion; //store the previous distortion
	currentDistortion = totalDistortion / universeSize; //find out the current distortion

	fprintf(consoleFile,"Iteration number - %lld\nUpdated Codebook -\n",iterationCount);
	for (i = 0; i < K; ++i)
	{
		for (j = 0; j < p; ++j)
			fprintf(consoleFile,"%lf ",codebook[i][j]);
		fprintf(consoleFile,"\n");
	}
	fprintf(consoleFile,"Distortion - %lf\n\n",currentDistortion);
	//printf("m = %d, prev = %lf, curr = %lf\n",m,previousDistortion,currentDistortion); //print

	}

	printf("Total iterations = %lld\nDistortion difference = %lf\nCurrent Distortion = %lf\n",iterationCount,abs(currentDistortion-previousDistortion),currentDistortion);
	return codebook;
}

double ** LBG(double ** universe, int universeSize, double * initialCodebook, int K, int p, double delta, double epsilon, double w[])
{
	double ** tempCodebook = NULL, ** codebook = NULL;
	int codebookSize = 1;
	int i = 0, j = 0;

	codebook = new double*[1];
	for (i = 0; i < 1; ++i)
		codebook[i] = new double[p];

	//store the initial codebook in codebook
	for (i = 0; i < 1; ++i)
		for (j = 0; j < p; ++j)
			codebook[i][j] = initialCodebook[j];

	//write output to console file
	FILE * consoleFile = fopen("ConsoleOutput.txt","w");
	printf("\nIteration wise codebook and distortions are printed in a file named ConsoleOutput.txt\n");

	while(codebookSize != K) //till the desired size is not reached
	{
		printf("\nNew Codebook size = %d\n",2*codebookSize);
		tempCodebook = new double*[2*codebookSize]; //create a double sized codebook
		for (j = 0; j < 2*codebookSize; ++j)
			tempCodebook[j] = new double[p];

		//split
		for (i = 0; i < codebookSize; ++i)
		{
			for (j = 0; j < p; ++j)
			{
				tempCodebook[2*i][j] = codebook[i][j]*(1+epsilon);
				tempCodebook[(2*i)+1][j] = codebook[i][j]*(1-epsilon);
			}
		}

		codebook = tempCodebook; //store the new codebook
		codebookSize *= 2; //update the new codebook size
		codebook = KMeans(universe, universeSize, codebook, codebookSize, p, delta, w, consoleFile); //run k means for optimal codebook
	}
	fclose(consoleFile);
	return codebook;
}
int _tmain(int argc, _TCHAR* argv[])
{
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/*variable declaration*/
	char * universeFileName = "Universe/Universe.csv"; //universe file name
	/*universeSize is the total number of vectors given
	p stores the value of p
	rowSize stores the size of one row of the .csv file
	K is the codebook size
	*/
	int universeSize = 6340, p = 12, rowSize = 1024, K = 8;
	double epsilon = 0.03, delta = 0.00001; // or 0.000001
	double w[] = {1.0,3.0,7.0,13.0,19.0,22.0,25.0,33.0,42.0,50.0,56.0,61.0}; //given Tokhura weights
	
	double ** universe = NULL;
	double ** codebook = NULL;
	double * initialCodebook = NULL;
	int i = 0, j = 0; //loop variables

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/*execution*/
	
	universe = readUniverseFile(universeFileName, universeSize, p, rowSize); //get the universe file

	initialCodebook = getInitialCodebook(universe, universeSize, p); //generate initial codebook

	//print it
	printf("\n********************************************\n");
	printf("***************Initial Codebook*************\n");
	printf("********************************************\n");
	for (j = 0; j < p; ++j)
		printf("%lf ",initialCodebook[j]);
	printf("\n");
	
	codebook = LBG(universe, universeSize, initialCodebook, K, p, delta, epsilon, w); //run K Means

	//print the optimal code book
	printf("\n\n********************************************\n");
	printf("****************Final Codebook**************\n");
	printf("********************************************\n");
	for (i = 0; i < K; ++i)
	{
		for (j = 0; j < p; ++j)
			printf("%lf ",codebook[i][j]);
		printf("\n");
	}

	printf("Breakpoint");
	
	return 0;
}


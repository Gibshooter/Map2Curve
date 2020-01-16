#include "face.h"
#include<bits/stdc++.h>

#ifndef __LSE_H_INCLUDED__
#define __LSE_H_INCLUDED__

using namespace std;

//https://www.geeksforgeeks.org/gaussian-elimination/

#define N 3        // Number of unknowns 

void SetMat(double mat[3][4], face Faces[3]);
void SetMat(double mat[3][4], face &F1, face &F2, face &F3);

// function to reduce matrix to r.e.f.  Returns a value to  
// indicate whether matrix is singular or not 
int forwardElim(double mat[N][N+1]);

// function to calculate the values of the unknowns 
void backSub(double mat[N][N+1], vertex &Isect);

bool gaussianElimination(double mat[N][N+1], vertex &Isect);
void swap_row(double mat[N][N+1], int i, int j);
void print(double mat[N][N+1]);




#endif

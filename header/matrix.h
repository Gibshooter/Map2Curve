#ifndef __MATRIX_H_INCLUDED__
#define __MATRIX_H_INCLUDED__

#include <iostream>

#define PI 3.14159265

using namespace std;

/* ===== EULER CLASS & FUNCTIONS ===== */

struct Euler
{
	float Pitch = 0;
	float Yaw = 0;
	float Roll = 0;
	
	Euler(float Angles[3]);
	Euler(float roll, float pitch, float yaw);
	Euler(){}
};

ostream &operator<<(ostream &ostr, Euler &eul);


/* ===== MATRIX CLASS & FUNCTIONS ===== */

struct Matrix
{
	float E[3][3]; // Elements
	
	void EulerToMatrix(Euler A);
	Euler MatrixToEuler();
	void LevelFields(int deci);
	
	Matrix();
};

ostream &operator<<(ostream &ostr, Matrix &M);

Matrix MatrixMultiply(Matrix M1, Matrix M2);


#endif

#include "matrix.h"
#include <math.h>
#include <iostream>

using namespace std;


/* ===== EULER METHODS & FUNCTIONS ===== */

Euler::Euler(float Angles[3])
{
	Roll = Angles[0];
	Pitch = Angles[1];
	Yaw = Angles[2];
}

Euler::Euler(float roll, float pitch, float yaw)
{
	Roll = roll;
	Pitch = pitch;
	Yaw = yaw;
}

ostream &operator<<(ostream &ostr, Euler &eul)
{
	return ostr << "( " << eul.Roll << " " << eul.Pitch << " " << eul.Yaw << " )";
}



/* ===== MATRIX METHODS & FUNCTIONS ===== */

void Matrix::LevelFields(int deci)
{
	for (int i = 0; i<3; i++)
		for (int j = 0; j<3; j++) {
			float &F = E[i][j];
			float m = 1/pow(10,deci);
			if (F<m&&F>-m) 		F = 0;
			else if (F>1-m) 	F = 1;
			else if (F<-1+m) 	F = -1;
		}
}

void Matrix::EulerToMatrix(Euler A)
{
	float P = A.Pitch*PI/180.0;
	float Y = A.Yaw*PI/180.0;
	float R = A.Roll*PI/180.0;
	
	float CP = cos(P);
	float SP = sin(P);
	float CY = cos(Y);
	float SY = sin(Y);
	float CR = cos(R);
	float SR = sin(R);
	
	E[0][0] = CP*CY;	E[0][1] = (SR*SP*SY)+(CR*-SY);		E[0][2] = (CR*SP*CY)+(-SR*-SY);
	E[1][0] = CP*SY;	E[1][1] = (SR*SP*SY)+(CR*CY);		E[1][2] = (CR*SP*SY)+(-SR*CY);
	E[2][0] = -SP;		E[2][1] = SR*CP;					E[2][2] = CR*CP;
	
	// case 2 SP = -1
	
	// case 3 SP = 1
	
	//E[0][0] = Cy*Cz;	E[0][1] = (Sx*Sy*Sz)+(Cx*-Sz);		E[0][2] = (Cx*Sy*Cz)+(-Sx*-Sz);
	//E[1][0] = Cy*Sz;	E[1][1] = (Sx*Sy*Sz)+(Cx*Cz);		E[1][2] = (Cx*Sy*Sz)+(-Sx*Cz);
	//E[2][0] = -Sy;		E[2][1] = Sx*Cy;					E[2][2] = Cx*Cy;
	LevelFields(6);
}

Euler Matrix::MatrixToEuler()
{
	bool dev = 0;
	float &R00 = E[0][0]; float &R01 = E[0][1]; float &R02 = E[0][2];
	float &R10 = E[1][0]; float &R11 = E[1][1]; float &R12 = E[1][2];
	float &R20 = E[2][0]; float &R21 = E[2][1]; float &R22 = E[2][2];
	if (dev) cout << " Matrix to Euler..." << endl;
	Euler A;
	//float R20R = round(R20);
	if (dev) cout << "   R20 (-Sin(Pitch)) " << R20 <<  endl;
	if (R20<1) {
		if (R20>-1) { // R20(sin(Pitch)) != -1 && != 1
			if (dev) cout << "     Case I: Pitch != 180/-180" << endl;
			A.Roll 	= atan2(R21, R22);
			A.Pitch = asin(R20);
			A.Yaw 	= -atan2(-R10, R00);
		} else { // R20(sin(Pitch)) = -1 = -90  | cos(Pitch) = 0
			if (dev) cout << "     Case II: Pitch = 90" << endl;
			A.Roll 	= 0;
			A.Pitch = PI/2;
			A.Yaw 	= -atan2(R01, R02);
		}
	} else { // R20(sin(Pitch)) = 1 = 90  | cos(Pitch) = 0
		if (dev) cout << "     Case III: Pitch = -90" << endl;
		A.Roll 	= 0;
		A.Pitch = -PI/2;
		A.Yaw 	= atan2(-R01, R11);
	}
	A.Roll *= 180.0 / PI; A.Pitch *= 180.0 / PI; A.Yaw *= 180.0 / PI;
	
	//A.Roll 	= atan2(R21, R22) / PI * 180.0;
	//A.Pitch = asin(R20) / PI * 180.0;
	//A.Yaw 	= -atan2(-R10, R00) / PI * 180.0;
	
	//if (dev) cout << "     Final Euler " << floorf(A.Roll*1000)/1000 << " " << floorf(A.Pitch*1000)/1000 << " " << floorf(A.Yaw*1000)/1000 << endl << endl;
	return A;
}

Matrix::Matrix()
{
	E[0][0] = 1.0;  E[0][1] = 0.0;  E[0][2] = 0.0;
	E[1][0] = 0.0;  E[1][1] = 1.0;  E[1][2] = 0.0;
	E[2][0] = 0.0;  E[2][1] = 0.0;  E[2][2] = 1.0;
}

ostream &operator<<(ostream &ostr, Matrix &M)
{
	for(int i = 0; i<3;i++)
		ostr << "[ " << M.E[i][0] << "\t|\t" << M.E[i][1] << "\t|\t" << M.E[i][2] << " ]\n";
	return ostr;
}


Matrix MatrixMultiply(Matrix M1, Matrix M2)
{
	Matrix MP; // Matrix Product
	
	for (int i = 0; i<3; i++) // multiply Lines of M1 with Rows of M2
	{
		MP.E[i][0] = (M1.E[i][0]*M2.E[0][0]) + (M1.E[i][1]*M2.E[1][0]) + (M1.E[i][2]*M2.E[2][0]);
		MP.E[i][1] = (M1.E[i][0]*M2.E[0][1]) + (M1.E[i][1]*M2.E[1][1]) + (M1.E[i][2]*M2.E[2][1]);
		MP.E[i][2] = (M1.E[i][0]*M2.E[0][2]) + (M1.E[i][1]*M2.E[1][2]) + (M1.E[i][2]*M2.E[2][2]);
	}
	MP.LevelFields(6);
	return MP;
}



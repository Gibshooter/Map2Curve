#ifndef __GVECTOR_H_INCLUDED__
#define __GVECTOR_H_INCLUDED__

#include "vertex.h"
#include <iostream>

using namespace std;

/* ===== GVECTOR CLASS ===== */

struct gvector
{
	float x = 0.0;
	float y = 0.0;
	float z = 0.0;
	bool IsHor = 0;
	bool IsNeg = 0; // orientation of tex vector

	void mult(float n);
	void div(float n);
	void set(float n);
	float len();
	gvector flip();
	void rotate(float degx, float degy, float degz);
	void CopyCoords(gvector &Source);
	void Normalize();	
	void AddVec(gvector AVec);
	void gVecRoundN(int n);
	
	gvector () {}
	gvector (vertex p1, vertex p2);
	gvector (float a, float b, float c) { x = a; y = b; z = c; }
	~gvector() {}
};

ostream &operator<<(ostream &ostr, gvector &v);

/* ===== LINEAR EQUATION (SYSTEM) CLASS ===== */

struct equat
{
	float R[4];
	
	void Multi(float M);
	void Subdiv(float M);
	void Set(float X[4]);

	equat operator+(equat L2);
	equat operator-(equat L2);
	
	equat(float X[4]);
	equat();
};

ostream &operator<<(ostream &ostr, equat &E);

struct equatsys2
{
	equat L[2];
	vertex I;
	
	void Fill(vertex V1, vertex V2, gvector Vec1, gvector Vec2);
	vertex Solve();
	
	equatsys2(vertex V1, vertex V2, gvector Vec1, gvector Vec2);
};

/* ===== GVECTOR RELATED FUNCTIONS ===== */

float GetVecAng(gvector Vec1, gvector Vec2);
float GetVecLen(gvector Vector);
float GetDot(gvector v1, gvector v2);
gvector Normalize(gvector Vector);
gvector GetCross(gvector v1, gvector v2);
float GetAngleCos(gvector Vector1, gvector Vector2);
float GetVecAlign(gvector Vector, int mode); // get vector alignment (mode 0 = returns yaw angle; mode 1 = returns pitch angle)
gvector VecAdd (gvector Vec1, gvector Vec2);
gvector GetVector(vertex p1, vertex p2);

float GetAdjaLen (gvector Hypo, gvector Vec);
float GetOppoLen (gvector Hypo, gvector Vec);

float GetAdjaLen (gvector Hypo, float Alpha);
float GetOppoLen (gvector Hypo, float Alpha);

float GetAdjaLen (float HypoLen, float Alpha);

vertex GetLineIsect(vertex V1, vertex V2, gvector Vec1, gvector Vec2);
















#endif

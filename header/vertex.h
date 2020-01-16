#include "gvector.h"

#ifndef __VERTEX_H_INCLUDED__
#define __VERTEX_H_INCLUDED__

#include <string>
#include <vector>

using namespace std;

struct gvector;

/* ===== VERTEX CLASS ===== */

struct vertex
{
	float x = 0.0;
	float y = 0.0;
	float z = 0.0;
	float Yaw = 0;
	float YawB = 0;
	float Pitch = 0;
	int bID = -1;
	float angle = 0.0;
	bool DoSplit = 0;
	string DevTex = "";
	bool IsGap = 0;
	float step = 0;
	int pID = 0; // Path ID this brush will belong to
	int Align = 0;
	bool DoRound = 0;
	bool DoAddHeight = 0;
	bool DoTri = 0;
	bool IsCCW = 0;
	bool IsValid = 1;
	int SecID = 0;
	int SegID = 0;
	bool carved = 0;
	
	/* ===== VERTEX METHODS ===== */
	
	void set(float n);
	void setall(float a, float b, float c);
	void operator=(gvector Vec);
	void Add(vertex V);
	void Add(gvector Vec);
	void rotate(float degx, float degy, float degz);
	void rotateOrigin(float degx, float degy, float degz, vertex orig);
	void push (int i);
	void move(float moveX, float moveY, float moveZ);
	void scale(float n);
	void ScaleOrigin(float n, vertex Origin);
	
	vertex () {}
	vertex (float a, float b, float c);
	~vertex() {}
};

ostream &operator<<(ostream &ostr, vertex &v);

/* ===== VERTEX FUNCTIONS ===== */

float GetFaceLen3 (vertex V0, vertex V1, vertex V2);
vertex GetMVertex(vertex v1, vertex v2);
vertex GetCenTri(vertex v1, vertex v2, vertex v3);
bool CompareVertices(vertex V0, vertex V1);
bool CompareVerticesDeci(vertex V0, vertex V1, int deciplace);
bool CompareVerticesXYDeci(vertex V0, vertex V1, int deciplace);
bool CompareVerticesR(vertex V0, vertex V1);
bool CompareVerticesXY(vertex V0, vertex V1);
vertex Add(vertex V1, vertex V2);
vertex Add(vertex V, gvector Vec);
bool IsVertexInList(vertex &V, vertex *VList, int vcount, bool UsePrecision, int deci);
bool IsVertexInList(vertex &V, vector<vertex> VList, bool UsePrecision, int deci);
bool IsVertexXYInList(vertex &V, vector<vertex> VList, bool UsePrecision, int deci);


/* ===== VERTEX GROUP CLASS ===== */

struct vGroup
{
	int size = 0;
	vertex Vertices[130];
	
	vGroup () {}
	vGroup (int s) { size = s; }
	~vGroup() {}
};


/* ===== VERTEX GROUP FUNCTIONS ===== */

vGroup GetCentroidV(vGroup Origin);




#endif

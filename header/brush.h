#ifndef __BRUSH_H_INCLUDED__
#define __BRUSH_H_INCLUDED__

#include "vertex.h"
#include "face.h"
#include "frames.h"

#include <math.h>
#include <string>

using namespace std;

/* ===== DIMENSIONS CLASS ===== */

struct dimensions
{
	float xs = 0;
	float xb = 0;
	float ys = 0;
	float yb = 0;
	float zs = 0;
	float zb = 0;
	
	void set(float a, float b, float c);
	void expand(int size);
};

dimensions DimensionCombine(dimensions D1, dimensions D2);

ostream &operator<<(ostream &ostr, dimensions &D);

/* ===== BRUSH CLASS ===== */

struct brush
{
	vertex centroid;
	string name 	= "Brush_Name";
	int t_faces 	= 0;
	int t_tri		= 0;
	int SecID 		= -1;
	int SegID 		= -1;
	int SegID2 		= -1;
	int BaseID 		= -1;
	int HeadID 		= -1;
	bool valid 		= 1;
	float vAngle_s 	= 0;
	float vAngle_b 	= 0;
	int t_tgroups 	= 0;
	bool IsDivisible= 0;
	bool IsEnt 		= 0;
	int entID 		= 0;
	bool draw 		= 1;
	bool IsWedge	= 0;
	bool DoSplit	= 0;
	bool IsWedge2	= 0; // this brush is the other half (upper right) of a triangulated Trapezoid
	bool IsGap		= 0;
	float step 		= 0;
	bool HasBack	= 0;
	int pID = 0; // Path ID this brush will belong to
	int oID = 0; // group of orientation this brush is part of
	int Align		= 0;
	bool exported   = 0;
	bool IsSpike	= 0;
	bool RCON 		= 0;
	int gID			= 0;
	int dID 		= 0;
	
	int* vlist 		= nullptr;
	circleset *cset = nullptr;
	face* Faces 	= nullptr;

	void Copy(brush &Source);
	void CopySimple(brush &Source);
	void Scale(float n);
	void ScaleOrigin(float n, vertex Origin);
	void Move(float x, float y, float z, bool fixShifts);
	void Rot(float x, float y, float z);
	void RotOrigin(float x, float y, float z, vertex Origin);
	void MakeCuboid(dimensions Box, string Tex);
	void MakeCube(float size, string Tex);
	bool CheckValidity();
	void GetFaceOrients();
	void CheckDivisibility();
	void CreateGap(int g);
	void ClearVertexList();
	void Triangulate();
	void TriTrapezoid();
	void TriTriangle();
	void TriComplex();
	void RoundVertices();
	void GetFacePlanarity();
	void MarkFaceVertices(face &Candidate, int Mode, bool Overwrite); // Mark all vertices of a brush that match a certain faces vertices; Mode 0 = DoRound; Mode 1 = DoAddHeight
	void CheckNULLFaces();
	void SetRound(bool State);
	void RefreshSpikeTents();
	void CreateTent();
	void Reconstruct();
	
	void GetSimpleCentroid();
	void GetFaceVertexSE();
	void GetVertexListSE();
	void GetVertexList();
	void GetRconVertices();
	void ConvertVerticesC2V();
	void GetFaceCentroids();
	void GetFaceCentroidsC();
	void GetFaceNormals();
	void GetFaceShifts();
	void GetTVecAligns();
	
	brush* Gap = nullptr;
	brush* Tri = nullptr;
	brush() {}
	brush(int tf, int tv);
	~brush();
};

/* ===== BRUSH FUNCTIONS ===== */

brush* Face2Brush(face &SrcFace);
brush* Face2BrushTri(face &SrcFace, int Side);
brush* Face2BrushTriBridge(face &SrcFace, int VID);
brush* Face2BrushTriFan(face &SrcFace, int VID);


#endif

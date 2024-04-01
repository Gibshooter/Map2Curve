#ifndef __BRUSH_H_INCLUDED__
#define __BRUSH_H_INCLUDED__

#include "frames.h"
#include "vertex.h"
#include "face.h"
#include "dimensions.h"

#include <math.h>
#include <string>

using namespace std;

/* ===== BRUSH CLASS ===== */

struct circleset;

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
	bool IsInside	= 0; // determins whether the brushes section is facing to the inside (longest edge is inside) or the outside (longest edge outside) - depends on where the first section is facing
	bool IsCCW		= 0; // not used currently; added it for Face method "ConvertToSheared()" in combination with type 3 (spline extrusion)
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
	int bID			= 0;
	float Yaw		= 0;
	float Pitch		= 0;
	face *HSourceL = nullptr;
	face *HSourceS = nullptr;
	dimensions D;
	vertex Origin;
	bool IsOrigin = 0;
	
	int* vlist 		= nullptr;
	circleset *cset = nullptr;
	face* Faces 	= nullptr;

	void Copy(brush &Source);
	void CopySimple(brush &Source);
	void Scale(float n);
	void ScaleOrigin(float n, vertex Origin, int g);
	void Move(float x, float y, float z, bool fixShifts, int g);
	void Rot(float x, float y, float z);
	void RotOrigin(float x, float y, float z, vertex Origin, int g);
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
	void CheckForHoles(vector<int> &Neighbors);
	void FixHoles();
	bool IsEdgeInBrush(vertex &E1, vertex &E2, int Exlude);
	
	void GetSourceFaces();
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
	void CarveBrush(gvector Plane);
	bool IsOriginBrush();
	void GetBrushDimensions(bool Overwrite);
	void FixBorderliner(int prec);
	void VecToBrush(gvector &Vec, gvector Normal, string Tex);
	
	brush* Gap = nullptr;
	brush* Tri = nullptr;
	brush() {}
	brush(int tf, int tv);
	~brush();
};

/* ===== BRUSH FUNCTIONS ===== */

ostream &operator<<(ostream &ostr, brush &Brush);

brush* Face2Brush(face &SrcFace);
brush* Face2BrushTri(face &SrcFace, int Side);
brush* Face2BrushTriBridge(face &SrcFace, int VID);
brush* Face2BrushTriFan(face &SrcFace, int VID);

void ExportBrushToOBJ(string OutputFile, brush &Brush);
brush* CreateCube(int size);
brush* MakeBoxHollow(dimensions D, float wallsize, string Tex);

#endif

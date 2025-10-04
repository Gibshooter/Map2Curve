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
	bool AllFacesNull= 0; // new v0.87 Update because this would cause issues with GetHorLengths()
	
	int* vlist 		= nullptr;
	circleset *cset = nullptr;
	face* Faces 	= nullptr;
	brush* Gap = nullptr;
	brush* Tri = nullptr;

	vector <vertex>DevVertices;
	
	// +---------------------------+
	// |-------- METHODS ----------|
	// +---------------------------+
	
	// Basic
	void Copy(brush &Source);
	void CopySimple(brush &Source);
	
	// Transformation
	void Scale(float n);
	void ScaleOrigin(float n, vertex Origin, int g);
	void Move(float x, float y, float z, bool fixShifts, int g);
	void Rot(float x, float y, float z);
	void RotOrigin(float x, float y, float z, vertex Origin, int g);
	void MirrorOrigin(int mode, vertex Origin, bool fixShifts, bool getEdges);

	// Modify Vertex Data
	void RoundVertices(int g);
	void FixBorderliner(int prec);
	
	// Generate Brush(es) from existing Brush
	void CarveBrush(gvector Plane, bool rebuild);
	void Triangulate();
	void TriTrapezoid();
	void TriTriangle();
	void TriComplex();
	void CreateTent();
	void RefreshSpikeTents();
	void CreateGap(int g);
	
	// Generate from nothing
	void MakeCuboid(dimensions Box, string Tex);
	void MakeCube(float size, string Tex);
	
	// Check and Modify Brush Data
	bool CheckValidity();
	void CheckDivisibility();
	void CleanUpBrush();
	void CheckNULLFaces(bool markDraw);
	bool IsBrushSloped();
	int FaceMethod(); // 0 = vertices, 1 = planes; check if faces are saved as vertices (Hammer, Jackhammer, etc.) or planes (Trenchbroom with simple brushes e.g. boxes without slopes)
	bool IsThisBrushMadeOfTrianglesEntirely();
	bool IsOriginBrush();
	
	// Get Brush Informations
	void GetBrushDimensions(bool Overwrite);
	void GetSimpleCentroid();
	
	// Get Face Informations
	void GetFaceOrients();
	void GetFacePlanarity();
	void GetSourceFaces();
	void GetFaceNormals();
	void GetFaceCentroids();
	void GetFaceCentroidsC();
	void GetFaceShifts();
	void GetTVecAligns();
	
	// Get Vertex Informations
	void GetBrushVertexList(bool Override = 0);
	void GetFaceVertexSE();
	void GetVertexAngles();
	void GetVertexListSE();
	void GetVertexList(bool Override = 0);
	void GetRconVertices();
	void ConvertVerticesC2V();
	void ClearVertexList();
	
	// Compare to others
	void MarkFaceVertices(face &Candidate, int Mode, bool Overwrite); // Mark all vertices of a brush that match a certain faces vertices; Mode 0 = DoRound; Mode 1 = DoAddHeight
	void SetRound(bool State);
	bool IsEdgeInBrush(vertex &E1, vertex &E2, int Exlude);
	
	// Reconstruct
	//void Reconstruct(); // removed due to v0.87 overhaul 2025
	void Reconstruct2025(bool onlyCurveBrushes = true);
	void IntersectAllFaces(vector<vertex>&newVertices, vector<string>&BlackList);
	void CheckForHoles(vector<int> &Neighbors);
	//void FixHoles(); //MADE UNNECESSARY IN THE v0.87 OVERHAUL UPDATE 2025
	
	// DEV
	void ApplyTempTex();
	void printSimple(bool r = 1);
	void VecToBrush(gvector &Vec, gvector Normal, string Tex);
	
	
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

void ExportBrushToOBJDev(string OutputFile, brush &Brush, bool append);
brush* CreateCube(int size);
brush* MakeBoxHollow(dimensions D, float wallsize, string Tex);

#endif

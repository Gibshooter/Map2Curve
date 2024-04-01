#ifndef __FACE_H_INCLUDED__
#define __FACE_H_INCLUDED__

#include "vertex.h"
#include "gvector.h"
#include <string>
#include <vector>
#include <iostream>

using namespace std;

struct brush;

/* ===== FACE CLASS ===== */

enum axis { X = 10, Y = 20, Z = 30 };

struct face
{
	int vcount = 0;
	int vcountC = 0;
	int fID = 0; // face ID: 0 = Foot; 1 = Head; 2 = Body
	gvector VecX;
	gvector VecY;
	string Texture 	= "NULL";
	float ShiftX 	= 0.0;
	float ShiftY 	= 0.0;
	float Rot 		= 0.0;
	float ScaleX 	= 1.0;
	float ScaleY 	= 1.0;
	vertex * Vertices = nullptr;
	vertex * VerticesC = nullptr; // new calculated Vertices
	bool draw = 1;
	int vAngle_s = 0; // vertex ID that has smallest angle (will be representative for this face)
	int vAngle_sbak = 0;
	int vAngle_b = 0;
	bool FaceAlign = 0; // if 0: World Align, else: Face Align
	int Orient = -1; // 0 = Left, 1 = Right, 2 = Top, 3 = Down, 4 = Front, 5 = Back
	float LengthO = 0;
	float LengthN = 0;
	float LenH = 0;
	float LenV = 0;
	float LenHO = 0;
	float LenVO = 0;
	float PitchO; // original face pitch
	float PitchN; // generated face pitch
	float OffsetX = 0;
	float OffsetY = 0;
	gvector *VecH = &VecX;
	gvector *VecV = &VecY;
	gvector Normal;
	vertex Centroid;
	int BaseX = 0; // Base Vertex for X Tex Vector
	int BaseY = 0; // Base Vertex for Y Tex Vector
	int BaseX2 = 0;
	int BaseY2 = 0;
	float BaseShiftX = 0;
	float BaseShiftY = 0;
	int tID = 0;
	int group = 0; // horizontal shift group
	int ugroup = 0; // unique horizontal shift group
	vector<int> BaseListX {0,0,0,0}; // sorted vertex ID list
	vector<int> BaseListY {0,0,0,0}; // sorted vertex ID list
	float EdgeLenL = 0;
	float EdgeLenS = 0;
	float HShiftL = 0;
	float HShiftS = 0;
	face *HSourceL = nullptr;
	face *HSourceS = nullptr;
	face *LHSourceL = nullptr;
	face *LHSourceS = nullptr;
	bool IsWedgeDown = 0;
	bool HasWorldAlign = 0;
	int EdgeIDs[6] = {0,0,0,0};
	gvector EdgeH;
	gvector EdgeV;
	gvector Hypo;
	bool IsSecBase = 0;
	bool Clockwise = 0;
	bool IsPlanar = 1;
	bool IsNULL = 0;
	int TentID = 0;
	string name="";
	brush *Mother = nullptr;
	
	void GetNormal();
	void CopyFace(face &Source, bool CopyVertices);
	void RevOrder(bool C);
	void AddHeight(float height);
	void pushVerts (int i);
	void RoundVertices();
	void RefreshEdges();
	void GetCentroid();
	void GetCentroidClassic();
	void GetCentroidC();
	void SetEdges(int V0, int V1, int V2, int V3, int V4, int V5);
	bool GetVertexOrder();
	bool GetPlanarity();
	void CreateRamp(int g, int b, int f, int SecID, bool IsWedge2, float Bstep);
	float GetLenHor(bool UseEdge);
	float GetLenVer(bool UseEdge);
	void RefreshTent(face &Base);
	void RotateVertices(float x, float y, float z);
	void MiniShift();
	void ConvertToShearedTri(bool IsWedge2, bool IsInside, bool Reverse, brush &Brush, int g);
	void ConvertToSheared(int g);
	void SortVertices(gvector nVec);
	int IsFaceBeyondPlane(gvector nVec);
	int CarveFace(gvector Plane);
	void AddNewVertex(vertex N);
	void Move(gvector &Vec);
	bool FaceIsValid(int CheckForID, bool CheckIsDraw, bool CheckIsNotNull, bool CheckIsNotNullHintTex);
	void SetFace(int vc, vertex Verts[], string Tex)
	{
		vcount = vc;
		Texture = Tex;
		Vertices = new vertex[vcount];
		for(int v=0; v<vcount; v++)
		{
			vertex &V = Vertices[v];
			V = Verts[v];
		}
		GetNormal();
	}
	
	face () {}
	face (int vc, vertex Verts[], string Tex)
	{
		vcount = vc;
		Texture = Tex;
		Vertices = new vertex[vcount];
		for(int v=0; v<vcount; v++)
		{
			vertex &V = Vertices[v];
			V = Verts[v];
		}
		GetNormal();
	}
	~face() {
		delete[] Vertices;
		delete[] VerticesC;
	}
};

void WriteFaceMAP(ostream &ostr, face &Face);
ostream &operator<<(ostream &ostr, face &Face);

/* ===== FACE FUNCTIONS ===== */

void AlignToWorld(face &Face);
axis GetWorldAlign(face &Face);
bool IsFaceAlignValid(face &Face);
float GetFaceLen(face &Face);
void GetBaseEdges(face &Face);
void GetBaseEdgesC(face &Face); // only for original imported Brush Faces
void GetTexOffset(face &Face, int mode);
void GetBaseShift(face &Face, int axis, bool longEdge, bool C);
void CheckFaceAlign(face &Face);
bool IsFaceParallel(face &F1, face &F2);
bool DoFacesShareVerts(face &F1, face &F2, int deciplaces);
bool IsVertexOnFace(face &Face, vertex &V, int deci);
double GetDistFaceVertex(face &Face, vertex &V);
bool IsVertexOnPlane(gvector &Normal, vertex &V, int deci);
double GetDistPlaneVertex(gvector &Normal, vertex &V);
bool DoFacesShareVertices(face &F1, face &F2);

void CoutBrushFacesDevInfo(brush &Brush, int b, int g, bool IsInRange);
void CoutFacesDevInfo(brush &Brush, face &Face, int f);







#endif




















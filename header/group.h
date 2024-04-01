#ifndef __GROUP_H_INCLUDED__
#define __GROUP_H_INCLUDED__

#include "brush.h"
#include "settings.h"
#include "entity.h"

#include <string>
#include <vector>

using namespace std;

struct vertex;
struct face;
struct brush;
struct entity;
struct dimensions;


/* ===== GROUP CLASS ===== */

struct group {
	int gID 		= 0;
	bool IsSrcMap	= 0;
	float SizeY 	= 0;
	float SizeZ 	= 0;
	int t_brushes 	= 0;
	int t_faces 	= 0;
	int t_arcs 		= 0;
	int t_ents		= 0;
	float biggestY 	= 0.0;
	float smallestY = 0.0;
	float biggestZ 	= 0.0;
	float smallestZ = 0.0;
	int sections 	= 0;
	int segments 	= 0;
	int invalids 	= 0;
	int range_start = 0;
	int range_end	= 0;
	vector<float> hEdgeLen; // edge-lengths for the horizontal texture shift calculation
	vector<float> hEdgeLen_temp; // edge-lengths for the horizontal texture shift calculation (LONGEST)
	vector<float> hEdgeLen_temp2; // edge-lengths for the horizontal texture shift calculation (SHORTEST)
	int hGroupsCount = 0; // total horizontal texture groups
	vector<face*> hSourceFace;
	vector<face*> hSourceFace2;
	vector<gvector> fGroupVecL; // DEV list of longest/shortest source vectors for each face group
	vector<gvector> fGroupVecS; // DEV
	vector<gvector> devFaceEdges; // DEV
	vector<float> heightTable;
	vector<float> heightTableSteps;
	vector<gvector> SecIsect; // intersections for detail groups if d_carve is active
	vector<bool> GapList;
	face **SecBaseFace = nullptr;
	dimensions Dimensions; // bounding box
	brush *Brushes = nullptr;
	entity *Entities = nullptr;
	brush *boundBox = nullptr;
	vertex Origin;
	bool valid = 1;
	bool RCON = 0;
	bool ValidMesh = 1;
	bool ValidSpline = 1;
	string groupname = "";
	vector<bool> IsSecInside; // list of inside sections; not used ATM!; meant to be used for proper texture shearing
	bool HasOrigin = 0;
	
	float d_pos = -1;
	int d_autopitch = -1;
	int d_autoyaw = -1;
	int d_separate = -1;
	int d_autoname = -1;
	int d_enable = -1;
	tform d_pos_rand;
	tform d_rotz_rand;
	tform d_movey_rand;
	tform d_scale_rand;
	int d_draw = -1;
	int d_draw_rand = -1;
	int d_skip = -1;
	int d_carve = -1;
	int d_circlemode = -1;
	
	vector <brush*>DevAssets; // DEV a bunch of Brushes for developer purposes
	
	void MarkGroupOriginObjects();
	void GetGroupDimensions(bool Overwrite, bool CustomOrigin);
	void Move(float x, float y, float z, bool LockBrushShifts);
	void MoveSecs(vector<gvector> &Move, bool LockBrushShifts);
	void ScaleOriginSecs(vector<float> &Scale, vector<vertex> &Origin);
	void RotOriginSecs(vector<float> &RotX, vector<float> &RotY, vector<float> &RotZ, vector<vertex> &Origin, bool LockBrushShifts);
	void Copy(group &Source);
	void CopyProps(group &Source);
	void FillUnsetKeySettings();
	
	void GetGroupOrigin();
	void CheckBrushValidity();
	void GetBrushFaceOrients();
	void CheckBrushDivisibility();
	void CreateBrushGaps();
	void ArrangeGaps();
	void RoundBrushVertices(bool Override);
	void Build();
	void GetBrushBodyFaceLengths();
	void GetHorLengths(); // Hor Texture Lengths/Shift Calculation
	void RotateVectors();
	void ShearVectors();
	void GetGroupVertexList();
	void ClearBrushVertexList();
	void ExportGroupToMap(string p);
	void GetBrushFacePlanarity();
	void Triangulate();
	void GetFaceGroups();
	void GetHeadVertices();
	void GetTransitVertices();
	void CheckNULLBrushes();
	bool CheckForSlopes();
	void CreateIsects();
	void CarveGroupSections();
	void WeldGroupVertices(bool WeldGaps);
	
	void AddBrushHeights();
	void CreateHeightTable();
	void CreateGapList();
	
	void Reconstruct();
	void ReconstructMap();
	void GetBrushSimpleCentroid();
	void GetBrushShifts();
	void GetRconBrushShifts();
	void GetBrushVertexAngles();
	void GetBrushFaceVertexSE();
	void GetBrushVertexListSE();
	void GetBrushVertexList();
	void GetRconBrushVertices();
	void ConvertRconBrushVertices();
	void GetBrushFaceCentroids();
	void GetBrushFaceCentroidsC();
	void GetBrushTVecAligns();
	void MarkInsideSecBrushes();
	void GetGroupOriginCustom();
	bool IsSecInRange(int secID);
	
	void GroupTexturize();
	void GroupTexturizeHStretch();
	void AddCustomShiftOffset();
	
	group() {}
	~group()
	{
		delete[] Brushes;
		delete[] boundBox;
		delete[] Entities;
		for(int i=0;i<DevAssets.size();i++)
			delete DevAssets[i];
		if(SecBaseFace!=nullptr)
		delete[] SecBaseFace;
	}
};

ostream &operator<<(ostream &ostr, group &g);

/* ===== GROUP SET CLASS ===== */

struct group_set
{
	group *Groups = nullptr;
	int t_groups = 0;
	dimensions Dimensions;
	
	void GetGroupSetDimensions(bool Overwrite);
	group_set() {}
	~group_set() { delete[] Groups; }
};



#endif

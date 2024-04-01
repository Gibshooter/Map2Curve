#ifndef __FRAMES_H_INCLUDED__
#define __FRAMES_H_INCLUDED__

#include "vertex.h"
#include "brush.h"
#include "dimensions.h"

#include <string>
#include <vector>
#include <math.h>

#define PI 3.14159265

using namespace std;

struct tform;
struct dimensions;

/* ===== PATH CLASSES & FUNCTIONS ===== */

struct path_corner
{
	string name = "";
	string target = "";
	vertex pos; // origin
	int pID = 0;
	float rot1 = 0;
	float rot2 = 0;
	float length = 0;
	float Yaw = 0;
	float Pitch = 0;
	int Align = 0; // 0 = UP, 1 = RIGHT, 2 = DOWN, 3 = LEFT
	bool NextIsGap90 = 0;
	bool NextIsGap180 = 0;
	bool NextIsCW = 0;
	float step = 0;
};

struct path
{
	path_corner *Corners = nullptr;
	int t_corners = 0;
	float t_length = 0;
	bool direct = 1; // direction 1 = forwards, 0 = backwards
	bool valid = 1;
	
	void Copy(path &Source);
	void reverse();
	void ScaleOrigin(tform n, vertex Origin);
	void Move(float x, float y, float z);
	void Expand(float n);
	void EvenOut();
	
	//~path () { if(Corners!=nullptr) delete[] Corners; }
};

ostream &operator<<(ostream &ostr, path &p);

struct path_set
{
	int gID = 0;
	path *Paths = nullptr;
	int t_paths = 0;
	int t_corners = 0;
	bool valid = 1;
	int Gaps = 0;
	bool cornerFix = 0;
	bool preverse = 0;
	int type = 0;
	float t_length = 0;
	vertex Origin;
	dimensions D;
	
	void GetSplineSetDimensions();
	void Scale(tform n);
	void Analyze();
	int CountSections();
	void PathToDevAssets();
	path_corner* GetCornerSecN(int i) // return corner that corresponds to a spline section index. a path of 5 verts only corresponds to 4 sections. last one is redundant
	{
		for (int p=0, tc=0; p<t_paths; p++)
			for (int c=0; c<Paths[p].t_corners-1; c++) {
				path_corner *CornerPtr = &Paths[p].Corners[c];
				if(tc==i||tc==t_corners) return CornerPtr;
				tc++; // total corners passed so far
			}
	}
	
	path_set () {}
	~path_set () { if(Paths!=nullptr) delete[] Paths; }
};

ostream &operator<<(ostream &ostr, path_set &ps);


/* ===== SPLINE FILE RELATED FUNCTIONS ===== */

void ParseCornerFile(string pFile, path_set &Path);




/* ===== CIRCLE CLASSES & FUNCTIONS ===== */

struct circle
{
	vertex *Vertices = nullptr;
	int tverts = 0;
	int SrcFace = 0;
	vector<gvector> InVec;

	void build_circlePi(int res, float rad, float height, bool flat);
	void build_circleGrid(int res, float rad, float height);
	void build_pathGrid(int g, float posy, float height, path_set &Set);
	void build_pathIntersect(int g, float posy, float height, path_set &Set);
	void reverse(int res);
	void AddHeight(int g, vector<path_set> &Set);
	void ConvertToSpline(int g);
	void GetAngles(int g);
	void GetInVec(int g);

	circle() {}
	~circle() { if(Vertices!=nullptr) delete[] Vertices; }
};

ostream &operator<<(ostream &ostr, circle &c);

struct circleset
{
	circle *c = nullptr;
	int tcircs = 0;
	
	circleset() {}
	~circleset() { if(c!=nullptr) delete[] c; }
};

ostream &operator<<(ostream &ostr, circleset &cs);

struct oldcircle
{
	float coords[2][768];
};


#endif

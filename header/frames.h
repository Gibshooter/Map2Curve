#ifndef __CIRCLE_H_INCLUDED__
#define __CIRCLE_H_INCLUDED__

#include "vertex.h"

#include <string>
#include <vector>
#include <math.h>

#define PI 3.14159265

using namespace std;

/* ===== PATH CLASSES & FUNCTIONS ===== */

struct path_corner
{
	string name = "";
	string target = "";
	vertex pos; // origin
	int pID = 0;
	float rot1 = 0;
	float rot2 = 0;
	float Yaw = 0;
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
	bool direct = 1; // direction 1 = forwards, 0 = backwards
	
	void reverse();
	
	~path () {delete[] Corners;}
};

struct path_set
{
	path *Paths = nullptr;
	int t_paths = 0;
	int t_corners = 0;
	//int sections = 0; // res
	bool valid = 1;
	int Gaps = 0;
	bool cornerFix = 0;
	bool preverse = 0;
	int type = 0;
	vector<float> heightTable;
	
	path_set () {}
	~path_set () {delete[] Paths;}
};





/* ===== CIRCLE CLASSES & FUNCTIONS ===== */

struct circle
{
	vertex *Vertices = nullptr;
	int tverts = 0;
	int SrcFace = 0;

	void build_circlePi(int res, float rad, float height);
	void build_circleGrid(int res, float rad, float height);
	void build_pathGrid(int g, float posy, float height, path_set &Set);
	void reverse(int res);

	circle() {}
	~circle() {delete[] Vertices;}
};

ostream &operator<<(ostream &ostr, circle &c);

struct circleset
{
	circle *c = nullptr;
	int tcircs = 0;
	
	circleset() {}
	~circleset() {delete[] c;}
};

struct oldcircle
{
	float coords[2][768];
};


#endif

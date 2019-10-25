#ifndef __ENTITY_H_INCLUDED__
#define __ENTITY_H_INCLUDED__

#include "brush.h"
#include "matrix.h"
#include "settings.h"

#include <string>
#include <vector>

using namespace std;

struct vertex;
struct Euler;
struct Matrix;

/* ===== ENTITY & KEY CLASS ===== */

struct key
{
	string name;
	string value;
	int type = 0;
};

struct entity
{
	int eID = 0;
	int dID = 0; // detail group ID
	int cID = 0;
	bool draw = 1;
	int SecID = 0;
	int type = 0; // 0 = world, 1 = solid, 2 = point
	int pos_start = 0;
	int pos_end = 0;
	int head_end = 0;
	string content = "";
	int t_brushes = 0; // number of brushes that this solid entity contains
	int t_faces = 0;
	string groupname = ""; // group identification name for detail objects
	vertex Origin;
	bool IsDetail = 0;
	brush *Brushes = nullptr;
	vector<key> Keys;
	Euler Angles; // Pitch Yaw Roll
	Matrix AMatrix; // Angle Matrix
	string key_classname = "";
	string key_target = "";
	string key_targetname = "";
	
	// Detail Group Properties
	int d_enable 		= -2;
	int d_autopitch 	= -2;
	int d_autoyaw 		= -2;
	int d_separate 		= -2;
	int d_autoname 		= -2;
	float d_pos 		= -2;
	tform d_pos_rand;
	tform d_rotz_rand;
	tform d_movey_rand;
	int d_draw = -2;
	int d_draw_rand = -2;
	int d_skip = -2;
	
	void RotateEntity(Euler RotAngles, bool UpdateEuler);
	void CreateBrushes();
	void CopySimple(entity &Source);
	void RotateOrigin(float x, float y, float z, vertex Origin);
	void GetKeyValues();
	void GetIntMapSettings(vector<string> &MapSettings);
	void ScaleOrigin(float n, vertex SOrigin);
	entity() {}
	~entity() { delete[] Brushes; }
};




#endif

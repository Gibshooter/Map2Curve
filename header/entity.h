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

enum keytype
{
	//KT_TARGETNAME;
	//KT_TARGET;
	//KT_ORIGIN;
	KT_ANGLES,
	KT_SCALE
};

struct key
{
	string name;
	string value;
	int type = 0;
	
	key(string str_name, string str_value) { name = str_name; value = str_value; }
	key() {}
	~key() {}
};

void ReplaceKeyInList(vector<key> &List, string candidate, string replace);

struct entity
{
	int eID = 0;
	int dID = 0; // detail group ID
	int cID = 0;
	bool draw = 1;
	int SecID = 0;
	int type = 0; // 0 = world, 1 = solid ent, 2 = point ent
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
	vector<key> Keys_Original;
	Euler Angles; // Pitch Yaw Roll
	Matrix AMatrix; // Angle Matrix
	string key_classname = "";
	string key_target = "";
	string key_targetname = "";
	int SpawnFlags = 0;
	float key_scale = -1;
	bool IsOrigin = 0;
	vector<keytype> KeyTypes;
	
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
	tform d_scale_rand;
	int d_draw 			= -2;
	int d_draw_rand 	= -2;
	int d_skip 			= -2;
	int d_carve 		= -2;
	int d_circlemode 	= -2;
	
	bool IsOriginEntity();
	void RotateEntity(Euler RotAngles, bool UpdateEuler);
	void CreateBrushes();
	void CopySimple(entity &Source);
	void RotateOrigin(float x, float y, float z, vertex Origin);
	void GetKeyValues();
	void GetKeyValues_M2C();
	void GetIntMapSettings(vector<string> &MapSettings);
	void ScaleOrigin(float n, vertex SOrigin);
	bool CarveEntity(gvector Plane);
	bool IsKeyType(keytype KT);
	
	entity() {}
	~entity() { delete[] Brushes; }
};

ostream &operator<<(ostream &ostr, entity &Entity);



#endif

#ifndef __FILE_H_INCLUDED__
#define __FILE_H_INCLUDED__

#include "entity.h"
#include "frames.h"

#include <string>
#include <vector>

using namespace std;

struct brush;

/* ===== FILE CLASS ===== */

struct file {
	int fID = 0;
	string fullpath ="";
	string name = "";
	string p_path = "";
	string path_cfg = "";
	string path_map = "";
	string str_cfg = "";
	string str_map = "";
	string str_path = "";
	bool valid_cfg = 1;
	bool valid_map = 1;
	bool InternalMapSettings = 0;
	int t_solids = 0;
	int t_dgroups = 0;
	int t_iarcs = 0; // total curve object count from internal map settings entities
	int type = 0; // type 1 = txt, type 2 = map
	vector<entity> EntityList;
	vector<string> settings; // settings from config file
	vector<string> settingsM; // settings from map-internal settings entity info_curve
	vector<int> settingsM_ID; // arc ID list for map-internal settings
	vector<path_set> PathList;
	int append = 0;
	string target = "UNSET";
	int d_autoassign = 0;
	
	vector<string> tTable_name; // texture information table
	vector<int> tTable_width;
	vector<int> tTable_height;
	
	// load and process map data
	void GetInfo();
	void CheckForCustomSource();
	void LoadMap();
	void LoadMap_DetailObj();
	void LoadMap_GetEntities();
	void LoadMap_GetTexInfo();
	void LoadMap_GetTexInfoScanBrush(brush &Brush);
	void LoadMap_ConvertWorld2Face();
	void LoadSpline(int g);
	void GetInternalMapSettings();
	
	// create curve data objects
	void createGroupMap();
	void createGroupSource();
	void createGroupBrush(int g);
	void createDetailGroupSource();
	void createDetailGroup(int g);
	void createBounds(int g);
	
	// process detail objects
	void FixDetailPos();
	
	// generate and process curve data
	void TransformSource();
	void createFramework(int g);
	void buildArcs(int g);
	void TransformDetailObj(int g);
	void texturize(int g);
	void Triangulate(int g);
	void RampIt(int g);
	void roundCoords(int g);
	void WeldVertices(int g);
	void TransformFinal(int g);
	void FixBorderliner(int g);
	
	// export final curve data
	void ExportToMap();
	void ExportToMapO(string p);
	void ExportToRMF();
	void ExportToObj();
	string GetMapEnts(string mapfile); // extracts the entity part of a map file string
	string GetMapWorld(string mapfile); // extracts the worldspawn part of a map file string
	
	file (string p)	{
		fullpath = p;
	}
	~file() {}
};

















#endif

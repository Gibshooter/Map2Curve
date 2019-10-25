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
	
	vector<string> tTable_name; // texture information table
	vector<int> tTable_width;
	vector<int> tTable_height;
	
	void ExportToMap();
	void ExportToMapO(string p);
	void GetInfo();
	void ExportToObj();
	void texturize(int g);
	void buildArcs(int g);
	void createGroupBrush(int g);
	void Triangulate(int g);
	void createFramework(int g);
	void createTableC();
	void LoadMap();
	void LoadMap_DetailObj();
	void LoadMap_GetEntities();
	void createGroupMap();
	void LoadMap_GetTexInfo();
	void LoadMap_GetTexInfoScanBrush(brush &Brush);
	void LoadMap_ConvertWorld2Face();
	void RampIt(int g);
	void roundCoords(int g);
	void LoadPaths(int g);
	void analyzePath(path_set &List);
	void CheckForCustomSource();
	string GetMapEnts(string mapfile);
	string GetMapWorld(string mapfile);
	void TransformSource();
	void TransformFinal(int g);
	void createBounds(int g);
	void createGroupSource();
	void createDetailGroup();
	void RotateDetailObj(int g);
	void GetInternalMapSettings();
	void FixDetailPos();

	file (string p)	{
		fullpath = p;
	}
	~file() {}
};



/* ===== FILE RELATED FUNCTIONS ===== */

void interpretPathFile(string pFile, path_set &Path);















#endif

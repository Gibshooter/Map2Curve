#ifndef __SETTINGS_H_INCLUDED__
#define __SETTINGS_H_INCLUDED__

#include "utils.h"
#include <string>
#include <vector>
#include <iostream>

using namespace std;



/* ===== GENERAL FUNCTIONS ===== */

void LoadWads();
void LoadDefaultSettings();

string GetValue(string text, int start_pos);
bool CheckPhrase(string text, int pos);
string GetCustomPhraseValue(string text, string phrase);
void GetSettings(string cfgstr, vector<string> &SettingList, vector<string> &lslist);

/* ===== TRANSFORM CLASS ===== */

struct tform {
	float x = 0;
	float y = 0;
	float z = 0;
	bool IsSet = 0;
	
	void set(string input);
};

ostream &operator<<(ostream &ostr, tform &t);


/* ===== CONSTRUCTION TABLE CLASS ===== */

struct ctable {
	float rad 		= 0;
	float offset_NO = 0; // difference between new and original radius, used for detail objects
	float offset 	= 0;
	int res 		= -1;
	int obj 		= -1;
	int map 		= -1;
	int type 		= -1; // 0 = PI, 1 = QUAKE
	int shift 		= -1; // hor shift mode; 0 = per section, 1 = per Brush, 2= per Texture
	int tri 		= -1;
	int round 		= -1;
	float height 	= -1;
	int ramp 		= -1;
	int cornerfix 	= -1;
	int preverse	= -1; // path reverse direction (forwards<>backwards)
	int ramptex   	= -1;
	int psplit   	= -1;
	tform p_scale;
	float p_expand 	= 0;
	int p_evenout	= -1;
	string target 	= "UNSET";
	string path 	= "UNSET";
	int append		= -1;
	int bound 		= -1;
	float range_start = -1;
	float range_end = -1;
	int transit_tri 	= -1;
	int transit_round 	= -1;
	int gaps		= -1;
	float gaplen	= -1;
	int skipnull	= -1;
	tform scale;
	tform scale_src;
	tform rot;
	tform rot_src;
	tform move;
	int c_enable 	= -1;
	int d_enable 	= -1;
	int d_autoyaw	= -1;
	int d_autopitch	= -1;
	float d_pos	= -1;
	int d_separate = -1;
	int d_autoname = -1;
	tform d_pos_rand;
	tform d_rotz_rand;
	tform d_movey_rand;
	int d_draw = -1;
	int d_draw_rand = -1;
	int d_skip = -1;
	string nulltex	= "UNSET";
	float spike_height = -1;
	int heightmode = -1; // 0 = Linear, 1 = Smooth, 2 = Spline, 3 = Random Jagged, 4 = Random Smooth
	int texmode = -1;
	
	void Print();
	void FillUnset(ctable &Filler);
	void FillDefaults();
	
	ctable () {}
	~ctable() {}
};

/* ===== CONSTRUCTION TABLE FUNCTIONS ===== */

ctable* createTableS(int CurveCount, vector<string> &SettingsList, bool IDOffset);
void createTableC();


/* ===== SETTING CLASS ===== */

/*struct sdata
{
	int type = 0; // 0 = bool, 1 = int, 2 = float, 3 = string, 4 = tform
	bool 	val_bool 	= 0;
	int 	val_int 	= 0;
	float 	val_float 	= 0;
	string 	val_string 	= "";
	tform	val_tform;
};*/

struct setting
{
	string name = "";
	int ID = -1;
	int type = 0; // 0 = bool, 1 = int, 2 = float, 3 = string
	bool IsSet = 0;
	
	bool 	val_bool 	= 0;
	int 	val_int 	= 0;
	float 	val_float 	= 0;
	string 	val_string 	= "";
	tform 	val_tform;
	
	int MIN = 0;
	int MAX = 0;
	
	void SetThis(string val);
	void Print();
	setting(string s_name, int s_ID, int s_type, int s_min, int s_max);
};

struct setting_list
{
	vector<setting> Settings;
	
	void SetEntry(string s_name, string s_val, bool &FoundAndSet)
	{
		bool dev = 0;
		if (dev)cout << " Set Values with Name " << s_name << " and Val "<<s_val<<" ..." << endl;
		for (int i=0; i<Settings.size(); i++)
		{
			setting &S = Settings[i];
			if (s_name==S.name&&!S.IsSet)
			{
				if (dev)cout << " Hit at Entry #" << i << " type "<<S.type<<" name " << S.name << " Is.Set " << S.IsSet<< endl;
				S.SetThis(s_val);
				FoundAndSet = 1;
			}
		}
	}
	void CreateList(vector<string> &list_names, vector<int> &list_id, vector<int> &list_types, vector<int> &list_min, vector<int> &list_max)
	{
		for(int i=0; i<list_names.size(); i++)
		{
			setting NewSetting(list_names[i], list_id[i], list_types[i], list_min[i], list_max[i]);
			
			Settings.push_back(NewSetting);
		}
	}
	
	void ConvertToTable(ctable &Table)
	{
		// bools
		if (Settings[4].IsSet)Table.obj 			= Settings[4].val_bool;
		if (Settings[48].IsSet)Table.map 			= Settings[48].val_bool;
		if (Settings[8].IsSet)Table.tri 			= Settings[8].val_bool;
		if (Settings[9].IsSet)Table.round 			= Settings[9].val_bool;
		if (Settings[12].IsSet)Table.cornerfix		= Settings[12].val_bool;
		if (Settings[13].IsSet)Table.preverse		= Settings[13].val_bool;
		if (Settings[14].IsSet)Table.ramptex  		= Settings[14].val_bool;
		if (Settings[15].IsSet)Table.psplit  		= Settings[15].val_bool;
		if (Settings[22].IsSet)Table.bound 			= Settings[22].val_bool;
		if (Settings[25].IsSet)Table.transit_tri	= Settings[25].val_bool;
		if (Settings[26].IsSet)Table.transit_round	= Settings[26].val_bool;
		if (Settings[27].IsSet)Table.gaps			= Settings[27].val_bool;
		if (Settings[29].IsSet)Table.skipnull		= Settings[29].val_bool;
		if (Settings[30].IsSet)Table.d_enable 		= Settings[30].val_bool;
		if (Settings[31].IsSet)Table.d_autoyaw		= Settings[31].val_bool;
		if (Settings[32].IsSet)Table.d_autopitch	= Settings[32].val_bool;
		if (Settings[37].IsSet)Table.d_autoname 	= Settings[37].val_bool;
		if (Settings[36].IsSet)Table.d_separate 	= Settings[36].val_bool;
		if (Settings[7].IsSet)Table.append 			= Settings[7].val_bool; // once!
		if (Settings[42].IsSet)Table.d_draw_rand	= Settings[42].val_bool;
		if (Settings[11].IsSet)Table.ramp 			= Settings[11].val_bool;
		if (Settings[47].IsSet)Table.p_evenout 		= Settings[47].val_bool;
		if (Settings[49].IsSet)Table.c_enable 		= Settings[49].val_bool;
				
		// integers
		if (Settings[2].IsSet)Table.res 			= Settings[2].val_int;
		if (Settings[3].IsSet)Table.type 			= Settings[3].val_int;
		if (Settings[5].IsSet)Table.shift 			= Settings[5].val_int;
		if (Settings[41].IsSet)Table.d_draw			= Settings[41].val_int;
		if (Settings[43].IsSet)Table.d_skip			= Settings[43].val_int;
		if (Settings[44].IsSet)Table.heightmode		= Settings[44].val_int;
		if (Settings[50].IsSet)Table.texmode		= Settings[50].val_int;
		
		// floats
		if (Settings[0].IsSet)Table.rad 			= Settings[0].val_float;
		if (Settings[1].IsSet)Table.offset 			= Settings[1].val_float;
		if (Settings[10].IsSet)Table.height 		= Settings[10].val_float;
		if (Settings[23].IsSet)Table.range_start	= Settings[23].val_float;
		if (Settings[24].IsSet)Table.range_end 		= Settings[24].val_float;
		if (Settings[28].IsSet)Table.gaplen			= Settings[28].val_float;
		if (Settings[33].IsSet)Table.d_pos			= Settings[33].val_float;
		if (Settings[35].IsSet)Table.spike_height 	= Settings[35].val_float;
		if (Settings[46].IsSet)Table.p_expand 		= Settings[46].val_float;
		
		// strings
		if (Settings[34].IsSet)Table.nulltex		= Settings[34].val_string;
		if (Settings[6].IsSet)Table.target 			= Settings[6].val_string; // once!
		if (Settings[16].IsSet)Table.path 			= Settings[16].val_string; // once!
		
		// tform scale 17
		if (Settings[17].IsSet)Table.scale			= Settings[17].val_tform;
		if (Settings[18].IsSet)Table.rot			= Settings[18].val_tform;
		if (Settings[19].IsSet)Table.move			= Settings[19].val_tform;
		if (Settings[20].IsSet)Table.scale_src		= Settings[20].val_tform;
		if (Settings[21].IsSet)Table.rot_src		= Settings[21].val_tform;
		if (Settings[38].IsSet)Table.d_pos_rand		= Settings[38].val_tform;
		if (Settings[39].IsSet)Table.d_rotz_rand	= Settings[39].val_tform;
		if (Settings[40].IsSet)Table.d_movey_rand	= Settings[40].val_tform;
		if (Settings[45].IsSet)Table.p_scale		= Settings[45].val_tform;
	}
	
	void Print()
	{
		for(int i=0; i<Settings.size(); i++)
			Settings[i].Print();
	}
	
	setting_list() {};
	setting_list(vector<string> &list_names, vector<int> &list_id, vector<int> &list_types, vector<float> &list_min, vector<float> &list_max)
	{
		for(int i=0; i<list_names.size(); i++)
		{
			setting NewSetting(list_names[i], list_id[i], list_types[i], list_min[i], list_max[i]);
			
			Settings.push_back(NewSetting);
		}
	}
	~setting_list() {};
};











#endif

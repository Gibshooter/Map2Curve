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
	bool obj 		= 0;
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
	
	void Print();
	void FillUnset(ctable &Filler);
	void FillDefaults();
	
	ctable () {}
	~ctable() {}
};

/* ===== CONSTRUCTION TABLE FUNCTIONS ===== */

ctable* createTableS(int CurveCount, vector<string> &SettingsList, bool IDOffset);


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
	
	void SetThis(string val)
	{
		bool dev = 0;
		string ValidCharsBool = "01";
		string ValidCharsInt ="-+01234567890";
		string ValidCharsFloat = ".-+01234567890";
		string ValidCharsTform = " .-+01234567890";
		
		if (dev)cout << " Trying to  set Setting " << name <<"(type "<<type<<", ID"<<ID<<") to " << val << "..." <<endl;
		if (type==0) { // bool
			if (!ContainsInvalids(val, ValidCharsBool)) {
				IsSet = 1;
				val_bool = stoi(val);
				if (dev)cout << "   Value is of type bool! val now " << val_bool << endl << endl;
			}
		}
		else if (type==1) { // int
			if (!ContainsInvalids(val, ValidCharsInt)) {
				IsSet = 1;
				val_int = stoi(val);
				if (val_int<MIN) val_int = 0;
				else if (val_int>MAX) val_int = 1;
				if (dev)cout << "   Value is of type int! val now " << val_int << endl << endl;
			}
		}
		else if (type==2) { // float
			if (!ContainsInvalids(val, ValidCharsFloat)) {
				IsSet = 1;
				val_float = stof(val);
				if (val_float<MIN) val_float = MIN;
				else if (val_float>MAX) val_float = MAX;
				if (dev)cout << "   Value is of type float! val now " << val_float << endl << endl;
			}
		}
		else if (type==3) { // string
			if (val!="UNSET"&&val!="ERR"&&val.length()>=MIN&&val.length()<=MAX) {
				IsSet = 1;
				val_string = val;
				if (dev)cout << "   Value is of type string! val now " << val_string << endl << endl;
			}
		}
		else if (type==4) { // tform
			if (!ContainsInvalids(val, ValidCharsTform)) {
				IsSet = 1;
				val_tform.set(val);
				if (dev)cout << "   Value is of type tform! val now " << val_tform << endl << endl;
			}
		}
		else { if (dev)cout << "   Didnt set anything!" << endl << endl; }
		
		if(dev)Print();
	}
	
	void Print() { cout << " Setting: " << name << " \tID " << ID << " \ttype " << type << " \tmin " << MIN << " \tmax " << MAX << endl;}
	
	setting(string s_name, int s_ID, int s_type, int s_min, int s_max)
	{
		name = s_name;
		ID = s_ID;
		type = s_type;
		MIN = s_min;
		MAX = s_max;
	}
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
		
		// integers
		if (Settings[2].IsSet)Table.res 			= Settings[2].val_int;
		if (Settings[3].IsSet)Table.type 			= Settings[3].val_int;
		if (Settings[5].IsSet)Table.shift 			= Settings[5].val_int;
		if (Settings[11].IsSet)Table.ramp 			= Settings[11].val_int;
		if (Settings[41].IsSet)Table.d_draw			= Settings[41].val_int;
		if (Settings[43].IsSet)Table.d_skip			= Settings[43].val_int;
		
		// floats
		if (Settings[0].IsSet)Table.rad 			= Settings[0].val_float;
		if (Settings[1].IsSet)Table.offset 			= Settings[1].val_float;
		if (Settings[10].IsSet)Table.height 		= Settings[10].val_float;
		if (Settings[23].IsSet)Table.range_start	= Settings[23].val_float;
		if (Settings[24].IsSet)Table.range_end 		= Settings[24].val_float;
		if (Settings[28].IsSet)Table.gaplen			= Settings[28].val_float;
		if (Settings[33].IsSet)Table.d_pos			= Settings[33].val_float;
		if (Settings[35].IsSet)Table.spike_height 	= Settings[35].val_float;
		
		// strings
		if (Settings[34].IsSet)Table.nulltex		= Settings[34].val_string;
		if (Settings[6].IsSet)Table.target 			= Settings[6].val_string; // once!
		
		// tform scale 17
		if (Settings[17].IsSet)Table.scale			= Settings[17].val_tform;
		if (Settings[18].IsSet)Table.rot			= Settings[18].val_tform;
		if (Settings[19].IsSet)Table.move			= Settings[19].val_tform;
		if (Settings[20].IsSet)Table.scale_src		= Settings[20].val_tform;
		if (Settings[21].IsSet)Table.rot_src		= Settings[21].val_tform;
		if (Settings[38].IsSet)Table.d_pos_rand		= Settings[38].val_tform;
		if (Settings[39].IsSet)Table.d_rotz_rand	= Settings[39].val_tform;
		if (Settings[40].IsSet)Table.d_movey_rand	= Settings[40].val_tform;
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

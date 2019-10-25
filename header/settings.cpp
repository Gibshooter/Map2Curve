#include "settings.h"
#include "file.h"
#include "utils.h"
#include "WAD3.h"

#include <iostream>
#include <vector>
#include <string>

using namespace std;

extern file *gFile;
extern vector<string> slist;
extern vector<int> slist_id;
extern vector<int> slist_type;
extern vector<int> slist_min;
extern vector<int> slist_max;
extern ctable* dTable;
extern bool G_DEV;
extern bool ValidDefaults;
extern string ROOT;
extern vector<WADFile> WADFiles;

/* ===== GENERAL FUNCTIONS ===== */

void LoadDefaultSettings()
{
	string DefaultsTXT = LoadTextFile("DEFAULTS.txt");
	vector<string> DefaultsList;
	
	ctable iTable; // Internal Defaults Table
	iTable.FillDefaults();
	
	if (DefaultsTXT!="ERR")
	{
		GetSettings(DefaultsTXT, DefaultsList, slist);
		
		dTable = createTableS(1, DefaultsList,0);
		dTable->FillUnset(iTable);
		
		if (G_DEV)
		dTable[0].Print();
	}
	else
	{
		// load internal fallback defaults entirely
		dTable = new ctable[1];
		dTable[0] = iTable;
		ValidDefaults = 0;
	}
}

void LoadWads()
{
	bool dev = 0;
	vector<string> WadFilePaths;
	string PATH_WAD_DIR = ROOT+"WAD\\";
	
	// get File List of Map2Curves WAD Directory and scan for *.wad files
	vector<string> WadDir;
	GetFileList(PATH_WAD_DIR, WadDir);
	for (int i = 0; i<WadDir.size(); i++)
	{
		string &cEntry = WadDir[i];
		if (cEntry.find(".wad",0)==cEntry.length()-4)
		{
			WadFilePaths.push_back(PATH_WAD_DIR+cEntry);
		}
	}
	
	// Get File List from WadList.txt in root Dir
	string WADListTxt = LoadTextFile(PATH_WAD_DIR+"WADList.txt");
	
	if (WADListTxt!="ERR")
	{
		WADListTxt += "\n";
		vector<string> Lines;
		SplitStrAtDelim(WADListTxt, '\n', Lines);
		
		for (int i=0;i<Lines.size();i++)
		{
			string &cL = Lines[i];
			string com="//";
			if (cL.find(com)!=0) // if line isnt commented out, add content to Wadlist and skip NewLine Char
			{
				WadFilePaths.push_back(cL.substr(0,cL.length()));
			}
		}
	}
	
	// Load WADs and get texture informations for the whole session of M2C
	for (int i=0;i<WadFilePaths.size();i++)
	{
		string &cWFP = WadFilePaths[i];
		if (CheckIfFileExists(cWFP))
		{
			WADFile nWAD(cWFP);
			if(nWAD.valid&&nWAD.t_entries>0)
			WADFiles.push_back(nWAD);
		}
	}
	
	if (dev) {
	cout << endl << "    Found WAD Files in WAD Dir: " << endl;
	for (int i = 0; i<WADFiles.size(); i++)
	cout << "  #" << i << " " << WADFiles[i].FilePath << endl;}
}


/* ===== TRANSFORM METHODS ===== */

void tform::set(string input)
{ 
	// example input: "5 5 5"
	int comma_1 = 0; comma_1 = input.find(" ",0);
	int comma_2 = 0; if (comma_1!=-1) comma_2 = input.find(" ",comma_1+1);
	string str_x="0", str_y="0", str_z="0";
	str_x = input.substr(0,comma_1);
	if (comma_1!=-1) {
	str_y = input.substr(comma_1+1,comma_2-comma_1+1);
	str_z = input.substr(comma_2+1,input.length()-comma_2+1); }
	x = stof(str_x);
	y = stof(str_y);
	z = stof(str_z);
	
	if (x!=0||y!=0||z!=0) IsSet = 1;
}

ostream &operator<<(ostream &ostr, tform &t)
{
	return ostr << "( " << t.x << " " << t.y << " " << t.z << " )";
}


/* ===== CONSTRUCTION TABLE METHODS ===== */

void ctable::FillUnset(ctable &Filler)
{
	if (res<0)			res			= Filler.res;
	if (type<0)			type		= Filler.type;
	if (shift<0)		shift		= Filler.shift;
	if (tri<0)			tri			= Filler.tri;
	if (round<0)		round		= Filler.round;
	if (height<0)		height		= Filler.height;
	if (ramp<0)			ramp		= Filler.ramp;
	if (cornerfix<0)	cornerfix	= Filler.cornerfix;
	if (preverse<0)		preverse	= Filler.preverse;
	if (ramptex<0)		ramptex		= Filler.ramptex;
	if (psplit<0)		psplit		= Filler.psplit;
	if (append<0)		append		= Filler.append;
	if (bound<0)		bound		= Filler.bound;
	if (range_start<0)	range_start	= Filler.range_start;
	if (range_end<0)	range_end	= Filler.range_end;
	if (transit_tri<0)	transit_tri	= Filler.transit_tri;
	if (transit_round<0)transit_round= Filler.transit_round;
	if (gaps<0)			gaps		= Filler.gaps;
	if (gaplen<0)		gaplen		= Filler.gaplen;
	if (skipnull<0)		skipnull	= Filler.skipnull;
	if (d_enable<0)		d_enable	= Filler.d_enable;
	if (d_autoyaw<0)	d_autoyaw	= Filler.d_autoyaw;
	if (d_autopitch<0)	d_autopitch	= Filler.d_autopitch;
	if (d_pos<0)		d_pos		= Filler.d_pos;
	if (d_separate<0)	d_separate	= Filler.d_separate;
	if (d_autoname<0)	d_autoname	= Filler.d_autoname;
	if (d_draw<0)		d_draw		= Filler.d_draw;
	if (d_draw_rand<0)	d_draw_rand	= Filler.d_draw_rand;
	if (d_skip<0)		d_skip		= Filler.d_skip;
	if (nulltex=="UNSET")nulltex	= Filler.nulltex;
	if (spike_height<0)	spike_height= Filler.spike_height;
}

void ctable::FillDefaults()
{
	rad 		= 0;
	offset 		= 0;
	res 		= 8;
	obj 		= 0;
	type 		= 0;
	shift 		= 5;
	tri 		= 0;
	round 		= 0;
	height 		= 0;
	ramp 		= 0;
	cornerfix	= 0;
	preverse	= 0;
	ramptex  	= 0;
	psplit  	= 0;
	bound 		= 0;
	range_start	= 0;
	range_end 	= 100;
	transit_tri	= 0;
	transit_round= 0;
	gaps		= 0;
	gaplen		= 256;
	skipnull	= 0;
	d_enable 	= 1;
	d_autoyaw	= 1;
	d_autopitch	= 1;
	d_autoname 	= 0;
	d_pos		= 0;
	d_separate 	= 0;
	nulltex		= "NULL";
	spike_height = 4;
	append 		= 0;
	target 		= "UNSET";
	d_draw		= 0;
	d_skip		= 0;
	d_draw_rand	= 0;
}

void ctable::Print()
{
	cout << " Construction Table Print:" << endl;
	cout << "   target \t" << target << endl;
	cout << "   append \t" << append << endl << endl;
	//cout << "   source \t" << source << endl;
	cout << "   rad \t\t" << rad << endl;
	cout << "   offset \t" << offset << endl;
	cout << "   res \t\t" << res << endl;
	cout << "   obj \t\t" << obj << endl;
	cout << "   type \t" << type << endl;
	cout << "   shift \t" << shift << endl;
	cout << "   tri \t\t" << tri << endl;
	cout << "   round \t" << round << endl;
	cout << "   height \t" << height << endl;
	cout << "   ramp \t" << ramp << endl;
	cout << "   cornerfix \t" << cornerfix << endl;
	cout << "   preverse \t" << preverse << endl;
	cout << "   psplit \t" << psplit << endl;
	cout << "   ramptex \t" << ramptex << endl;
	cout << "   path \t" << path << endl;
	cout << "   bound \t" << bound << endl;
	cout << "   range_start \t" << range_start << endl;
	cout << "   range_end \t" << range_end << endl;
	cout << "   transit_tri \t" << transit_tri << endl;
	cout << "   transit_round \t" << transit_round << endl;
	cout << "   gaps \t" << gaps << endl;
	cout << "   gaplen \t" << gaplen << endl;
	cout << "   skipnull \t" << skipnull << endl;
	cout << "   scale \t" << scale << endl;
	cout << "   scale_src \t" << scale_src << endl;
	cout << "   rot \t\t" << rot << endl;
	cout << "   rot_src \t" << rot_src << endl;
	cout << "   move \t" << move << endl;
	cout << "   d_enable \t" << d_enable << endl;
	cout << "   d_autoyaw \t" << d_autoyaw << endl;
	cout << "   d_autopitch \t" << d_autopitch << endl;
	cout << "   d_pos \t" << d_pos << endl;
	cout << "   d_separate \t" << d_separate << endl;
	cout << "   d_pos_rand \t" << d_pos_rand << endl;
	cout << "   d_rotz_rand \t" << d_rotz_rand << endl;
	cout << "   d_movey_rand \t" << d_movey_rand << endl;
	cout << "   d_autoname \t" << d_autoname << endl;
	cout << "   d_draw \t" << d_draw << endl;
	cout << "   d_skip \t" << d_skip << endl;
	cout << "   d_draw_rand \t" << d_draw_rand << endl;
	cout << "   nulltex \t" << nulltex << endl;
	cout << "   spike_height \t" << spike_height << endl;
}


/* ===== CONSTRUCTION TABLE FUNCTIONS ===== */

// create original settings table
ctable* createTableS(int CurveCount, vector<string> &SettingsList, bool IDOffset)
{
	bool dev = 0;
	ctable *Table = new ctable[CurveCount];
	
	// create Evaluation List
	setting_list EvalList[CurveCount];
	for (int i=0; i<CurveCount; i++)
		EvalList[i].CreateList(slist, slist_id, slist_type, slist_min, slist_max);
	
	// Fill Eval List with raw String Settings from text file
	for (int s=0; s<SettingsList.size(); s+=2) // setting loop
	{
		for (int i=0; i<CurveCount; i++)
		{
			bool FoundAndSet = 0;
			EvalList[i].SetEntry(SettingsList[s], SettingsList[s+1], FoundAndSet);
			
			if (FoundAndSet) break;
		}
	}
	
	// cout
	if(dev)
	for (int s=0; s<SettingsList.size(); s+=2) // setting loop
		cout << " SettingsList["<<s<<"] -> " << SettingsList[s] << " content: [" << SettingsList[s+1] <<"]"<< endl;
	
	// Fill Settings Table with evaluated Settings
	for (int i=0; i<CurveCount; i++)
		EvalList[i].ConvertToTable(Table[i]);
	
	return Table;
}





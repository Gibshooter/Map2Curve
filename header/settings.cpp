#include "settings.h"
#include "file.h"
#include "utils.h"
#include "WAD3.h"
#include "group.h"

#include <iostream>
#include <vector>
#include <string>

#define DEBUG 0

using namespace std;

extern file *gFile;
extern vector<string> slist;
extern vector<int> slist_id;
extern vector<int> slist_type;
extern vector<int> slist_min;
extern vector<int> slist_max;
extern ctable* cTable;
extern ctable* dTable; // default Settings consisting of DEFAULTS.txt or internal defaults
extern ctable* sTable;
extern bool G_DEV;
extern bool ValidDefaults;
extern string ROOT;
extern vector<WADFile> WADFiles;
extern group *mGroup;
extern group *sGroup;
extern group *mDetailGroup;


/* ===== GENERAL FUNCTIONS ===== */

void LoadDefaultSettings()
{
	string DefaultsTXT = LoadTextFile(ROOT+"DEFAULTS.txt");
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
	#if DEBUG > 0
	bool dev = 0;
	#endif
	
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
	
	#if DEBUG > 0
	if (dev) {
	cout << endl << "    Found WAD Files in WAD Dir: " << endl;
	for (int i = 0; i<WADFiles.size(); i++)
	cout << "  #" << i << " " << WADFiles[i].FilePath << endl;}
	#endif
}



void GetSettings(string cfgstr, vector<string> &SettingList, vector<string> &lslist)
{
	#if DEBUG > 0
	bool dev = 0;
	#endif
	
	cfgstr += "\n";
	// count lines of config string
	int found = 0;
	int ctr = 0;
	while(found!=cfgstr.npos) {
		found = cfgstr.find_first_of("\n",found+1);
		ctr++;
	}
	//ctr++;
	
	// copy line by line into new config string
	#if DEBUG > 0
	if(dev) { cout << endl << " Copying line by line into new config string..." << endl; }
	#endif
	
	bool cfg_scanned[ctr]; for (int i = 0; i<ctr; i++) cfg_scanned[i] = 0;
	string cfg_lines[ctr];
	int start = 0;
	int end = 0;
	for (int i = 0; i<ctr; i++) {
		end = cfgstr.find_first_of("\n", end)+1;
		//int end_pos = cfgstr.find_first_of("\n\r", end);
		//end = end_pos+1;
		//if (end_pos==-1) end = cfgstr.length();
		cfg_lines[i] = cfgstr.substr(start,end-start);
		start = end;
		
		#if DEBUG > 0
		if(dev) {cout << "cfg_lines["<<i<<"]" << cfg_lines[i];}
		#endif
	}
	// start searching for keywords line by line
	
	#if DEBUG > 0
	if(dev) { cout << endl << " Searching for keywords line by line..." << endl; }
	#endif
	
	string phrase = "";
	// phrase loop
	int phrase_counter = slist.size(); //(sizeof(slist)/sizeof(*slist));
	for (int j = 0; j < phrase_counter; j++) {
		phrase = slist[j];
		int phrase_len = slist[j].length();
		
		#if DEBUG > 0
		if(dev) { cout << "  current phrase [" << phrase << "] #"<<j<<"/"<<phrase_counter<<" len " << phrase_len << endl; }
		#endif
		
		// config lines loop
		for (int i = 0; i<ctr; i++) // search for phrase
		{
			if (!cfg_scanned[i])
			{
				int found_p = cfg_lines[i].find(phrase,0);
				int found_p_space; if(found_p!=-1) found_p_space = cfg_lines[i].find_first_of(" \t",found_p);
				int found_p_len = found_p_space-found_p;
				
				if (found_p_len==phrase_len)
				{
					bool comment = false;
					bool lookalike = false;
					
					// check if phrase is commented out
					if(found_p!=-1) {
						int found_comm = cfg_lines[i].find("//",0);
						if ( found_comm!=-1 && found_comm<found_p )
							comment = 1;
					}
					
					// check if found phrase is really the phrase that is being looked for: tri!=transit_tri (is true if position in front of found is new line or spacer)
					if(found_p!=-1&&!comment) {
						char c = cfg_lines[i][found_p-1];
						if ( found_p>0 &&
							c!=' ' &&
							c!='\t' )
							lookalike = 1;
					}
					
					//Phrase found, search for value
					if(found_p!=-1&&!comment&&!lookalike)
					{
						#if DEBUG > 0
						if(dev) { cout<< "     Scanned list #"<<i<<"/"<<ctr<< " Phrase found, no Comment/Lookalike, searching value..."; }
						#endif
						
						string str_f_value = GetValue(cfg_lines[i], found_p+phrase_len);
						if(str_f_value!="ERR")
						{
							#if DEBUG > 0
							if(dev) { cout << " found!" << endl; }
							#endif
							
							SettingList.push_back(phrase);
							SettingList.push_back(str_f_value);
						}
						else
						{
							#if DEBUG > 0
							if(dev) {cout << " NOT found!" << endl;}
							#endif
						}
						cfg_scanned[i] = 1;
					}
				}
			}
		}
		
		#if DEBUG > 0
		if(dev) { cout << endl; }
		#endif
	}
	
	// look for d_autoassign command in advance
	for (int i = 0; i < SettingList.size(); i+=2) {
		if(SettingList[i]=="d_autoassign" && SettingList[i+1]=="1")
		gFile->d_autoassign = 1;
	}
	
	#if DEBUG > 0
	if (dev) {
		int found_arcs = 0;
		cout << endl << "All found phrases and values:\n";
		for (int i = 0; i < SettingList.size(); i+=2) {
			cout << "  Phrase #"<<i<<" [" << SettingList[i] << "] has value [" << SettingList[i+1] << "]" << endl;
			if (SettingList[i]=="rad") found_arcs++;
		}
		cout << " Found a total of " << found_arcs << " arcs!" << endl;
		system("pause");
	}
	#endif
}

string GetCustomPhraseValue(string text, string phrase)
{
	#if DEBUG > 0
	bool dev = 0;
	#endif
	
	// Check for custom source path in settings file

	string value = "";
	int f_pos = 0;
	
	while (f_pos!=-1)
	{
		#if DEBUG > 0
		if(dev) cout << "    f_pos: " << f_pos << " find phrase ";
		#endif
		
		f_pos = text.find(phrase, f_pos);

		#if DEBUG > 0
		if(dev) cout << f_pos << endl;
		#endif
		
		if (CheckPhrase(text, f_pos))
		{
			#if DEBUG > 0
			if(dev) cout << "      Phrase at pos " << f_pos << " is NOT commented out!" << endl;
			#endif
			
			value = GetValue(text, f_pos+phrase.length());
			
			#if DEBUG > 0
			if(dev) cout << "      value " << value << " file exists " << CheckIfFileExists(value) <<  endl;
			#endif
			
			return value;
		}
		f_pos = text.find(phrase, f_pos+1);
	}
	return "ERR";
}

string GetValue(string text, int start_pos)
{
	#if DEBUG > 0
	bool dev = 0;
	#endif
	
	int f_pos_v = text.find_first_not_of(" \t", start_pos);
	int f_pos_n = text.find("\n", start_pos);
	int f_pos_c = text.find("//", start_pos);

	#if DEBUG > 0
	if(dev) cout << "         Get Value - value pos " << f_pos_v << " newline pos " << f_pos_n << " comment pos " << f_pos_c << endl;
	#endif

	int f_pos_v_end = 0;
	if (f_pos_v==-1||f_pos_v==f_pos_n||f_pos_v==f_pos_c)
	{
		#if DEBUG > 0
		if(dev) cout << "           Value not found OR Value is comment!" << endl;
		#endif
		
		return "ERR";
	}
	else if (f_pos_v!=-1&&(f_pos_v<f_pos_n||f_pos_n==-1)&&(f_pos_v<f_pos_c||f_pos_c==-1))
	{
		bool quotes = 0;
		string value = "";
		
		#if DEBUG > 0
		if(dev) cout << "           Value found at " << f_pos_v;
		#endif
		
		if (text[f_pos_v]=='\"')
		{
			#if DEBUG > 0
			if(dev) cout << "Found quotes! ";
			#endif
			
			quotes = 1;
			f_pos_v_end = text.find_first_of("\"\n\t", f_pos_v+1);
			value = text.substr(f_pos_v+1, f_pos_v_end-f_pos_v-1);
		}
		else
		{
			#if DEBUG > 0
			if(dev) cout << "Didnt find quotes! ";
			#endif
			
			f_pos_v_end = text.find_first_of("\n \t", f_pos_v);
			value = text.substr(f_pos_v, f_pos_v_end-f_pos_v);
		}
		#if DEBUG > 0
		if(dev) cout << " end " << f_pos_v_end << " final " << value << endl;
		#endif
		
		return value;
	}
}

bool CheckPhrase(string text, int pos)
{
	#if DEBUG > 0
	bool dev = 0;
	if(dev)cout << "           Checking Validity of Phrase..." << endl;
	#endif
	
	if (pos==-1) return 0;
	else if (pos==0) return 1;
	else
	{
		#if DEBUG > 0
		if (dev) cout << " Checking if found phrase is commented out..." << endl;
		#endif
		
		// check if found phrase has anything but space or tabs in front of it (if it does, it is invalid!)
		// check if found phrase is commented out
		int newline = text.rfind("\n",pos); // new line
		int comment = text.rfind("//",pos); // comment brackets
		int dirt = text.find_last_not_of(" \t",pos-1);
		
		#if DEBUG > 0
		if(dev) cout << "             pos " << pos << " newline " << newline << " comment " << comment << " dirt " << dirt << endl;
		#endif
		
		if ((newline==-1&&comment==-1&&dirt==-1)||(comment<newline&&dirt<=newline)) // phrase is located in first line and/or not commented out
			return 1;
		else if (comment>newline||dirt>newline)
			return 0;
		else 
			return 0;
	}
}









/* ===== TRANSFORM METHODS ===== */

void tform::set(string input)
{
	#if DEBUG > 0
	bool dev = 0;
	#endif
	
	int HasNumbers = input.find_first_of("0123456789");
	// example input: "5 5 5"
	if(HasNumbers!=-1)
	{
		#if DEBUG > 0
		if(dev) cout << " Raw Tform input: [" << input << "]" << endl;
		#endif
		
		string V = "-0123456789.|";
		string Clean;
		for (int i=0,s=0; i<input.length(); i++) {
			if (input[i]!=' ') {
				Clean += input[i];
			} else {
				if (i>0&&input[i-1]!=' '&&s<2) {
				Clean += "|";
				s++;
				}
			}
		}
		if(Clean[Clean.length()-1]=='|') Clean.erase(Clean.begin()+Clean.length()-1);

		#if DEBUG > 0
		if(dev) { cout << " Cleaned Tform input: [" << Clean << "]" << endl; }
		#endif
		
		if (!ContainsInvalids(Clean, V))
		{
			int comma_1 = 0; comma_1 = Clean.find("|",0);
			int comma_2 = 0; if (comma_1!=-1) comma_2 = Clean.find("|",comma_1+1);
			string str_x="0", str_y="0", str_z="0";
			str_x = Clean.substr(0,comma_1);
			if (comma_1!=-1) {
			str_y = Clean.substr(comma_1+1,comma_2-comma_1+1);
			if (comma_2!=-1)
			str_z = Clean.substr(comma_2+1,Clean.length()-comma_2+1); }
			x = stof(str_x);
			y = stof(str_y);
			z = stof(str_z);
			
			IsSet = 1; //if (x!=0||y!=0||z!=0) 
		}

		#if DEBUG > 0
		if(dev) cout << *this << endl;
		if(dev) system("pause");
		#endif
	}
}

ostream &operator<<(ostream &ostr, tform &t)
{
	return ostr << "( " << t.x << " " << t.y << " " << t.z << " )";
}


/* ===== CONSTRUCTION TABLE METHODS ===== */

void setting::SetThis(string val)
{
	string ValidCharsBool = "01";
	string ValidCharsInt ="-+01234567890";
	string ValidCharsFloat = ".-+01234567890";
	string ValidCharsTform = " .-+01234567890";
	
	#if DEBUG > 0
	bool dev = 0;
	if (dev)cout << " Trying to  set Setting " << name <<"(type "<<type<<", ID"<<ID<<") to " << val << "..." <<endl;
	#endif
	
	if (type==0) { // bool
		if (!ContainsInvalids(val, ValidCharsBool)) {
			IsSet = 1;
			if (val=="1") val_bool = 1; else val_bool = 0;
			
			#if DEBUG > 0
			if (dev)cout << "   Value is of type bool! val now " << val_bool << endl << endl;
			#endif
		}
	}
	else if (type==1) { // int
		if (!ContainsInvalids(val, ValidCharsInt)) {
			IsSet = 1;
			val_int = stoi(val);
			if (val_int<MIN) val_int = MIN;
			else if (val_int>MAX) val_int = MAX;
			
			#if DEBUG > 0
			if (dev)cout << "   Value is of type int! val now " << val_int << endl << endl;
			#endif
		}
	}
	else if (type==2) { // float
		if (!ContainsInvalids(val, ValidCharsFloat)) {
			IsSet = 1;
			val_float = stof(val);
			if (val_float<MIN) val_float = MIN;
			else if (val_float>MAX) val_float = MAX;
			
			#if DEBUG > 0
			if (dev)cout << "   Value is of type float! val now " << val_float << endl << endl;
			#endif
		}
	}
	else if (type==3) { // string
		if (val!="UNSET"&&val!="ERR"&&val.length()>=MIN&&val.length()<=MAX) {
			IsSet = 1;
			val_string = val;
			
			#if DEBUG > 0
			if (dev)cout << "   Value is of type string! val now " << val_string << endl << endl;
			#endif
		}
	}
	else if (type==4) { // tform
		if (!ContainsInvalids(val, ValidCharsTform)) {
			val_tform.set(val);
			if(val_tform.IsSet) IsSet = 1;
			
			#if DEBUG > 0
			if (dev)cout << "   Value is of type tform! val now " << val_tform << endl << endl;
			#endif
		}
	}
	else
	{
		#if DEBUG > 0
		if (dev)cout << "   Didnt set anything!" << endl << endl;
		#endif
	}
	
	#if DEBUG > 0
	if(dev)Print();
	#endif
}

void setting::Print() { cout << " Setting: " << name << " \tID " << ID << " \ttype " << type << " \tmin " << MIN << " \tmax " << MAX << endl;}

setting::setting(string s_name, int s_ID, int s_type, int s_min, int s_max)
{
	name = s_name;
	ID = s_ID;
	type = s_type;
	MIN = s_min;
	MAX = s_max;
}

void ctable::FillUnset(ctable &Filler)
{
	//if (rad<0)			rad			= Filler.rad;
	if (offset==-1)		offset		= Filler.offset;
	if (res<0)			res			= Filler.res;
	if (type<0)			type		= Filler.type;
	if (shift<0)		shift		= Filler.shift;
	if (tri<0)			tri			= Filler.tri;
	if (round<0)		round		= Filler.round;
	if (height==-1)		height		= Filler.height;
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
	if (path=="UNSET")	path		= Filler.path;
	if (heightmode<0)	heightmode	= Filler.heightmode;
	if (p_expand==0)	p_expand	= Filler.p_expand;
	if (p_evenout<0)	p_evenout	= Filler.p_evenout;
	if (c_enable<0)		c_enable	= Filler.c_enable;
	if (texmode<0)		texmode		= Filler.texmode;
	if (d_carve<0)		d_carve		= Filler.d_carve;
	if (d_autoassign<0)	d_autoassign= Filler.d_autoassign;
	if (d_circlemode<0)d_circlemode= Filler.d_circlemode;
	if (flatcircle<0)	flatcircle	= Filler.flatcircle;
	if (hstretch<0)		hstretch	= Filler.hstretch;
	if (hstretchamt<0)	hstretchamt	= Filler.hstretchamt;
	if (hshiftoffset==-1)	hshiftoffset	= Filler.hshiftoffset;
	if (hshiftsrc<0)	hshiftsrc	= Filler.hshiftsrc;
	
	if (!scale.IsSet)		scale		= Filler.scale;
	if (!scale_src.IsSet)	scale_src	= Filler.scale_src;
	if (!rot.IsSet)			rot			= Filler.rot;
	if (!rot_src.IsSet)		rot_src		= Filler.rot_src;
	if (!move.IsSet)		move		= Filler.move;
	if (!d_pos_rand.IsSet)	d_pos_rand	= Filler.d_pos_rand;
	if (!d_rotz_rand.IsSet)	d_rotz_rand	= Filler.d_rotz_rand;
	if (!d_movey_rand.IsSet)d_movey_rand= Filler.d_movey_rand;
	if (!d_scale_rand.IsSet)d_scale_rand= Filler.d_scale_rand;
	if (!p_scale.IsSet)		p_scale		= Filler.p_scale;
}

void ctable::CopyAll(ctable &Source)
{
	rad			= Source.rad;
	offset		= Source.offset;
	res			= Source.res;
	type		= Source.type;
	shift		= Source.shift;
	tri			= Source.tri;
	round		= Source.round;
	height		= Source.height;
	ramp		= Source.ramp;
	cornerfix	= Source.cornerfix;
	preverse	= Source.preverse;
	ramptex		= Source.ramptex;
	psplit		= Source.psplit;
	append		= Source.append;
	bound		= Source.bound;
	range_start	= Source.range_start;
	range_end	= Source.range_end;
	transit_tri	= Source.transit_tri;
	transit_round= Source.transit_round;
	gaps		= Source.gaps;
	gaplen		= Source.gaplen;
	skipnull	= Source.skipnull;
	d_enable	= Source.d_enable;
	d_autoyaw	= Source.d_autoyaw;
	d_autopitch	= Source.d_autopitch;
	d_pos		= Source.d_pos;
	d_separate	= Source.d_separate;
	d_autoname	= Source.d_autoname;
	d_draw		= Source.d_draw;
	d_draw_rand	= Source.d_draw_rand;
	d_skip		= Source.d_skip;
	nulltex		= Source.nulltex;
	spike_height= Source.spike_height;
	path		= Source.path;
	heightmode	= Source.heightmode;
	p_expand	= Source.p_expand;
	p_evenout	= Source.p_evenout;
	c_enable	= Source.c_enable;
	texmode		= Source.texmode;
	d_carve		= Source.d_carve;
	d_autoassign= Source.d_autoassign;
	d_circlemode= Source.d_circlemode;
	flatcircle	= Source.flatcircle;
	hstretch	= Source.hstretch;
	hstretchamt	= Source.hstretchamt;
	hshiftoffset= Source.hshiftoffset;
	hshiftsrc	= Source.hshiftsrc;

	scale		= Source.scale;
	scale_src	= Source.scale_src;
	rot			= Source.rot;
	rot_src		= Source.rot_src;
	move		= Source.move;
	d_pos_rand	= Source.d_pos_rand;
	d_rotz_rand	= Source.d_rotz_rand;
	d_movey_rand= Source.d_movey_rand;
	d_scale_rand= Source.d_scale_rand;
	p_scale		= Source.p_scale;
}
void ctable::FillDefaults()
{
	rad 		= 0;
	offset 		= 0;
	res 		= 8;
	obj 		= 0;
	map 		= 0;
	rmf			= 1;
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
	nulltex		= "SOLIDHINT";
	spike_height = 4;
	append 		= 0;
	target 		= "UNSET";
	d_draw		= 0;
	d_skip		= 0;
	d_draw_rand	= 0;
	heightmode 	= 0;
	texmode 	= 0;
	d_carve 	= 0;
	d_autoassign= 0;
	p_expand 	= 0;
	p_evenout	= 0;
	flatcircle	= 0;
	d_circlemode = 0;
	c_enable	= 1;
	hstretch	= 0;
	hstretchamt	= 0;
	hshiftoffset= 0;
	hshiftsrc	= 1;
}

void ctable::Print()
{
	cout << " Construction Table Print:" << endl;
	cout << "   target \t" << target << endl;
	cout << "   append \t" << append << endl << endl;
	//cout << "   source \t" << source << endl;
	cout << "   rad \t\t" << rad << endl;
	cout << "   offset \t" << offset << endl;
	cout << "   offsetNO \t" << offset_NO << endl;
	cout << "   res \t\t" << res << endl;
	cout << "   obj \t\t" << obj << endl;
	cout << "   rmf \t\t" << rmf << endl;
	cout << "   map \t\t" << map << endl;
	cout << "   type \t" << type << endl;
	cout << "   shift \t" << shift << endl;
	cout << "   tri \t\t" << tri << endl;
	cout << "   round \t" << round << endl;
	cout << "   height \t" << height << endl;
	cout << "   heightmode \t" << heightmode << endl;
	cout << "   d_carve \t" << d_carve << endl;
	cout << "   d_autoassign \t" << d_autoassign << endl;
	cout << "   texmode \t" << texmode << endl;
	cout << "   ramp \t" << ramp << endl;
	cout << "   p_cornerfix \t" << cornerfix << endl;
	cout << "   p_reverse \t" << preverse << endl;
	cout << "   p_split \t" << psplit << endl;
	cout << "   p_scale \t" << p_scale << endl;
	cout << "   p_expand \t" << p_expand << endl;
	cout << "   p_evenout \t" << p_evenout << endl;
	cout << "   flatcircle \t" << flatcircle << endl;
	cout << "   ramptex \t" << ramptex << endl;
	cout << "   splinefile \t" << path << endl;
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
	cout << "   d_scale_rand \t" << d_scale_rand << endl;
	cout << "   d_autoname \t" << d_autoname << endl;
	cout << "   d_draw \t" << d_draw << endl;
	cout << "   d_skip \t" << d_skip << endl;
	cout << "   d_draw_rand \t" << d_draw_rand << endl;
	cout << "   d_circlemode \t" << d_circlemode << endl;
	cout << "   nulltex \t" << nulltex << endl;
	cout << "   spike_height \t" << spike_height << endl;
	cout << "   c_enable \t" << c_enable << endl;
	cout << "   hstretch \t" << hstretch << endl;
	cout << "   hstretchamt \t" << hstretchamt << endl;
	cout << "   hshiftoffset \t" << hshiftoffset << endl;
	cout << "   hshiftsrc \t" << hshiftsrc << endl;
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
	
	if(dev||G_DEV)
	for (int s=0; s<SettingsList.size(); s+=2) // setting loop
		cout << " SettingsList["<<s<<"] -> " << SettingsList[s] << " content: [" << SettingsList[s+1] <<"]"<< endl;
	
	// Fill Settings Table with evaluated Settings
	for (int i=0; i<CurveCount; i++)
		EvalList[i].ConvertToTable(Table[i]);
	
	return Table;
}


// calculate the resulting settings for each arc
void createTableC()
{
	#if DEBUG > 0
	bool dev = 0;
	if (dev) cout << "Creating " << mGroup->t_arcs << " Construction Tables..." << endl;
	#endif
	
	// Create construction Tables
	int &t_arcs = mGroup->t_arcs;
	if (cTable!=nullptr)
	{
		delete[] cTable;
		
		#if DEBUG > 0
		if (dev) cout << " Deleting existing cTable, containing " << t_arcs << " objects and creating a new one..." << endl;
		#endif
	}
	cTable = new ctable[t_arcs];
	
	// Fill Construction Tables with previously loaded Settings
	#if DEBUG > 0
	if (dev) cout << "   Filling " << t_arcs << " Construction Tables with previously loaded Settings..." << endl;
	#endif
	
	for (int a = 0; a<t_arcs; a++) {
		cTable[a] = sTable[a];

		#if DEBUG > 0
		if (dev) cout << "     Arc " << a << " rad: " << sTable[a].rad << "\t offset: " << sTable[a].offset << "\t type: " << sTable[a].type << "\t res: " << sTable[a].res << "\t shift: " << sTable[a].shift << "\t height: " << sTable[a].height << endl;
		#endif
	}
	
	// export settings
	if (cTable[0].target!="UNSET") gFile->target = cTable[0].target; else gFile->target = dTable[0].target;
	if (cTable[0].append!=-1) gFile->append = cTable[0].append; else gFile->append = dTable[0].append;
	
	#if DEBUG > 0
	if (dev) cout << "   Evaluate settings..." << endl;
	#endif
	
	// Evaluate settings
	for (int a = 0; a<t_arcs; a++) // arc loop
	{
		// type
		if 		(a==0&&cTable[0].type<0) 	cTable[0].type = dTable[0].type;	// if first type is 0, set to PI
		else if (a>0&&cTable[a].type<0) 	cTable[a].type = cTable[a-1].type; // if x-th type is 0, type is same as previous type
		
		// d_carve
		if 		(a==0&&cTable[0].d_carve<0) cTable[0].d_carve = dTable[0].d_carve;
		else if (a>0&&cTable[a].d_carve<0) 	cTable[a].d_carve = cTable[a-1].d_carve;
		
		// d_autoassign (unnecessary at the moment)
		if 		(a==0&&cTable[0].d_autoassign<0) 	cTable[0].d_autoassign = dTable[0].d_autoassign;
		else if (a>0&&cTable[a].d_autoassign<0) 	cTable[a].d_autoassign = cTable[a-1].d_autoassign;
		
		// obj
		if 		(a==0&&cTable[0].obj<0) 	cTable[0].obj = dTable[0].obj;
		else if (a>0&&cTable[a].obj<0) 		cTable[a].obj = cTable[a-1].obj;
		
		// map
		if 		(a==0&&cTable[0].map<0) 	cTable[0].map = dTable[0].map;
		else if (a>0&&cTable[a].map<0) 		cTable[a].map = cTable[a-1].map;
		
		// rmf
		if 		(a==0&&cTable[0].rmf<0) 	cTable[0].rmf = dTable[0].rmf;
		else if (a>0&&cTable[a].rmf<0) 		cTable[a].rmf = cTable[a-1].rmf;
		
		// offset
		if 		(a==0&&cTable[0].offset==-1) 	cTable[0].offset = dTable[0].offset;
		else if (a>0&&cTable[a].offset==-1) 	cTable[a].offset = cTable[a-1].offset;
		
		// rad & offset
		float MaxY = sGroup[a].Dimensions.yb;
		float MinY = sGroup[a].Dimensions.ys;
		float size = MaxY - MinY;
		if(size==0) { MaxY = mDetailGroup->Dimensions.yb; MinY = mDetailGroup->Dimensions.ys; size = MaxY - MinY; }
		
		if 		(cTable[a].rad==0) 			cTable[a].rad = MaxY+cTable[a].offset;	// if first rad is 0, set rad to biggest map-y coord + offset
		else 								cTable[a].rad += cTable[a].offset;
		
		// if (cTable[a].type<2) // prevent arc radius smaller than 0 or ect size if arc type is PI or GRID Circle (paths can have negative source-ect coordinates)
		if ( cTable[a].rad-size <= 0) cTable[a].rad = size; //+cTable[a].offset
		if ( cTable[a].type==2 )
		{
			cTable[a].rad = size+cTable[a].offset;
			//cout << " FIXING Radius of Grid Path Type source map... Rad currently " << cTable[a].rad << " map Y size " << size << " offset " << cTable[a].offset << endl;
			if ( cTable[a].rad < size) { cTable[a].rad = size; } //cout << "   Rad bigger than size+offset ("<<size+cTable[a].offset<<") Rad now " << cTable[a].rad << endl; }
		}
		else if ( cTable[a].type==3 )
		{
			cTable[a].rad = (size/2)+cTable[a].offset;
		}
		sGroup[a].SizeY = size;
		
		// original radius offset
		cTable[a].offset_NO = (cTable[a].rad-(size/2)) - (mGroup->Dimensions.yb-(mGroup->SizeY/2));
		if(sGroup[a].t_brushes==0) cTable[a].offset_NO = (cTable[a].rad-(size/2)) - (mDetailGroup->Dimensions.yb-(mDetailGroup->SizeY/2));
		
		//res
		if (cTable[a].type==0) { // PI Circle Type
			if 		(a==0&&cTable[0].res<=0) cTable[0].res = dTable[0].res;	// if first res is 0, set to minimum of 8
			else if (a>0&&cTable[a].res<=0) {
				float m = cTable[a].rad/cTable[a-1].rad; //cout << "multiplier: " << m << endl;
				int temp = (m*cTable[a-1].res)/4.0; //cout << "temp (last res/4): " << m << endl;
				int temp2 = temp*4; //cout << "temp2 (temp*4): " << m << endl;
				cTable[a].res = temp2; // if x-th res is 0, set it to value based on the current and last radius+offset
			} else if (a==0&&cTable[0].res>0) {
				int temp = cTable[a].res/4.0;
				cTable[a].res = temp*4;
			}
			if (cTable[a].res <= 4) cTable[a].res = 4; // always set res to a minimum of 4
		}
		else if (cTable[a].type==1) { // grid Circle Type
			
			#if DEBUG > 0
			if(dev) cout << endl << "          curve#" << a << endl;	
			#endif
			
			if (a==0&&cTable[a].res<=0)
				cTable[a].res = 12;							// if first res is 0, set to minimum of 12

			//else if (a>0&&cTable[a].res==-1) cTable[a].res = cTable[a-1].res;
				
			else if (a>0&&cTable[a].res<=0)					// if x-th res is 0, get res based on previous res
			{
				//float m = cTable[a].rad/cTable[a-1].rad;	// old method pre 15. Dec 2023
				//int temp = ((m/2)*cTable[a-1].res);		// ^
				float m = cTable[a].rad/cTable[a-1].rad;
				int temp = (m*cTable[a-1].res)/4.0;
				int temp2 = temp*4;
				
				#if DEBUG > 0
				if(dev) cout << "          rad: " << cTable[a].rad <<endl << "          last rad:"<< cTable[a-1].rad <<endl << "          result m: " << m << endl;
				if(dev) cout << "          temp: " << temp << endl;
				#endif
				
				CheckFixRes(temp2, 2);
				cTable[a].res = temp2;						// if x-th res is 0, set it to value based on the current and last radius+offset
			}
			else if (cTable[a].res>0&&cTable[a].res<7) 		// res 1..6 (12..384)
			{
				if (cTable[a].res>1) {						// res 2..6 (24..384)
					int m = 12;
					for (int i = 0; i<cTable[a].res-1; i++) {
						m *= 2;
					}
					cTable[a].res = m;
				}
				else cTable[a].res = 12;						// res 1 (12)
				
			}
			else if (cTable[a].res>=7&&cTable[a].res<=384)		// res 12..384
				CheckFixRes(cTable[a].res, 2);
		}
		#if DEBUG > 0
		if(dev) cout << "          final res: " << cTable[a].res << endl << endl;
		#endif
		
		// heightmode
		if 		(a==0&&cTable[0].heightmode<0) cTable[0].heightmode = dTable[0].heightmode;
		else if (a>0&&cTable[a].heightmode<0) 	cTable[a].heightmode = cTable[a-1].heightmode;
		
		// texmode
		if 		(a==0&&cTable[0].texmode<0) cTable[0].texmode = dTable[0].texmode;
		else if (a>0&&cTable[a].texmode<0) 	cTable[a].texmode = cTable[a-1].texmode;
		
		// path
		if 		(a==0&&cTable[a].path=="UNSET") 	cTable[a].path = dTable[0].path;
		if 		(a>0&&cTable[a].path=="UNSET") 		cTable[a].path = cTable[a-1].path;
		
		// bound
		if 		(a==0&&cTable[0].bound<0) 	cTable[0].bound = dTable[0].bound;
		else if (a>0&&cTable[a].bound<0) 	cTable[a].bound = cTable[a-1].bound;
		
		// skipnull
		if 		(a==0&&cTable[0].skipnull<0) 	cTable[0].skipnull = dTable[0].skipnull;
		else if (a>0&&cTable[a].skipnull<0) 	cTable[a].skipnull = cTable[a-1].skipnull;
		
		//shift
		if 		(a==0&&cTable[0].shift<0) 	cTable[0].shift = dTable[0].shift;
		else if (a>0&&cTable[a].shift<0) 	cTable[a].shift = cTable[a-1].shift;
		
		//ramp
		if 		(a==0&&cTable[0].ramp<0) 	cTable[0].ramp = dTable[0].ramp;
		else if (a>0&&cTable[a].ramp<0) 	cTable[a].ramp = cTable[a-1].ramp;
		
		//height
		if 		(a==0&&cTable[0].height==-1) 	cTable[0].height = dTable[0].height;
		else if (a>0&&cTable[a].height==-1) 	cTable[a].height = cTable[a-1].height;
		
		// tri
		if 		(a==0&&cTable[0].tri<0) 	cTable[0].tri = dTable[0].tri;
		else if (a>0&&cTable[a].tri<0) 		cTable[a].tri = cTable[a-1].tri;

		
		// round
		if 		(a==0&&cTable[0].round<0) 	cTable[0].round = dTable[0].round;
		else if (a>0&&cTable[a].round<0) 	cTable[a].round = cTable[a-1].round;

		
		// cornerfix (if type == 2 - Grid Path - fixes overlapping inner corners)
		if 		(a==0&&cTable[0].cornerfix<0) cTable[0].cornerfix = dTable[0].cornerfix;
		else if (a>0&&cTable[a].cornerfix<0)  cTable[a].cornerfix = cTable[a-1].cornerfix;

		
		// reverse path direction
		if 		(a==0&&cTable[0].preverse<0) cTable[0].preverse = dTable[0].preverse;
		else if (a>0&&cTable[a].preverse<0)  cTable[a].preverse = cTable[a-1].preverse;

		
		// ramptex - set horizontal texturing mode for ramps
		if 		(a==0&&cTable[0].ramptex<0) cTable[0].ramptex = dTable[0].ramptex;
		else if (a>0&&cTable[a].ramptex<0)  cTable[a].ramptex = cTable[a-1].ramptex;

		
		// split - determine whether a path gets split into little pieces or stays whole on export
		if 		(a==0&&cTable[0].psplit<0) cTable[0].psplit = dTable[0].psplit;
		else if (a>0&&cTable[a].psplit<0)  cTable[a].psplit = cTable[a-1].psplit;

		
		//range_start
		if 		(a==0&&cTable[0].range_start==-1) 	cTable[0].range_start = dTable[0].range_start;
		else if (a>0&&cTable[a].range_start==-1) 	cTable[a].range_start = cTable[a-1].range_start;

		
		//range_end
		if 		(a==0&&cTable[0].range_end==-1) 	cTable[0].range_end = dTable[0].range_end;
		else if (a>0&&cTable[a].range_end==-1) 		cTable[a].range_end = cTable[a-1].range_end;

		
		// range fix
		if 		(cTable[a].range_end<cTable[a].range_start) 	cTable[a].range_end=100;
		if 		(cTable[a].range_start>cTable[a].range_end) 	cTable[a].range_start=0;

		
		//transit_tri
		if 		(a==0&&cTable[0].transit_tri<0) 	cTable[0].transit_tri = dTable[0].transit_tri;
		else if (a>0&&cTable[a].transit_tri<0)  	cTable[a].transit_tri = cTable[a-1].transit_tri;

		
		//transit_round
		if 		(a==0&&cTable[0].transit_round<0) 	cTable[0].transit_round = dTable[0].transit_round;
		else if (a>0&&cTable[a].transit_round<0) 	cTable[a].transit_round = cTable[a-1].transit_round;

		
		// gaps
		if 		(a==0&&cTable[0].gaps<0) 			cTable[0].gaps = dTable[0].gaps;
		else if (a>0&&cTable[a].gaps<0) 			cTable[a].gaps = cTable[a-1].gaps;

		
		// scale
		if 		(a==0&&!cTable[0].scale.IsSet) 		cTable[0].scale = dTable[0].scale;
		else if (a>0&&!cTable[a].scale.IsSet) 		cTable[a].scale = dTable[0].scale;

		
		// scale_src
		if 		(a==0&&!cTable[0].scale_src.IsSet) 	cTable[0].scale_src = dTable[0].scale_src;
		else if (a>0&&!cTable[a].scale_src.IsSet) 	cTable[a].scale_src = dTable[0].scale_src;

		
		// rot
		if 		(a==0&&!cTable[0].rot.IsSet) 		cTable[0].rot = dTable[0].rot;
		else if (a>0&&!cTable[a].rot.IsSet) 		cTable[a].rot = dTable[0].rot;

		
		// rot_src
		if 		(a==0&&!cTable[0].rot_src.IsSet) 	cTable[0].rot_src = dTable[0].rot_src;
		else if (a>0&&!cTable[a].rot_src.IsSet) 	cTable[a].rot_src = dTable[0].rot_src;

		
		// move
		if 		(a==0&&!cTable[0].move.IsSet) 		cTable[0].move = dTable[0].move;
		else if (a>0&&!cTable[a].move.IsSet) 		cTable[a].move = dTable[0].move;

		
		// gaplen
		if 		(a==0&&cTable[0].gaplen<0) 			cTable[0].gaplen = dTable[0].gaplen;
		else if (a>0&&cTable[a].gaplen<0) 			cTable[a].gaplen = cTable[a-1].gaplen;

		
		// d_enable
		if 		(a==0&&cTable[0].d_enable<0) 		cTable[0].d_enable = dTable[0].d_enable;
		else if (a>0&&cTable[a].d_enable<0) 		cTable[a].d_enable = cTable[a-1].d_enable;

		// c_enable
		if 		(a==0&&cTable[0].c_enable<0) 		cTable[0].c_enable = dTable[0].c_enable;
		else if (a>0&&cTable[a].c_enable<0) 		cTable[a].c_enable = cTable[a-1].c_enable;
		
		// d_autoyaw
		if 		(a==0&&cTable[0].d_autoyaw<0) 		cTable[0].d_autoyaw = dTable[0].d_autoyaw;
		else if (a>0&&cTable[a].d_autoyaw<0) 		cTable[a].d_autoyaw = cTable[a-1].d_autoyaw;

		
		// d_autopitch
		if 		(a==0&&cTable[0].d_autopitch<0) 	cTable[0].d_autopitch = dTable[0].d_autopitch;
		else if (a>0&&cTable[a].d_autopitch<0) 		cTable[a].d_autopitch = cTable[a-1].d_autopitch;

		
		// d_pos
		if 		(a==0&&cTable[0].d_pos<0) 			cTable[0].d_pos = dTable[0].d_pos;
		else if (a>0&&cTable[a].d_pos<0) 			cTable[a].d_pos = cTable[a-1].d_pos;

		
		// d_separate
		if 		(a==0&&cTable[0].d_separate<0) 		cTable[0].d_separate = dTable[0].d_separate;
		else if (a>0&&cTable[a].d_separate<0) 		cTable[a].d_separate = cTable[a-1].d_separate;

		
		// d_autoname
		if 		(a==0&&cTable[0].d_autoname<0) 		cTable[0].d_autoname = dTable[0].d_autoname;
		else if (a>0&&cTable[a].d_autoname<0) 		cTable[a].d_autoname = cTable[a-1].d_autoname;

		
		// nulltex
		if 		(a==0&&cTable[0].nulltex=="UNSET") 	cTable[0].nulltex = dTable[0].nulltex;
		if 		(a>0&&cTable[a].nulltex=="UNSET") 	cTable[a].nulltex = cTable[a-1].nulltex;

		
		// spike_height
		if 		(a==0&&cTable[0].spike_height<0) 	cTable[0].spike_height = dTable[0].spike_height;
		else if (a>0&&cTable[a].spike_height<0) 	cTable[a].spike_height = cTable[a-1].spike_height;

		
		// d_pos_rand
		if 		(a==0&&!cTable[0].d_pos_rand.IsSet) 	cTable[0].d_pos_rand = dTable[0].d_pos_rand;
		else if (a>0&&!cTable[a].d_pos_rand.IsSet) 		cTable[a].d_pos_rand = cTable[a-1].d_pos_rand;

		
		// d_rotz_rand
		if 		(a==0&&!cTable[0].d_rotz_rand.IsSet) 	cTable[0].d_rotz_rand = dTable[0].d_rotz_rand;
		else if (a>0&&!cTable[a].d_rotz_rand.IsSet) 	cTable[a].d_rotz_rand = cTable[a-1].d_rotz_rand;

		
		// d_movey_rand
		if 		(a==0&&!cTable[0].d_movey_rand.IsSet) 	cTable[0].d_movey_rand = dTable[0].d_movey_rand;
		else if (a>0&&!cTable[a].d_movey_rand.IsSet) 	cTable[a].d_movey_rand = cTable[a-1].d_movey_rand;

		
		// d_draw
		if 		(a==0&&cTable[0].d_draw<0) 			cTable[0].d_draw = dTable[0].d_draw;
		else if (a>0&&cTable[a].d_draw<0) 			cTable[a].d_draw = cTable[a-1].d_draw;

		
		// d_skip
		if 		(a==0&&cTable[0].d_skip<0) 			cTable[0].d_skip = dTable[0].d_skip;
		else if (a>0&&cTable[a].d_skip<0) 			cTable[a].d_skip = cTable[a-1].d_skip;

		
		// d_circlemode
		if 		(a==0&&cTable[0].d_circlemode<0) 	cTable[0].d_circlemode = dTable[0].d_circlemode;
		else if (a>0&&cTable[a].d_circlemode<0) 	cTable[a].d_circlemode = cTable[a-1].d_circlemode;

		// d_draw_rand
		if 		(a==0&&cTable[0].d_draw_rand<0) 	cTable[0].d_draw_rand = dTable[0].d_draw_rand;
		else if (a>0&&cTable[a].d_draw_rand<0) 		cTable[a].d_draw_rand = cTable[a-1].d_draw_rand;


		// p_evenout
		if 		(a==0&&cTable[0].p_evenout<0) 		cTable[0].p_evenout = dTable[0].p_evenout;
		else if (a>0&&cTable[a].p_evenout<0) 		cTable[a].p_evenout = cTable[a-1].p_evenout;

		
		// flatcircle
		if 		(a==0&&cTable[0].flatcircle<0) 		cTable[0].flatcircle = dTable[0].flatcircle;
		else if (a>0&&cTable[a].flatcircle<0) 		cTable[a].flatcircle = cTable[a-1].flatcircle;
		
		
		// hstretch new since v0.8 update Dec 2023
		if 		(a==0&&cTable[0].hstretch<0) 		cTable[0].hstretch = dTable[0].hstretch;
		else if (a>0&&cTable[a].hstretch<0) 		cTable[a].hstretch = cTable[a-1].hstretch;
		
		// hstretchamt new since v0.8 update Dec 2023
		if 		(a==0&&cTable[0].hstretchamt<0) 	cTable[0].hstretchamt = dTable[0].hstretchamt;
		else if (a>0&&cTable[a].hstretchamt<0) 		cTable[a].hstretchamt = cTable[a-1].hstretchamt;
		
		// hshiftoffset new since v0.8 update Dec 2023
		if 		(a==0&&cTable[0].hshiftoffset==-1) 	cTable[0].hshiftoffset = dTable[0].hshiftoffset;
		else if (a>0&&cTable[a].hshiftoffset==-1) 	cTable[a].hshiftoffset = cTable[a-1].hshiftoffset;
		
		// hstretchamt new since v0.8 update Dec 2023
		if 		(a==0&&cTable[0].hshiftsrc<0) 		cTable[0].hshiftsrc = dTable[0].hshiftsrc;
		else if (a>0&&cTable[a].hshiftsrc<0) 		cTable[a].hshiftsrc = cTable[a-1].hshiftsrc;
		
		
		#if DEBUG > 0
		if (dev) { cTable[a].Print(); system("pause"); }
		#endif
	}
}


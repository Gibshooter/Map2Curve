#include "utils.h"
#include "settings.h"

#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <dirent.h> // dir, opendir, readdir, closedir
#include <math.h> // floorf, pow, sqrt, isnan, isinf, round
#include <sstream> // stringstream

using namespace std;

extern vector<string> slist;

/* ===== FILE & STRING HANDLING FUNCTIONS ===== */

bool ContainsInvalids(string Subject, string Valids)
{
	if (Subject.find_first_not_of(Valids))
	return 0;
	else return 1;
}

bool CheckPhrase(string text, int pos)
{
	bool dev = 0;
	//cout << "           Checking Validity of Phrase..." << endl;
	if (pos==-1) return 0;
	else if (pos==0) return 1;
	else
	{
		if (dev) cout << " Checking if found phrase is commented out..." << endl;
		// check if found phrase has anything but space or tabs in front of it (if it does, it is invalid!)
		// check if found phrase is commented out
		int newline = text.rfind("\n",pos); // new line
		int comment = text.rfind("//",pos); // comment brackets
		int dirt = text.find_last_not_of(" \t",pos-1);
		if(dev) cout << "             pos " << pos << " newline " << newline << " comment " << comment << " dirt " << dirt << endl;
		
		if ((newline==-1&&comment==-1&&dirt==-1)||(comment<newline&&dirt<=newline)) // phrase is located in first line and/or not commented out
			return 1;
		else if (comment>newline||dirt>newline)
			return 0;
		else 
			return 0;
	}
}

void SplitString(string value,string delimiter, vector<string> &Target)
{
	bool dev = 0;
	if (dev) cout << " Splitting string [" << value << "] at delimiter [" << delimiter << "]..." << endl;
	if (value.length()>2)
	{
		bool search = 1;
		int last = 0;
		int dlen = delimiter.length();
		// example value: 123 -123 444
		while(search)
		{
			int v_start = last;
			int v_end = value.find(delimiter, v_start);
			if (v_end==-1) { // if no delimiter was found, put cursor to end of string
				v_end = value.length();
				search = 0;
			}
			if (dev) cout << "   Value Start: " << v_start << " End " << v_end << endl;
			string sub_value = value.substr(v_start, v_end-v_start);
			if (dev) cout << "   New sub value: " << sub_value << " (from start " << v_start <<" and end " << v_end << ")" << endl;
			Target.push_back(sub_value);
			
			last = v_end+dlen;
			if (dev) cout << "   last now " << last << endl;
		}
	}
}

void SplitStrAtDelim(string Str, char Delim, vector<string> &Lines)
{
	int start = 0, end = 0;
	while (end!=-1) {
		end = Str.find_first_of("\n", start);
		if (end!=-1&&end>start+5) { // ignore empty and short lines
			Lines.push_back(Str.substr(start,end-start));
		}
		start = end+1;
	}
}

void WriteTextToFile(string L_FilePath, string Text)
{
	ofstream L_File;
	L_File.open(L_FilePath);
	
	if (L_File.is_open())
	{
		L_File << Text;
	}
	L_File.close();
}

/*void WriteTextToFile(string L_FilePath, ostream &ostr)
{
	ofstream L_File;
	L_File.open(L_FilePath);
	
	if (L_File.is_open())
	{
		L_File << ostr;
	}
	L_File.close();
}*/

void GetSettings(string cfgstr, vector<string> &SettingList, vector<string> &lslist)
{
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
	bool cfg_scanned[ctr];
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
		//cout << "cfg_lines["<<i<<"]" << cfg_lines[i] << endl;
	}
	// start searching for keywords line by line
	string phrase = "";
	// phrase loop
	int phrase_counter = slist.size(); //(sizeof(slist)/sizeof(*slist));
	for (int j = 0; j < phrase_counter; j++) {
		phrase = slist[j];
		int phrase_len = slist[j].length();
		//cout << " current phrase " << phrase << endl;
		// config lines loop
		for (int i = 0; i<ctr; i++) { // search for phrase
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
					if(found_p!=-1&&!comment&&!lookalike) {
						/*int phrase_end = found_p+phrase.length();
						int phrase_spacecom = cfg_lines[i].find_first_of(" \t",phrase_end);
						int found_val = cfg_lines[i].find_first_not_of(" \t\"", phrase_spacecom);
						int value_len = 0;
						if (cfg_lines[i][found_val-1]=='\"')
							value_len = cfg_lines[i].find_first_of("\"\t\n",found_val)-found_val;
						else
							value_len = cfg_lines[i].find_first_of(" \t\n",found_val)-found_val;
						
						string str_f_value = cfg_lines[i].substr(found_val,value_len);
						//cout << " config line "<<i<< " phrase " << phrase << " pos "<< found_p<< " value " << str_f_value << " pos " << found_val << " length " << value_len << endl;
						*/
						string str_f_value = GetValue(cfg_lines[i], found_p+phrase_len);
						if(str_f_value!="ERR") {
							SettingList.push_back(phrase);
							SettingList.push_back(str_f_value);
						}
						cfg_scanned[i] = 1;
					}
				}
			}
		}
	}

	/*cout << "All found phrases and values:\n";
	for (int i = 0; i < settings.size(); i+=2) {
		cout << "Phrase: [" << settings[i] << "] has value [" << settings[i+1] << "]" << endl;
	}*/
}

string GetCustomPhraseValue(string text, string phrase)
{
	// Check for custom source path in settings file

	//string phrase = "source";
	string value = "";
	int f_pos = 0;
	
	while (f_pos!=-1)
	{
		//cout << "    f_pos: " << f_pos << " find phrase ";
		f_pos = text.find(phrase, f_pos);
		//cout << f_pos << endl;
		
		if (CheckPhrase(text, f_pos))
		{
			//cout << "      Phrase at pos " << f_pos << " is NOT commented out!" << endl;
			value = GetValue(text, f_pos+phrase.length());
			//cout << "      value " << value << " file exists " << CheckIfFileExists(value) <<  endl;
			
			return value;
		}
		f_pos = text.find(phrase, f_pos+1);
	}
	return "ERR";
}

bool CheckIfFileExists (string p) {
	ifstream ifile(p.c_str());
	return (bool)ifile;
}

string LoadTextFile(string path)
{
	string filecontent = "";
	if (CheckIfFileExists(path))
	{
	    ifstream ifs(path);
		filecontent.assign( 	(istreambuf_iterator<char>(ifs) ),
	  							(istreambuf_iterator<char>()    ) );
	  	
	  	if (filecontent.length()==0) return "ERR";
	  	else return filecontent;
	}
	else return "ERR";
}

// get file list of a directory and store it in the passed vector
void GetFileList(string path, vector<string> &list)
{
	const char * c_path = path.c_str();
	DIR *dir;
	struct dirent *ent;
	if ((dir = opendir (c_path)) != NULL) {
	  /* print all the files and directories within directory */
	  while ((ent = readdir (dir)) != NULL) {
	    list.push_back(ent->d_name);
	  }
	  closedir (dir);
	}
}

int CheckFileType (string p)
{
	int f = p.find_last_of(".");
	string t = p.substr(f+1);
	if ( t == "txt" ) return 1;
	else if ( t == "map" ) return 2;
	else return 0;
}

string GetValue(string text, int start_pos)
{
	int f_pos_v = text.find_first_not_of(" \t", start_pos);
	int f_pos_n = text.find("\n", start_pos);
	int f_pos_c = text.find("//", start_pos);
	//cout << "         Get Value - value pos " << f_pos_v << " newline pos " << f_pos_n << " comment pos " << f_pos_c << endl;
	int f_pos_v_end = 0;
	if (f_pos_v==-1||f_pos_v==f_pos_n||f_pos_v==f_pos_c)
	{
		//cout << "           Value not found OR Value is comment!" << endl;
		return "ERR";
	}
	else if (f_pos_v!=-1&&(f_pos_v<f_pos_n||f_pos_n==-1)&&(f_pos_v<f_pos_c||f_pos_c==-1))
	{
		bool quotes = 0;
		string value = "";
		
		//cout << "           Value found at " << f_pos_v;
		if (text[f_pos_v]=='\"')
		{
			//cout << "Found quotes! ";
			quotes = 1;
			f_pos_v_end = text.find_first_of("\"\n\t", f_pos_v+1);
			value = text.substr(f_pos_v+1, f_pos_v_end-f_pos_v-1);
		}
		else
		{
			//cout << "Didnt find quotes! ";
			f_pos_v_end = text.find_first_of("\n \t", f_pos_v);
			value = text.substr(f_pos_v, f_pos_v_end-f_pos_v);
		}
		//cout << " end " << f_pos_v_end << " final " << value << endl;
		return value;
	}
}







/* ===== SMALL MATH FUNCTIONS ===== */

float RoundFloatToDeci(float N1, int deci)
{
	int d = pow(10, deci);
	if (d==0) d=1;
	return floorf(N1*d)/d;
}

float GetIsectCircleLine(float rad, float x)
{
	// circle (x-m1)²+(y-m2)² = r²
	// (x)²+(y)² = r²
	// straight line x = 32;
	// x² - r² = -y²
	// 1024-25600 = -y²
	// -24576 = -y²
	// 24576 = y²
	// y = 156,76
	
	float ext1 = pow(x,2) - pow(rad,2);
	ext1 *= -1;
	ext1 = sqrt(ext1);
	
	return ext1;
}

int n_pow(int a, int expo)
{
	if (expo == 1)
		return a;
	else if (expo == 0)
		return 1;
	else {
		
	int power = a;
	
	for (int i = 1; i < expo; i++ ){
		power *= a;
	}
	
	return power;
	}
}








/* ===== MISC NUMBER FUNCTIONS ===== */

float GetRandInRange(float min, float max)
{
	float r1 = (rand() % 10) / 10.0;
	float r2 = (rand() % 10) / 100.0;
	float m = r1 + r2;
	//cout << "    multiplier: " << m << " r1 " << r1 << " + r2 " << r2 << endl;
	float range = max - min;
	//cout << "     range: " << range << endl;
	//cout << "     Range*m ("<<range*m<<")+ min("<<min<<"): " << (range*m) + min << endl;
	return (range*m) + min;
}

bool CompareFloatDeci(float N1, float N2, int deciplace)
{
	bool dev = 0;
	int d = pow(10, deciplace);
	if (d==0) d=1;
	if ( floorf(N1*d)/d == floorf(N2*d)/d)
		return true;
	else
		return false;
}

bool IsNULL(double n)
{
	if (floorf(n*1000)/1000 == 0) return true;
	else return false;
}

bool IsValid(int n)
{
	bool valid = 0;
	if (isnan(n)||isinf(n)) valid = 0; else valid = 1;
	return valid;
}

bool IsValid(float n)
{
	bool valid = 0;
	if (isnan(n)||isinf(n)) valid = 0; else valid = 1;
	return valid;
}

int GetDeciPlaces(float RawFloat) {
	
	string floatstr = "";
	string decimal = "";
	
	// create stream
	stringstream stream;
	
	// put decimal number into stream
	stream.precision(10);
	stream << RawFloat;
	// safe stream with decimal number in this string
	floatstr = stream.str();
	
	int dotpos = floatstr.find(".");
	
	if (dotpos != string::npos)
	// safe the digits behind the decimal point in a new string
	decimal = floatstr.substr(dotpos+1);
	
	return decimal.length();
}

int reduce(int res)
{
	if (res == 12)
		return 1;
	
	int c = 0;
	for (int i = 12; i <= res; i *= 2)
		c++;
		
	return c;
}

bool IsResValid(int reso){
	
	int valids[] {24, 48, 96, 192, 384};
	
	if (reso == 12)
	{
		return true;
	}
	else if (reso <= 384 && reso > 12)
	{
		for (int i = 0; i<5; i++) {
			if (reso == valids[i]) return true;
		}
		return false;
		/*
		while ((reso != 12) && (reso > 12))
		{
			reso /= 2;
			
			if (reso == 12) {
				return true;
			} else if (reso < 12) {
				return false;
			}
		}*/
	} else {
		return false;
	}
}

void CheckFixRes (int& reso, int type) {
	if (reso>384) 		reso = 384;
	
	if (type==0) {
		if (reso<4) 	reso = 4;
		
	} else {
		if (reso<12)
		{
			reso = 12;
		}
		else {
			if (!IsResValid(reso)) {
				
				float valids[] {12.0, 24.0, 48.0, 96.0, 192.0, 384.0};
				float current;
				for (int i = 0; i < 6; i++)
				{
					current = reso/valids[i];
					
					if (round(current)==1) {
						reso = valids[i];
						break;
					}
				}
				if (!IsResValid) reso = 12;
			}
		}
	}
}


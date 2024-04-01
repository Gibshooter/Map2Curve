#include "utils.h"
#include "settings.h"
#include "frames.h"
#include "group.h"
#include "file.h"

#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <iomanip> // precision
#include <dirent.h> // dir, opendir, readdir, closedir
#include <math.h> // floorf, pow, sqrt, isnan, isinf, round
#include <sstream> // stringstream
#include <cwchar>

using namespace std;

extern vector<string> slist;
extern ctable *cTable;
extern group *bGroup;
extern file *gFile;
#define DEBUG 0

/* ===== NUMBER LIST GENERATOR FUNCTIONS ===== */


void CreateSlopeLinear(float height, int size, vector<float>&List, vector<float> &Lengths, int g)
{
	int rs = bGroup[g].range_start;
	int re = bGroup[g].range_end;
	int r = re-rs;
	
	List.resize(size+1);
	float maxlength = Lengths[re] - Lengths[rs]; // New v0.8
	float step = height/r;
	float m = height/maxlength;  // New v0.8
	
	#if DEBUG > 0
	bool dev = 0;
	if (dev) cout << " CreateSlopeLinear using - res " << size << " rs " << rs << " re " << re << " r " << r << " maxlength " << maxlength << " of max "<< Lengths[Lengths.size()-1] << " height " << height <<endl;
	#endif
	
	for (int i=rs, j=0; i<size+1; i++)
	{
		if (cTable[g].type>1) // path extrusions type 2/3 - New v0.8
		{
			float LengthStart = Lengths[i];
			if(rs>0) LengthStart = Lengths[i]-Lengths[rs]; // subtract previous lengths to start with 0 if range_start is >0
			List[i] = m * LengthStart;
		}
		else // circles type 0/1 - Old pre 0.8
		{
			List[i] = step*j;
			j++;
		}
	}
	
	#if DEBUG > 0
	if (dev) { cout << " FINAL height table (m "<<m<<" = size "<<size<<" / maxlength "<<maxlength<<"):" << endl; for (int i = 0; i<List.size(); i++) cout << "   #" << i << " " << List[i] << endl; system("pause"); }
	#endif
}

void CreateSlopeRandom(float step, int size, vector<float>&List, int g)
{
	#if DEBUG > 0
	bool dev = 0;
	if (dev) cout << " CreateSlopeRandom... size " << size << endl;
	#endif
	
	if(step!=0)
	{
		List.resize(size);
		for (int i = 0; i<List.size(); i++)
			List[i] = GetRandInRange(0, cTable[g].height); // pre v0.8: step*size);
	}
	else
	{
		List.resize(size);
		fill (List.begin(), List.end(), 0);
	}
	
	#if DEBUG > 0
	if (dev) { cout << " FINAL height table:" << endl; for (int i = 0; i<List.size(); i++) cout << "   #" << i << " " << List[i] << endl; system("pause"); }
	#endif
}


void CreateSlopeSpline(path_set &Spline, int size, vector<float>&List, vector<float>&Steps)
{
	#if DEBUG > 0
	bool dev = 0;
	#endif

	if (Spline.t_corners>0)
	{
		#if DEBUG > 0
		if (dev) cout << " CreateSlopeSpline... Spline Knots: " << Spline.t_corners <<" List size " << size << endl;
		#endif
		
		List.resize(size); 
		Steps.resize(size);
		for (int i = 0; i<List.size(); i++) {
			List[i]=0;
			Steps[i]=0;
		}
		
		int Secs = Spline.t_corners-1+Spline.Gaps;
		
		vector<float> SplineZList;
		vector<float> SplineStepList;
		SplineZList.resize(Secs);
		SplineStepList.resize(Secs);
		// extract height info of all corners first
		for (int p=0, z=0; p<Spline.t_paths; p++)
		{
			path &Path = Spline.Paths[p];
			for (int c = 0; c<Path.t_corners-1; c++)
			{
				path_corner &Corner = Path.Corners[c];
				path_corner &CornerN = Path.Corners[c+1];
				
				SplineZList[z] = Corner.pos.z;
				SplineStepList[z] = Corner.step;
				
				#if DEBUG > 0
				if (dev) cout << "   Corner #" << z << " " << Corner.pos << "\tStep" << Corner.step << "\tNext is Gap 90/180 " << Corner.NextIsGap90 << "/" <<Corner.NextIsGap180 << endl;
				#endif
				
				if (Corner.NextIsGap90) {
					SplineZList[z] = Corner.pos.z;
					SplineZList[z+1] = CornerN.pos.z;
					z+=2;
				}
				else if (Corner.NextIsGap180) {
					SplineZList[z] = Corner.pos.z;
					SplineZList[z+1] = CornerN.pos.z;
					SplineZList[z+2] = CornerN.pos.z;
					z+=3;
				}
				else
				{
					SplineZList[z] = Corner.pos.z;
					z++;
				}
			}
		}
		
		// now fill List with Z info from spline; repeat from start if spline too small
		for (int i=0, j=0; i<List.size(); i++)
		{
			List[i] = round(SplineZList[j]); // round Z heights too
			Steps[i] = round(SplineStepList[j]);
			
			#if DEBUG > 0
			if (dev) cout << "  hTable #" << i << " " << List[i] << "\tSplineZList["<<j<<"] " << SplineZList[j] << endl;
			#endif
			
			j++;
			if (j==SplineZList.size()) j=0;
		}
		
		#if DEBUG > 0
		if (dev) { cout << " height table:" << endl; for (int i = 0; i<List.size(); i++) cout << "   #" << i << " " << List[i]<< endl; system("pause"); }
		#endif
	}
	else
	{
		#if DEBUG > 0
		if (dev) cout << " No Spline loaded!" << endl;
		#endif
	}
}


void CreateSlopeEasings(float height, int size, vector<float>&List, vector<float> &Lengths, int g)
{
	#if DEBUG > 0
	bool dev = 0;
	#endif
	
	if (height!=0)
	{
		//bool NegHeight = 0;
		//if (height<0) NegHeight = 1;
		//if (NegHeight) height= -height;
		if (size<=1) size = 2;
		
		int rs = bGroup[g].range_start;
		int re = bGroup[g].range_end;
		if (cTable[g].type==2) re-=gFile->PathList[g].Gaps;
		int r = re-rs;
		
		float maxlength = Lengths[re] - Lengths[rs];
		float m = height/maxlength;
	 	
		#if DEBUG > 0
	 	if (dev) cout << " maxlen " << maxlength << " m " << m << " rs " << rs << " re " << re << " r " << r << endl;
		#endif
	 	
		List.resize(size+1);
		for (int i = 0; i<size+1; i++)
			List[i] = height;
		
		int gapoff = 0; if(cTable[g].type==2) gapoff = gFile->PathList[g].Gaps;
		
		for (int i=rs, l=0; i<size+1-gapoff; i++)
		{
			float x;
			if (cTable[g].type>1)
			{
				float LengthStart = Lengths[i];
				if(rs>0) LengthStart = Lengths[i]-Lengths[rs]; // subtract previous lengths to start with 0 if range_start is >0
				List[i] = m * LengthStart;
			
				x = LengthStart/maxlength;
			}
			else
			{
				x = (1.0/r)*l;
				l++;
			}
			
			#if DEBUG > 0
			if (dev) cout << " i " << i << " x " << x << " 1.0/size " << 1.0/size << endl;
			#endif
			
			List[i] = easing(x,cTable[g].heightmode);
			List[i] *= height;
		}
		
		//if (NegHeight) for(int i=0;i<List.size(); i++) List[i] *= -1;
		
		#if DEBUG > 0
		if (dev) { cout << " FINAL height table EASING:" << endl; for (int i = 0; i<List.size(); i++) cout << "   #" << i << " " << List[i] << endl; system("pause");
		system("pause"); }
		#endif
	}
	else
	{
		List.resize(size+1);
		for (int i = 0; i<size+1; i++)
			List[i] = 0;
	}
}






/* ===== FILE & STRING HANDLING FUNCTIONS ===== */

void ReplaceInList(vector<string> &List1, vector<string> &List2, string L1_candidate, string L2_replace)
{
	for(int i=0; i<List1.size(); i++)
	{
		string &EntryL1 = List1[i];
		string &EntryL2 = List2[i];
		if(EntryL1==L1_candidate) { EntryL2 = L2_replace; break; }
	}
}

bool ContainsInvalids(string Subject, string Valids)
{
	if (Subject.find_first_not_of(Valids)==-1)
	return 0;
	else return 1;
}

void SplitString(string value,string delimiter, vector<string> &Target)
{
	#if DEBUG > 0
	bool dev = 0;
	if (dev) cout << " Splitting string [" << value << "] at delimiter [" << delimiter << "]..." << endl;
	#endif
	
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
			#if DEBUG > 0
			if (dev) cout << "   Value Start: " << v_start << " End " << v_end << endl;
			#endif
			
			string sub_value = value.substr(v_start, v_end-v_start);
			
			#if DEBUG > 0
			if (dev) cout << "   New sub value: " << sub_value << " (from start " << v_start <<" and end " << v_end << ")" << endl;
			#endif
			
			Target.push_back(sub_value);
			
			last = v_end+dlen;
			
			#if DEBUG > 0
			if (dev) cout << "   last now " << last << endl;
			#endif
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
	  // print all the files and directories within directory
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







/* ===== SMALL MATH FUNCTIONS ===== */


float RoundFloatToDeci(float N1, int deci)
{
	int d = pow(10, deci);
	if (d==0) d=1;
	return round(N1*d)/d;
}


float easing(float x, int mode)
{
	//if (mode==0) // linear
	//	return 
	
	// The following functions were taken from https://easings.net
	if (mode==7) // easeInSine
		return 1 - cos((x * PI) / 2);
	else if (mode==11) // easeOutSine
		return sin((x * PI) / 2);
	else if (mode==3) // easeInOutSine
		return -( cos(PI*x)-1 ) / 2;
		
	else if (mode==8) // easeInQuad
		return x * x;
	else if (mode==12) // easeOutQuad
		return 1 - (1 - x) * (1 - x);
	else if (mode==4) // easeInOutQuad
		return x < 0.5 ? 2 * x * x : 1 - pow(-2 * x + 2, 2) / 2;
		
	else if (mode==9) // easeInCubic
		return x * x * x;
	else if (mode==13) // easeOutCubic
		return 1 - pow(1 - x, 3);
	else if (mode==5) // easeInOutCubic
		return x < 0.5 ? 4 * x * x * x : 1 - pow(-2 * x + 2, 3) / 2;
		
	else if (mode==10) // easeInQuart
		return x * x * x * x;
	else if (mode==14) // easeOutQuart
		return 1 - pow(1 - x, 4);
	else if (mode==6) // easeInOutQuart
		return x < 0.5 ? 8 * x * x * x * x : 1 - pow(-2 * x + 2, 4) / 2;
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

int IsBorderliner(float n, int prec)
{
	
	if(n<0) n=-n;
	
	#if DEBUG > 0
	bool dev = 0;
	if(dev) cout << setprecision(8) << " IsBorderliner " << n << " precision " << prec << endl;
	#endif

	int d = pow(10, prec);
	if (d==0) d=1;
	
	// n = 5.0001
	int n_round = n; // n_round = 5
	if( n == n_round )
	{
		#if DEBUG > 0
		if(dev)cout << " Number is even! "<<endl<<endl; return 2;
		#endif
		
	} // number is even
	else
	{
		float n_deci = n-n_round; // n_deci = 5.0001 - 5 = 0.0001
		if(round(n_deci)==0) // round(0.0001) == 0
		{
		}
		else // e.g. round(0.9999) == 1
		{
			n_deci -= 1; // 0.9999 - 1 = -0.0001
			n_deci = -n_deci; // n_deci = 0.0001
		}
		float temp = round(n_deci*d); // e.g. d=100; temp = 0.0001 * 100 = 0.01
		
		#if DEBUG > 0
		if(dev) cout << "   n_deci " << n_deci << endl;
		if(dev) cout << "   temp " << temp << endl;
		#endif
		
		if(temp==0)
		{
			#if DEBUG > 0
			if(dev)cout << " Number is Borderliner! "<<endl<<endl;
			#endif
			
			return 1;
		}
		else
		{
			#if DEBUG > 0
			if(dev)cout << " Number is NO Borderliner! "<<endl<<endl;
			#endif
			
			return 0;
		}
	}
}

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

bool IsResValid(int reso)
{
	int valids[] {24, 48, 96, 192, 384};
	
	if (reso == 12)
	{
		return true;
	}
	else if (reso <= 384 && reso > 12)
	{
		for (int i = 0; i<5; i++)
		{
			if (reso == valids[i]) return true;
		}
		return false;
	}
	else
	{
		return false;
	}
}

void CheckFixRes (int& reso, int type)
{
	if (reso>384) 		reso = 384;
	
	if (type==0)
	{
		if (reso<4) 	reso = 4;
	}
	else
	{
		if (reso<12)
		{
			reso = 12;
		}
		else
		{
			if (!IsResValid(reso))
			{
				float valids[] {12.0, 24.0, 48.0, 96.0, 192.0, 384.0};
				float current;
				for (int i = 0; i < 6; i++)
				{
					current = reso/valids[i];
					
					if (round(current)==1)
					{
						reso = valids[i];
						break;
					}
				}
				if (!IsResValid) reso = 12;
			}
		}
	}
}


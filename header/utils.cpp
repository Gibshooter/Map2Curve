#include "utils.h"
#include "settings.h"
#include "frames.h"

#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <iomanip> // precision
#include <dirent.h> // dir, opendir, readdir, closedir
#include <math.h> // floorf, pow, sqrt, isnan, isinf, round
#include <sstream> // stringstream
#include <conio.h> // getch
#include <windows.h>
#include <cwchar>

using namespace std;

extern vector<string> slist;



/* ===== CANVAS METHODS ===== */

void canvas::CreateLine(float scale, vertex V1, vertex V2)
{
	bool dev = 0;
	if (dev) cout << " Creating Line..." << V1 << V2 << endl;
	if (scale!=1.0) { V1.scale(scale); V2.scale(scale); }
	gvector Vec = GetVector(V1, V2);
	if (round(Vec.x)!=round(Vec.y))
	{
		float VLen = Vec.len(); // 9.2
		gvector HBase(1,0,0);
		int Hres = ceil(GetAdjaLen(Vec, HBase))*2.0; // 5
		if (Hres<0) Hres*=-1;
		float SegLen = (VLen/Hres); // 0.92
		gvector VLenDiv = Vec; VLenDiv.div(Hres);
		if(dev) cout << " Vec " << Vec << " VLen " << VLen << " Hres " << Hres << " SegLen " << SegLen << " VLenDiv " << VLenDiv <<  endl;
		gvector Off[Hres];
		for (int i=0;i<Hres;i++) {
			Off[i] = VLenDiv;
			Off[i].mult(i+1);
			if(dev) cout << " Offset #" << i << " " << Off[i] <<  endl;
		}
		vector<vertex> Line; Line.resize(Hres);
		if(dev) cout << " Creating Line... HRes" << Hres << endl;
		for (int i=0;i<Hres;i++) {
			Line[i].x = floor(V1.x + Off[i].x);
			Line[i].y = floor(V1.y + Off[i].y);
			if(dev) cout << " Line V" << i << " X " << Line[i].x << " Y " << Line[i].y << endl;
		}
		if(dev) getch();
		if (Hres>0) PasteLine(Line);
	}
	else
	{
		vector<vertex> Line;
		Line.push_back(V1);
		PasteLine(Line);
	}
}

void canvas::PasteLine(vector<vertex>&Line)
{
	bool dev = 0;
	if(dev) cout << " Pasting Line.." << endl;
	for (int i=0; i<Line.size(); i++) {
		if (Line[i].y<p_h && Line[i].y>=0 && Line[i].x<p_w && Line[i].x>=0)
			if (i==0||i==Line.size()-1)
				Pixels[Line[i].y][Line[i].x] = 'O';
			else
				Pixels[Line[i].y][Line[i].x] = 'X';
	}
}

void canvas::Print()
{
	bool dev = 0;
	cout << " Printing Canvas of Size " << p_h << " x " << p_w << endl;
	
	/*CONSOLE_FONT_INFOEX cfi;
	cfi.cbSize = sizeof(cfi);
	cfi.nFont = 0;
	cfi.dwFontSize.X = 8;                   // Width of each character in the font
	cfi.dwFontSize.Y = 8;                  // Height
	cfi.FontFamily = FF_DONTCARE;
	cfi.FontWeight = 100;
	std::wcscpy(cfi.FaceName, L"Raster Fonts"); // Choose your font
	SetCurrentConsoleFontEx(GetStdHandle(STD_OUTPUT_HANDLE), FALSE, &cfi);*/
	
	for (int i=p_h-1;i>=0; i--)
		cout << Pixels[i];
	cout << endl;
}

canvas::canvas(int w, int h)
{
	bool dev = 0;
	if(dev) cout << " Creating Canvas... w"<< w<< " h " << h  << endl;
	p_w = w; if(p_w>200) p_w=200;
	p_h = h; if(p_h>400) p_h=400;
	
	Pixels.resize(p_h);
	for (int i=0;i<p_h; i++) {
		Pixels[i].replace(0,p_w,p_w,' ');
		Pixels[i] += "\n";
	}
	if(dev) cout << " Canvas created! Height: " << Pixels.size() << " Width " << Pixels[0].size() << endl;
	if(dev) getch();
}






/* ===== NUMBER LIST GENERATOR FUNCTIONS ===== */


void CreateSlopeLinear(float step, int size, vector<float>&List)
{
	bool dev = 0;
	if (dev) cout << " CreateSlopeLinear using step " << step << endl;
	
	List.resize(size+1);
	for (int i=0; i<size+1; i++)
		List[i] = step*i;
	
	/*ListRel.resize(res);
	for (int i=0, j=0, gap=0; i<res; i++)
	{
		if(dev) cout << " List #" << i << "/"<<res<< " start-offset " << j << " Gap " << GapList[i] << endl;
		List[i] = 0;
		ListRel[i] = 0;
		if (i>=start) {
			if (!GapList[i]) {
				List[i] = step*j;
				ListRel[i] = step;
				j++;
			} else {
				List[i] = step*(j-gap);
				ListRel[i] = 0;
			}
		}
	}*/
	
	if (dev) { cout << " FINAL height table:" << endl; for (int i = 0; i<List.size(); i++) cout << "   #" << i << " " << List[i] << endl; getch(); }
}

void CreateSlopeRandom(float step, int size, vector<float>&List)
{
	bool dev = 0;
	if (dev) cout << " CreateSlopeRandom... size " << size << endl;
	
	if(step!=0)
	{
		List.resize(size);
		for (int i = 0; i<List.size(); i++)
			List[i] = GetRandInRange(0, step*size);
		
		/*for (int i=0, j=0; i<res; i++)
		{
			if(dev) cout << " List #" << i << "/"<<res<< " start-offset " << j << " Gap " << GapList[i] << endl;
			List[i] = 0;
			ListRel[i] = 0;
			if (i>=start) {
				if (!GapList[i]) {
					List[i] = RandList[j];
					ListRel[i] = RandList[j+1]-RandList[j];
					j++;
				} else {
					List[i] = RandList[j];
					ListRel[i] = 0;
				}
			}
		}*/
		
		// create relative height (step) list
		/*for (int i = 0; i<List.size()-1; i++)
		{
			//ListRel[i] = List[i+1] - List[i];
			if (i==List.size()-2) ListRel[i+1] = List[0] - List[i+1];
		}*/
	}
	else
	{
		List.resize(size);
		fill (List.begin(), List.end(), 0);
	}
	
	if (dev) { cout << " FINAL height table:" << endl; for (int i = 0; i<List.size(); i++) cout << "   #" << i << " " << List[i] << endl; getch(); }
}


void CreateSlopeSpline(path_set &Spline, int size, vector<float>&List, vector<float>&Steps)
{
	bool dev = 0;
	if (Spline.t_corners>0)
	{
		if (dev) cout << " CreateSlopeSpline... Spline Knots: " << Spline.t_corners <<" List size " << size << endl;
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
				if (dev) cout << "   Corner #" << z << " " << Corner.pos << "\tStep" << Corner.step << "\tNext is Gap 90/180 " << Corner.NextIsGap90 << "/" <<Corner.NextIsGap180 << endl;
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
			if (dev) cout << "  hTable #" << i << " " << List[i] << "\tSplineZList["<<j<<"] " << SplineZList[j] << endl;
			j++;
			if (j==SplineZList.size()) j=0;
		}
		
		// create relative height (step) list
		/*for (int i = 0; i<List.size(); i++)
		{
			ListRel[i] = List[i+1] - List[i];
		}*/
		
		if (dev) { cout << " height table:" << endl; for (int i = 0; i<List.size(); i++) cout << "   #" << i << " " << List[i]<< endl; getch(); }
	}
	else { if (dev) cout << " No Spline loaded!" << endl; }
}


void CreateSlopeSmooth(float step, int size, vector<float>&List)
{
	bool dev = 0;
	if (step!=0)
	{
		bool NegHeight = 0;
		if (step<0) NegHeight = 1;
		if (NegHeight) step= -step;
		if (size<=1) size = 2;
		if (dev) cout << " CreateSlopeSmooth... size " << size << endl;
		//create source circle
		// circle segment
		float height_max = step * size;
		float height_half = (step * size)/2;
		float chord = 4*height_half;
		float rad = (  (4*pow(height_half,2)) + pow(chord,2)  ) / (8 * height_half);
		float x_step = step*2;
		if (dev) cout << " height_max" << height_max << " height_half" << height_half << " chord " << chord << " rad" << rad << " x_step" << x_step << endl;
		
		List.resize(size);
		for (int i = 0; i<size; i++)
			List[i] = height_max;
		
		if (dev) cout << " Fill first Half..." << endl;
		// Fill first Half of the Height Table
		for (int i = 0; i<ceil(size/2.0); i++)
		{
			float x = 0, y = 0, y_fixed = 0;
			x = (height_max/(size/2.0))*(i+1);
			y = GetIsectCircleLine(rad, x);
			y_fixed = rad - y;
			List[i] = y_fixed;
			if (dev) cout << "  i "<<i<< " x " << x << " isect " << y << " y_fixed " << y_fixed << endl;
		}
		if (dev) for (int i = 0; i<List.size(); i++) cout << " HALF height table #" << i << " y " << round(List[i]) << endl;
		
		if (dev) cout << " Fill second Half..." << endl;
		// Fill second Half by subtracting the first Half from the max height
		for (int i = 0, j = List.size()-2; i<ceil(size/2); i++)
		{
			float y = height_max - List[i];
			if (dev) cout << " Height Table #" << i << " j " << j << " of range " << size/2 << " height " << y << " = height_max " << height_max << " - List[j] " << List[j] << endl;
			List[j] = y;
			j--;
		}
		List.insert(List.begin(),0);
		
		if (NegHeight) for(int i=0;i<List.size(); i++) List[i] = -List[i];
		if (dev) { cout << " FINAL height table:" << endl; for (int i = 0; i<List.size(); i++) cout << "   #" << i << " " << List[i] << endl; getch(); }
	}
	else
	{
		List.resize(size);
		for (int i = 0; i<size; i++)
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
	bool dev = 0;
	if(n<0) n=-n;
	if(dev) cout << setprecision(8) << " IsBorderliner " << n << " precision " << prec << endl;
	int d = pow(10, prec);
	if (d==0) d=1;
	
	// n = 5.0001
	int n_round = n; // n_round = 5
	if( n == n_round ) { if(dev)cout << " Number is even! "<<endl<<endl; return 2; } // number is even
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
		if(dev) cout << "   n_deci " << n_deci << endl;
		if(dev) cout << "   temp " << temp << endl;
		if(temp==0) { if(dev)cout << " Number is Borderliner! "<<endl<<endl; return 1; }
		else { if(dev)cout << " Number is NO Borderliner! "<<endl<<endl; return 0; }
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


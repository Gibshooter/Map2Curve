#ifndef __UTILS_H_INCLUDED__
#define __UTILS_H_INCLUDED__

#include "frames.h"
#include "gvector.h"
#include "vertex.h"

#include <string>
#include <vector>

using namespace std;

struct path_set;



/* ===== NUMBER LIST GENERATOR FUNCTIONS ===== */

void CreateSlopeLinear(float height, int size, vector<float>&List, vector<float> &Lengths, int g);
void CreateSlopeRandom(float height, int size, vector<float>&List, int g);
void CreateSlopeSpline(path_set &Spline, int size, vector<float>&List, vector<float>&Steps);
void CreateSlopeEasings(float height, int size, vector<float>&List, vector<float> &Lengths, int g);

/* ===== FILE & STRING HANDLING FUNCTIONS ===== */

bool CheckIfFileExists(string p);
string LoadTextFile(string wadpath);
void GetFileList(string path, vector<string> &list);
int CheckFileType (string p);
void WriteTextToFile(string L_FilePath, string Text);
//void WriteTextToFile(string L_FilePath, ostream &ostr);
void SplitStrAtDelim(string Str, char Delim, vector<string> &Lines);
void SplitString(string value,string delimiter, vector<string> &Target);
bool ContainsInvalids(string Subject, string Valids);
void ReplaceInList(vector<string> &List1, vector<string> &List2, string L1_candidate, string L2_replace);

/* ===== MISC NUMBER FUNCTIONS ===== */

void CheckFixRes(int& reso, int type);
bool IsResValid(int reso);
int reduce(int res);
int GetDeciPlaces(float RawFloat);
bool IsValid(int n);
bool IsValid(float n);
bool IsNULL(double n);
bool CompareFloatDeci(float N1, float N2, int deciplace);
float GetRandInRange(float min, float max);
int IsBorderliner(float n, int prec);
float easing(float x, int mode);

/* ===== SMALL MATH FUNCTIONS ===== */

int n_pow(int a, int expo);
float GetIsectCircleLine(float rad, float x);
float RoundFloatToDeci(float N1, int deci);





#endif

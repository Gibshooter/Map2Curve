#ifndef __UTILS_H_INCLUDED__
#define __UTILS_H_INCLUDED__

#include <string>
#include <vector>

using namespace std;

/* ===== FILE & STRING HANDLING FUNCTIONS ===== */

bool CheckIfFileExists(string p);
string LoadTextFile(string wadpath);
void GetFileList(string path, vector<string> &list);
int CheckFileType (string p);
string GetValue(string text, int start_pos);
bool CheckPhrase(string text, int pos);
string GetCustomPhraseValue(string text, string phrase);
void GetSettings(string cfgstr, vector<string> &SettingList, vector<string> &lslist);
void WriteTextToFile(string L_FilePath, string Text);
//void WriteTextToFile(string L_FilePath, ostream &ostr);
void SplitStrAtDelim(string Str, char Delim, vector<string> &Lines);
void SplitString(string value,string delimiter, vector<string> &Target);
bool ContainsInvalids(string Subject, string Valids);

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

/* ===== SMALL MATH FUNCTIONS ===== */

int n_pow(int a, int expo);
float GetIsectCircleLine(float rad, float x);
float RoundFloatToDeci(float N1, int deci);


#endif

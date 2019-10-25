#ifndef __WAD3_H_INCLUDED__
#define __WAD3_H_INCLUDED__

#include <string>
#include <vector>

// WAD 3 texture file support

using namespace std;

struct WADTexture
{
	string szName;
	int nWidth = 0;
	int nHeight = 0;
};

struct WADEntry
{
	string szName;
	int nFilePos = 0;
	char nType = 0;
};

struct WADFile
{
	string FilePath;
	int EntryOffset = 0;
	int t_entries = 0;
	int t_tex = 0;
	vector<WADEntry> Entries;
	vector<WADTexture> Textures;
	bool valid = 1;
	
	int FindTexture(string TexName);
	void GetTexInfo(int tID, int &w, int &h);
	void ReadWADFile(string path);
	void GetHeader(ifstream &WADFile);
	void GetEntries(ifstream &WADFile);
	void GetTextures(ifstream &WADFile);
	
	WADFile() {}
	WADFile(string filepath);
	~WADFile() {};
};


#endif

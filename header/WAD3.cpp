
#include "WAD3.h"
#include <iostream>
#include <string>
#include <fstream>

// WAD 3 texture file support

using namespace std;

int WADFile::FindTexture(string TexName)
{
	for (int i=0; i<Textures.size(); i++)
	{
		WADTexture &cTex = Textures[i];
		string szNameUC = cTex.szName;
		for (int i=0;i<szNameUC.length();i++) szNameUC[i] = toupper(szNameUC[i]);
		if (szNameUC==TexName)
			return i;
	}
	return -1;
}

void WADFile::GetTexInfo(int tID, int &w, int &h)
{
	if (tID<Textures.size()) {
		w = Textures[tID].nWidth;
		h = Textures[tID].nHeight;
	}
}

void WADFile::ReadWADFile(string path)
{
	ifstream WADFile;
	WADFile.open(path, ios::in | ios::binary);
	
	if (WADFile.is_open())
	{
		int size = 0; 
		WADFile.seekg(0, ios::end);
		size = WADFile.tellg();
		
		GetHeader(WADFile);
		GetEntries(WADFile);
		GetTextures(WADFile);
		
		WADFile.close();
		
		FilePath = path;
	}
	else
	{
		valid = 0;
		cout << "|    [WARNING] There was a problem opening a WAD file ("<<path<<")!" << endl;
	}
}

void WADFile::GetHeader(ifstream &WADFile)
{
	WADFile.seekg(0, ios::beg);
	char szMagic[4];
	WADFile.read(szMagic,4);
	
	if (szMagic[0]=='W'&&szMagic[1]=='A'&&szMagic[2]=='D'&&szMagic[3]=='3')
	{
		uint32_t nDir;
		WADFile.read(reinterpret_cast<char *>(&nDir), sizeof(nDir));
		t_entries = nDir;
		
		uint32_t nDirOffset;
		WADFile.read(reinterpret_cast<char *>(&nDirOffset), sizeof(nDirOffset));
		EntryOffset = nDirOffset;
	}
	else
	{
		valid = 0;
	}
}

void WADFile::GetEntries(ifstream &WADFile)
{
	WADFile.seekg(EntryOffset, ios::beg);
	
	for (int i=0;i<t_entries;i++)
	{
		WADEntry Current;
		
		// Get File Position
		uint32_t nFilePos;
		WADFile.read(reinterpret_cast<char *>(&nFilePos), 4);
		Current.nFilePos = nFilePos;
		
		// Get File Type
		WADFile.seekg(8, ios::cur);
		char nType;
		WADFile.read(&nType, 1);
		Current.nType = nType;
		
		// Get File Name
		WADFile.seekg(3, ios::cur);
		char c_Name[16];
		WADFile.read(c_Name, 16);
		string str_c_Name(c_Name);
		Current.szName = str_c_Name;
		
		Entries.push_back(Current);
	}
}

void WADFile::GetTextures(ifstream &WADFile)
{
	for (int t=0;t<t_entries;t++)
	{
		WADEntry &Entry = Entries[t];
		if (Entry.nType=='C')
		{
			WADTexture Tex;
			// Get Texture Name (size 16)
			WADFile.seekg(Entry.nFilePos, ios::beg);
			char c_Name[16];
			WADFile.read(c_Name, 16);
			string str_c_Name(c_Name);
			Tex.szName = str_c_Name;
			
			// Get Texture Width (size 4)
			uint32_t nWidth;
			WADFile.read(reinterpret_cast<char *>(&nWidth), 4);
			Tex.nWidth = nWidth;
			
			// Get Texture Height (size 4)
			uint32_t nHeight;
			WADFile.read(reinterpret_cast<char *>(&nHeight), 4);
			Tex.nHeight = nHeight;
			
			Textures.push_back(Tex);
		}
	}
}
	
WADFile::WADFile(string filepath)
{
	ReadWADFile(filepath);
}


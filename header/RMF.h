#ifndef __RMF_H_INCLUDED__
#define __RMF_H_INCLUDED__

#include <string>
#include <vector>
#include <iostream>
#include <fstream>
//#include <stdlib.h>
#include <cstring>

#include "entity.h"
#include "face.h"
#include "gvector.h"
#include "vertex.h"

using namespace std;

/* ===== RMF OBJECT ===== */

struct RMF
{
	ofstream RMFBuffer;
	//ofstream *Stream_Ptr = nullptr;
	
	// File Structure
	void WriteHeader();
	void WriteVisGroups(int n_vis);
	void WriteWorldSpawn(entity &Entity_WS);
	void WritePaths(int n_paths);
	
	// Group
	void WriteGroup(int n_obj, unsigned char color[3]);
	
	// Face
	void WriteFace(face &Face);
	
	// Solid
	void WriteSolid(brush &Brush);
	
	// Entity
	void WritePointEntity(entity &Entity);
	void WriteEntityHeader(entity &Entity);
	void WriteEntityFooter(entity &Entity);
	
	// Primitives
	void UpdateAtPosAndReturn(unsigned long pos_target, int &content);
	unsigned long WriteCounterGetPos();
	
	void WriteByte(int content);
	void WriteByte(float content);
	void WriteByteEmpty(int length);
	void WriteFixedTString(string s);
	void WriteTString(string s);
	void WriteColor(unsigned char r, unsigned char g, unsigned char b);
	void WriteVector(gvector V);
	void WriteVector(vertex V);
	
	void CloseFile();
	
	RMF(string FilePath) {
		RMFBuffer.open(FilePath, ios::out | ios::binary);
		RMFBuffer.seekp(0);
	}
	//RMF(ofstream &file) { Stream_Ptr = &file; }
	~RMF() {}
};



#endif

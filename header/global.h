#ifndef __GLOBAL_H_INCLUDED__
#define __GLOBAL_H_INCLUDED__

#include "file.h"
#include "group.h"
#include "WAD3.h"
#include "settings.h"
#include "vertex.h"
#include "RMF.h"
#include "LSE.h"
#include "teestream.h"

#include <string>
#include <vector>

using namespace std;


void DELETE_ALL_POINTERS();
void DELETE_DTABLE_POINTER();

		

// Global Objects

extern vertex Zero;
extern vector<WADFile> WADFiles;

extern file *gFile; // global pointer for current file

extern ctable *cTable; // final construction Table
extern ctable *sTable; // original imported settings storage
extern ctable *mTable;

extern bool ValidDefaults;
extern ctable *dTable; // Defaults construction Table

extern group *mGroup; 	// Imported map
extern group *sGroup; 	// Transformed map (1 for each curve object)
extern group *bGroup; 	// Generated curves

extern group *mDetailGroup;	// original detail objects
extern group_set *sDetailSet; // copied detail objects for each individual curve object
extern group_set *DetailSet; // final detail objects

extern string def_nulltex;
extern float def_spikesize;

extern bool G_LOG;
extern bool G_AUTOCLOSE;
extern bool G_DEV;

extern string ROOT;

extern int ErrorCode;

extern teestream *tee_ptr;






#endif

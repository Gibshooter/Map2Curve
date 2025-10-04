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


// Global Objects

vertex Zero;
vector<WADFile> WADFiles;

file *gFile = nullptr; // global pointer for current file

ctable *cTable = nullptr; // final construction Table
ctable *sTable = nullptr; // original imported settings storage
ctable *mTable = nullptr;

bool ValidDefaults = 1;
ctable *dTable = nullptr; // Defaults construction Table

group *mGroup = nullptr; 	// Imported map
group *sGroup = nullptr; 	// Transformed map (1 for each curve object)
group *bGroup = nullptr; 	// Generated curves

group *mDetailGroup = nullptr;	// original detail objects
group_set *sDetailSet = nullptr; // copied detail objects for each individual curve object
group_set *DetailSet = nullptr; // final detail objects

string def_nulltex = "SOLIDHINT";
float def_spikesize = 4;

bool G_LOG = 0;
bool G_AUTOCLOSE = 0;
bool G_DEV = 0;

string ROOT = "";

int ErrorCode = 1;

teestream *tee_ptr = nullptr;

void DELETE_ALL_POINTERS()
{
	if (mGroup!=nullptr) {delete mGroup;   mGroup = nullptr; }
	if (bGroup!=nullptr) {delete[] bGroup; bGroup = nullptr; }
	if (sGroup!=nullptr) {delete[] sGroup; sGroup = nullptr; }
	if (cTable!=nullptr) {delete[] cTable; cTable = nullptr; }
	if (sTable!=nullptr) {delete[] sTable; sTable = nullptr; }
	if (mTable!=nullptr) {delete[] mTable; mTable = nullptr; }
	
	if (mDetailGroup!=nullptr) {delete[] mDetailGroup; mDetailGroup = nullptr; }
	if (sDetailSet!=nullptr) {delete[] sDetailSet; sDetailSet = nullptr; }
	if (DetailSet!=nullptr) {delete[] DetailSet; DetailSet = nullptr; }
}



void DELETE_DTABLE_POINTER()
{
	if (dTable!=nullptr) {delete[] dTable; dTable = nullptr; }
}


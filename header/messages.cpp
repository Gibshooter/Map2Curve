#include "messages.h"
#include "WAD3.h"
#include "global.h"

#include "file.h"
#include "group.h"
#include "settings.h"
#include "teestream.h"

#include <string>
#include <vector>
#include <iostream>

using namespace std;

extern file *gFile;
extern vector<string> slist;
extern vector<int> slist_id;
extern vector<int> slist_type;
extern vector<int> slist_min;
extern vector<int> slist_max;
extern ctable* cTable;
extern ctable* dTable; // default Settings consisting of DEFAULTS.txt or internal defaults
extern ctable* sTable;
extern bool G_DEV;
extern bool ValidDefaults;
extern string ROOT;
extern vector<WADFile> WADFiles;
extern group *mGroup;
extern group *sGroup;
extern group *mDetailGroup;
extern int  ErrorCode;
extern ofstream logfile;
extern teestream *tee_ptr;



void WAIT() { system("pause"); }


void MESSENGER(MSG_ID i)
{
	teestream &tee = *tee_ptr;

	switch (i)
	{
		case MSG_INTRO_CREDITS:
		{
								tee << "++---------------------++" << endl;
								tee << "||   Map2Curve v0.87   ||" << endl;
								tee << "||    by Gibshooter    ||" << endl;
								tee << "||     October 2025    ||" << endl;
								tee << "++---------------------++" << endl << endl;
		}
		break;
		
		case MSG_VLINE:			tee << "|" << endl; break;
		case MSG_DLINE:			tee << "+-----------------------------------------------------+" << endl; break;
		
		case MSG_LOAD_WADS____: tee << "            Scanning WAD Folder and WADList.txt..." << endl; break;
		case MSG_WADS__LOADED_:
		{
								tee << "            " << WADFiles.size() << " WAD files loaded:" << endl;
								
								for(int j=0; j<WADFiles.size(); j++)
								tee << "            #" << j+1 << " " << WADFiles[j].FilePath << endl;
								tee << endl;
								tee << "     [INFO] Loading Default Settings (DEFAULTS.txt) in Root Dir..." << endl;
		}
		break;
		case MSG_LOAD_DEFAULTS:
		{
								if (ValidDefaults)
								tee << "            Successfully loaded!" << endl;
								else
								tee << "            Not found or empty. Using internal Defaults!" << endl;
		}
		break;
		
		case MSG_CREAT_SRC_OBJ:	tee << "|  Creating Source Objects..." << endl; break;
		case MSG_CONSTRUCT_TAB:	tee << "|  Creating Construction Tables..." << endl; break;
		case MSG_USING_MAP_INT:	tee << "|  (Using map-internal settings from info_curve and info_curve_export entities)" << endl; break;
		case MSG_USING_DEF_SET:	tee << "|  (Using default settings)" << endl; break;
		case MSG_CREATE_DET_OB:	tee << "|  Creating Source Detail Objects..." << endl; break;
		case MSG_APPLY_SRC_TRA: tee << "|  Applying Source Transformations..." << endl; break;
		case MSG_CARVING_SOURC: tee << "|  Carving Source Map..." << endl; break;
		case MSG_LOAD_SPLINE_F: tee << "|    Loading Spline File...\n"; break;
		case MSG_CREAT_DET_OBJ: tee << "|    Creating Detail Objects...\n"; break;
		case MSG_CREAT_CON_FRM: tee << "|    Creating Construction Framework...\n"; break;
		case MSG_CREAT_CUR_OBJ: tee << "|    Creating Curve Objects...\n"; break;
		case MSG_BUILD_CUR_BRU: tee << "|    Building Curve Brushes...\n"; break;
		case MSG_TEXTURING____: tee << "|    Texturing...\n"; break;
		case MSG_PERFORM_CLEAN: tee << "|    Performing Cleanup...\n"; break;
		case MSG_TRIANGULATING: tee << "|    Triangulating...\n"; break;
		case MSG_SHEARING_TEXT: tee << "|    Shearing Textures...\n"; break;
		case MSG_CREATING_RAMP: tee << "|    Creating Ramp...\n"; break;
		case MSG_FINAL_TRANSFO: tee << "|    Applying Final Transformations...\n"; break;
		case MSG_POSTPROCESSIN: tee << "|    Further Postprocessing...\n"; break;
		case MSG_BOUNDING_BOXE: tee << "|    Creating Bounding Boxes...\n"; break;
		case MSG_DEVELOPER_ASS: tee << "|    Creating Developer Assets...\n|\n"; break; // lel
		case MSG_EXPORTING_MAP: tee << "|  Exporting selected data to MAP file:\n"; break;
		case MSG_EXPORTING_RMF: tee << "|  Exporting selected data to RMF file:\n"; break;
		case MSG_EXPORTING_OBJ: tee << "|  Exporting selected data to OBJ file(s)...\n"; break;
		case MSG_NFO_INTPRESET:
		{
								tee << "|    [INFO] Using preset-entity (info_curve) from map-file!" << endl;
								tee << "|" << endl;
		}
		break;
		
		// ERRORS
		case MSG_ERROR_CODE___:	tee << " ############## ErrorCode " << ErrorCode << "  ############## " <<endl; break;
		case MSG_ERR_BRCONFAIL:	tee << "|    [ERROR] There was a problem reconstructing a Brush, skipping..." << endl; break;
		case MSG_SPLINE_ERR_SK:	tee << "|    [ERROR] Spline only contains single knots!" << endl; break;
		case MSG_ERR_INVALID_S:
		{
								tee << "|    [ERROR] Spline file invalid! Aborting..."<< endl;
								WAIT();
		}
		break;
		case MSG_SPLINE_WRN_SK:	tee << "|    [WARNING] Spline contains single knots which had been discarded!" << endl; break;
		
		case MSG_ERR_INVALID_M:
		{
								tee << "|    [ERROR] Something caused the original mesh"<< endl;
								tee << "|            to become invalid! Aborting..." << endl;
								tee << "|" << endl;
								/*tee << "|            Transformations were:" << endl;
								tee << "|            - offset   " << cTable[g].offset << endl;
								tee << "|            - rot_src   " << cTable[g].rot_src << endl;
								tee << "|            - scale_src " << cTable[g].scale_src << endl;*/
								tee << "|            For more information abcout valid source Brushes" << endl;
								tee << "|            see the online documentation." << endl;
								tee << "|" << endl;
								WAIT();
		}
		break;
		
		case MSG_ERR_NO_VALIDS:
		{
								tee << "|  [ERROR] Map File doesn't seem to contain valid"<< endl;
								tee << "|          Brushes for Curve-Generation! Aborting..." << endl;
								tee << "|" << endl;
								tee << "|          For more information abcout valid source Brushes" << endl;
								tee << "|          see the online documentation." << endl;
								tee << "|" << endl;
								tee <<	"+-----------------------------------------------------+" << endl << endl;
								WAIT();
		}
		break;
		
		
		
		// WARNINGS
		case MSG_WARN_NOOUTPUT: tee << "|  [WARNING] No coutput format set! Using RMF file format..."<< endl; break;
		case MSG_WARN_HIGHMODE:
		{
								tee << "|    [WARNING] Heightmode set to \"Spline\", but no valid spline file found!" << endl;
								tee << "|              Using linear heightmode instead..."<<endl;
		}
		break;
		
		
		// MISC
		case MSG_NO_INPUT_FILE:
		{
								cout << "This program works by feeding it files.\n";
								cout << "Valid input files are Goldsource-Map (*.map) and Map2Curve Preset-Files (*.txt).\n\n";
								WAIT();
		}
		break;
		
		
		// DEV
		case MSG_PRINT_S_TABLE: for (int g = 0; g<mGroup->t_arcs; g++) {
									if (sTable!=nullptr) {
									tee << " sTable #" << g << endl;
									sTable[g].Print();
									system("pause");} }
		break;
		case MSG_PRINT_C_TABLE: for (int g = 0; g<mGroup->t_arcs; g++) {
									if (sTable!=nullptr) {
									tee << " cTable #" << g << endl;
									cTable[g].Print();
									system("pause");}}
		break;
		
	}
}

void MESSENGER(MSG_ID i, vector<string> str, vector<int> val_i, vector<float> val_f)
{
	teestream &tee = *tee_ptr;

	switch (i)
	{
		case MSG_NFO_CUSTOMSRC:
		{
								tee << "|    [INFO] Custom source file ("<< str[0] <<") found!" << endl;
								tee << "|" << endl;
		}
		break;
		case MSG_ERR_CUSTOMSRC:
		{
								tee << "|    [ERROR] Custom source file ("<< str[0] <<") not found!"<<endl;
								tee << "|            Trying original source file ("<< gFile->path_map <<")..." << endl;
								tee << "|" << endl;
		}
		break;
		case MSG_NFO_NOPRESETF:
		{
								bool b = 1;
								if (val_i[0]==0) b = 0;
								if (!b) {
								tee << "|    [INFO] No matching preset-file ("<< str[0] <<".txt) or entity (info_curve) found in map-file!" << endl;
								tee << "|           Using default settings..." << endl;
								tee << "|" << endl;
								} else {
								tee << "|    [INFO] No preset-entity (info_curve) found in map-file!" << endl;
								tee << "|           Using preset-file ("<<str[1]<<")..." << endl;
								tee << "|" << endl;
								}
		}
		break;
		case MSG_EXPO_FILENAME: tee << "|  " << str[0] << endl; break;
		case MSG_X_VALID_FILES: tee << "     [INFO] Found " << val_i[0] << " valid file(s).\n\n"; break;
		case MSG_CHECKING_FILE: tee << " Checking File #" << val_i[0]+1 << endl; break;
		case MSG_GETI_FILEPATH:
		{
								if (val_i[0]==1) {
								tee << "|    Type: Preset-File (*.txt): \t" << endl;
								tee << "|    Path: " << str[0] << endl;
								} else {
								tee << "|    Type: Map-File (*.map)" << endl;
								tee << "|    Path: " << str[0] << endl;
								}
								tee << "|    " << endl;
		}
		break;
		case MSG_PROCESS_FILE_: tee << "\n  Processing File #"<<val_i[0]+1 << endl; break;
		case MSG_USING_PRESETF:	tee << "|  (Using settings from preset-file " << str[0] << ".txt)" << endl; break;
		case MSG_GENERAT_CURVE:
		{
								tee << "|" << endl;
								tee << "|  +----------------------+" << endl;
								tee << "|  | Generating Curve #"<< left << setw(3)<<val_i[0]<<"|" << endl;
								tee << "|  +----------------------+" << endl;
								tee << "|" << endl;
		}
		break;
		
		case MSG_CURVE_INFO___:
		{
								int t = cTable[val_i[0]].type;
								int hm = cTable[val_i[0]].heightmode;
								int &m = cTable[val_i[0]].mirror;
								int &ms = cTable[val_i[0]].mirror_src;
								string mstr = "None", msstr = "None";
								if (m>0) 					mstr = "";
								if (m==1||m==4||m==5||m==7) mstr += "X ";
								if (m==2||m==4||m==6||m==7) mstr += "Y ";
								if (m==3||m==5||m==6||m==7) mstr += "Z";
								if (ms>0) 					msstr = "";
								if (ms==1||ms==4||ms==5||ms==7) msstr += "X ";
								if (ms==2||ms==4||ms==6||ms==7) msstr += "Y ";
								if (ms==3||ms==5||ms==6||ms==7) msstr += "Z";
								tee << "|    Type:          "; if (t==0) tee << "Pi Circle" << endl; else if (t==1) tee << "Grid Circle" << endl; else if (t==2) tee << "Simple Spline Extrusion" << endl; else if (t==3) tee << "Intersecting Spline Extrusion" << endl;
								tee << "|    Radius:        "<< cTable[val_i[0]].rad << endl;
								tee << "|    Depth:         "<< sGroup[val_i[0]].SizeY << endl;
								tee << "|    Sides:         "<< cTable[val_i[0]].res << endl;
								tee << "|    Height:        "<< cTable[val_i[0]].height << endl;
								tee << "|    Heightmode:    "; if (hm==0) tee << "Linear Slope" << endl; else if (hm==1) tee << "Path-Corner" << endl; else if (hm==2) tee << "Random Jagged" << endl; else tee << "Easings #" << hm << endl;
								tee << "|    Transform SRC: rotate " << cTable[val_i[0]].rot_src << endl;
								tee << "|                   scale  " << cTable[val_i[0]].scale_src << endl;
								tee << "|                   mirror " << msstr << endl;
								tee << "|    Transform:     move   " << cTable[val_i[0]].move << endl;
								tee << "|                   rotate " << cTable[val_i[0]].rot << endl;
								tee << "|                   scale  " << cTable[val_i[0]].scale << endl;
								tee << "|                   mirror " << mstr << endl;
								tee << "|    Range:         Section " << floor(cTable[val_i[0]].res*(cTable[val_i[0]].range_start/100.0))+1 << " - " << floor(cTable[val_i[0]].res*(cTable[val_i[0]].range_end/100.0)) << " of total " << cTable[val_i[0]].res << endl; //
								tee << "|" << endl;
		}
		break;
		
		// SPLINE
		case MSG_SPLINE_LOADED:	tee << "|    [INFO] Spline file #"<<val_i[0]+1<<" ("<<str[0]<<") successfully loaded!"<< endl; break;
		case MSG_SPLINE_INVALD:	tee << "|    [ERROR] Spline file #"<<val_i[0]+1<<" ("<<str[0]<<") contains invalid information!"<< endl; break;
		case MSG_SPLINE_NOTFND:	tee << "|    [ERROR] Spline file #"<<val_i[0]+1<<" ("<<str[0]<<") does NOT exist!" << endl; break;
		case MSG_SPLINE_ERR_IK:	tee << "|    [ERROR] Spline contains invalid knots! Path #"<<val_i[0]<< ", Knot #" << val_i[1] << " (" << val_f[0] << " " << val_f[1] << " " << val_f[2] << "), section has invalid length: " << val_f[3] << endl; break;
		
		// WAD
		case MSG_WAD_FILES_LST:
		{
								int w = str.size();
								tee << "|" <<endl;
								tee << "|    [INFO] " << w << " WAD files are listed in this map file:" << endl;
								for (int i = 0; i<w; i++)
								tee << "|           #" << i+1 << " " << str[i] << endl;
		}
		break;
		case MSG_WAD_FILES_KWN:
		{
		if (val_i[1]==val_i[0]){tee << "|           ALL";}
		else if (val_i[1]==0){	tee << "|           NONE";}
		else{					tee << "|           " << val_i[1];}
								tee << " of them are known by Map2Curve!" << endl;
								tee << "|" <<endl;
		}
		break;
		case MSG_WAD_WRN_OPENW: tee << "|    [WARNING] There was a problem opening a WAD file ("<<str[0]<<")!" << endl; break;
		case MSG_WARN_UNKWNTEX:
		{
								tee << "|    [WARNING] Could not get informations of " << str.size() << " texture(s):" << endl;
								for (int i = 0; i<str.size(); i++)
								tee << "|              " << str[i] << endl;
								tee << "|              Using default width and height (128px)."<< endl;
								tee << "|              Texture Offsets will probably be wrong!" << endl;
								tee << "|" << endl;
		}
		break;
		
		// INFO
		case MSG_INFO_HSTRETCH:
		{
								tee << "|" << endl;
								tee << "|    [INFO] Horizontal Stretch Amount ("<<cTable[val_i[0]].hstretchamt<<") caused strongly distorted texture(s)!" << endl;
								tee << "|           Original Scale was used instead."<<endl;
								tee << "|" << endl;
		}
		break;
		
		case MSG_LOAD_EXT_SETT: tee << "|  Loading Preset File: "<< str[0] <<"...\n"; break;
		case MSG_LOADING_MAP_F:	tee << "|  Loading Map File:    "<< str[0] <<"...\n"; break;
		
		
		// WARNINGS
		case MSG_WARN_CVWALIGN:
		{
								tee << "|    [WARNING] Face #" << val_i[0]+1 << " of Brush #"<<val_i[1]+1<<" (Tex: " << str[0] << ")"<< endl;
								tee << "|              has no valid align for arc generation."<<endl;
								tee << "|              World Align is being applied." << endl;
								tee << "|" << endl;
		}
		break;
		
		
		// ERRORS
		case MSG_ERR_MAPNOTFOU:
		{
								tee << "|            File: "<< str[0] << endl;
								tee << "|    [ERROR] Map-file not found or empty! Aborting..." << endl;
								tee << "|" << endl;
		}
		break;
		case MSG_ERR_CANT_HAND:
		{
								tee << "|    [ERROR] Can't handle this file type! Please use *.txt and *.map files only!\n";
								tee << "|            File: " << str[0] << "\n";
								tee << "|\n";
								tee << "+-----------------------------------------------------+\n\n";
		}
		break;
		case MSG_WARN_INVALBRU:
		{
								tee << "|  [WARNING] Brush ["<<val_i[0]<<"] of Entity ["<<val_i[1]<<"] seems to have an invalid mesh and won't be processed." << endl;
								tee << "|" << endl;
		}
		break;
		case MSG_WARN_HORWRONG:	tee << "|    [WARNING] Something went horribly wrong! The Source Map doesn't contain valid brushes ("<< val_i[0] << ")!" << endl; break;
	}
}







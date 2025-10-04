#include "header/file.h"
#include "header/group.h"
#include "header/WAD3.h"
#include "header/settings.h"
#include "header/utils.h"
#include "header/vertex.h"
#include "header/RMF.h"
#include "header/LSE.h"
#include "header/slist.h"
#include "header/global.h"
#include "header/messages.h"
#include "header/export.h"
#include "header/teestream.h"

#include <iostream>
#include <string>
#include <vector>
#include <time.h>

#define PI 3.14159265
#define DEBUG 0


#include <ostream>
#include <iomanip>
#include <fstream>

using namespace std;





int main(int argc, char *argv[])
{
	#if DEBUG > 0
	bool dev = 0;
	#endif
	
	srand (time(NULL)); // to generate more random numbers with rand()
	
	
	
	// Get the root directory from argv[0]
	string MAIN(argv[0]);
	ROOT = MAIN.substr(  0, MAIN.rfind( '\\', MAIN.length() )+1  );
	

	
	
	// fill settings list ID vector
	slist_id.resize(slist.size());
	for(int i=0; i<slist.size(); i++) slist_id[i] = i;
	
	
	if (argc >= 2)
	{
		
		vector<string> CleanFileList;
		
		/* ======================================== STARTING PARAMETERS ======================================== */
		// look for starting parameters at the beginning
		for (int i = 1; i < argc; i++)
		{
			string C_PARAM = argv[i];
			
			if (C_PARAM[0]=='-') // param is probably a command
			{
				C_PARAM = C_PARAM.substr(1);
				if 		(C_PARAM=="autoclose") 	G_AUTOCLOSE = 1;
				else if (C_PARAM=="log") 		G_LOG 		= 1;
				else if (C_PARAM=="dev") 		G_DEV 		= 1;
			}
			else CleanFileList.push_back(argv[i]); // param is probably a filepath
		}
		/* ... END .......................... */
		
		
		
		
		/* ======================================== LOG FILE ======================================== */
		
		ofstream logFile;
		
		if(G_DEV||G_LOG)
		logFile.open (ROOT+"Map2Curve.log", ofstream::out);
		
		teestream tee(cout, logFile);
		tee_ptr = &tee;
		
		/* .......................................................................................... */
		
		
	
		
		/* ======================================== INPUT FILE CHECK ======================================== */
		
		// check for valid files
		
		int vcount = 0; // valid files counter
		vector<file> Filelist;
		
		/* COUT */ MESSENGER( MSG_INTRO_CREDITS );
		
		for (int i = 0; i<CleanFileList.size(); i++)
		{
			/* COUT */ MESSENGER( MSG_CHECKING_FILE, vector<string>{}, vector<int>{i}, vector<float>{} );
			/* COUT */ MESSENGER( MSG_DLINE );
			
			string &cFile = CleanFileList[i];
			
			//check if filetype is valid (txt and map)
			int filetype = CheckFileType(cFile);
			
			if (filetype==1 || filetype==2)
			{
				Filelist.push_back(cFile);
				Filelist[vcount].type = filetype;
				Filelist[vcount].fID = i;
				Filelist[vcount].GetInfo();
				
				if (!Filelist[vcount].valid_map)
					Filelist.pop_back();
				else
					vcount++;
				
				/* COUT */ MESSENGER( MSG_DLINE );
			}
			else
			{
				// ERROR: Can not handle this filetype
				/* COUT */ MESSENGER( MSG_ERR_CANT_HAND, vector<string>{cFile}, vector<int>{}, vector<float>{} );
			}
		}
		
		
		/* ... END .......................... */
		
		
		
		
		
		
		/* =================================== LOAD WADS AND DEFAULT SETTINGS =================================== */
		
		
		if (Filelist.size()>0)
		{
			/* COUT */ MESSENGER( MSG_X_VALID_FILES, vector<string>{}, vector<int>{int(Filelist.size())}, vector<float>{} );
			/* COUT */ MESSENGER( MSG_LOAD_WADS____ );
			
			LoadWads();
			
			/* COUT */ MESSENGER( MSG_WADS__LOADED_ );
			
			LoadDefaultSettings();
			
			/* COUT */ MESSENGER( MSG_LOAD_DEFAULTS );
		}
		
		/* ... END .......................... */
		
		int t_valids = 0;

			


		/* =========================== INPUT FILE PROCESS LOOP =========================== */
		
		// when there are multiple files forwarded to m2c.exe
		
		for (int i = 0; i < Filelist.size(); i++)
		{
			file &cFile = Filelist[i];
			gFile = &Filelist[i];
			
			
			
			/* COUT */ MESSENGER( MSG_PROCESS_FILE_, vector<string>{}, vector<int>{i}, vector<float>{} );
			
			/* COUT */ MESSENGER( MSG_DLINE );
			
			if ( cFile.valid_cfg && !cFile.InternalMapSettings )
			{
				/* COUT */ MESSENGER( MSG_LOAD_EXT_SETT, vector<string>{cFile.path_cfg}, vector<int>{}, vector<float>{} );
				
				GetSettings( cFile.str_cfg, cFile.settings, slist );
			}
			
			
			
			
			
			
			
			
			/* =========================== LOAD MAP FILE =========================== */
			
			// read map-file and create copies from it for m2c to further process the maps data
			// this includes separating normal curve brushes from detail object brushes and point entities
			
			/* COUT */ MESSENGER( MSG_LOADING_MAP_F, vector<string>{cFile.path_map}, vector<int>{}, vector<float>{} );
			
			cFile.LoadMap();
			
			/* COUT */ MESSENGER( MSG_VLINE );
			
			/*
			g = curves
			d = detail objects
			b = brushes
			e = brush/point entities
				
			File.LoadMap()
			|
			|-> File.createGroupMap()
			| 	|-> File.LoadMap_GetEntities()				collect all entities from map-file and copy them into File.EntityList
			|	|	|-> GetInternalMapSettings()		
			|	|	|-> mDetailGroup[g]						list of detail groups is being created
			|	|	|-> File.EntityList[e].CreateBrushes()	create Brushes for all entities found in map string (World and Solid)
			|	|	|	|-> EntityList[e].Brushes[b]
			|	|	|	|-> EntityList[e].Brushes[b].Reconstruct2025()
			|	|	|
			|	|	|-> mDetailGroup[g].Entities[e]			point entities for detail objects are being created
			|	|	|-> mDetailGroup[g].Entities[e].CopySimple(File.EntityList[e])
			|	|
			| 	|-> mGroup = new group						create and fill __RAW BRUSH GROUP__ from File.EntityList
			| 	|-> mGroup->GetGroupDimensions()
			|
			|-> File.LoadMap_GetTexInfo()					get dimensions (width and height) of all textures that were used in the source map
			|-> File.LoadMap_DetailObj()					Create Detail Objects from File.EntityList
			| 	|-> EntityList.Brushes.GetFaceShifts()		get original Texture Shifts of only the detail brushes
			|	|	|-> GetBaseEdges()
			|	|	|-> GetBaseShift()
			|	|	|-> GetTexOffset()
			|	|
			|	|-> mDetailGroup = new group[g]				create and fill __RAW DETAIL OBJECT GROUPS__ from File.EntityList
			|	|-> mDetailGroup[g].MarkGroupOriginObjects()
			|	|-> mDetailGroup[g].GetGroupDimensions()
			|
			|-> mGroup->CheckBrushValidity()				can brushes actually be turned into curves? gets base and head (Left-Right) face ID too which is necessary for GetBrushFaceOrients()
			|-> mGroup->GetBrushFaceOrients()				determine orientation of body faces (Up-Down-Back-Front)
			|-> mGroup->GetBrushTVecAligns()				see if texture vectors are aligned correctly and check their orientation
			|-> mGroup->ReconstructMap()					get certain brush information for further calculations
			| 	|-> mGroup->GetGroupVertexList()
			|	 	|-> Group.GetBrushSimpleCentroid()
			|	 	|-> Group.ClearBrushVertexList()
			|	 	|-> Group.GetBrushVertexAngles()
			|	 	|-> Group.GetBrushFaceVertexSE()
			|	 	|-> Group.GetBrushVertexListSE()
			|	 	|-> Group.GetGroupBrushVertexList()
			|
			|->	mGroup->CheckBrushDivisibility()			check whether a brush can be triangulated or not
			|->	mGroup->GetBrushShifts()					get original texture shifts
			| 	|-> GetBaseEdges()							gets BaseX(2) and BaseY(2) of a face for GetBaseShift
			|	|-> GetBaseShift()							gets base shift of a face
			| 	|-> GetTexOffset()							gets added on top of the base shift
			|
			|-> File.LoadMap_ConvertWorld2Face()
			*/
			



	
		



			
			if (mGroup->valid)
			{
				/* =========================== CREATE __WIP BRUSH GROUPS__ FROM PREVIOUSLY LOADED __RAW BRUSH GROUP__ =========================== */
				
				// copy map brushes for each curve object in case there are individual transformations
				
				/* COUT */ MESSENGER( MSG_CREAT_SRC_OBJ );
				
				cFile.createGroupSource();
				
				/*
				g = curves
				d = detail objects
				b = brushes
				e = brush/point entities
				
				File.createGroupSource()
				|
				|-> sGroup 			= new group[g]			create and fill __WIP BRUSH GROUPS__
				|-> sGroup[g].Copy	(mGroup)
				*/
	
				
				
				
				
				/* =========================== CREATE CONSTRUCTION TABLES =========================== */
				
				// ...WITH RAW CURVE SETTINGS
				
				/* COUT */ MESSENGER( MSG_CONSTRUCT_TAB );
				
				if (	(cFile.type==1 && cFile.valid_cfg) ||
						(cFile.type==2 && cFile.valid_cfg &&
						!cFile.InternalMapSettings ) )
				{
					
					/* COUT */ MESSENGER( MSG_USING_PRESETF, vector<string>{cFile.name}, vector<int>{}, vector<float>{} );
					
					sTable = createTableS(mGroup->t_arcs, cFile.settings,0); // create raw settings table
					
				}
				else if (cFile.type==2 && cFile.InternalMapSettings)
				{
					/* COUT */ MESSENGER( MSG_USING_MAP_INT );
					
					mGroup->t_arcs = cFile.t_iarcs;
					sTable = createTableS(cFile.t_iarcs, cFile.settingsM,1);
					
				}
				else
				{
					/* COUT */ MESSENGER( MSG_USING_DEF_SET );
					
					sTable = new ctable[mGroup->t_arcs];
					for (int i = 0; i<mGroup->t_arcs; i++)
						sTable[i].CopyAll(*dTable);
					
				}
				
				// DEV PRINT STABLE
				if(G_DEV) /* COUT */ MESSENGER( MSG_PRINT_S_TABLE );
				
				// WITH FINAL CURVE SETTINGS
				// CREATE FINAL TABLES WITH CURVE SETTINGS
				createTableC();
				
				// DEV PRINT CTABLE
				if(G_DEV) /* COUT */ MESSENGER( MSG_PRINT_C_TABLE );


				
				
				
				/* =========================== CREATE WIP DETAIL OBJECT GROUPS =========================== */
				
				/* COUT */ MESSENGER( MSG_CREATE_DET_OB );
				
				cFile.createDetailGroupSource();
				
				/* ... END .......................... */
				
				/*
				g = curves
				d = detail objects
				b = brushes
				e = brush/point entities
				
				File.createDetailGroupSource()
				|
				|-> sDetailSet 										= new group_set[g]					create and fill __WIP DETAIL OBJECT GROUPS__
				|-> sDetailSet[g].Groups 							= new group[d]						create and fill __WIP DETAIL OBJECT GROUPS__
				|-> sDetailSet[g].Groups[d].CopyProps				(mDetailGroup[g])					detail group properties
				|-> sDetailSet[g].Groups[d].Brushes[b].Copy			(mDetailGroup[g].Brushes[b])		Solid Brushes of this detail group
				|-> sDetailSet[g].Groups[d].Brushes[b].CopySimple	(mDetailGroup[g].Entities[e])		Point Entities of this detail group
				*/
				

				
				
				// Check if the following functions need to be executed at all
				
				bool 	IsSetSTforms = 0,
						IsSetMap = 0,
						IsSetObj = 0,
						IsSetRMF = 0,
						IsSetMapCarve = 0,
						CarveBefor = 0,
						CarveAfter = 0;
				
				for (int g = 0; g<mGroup->t_arcs; g++)
				{
					if ( ( cTable[g].scale_src.IsSet &&
						   cTable[g].scale_src.x!=0 &&
						   cTable[g].scale_src.x!=1 )
						||
						 ( cTable[g].rot_src.IsSet && (
						   cTable[g].rot_src.x!=0 ||
						   cTable[g].rot_src.y!=0 ||
						   cTable[g].rot_src.z!=0 ) )
						||
						   cTable[g].mirror_src>0
					)										IsSetSTforms = 1;
					if ( cTable[g].mapcarve>0 ) 			IsSetMapCarve = 1;
					if ( cTable[g].mapcarve==1 ) 			CarveBefor = 1;
					if ( cTable[g].mapcarve==2 ) 			CarveAfter = 1;
					if ( cTable[g].map>0 ) 					IsSetMap = 1;
					if ( cTable[g].obj>0 ) 					IsSetObj = 1;
					if ( cTable[g].rmf>0 ) 					IsSetRMF = 1;
				}
				
				
				
				
				
				
				/* =========================== SOURCE TRANSFORMATIONS =========================== */
				
				// CARVING _BEFORE_ SOURCE TRANSFORMATION; new as of v0.87 update
				if ( IsSetMapCarve && CarveBefor )
				{
					/* COUT */ MESSENGER( MSG_CARVING_SOURC );
					
					cFile.CarveSource();
				}
				
				if (IsSetSTforms)
				{
					
					/* COUT */ MESSENGER( MSG_APPLY_SRC_TRA );
					
					cFile.TransformSource();
					
					/*
					g = curves
					d = detail objects
					b = brushes
					e = brush/point entities
					
					File.TransformSource()
					|
					|-> sGroup[g].Brushes[b].ScaleOrigin()						SCALE
					|-> sGroup[g].Brushes[b].RotOrigin()						ROTATE
					|-> sGroup[g].Brushes[b].MirrorOrigin()						MIRROR
					|-> sGroup[g].RoundBrushVertices()
					|
					|-> sDetailSet[g].Groups[d].Brushes[b].ScaleOrigin()		SCALE
					|-> sDetailSet[g].Groups[d].Brushes[b].RotOrigin()			ROTATE
					|-> sDetailSet[g].Groups[d].Brushes[b].MirrorOrigin()		MIRROR
					|
					|-> sDetailSet[g].Groups[d].Entities[e].ScaleOrigin()		SCALE
					|-> sDetailSet[g].Groups[d].Entities[e].RotOrigin()			ROTATE
					|-> sDetailSet[g].Groups[d].Entities[e].RotateEntity()		ROTATE
					|-> sDetailSet[g].Groups[d].Entities[e].MirrorOrigin()		MIRROR
					|
					|-> sDetailSet[g].Groups[d].CarveGroup()					CARVE
					|-> sDetailSet[g].Groups[d].CleanUpGroup()
					|-> sDetailSet[g].Groups[d].GetGroupDimensions()
					|
					|-> sGroup[g].CarveGroup()									CARVE
					|-> sGroup[g].CleanUpGroup()								
					|-> sGroup[g].CheckBrushValidity()							
					|-> sGroup[g].CleanUpGroup()								
					|-> sGroup[g].GetGroupDimensions()							
					|-> sGroup[g].GetBrushFaceOrients()							
					|-> sGroup[g].GetBrushTVecAligns()							
					|-> sGroup[g].GetGroupVertexList()							
					|-> sGroup[g].CheckBrushDivisibility()						
					*/
					
				}
				
				// CARVING _AFTER_ SOURCE TRANSFORMATION; new as of v0.87 update
				if ( IsSetMapCarve && CarveAfter )
				{
					/* COUT */ MESSENGER( MSG_CARVING_SOURC );
					
					cFile.CarveSource();
				}
								
				// create tables with final curve settings again because due to source transformations some values might have changed
				if ( IsSetSTforms || IsSetMapCarve )
				{
					createTableC();
				}
				
				/* COUT */ MESSENGER( MSG_VLINE );
				
				
				// fix x position of detail objects after eventual source transformation
				cFile.FixDetailPos();
			
			
				/* ... END .......................... */
				
				
				
				
				
				
				
				/* =========================== FINAL CURVE LOOP =========================== */
				
				int validGroups = mGroup->t_arcs;
				
				for (int g = 0; g<mGroup->t_arcs; g++)
				{
					group &Group = sGroup[g];
					
					/* COUT */ MESSENGER( MSG_GENERAT_CURVE, vector<string>{}, vector<int>{g+1}, vector<float>{} );
					
					
					
					/* =========================== LOAD SPLINE FILES =========================== */
					
					if (  cTable[g].type==3 || cTable[g].type==2  || cTable[g].heightmode==1  )
					{
						/* COUT */ MESSENGER( MSG_LOAD_SPLINE_F );
						
						cFile.LoadSpline(g);
						
						if (!Group.valid) { validGroups--; }
					}
					
					/* ... END .......................... */
					
					
					
					
					
					
					/* =========================== GENERATE CURVE =========================== */
					
					bool InvButDet = 0; if(sDetailSet!=nullptr || mDetailGroup!=nullptr) InvButDet = 1; // Overwrite for when main group has become invalid/empty but there are still detail objects to process
					
					if (Group.valid || InvButDet)
					{
						def_nulltex = cTable[g].nulltex;
						for (int i=0;i<def_nulltex.length();i++) def_nulltex[i] = toupper(def_nulltex[i]);
						def_spikesize = cTable[g].spike_height;
						
						
						// CURVE INFO
						/* COUT */ MESSENGER( MSG_CURVE_INFO___, vector<string>{}, vector<int>{g}, vector<float>{} );
						
						
						// -----> CREATE FINAL DETAIL OBJECTS <-------
						if (Group.valid || InvButDet)
						{
							/* COUT */ MESSENGER( MSG_CREAT_DET_OBJ);
							cFile.createDetailGroup(g);
						}
						
						/*
						g = curves
						d = detail objects
						b = brushes
						e = brush/point entities
				
						File.createDetailGroup()
						|
						|-> DetailSet 										= new group_set[g]
						|-> DetailSet[g].Groups 							= new group[d]
						|-> DetailSet[g].Groups[d].CopyProps				(sDetailSet[g].Groups[d])
						|-> DetailSet[g].Groups[d].Brushes[b].Copy			(sDetailSet[g].Groups[d].Brushes[b])
						|-> DetailSet[g].Groups[d].Entities[e].CopySimple	(sDetailSet[g].Groups[d].Entities[e])
						*/
						
						// -----> CREATE CONSTRUCTION FRAMEWORK <-------
						if (Group.valid || InvButDet)
						{
							/* COUT */ MESSENGER( MSG_CREAT_CON_FRM);
							cFile.createFramework(g);
						}
						
						// -----> CREATE EMPTY BRUSHES <-------
						if (Group.valid || InvButDet)
						{
							/* COUT */ MESSENGER( MSG_CREAT_CUR_OBJ);
							cFile.createGroupBrush(g);
						}
						// ------------> BUILD CURVE BRUSHES BASED ON PREVIOUSLY CREATED FRAMEWORK <--------------
						if (Group.valid || InvButDet)
						{
							/* COUT */ MESSENGER( MSG_BUILD_CUR_BRU);
							cFile.buildArcs(g);
						}

						// ADD TEXTURES
						if (Group.valid)
						{
							/* COUT */ MESSENGER( MSG_TEXTURING____);
							cFile.texturize(g);
						}

						// CLEAN UP, e.g. welding vertices
						if (Group.valid)
						{
							/* COUT */ MESSENGER( MSG_PERFORM_CLEAN );
							cFile.WeldVertices(g);
							cFile.FixBorderliner(g);
						}

						// TRIANGULATE BRUSHES
						if (  Group.valid && ( cTable[g].tri>0 || cTable[g].ramp>0 || cTable[g].transit_tri>0 || cTable[g].round>0 || cTable[g].transit_round>0 ) && cTable[g].type!=2  )
						{
							/* COUT */ MESSENGER( MSG_TRIANGULATING );
							cFile.Triangulate(g);
						}

						// SHEAR TEXTURES
						if (  Group.valid && cTable[g].texmode==1  )
						{
							/* COUT */ MESSENGER( MSG_SHEARING_TEXT );
							cFile.ShearVectors(g);
						}

						// CREATE A RAMP
						if (  Group.valid && cTable[g].ramp>0 )
						{
							/* COUT */ MESSENGER( MSG_CREATING_RAMP );
							cFile.RampIt(g);
						}
						
						// FINAL TRANSFORMATION
						if (  ( Group.valid || InvButDet ) && cTable[g].scale.IsSet || cTable[g].move.IsSet || cTable[g].rot.IsSet || cTable[g].mirror>0 )
						{
							/* COUT */ MESSENGER( MSG_FINAL_TRANSFO );
							cFile.TransformFinal(g);
						}
						
						// POST PROCESSING, e.g. vertex snapping
						if (  Group.valid && (cTable[g].round>0 || cTable[g].transit_round>0 || cTable[g].hshiftoffset!=0)  )
						{
							/* COUT */ MESSENGER( MSG_POSTPROCESSIN );
							cFile.postProcessing(g);
						}
						
						// CREATE BOUNDING BOXES
						if (  Group.valid && cTable[g].bound>0  )
						{
							/* COUT */ MESSENGER( MSG_BOUNDING_BOXE );
							cFile.createBounds(g);
						}
						
						// DEVELOPER ASSETS
						if (  Group.valid && G_DEV  )
						{
							/* COUT */ MESSENGER( MSG_DEVELOPER_ASS );
							cFile.CreateDevAssets(g);
						}
						
						// WARNING: NO OUTPUT FORMAT SET
						if( cTable[g].map==0 && cTable[g].obj==0 && cTable[g].rmf==0 )
						{
							cTable[g].rmf=1;
							/* COUT */ MESSENGER( MSG_WARN_NOOUTPUT);
							//ErrorCode = 2;
						}
						
						
						
						if(!Group.valid && !InvButDet) validGroups--;
					}
					else
					{
						// ERROR: INVALID MESH
						if (!Group.ValidMesh)
						{
							/* COUT */ MESSENGER( MSG_ERR_INVALID_M);
						}
						// ERROR: INVALID SPLINE FILE
						if (!Group.ValidSpline)
						{
							/* COUT */ MESSENGER( MSG_ERR_INVALID_S);
						}
						
						
						
						validGroups--;
					}
				}
				
				/* ... END OF CURVE LOOP .......................... */
				
				
				
				
				
				/* COUT */ MESSENGER( MSG_VLINE );
				
				if (validGroups>0) {
					if (IsSetMap) {
						/* COUT */ MESSENGER( MSG_EXPORTING_MAP );
						ExportToMap();
					}
					
					if (IsSetRMF) {
						/* COUT */ MESSENGER( MSG_EXPORTING_RMF );
						ExportToRMF();
					}
					
					if (IsSetObj) {
						/* COUT */ MESSENGER( MSG_EXPORTING_OBJ );
						ExportToObj();
					}
				}
				
				/* COUT */ MESSENGER( MSG_VLINE );
				/* COUT */ MESSENGER( MSG_DLINE );
				
				t_valids = validGroups;
			}
			else
			{
				// ERROR: no valid Brushes
				/* COUT */ MESSENGER( MSG_ERR_NO_VALIDS );
			}
			
			if ( mGroup->valid && t_valids>0 )
			{
				if ( ErrorCode!=2 ) 	ErrorCode = 0;
			}
			else
			{
				if ( ErrorCode==2 ) 	ErrorCode = 1;
			}
			
			
			DELETE_ALL_POINTERS();
			
			
		}
		/* ... END OF INPUT FILE LOOP .......................... */
		
		DELETE_DTABLE_POINTER();
		

		// ERROR CODE MESSAGE
		
		if (G_DEV) /* COUT */ MESSENGER( MSG_ERROR_CODE___ );
		
		
		
		
		// IN CASE OF FURTHER PARAMETERS
		
		if (!G_AUTOCLOSE&&G_LOG) WAIT();
		if(G_DEV||G_LOG) logFile.close();
		
		return ErrorCode;
	}
	else
	{
		// NO INPUT FILE
		// argc == 1
		
		/* COUT */ MESSENGER( MSG_NO_INPUT_FILE );
		
		return 0;
	}
}








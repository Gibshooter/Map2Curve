#include "file.h"
#include "settings.h"
#include "group.h"
#include "vertex.h"
#include "WAD3.h"
#include "utils.h"
#include "face.h"

#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <iomanip> // precision

#define PI 3.14159265
#define DEBUG 0

using namespace std;

//enum keytype;
extern ctable *cTable;
extern ctable *sTable;
extern ctable *dTable;
extern group *mGroup;
extern group *sGroup;
extern group *bGroup;
extern group *mDetailGroup;
extern group_set *sDetailSet;
extern group_set *DetailSet;
extern vector<WADFile> WADFiles;
extern vertex Zero;
extern file *gFile;
extern bool G_DEV;

/* ===== FILE METHODS ===== */

void file::CreateDevAssets(int g)
{
	group &Group = bGroup[g];
	bool UseLongEdge = cTable[g].hshiftsrc;
	
	// Create Brushes from horizontal Source Edges for DEV purposes
	for (int i = 0; i<Group.hSourceFace.size(); i++)
	{
		if (Group.hSourceFace[i]!=nullptr)
		{
			face &FaceL = *Group.hSourceFace[i];
			face &FaceS = *Group.hSourceFace2[i];
			
			gvector &VecL = Group.fGroupVecL[i];
			gvector &VecS = Group.fGroupVecS[i];
			
			VecL.Normalize();
			VecS.Normalize();

			//if(Group.hEdgeLen_temp[i]>0.1 && Group.hEdgeLen_temp2[i]>0.1)
			{
				VecL.mult(Group.hSourceFace[i]->EdgeLenL);
				VecS.mult(Group.hSourceFace2[i]->EdgeLenS);
				brush* VecBrushL = new brush; VecBrushL->SecID = FaceL.Mother->SecID; VecBrushL->name="VecBrushL";
				brush* VecBrushS = new brush; VecBrushS->SecID = FaceS.Mother->SecID; VecBrushS->name="VecBrushS";
				
				if(UseLongEdge) {
				if (Group.hEdgeLen_temp[i]>0.1)		VecBrushL->VecToBrush(VecL, FaceL.Normal, "RED");
				} else {
				if (Group.hEdgeLen_temp2[i]>0.1)	VecBrushS->VecToBrush(VecS, FaceS.Normal, "YELLOW"); }
				
				if(UseLongEdge) {
				if (Group.hEdgeLen_temp[i]>0.1)		Group.DevAssets.push_back(VecBrushL);
				} else {
				if (Group.hEdgeLen_temp2[i]>0.1)	Group.DevAssets.push_back(VecBrushS); }
				
				//cout << " created DEV Asset #" << i << " from VecL " << VecL << " and VecS" << VecS << " Sec " << VecBrushL->SecID << " Mother secID " << FaceL.Mother->SecID << endl;
			}
		}
	}
}


void file::GetInfo() {
	
	// GetInfo gets filename and path from full path
	// check if path is absolute or relative first
	string bracket = "\\";
	int ext_start = fullpath.rfind(".");
	int name_start = fullpath.rfind(bracket)+1;
	
	if (name_start!=-1)	p_path = fullpath.substr(0,name_start);
	else				p_path = "";
	if (ext_start!=-1)	name = fullpath.substr(name_start, ext_start-name_start);
	else				name = fullpath.substr(0, ext_start);
	
    if (type==1) // *.txt file
	{
		path_cfg = fullpath;
		path_map = p_path+name+".map";
	}
    else if (type==2) // *.map file
    {
		path_cfg = p_path+name+".txt"; //"defaults.txt";
		path_map = fullpath;
    }
 	if (type==1) {
		cout << "|    Type: Preset-File (*.txt): \t" << endl;
		cout << "|    Path: " << fullpath << endl;
	} else {
		cout << "|    Type: Map-File (*.map)" << endl;
		cout << "|    Path: " << fullpath << endl;
	}
	cout << "|    " << endl;
   
	str_cfg = LoadTextFile(path_cfg);
	CheckForCustomSource();
	string str_map_temp = path_map;
	str_map = LoadTextFile(path_map);
	
	if (str_map=="ERR") valid_map = 0;
	
	// check for internal Map settings entity
	if (type==2&&valid_map)
	{
		bool Found_IC = 0; if(str_map.find("\"classname\" \"info_curve\"")!=-1) Found_IC = 1;
		if (Found_IC)
		{
			InternalMapSettings = 1; // internal settings entity "e.g. info_curve" was found
		}
	}
	
	if (str_cfg=="ERR") valid_cfg = 0;
	
	if (!valid_map) {
		cout << "|    [ERROR] Map-file not found or empty! Aborting..." << endl;
		cout << "|            File: "<< str_map_temp << endl;
		cout << "|" << endl;
	}
	
	if (type==2&&!InternalMapSettings)
	{
		if (!valid_cfg) {
			cout << "|    [INFO] No matching preset-file ("<< name <<".txt) or entity (info_curve) found in map-file!" << endl;
			cout << "|           Using default settings..." << endl;
			cout << "|" << endl;
		} else {
			cout << "|    [INFO] No preset-entity (info_curve) found in map-file!" << endl;
			cout << "|           Using preset-file ("<<path_cfg<<")..." << endl;
			cout << "|" << endl;
		}
	}
	else if (type==2&&InternalMapSettings) {
	cout << "|    [INFO] Using preset-entity (info_curve) from map-file!" << endl;
	cout << "|" << endl;}
}


void file::CheckForCustomSource()
{
	// Check for custom source path in settings file
	//cout << "  Check for custom source path in settings file..." << endl; 
	if (str_cfg!="ERR")
	{
		string phrase = "source";
		string value = "";
		int f_pos = 0;
		
		while (f_pos!=-1)
		{
			//cout << "    f_pos: " << f_pos << " find phrase ";
			f_pos = str_cfg.find(phrase, f_pos);
			//cout << f_pos << endl;
			
			if (CheckPhrase(str_cfg, f_pos))
			{
				//cout << "      Phrase at pos " << f_pos << " is NOT commented out!" << endl;
				value = GetValue(str_cfg, f_pos+phrase.length());
				//cout << "      value " << value << " file exists " << CheckIfFileExists(value) <<  endl;
				
				/*if (!CheckIfFileExists(value)) {
					value = ROOT + value;
					if (CheckIfFileExists(value)) ISROOTED = 1;
				}*/
				
				if (value!="ERR"&&CheckIfFileExists(value))
				{
					cout << "|    [INFO] Custom source file ("<< value <<") found!" << endl;
					cout << "|" << endl;
					path_map = value;
					break;
				} else {
					cout << "|    [ERROR] Custom source file ("<< value <<") not found!"<<endl;
					cout << "|            Trying original source file ("<<path_map /*name+".map"*/<<")..." << endl;
					cout << "|" << endl;
				}
			}
			f_pos = str_cfg.find(phrase, f_pos+1);
		}
	}
}


void file::texturize(int g)
{
	group &Group = bGroup[g];
	
	if(cTable[g].hstretch>0)
	Group.GroupTexturizeHStretch();
	
	Group.GroupTexturize();
}

void file::createBounds(int g)
{
	// copy ranges to detail group
	for(int d = 0; d<DetailSet[g].t_groups; d++) {
		DetailSet[g].Groups[d].range_end = bGroup[g].range_end;
		DetailSet[g].Groups[d].range_start = bGroup[g].range_start;
	}
	
	// get total dimensions of this arc and create a bounding box afterwards
	bGroup[g].GetGroupDimensions(0,0); // brush group
	DetailSet[g].GetGroupSetDimensions(0); // detail group
	//cout << " bGroup #"<<g<<" Dimensions " << bGroup[g].Dimensions << endl;
	//cout << " dGroup #"<<g<<" Dimensions " << DetailSet[g].Dimensions << endl;
	
	dimensions AllCombined;
	if (t_dgroups>0&&bGroup[g].t_brushes>0) AllCombined = DimensionCombine(bGroup[g].Dimensions, DetailSet[g].Dimensions);
	else if(bGroup[g].t_brushes==0&&t_dgroups>0) AllCombined = DetailSet[g].Dimensions;
	else AllCombined = bGroup[g].Dimensions;
	
	AllCombined.expand(64);
	
	if(cTable[g].bound==1)
	{
		bGroup[g].boundBox = new brush[1]; bGroup[g].boundBox->name="BoundingBox";
		bGroup[g].boundBox->MakeCuboid(AllCombined, "SKIP");
	}
	else if(cTable[g].bound==2)
	{
		AllCombined.xb += 512;
		AllCombined.xs -= 512;
		AllCombined.yb += 512;
		AllCombined.ys -= 512;
		AllCombined.zb += 512;
		
		bGroup[g].boundBox = MakeBoxHollow(AllCombined, 16, "SKY");
		for(int i=0;i<6;i++)
		bGroup[g].boundBox[i].name="BoundingBox";
	}
}

void file::buildArcs(int g)
{
	#if DEBUG > 0
	bool dev = 0;
	if (dev)cout << "Constructing Arcs..."  << endl;
	#endif
	
	// smooth ramps need special height table
	#if DEBUG > 0
	if (dev)cout << "  Creating Height table..."  << endl;
	#endif
	bGroup[g].CreateHeightTable();
	
	// build the curve object from previously created construction framework
	#if DEBUG > 0
	if (dev)cout << "  Building curve object from previously created construction framework..."  << endl;
	#endif
	bGroup[g].Build();
	
	group &Group = sGroup[g];
	for (int b=0; b<bGroup[g].t_brushes; b++)
	{
		brush &Brush = bGroup[g].Brushes[b];

		// check for Brushes with less than 3 valid faces (minimum valid brush is a wedge with 4 faces)
		// this occurs because of the spline generation process
		for (int f = 0, inv=0; f<Brush.t_faces; f++)
		{
			face &Face = Brush.Faces[f];
			if (Face.draw==0) inv++;
			if (f==Brush.t_faces-1&&Brush.t_faces-inv<3) { Brush.draw = 0; Brush.valid = 0; }
		}
	}
	
	#if DEBUG > 0
	if (dev)cout << "  Marking Head Vertices..."  << endl;
	#endif
	bGroup[g].GetHeadVertices();
	
	#if DEBUG > 0
	if (dev)cout << "  Adding Simple Heights to Brushes..."  << endl;
	#endif
	bGroup[g].AddBrushHeights();
	
	
	// for DEV PURPOSES write all coordinates into the faces texture name
	/*for (int b = 0; b<bGroup[g].t_brushes; b++)
	{
		brush &Brush = bGroup[g].Brushes[b];
		
		for (int f = 0; f<Brush.t_faces; f++)
		{
			face &Face = Brush.Faces[f];
			Face.Texture = "v0_x("+to_string(static_cast<int>(Face.Vertices[0].x))+")_y("+to_string(static_cast<int>(Face.Vertices[0].y))+")_";//+to_string(static_cast<int>(Face.Vertices[0].z))+"_";
			Face.Texture += "v1_x("+to_string(static_cast<int>(Face.Vertices[1].x))+")_y("+to_string(static_cast<int>(Face.Vertices[1].y))+")_";//+to_string(static_cast<int>(Face.Vertices[1].z))+"_";
			Face.Texture += "v2_x("+to_string(static_cast<int>(Face.Vertices[2].x))+")_y("+to_string(static_cast<int>(Face.Vertices[2].y))+")";//+to_string(static_cast<int>(Face.Vertices[2].z))+"_";
		}
	}*/
	
	#if DEBUG > 0
	if (dev)cout << "  Getting Body Face Lengths..."  << endl;
	#endif
	bGroup[g].GetBrushBodyFaceLengths();
	
	// get exact centroid of all faces
	#if DEBUG > 0
	if(dev) cout << "  Getting Face Centroids..." << endl;
	#endif
	bGroup[g].GetBrushFaceCentroids();
	
	// Get Section Lengths and source faces for future horizontal texture shifts
	#if DEBUG > 0
	if (dev)cout << "  Getting Hor Lengths..."  << endl;
	#endif
	bGroup[g].GetHorLengths();
	
	// Create middle-sections between the existing curve sections for mapping purposes
	#if DEBUG > 0
	if(dev) cout << "  Creating Gaps..." << endl;
	#endif
	bGroup[g].CreateBrushGaps();
	#if DEBUG > 0
	if(dev) cout << "  Arranging Gaps..." << endl;
	#endif
	bGroup[g].ArrangeGaps();
	
	// Rotate the Texture Vectors of all Brush Faces
	#if DEBUG > 0
	if(dev) cout << "  Rotating Tex Vectors..." << endl;
	#endif
	bGroup[g].RotateVectors();
	
	// GetBaseEdges, Face Normals and Baseshift
	#if DEBUG > 0
	if(dev) cout << "  Getting Base Edges and Shifts..." << endl;
	#endif
	for (int b = 0; b < bGroup[g].t_brushes; b++)
	{
		brush &Brush = bGroup[g].Brushes[b];
		for (int f = 0; f<Brush.t_faces; f++)
		{
			face &Face = Brush.Faces[f];
			
			GetBaseEdges(Face);
			GetBaseShift(Face, 0, 1, 0);
		}
	}
	
	// Rotate and Move Detail Group Objects
	#if DEBUG > 0
	if(dev) cout << "  Rotate and Move Detail Group Objects..." << endl;
	#endif
	TransformDetailObj(g);
}

void file::GetInternalMapSettings()
{
	if (InternalMapSettings)
	{
		// count info_curve entities
		for (int e=0; e<EntityList.size(); e++)
			if (EntityList[e].key_classname=="info_curve") t_iarcs++;
		
		// create mTables
		for (int e=0,id=0; e<EntityList.size(); e++)
		{
			entity &Entity = EntityList[e];
			if (Entity.key_classname=="info_curve") {
				Entity.cID = id;
				Entity.GetIntMapSettings(settingsM);
				id++;
			}
			if (Entity.key_classname=="info_curve_export") {
				Entity.GetIntMapSettings(settingsM);
			}
		}
	} else {
		for (int e=0,id=0; e<EntityList.size(); e++)
		{
			entity &Entity = EntityList[e];
			if (Entity.key_classname=="info_curve_export") {
				Entity.GetIntMapSettings(settings);
			}
		}
	}
}

void file::FixDetailPos()
{
	#if DEBUG > 0
	bool dev = 0;
	#endif
	
	// fix position of basic detail objects
	for (int g = 0; g<mGroup->t_arcs; g++) {
		for (int i = 0; i<t_dgroups; i++)
		{
			#if DEBUG > 0
			if (dev) cout << " Fixing X-axis Pos of detail group " << i << " of arc " << g << endl;
			#endif
			
			group &Group = sDetailSet[g].Groups[i];
			// get offset, if X position of objects origin is not 0
			if (Group.Origin.x!=0)
			{
				#if DEBUG > 0
				if (dev) cout << "   Group Origin is not 0 -> " << Group.Origin << " Moving all brushes ("<<Group.t_brushes<<") and Entities ("<< Group.t_ents <<") by "<<-(Group.Origin.x)<<" on X axis..." << endl;
				#endif
				
				for (int b = 0; b<Group.t_brushes; b++)
				{
					brush &Brush = Group.Brushes[b];
					#if DEBUG > 0
					if (dev) cout << "     Brush " << b << " moving on X by " << -(Group.Origin.x) << endl;
					#endif
					
					Brush.Move(-(Group.Origin.x),0,0,1,g);
				}
				for (int k = 0; k<Group.t_ents; k++)
				{
					entity &Entity = Group.Entities[k];
					
					#if DEBUG > 0
					if (dev) cout << "     Entity " << k << " Origin " << Entity.Origin << " moving on X by " << -(Group.Origin.x);
					#endif
					
					Entity.Origin.x -= Group.Origin.x;
					
					#if DEBUG > 0
					if (dev) cout << " Origin now " << Entity.Origin << endl;
					#endif
				}
			}
			Group.Origin.x = 0;
			
			#if DEBUG > 0
			if (dev) cout << "   New Group Origin is " <<  Group.Origin << endl << endl;
			#endif
		}
	}
	
	#if DEBUG > 0
	if (dev) system("pause");
	#endif
}


// Transform Detail Objects
void file::TransformDetailObj(int g)
{
	#if DEBUG > 0
	bool dev = 0;
	if (dev) cout << " Transform Detail Objects..." << endl;
	#endif
	
	group_set &Set = DetailSet[g];
	
	for (int d = 0; d<Set.t_groups; d++)
	{
		group &dGroup = Set.Groups[d];
		dGroup.FillUnsetKeySettings();
		int res = cTable[g].res;
		float offset = cTable[g].offset_NO; // difference (offset) between new and old (N/O) curve radius (original map position and generated one from rad+offset setting)
		vertex &Origin = dGroup.Origin;
		
		bool L_YAW   	= dGroup.d_autoyaw;
		bool L_PITCH 	= dGroup.d_autopitch;
		bool L_ENABLE 	= dGroup.d_enable;
		bool L_CIRCLE 	= dGroup.d_circlemode;
		bool L_POS_RAND = dGroup.d_pos_rand.x;
		bool L_RZ_RAND 	= dGroup.d_rotz_rand.x;
		bool L_MY_RAND 	= dGroup.d_movey_rand.x;
		bool L_SC_RAND 	= dGroup.d_scale_rand.x;
		
		#if DEBUG > 0
		if (dev) cout << " L_YAW " << L_YAW << " L_PITCH " << L_PITCH << " L_ENABLE " << L_ENABLE << " L_angle " << dGroup.d_pos << endl;
		#endif
		
		if (L_ENABLE&&!L_CIRCLE)
		{
			vector<vertex> OriginN (res);
			vector<float> T_Pos (res);
			vector<float> T_MoveYR (res); // random MoveY
			vector<float> T_Pitch (res);
			vector<float> T_Yaw (res);
			vector<float> T_YawR (res); // random Yaw
			vector<float> T_YawC (res); // Yaw + Random Yaw combined
			vector<gvector> T_Move (res);
			vector<float> Dummy (res); // list of "0" for Rotation
			vector<float> T_Scale (res);
			
			dGroup.GetGroupDimensions(1,1);
			
			#if DEBUG > 0
			if (dev) cout << " ============================================= Detail Group " << d << " of curve " << g <<" Origin " << dGroup.Origin << " ============================================= " << endl;
			#endif
			
			// spline from which to get position vector (xyz location) for this detail group
			circle Spline;
			
			#if DEBUG > 0
			if (dev) cout << " Creating circle for this DGroup..." << endl;
			#endif
			
			float OriginOffY = dGroup.Origin.y + offset;
			
			#if DEBUG > 0
			if (dev) cout << " dGroup.SizeY " << dGroup.SizeY << " dGroup.Origin " << dGroup.Origin << " offset " << offset << " OriginOffY " << OriginOffY << endl;
			#endif
			
			if (cTable[g].type==0)
			{
				Spline.build_circlePi(res, OriginOffY, Origin.z, cTable[g].flatcircle);
				Spline.tverts = res;
			}
			else if (cTable[g].type==1)
			{
				Spline.build_circleGrid(res, OriginOffY, Origin.z);
				Spline.tverts = res;
			}
			else if (cTable[g].type==2)
			{
				Spline.build_pathGrid(g, OriginOffY, Origin.z, PathList[g]);
			}
			else if (cTable[g].type==3)
			{
				Spline.build_pathIntersect(g, OriginOffY, Origin.z, PathList[g]);
			}
			
			Spline.AddHeight(g, PathList);
			if (cTable[g].type==0||cTable[g].type==1)
			Spline.ConvertToSpline(g);
			Spline.GetAngles(g);
			
			#if DEBUG > 0
			if (dev) {
				cout << "   Generated Spline - type " << cTable[g].type << " res " << cTable[g].res << " secs " << bGroup[g].sections << endl;
				for (int v=0;v<Spline.tverts;v++)
				cout << "     v " << v<< Spline.Vertices[v] <<endl;
			}
			#endif
			
			
			// ================ TRANSFORMATION CALCULATION ================
			
				// create individual Origins for each section
				for(int i=0;i<res;i++)
					OriginN[i] = dGroup.Origin;
				
				// get initial position along section axis (equal to x movement)
				if ( L_POS_RAND )
				{
					for(int i=0;i<res;i++) { T_Pos[i] = GetRandInRange(dGroup.d_pos_rand.y,dGroup.d_pos_rand.z); }
				}
				else
				{
					for(int i=0;i<res;i++) { T_Pos[i] = dGroup.d_pos; }
				}
				
				#if DEBUG > 0
				if (dev) for(int i=0;i<res;i++) cout << " T_Pos " << T_Pos[i] << endl;
				#endif
				
				// get random offset on Y axis within given limits
				for(int i=0;i<res;i++) T_MoveYR[i] = 0;
				if ( L_MY_RAND )
				{
					for(int i=0;i<res;i++)
					T_MoveYR[i] = GetRandInRange(dGroup.d_movey_rand.y,dGroup.d_movey_rand.z);
				}
				
				#if DEBUG > 0
				if (dev) for(int i=0;i<res;i++) cout << " T_MoveYR " << T_MoveYR[i] << endl;
				#endif
				
				// get random Scaling within given limits
				for(int i=0;i<res;i++) T_Scale[i] = 1.0;
				if ( L_SC_RAND )
				{
					for(int i=0;i<res;i++)
					T_Scale[i] = GetRandInRange(dGroup.d_scale_rand.y,dGroup.d_scale_rand.z);
				}
				
				#if DEBUG > 0
				if (dev) for(int i=0;i<res;i++) cout << " T_Scale " << T_Scale[i] << endl;
				#endif
				
				// get yaw
				if ( L_YAW )
				{
					for(int i=0, v=0;i<res;i++)
					{
						vertex &V = Spline.Vertices[v];
						if (T_Pos[i]!=0&&T_Pos[i]!=1)
							T_Yaw[i] = V.Yaw;
						else {
							T_Yaw[i] = V.YawB;
						}
						v+=2;
					}
				}
				
				#if DEBUG > 0
				if (dev) for(int i=0;i<res;i++) cout << " T_Yaw " << T_Yaw[i] << endl;
				#endif
				
				// get random Z axis rotation (offset)
				if ( L_RZ_RAND ) {
					float min = 0, max = 0;
					min = dGroup.d_rotz_rand.y;
					max = dGroup.d_rotz_rand.z;
					for(int i=0;i<res;i++) T_YawR[i] = GetRandInRange(min, max);
				}
				
				#if DEBUG > 0
				if (dev) for(int i=0;i<res;i++) cout << " T_YawR " << T_YawR[i] << endl;
				#endif
				
				// combine Yaw and Random Yaw
				for(int i=0;i<res;i++)
					T_YawC[i] = T_Yaw[i] + T_YawR[i];
				
				// get pitch
				if ( ( ( cTable[g].height!=0 && cTable[g].ramp>0 ) || cTable[g].type==2 || cTable[g].type==3) && L_PITCH ){
					for(int i=0, v=0;i<res;i++)
					{
						vertex &V = Spline.Vertices[v];
						T_Pitch[i] = V.Pitch;
						v+=2;
					}
				}
				
				#if DEBUG > 0
				if (dev) for(int i=0;i<res;i++) cout << " T_Pitch " << T_Pitch[i] << endl;
				if (dev) cout << " Spline res " << Spline.tverts << " res " << res << endl;
				#endif
				
				// Get new location (XYZ)
				for(int sec=0,v=0; sec<res; sec++)
				{
					float m = T_Pos[sec];
					vertex &PosA = Spline.Vertices[v];
					vertex &PosB = Spline.Vertices[v+1];
					gvector VecOA = GetVector(Origin, PosA); // Origin -> PosA
					gvector VecAB = GetVector(PosA,PosB); // PosA -> PosB
					
					// section position vector
					gvector VecAC; // PosA -> Pos C (VecAB * Position e.g. 0.5)
					VecAC = VecAB; VecAC.mult(m); if(cTable[g].ramp==0) VecAC.z = 0;
					
					// y movement vector
					gvector VecYR;
					if		(T_Pos[sec]==0.0) 				{ VecYR = Spline.InVec[sec]; }
					else if (T_Pos[sec]==1.0&&sec<res-1) 	{ VecYR = Spline.InVec[sec+1]; }
					else 									{ VecYR = Normalize(VecAB); VecYR.rotate(0,0,90); }
					if		(T_MoveYR[sec]!=0) 				{ VecYR.mult(T_MoveYR[sec]); }
					else 									{ VecYR.set(0); }
					
					#if DEBUG > 0
					if (dev) cout << " sec" << sec << " m " << m << " PosA " << PosA << " PosB " << PosB << " VecOA " << VecOA << " VecAB " << VecAB << " VecAC " << VecAC << " VecYR " << VecYR << endl;
					#endif
					
					OriginN[sec] = Add(  Add(  Add(Origin,VecOA), VecAC  ), VecYR  );
					T_Move[sec] = VecAdd ( VecAdd(VecOA, VecAC), VecYR );
					v+=2;
				}
				
				#if DEBUG > 0
				if (dev) for(int i=0;i<res;i++) cout << " T_Move " << T_Move[i] << " OriginN " << OriginN[i] << endl;
				#endif
			
			// ================ END ================
			// move to new Location
			#if DEBUG > 0
			if (dev) cout << " Moving to new Location..." << endl;
			#endif
			dGroup.MoveSecs(T_Move, 1);
			
			// Add Random Yaw
			#if DEBUG > 0
			if (dev) cout << " Adding Yaw Random..." << endl;
			#endif
			dGroup.RotOriginSecs(Dummy, Dummy, T_YawR, OriginN, 1);
			
			// Add Pitch
			#if DEBUG > 0
			if (dev) cout << " Adding Pitch..." << endl;
			#endif
			dGroup.RotOriginSecs(Dummy, T_Pitch, Dummy, OriginN, 1);
			
			// Add Scale
			#if DEBUG > 0
			if (dev) cout << " Adding Scale..." << endl;
			#endif
			dGroup.ScaleOriginSecs (T_Scale, OriginN);
			
			// Add Yaw
			#if DEBUG > 0
			if (dev) cout << " Adding Yaw..." << endl;
			#endif
			dGroup.RotOriginSecs(Dummy, Dummy, T_Yaw, OriginN, 1);
		}
		else if(L_ENABLE&&L_CIRCLE)
		{
			#if DEBUG > 0
			if(dev) {cout << " dGroup " << dGroup.groupname  << " is Array!" << endl; system("pause");}
			#endif
			
			vector<vertex> OriginN (res);
			vector<float> T_Yaw (res);
			vector<float> Dummy (res); // list of "0" for Rotation
			vector<gvector> T_Move (res);
			
			// Get new location (XYZ)
			for(int i=0; i<res; i++)
			{
				gvector &Move = T_Move[i];
				Move.y = cTable[g].offset;
			}
			
			// create individual Origins for each section
			for(int i=0;i<res;i++)
				OriginN[i] = Zero;
			
			// get yaw
			float step = 360.0/res;
			float step_half = step/2;
			if(cTable[g].flatcircle==1) step_half = 0;
			
			for(int i=0;i<res;i++)
			{
				T_Yaw[i] = -(i*step)-step_half;
			}
			
			#if DEBUG > 0
			if(dev) for(int i=0;i<res;i++) cout << " T_Yaw " << T_Yaw[i] << endl;
			#endif
			
			// Add OffsetY
			dGroup.MoveSecs(T_Move, 1);
			
			// Add Yaw
			dGroup.RotOriginSecs(Dummy, Dummy, T_Yaw, OriginN, 1);
		}
	}
	
	// create Intersection Planes
	if(cTable[g].type==0)
	{
		for (int d = 0; d<Set.t_groups; d++)
		{
			group &Group = Set.Groups[d];
			if( Group.d_enable>0 && Group.d_carve>0 )
			Group.CreateIsects();
		}
		
		// Carve Detail Objects
		for (int d = 0; d<Set.t_groups; d++)
		{
			group &Group = Set.Groups[d];
			if( Group.d_enable>0 && Group.d_carve>0 )
			Group.CarveGroupSections();
		}
	}
	
	// print all
	#if DEBUG > 0
	if(dev)
	for (int d = 0; d<Set.t_groups; d++)
	{
		group &dGroup = Set.Groups[d];
		cout << dGroup;
	}
	if(dev) system("pause");
	#endif
}

void file::createGroupBrush(int g)
{
	#if DEBUG > 0
	bool dev = 0;
	if (dev) cout << "Now creating Brush Groups..."  << endl;
	#endif
	
	// create X Groups of Brushes, Faces and Vertices
	int &t_arcs = mGroup->t_arcs;
	if (bGroup==nullptr) bGroup = new group[t_arcs]; // NEW BRUSH GROUPS - Amount [X] (Total Arcs)
	
	// create brushes from loaded Path edges for DEV Purposes
	if(G_DEV&&gFile->PathList.size()>0)
	gFile->PathList[g].PathToDevAssets();
	
	#if DEBUG > 0
	if (dev) cout << "Created " << t_arcs << " new Brush Groups!" << endl;
	#endif
	
	group &Group = sGroup[g];

	#if DEBUG > 0
	if (dev) cout << "  Entering Arc #" << g << "..." << endl;
	#endif

	bGroup[g].gID = g;
	bGroup[g].t_brushes = Group.t_brushes*cTable[g].res; // old(2019-05-27) (mGroup->t_brushes-mGroup->invalids)*cTable[g].res;
	bGroup[g].Brushes = new brush[bGroup[g].t_brushes]; 		// NEW BRUSHES - Amount: [X*Y] (total Map Brushes * Arc Resolution)

	#if DEBUG > 0
	if (dev) cout << "  Created " << Group.t_brushes*cTable[g].res << " new Brushes." << endl;
	#endif

	bGroup[g].sections = cTable[g].res;
	bGroup[g].SecBaseFace = new face*[cTable[g].res];
	for (int i = 0; i<cTable[g].res; i++) bGroup[g].SecBaseFace[i]=nullptr;
	bGroup[g].segments = Group.t_brushes;
	int res = bGroup[g].sections;
	
	bGroup[g].range_start = floor(bGroup[g].sections*(cTable[g].range_start/100.0));
	bGroup[g].range_end   = floor(bGroup[g].sections*(cTable[g].range_end/100.0));
	
	// original map brush loop (x = amount of imported map brushes)
	for (int o = 0, seg2=0, b = 0; o<Group.t_brushes; o++)
	{
		#if DEBUG > 0
		if (dev) cout << "    Entering map Brush #" << o << "..." << endl;
		#endif
		
		brush &OBrush = Group.Brushes[o];
		int Ofcount = OBrush.t_faces;
		if (OBrush.valid)
		{
			// generated brush loop (x = res)
			for (int r = 0; r<res; r++)
			{
				#if DEBUG > 0
				if (dev) cout << "      Entering generated Brush #" << b << "..." << endl;
				#endif
				
				brush &Brush = bGroup[g].Brushes[b];
				
				// NEW FACES - Amount: [X] (amount of faces of original brush)
				Brush.Faces  = new face[Ofcount];
				Brush.t_faces = Ofcount;
				Brush.IsDivisible = OBrush.IsDivisible;
				Brush.entID = OBrush.entID;

				#if DEBUG > 0
				if (dev) cout << "      Created " << Ofcount << "new faces for this Brush." << endl;
				#endif
				
				Brush.SecID = r;
				Brush.SegID = o;
				Brush.SegID2 = seg2;
				Brush.name = "[BRUSH]SEC" + to_string(r) +"/"+ to_string(cTable[g].res) + "SEG" + to_string(o) + "FC" + to_string(Ofcount);
				Brush.gID = g;
				
				#if DEBUG > 0
				if (dev) cout << "  Brush #"<<b<< ", Seg #"<<Brush.SegID<< ", Sec #" << Brush.SecID << ", Tfaces: " << Brush.t_faces << ", Ofcount: " << Ofcount<< endl;
				#endif
				
				// face loop (x = faces per brush)
				for (int f = 0; f<Brush.t_faces; f++)
				{
					#if DEBUG > 0
					if (dev) cout << "        Entering Face #" << f << "..." << endl;
					#endif
					
					face &Face = Brush.Faces[f];
					Face.Mother = &Brush;
					
					if (f==0||f==1)	// base and head faces
					{
						Face.vcount = Ofcount-2;
						Face.Vertices = new vertex[Face.vcount];
						if (f==0) Face.fID=0;
						else Face.fID=1;
						
						#if DEBUG > 0
						if (dev) cout << "        Created " << Face.vcount << " new vertices for this face. Face is BASE/HEAD." << endl;
						#endif
					}
					else // Body faces
					{
						Face.vcount = 4;
						Face.Vertices = new vertex[4];
						Face.fID=2;
						
						#if DEBUG > 0
						if (dev) cout << "        Created " << Face.vcount << " new vertices for this face. Face is BODY." << endl;
						#endif
					}
					Face.name = "[FACE]B"+to_string(b)+"F"+to_string(f)+"/"+to_string(Brush.t_faces)+"FID"+to_string(Face.fID)+"VC"+to_string(Face.vcount);
				}
				b++;
			}
			seg2++;
		}
	}
	
	#if DEBUG > 0
	if (dev) system("pause");
	#endif
}

void file::LoadSpline(int g)
{
	#if DEBUG > 0
	bool dev = 0;
	#endif
	
	int &t_arcs = mGroup->t_arcs;
	if (PathList.size()==0)
		PathList.resize(t_arcs);
	
	#if DEBUG > 0
	if (dev) cout << "Loading Path #"<<g+1<<"..." << endl; if (dev) system("pause");
	if (dev) cout << "  Scanning map dir for valid path file ("<<cTable[g].path<<")..." << endl;
	#endif
	
	// scan map dir for valid path files
	// path file convention: mapfilename_path1.map
	string filename;
	if (cTable[g].path!="UNSET")
	{
		filename = cTable[g].path;
		if (filename[0]=='\"')
		{
			filename.erase(0,1);
			filename.erase(filename.length()-1,1);
		}
	}
	else
	{
		filename = p_path + name + "_spline" + to_string(g+1) +".map";
		if(!CheckIfFileExists(filename)&&g>0&&cTable[0].path!="UNSET")
		filename = cTable[0].path;
	}
	
	#if DEBUG > 0
	if (dev) cout << "  Spline-File " << filename << " is Valid? ";
	#endif
	
	str_path = LoadTextFile(filename);
	
	#if DEBUG > 0
	if (dev) if(str_path=="ERR") cout << " NO!" << endl; else cout << " YES!" << endl; 
	#endif
	
	if (str_path!="ERR")
	{
		cTable[g].path = filename;
		
		#if DEBUG > 0
		if (dev) cout << "  Spline file " << filename << " exists and is now being interpreted..." << endl;
		#endif
		
		ParseCornerFile(str_path, PathList[g]);
		PathList[g].gID = g;
		PathList[g].preverse = cTable[g].preverse;
		PathList[g].cornerFix = cTable[g].cornerfix;
		PathList[g].type = cTable[g].type;
		if (cTable[g].p_scale.IsSet)
		PathList[g].Scale(cTable[g].p_scale);
		PathList[g].Analyze();
		
		if (PathList[g].valid) {
			cout << "|    [INFO] Spline file #"<<g+1<<" ("<<filename<<") successfully loaded!"<< endl;
			if (cTable[g].type==2)
			{
				cTable[g].res = PathList[g].t_corners-PathList[g].t_paths+PathList[g].Gaps;
			}
			else if  (cTable[g].type==3)
			{
				cTable[g].res = PathList[g].CountSections();
			}
			
			#if DEBUG > 0
			if (dev) cout << " Path res now " << cTable[g].res << " (if type==2: total corners "<<PathList[g].t_corners<< " - total paths "<<PathList[g].t_paths<<" + Gaps "<<PathList[g].Gaps<<")" << endl;if (dev)  system("pause");
			#endif
			
		} else {
			cout << "|    [ERROR] Spline file #"<<g+1<<" ("<<filename<<") contains invalid information!"<< endl;
			PathList[g].valid = 0;
			sGroup[g].ValidSpline = 0;
			cTable[g].heightmode = 0;
			if (  cTable[g].type==3 || cTable[g].type==2  )
			sGroup[g].valid = 0;
		}
	}
	else
	{
		cout << "|    [ERROR] Spline file #"<<g+1<<" ("<<filename<<") does NOT exist!" << endl;
		PathList[g].valid = 0;
		sGroup[g].ValidSpline = 0;
		cTable[g].heightmode = 0;
		if (  cTable[g].type==3 || cTable[g].type==2  )
		sGroup[g].valid = 0;
	}
	
	#if DEBUG > 0
	if(dev)system("pause");
	#endif
}


void file::postProcessing(int g)
{
	if (cTable[g].round==0&&cTable[g].transit_round>0)
	bGroup[g].GetTransitVertices();
	
	if (cTable[g].round>0||cTable[g].transit_round>0)
	bGroup[g].RoundBrushVertices(0);
	
	if (cTable[g].hshiftoffset!=0)
	bGroup[g].AddCustomShiftOffset();
}

void file::WeldVertices(int g)
{
	// only gap brushes and carved detail brushes have to be welded, because of precision issues
	
	// gaps
	if(cTable[g].gaps==1&&(cTable[g].type==0||cTable[g].type==1))
	bGroup[g].WeldGroupVertices(1);
	
	// detail brushes
	if(t_dgroups>0&&cTable[g].d_carve==1)
	for(int d=0;d<DetailSet[g].t_groups; d++)
		DetailSet[g].Groups[d].WeldGroupVertices(0);
}

void file::FixBorderliner(int g)
{
	#if DEBUG > 0
	bool dev = 0;
	#endif
	
	group &Group = bGroup[g];
	
	// curve brushes
	for(int b=0; b<Group.t_brushes; b++)
	{
		brush &Brush = Group.Brushes[b];
		int sec = Brush.SecID;
		
		#if DEBUG > 0
		if(dev)cout << " Brush sec" << Brush.SecID << " range start " << Group.range_start << " range end " << Group.range_end << endl;
		#endif
		
		if( Group.IsSecInRange(sec) && Brush.draw )
		{
			#if DEBUG > 0
			if(dev)cout << "  Fixing Borderliner! " << endl;
			#endif
			
			Brush.FixBorderliner(2);
			
			if(Brush.Gap!=nullptr)
				Brush.Gap->FixBorderliner(2);
		}
	}
}

// Create a Ramp & adjust Face Texture Vectors accordingly
void file::RampIt(int g)
{
	#if DEBUG > 0
	bool dev = 0;
	#endif
	
	group &Group = bGroup[g];
	
	if (  cTable[g].ramp>0 && ( cTable[g].height!=0 || cTable[g].heightmode==1 )  )
	for (int b=0; b<bGroup[g].t_brushes; b++)
	{
		brush &Brush = bGroup[g].Brushes[b];
		int sec = Brush.SecID;
		
		#if DEBUG > 0
		if (dev) {cout << " creating Ramp of arc " << g << " brush " << b << "/"<<bGroup[g].t_brushes << " sec " << sec<<" type " << " step "<< Brush.step << endl; system("pause");}
		#endif
		
		if (Brush.valid && Brush.draw && !Brush.IsGap && Group.IsSecInRange(sec) )
		{
			#if DEBUG > 0
			if (dev) cout << "  ^^^^^^^^^ creating Ramp of this brush! ^^^^^^^^^" << endl;
			if (dev) cout << " Brush.valid " << Brush.valid << " Brush.draw "<< Brush.draw << " Brush.IsGap " << Brush.IsGap  << " SecInRange(sec) " << Group.IsSecInRange(sec) << endl;
			#endif
			
			if (Brush.Tri==nullptr)
			{
				#if DEBUG > 0
				if (dev) cout << "  Triangulation inactive!" << endl;
				#endif
				
				for (int f = 0; f<Brush.t_faces; f++)
				{
					face &Face = Brush.Faces[f];
					
					#if DEBUG > 0
					//if (dev) cout << " g " << g << " b "<< b << " f " << f << " draw " << Face.draw << " sec " << Brush.SecID << " Wedge2 " << Brush.IsWedge2 << " Step " << Brush.step << endl;
					#endif
					
					if (Face.draw)
					Face.CreateRamp(g, b, f, Brush.SecID, Brush.IsWedge2, Brush.step);
				}
			}
			else
			{
				#if DEBUG > 0
				if (dev) cout << "  Triangulation active! Tri Count " << Brush.t_tri << endl;
				#endif
				
				for (int bt=0; bt<Brush.t_tri; bt++)
				{
					brush &TriBrush = Brush.Tri[bt];
					for (int f = 0; f<TriBrush.t_faces; f++)
					{
						face &Face = TriBrush.Faces[f];
						
						if (Face.draw)
						Face.CreateRamp(g, bt, f, Brush.SecID, TriBrush.IsWedge2, Brush.step);
					}
						
					if (TriBrush.IsSpike)
					TriBrush.RefreshSpikeTents();
				}
			}
		}
	}

	#if DEBUG > 0
	if (dev) cout << " Ramp Created!" << endl;
	#endif
}

void file::Triangulate(int g)
{
	bGroup[g].Triangulate();
}

void file::ShearVectors(int g)
{
	bGroup[g].ShearVectors();
}



void file::createFramework(int g)
{
	#if DEBUG > 0
	bool dev = 0;
	if (dev) cout << "Creating Construction Framework..."<<endl;
	#endif
	
	group &Group = sGroup[g];
	for (int b = 0; b<Group.t_brushes; b++) // brush loop
	{
		brush &Brush = Group.Brushes[b];
		if (Brush.valid)
		{
			Brush.cset = new circleset; // this set of circles will contain one circle per brush body face
			
			int tcircs = Brush.t_faces-2;
			int res = cTable[g].res;
			float rad = cTable[g].rad;
			int type = cTable[g].type;

			#if DEBUG > 0
			if(dev) cout << "Circle Set #" << g << ", total arcs " << mGroup->t_arcs << ", rad: " << cTable[g].rad << ", res: " << cTable[g].res << ", type: " << cTable[g].type << endl;
			#endif
			
			// Every set of circle gets x circles (x=brushfaces-2)
			#if DEBUG > 0
			if(dev) cout << "   Creating " << tcircs << " circles for Brush " << b << endl;
			#endif
			
			Brush.cset->c = new circle[tcircs];
			circleset &Set = *Brush.cset;
			Set.tcircs = tcircs;
			
			#if DEBUG > 0
			if(dev) cout << "   Creating Circle Vertices..." << endl;
			#endif
			
			for (int c = 0; c<tcircs; c++) // circle loop
			{
				#if DEBUG > 0
				if (dev) cout << "     basevertID " << Brush.vlist[c] << endl;
				if (dev) cout << "     smallest Angle " << Brush.Faces[ Brush.vlist[c] ].vAngle_s << endl;
				#endif
				
				circle &Circle = Set.c[c];
				Circle.tverts = res;
				vertex basevert = Brush.Faces[ Brush.vlist[c] ].Vertices[  Brush.Faces[ Brush.vlist[c] ].vAngle_s  ];
				Circle.SrcFace = Brush.vlist[c];
				float yb = Group.Dimensions.yb;
				float ys = Group.Dimensions.ys;
				
				// Every Circle gets x vertices (x=resolution of current arc)
				float indrad = 0.0; // individual rad, based on basevert y pos and new rad
				
				if (rad == yb)
					indrad = basevert.y; // individual rad, based on basevert y pos and new rad
				else
					indrad = basevert.y + (rad - yb); // ^^^^
				
				float height = basevert.z;
				
				#if DEBUG > 0
				if(dev) cout << "circle-loop #" << c  << ", res: " << res<< ", basevert: " << Brush.Faces[ Brush.vlist[c] ].Vertices[ Brush.Faces[Brush.vlist[c]].vAngle_s ] << ", yb: " << yb <<  endl;
				#endif
				
				// generate Circles
				if (type==0)
					Circle.build_circlePi(res,indrad,height,cTable[g].flatcircle);
				else if (type==1)
				{
					//cout << "Attempting to build Grid Circle... (res,rad,height)" << res << "," << indrad << "," << height << endl;
					Circle.build_circleGrid(res,indrad,height);
				}
				else if (type==2)
				{
					#if DEBUG > 0
					if(dev)cout << " building simple spline (type 2) of arc #" << g << " spline " << c << endl;
					if(dev)system("pause");
					#endif
					
					path_set &PSet = PathList[g];
					Circle.build_pathGrid(g, indrad, height, PSet);
					
					#if DEBUG > 0
					if(dev)cout << Circle << endl;
					#endif
				}
				else if (type==3)
				{
					#if DEBUG > 0
					if(dev)cout << " building intersect spline (type 3) of arc #" << g << " spline " << c << endl;
					if(dev)system("pause");
					#endif
					
					path_set &PSet = PathList[g];
					Circle.build_pathIntersect(g, indrad, height, PSet);
				}
				
				#if DEBUG > 0
				if(dev) cout << " Circle " << Circle << endl;
				#endif
			}
		}
	}
}

void file::LoadMap_GetEntities()
{
	#if DEBUG > 0
	bool dev = 0;
	#endif
	
	// count entities in map file and save start pos and Entity ID of each one
	int findEntStart = 0;
	int lastEntPos = 0;
	int eID = 0;
	while (findEntStart!=-1)
	{
		findEntStart = str_map.find("{\n\"classname\"", lastEntPos);
		
		if (findEntStart!=-1)
		{
			entity E;
			E.eID = eID;
			EntityList.push_back(E);
			entity &Entity = EntityList[eID];
			Entity.pos_start = findEntStart;
			lastEntPos = findEntStart+1;
			eID++;
			
			#if DEBUG > 0
			if (dev) cout << " Entity detected - ID " << E.eID << endl;
			#endif
		}
	}
	
	// determine end pos of each entity and save content to new string
	#if DEBUG > 0
	if (dev) cout << " Determine end pos of each entity..." << endl;
	#endif
	
	for (int i = 0, t_ents = EntityList.size(); i<EntityList.size(); i++)
	{
		entity &Entity = EntityList[i];
		if (i==t_ents-1)	Entity.pos_end = str_map.length();
		else				Entity.pos_end = EntityList[i+1].pos_start;
		
		Entity.content = str_map.substr(Entity.pos_start, Entity.pos_end-Entity.pos_start);
	}
	
	// determine entity types
	#if DEBUG > 0
	if (dev) cout << " Determine entity types..." << endl;
	#endif
	
	for (int i = 0; i<EntityList.size(); i++)
	{
		entity &Entity = EntityList[i];
		int &tb = Entity.t_brushes;
		int findPos = 0;
		int lastPos = 0;
		while (findPos!=-1)
		{
			findPos = Entity.content.find("{\n( ", lastPos);
			if (findPos!=-1) {
				lastPos = findPos+1;
				tb++;
				//mGroup->t_brushes++;
			}
		}
		int worldspawn = Entity.content.find("\"worldspawn\"");
		if (worldspawn==-1&&tb==0)
		Entity.type = 2;
		if (worldspawn==-1&&tb>0)
		Entity.type = 1;
		
		if (Entity.type==1) t_solids++;
		
		#if DEBUG > 0
		if (dev) { cout << "   Entity #" << i; if (Entity.type==2) cout << " is a Point Entity!"<<endl; else if (Entity.type==1) cout << " is a Solid Entity!"<<endl; else cout << " is Worldspawn!"<<endl; }
		#endif
	}
	
	// determine internal head end pos
	#if DEBUG > 0
	if (dev) cout << " Determine internal head end pos..." << endl;
	#endif
	for (int i = 0; i<EntityList.size(); i++)
	{
		entity &Entity = EntityList[i];
		
		//int findEndPos = 0;
		if (Entity.type<=1)
		{
			if (Entity.t_brushes>0)
			Entity.head_end = Entity.content.find("\n{",0);
			else
			Entity.head_end = Entity.content.find("\n}",0);
		}
		else if (Entity.type==2)
		{
			Entity.head_end = Entity.content.length();
		}
	}
	
	// check for active d_autoassign command in solid and point entities
	bool PRE_autoassign = 0;
	for (int i = 0; i<EntityList.size(); i++)
	{
		entity &Entity = EntityList[i];
		if (Entity.type>=1)
		{
			int find = Entity.content.find("\"m2c_d_autoassign\" \"");
			if (find!=-1)
			{
				find+=20;
				if(Entity.content[find]=='1') PRE_autoassign = 1;
			}
		}
	}
	
	#if DEBUG > 0
	if (dev) cout << " Check if entity is part of a detail group..." << endl;
	#endif
	// check if entity is part of a detail group
	for (int i = 0, c=0; i<EntityList.size(); i++)
	{
		entity &Entity = EntityList[i];
		#if DEBUG > 0
		if (dev) cout << "   Entity #" << i << " Content-Size " << Entity.content.length() << endl;
		#endif
		
		int find = Entity.content.find("\"m2c_d_group\" \"");
		if (Entity.content.find("info_curve")==-1)
		if (find!=-1) // m2c_d_group indicator found
		{
			find += 15; // put cursor to beginning of value
			int groupname_end = Entity.content.find("\"",find);
			if ( groupname_end!=-1 && groupname_end-find>0 )
			{
				Entity.groupname = Entity.content.substr(find, groupname_end-find);
				Entity.IsDetail = 1;
			}
		} else { // detail group indicator NOT found
			if(d_autoassign==0&&!PRE_autoassign) { // contruction method is "framework" (default)
				if(Entity.type==2) {
					Entity.IsDetail = 1;
					Entity.groupname = "detail_custom0"+to_string(c);
					
					#if DEBUG > 0
					if (dev)cout << " Found empty DetailGroup? Name [" << Entity.groupname << "] type [" << Entity.type << "]" <<endl;
					#endif
					
					c++;
				}
			} else if(d_autoassign==1||PRE_autoassign) { // construction method is "duplication array" = all entities without a groupname will be added to the "array" group
				if(Entity.type==1||Entity.type==2) {
					Entity.IsDetail = 1;
					Entity.groupname = "detail_array";
				}
			}
		}
		
		#if DEBUG > 0
		if (dev)cout << " Found DetailGroup? " << Entity.IsDetail << " Name " << Entity.groupname << " type " << Entity.type <<endl;
		if (dev)cout << " content " << endl << Entity.content << endl << endl;
		#endif
	}
	
	// Get specific Keys and KeyValues of all Solid and Point Entities
	for (int i = 0; i<EntityList.size(); i++)
	{
		entity &Entity = EntityList[i];
		Entity.GetKeyValues();
		Entity.GetKeyValues_M2C();
	}
	
	// Get internal map settings from settings entity info_curve and store them in settingsM variable
	GetInternalMapSettings();
	/*for (int i = 0; i<settingsM.size()-1; i+=2)
		cout << " settingsM #" << i << " "<<settingsM[i] << " = " << settingsM[i+1] << endl;*/
	
	//count total unique dgroups
	vector<string> unique_gnames;
	for (int i = 0; i<EntityList.size(); i++)
	{
		entity &Entity = EntityList[i];
		string gname = Entity.groupname;
		
		// search current groupname in a list. stop when it already exists and do nothing. when end of list is reached and nothing was found, increase total group amount and add groupname to list
		if (unique_gnames.size()==0)
		{
			unique_gnames.push_back(gname);
			t_dgroups++;
		}
		else
		for (int c = 0; c<unique_gnames.size(); c++)
		{
			string &ename = unique_gnames[c];
			
			if (gname==ename)
				break;

			if (c==unique_gnames.size()-1)
			{
				unique_gnames.push_back(gname);
				t_dgroups++;
			}
		}
		
		// count point entities without detail group name as individual detail group
		#if DEBUG > 0
		if (dev)cout << " count point entities without detail group name as individual detail group ..." << endl;
		#endif
		if (Entity.IsDetail && Entity.type==2 && gname.size()==0)
		{
			t_dgroups++;
			
			#if DEBUG > 0
			if (dev)cout << "   found one! Entity #" << i << " class " << Entity.key_classname << endl;
			#endif
		}
	}
	t_dgroups--; // always subtract 1 from total group amount, because empty group names (empty groupname = non-detail brush) dont count, but are included during search too
	
	#if DEBUG > 0
	if (dev) {
	cout << " Total unique detail groups " << t_dgroups << endl;
	for (int i = 0; i<EntityList.size(); i++) {
		entity &Entity = EntityList[i];
		cout << "   #" << i << " " << Entity.groupname << endl;}
	}
	#endif
	
	mDetailGroup = new group[t_dgroups]; // create original detail object group
	#if DEBUG > 0
	if (dev) cout << "Created " << t_dgroups << " detail object groups! " << endl;
	#endif
	
	for (int i = 0; i<t_dgroups; i++)
		mDetailGroup[i].IsSrcMap = 1;
	
	// get total amount of map brushes
	for (int i = 0; i<EntityList.size(); i++)
	{
		entity &Entity = EntityList[i];
		
		if (!Entity.IsDetail)
		mGroup->t_brushes += Entity.t_brushes;
	}
	#if DEBUG > 0
	if (dev) cout << " Total Map Brushes " << mGroup->t_brushes << endl;
	#endif
	
	// create Brushes of each Entity (World and Solid)
	#if DEBUG > 0
	if (dev) cout << " Creating Brushes for "<<EntityList.size()<<" Entities... " << endl;
	#endif
	
	for (int i = 0; i<EntityList.size(); i++)
	{
		entity &Entity = EntityList[i];
		Entity.CreateBrushes();
	}
	
	// assign detail group ID to each entity brush and Entity
	#if DEBUG > 0
	if (dev) cout << "  Assigning detail group ID to each entity brush... " << endl;
	#endif
	
	for (int i = 1; i<t_dgroups+1; i++)
	{
		for (int e = 0; e<EntityList.size(); e++)
		{
			entity &Entity = EntityList[e];
			if (Entity.IsDetail)
			{
				if (Entity.groupname==unique_gnames[i])
				{
					Entity.dID = i-1;
					
					#if DEBUG > 0
					if (dev) cout << " Entity #" << e << " dID " << Entity.dID << " type " << Entity.type << endl;
					#endif
					
					for (int b = 0; b<Entity.t_brushes; b++)
					{
						brush &Brush = Entity.Brushes[b];
						Brush.dID = i-1;
						
						#if DEBUG > 0
						if (dev) cout << "    Brush #" << b << " dID " << Brush.dID << endl;
						#endif
					}
				} else if (Entity.groupname=="") {
					
					Entity.dID = i-1;
					
					#if DEBUG > 0
					if (dev) cout << " Entity #" << e << " dID " << Entity.dID << " type " << Entity.type << " Name " << Entity.groupname << " class " << Entity.key_classname <<endl;
					#endif
					
					for (int b = 0; b<Entity.t_brushes; b++)
					{
						brush &Brush = Entity.Brushes[b];
						Brush.dID = i-1;
					}
				}
			}
		}
	}
	
	// get total amount of brushes and entities for each detail group
	for (int dg = 0; dg<t_dgroups; dg++)
	{
		group &dGroup = mDetailGroup[dg];
		for (int i = 0; i<EntityList.size(); i++)
		{
			entity &Entity = EntityList[i];
			int dID = Entity.dID;
			
			if (Entity.IsDetail && Entity.dID==dg)
			mDetailGroup[dID].t_brushes += Entity.t_brushes;
			
			if (Entity.type==2 && Entity.IsDetail && Entity.dID==dg && Entity.key_classname!="info_detailgroup" && Entity.key_classname!="info_curve")
			mDetailGroup[dID].t_ents++;
			
			if (Entity.dID==dg&&dGroup.groupname=="")
			dGroup.groupname = Entity.groupname;
		}
		
		#if DEBUG > 0
		if (dev) cout << " Total Brushes for Detail Group " << dg << " - " << mDetailGroup[dg].t_brushes << endl;
		if (dev) cout << " Total Entities for Detail Group " << dg << " - " << mDetailGroup[dg].t_ents << endl;
		#endif
	}
	
	// create Point Entities for Detail Groups
	for (int dg = 0; dg<t_dgroups; dg++)
	{
		group &dGroup = mDetailGroup[dg];
		dGroup.Entities = new entity[dGroup.t_ents];
	}
	
	// copy point entities that are part of a detail group to the respective detail group
	for (int dg = 0, ed = 0; dg<t_dgroups; dg++)
	{
		group &dGroup = mDetailGroup[dg];
		for (int e = 1; e<EntityList.size(); e++)
		{
			entity &Source = EntityList[e];
			int dID = Source.dID;
			
			#if DEBUG > 0
			if (dev) cout << " DGroup " << dg << " Entity " << e << " List #" << ed<< endl;
			#endif
			
			if ( Source.type==2 && Source.IsDetail && Source.dID==dg && Source.key_classname!="info_detailgroup")
			{
				#if DEBUG > 0
				if (dev) cout << "   Copying point entity " << e << " of dgroup " << dg << " to this dgroups Entity list (current #" << ed<<")"<< endl;
				#endif
				
				entity &Entity = dGroup.Entities[ed];
				Entity.CopySimple(Source);
				ed++;
			}
		}
		ed=0;
	}
	
	// scan all solid entities (brush eID) and point entities of a detail group for property keyvalues (starting angle, pitch, yaw, separation, etc.)
	// double values are being overwritten, there can only be one command/entity that controls the whole detail group
	#if DEBUG > 0
	if (dev) cout << "  Scanning all entities of a detail group for property keyvalues..." << endl;
	#endif
	
	for (int d = 0; d<t_dgroups; d++)
	{
		group &dGroup = mDetailGroup[d];
		
		#if DEBUG > 0
		if (dev) cout << "     Detail Group #" << d << " total point ents " << dGroup.t_ents << " total brushes " << dGroup.t_brushes << endl;
		#endif
		
		for (int i = 0; i<EntityList.size(); i++)
		{
			entity &Entity = EntityList[i];
			if (Entity.type>=1 && Entity.IsDetail && Entity.dID==d)
			{
				#if DEBUG > 0
				if (dev) cout << "     Entity #" << i << " class: " << Entity.key_classname << " Keys: enable " << Entity.d_enable << " angle " << Entity.d_pos << " pitch " << Entity.d_autopitch << " yaw " << Entity.d_autoyaw << " sep " << Entity.d_separate << " d_autoname " << Entity.d_autoname  <<endl;
				#endif
				
				if (Entity.d_pos>0) 		dGroup.d_pos 		= Entity.d_pos;				else if (Entity.d_pos==-1)			dGroup.d_pos = 0;
				if (Entity.d_autopitch>0) 	dGroup.d_autopitch 	= Entity.d_autopitch;		else if (Entity.d_autopitch==-1) 	dGroup.d_autopitch 	= 0;
				if (Entity.d_autoyaw>0) 	dGroup.d_autoyaw 	= Entity.d_autoyaw;			else if (Entity.d_autoyaw==-1) 		dGroup.d_autoyaw 	= 0;
				if (Entity.d_separate>0) 	dGroup.d_separate 	= Entity.d_separate;		else if (Entity.d_separate==-1) 	dGroup.d_separate 	= 0;
				if (Entity.d_autoname>0) 	dGroup.d_autoname 	= Entity.d_autoname;		else if (Entity.d_autoname==-1) 	dGroup.d_autoname 	= 0;
				if (Entity.d_enable>0) 		dGroup.d_enable 	= Entity.d_enable;			else if (Entity.d_enable==-1) 		dGroup.d_enable 	= 0;
				if (Entity.d_circlemode>0) 	dGroup.d_circlemode	= Entity.d_circlemode;		else if (Entity.d_circlemode==-1) 	dGroup.d_circlemode = 0;
				if (Entity.d_carve>0) 		dGroup.d_carve 		= Entity.d_carve;			else if (Entity.d_carve==-1) 		dGroup.d_carve 		= 0;
				if (Entity.d_scale_rand.IsSet) 	dGroup.d_scale_rand 	= Entity.d_scale_rand;
				if (Entity.d_pos_rand.IsSet) 	dGroup.d_pos_rand 		= Entity.d_pos_rand;
				if (Entity.d_rotz_rand.IsSet) 	dGroup.d_rotz_rand 		= Entity.d_rotz_rand;
				if (Entity.d_movey_rand.IsSet) 	dGroup.d_movey_rand 	= Entity.d_movey_rand;
				if (Entity.d_draw>0) 		dGroup.d_draw 		= Entity.d_draw;		else if (Entity.d_draw==-1) 		dGroup.d_draw 		= 0;
				if (Entity.d_skip>0) 		dGroup.d_skip 		= Entity.d_skip;		else if (Entity.d_skip==-1) 		dGroup.d_skip 		= 0;
				if (Entity.d_draw_rand>0) 	dGroup.d_draw_rand 	= Entity.d_draw_rand;	else if (Entity.d_draw_rand==-1) 	dGroup.d_draw_rand 	= 0;
				
				#if DEBUG > 0
				if (dev) cout << "     dGroup #" << d << " Name: " << Entity.groupname << " angle: " << dGroup.d_pos << " pitch " << dGroup.d_autopitch << " yaw " << dGroup.d_autoyaw << " separate " << dGroup.d_separate << " enable " << dGroup.d_enable << " d_autoname " << dGroup.d_autoname <<endl;
				#endif
			}
		}
	}
}


void file::TransformSource()
{
	#if DEBUG > 0
	bool dev = 0;
	if (dev) cout << " Applying custom Source transformations..." << endl;
	if (dev) system("pause");
	#endif
	
	// Apply custom transformations
	for (int g = 0; g<mGroup->t_arcs; g++)
	{
		group &Group = sGroup[g];
		float scale_x = cTable[g].scale_src.x;
		float scale_y = cTable[g].scale_src.y;
		float scale_z = cTable[g].scale_src.z;
		
		float rot_x = cTable[g].rot_src.x;
		float rot_y = cTable[g].rot_src.y;
		float rot_z = cTable[g].rot_src.z;
					
		if (Group.valid)
		{
			#if DEBUG > 0
			if (dev) cout << "   Group "<<g<<"..." <<endl;
			#endif
			
			for (int b = 0; b<Group.t_brushes; b++)
			{
				brush &Brush = Group.Brushes[b];
				if (Brush.valid)
				{
					#if DEBUG > 0
					if (dev&&b==0) cout << "     Scale "  << scale_x << endl;
					if (cTable[g].scale_src.IsSet)
					#endif
					
					Brush.ScaleOrigin(scale_x, Group.Origin, g);
					
					#if DEBUG > 0
					if (dev&&b==0) cout << "     Rot "  << rot_x << "," << rot_y << "," <<rot_z << endl;
					#endif
					
					if (cTable[g].rot_src.IsSet)
					Brush.RotOrigin(rot_x,rot_y,rot_z, Group.Origin, g);
				}
			}
			
			if (cTable[g].rot_src.y!=0||cTable[g].rot_src.z!=0)
			{
				#if DEBUG > 0
				if(dev) cout << "  rot_src active -> Rounding Vertices..." << endl;
				#endif
				
				Group.RoundBrushVertices(1);
			}
			
			// Detail Objects
			group_set &Set = sDetailSet[g];
			for (int d = 0; d<t_dgroups; d++)
			{
				group &dGroup = Set.Groups[d];
				vertex nOrigin = Group.Origin;
				//nOrigin.x = 0;
				
				for (int b = 0; b<dGroup.t_brushes; b++)
				{
					brush &Brush = dGroup.Brushes[b];
					
					if (cTable[g].scale_src.IsSet)	Brush.ScaleOrigin(scale_x, nOrigin, g);
					if (cTable[g].rot_src.IsSet)	Brush.RotOrigin(rot_x,rot_y,rot_z, nOrigin, g);
				}
				for (int e = 0; e<dGroup.t_ents; e++)
				{
					entity &Entity = dGroup.Entities[e];
					
					if (cTable[g].scale_src.IsSet)	Entity.ScaleOrigin(scale_x,nOrigin);
					if (cTable[g].rot_src.IsSet)	Entity.RotateOrigin(rot_x,rot_y,rot_z, nOrigin);
					Euler RotEuler(rot_x,rot_y,rot_z);
					if (cTable[g].rot_src.IsSet)	Entity.RotateEntity(RotEuler,1);
				}
				dGroup.GetGroupDimensions(1,1);
				
				// fix position of basic detail objects
				/*if (dGroup.Origin.x!=0)
				{
					for (int b = 0; b<dGroup.t_brushes; b++)
					{
						brush &Brush = dGroup.Brushes[b];
						Brush.Move(-(dGroup.Origin.x),0,0,1);
					}
					for (int k = 0; k<dGroup.t_ents; k++)
					{
						entity &Entity = dGroup.Entities[k];
						Entity.Origin.x -= dGroup.Origin.x;
					}
				}
				dGroup.Origin.x = 0;*/
			}
			#if DEBUG > 0
			if (dev) cout << "   Getting Dimensions..." <<endl;
			#endif
			
			Group.GetGroupDimensions(1,0);
			
			#if DEBUG > 0
			if (dev) cout << "   Checking Brush Validity..." <<endl;
			#endif
			
			Group.CheckBrushValidity();
			
			#if DEBUG > 0
			if (dev) cout << "   Getting Face Orients..." <<endl;
			#endif
			
			Group.GetBrushFaceOrients();
			
			#if DEBUG > 0
			if (dev) cout << "   Getting Texture Aligns..." <<endl;
			#endif
			
			Group.GetBrushTVecAligns();
			
			#if DEBUG > 0
			if (dev) cout << "   Updating Vertex List..." <<endl;
			#endif
			
			Group.GetGroupVertexList();
			
			#if DEBUG > 0
			if (dev) cout << "   Checking Brush Divisibility..." <<endl;
			#endif
			
			Group.CheckBrushDivisibility();
		}
	}
}


void file::TransformFinal(int g)
{
	#if DEBUG > 0
	bool dev = 0;
	#endif
	
	group &Group = bGroup[g];
	
	float scale_x = cTable[g].scale.x;
	//float scale_y = cTable[g].scale.y;
	//float scale_z = cTable[g].scale.z;
	
	float move_x = cTable[g].move.x;
	float move_y = cTable[g].move.y;
	float move_z = cTable[g].move.z;
	
	float rot_x = cTable[g].rot.x;
	float rot_y = cTable[g].rot.y;
	float rot_z = cTable[g].rot.z;
	
	bool scale_valid = 1; 	if(scale_x==0&&scale_x==1) 			scale_valid = 0;
	bool rot_valid = 1; 	if(rot_x==0&&rot_y==0&&rot_z==0) 	rot_valid = 0;
	bool move_valid = 1; 	if(move_x==0&&move_y==0&&move_z==0) move_valid = 0;
	
	#if DEBUG > 0
	if (dev) cout << " ######## Final Transformation: ######## " << cTable[g].rot << endl;
	#endif
	
	// Apply custom transformations
	for (int b = 0; b<bGroup[g].t_brushes; b++)
	{
		brush &Brush = bGroup[g].Brushes[b];
		
		if( Brush.valid && Group.IsSecInRange(Brush.SecID) )
		{
			//if (Brush.Tri==nullptr)
			{
				if (cTable[g].scale.IsSet&&scale_valid) {
					Brush.Scale(scale_x);
					//Brush.Scale(scale_y);
					//Brush.Scale(scale_z);
				}
				
				if (cTable[g].rot.IsSet&&rot_valid)
					Brush.Rot(rot_x,rot_y,rot_z);
					
				if (cTable[g].move.IsSet&&move_valid)
					Brush.Move(move_x,move_y,move_z,1,g);
			}
			
			if (Brush.Tri!=nullptr)
			{
				for (int bt = 0; bt<Brush.t_tri; bt++)
				{
					brush &TriBrush = Brush.Tri[bt];
					if (cTable[g].scale.IsSet&&scale_valid)		TriBrush.Scale(scale_x);
					if (cTable[g].rot.IsSet&&rot_valid)			TriBrush.Rot(rot_x,rot_y,rot_z);
					if (cTable[g].move.IsSet&&move_valid)		TriBrush.Move(move_x,move_y,move_z,1,g);
				}
			}
		}
	}
	
	// detail groups
	for (int d = 0; d<t_dgroups; d++)
	{
		group &EGroup = DetailSet[g].Groups[d];
		for (int b = 0; b<EGroup.t_brushes; b++)
		{
			brush &Brush = EGroup.Brushes[b];
			
			if( Brush.valid && Brush.draw && EGroup.IsSecInRange(Brush.SecID) )
			{
				if (cTable[g].scale.IsSet&&scale_valid)	Brush.Scale(scale_x);
				if (cTable[g].rot.IsSet&&rot_valid)		Brush.Rot(rot_x,rot_y,rot_z);
				if (cTable[g].move.IsSet&&move_valid)	Brush.Move(move_x,move_y,move_z,1,d);
			}
		}
		for (int e = 0; e<EGroup.t_ents; e++)
		{
			entity &Entity = EGroup.Entities[e];
			
			if( Entity.draw && EGroup.IsSecInRange(Entity.SecID) )
			{
				if (cTable[g].scale.IsSet&&scale_valid)	Entity.ScaleOrigin(scale_x, Zero);
				if (cTable[g].rot.IsSet&&rot_valid)		Entity.RotateOrigin(rot_x,rot_y,rot_z, Zero);
				Euler RotEuler(rot_x,rot_y,rot_z);
				if (cTable[g].rot.IsSet&&rot_valid)		Entity.RotateEntity(RotEuler,1);
				if (cTable[g].move.IsSet&&move_valid)	Entity.Origin.move(move_x,move_y,move_z);
			}
		}
	}
}


void file::createGroupMap()
{
	#if DEBUG > 0
	bool dev = 0;
	#endif
	
	mGroup = new group;
	mGroup->IsSrcMap = 1;
	int &tbc = mGroup->t_brushes;
	int &tfc = mGroup->t_faces;
	
	#if DEBUG > 0
	if (dev) cout << " LoadMap_GetEntities..."<<endl;
	#endif
	
	LoadMap_GetEntities();
	
	// copy entity brushes that are not of a detail type into curve source object
	mGroup->Brushes = new brush[tbc];
	
	#if DEBUG > 0
	if (dev) cout << "Created " << tbc << " Map Brushes!" << endl;
	#endif
	
	for (int e = 0, b=0; e<EntityList.size(); e++)
	{
		entity &Entity = EntityList[e];
		if (!Entity.IsDetail)
		{
			for (int eb = 0; eb<Entity.t_brushes; eb++)
			{
				brush &Brush = mGroup->Brushes[b];
				Brush.Copy(Entity.Brushes[eb]);
				Brush.SegID = b;
				b++;
			}
		}
	}
	
	#if DEBUG > 0
	if (dev) cout << "  GetDimensions..." << endl;
	#endif
	
	mGroup->GetGroupDimensions(0,0);
	
	#if DEBUG > 0
	if (dev) system("pause");
	/*
	cout << " Dimensions: x " << mGroup->Dimensions.xs << " " << mGroup->Dimensions.xb << endl;
	cout << " Dimensions: y " << mGroup->Dimensions.ys << " " << mGroup->Dimensions.yb << endl;
	cout << " Dimensions: z " << mGroup->Dimensions.zs << " " << mGroup->Dimensions.zb << endl;
	cout << " Origin: " << mGroup->Origin << endl;
	*/
	#endif
}


void file::LoadMap_GetTexInfo()
{
	#if DEBUG > 0
	bool dev = 0;
	#endif
	
	string WadFileString = "";
	int &tbc = mGroup->t_brushes;
	
	// look for Textures and WADs used in Map File
	#if DEBUG > 0
	if (dev) cout << " Looking for Textures and WADs used in Map File..." << endl;
	#endif
	
	vector<string> WadListMap;
	int WADline_start = str_map.find("\"wad\" ", 0);
	if (WADline_start!=str_map.npos)
	{
		 // seperate line with wad list
		int WADline_end = str_map.find("\n", WADline_start+5);
		string WADline = "";
		WADline = str_map.substr(WADline_start+5, WADline_end-WADline_start-5);
		
		//cout << "WADline: " << WADline << endl;
		bool linux = 0;
		if (WADline.find("\\") ==-1) linux = 1;
		
		// extract wads being used in map file
		int found_start = 0, found_end = 0;
		int last_pos = 0;
		while (last_pos!=WADline.npos)
		{
			found_end = WADline.find(".wad", last_pos);
			//wadcount++;
			if (linux) found_start = WADline.rfind("/",found_end)+1;
			else found_start = WADline.rfind("\\",found_end)+1;
			//cout << "  found_start: " << found_start << " found_end: " << found_end << " new substring: " << WADline.substr(found_start, found_end-found_start) << endl;
			WadListMap.push_back(WADline.substr(found_start, found_end-found_start));
			last_pos = WADline.find(".wad", found_end+5);
		}
		
		//cout << endl << "    Used WADs in Map File: " << endl;
		cout << "|" <<endl;
		cout << "|    [INFO] " << WadListMap.size() << " WAD files are listed in this map file:" << endl;
		for (int i = 0; i<WadListMap.size(); i++)
		cout << "|           #" << i+1 << " " << WadListMap[i] << endl;
		
		// compare scanned wads from map file with WAD directory of Map2Curve
		for (int i = 0, succeed=0; i<WadListMap.size(); i++)
		{
			for (int j = 0; j<WADFiles.size(); j++)
			{
				int look4wad = WADFiles[j].FilePath.find(WadListMap[i], 0);
				
				if (look4wad!=-1) {
					succeed++;
				}
			}
			
			if (i==WadListMap.size()-1)
			{
				if (succeed==WadListMap.size()) 	{cout << "|           ALL";}
				else if (succeed==0) 				{cout << "|           NONE";}
				else 							{cout << 	 "|           " << succeed;}
				cout << " of them are known by Map2Curve!" << endl;
				cout << "|" <<endl;
			}
		}
	}
	
	for (int b = 0; b<mGroup->t_brushes; b++)
	{
		brush &Brush = mGroup->Brushes[b];
		LoadMap_GetTexInfoScanBrush(Brush);
	}
	
	for (int e = 0; e<EntityList.size(); e++)
	{
		if (EntityList[e].IsDetail)
		for (int b = 0; b<EntityList[e].t_brushes; b++)
		{
			brush &Brush = EntityList[e].Brushes[b];
			LoadMap_GetTexInfoScanBrush(Brush);
		}
	}
	
	// load texture informations from wad3 files
	#if DEBUG > 0
	if(dev) cout << "Loading texture informations from WAD3 files..." << endl;
	#endif
	
	vector<string> fail_tex;
	for (int i = 0; i<tTable_name.size(); i++) 
	{
		string &texn = tTable_name[i];
		int &texw = tTable_width[i];
		int &texh = tTable_height[i];
		
		int tID = -1;
		int wID = 0;
		
		#if DEBUG > 0
		if(dev) cout << " looking for texture " << texn << " in previously loaded wad files... " << endl;
		#endif
		
		for (int j=0; j<WADFiles.size(); j++)
		{
			tID = WADFiles[j].FindTexture(texn);
			
			#if DEBUG > 0
			if(dev) cout << " found tID " << tID << " WAD #" << j << " ("<<WADFiles[j].FilePath<<")" <<endl;
			#endif
			
			if (tID!=-1) { wID=j; break; }
		}
		// when texture name was found, add its width and height to the current texture list entry
		if (tID!=-1)
		{
			WADFiles[wID].GetTexInfo(tID, texw, texh);
		}
		else // if texture name wasnt found, use default size
		{
			#if DEBUG > 0
			if(dev) cout << "  Texture name " << texn << " not found..." << endl;
			#endif
			
			fail_tex.push_back(texn);
		}
		
		// at the end of texture list, check if there were failed textures and display a User-message
		if (i==tTable_name.size()-1&&fail_tex.size()>0)
		{
			cout << "|    [WARNING] Could not get informations of " << fail_tex.size() << " texture(s):" << endl;
			for (int i = 0; i<fail_tex.size(); i++)
			cout << "|              " << fail_tex[i] << endl;
			cout << "|              Using default width and height (128px)."<< endl;
			cout << "|              Texture Offsets will probably be wrong!" << endl;
			cout << "|" << endl;
		}
	}
	
	#if DEBUG > 0
	if(dev) cout << "Final Texture List and Info:" << endl;
	if(dev)
	for (int i = 0; i<tTable_name.size(); i++) 
	{
		string &texn = tTable_name[i];
		int &texw = tTable_width[i];
		int &texh = tTable_height[i];
		
		cout << " Tex #" << i << " Name " << texn << " width " << texw << " height " << texh << endl;
	}
	if(dev) system("pause");
	#endif
}

// scan all faces for Textures and add those to the files texture list
void file::LoadMap_GetTexInfoScanBrush(brush &Brush)
{
	#if DEBUG > 0
	bool dev = 0;
	if(dev) cout << "Scanning all faces for Textures and add those to the files texture list..." << endl;
	#endif
	
	for(int f = 0; f<Brush.t_faces; f++)
	{
		face &Face = Brush.Faces[f];
		int tex_count = tTable_name.size();
		
		#if DEBUG > 0
		if(dev) cout << " Checking Texture "<<Face.Texture<<" of Face " << f << " tTable_name.size: " << tex_count << endl;
		#endif
		
		if (tex_count==0)
		{
			tTable_name.push_back(Face.Texture);
			tTable_width.push_back(128);
			tTable_height.push_back(128);
			
			#if DEBUG > 0
			if(dev) Face.tID = 0;
			#endif
		}
		else
		{
			for (int j = 0; j<tex_count; j++)
			{
				if (Face.Texture==tTable_name[j]) // current Face Texture is already in the list
				{
					Face.tID=j;
					
					#if DEBUG > 0
					if(dev) cout << "   Face.Texture("<<Face.Texture<<")==tTable_name["<<j<<"]("<<tTable_name[j]<<")... Aborting! Face.tID " << Face.tID << endl;
					#endif
					
					break;
				}
				else if (j==tex_count-1)
				{
					Face.tID=j+1;
					
					#if DEBUG > 0
					if(dev) cout << "   Reached end of tTable_name list. Adding Face.Texture ("<<Face.Texture<<") to tTable_name. Face.tID " << Face.tID << endl;
					#endif
					
					tTable_name.push_back(Face.Texture);
					tTable_width.push_back(128);
					tTable_height.push_back(128);
				}
			}
			
			#if DEBUG > 0
			if(dev) cout << endl;
			#endif
		}
	}
	
	#if DEBUG > 0
	if(dev) cout << " Texture List size now " << tTable_name.size() << "!" << endl;
	//system("pause");
	#endif
}

void file::LoadMap_ConvertWorld2Face()
{
	#if DEBUG > 0
	bool dev = 0;
	#endif
	
	// this turns world alignment of faces into face alignment, so the new vectors can easily be created from a generated mesh 1:1
	// this is being done, because otherwise it would be much more complicated to determine the new texture vector align
	
	int &tbc = mGroup->t_brushes;
	//check which body Faces have a Face Align and mark those which do not
	for (int b = 0; b<tbc; b++)
	{
		brush &Brush = mGroup->Brushes[b];
		if (Brush.valid)
		{
			for (int f = 0; f<Brush.t_faces; f++)
			{
				face &Face = mGroup->Brushes[b].Faces[f];
				
				CheckFaceAlign(Face);
			}
		}
	}
	
	// check for Faces with World Alignments and turn those into Face Alignments
	for (int b = 0; b<tbc; b++)
	{
		brush &Brush = mGroup->Brushes[b];
		if (Brush.valid)
		{
			for (int f = 0; f<Brush.t_faces; f++)
			{
				face &Face = mGroup->Brushes[b].Faces[f];
				
				#if DEBUG > 0
				if(dev)cout << " Brush " << b << " Face " << f << " Tex" << Face.Texture << endl;
				#endif
				
				if (Face.fID==2&&Face.HasWorldAlign)
				{
					gvector &Vec = *Face.VecV;
					gvector EdgeEquiv;
					
					//check if Face Edge v0/v1 is relevant for this Tex Vector, flip it if not
					gvector Edge = GetVector( Face.VerticesC[0],Face.VerticesC[1] );
					gvector EdgeN = Normalize(Edge);

					#if DEBUG > 0
					if(dev) cout << "     Getting Dot of Edge(N) " << EdgeN << " and Vec " << Vec;
					#endif

					float Dot = GetDot(EdgeN, Vec);
					
					#if DEBUG > 0
					if(dev) cout << " Dot " << Dot;
					#endif
					
					if (Dot<0) { Edge.flip(); EdgeN.flip(); } /*EdgeN.flip(); Dot = GetDot(EdgeN, Vec); cout << " - Flipped Edge! Dot now " << Dot << endl; } else cout << endl;*/
					
					#if DEBUG > 0
					if(dev) cout << "     TVec Old " << Vec << " Edge " << Edge << endl;
					#endif
					
					// fix Texture Scale
					float TexLenO = GetAdjaLen (Edge, Vec);
					float TexLenN = GetVecLen (Edge);
					float m = TexLenO/TexLenN;
					
					#if DEBUG > 0
					if(dev)cout << "     TexLenO " << TexLenO << " TexLenN " << TexLenN << " Mod " << m << endl << endl;
					#endif
					
					Vec.CopyCoords(EdgeN);
					
					if (Face.VecX.IsHor)
					{
						Face.ScaleY /= m;
						GetBaseShift(Face, 2, 1, 0);
						Face.ShiftY = Face.BaseShiftY + Face.OffsetY;
					}
					else
					{
						Face.ScaleX /= m;
						GetBaseShift(Face, 1, 1, 0);
						Face.ShiftX = Face.BaseShiftX + Face.OffsetX;
					}
				}
			}
		}
	}
}

void file::createDetailGroupSource()
{
	#if DEBUG > 0
	bool dev = 0;
	#endif
	
	int t_arcs = mGroup->t_arcs;
	sDetailSet = new group_set[t_arcs];
	
	// create source detail groups
	for (int a = 0; a<t_arcs; a++)
	{
		group_set &Set = sDetailSet[a];
		Set.Groups = new group[t_dgroups];
		Set.t_groups = t_dgroups;
		for (int d = 0; d<Set.t_groups; d++)
		{
			group &dGroup = Set.Groups[d];
			
			// detail group properties
			dGroup.CopyProps(mDetailGroup[d]);
			
			// solid entities
			int brushcount 		= mDetailGroup[d].t_brushes;
			dGroup.Brushes 		= new brush[ brushcount ];
			dGroup.t_brushes 	= brushcount;
			
			// point entities
			int entcount 		= mDetailGroup[d].t_ents;
			dGroup.Entities 	= new entity[ entcount ];
			dGroup.t_ents 		= entcount;
			
			dGroup.gID			= a;
			dGroup.Dimensions 	= mDetailGroup[d].Dimensions;
			dGroup.Origin 		= mDetailGroup[d].Origin;
			dGroup.groupname 	= mDetailGroup[d].groupname;
			dGroup.HasOrigin	= mDetailGroup[d].HasOrigin;
		}
	}
	// fill source detail object groups
	for (int a = 0; a<t_arcs; a++)
	{
		group_set &Set = sDetailSet[a];
		for (int d = 0; d<Set.t_groups; d++)
		{
			group &dGroup = Set.Groups[d];
			
			// Solid Brushes of this detail group
			for (int b = 0; b<dGroup.t_brushes; b++)
			{
				brush &Brush = dGroup.Brushes[b];
				Brush.Copy(mDetailGroup[d].Brushes[b]);
			}
			
			// Point Entities of this detail group
			for (int e = 0; e<dGroup.t_ents; e++)
			{
				entity &Entity = dGroup.Entities[e];
				Entity.CopySimple(mDetailGroup[d].Entities[e]);
			}
		}
	}
}

void file::createDetailGroup(int g)
{
	#if DEBUG > 0
	bool dev = 0;
	#endif
	
	int t_arcs = mGroup->t_arcs;
	
	// create final detail object group
	#if DEBUG > 0
	if (dev) cout << " Creating final detail object group..." << endl;
	#endif
	
	if (DetailSet==nullptr) DetailSet = new group_set[t_arcs];

	#if DEBUG > 0
	if (dev) cout << "   " << t_arcs << " Group Sets created!" << endl;
	#endif
	
	// detail group sets
	{
		group_set &Set = DetailSet[g];
		group_set &SetSource = sDetailSet[g];
		Set.Groups = new group[t_dgroups];
		Set.t_groups = t_dgroups;
		
		#if DEBUG > 0
		if (dev) cout << "     " << t_dgroups << " Detail groups created inside Set #"<<g<<"!" << endl;
		#endif
		
		// detail groups (group of solid and point entities for an entire curve (*res))
		for (int d = 0; d<Set.t_groups; d++)
		{
			group &dGroup = Set.Groups[d];
			group &sdGroup = SetSource.Groups[d];
			
			#if DEBUG > 0
			if (dev) cout << "       brushcount is res "<< cTable[g].res <<" * " << " sdGroup.t_brushes " << sdGroup.t_brushes << " = " << cTable[g].res * sdGroup.t_brushes << endl;
			#endif
			
			// detail group properties
			dGroup.CopyProps(sdGroup);
			
			// solid entities
			int brushcount = cTable[g].res * sdGroup.t_brushes;
			dGroup.Brushes = new brush[ brushcount ];
			
			#if DEBUG > 0
			if (dev) cout << "       " << brushcount << " Brushes created inside Detail Group #"<<d<<"!" << endl;
			#endif
			
			dGroup.t_brushes = brushcount;
			
			// point entities
			int entcount = cTable[g].res * sdGroup.t_ents;
			dGroup.Entities = new entity[ entcount ];
			dGroup.t_ents = entcount;
			
			#if DEBUG > 0
			if (dev) cout << "       " << entcount << " Entities created inside Detail Group #"<<d<<"!" << endl;
			#endif
			
			dGroup.gID			= g;
			dGroup.Dimensions 	= sdGroup.Dimensions;
			dGroup.Origin 		= sdGroup.Origin;
			dGroup.groupname 	= sdGroup.groupname;
			dGroup.HasOrigin	= sdGroup.HasOrigin;
			
			#if DEBUG > 0
			if (dev) cout << "       Dimensions " << dGroup.Dimensions << " Origin " << dGroup.Origin << endl;
			#endif
		}
	}
	
	// fill final detail object group
	#if DEBUG > 0
	if (dev) system("pause");
	if (dev) cout << endl << " Filling final detail object group..." << endl;
	#endif
	{
		group_set &Set = DetailSet[g];
		group_set &SetSource = sDetailSet[g];
		
		for (int d = 0; d<Set.t_groups; d++)
		{
			group &dGroup = Set.Groups[d];
			group &sdGroup = SetSource.Groups[d];
			
			#if DEBUG > 0
			if (dev) cout << "   " << t_dgroups << " Filling Brushes of Detail Group #"<<d<<"!" << endl;
			#endif
			
			// Solid Brushes of this detail group
			for (int b = 0, sb=0, sec=0; b<dGroup.t_brushes; b++)
			{
				brush &Brush = dGroup.Brushes[b];
				Brush.Copy(sdGroup.Brushes[sb]);
				Brush.SecID = sec;
				
				#if DEBUG > 0
				if (dev) cout << "     Brush "<<b<<" source Brush " << sb << " (of max " << sdGroup.t_brushes << ")" << " sec " << sec << " (of max " << cTable[g].res << ")" << endl;
				#endif
				
				sec++;
				if (sec==cTable[g].res)
				{
					#if DEBUG > 0
					if (dev) cout << "       Final Section reached! Resetting Sec to 0. Increasing source brush to " << sb+1 << " of max " << sdGroup.t_brushes << endl;
					#endif
					
					sec=0;
					sb++;
				}
			}
			
			// Point Entities of this detail group
			for (int e = 0, se=0, sec=0; e<dGroup.t_ents; e++)
			{
				entity &Entity = dGroup.Entities[e];
				Entity.CopySimple(sdGroup.Entities[se]);
				Entity.SecID = sec;
				
				#if DEBUG > 0
				if (dev) cout << "     Entity "<<e<<" source Entity " << se << " type "<< Entity.Keys[0].value << " (of max "<<sdGroup.t_ents<<")" << " sec " << sec << " (of max " << cTable[g].res << ") Euler " << Entity.Angles << endl;
				#endif
				
				sec++;
				
				if (sec==cTable[g].res)
				{
					#if DEBUG > 0
					if (dev) cout << "       Final Section reached! Resetting Sec to 0. Increasing source entity to " << se+1 << " (of max "<< sdGroup.t_ents <<")" << endl;
					#endif
					
					sec=0;
					se++;
				}
			}
		}
	}
	
	
	// apply draw, skip or random draw settings
	{
		group_set &Set = DetailSet[g];
		int G_DRAW = 0; if (cTable[g].d_draw>0) G_DRAW = cTable[g].d_draw;
		int G_SKIP = 0; if (cTable[g].d_skip>0) G_SKIP = cTable[g].d_skip;
		bool G_DRAW_RAND = 0; if (cTable[g].d_draw_rand==1) G_DRAW_RAND = 1;
		
		#if DEBUG > 0
		if (dev) cout << " G_DRAW " <<  G_DRAW << " G_SKIP " << G_SKIP << " G_DRAW_RAND " << G_DRAW_RAND << endl;
		#endif
		
		for (int d = 0; d<Set.t_groups; d++)
		{
			group &dGroup = Set.Groups[d];
			int L_DRAW = 0; if (dGroup.d_draw>0) L_DRAW = dGroup.d_draw;
			int L_SKIP = 0; if (dGroup.d_skip>0) L_SKIP = dGroup.d_skip;
			bool L_DRAW_RAND = 0; if (dGroup.d_draw_rand==1||(G_DRAW_RAND&&dGroup.d_draw_rand<0)) L_DRAW_RAND = 1;
			
			int F_DRAW = G_DRAW; if (L_DRAW>0) F_DRAW = L_DRAW;
			int F_SKIP = G_SKIP; if (L_SKIP>0) F_SKIP = L_SKIP;
			
			#if DEBUG > 0
			if (dev) cout << " L_DRAW " <<  L_DRAW << " L_SKIP " << L_SKIP << " L_DRAW_RAND " << L_DRAW_RAND << " F_DRAW " << F_DRAW << " F_SKIP " << F_SKIP << endl;
			#endif
			
			bool R_DRAW[cTable[g].res]; for (int i=0; i<cTable[g].res; i++) R_DRAW[i] = 1;
			if ( (G_DRAW_RAND&&L_DRAW_RAND) || (!G_DRAW_RAND&&L_DRAW_RAND) )
			for (int i=0; i<cTable[g].res; i++) {
				int RandZero = rand() % 10;
				if (RandZero % 2 != 0) R_DRAW[i] = 0;
			}
			
			for (int b=0, draw_ctr, skip_ctr; b<dGroup.t_brushes; b++) // Brushes
			{
				brush &Brush = dGroup.Brushes[b];
				int sec = Brush.SecID;
				if (sec==0) {
				draw_ctr = F_DRAW; if (draw_ctr>=0) draw_ctr--;
				skip_ctr = F_SKIP; if (skip_ctr>=0) skip_ctr--; }
				
				if ( (G_DRAW_RAND&&L_DRAW_RAND) || (!G_DRAW_RAND&&L_DRAW_RAND) ) {
					Brush.draw = R_DRAW[sec];
				} else {
					if (F_DRAW>0 && sec!=draw_ctr) {
						Brush.draw = 0;
					} else if (F_DRAW>0 && sec==draw_ctr) {
						draw_ctr += F_DRAW;
					}
					
					if (F_SKIP>0 && sec==skip_ctr) {
						Brush.draw = 0;
						skip_ctr += F_SKIP;
					}
				}
			}
			for (int e = 0, draw_ctr, skip_ctr; e<dGroup.t_ents; e++) // Entities
			{
				entity &Entity = dGroup.Entities[e];
				int sec = Entity.SecID;
				if (sec==0) {
				draw_ctr = F_DRAW; if (draw_ctr>=0) draw_ctr--;
				skip_ctr = F_SKIP; if (skip_ctr>=0) skip_ctr--; }
				
				if ( (G_DRAW_RAND&&L_DRAW_RAND) || (!G_DRAW_RAND&&L_DRAW_RAND) )
				{
					Entity.draw = R_DRAW[sec];
					
					#if DEBUG > 0
					if (dev) cout << " sec " << sec << " random draw is " << R_DRAW[sec] <<endl;
					#endif
					
				} else {
					if (F_DRAW>0 && sec!=draw_ctr)
					{
						Entity.draw = 0;
						
						#if DEBUG > 0
						if (dev) cout << " sec " << sec << " is != draw_ctr " << draw_ctr << " Entity gets SKIPPED! " <<endl;
						#endif
						
					} else if (F_DRAW>0 && sec==draw_ctr)
					{
						#if DEBUG > 0
						if (dev) cout << " sec " << sec << " is == draw_ctr " << draw_ctr << " Entity gets DRAWN! draw_ctr now " << draw_ctr+F_DRAW <<endl;
						#endif
						
						draw_ctr += F_DRAW;
					}
					
					if (F_SKIP>0 && sec==skip_ctr)
					{
						Entity.draw = 0;
						
						#if DEBUG > 0
						if (dev) cout << " sec " << sec << " is = skip_ctr " << skip_ctr << " Entity gets SKIPPED! skip_ctr now " << skip_ctr+F_SKIP<< endl;
						#endif
						
						skip_ctr += F_SKIP;
					}
				}
			}
		}
	}
	
	#if DEBUG > 0
	if (dev) system("pause");
	#endif
}

void file::createGroupSource()
{
	#if DEBUG > 0
	bool dev = 0;
	// count rad-commands from settings list to get max amount of indivdual arcs to be generated
	if (dev) cout << " count rad-commands from settings list..." << endl;
	#endif
	
	int &t_arcs = mGroup->t_arcs;
	
	#if DEBUG > 0
	if (dev) cout << " Total Arcs Pre " << t_arcs << endl;
	#endif
	
	for (int i = 0; i<settings.size(); i++)
	{
		if (settings[i]=="rad") t_arcs++;
		
		#if DEBUG > 0
		if(dev&&i%2==0) { cout << "   settings["<<i<<"] [" << settings[i] << "] Found? "; if (settings[i]=="rad") cout << "YES!"; else cout << "NO!"; cout << " t-arcs now " << t_arcs << endl; }
		#endif
	}
	if (t_arcs==0) t_arcs = 1;
	if (InternalMapSettings) t_arcs = t_iarcs;

	#if DEBUG > 0
	//cout << "File contains " << t_arcs << " rad commands." << endl; system("pause");
	if (dev) cout << " Total Arcs After " << t_arcs << endl;
	#endif
	
	sGroup = new group[t_arcs];
	for (int g = 0; g<t_arcs; g++)
	{
		if (mGroup->t_brushes>0&&mGroup->t_brushes-mGroup->invalids>0)
		{
			#if DEBUG > 0
			if (dev) cout << " #"<<g<<" Copying mGroup... invalids " << mGroup->invalids << " valids " << mGroup->t_brushes  << endl;
			#endif
			
			sGroup[g].Copy(*mGroup);
			sGroup[g].IsSrcMap = 1;
			sGroup[g].gID = g;
			
			#if DEBUG > 0
			if (dev) cout << " mGroup copied to sGroup"<<g<<" invalids " << sGroup[g].invalids << " valids " << sGroup[g].t_brushes  << endl;
			#endif
		}
		else if(mGroup->t_brushes==0)
		{
			#if DEBUG > 0
			if (dev) cout << "|    [WARNING] No Curve Brushes found! Is this a problem or not, I do not know. Let us see..." << endl;
			#endif
		}
		else
		{
			cout << "|    [WARNING] Something went horribly wrong! The Source Map doesn't contain valid brushes ("<< mGroup->t_brushes-mGroup->invalids << ")!" << endl;
		}
	}
	
	#if DEBUG > 0
	if (dev) system("pause");
	#endif
}

void file::LoadMap()
{
	#if DEBUG > 0
	bool dev = 0;
	#endif
	
	#if DEBUG > 0
	if (dev) cout << " createGroupMap..."<<endl;
	#endif
	createGroupMap();
	
	#if DEBUG > 0
	if (dev) cout << " LoadMap_GetTexInfo..."<<endl;
	#endif
	// get dimensions (width and height) of all textures that were used in the source map
	LoadMap_GetTexInfo();
	
	#if DEBUG > 0
	if (dev) cout << " Create Detail Objects..."<<endl;
	#endif
	LoadMap_DetailObj();
	
	#if DEBUG > 0
	if (dev) cout << " CheckBrushValidity..."<<endl;
	#endif
	mGroup->CheckBrushValidity(); // can brushes actually be turned into curves?
	mGroup->GetBrushFaceOrients(); // determine base/head and body faces
	
	#if DEBUG > 0
	if (dev) cout << " GetBrushTVecAligns..."<<endl;
	#endif
	mGroup->GetBrushTVecAligns(); // see if texture vectors are aligned correctly and check their orientation
	
	#if DEBUG > 0
	if (dev) cout << " ReconstructMap..."<<endl;
	#endif
	mGroup->ReconstructMap(); // get all of the missing vertices for further calculations (map-files only save 3 vertices per plane)
	
	#if DEBUG > 0
	if (dev) cout << " CheckBrushDivisibility..."<<endl;
	#endif
	mGroup->CheckBrushDivisibility(); // check whether a brush can be triangulated or not
	
	#if DEBUG > 0
	if (dev) cout << " GetRconBrushShifts..."<<endl;
	#endif
	mGroup->GetBrushShifts(); // get original texture shifts
	
	#if DEBUG > 0
	if (dev) cout << " LoadMap_ConvertWorld2Face..."<<endl;
	#endif
	LoadMap_ConvertWorld2Face(); // turn world alignment of faces into face alignment, so the new vectors can easily be created from a generated mesh 1:1
	
	
	#if DEBUG > 0
	//if (dev) cout << "  Exporting Raw Source to Mapfile..." << endl;
	#endif
}

void file::LoadMap_DetailObj()
{
	#if DEBUG > 0
	bool dev = 0;
	if (dev) cout << "  Reconstructing original detail brushes..." << endl;
	if (dev) system("pause");
	#endif
	
	// reconstruct original detail brushes and get their texute offsets
	for (int e = 0; e<EntityList.size(); e++)
	{
		entity &Entity = EntityList[e];
		if (Entity.IsDetail)
		{
			for (int eb = 0; eb<Entity.t_brushes; eb++)
			{
				brush &Brush = Entity.Brushes[eb];
				
				// restore missing vertices of each face
				Brush.Reconstruct();
			}
		}
	}
	
	#if DEBUG > 0
	if (dev) cout << "  Fixing Holes..." << endl;
	if (dev) system("pause");
	#endif
	
	for (int e = 0; e<EntityList.size(); e++)
	{
		entity &Entity = EntityList[e];
		if (Entity.IsDetail)
		{
			for (int eb = 0; eb<Entity.t_brushes; eb++)
			{
				brush &Brush = Entity.Brushes[eb];
				
				// restore missing vertices of each face
				Brush.FixHoles();
			}
		}
	}
	
	#if DEBUG > 0
	if (dev) cout << "  Getting original detail brushes Texture Shifts..." << endl;
	if (dev) system("pause");
	#endif
	
	// get original detail brushes Texture Shifts
	for (int e = 0; e<EntityList.size(); e++)
	{
		entity &Entity = EntityList[e];
		if (Entity.IsDetail)
		{
			for (int eb = 0; eb<Entity.t_brushes; eb++)
			{
				brush &Brush = Entity.Brushes[eb];
				
				Brush.GetFaceShifts();
			}
		}
	}
	
	// copy entity brushes that are of a detail type into detail source object(s)
	for (int dg = 0, b = 0; dg<t_dgroups; dg++)
	{
		group &Group = mDetailGroup[dg];
		Group.Brushes = new brush[Group.t_brushes];
		
		#if DEBUG > 0
		if (dev) cout << "Created " << Group.t_brushes << " Brushes for Detail Group "<<dg<<"!" << endl;
		#endif
		
		for (int e = 0; e<EntityList.size(); e++)
		{
			entity &Entity = EntityList[e];
			if (Entity.IsDetail && Entity.dID==dg)
			{
				for (int eb = 0; eb<Entity.t_brushes; eb++)
				{
					brush &Brush = mDetailGroup[dg].Brushes[b];
					
					Brush.Copy(Entity.Brushes[eb]);
					Brush.SegID = b;
					b++;
				}
			}
		}
		b = 0;
	}
	
	for (int dg = 0; dg<t_dgroups; dg++)
		mDetailGroup[dg].MarkGroupOriginObjects();
	
	for (int dg = 0; dg<t_dgroups; dg++)
		mDetailGroup[dg].GetGroupDimensions(1,1);
}

string file::GetMapWorld(string mapfile)
{
	int ents_start = 0;
	string worldspawn = "";
	
	// are there (solid) entities?
	bool no_ents = 0;
	ents_start = mapfile.find("{\n\"classname\" \"",15);
	if (ents_start==-1)
		no_ents = 1;
	
	// are there world brushes?
	if (no_ents)
	{
		int map_end = mapfile.rfind("}\n");
		worldspawn = mapfile.substr(0, map_end);
	}
	else
	{
		worldspawn = mapfile.substr(0, ents_start-2);
	}
	
	return worldspawn;
}

string file::GetMapEnts(string mapfile)
{
	int ents_start = 0;
	int ents_end = 0;
	string entities = "";
	
	// are there (solid) entities?
	ents_start = mapfile.find("{\n\"classname\" \"",15);
	if (ents_start!=-1)
		ents_end = mapfile.rfind("}\n")+2;
	
	entities = mapfile.substr(ents_start, ents_end-ents_start);
	
	return entities;
}



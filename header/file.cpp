#include "file.h"
#include "group.h"
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
#include <conio.h> // getch
#include <iomanip> // precision

#define PI 3.14159265

using namespace std;

extern ctable *cTable;
extern ctable *sTable;
extern ctable *dTable;
extern group *bGroup;
extern group *mGroup;
extern group *dmGroup;
extern group *sGroup;
extern group_set *dSet;
extern vector<WADFile> WADFiles;
extern vertex Zero;

/* ===== FILE METHODS ===== */

// Export Brush to .map-File
void file::ExportToMap()
{
	bool dev = 0;
	if (dev) cout << " Exporting Brush to .map-File..." << endl;
	
	ofstream mapfile;
	
	// custom export file
	if (dev) cout << " Custom export file..." << endl;
	
	string str_target = "";
	if (target!="ERR"&&target!="UNSET"&&target.length()>0)
		str_target = target;
	else
		str_target = p_path+name+"_arc.map";
	
	// Console Info Message
	if (dev) cout << " Console Info Message..." << endl;
	cout << str_target;
	
	// append
	string targetFile = "";
	string targetFile_world;
	string targetFile_ents;
	if (append&&CheckIfFileExists(str_target))
	{
		targetFile 			= LoadTextFile(str_target);
		targetFile_world 	= GetMapWorld(targetFile);
		targetFile_ents 	= GetMapEnts(targetFile);
	} else append = 0;
	
	if (dev) cout << " append " << cTable[0].append << " target len " << targetFile.length() << " worldspawn len " << targetFile_world.length() << " entities len " << targetFile_ents.length() << endl;
	
	// Skipping Brushes with only Nullfaces
	for (int g = 0; g < mGroup->t_arcs; g++)
	{
		group &Group = bGroup[g];
		if (cTable[g].skipnull>0)
		Group.CheckNULLBrushes();
	}
	
	// Export to file
	mapfile.open(str_target);
	
	// Write worldspawn header
	if (dev) cout << " Worldspawn header..." << endl;
	if (append)
	mapfile << targetFile_world;
	else
	mapfile << EntityList[0].content.substr(0, EntityList[0].head_end) << endl;
	
	// Write all world brushes first (all arcs)
	if (dev) cout << " Writing all world brushes first (all arcs)..." << endl;
	for (int g = 0; g < mGroup->t_arcs; g++)
	{
		// arc settings
		int secs = bGroup[g].sections;
		
		// Iterate Brushes
		for (int b = 0; b < bGroup[g].t_brushes; b++)
		{
			brush &Brush = bGroup[g].Brushes[b];
			int sec = Brush.SecID;
			
			if (Brush.entID==0 && sec<bGroup[g].range_end && sec>=bGroup[g].range_start && Brush.draw)  // custom range (e.g. 0 to 100%) determined by range_start/end command
			{
				if (dev) cout << "  Writing Brush " << b << "..." << endl;
				if (Brush.Tri==nullptr)
				{
					mapfile << "{" << endl;
					
					// Iterate Faces
					for (int f = 0; f < Brush.t_faces; f++)
					{
						face &Face = Brush.Faces[f];
						
						if (Face.draw)
						mapfile << Face;
					}
					
					mapfile << "}" << endl;
				}
				else
				{
					for (int bt=0; bt<Brush.t_tri; bt++)
					{
						brush &TriBrush = Brush.Tri[bt];
						if (TriBrush.draw)
						{
							mapfile << "{" << endl;
							
							// Iterate Faces
							for (int f = 0; f < TriBrush.t_faces; f++)
							{
								face &FaceTB = TriBrush.Faces[f];
								
								if (FaceTB.draw)
								mapfile << FaceTB;
							}
							
							mapfile << "}" << endl;
						}
					}
				}
				
				// Gap of current Brush
				if (cTable[g].gaps>0&&Brush.Gap!=nullptr)
				{
					if (dev) cout << "   Writing Gap Brush..." << endl;
					brush &Gap = *Brush.Gap;
					mapfile << "{" << endl;
					for (int f = 0; f < Gap.t_faces; f++)
					{
						face &GFace = Gap.Faces[f];
						
						if (GFace.draw)
						mapfile << GFace;
					}
					mapfile << "}" << endl;
				}
			}
		}
	}
	mapfile << "}" << endl; // finish world brushes
	
	if (append)
	mapfile << targetFile_ents;

	// write solid entities
	if (dev) cout << " Writing solid entities..." << endl;
	for (int g = 0; g < mGroup->t_arcs; g++)
	{
		if (cTable[g].type!=2)
		for (int e = 0; e < EntityList.size(); e++) // entity loop
		{
			entity &Entity = EntityList[e];
			
			// current solid entity header
			if ( Entity.type==1 && !Entity.IsDetail )
			{
				mapfile << Entity.content.substr(0, Entity.head_end) << "\n";
				//cout << "Arc #"<<g<<" Entity ID " << e << " Entity Type " << Entity.type << endl;
				
				// Iterate Brushes
				for (int b = 0; b < bGroup[g].t_brushes; b++)
				{
					brush &Brush = bGroup[g].Brushes[b];
					int sec = Brush.SecID;
					
					if (Brush.entID==e && sec<bGroup[g].range_end && sec>=bGroup[g].range_start && Brush.draw)
					{
						//cout << "Brush.SecID: " << Brush.SecID << ", range: " << range << endl;
						if (Brush.Tri==nullptr)
						{
							mapfile << "{" << endl;
							
							// Iterate Faces
							for (int f = 0; f < Brush.t_faces; f++)
							{
								face &Face = Brush.Faces[f];
								
								if (Face.draw)
								mapfile << Face;
							}
							
							mapfile << "}" << endl;
						}
						else
						{
							for (int bt=0; bt<Brush.t_tri; bt++)
							{
								brush &TriBrush = Brush.Tri[bt];
								if (TriBrush.draw)
								{
									mapfile << "{" << endl;
									
									// Iterate Faces
									for (int f = 0; f < TriBrush.t_faces; f++)
									{
										face &FaceTB = TriBrush.Faces[f];
										
										if (FaceTB.draw)
										mapfile << FaceTB;
									}
									
									mapfile << "}" << endl;
								}
							}
						}
						// Iterate Faces
						/*
						for (int f = 0; f < Brush.t_faces; f++)
						{
							face &Face = Brush.Faces[f];
							
							if (Face.draw)
							mapfile << Face;
						}
						mapfile << "}" << endl;*/
						
						// Gap of current Brush
						if (cTable[g].gaps>0&&Brush.Gap!=nullptr)
						{
							if (dev) cout << "   Writing Gap Brush..." << endl;
							brush &Gap = *Brush.Gap;
							mapfile << "{" << endl;
							for (int f = 0; f < Gap.t_faces; f++)
							{
								face &GFace = Gap.Faces[f];
								
								if (GFace.draw)
								mapfile << GFace;
							}
							mapfile << "}" << endl;
						}
					}
				}
				mapfile << "}" << endl; // finish current solid entity
			}
		}
		else if (cTable[g].type==2)
		{
			int t_orients 	= 1;
			int t_paths 	= PathList[g].t_paths;
			// count total orientations and assign orient ID to each Brush
			//for (int sec = 0; sec<bGroup[g].sections; sec++)
			for (int b = 1, o=0; b < bGroup[g].t_brushes; b++)
			{
				brush &Brush = bGroup[g].Brushes[b];
				brush &LBrush = bGroup[g].Brushes[b-1];
				
				if (Brush.draw)
				{
					int sec = Brush.SecID;
					
					if (Brush.Align!=LBrush.Align)
					{
						o++;
						if (sec==0) o=0;
						if (b < bGroup[g].sections) t_orients++;
						Brush.oID = o;
					}
					else if (Brush.Align==LBrush.Align)
					{
						if (sec==0) o=0;
						Brush.oID = o;
					}
					
					//Brush.Faces[2].Texture = "o_" + to_string(o) + "_p" + to_string(Brush.pID) + "_e" + to_string(Brush.entID);
					//Brush.Faces[3].Texture = "o_" + to_string(o) + "_p" + to_string(Brush.pID) + "_e" + to_string(Brush.entID);
					//Brush.Faces[4].Texture = "o_" + to_string(o) + "_p" + to_string(Brush.pID) + "_e" + to_string(Brush.entID);
					//cout << " Brush " << b << " sec " << sec << " Align " << Brush.Align << " oID " << Brush.oID << " pID " << Brush.pID << endl;
				}
			}
			//cout << " total orient groups " << t_orients << endl;
			//cout << " total brushes " << bGroup[g].t_brushes;
			//cout << " total ents " << EntityList.size() << endl;
			//getch();
			
			for (int o = 0; o < t_orients; o++) // orientation loop
			for (int p = 0; p < t_paths; p++) // paths loop
			for (int e = 0; e < EntityList.size(); e++) // entity loop
			{
				entity &Entity = EntityList[e];
				
				// current solid entity header
				if (Entity.type==1)
				{
					bool wrote_head = 0;
					bool wrote_foot = 0;
					// Iterate Brushes
					for (int b = 0; b < bGroup[g].t_brushes; b++)
					{
						brush &Brush = bGroup[g].Brushes[b];
						int sec = Brush.SecID;
						//cout << "  Brush "<<b<<" Exp"; if (Brush.exported) cout << " YES"; else cout <<" NO ";
						//cout << " eID " << Brush.entID << " pID " << Brush.pID << " oID " << Brush.oID << " Draw " << Brush.draw << endl;
						
						if (!Brush.exported && Brush.entID==e && ((cTable[g].psplit==1&&Brush.oID==o)||(cTable[g].psplit==0)) && Brush.pID==p && sec<bGroup[g].range_end && sec>=bGroup[g].range_start && Brush.draw)
						{
							if (!wrote_head)
							{
								mapfile << Entity.content.substr(0, Entity.head_end) << "\n";
								wrote_head = 1;
								//cout << "    Wrote Head!"<< endl;
							}
							
							mapfile << "{" << endl;
							
							// Iterate Faces
							for (int f = 0; f < Brush.t_faces; f++)
							{
								face &Face = Brush.Faces[f];
								
								if (Face.draw)
								mapfile << Face;
							}
							mapfile << "}" << endl;
							
							Brush.exported = 1;
							//cout << "    Wrote Brush #"<<b<<"!"<< endl;
						}
						if (b==bGroup[g].t_brushes-1&&!wrote_foot&&wrote_head)
						{
							//cout << "    Wrote Footer!"<< endl;
							mapfile << "}" << endl;
							wrote_foot = 1;
						}
					}
					//mapfile << "}" << endl; // finish current solid entity
				}
			}
		}
	}

	// Bounding Boxes
	if (dev) cout << " Bounding Boxes..." << endl;
	for (int g = 0; g < mGroup->t_arcs; g++)
	{
		if (cTable[g].bound)
		{
			brush &Box = *bGroup[g].boundBox;
			mapfile << "{\n\"classname\" \"func_detail\"\n\"zhlt_detaillevel\" \"1\"\n\"zhlt_clipnodedetaillevel\" \"1\"\n";
			mapfile << "{\n";
			
			for (int f=0; f<6; f++)
			{
				face &Face = Box.Faces[f];
				mapfile << Face;
			}
			mapfile << "}\n}\n";
		}
	}
	
	// Detail Objects
	if (dev) cout << " Detail Objects..." << endl;
	for (int g = 0; g < mGroup->t_arcs; g++)
	{
		group_set &Set = dSet[g];
		
		if ( cTable[g].type==0 && cTable[g].d_enable )
		for (int dg = 0; dg < Set.t_groups; dg++)
		{
			group &dGroup = Set.Groups[dg];
			
			if ( dGroup.d_enable!=0 )
			{
				for (int e = 0; e < EntityList.size(); e++) // entity loop
				{
					entity &Entity = EntityList[e];
					
					if ( Entity.dID==dg && Entity.type == 1 && Entity.t_brushes>0 && Entity.IsDetail )
					{
						// export detail objects as whole objects
						if ( (cTable[g].d_separate==0 && dGroup.d_separate<=0) || (cTable[g].d_separate==1 && dGroup.d_separate==0) )
						{
							// current solid entity header
							mapfile << Entity.content.substr(0, Entity.head_end) << "\n";
							//cout << "Arc #"<<g<<" Entity ID " << e << " Entity Type " << Entity.type << endl;
							//mapfile << " Entity dID " << Entity.dID << " eID " << Entity.eID << " brushes " << Entity.t_brushes << endl;
							
							// Iterate Brushes
							for (int b = 0; b < dGroup.t_brushes; b++)
							{
								brush &Brush = dGroup.Brushes[b];
								int sec = Brush.SecID;
								if ( Brush.entID==e && sec<bGroup[g].range_end && sec>=bGroup[g].range_start && Brush.draw )
								{
									mapfile << "{" << endl;
									
									// Iterate Faces
									for (int f = 0; f < Brush.t_faces; f++)
									{
										face &Face = Brush.Faces[f];
										
										if (Face.draw)
										mapfile << Face;
									}
									
									mapfile << "}" << endl;
								}
							}
							mapfile << "}" << endl; // finish current solid entity
						}
						// export detail objects as individual solid objects
						else
						{
							for (int s=0; s<bGroup[g].sections; s++)
							{
								mapfile << Entity.content.substr(0, Entity.head_end) << "\n";
								if (dev) {
								cout << endl << " Writing Point Entities of dGroup..." << endl;
								cout << "   Global Autoname:" << cTable[g].d_autoname << endl;
								cout << "   Local Autoname:" << dGroup.d_autoname << endl;
								getch();}
								if (cTable[g].d_autoname>0 || dGroup.d_autoname>0) {
									string L_name = "", L_target = "";
									if(Entity.key_target!="") {
										L_target = Entity.key_target+"_"+to_string(s);
										mapfile << "\"target\" \"" << L_target << "\"" << endl;
									}
									if(Entity.key_targetname!="") {
										L_name = Entity.key_targetname+"_"+to_string(s);
										mapfile << "\"targetname\" \"" << L_name << "\"" << endl;
									}
								}
								
								for (int b = 0; b < dGroup.t_brushes; b++)
								{
									brush &Brush = dGroup.Brushes[b];
									int sec = Brush.SecID;
									
									if (sec==s && Brush.entID==e && sec<bGroup[g].range_end && sec>=bGroup[g].range_start && Brush.draw)
									{
										mapfile << "{" << endl;
										// Iterate Faces
										for (int f = 0; f < Brush.t_faces; f++) {
											face &Face = Brush.Faces[f];
											if (Face.draw)
											mapfile << Face;
										}
										mapfile << "}" << endl;
									}
								}
								mapfile << "}" << endl; // finish current solid entity
							}
						}
					}
				}
				// Export Point Entities
				for (int e = 0; e < dGroup.t_ents; e++)
				{
					entity &Entity = dGroup.Entities[e];
					int sec = Entity.SecID;
					if ( sec<bGroup[g].range_end && sec>=bGroup[g].range_start && Entity.draw )
					{
						mapfile << Entity.content.substr(0,Entity.head_end-2);
						mapfile << "\"angles\" \"" << Entity.Angles.Pitch << " " << Entity.Angles.Yaw << " " << Entity.Angles.Roll << "\"" << endl;
						mapfile << "\"origin\" \"" << Entity.Origin.x << " " << Entity.Origin.y << " " << Entity.Origin.z << "\"" << endl;
						if (cTable[g].d_autoname>0 || dGroup.d_autoname>0) {
							string L_name = "", L_target = "";
							if(Entity.key_target!="") {
								L_target = Entity.key_target+"_"+to_string(sec);
								mapfile << "\"target\" \"" << L_target << "\"" << endl;
							}
							if(Entity.key_targetname!="") {
								L_name = Entity.key_targetname+"_"+to_string(sec);
								mapfile << "\"targetname\" \"" << L_name << "\"" << endl;
							}
						}
						mapfile << "}" << endl;
					}
				}
			}
		}
	}
	
	mapfile << "\n";
	mapfile.close();
}


// Export Original Map to Map again
void file::ExportToMapO(string p)
{
	if (p=="") p = p_path+name+"_original.map";
	
	ofstream mapfile;
	mapfile.open(p);

	// worldspawn header
	mapfile << EntityList[0].content.substr(0, EntityList[0].head_end) << endl;
	
	for (int g = 0; g < mGroup->t_arcs; g++)
	{
		// write all world brushes first (all arcs)
		// Iterate Brushes
		for (int b = 0; b < sGroup[g].t_brushes; b++)
		{
			brush &Brush = sGroup[g].Brushes[b];
			
			mapfile << "{" << endl;
			
			// Iterate Faces
			for (int f = 0; f < Brush.t_faces; f++)
			{
				face &Face = Brush.Faces[f];
				
				mapfile << setprecision(8) << fixed;
				mapfile << Face.Vertices[0] << Face.Vertices[1] << Face.Vertices[2];
				mapfile << setprecision(8) << fixed;
				
				mapfile << " " << Face.Texture;
				mapfile << " [ ";
				mapfile << Face.VecX.x << " ";
				mapfile << Face.VecX.y << " ";
				mapfile << Face.VecX.z << " ";
				mapfile << Face.ShiftX << " ";
				mapfile << "] [ ";
				mapfile << Face.VecY.x << " ";
				mapfile << Face.VecY.y << " ";
				mapfile << Face.VecY.z << " ";
				mapfile << Face.ShiftY << " ";
				mapfile << "] ";
				mapfile << Face.Rot << " ";
				mapfile << Face.ScaleX << " ";
				mapfile << Face.ScaleY << " ";
				mapfile << endl;
			}
			mapfile << "}" << endl;
		}
	}
	mapfile << "}" << endl;
	
	mapfile << endl;
	mapfile.close();
}

void file::GetInfo() { //get filename and path from full path
	
	// check if path is absolute or relative first
	string bracket = "\\";
	int ext_start = fullpath.rfind(".");
	int name_start = fullpath.rfind(bracket)+1;
	
	if (name_start!=-1)	p_path = fullpath.substr(0,name_start);
	else				p_path = "";
	if (ext_start!=-1)	name = fullpath.substr(name_start, ext_start-name_start);
	else				name = fullpath.substr(0, ext_start);
	//cout << " path: " << path << " name: " << name << endl;
	
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
    //cout << " Type: " << type << " path_cfg: " << path_cfg << " path_map: " << path_map << endl;
 	if (type==1) {
		cout << "|    Type: Settings File (*.txt): \t" << endl;
		cout << "|    Path: " << fullpath << endl;
	} else {
		cout << "|    Type: Map File (*.map)" << endl;
		cout << "|    Path: " << fullpath << endl;
		//if (InternalMapSettings)
		//cout << "|    Map Contains Setting Entities (info_curve)!" << endl;
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
		//cout << " Searching Map-File for info_curve..." << endl;
		bool Found_IC = 0; if(str_map.find("\"classname\" \"info_curve\"")!=-1) Found_IC = 1;
		if (Found_IC) //||str_map.find("\"classname\" \"info_curve_export\"")!=-1
		{
			//cout << " Found at least one info_curve in Map-File ("<<name<<")!" << endl;
			InternalMapSettings = 1; // internal settings entity "e.g. info_curve" was found
		}
		//else cout << " Found no info_curve in Map-File ("<<name<<")!" << endl;
	}
	
	if (str_cfg=="ERR") valid_cfg = 0;
	
	if (!valid_map) {
		cout << "|    [ERROR] Map-File not found or empty! Aborting..." << endl;
		cout << "|            File: "<< str_map_temp << endl;
		cout << "|" << endl;
	}
	
	if (type==2&&!InternalMapSettings)
	{
		if (!valid_cfg) {
			cout << "|    [WARNING] No valid Preset-File (TXT) or Entity (info_curve) found in Map-File!" << endl;
			cout << "|              Using Default Settings..." << endl;
			cout << "|" << endl;
		} else {
			cout << "|    [INFO] No internal Preset-Entity (info_curve) found in Map-File!" << endl;
			cout << "|           Using external Settings File ("<<path_cfg<<")..." << endl;
			cout << "|" << endl;
		}
	}
	else if (type==2&&InternalMapSettings) {
	cout << "|    [INFO] Map-internal Preset-Entity (info_curve) found!" << endl;
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
				if (value!="ERR"&&CheckIfFileExists(value))
				{
					cout << "|    [INFO] Custom source file ("<< value <<") found!" << endl;
					cout << "|" << endl;
					path_map = value;
					break;
				} else
					cout << "|    [ERROR] Custom source file not found! Trying original source file ("<<name+".map"<<")..." << endl;
					cout << "|" << endl;
			}
			f_pos = str_cfg.find(phrase, f_pos+1);
		}
	}
}

/*string file::GetCustomPhraseValue(string phrase, string hay)
{
	if (hay!="ERR"&&hay.length()>0)
	{
		int f_pos = 0;
		string value = "";
		while (f_pos!=-1)
		{
			f_pos = hay.find(phrase, f_pos);
			if (CheckPhrase(hay, f_pos))
			{
				value = GetValue(hay, f_pos+phrase.length());
				if (value!="ERR")
				{
					return value;
				}
			}
			f_pos = hay.find(phrase, f_pos+1);
		}
	}
}*/

void file::ExportToObj()
{
	for (int g = 0; g < mGroup->t_arcs; g++)
	{
		if (cTable[g].obj&&bGroup[g].valid) // wether or not this arc is being exported to an obj file
		{
			ofstream objfile;
			objfile.open(p_path+name+"_"+to_string(g+1)+".obj");
			
			for (int b = 0; b < bGroup[g].t_brushes; b++)
			{
				brush &Brush = bGroup[g].Brushes[b];
				int sec = Brush.SecID;
				
				if (sec < bGroup[g].range_end && sec >= bGroup[g].range_start && Brush.draw)  // custom range (e.g. 0 to 90 degree) determined by arc command in settings file
				{
					if (Brush.Tri==nullptr)
					{
						objfile << "g brush_export" << b << endl;
						
						for (int f = 0; f < Brush.t_faces; f++)
						{
							face &Face = Brush.Faces[f];
							int tverts = Face.vcount;
							
							if (Face.draw)
							{
								for(int v = 0; v < tverts; v++)
								{
									objfile << "v " << Face.Vertices[v].x << " " << Face.Vertices[v].y << " " << Face.Vertices[v].z << " " << endl;
								}
								
								objfile << "f";
								
								for(int i = 0; i < tverts; i++)
								{
									objfile << " -" << i+1;
								}
								
								objfile << endl << endl;
							}
						}
					}
					else
					{
						for (int bt=0; bt<Brush.t_tri; bt++)
						{
							brush &TriBrush = Brush.Tri[bt];
							objfile << "g brush_export" << b << endl;
							
							for (int f = 0; f < TriBrush.t_faces; f++)
							{
								face &Face = TriBrush.Faces[f];
								int tverts = Face.vcount;
								
								if (Face.draw)
								{
									for(int v = 0; v < tverts; v++)
									{
										objfile << "v " << Face.Vertices[v].x << " " << Face.Vertices[v].y << " " << Face.Vertices[v].z << " " << endl;
									}
									
									objfile << "f";
									
									for(int i = 0; i < tverts; i++)
									{
										objfile << " -" << i+1;
									}
									
									objfile << endl << endl;
								}
							}
						}
					}
				}
			}

			// Gaps
			if (cTable[g].gaps>0)
			for (int b = 0; b < bGroup[g].t_brushes; b++)
			{
				if(bGroup[g].Brushes[b].Gap!=nullptr)
				{
					brush &Gap = *bGroup[g].Brushes[b].Gap;
					int sec = Gap.SecID;
	
					if (sec<bGroup[g].range_end && sec>=bGroup[g].range_start)
					{
						objfile << "g brush_export" << b << endl;
						
						for (int f=0; f<Gap.t_faces; f++)
						{
							face &Face = Gap.Faces[f];
							int tverts = Face.vcount;
							if (Face.draw)
							{
								for(int v = 0; v < tverts; v++)
								{
									objfile << "v " << Face.Vertices[v].x << " " << Face.Vertices[v].y << " " << Face.Vertices[v].z << " " << endl;
								}
								
								objfile << "f";
								
								for(int i = 0; i < tverts; i++)
								{
									objfile << " -" << i+1;
								}
								
								objfile << endl << endl;
							}
						}
						objfile << endl << endl;
					}
				}
			}
			objfile.close();
		}
	}
}


void file::texturize(int g)
{
	bool dev = 0;
	
	if (dev) cout << "Adding basic Texture Shifts" << endl;
	// Fix Texture Scale and Shift for Body Faces that are smaller than before (happens on curve generation)
	//for (int g = 0; g < mGroup->t_arcs; g++) // Arc Object loop (Total Arcs)
	for (int b=0; b<bGroup[g].t_brushes; b++)
	{
		brush &Brush = bGroup[g].Brushes[b];
		int seg = Brush.SegID;
		int sec = Brush.SecID;
		
		brush *LBrush = nullptr;
		if (sec>0) LBrush = &bGroup[g].Brushes[b-1];
		
		for (int f = 0, c=0; f<Brush.t_faces; f++)
		{
			face &Face  = Brush.Faces[f];
			// Fix vertical Body Face Scales, if New Vertical Face Lengths differ from old ones
			float mx = 1.0;
			float my = 1.0;
			
			if (Face.fID==2/*||(Face.fID<=1&&cTable[g].type==2)*/)
			{
				if (Face.LengthO!=Face.LengthN)
				{
					float m = Face.LengthO/Face.LengthN;
					if (Face.VecX.IsHor) 	my = m;
					else 					mx = m;
				}
			}
			float msh = 1;
			float msx = 1.0/(Face.ScaleX);
			float msy = 1.0/(Face.ScaleY);
			if (Face.VecX.IsHor)  msh = msx; else msh = msy;
			
			// Compose Face Shift
			Face.ShiftX = Face.BaseShiftX * mx;
			Face.ShiftY = Face.BaseShiftY * my;
			Face.ScaleX /= mx;
			Face.ScaleY /= my;
			/*
			FaceT.ShiftX = FaceT.BaseShiftX * mx;
			FaceT.ShiftY = FaceT.BaseShiftY * my;
			FaceT.ScaleX /= mx;
			FaceT.ScaleY /= my;
			*/
			// Add Shifts to horizontal Body Faces
			if (Face.fID==2)
			{
				face *hFace = nullptr;
				float Temp_BaseShift = 0; 
				float Temp_EdgeLen = 0;

				if (cTable[g].shift==0)
				{
					if (Face.VecX.IsHor) 	{ Face.ShiftX = 0; Face.OffsetX = 0; }
					else 				 	{ Face.ShiftY = 0; Face.OffsetY = 0; }
				}
				else if (cTable[g].shift>0 && cTable[g].shift<6)
				{
					hFace = Face.HSourceL;
					if (hFace->VecX.IsHor) Temp_BaseShift = hFace->BaseShiftX; else Temp_BaseShift = hFace->BaseShiftY;
					if (sec>0) Temp_EdgeLen = LBrush->Faces[f].HShiftL * msh;
					
					if ( (!Face.VecH->IsNeg && hFace->VecH->IsNeg) || (Face.VecH->IsNeg && !hFace->VecH->IsNeg) )
					{
						Temp_BaseShift *= -1; // do this because positive hor tex vectors Baseshift is at wrong side (aligned R instead of L)
						Temp_EdgeLen *= -1;
					}
					if (hFace->VecH->IsNeg) Temp_EdgeLen *= -1;
					
					if (sec>0&&cTable[g].shift!=4)
						if (Face.VecX.IsHor) 	{ Face.ShiftX = Temp_BaseShift + Temp_EdgeLen; Face.OffsetX = Temp_EdgeLen; }
						else 				 	{ Face.ShiftY = Temp_BaseShift + Temp_EdgeLen; Face.OffsetY = Temp_EdgeLen; }
					else
						if (Face.VecX.IsHor) 	{ Face.ShiftX = Temp_BaseShift; Face.OffsetX = 0; }
						else 				 	{ Face.ShiftY = Temp_BaseShift; Face.OffsetY = 0;}
					
					//if (Face.VecX.IsHor) 	Face.ShiftX += Face.OffsetX;
					//else 				 	Face.ShiftY += Face.OffsetY;
				}
			}
			
			// Add Offset to vertical body and head/base Face Shift
			if (Face.fID==2)
			{
				if (Face.VecX.IsHor) 	Face.ShiftY += Face.OffsetY;
				else 				 	Face.ShiftX += Face.OffsetX;
			} else {
										Face.ShiftY += Face.OffsetY;
										Face.ShiftX += Face.OffsetX;
			}
			
			/*
			if (Face.fID==2)
				if (Face.VecX.IsHor) 	Face.ShiftX = 0 + Face.OffsetX;
				else 					Face.ShiftY = 0 + Face.OffsetY;*/
			
			/*if (Face.fID==2)
				if (Face.VecX.IsHor) 	Face.ShiftX = Face.BaseShiftX;// + Face.OffsetX;
				else 					Face.ShiftY = Face.BaseShiftY;// + Face.OffsetY;*/
		}
	}
	
	// fix Gap Textures
	if (cTable[g].gaps>0&&cTable[g].type<=1)
	for (int b=0; b<bGroup[g].t_brushes; b++)
	{
		brush &Gap = *bGroup[g].Brushes[b].Gap;
		for (int f = 0; f<Gap.t_faces; f++)
		{
			face &Face  = Gap.Faces[f];
			GetBaseEdges(Face);
			GetBaseShift(Face,0,1,0);
			
			Face.ShiftX = Face.BaseShiftX + Face.OffsetX;
			Face.ShiftY = Face.BaseShiftY + Face.OffsetY;
			//cout << " Gap Face BaseShiftX " << Face.BaseShiftX << " + OffsetX " << Face.OffsetX << " = ShiftX " << Face.ShiftX << endl;
			//cout << " Gap Face BaseShiftY " << Face.BaseShiftY << " + OffsetY " << Face.OffsetY << " = ShiftY " << Face.ShiftY << endl;
		}
	}
}

void file::createBounds(int g)
{
	// copy ranges to detail group
	for(int d = 0; d<dSet[g].t_groups; d++) {
		dSet[g].Groups[d].range_end = bGroup[g].range_end;
		dSet[g].Groups[d].range_start = bGroup[g].range_start;
	}
	
	// get total dimensions of this arc and create a bounding box afterwards
	bGroup[g].GetDimensions(0); // brush group
	dSet[g].GetDimensions(0); // detail group
	
	//cout << " bGroup #"<<g<<" Dimensions " << bGroup[g].Dimensions << endl;
	//cout << " dGroup #"<<g<<" Dimensions " << dSet[g].Dimensions << endl;
	
	dimensions AllCombined;
	if (t_dgroups>0) AllCombined = DimensionCombine(bGroup[g].Dimensions, dSet[g].Dimensions);
	else AllCombined = bGroup[g].Dimensions;
	
	AllCombined.expand(64);
	bGroup[g].boundBox->MakeCuboid(AllCombined, "NULL");
}

void file::buildArcs(int g)
{
	bool dev = 0;
	if (dev)cout << "Constructing Arcs..."  << endl;
	
	// smooth ramps need special height table
	if (dev)cout << "  Smooth ramps height table..."  << endl;
	bGroup[g].CreateHeightTableSmooth();
	
	// build the curve object from previously created construction framework
	if (dev)cout << "  Building curve object from previously created construction framework..."  << endl;
	bGroup[g].Build();
	
	group &Group = sGroup[g];
	for (int b=0; b<bGroup[g].t_brushes; b++)
	{
		brush &Brush = bGroup[g].Brushes[b];

		// check for Brushes with less than 3 valid faces (minimum valid brush is a wedge with 4 faces)
		// this occurs because of the path generation process
		for (int f = 0, inv=0; f<Brush.t_faces; f++)
		{
			face &Face = Brush.Faces[f];
			if (Face.draw==0) inv++;
			if (f==Brush.t_faces-1&&Brush.t_faces-inv<3) { Brush.draw = 0; Brush.valid = 0; }
		}
	}
	
	if (dev)cout << "  Marking Head Vertices..."  << endl;
	bGroup[g].GetHeadVertices();
	
	if (dev)cout << "  Adding Simple Heights to Brushes..."  << endl;
	bGroup[g].AddBrushHeights();
	
	/*
	// for DEV PURPOSES write all coordinates into the faces texture name
	for (int b = 0; b<bGroup[g].t_brushes; b++)
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
	
	if (dev)cout << "  Getting Body Face Lengths..."  << endl;
	bGroup[g].GetBrushBodyFaceLengths();
	
	// get exact centroid of all faces
	if(dev) cout << "  Getting Face Centroids..." << endl;
	bGroup[g].GetBrushFaceCentroids();
	
	// Get Section Lengths and source faces for future horizontal texture shifts
	if (dev)cout << "  Getting Hor Lengths..."  << endl;
	bGroup[g].GetHorLengths();
	
	// Create middle-sections between the existing curve sections for mapping purposes
	if(dev) cout << "  Creating Gaps..." << endl;
	bGroup[g].CreateBrushGaps();
	if(dev) cout << "  Arranging Gaps..." << endl;
	bGroup[g].ArrangeGaps();
	
	// Rotate the Texture Vectors of all Brush Faces
	if(dev) cout << "  Rotating Tex Vectors..." << endl;
	bGroup[g].RotateVectors();
	
	// GetBaseEdges, Face Normals and Baseshift
	if(dev) cout << "  Getting Base Edges and Shifts..." << endl;
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
	RotateDetailObj(g);
	
	// Print updated Section Base Faces
	/*for (int g = 0; g < mGroup->t_arcs; g++)
	{
		for (int i = 0; i<cTable[g].res; i++)
		{
			bGroup[g].SecBaseFace[i]->IsSecBase = 1;
			cout << " Section " << i << " Base Face fID " << bGroup[g].SecBaseFace[i]->fID << " Tex " << bGroup[g].SecBaseFace[i]->Texture << " EdgeLenL " << bGroup[g].SecBaseFace[i]->EdgeLenL << " Base X " << bGroup[g].SecBaseFace[i]->BaseX << " BaseY " << bGroup[g].SecBaseFace[i]->BaseY << endl;
		}
	}*/
}

void file::GetInternalMapSettings()
{
	if (InternalMapSettings)
	{
		// count info_curve entities
		for (int e=0; e<EntityList.size(); e++)
			if (EntityList[e].key_classname=="info_curve") t_iarcs++;
		
		// create mTables
		//mTable = new ctable[t_iarcs];
		
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
	bool dev = 0;
	// fix position of basic detail objects
	for (int g = 0; g<mGroup->t_arcs; g++) {
		for (int i = 0; i<t_dgroups; i++)
		{
			if (dev) cout << " Fixing X-axis Pos of detail group " << i << " of arc " << g << endl;
			group &Group = dSet[g].Groups[i];
			// get offset, if X position of objects origin is not 0
			if (Group.Origin.x!=0)
			{
				if (dev) cout << "   Group Origin is not 0 -> " << Group.Origin << " Moving all brushes ("<<Group.t_brushes<<") and Entities ("<< Group.t_ents <<") by "<<-(Group.Origin.x)<<" on X axis..." << endl;
				for (int b = 0; b<Group.t_brushes; b++)
				{
					brush &Brush = Group.Brushes[b];
					if (dev) cout << "     Brush " << b << " moving on X by " << -(Group.Origin.x) << endl;
					Brush.Move(-(Group.Origin.x),0,0,1);
				}
				for (int k = 0; k<Group.t_ents; k++)
				{
					entity &Entity = Group.Entities[k];
					if (dev) cout << "     Entity " << k << " Origin " << Entity.Origin << " moving on X by " << -(Group.Origin.x);
					Entity.Origin.x -= Group.Origin.x;
					if (dev) cout << " Origin now " << Entity.Origin << endl;
				}
			}
			Group.Origin.x = 0;
			if (dev) cout << "   New Group Origin is " <<  Group.Origin << endl << endl;
		}
	}
	if (dev) getch();
}


// Move and Rotate Detail Objects
void file::RotateDetailObj(int g)
{
	bool dev = 0;
	if (dev) cout << " Move and Rotate Detail Objects..." << endl;
	if (cTable[g].type==0 && cTable[g].d_enable>0)
	{
		group_set &Set = dSet[g];
		float step = 360.0/cTable[g].res;
		int res = cTable[g].res;
		float offset = cTable[g].offset_NO; // difference (offset) between new and old (N/O) curve radius (original map position and generated one from rad+offset setting)
		float dangle[res], init_degZ[res], multi[res], degY = 0;
		vertex IndiOrigin[res];
		int start = bGroup[g].range_start;
		int end = bGroup[g].range_end;
		
		bool G_YAW   = 1; if (cTable[g].d_autoyaw!=1)     G_YAW = 0;
		bool G_PITCH = 1; if (cTable[g].d_autopitch!=1) G_PITCH = 0;
		bool G_ENABLE = 1; if (cTable[g].d_enable!=1)  G_ENABLE = 0;
		bool G_POS_RAND = 1; if (!cTable[g].d_pos_rand.IsSet||cTable[g].d_pos_rand.x==0)  G_POS_RAND = 0;
		bool G_RZ_RAND = 1; if (!cTable[g].d_rotz_rand.IsSet||cTable[g].d_rotz_rand.x==0)  G_RZ_RAND = 0;
		bool G_MY_RAND = 1; if (!cTable[g].d_movey_rand.IsSet||cTable[g].d_movey_rand.x==0)  G_MY_RAND = 0;
		
		if (dev) cout << " G_YAW " << G_YAW << " G_PITCH " << G_PITCH << " G_ENABLE " << G_ENABLE << " G_ANGLE " << cTable[g].d_pos << endl; 
		if (dev) cout << " Step " << step << " offset " << offset << endl;
		for (int d = 0; d<Set.t_groups; d++)
		{
			group &dGroup = Set.Groups[d];
			bool L_YAW   = 1; if (dGroup.d_autoyaw>0)     L_YAW = 1; else if (G_YAW&&dGroup.d_autoyaw<0) L_YAW = 1;			else L_YAW = 0;
			bool L_PITCH = 1; if (dGroup.d_autopitch>0) L_PITCH = 1; else if (G_PITCH&&dGroup.d_autopitch<0) L_PITCH = 1;	else L_PITCH = 0;
			bool L_ENABLE = 1; if (dGroup.d_enable>0) L_ENABLE = 1;  else if (G_ENABLE&&dGroup.d_enable<0) L_ENABLE = 1;	else L_ENABLE = 0;
			bool L_POS_RAND = 1; if (!dGroup.d_pos_rand.IsSet||dGroup.d_pos_rand.x==0) L_POS_RAND = 0;
			bool L_RZ_RAND = 1; if (!dGroup.d_rotz_rand.IsSet||dGroup.d_rotz_rand.x==0) L_RZ_RAND = 0;
			bool L_MY_RAND = 1; if (!dGroup.d_movey_rand.IsSet||dGroup.d_movey_rand.x==0) L_MY_RAND = 0;
			
			if (dev) cout << " L_YAW " << L_YAW << " L_PITCH " << L_PITCH << " L_ENABLE " << L_ENABLE << " L_angle " << dGroup.d_pos << endl;
			if (L_ENABLE)
			{
				dGroup.GetDimensions(1);
				if (dev) cout << "  Detail Group " << d << " Origin " << dGroup.Origin << endl;
				
				// create individual Origins for all subgroups
				for(int i=0;i<res;i++)
				IndiOrigin[i] = dGroup.Origin;
				
				// initial Z angle calculation per detail group
				if (G_POS_RAND||L_POS_RAND) {
					if (G_POS_RAND&&!L_POS_RAND) {
						for(int i=0;i<res;i++) {
							dangle[i] = GetRandInRange(cTable[g].d_pos_rand.y,cTable[g].d_pos_rand.z);
							if (dev)cout << " global d_pos #" << i << " - " << dangle[i] << " min " << cTable[g].d_pos_rand.y << " max " << cTable[g].d_pos_rand.z << endl; } }
					else {
						for(int i=0;i<res;i++) {
							dangle[i] = GetRandInRange(dGroup.d_pos_rand.y,dGroup.d_pos_rand.z);
							if (dev)cout << " local d_pos #" << i << " - " << dangle[i] << " min " << dGroup.d_pos_rand.y << " max " << dGroup.d_pos_rand.z << endl; } }
				} else {
					if ( dGroup.d_pos!=-1 )	for(int i=0;i<res;i++) dangle[i] = dGroup.d_pos;
					else						for(int i=0;i<res;i++) dangle[i] = cTable[g].d_pos;
				}
				
				for(int i=0;i<res;i++) init_degZ[i] = step*dangle[i];
				for(int i=0;i<res;i++) multi[i] = init_degZ[i]/step;
				
				if (dev) for(int i=0;i<res;i++) cout << " init_degZ " << init_degZ[i] << " multi " << multi[i] << " dangle " << dangle[i] << endl;
				
				// calculate Y axis compensation of objects, that have an initial z rotation greater than 0
				float CompenY[res];
				for(int i=0;i<res;i++)
				if (dangle[i]>0&&dangle[i]<1)
				{
					if (dev) cout << " Calculating Y axis distance compensation..." << endl;
					vertex OriginOffset;
					OriginOffset.y = dGroup.Origin.y + offset;
					
					vertex OriginRotStep = OriginOffset;
					OriginRotStep.rotate(0,0,-step);
					gvector VecStep(OriginRotStep.x,OriginRotStep.y,OriginRotStep.z);
					
					gvector VecSpan = GetVector(OriginOffset, OriginRotStep);
					VecSpan.flip();
					gvector Hypo(0,dGroup.Origin.y+offset,0);
					
					float alpha, beta, gamma;
					alpha = GetVecAng(Hypo, VecSpan);
					beta  = init_degZ[i];
					gamma = 180-alpha-beta;
					alpha *= PI / 180; beta *= PI / 180; gamma *= PI / 180;
					
					CompenY[i] = OriginOffset.y - ( Hypo.y / sin(gamma) ) * sin(alpha);
					if (dev) cout << " OriginOffset " << OriginOffset << "  \n VecStep " << VecStep << "  \n VecSpan " << VecSpan << "  \n Hypo " << Hypo << "  \n alpha " << alpha/PI*180 <<"  \n beta " << beta/PI*180 <<"  \n gamma " << gamma/PI*180 << "  \n CompenY " << CompenY << endl << endl;
				}
				// circle from which to get elevation
				circle CircElev;
				if ( cTable[g].height>0 && cTable[g].ramp>0 && cTable[g].type!=2)
				{
					if (dev) cout << " Creating circle from which to get elevation... " << endl;
					vertex OriginOff = dGroup.Origin;
					OriginOff.y += offset; //+(dGroup.SizeY/2)
					if (dev) cout << " dGroup.SizeY " << dGroup.SizeY << " dGroup.Origin " << dGroup.Origin << " offset " << offset << " OriginOff.y " << OriginOff.y << endl;
					CircElev.build_circlePi(cTable[g].res,OriginOff.y,0);
					CircElev.tverts = cTable[g].res;
					
					if (cTable[g].ramp==1)
					{
						CircElev.Vertices[1].z += cTable[g].height;
						gvector VecElev = GetVector(CircElev.Vertices[0],CircElev.Vertices[1]);
						gvector VecBase = VecElev;
						VecBase.z = 0;
						degY = GetVecAng(VecElev, VecBase);
					}
					else if (cTable[g].ramp==2)
					{
						for (int i = 0; i<cTable[g].res+1; i++)
						{
							if (i>=start && i<end)
							{
								int j = i+1;
								float height = bGroup[g].heightTable[i-start];
								//cout << " height = bGroup[g "<<g<<"].heightTable[i-start "<< i-start <<"]" << bGroup[g].heightTable[i-start] << endl;
								//if (dev) cout << "        i["<<i<<"] Increasing Z of ElevCircle #["<<j<<"] by [" << height <<  "] (htable["<<i-start<<"]: "<<bGroup[g].heightTable[i-start]<<") from CircElev.V["<<j<<"].Z " << CircElev.Vertices[j].z << " to [" << CircElev.Vertices[j].z+height << "]" << endl;
								CircElev.Vertices[j].z += height;
								if (i==cTable[g].res-start)
								CircElev.Vertices[j].z = CircElev.Vertices[1].z + CircElev.Vertices[j-1].z;
								//cout << " height = bGroup[g "<<g<<"].heightTable[i-start "<< i-start <<"]" << bGroup[g].heightTable[i-start] << endl;
							}
						}
					}
					
					if (dev) cout << "   Generated PI Circle for height calculation: HEIGHT/BASE..." << endl;
					if (dev)
					for (int v=0;v<CircElev.tverts+1;v++) cout << "     v " << v<< CircElev.Vertices[v] <<endl;
					//for (int v=0;v<CircElevBase.tverts;v++) cout << "     v " << v<< CircElevBase.Vertices[v] <<endl;
				}
				// fixing offset N/O (Y axis)
				if (cTable[g].offset_NO!=0)
				{
					if (dev) cout << " Fixing offset N/O (Y axis)..." << endl;
					for (int b = 0; b<dGroup.t_brushes; b++) // Brushes
					{
						brush &Brush = dGroup.Brushes[b];
						Brush.Move(0,cTable[g].offset_NO,0,1);
					}
					for (int e = 0; e<dGroup.t_ents; e++) // Entities
					{
						entity &Entity = dGroup.Entities[e];
						Entity.Origin.y += cTable[g].offset_NO;
					}
					if (dev) cout << "   Moved Brushes and Entities by "<<cTable[g].offset_NO<<" along Y axis!" << endl;
					
					for(int i=0;i<res;i++)
						IndiOrigin[i].y += cTable[g].offset_NO;
				}
				
				float OffsetY_rand[res];
				for(int i=0;i<res;i++)
					OffsetY_rand[i] = 0;
				// for each subgroup generate random offset on Y axis within given limits
				if (G_MY_RAND||L_MY_RAND)
				for(int i=0;i<res;i++)
				{
					if (G_MY_RAND&&!L_MY_RAND)
						OffsetY_rand[i] = GetRandInRange(cTable[g].d_movey_rand.y,cTable[g].d_movey_rand.z);
					else
						OffsetY_rand[i] = GetRandInRange(dGroup.d_movey_rand.y,dGroup.d_movey_rand.z);
				}
				
				// add additional Y offset if initial Z angle is not equal to step
				//if (dangle[i]>0&&dangle[i]<1)
				//{
					if (dev) cout << " Adding additional Y offset to compensate low circle resolutions..." << endl;
					for (int b = 0; b<dGroup.t_brushes; b++) // Brushes
					{
						brush &Brush = dGroup.Brushes[b];
						int sec = Brush.SecID;
						if (dangle[sec]>0&&dangle[sec]<1)
						Brush.Move(0,-CompenY[sec]+OffsetY_rand[sec],0,1);
					}
					for (int e = 0; e<dGroup.t_ents; e++) // Entities
					{
						entity &Entity = dGroup.Entities[e];
						int sec = Entity.SecID;
						if (dangle[sec]>0&&dangle[sec]<1)
						Entity.Origin.y += -CompenY[sec]+OffsetY_rand[sec];
					}
					for(int i=0;i<res;i++)
					if (dangle[i]>0&&dangle[i]<1)
						IndiOrigin[i].move(0,-CompenY[i]+OffsetY_rand[i],0);
					//dGroup.Origin.move(0,-CompenY,0);
				//}
				
				
				// Rotate randomly around Z axis BEFORE Y Rotation (Pitch) is applied!
				if (L_RZ_RAND||G_RZ_RAND)
				{
					float min = 0, max = 0;
					if (G_RZ_RAND&&!L_RZ_RAND) {
						min = cTable[g].d_rotz_rand.y;
						max = cTable[g].d_rotz_rand.z;
					} else {
						min = dGroup.d_rotz_rand.y;
						max = dGroup.d_rotz_rand.z;
					}
					float degZR[res];
					for(int i=0;i<res;i++) degZR[i] = GetRandInRange(min, max);
					
					for (int b = 0; b<dGroup.t_brushes; b++) // Brushes
					{
						brush &Brush = dGroup.Brushes[b];
						int sec = Brush.SecID;
						Brush.RotOrigin(0,0,degZR[sec],IndiOrigin[sec]);
					}
					for (int e = 0; e<dGroup.t_ents; e++) // Entities
					{
						entity &Entity = dGroup.Entities[e];
						int sec = Entity.SecID;
						Euler RotAngles(0,0,degZR[sec]);
						Entity.RotateEntity(RotAngles,1);
						Entity.RotateOrigin(0,0,degZR[sec], IndiOrigin[sec]);
					}
				}
				
				// Rotate around Y (PITCH)
				if ( cTable[g].height>0 && cTable[g].ramp>0 && cTable[g].type!=2 && ( (G_PITCH&&L_PITCH) || (!G_PITCH&&L_PITCH) ) )
				{
					if (dev) cout << " Rotate around Y, d_autopitch is " << dGroup.d_autopitch << endl;
					// Brushes
					for (int b = 0; b<dGroup.t_brushes; b++)
					{
						brush &Brush = dGroup.Brushes[b];
						int sec = Brush.SecID;
						if (dev) cout << "      B " << b << " Sec " << sec << " start " << start << endl;
						
						if (dev) cout << "      Creating Ramp!" << endl;
						int secb = sec+1;
						if (cTable[g].ramp==2)
						{
							gvector VecElev = GetVector(CircElev.Vertices[sec],CircElev.Vertices[secb]);
							gvector VecBase = VecElev;
							VecBase.z = 0;
							if (dev) cout << "        Creating Vector from Vertices "<<sec << CircElev.Vertices[sec] <<" and " << secb << CircElev.Vertices[secb] << " = " << VecElev << endl;
							degY = GetVecAng(VecElev, VecBase);
							if (dev) cout << "        Getting Elevation from 2 Vectors VecElev "<< VecElev <<" and VecBase " << VecBase << " = " << -degY << endl;
							if (dev) cout << "        Pitch of Elev Vector:" << -(degY) << endl;
						}
						
						if ( degY!=0 )
						{
							Brush.RotOrigin(0,-(degY),0,IndiOrigin[sec]);//dGroup.Origin
							if (dev) cout << "        Previously rotating around Y axis and Origin ("<<dGroup.Origin<<") by " << -(degY)  << " degrees!" << endl;
						}
					}
					// Point Entities
					for (int e = 0; e<dGroup.t_ents; e++)
					{
						entity &Entity = dGroup.Entities[e];
						int sec = Entity.SecID;
						int secb = sec+1;
						if (cTable[g].ramp==2)
						{
							gvector VecElev = GetVector(CircElev.Vertices[sec],CircElev.Vertices[secb]);
							gvector VecBase = VecElev;
							VecBase.z = 0;
							degY = GetVecAng(VecElev, VecBase);
						}
						if ( degY!=0 )
						{
							if (d==0&&dev) cout << " DGroup " << d << " t_ents " << dGroup.t_ents << " sec " << sec << " Rot around Y by " << degY  << " deg! Old Euler " << Entity.Angles;
							Entity.Origin.rotateOrigin(0,-(degY),0,IndiOrigin[sec]);//dGroup.Origin
							Euler RotAngles(0,-degY,0);
							Entity.RotateEntity(RotAngles,1);
							if (d==0&&dev) cout << "New Euler " << Entity.Angles << endl;
							if (sec==bGroup[g].sections-1&&dev) cout << endl;
							if (dev) cout << "        Rotating Entity Euler around Y axis and Origin ("<<dGroup.Origin<<") by " << -(degY)  << " degrees! Euler now " << Entity.Angles << endl;
						}
					}
				}
				
				// update dgroup origin if N/O Offset is not 0
				if (offset!=0) {
					dGroup.GetDimensions(1);
				}
				
				// Rotate around Z axis
				if (dev) cout << " DGroup " << d << " Rotate around Z, d_autoyaw is " << dGroup.d_autoyaw << " start angle is " << dGroup.d_pos << endl;
				float OffsetX, OffsetY;
				for (int b = 0; b<dGroup.t_brushes; b++) // Brushes
				{
					brush &Brush = dGroup.Brushes[b];
					int sec = Brush.SecID;
					float deg = (step*sec)+init_degZ[sec];
					vertex OrigBAK = IndiOrigin[sec]; //dGroup.Origin;
					OrigBAK.rotate(0,0,-deg);
					OffsetX = OrigBAK.x - IndiOrigin[sec].x; //dGroup.Origin.x;
					OffsetY = OrigBAK.y - IndiOrigin[sec].y; //dGroup.Origin.y;
					Brush.Move(OffsetX,OffsetY,0,1);
					float degB;
					//cout << " global yaw " << G_YAW << " from cTable " << cTable[g].d_autoyaw << " local yaw " << L_YAW << " from dGroup " << dGroup.d_autoyaw << endl;
					if ( (G_YAW&&L_YAW) || (!G_YAW&&L_YAW) ) {
						if (dangle[sec]==0)		degB = -(step*sec);
						else if (dangle[sec]==1)	degB = -(step*(sec+1));
						else				degB = -(step*(sec+1)-(step/2));
						Brush.RotOrigin(0,0,degB,OrigBAK);
					}
					if (dev) cout << "      Brush Z Rot (sec "<<Brush.SecID<<" * step "<<step<<") by X " << OffsetX << " Y " << OffsetY << " OrigBAK " << OrigBAK << " degB " << degB  << "  dGroup.Origin " << dGroup.Origin << endl;
				}
				for (int e = 0; e<dGroup.t_ents; e++) // Entities
				{
					entity &Entity = dGroup.Entities[e];
					int sec = Entity.SecID;
					float deg = (step*sec)+init_degZ[sec];
					vertex OrigBAK = IndiOrigin[sec]; //dGroup.Origin;
					OrigBAK.rotate(0,0,-deg);
					OffsetX = OrigBAK.x - IndiOrigin[sec].x; //dGroup.Origin.x;
					OffsetY = OrigBAK.y - IndiOrigin[sec].y; //dGroup.Origin.y;
					Entity.Origin.move(OffsetX,OffsetY,0);
					float degB;
					if ( (G_YAW&&L_YAW) || (!G_YAW&&L_YAW) ) { // || (G_YAW&&!L_YAW)  //if (cTable[g].d_autoyaw>0 && dGroup.d_autoyaw>0) {
						if (dangle[sec]==0)		degB = -(step*sec);
						else if (dangle[sec]==1)	degB = -(step*(sec+1));
						else				degB = -(step*(sec+1)-(step/2));
						Entity.Origin.rotateOrigin(0,0,degB,OrigBAK);
						Euler RotAngles(0,0,degB);
						Entity.RotateEntity(RotAngles,1);
					}
					if (dev) cout << "      Entity Z Rot (sec "<<Entity.SecID<<" * step "<<step<<") by X " << OffsetX << " Y " << OffsetY << " OrigBAK " << OrigBAK << " degB " << degB  << " New Origin " << Entity.Origin << " Euler " << Entity.Angles << endl;
				}
				
				/*dev = 1;
				if (dev) {
					cout << " height table of curve #"<<g<<" (adress "<< &bGroup[g].heightTable <<"): " << endl;
					for (int i = 0; i<bGroup[g].heightTable.size(); i++)
						cout << "  #" << i << " (adress "<<&bGroup[g].heightTable[i]<<")" << bGroup[g].heightTable[i]<<endl;
				}*/
				
				// Add Z Height
				for (int b = 0; b<dGroup.t_brushes; b++) // Brushes
				{
					brush &Brush = dGroup.Brushes[b];
					int sec = Brush.SecID;
					
					// height (ramp or steps)
					if ( cTable[g].height>0 && cTable[g].type!=2 )
					{
						float height = cTable[g].height * sec;
						if (dev) cout << " DGroup [" << d << "/"<< Set.t_groups<<"] Brush [" << b << "/" << dGroup.t_brushes <<"] sec " << sec << " start " << start << " sec-start " << sec-start << " end " << end << endl;
						if (dev) getch();
						if ( cTable[g].ramp==2 && sec>=start && sec<end )
						{
							float height_step = 0;
							if (dev) cout << " sec " << sec << " htable size " << bGroup[g].heightTable.size() <<endl;
							if (sec-start>0)	{
								//if (dev) cout << " AAA heightTable[sec-start] " << bGroup[g].heightTable[sec-start] << " heightTable[sec-start-1] " << bGroup[g].heightTable[sec-start-1] << endl;
								height_step = bGroup[g].heightTable[sec-start] - bGroup[g].heightTable[sec-start-1];
							}
							else {
								//if (dev) cout << " BBB height_step = heightTable[sec-start("<<sec-start<<")] " << bGroup[g].heightTable[sec-start] << endl;
								height_step = bGroup[g].heightTable[sec-start];
							}
							
							if (sec-start>0) {
								//if (dev) cout << " CCC height " << height << " = heightTable[sec-start-1("<<sec-start-1<<")] " << bGroup[g].heightTable[sec-start-1] << " + height_step * multi " << height_step * multi << endl;
								height = bGroup[g].heightTable[sec-start-1] + (height_step * multi[sec]);
							}
							else {
								//if (dev) cout << " DDD height " << height << " = height_step * multi " << height_step * multi << endl;
								height = height_step * multi[sec];
							}
							
							//if (sec>0&&dev) cout << " height: " << height << " heightTable["<<sec-start<<"] " << bGroup[g].heightTable[sec-start-1] << " + (height_step " << height_step << " * multi " << multi << ")" << endl;
						} else if (cTable[g].ramp==1 && sec>=start) {
							height = (cTable[g].height * sec) + (cTable[g].height * multi[sec]);
						}
						Brush.Move(0,0,height,1);
						if (dev) cout << "        Adding height to Detail Brush:" << height << endl;
					}
				}
				for (int e = 0; e<dGroup.t_ents; e++) // Entities
				{
					entity &Entity = dGroup.Entities[e];
					int sec = Entity.SecID;
					
					if ( cTable[g].height>0 && cTable[g].type!=2 )
					{
						float height = cTable[g].height * sec;
						if ( cTable[g].ramp==2 && sec>=start && sec<end )
						{
							float height_step = 0;
							if (sec-start>0)	height_step = bGroup[g].heightTable[sec-start] - bGroup[g].heightTable[sec-start-1];
							else		height_step = bGroup[g].heightTable[sec-start];
							if (sec-start>0)
								height = bGroup[g].heightTable[sec-start-1] + (height_step * multi[sec]);
							else 
								height = height_step * multi[sec];
						} else if (cTable[g].ramp==1 && sec>=start) {
							height = (cTable[g].height * sec) + (cTable[g].height * multi[sec]);
						}
						Entity.Origin.z += height;
					}
				}
			}
		}
	}
}

void file::createGroupBrush(int g)
{
	bool dev = 0;
	if (dev) cout << "Now creating Brush Groups..."  << endl;
	// create X Groups of Brushes, Faces and Vertices
	int &t_arcs = mGroup->t_arcs;
	if (bGroup==nullptr) bGroup = new group[t_arcs]; // NEW BRUSH GROUPS - Amount [X] (Total Arcs)
	if (dev) cout << "Created " << t_arcs << " new Brush Groups!" << endl;
	
	//for (int g = 0; g < t_arcs; g++) // Arc Object loop (Total Arcs)
	{
		group &Group = sGroup[g];
		if (dev) cout << "  Entering Arc #" << g << "..." << endl;
		bGroup[g].gID = g;
		bGroup[g].t_brushes = Group.t_brushes*cTable[g].res; // old(2019-05-27) (mGroup->t_brushes-mGroup->invalids)*cTable[g].res;
		bGroup[g].Brushes = new brush[bGroup[g].t_brushes]; 		// NEW BRUSHES - Amount: [X*Y] (total Map Brushes * Arc Resolution)
		if (dev) cout << "  Created " << Group.t_brushes*cTable[g].res << " new Brushes." << endl;
		bGroup[g].sections = cTable[g].res;
		bGroup[g].SecBaseFace = new face*[cTable[g].res];
		for (int i = 0; i<cTable[g].res; i++) bGroup[g].SecBaseFace[i]=nullptr;
		bGroup[g].segments = Group.t_brushes;
		int res = bGroup[g].sections;
		
		//bGroup[g].secs_range = floor(bGroup[g].sections*(cTable[g].arc/360.0));
		bGroup[g].range_start = floor(bGroup[g].sections*(cTable[g].range_start/100.0));
		bGroup[g].range_end   = floor(bGroup[g].sections*(cTable[g].range_end/100.0));
		//cout << " range start cTable " << cTable[g].range_start << " group secID " << bGroup[g].range_start << endl;
		//cout << " range end   cTable " << cTable[g].range_end << " group secID " << bGroup[g].range_end << endl;
		
		for (int o = 0, seg2=0, b = 0; o<Group.t_brushes; o++) 							// original map brush loop (x = amount of imported map brushes)
		{
			if (dev) cout << "    Entering map Brush #" << o << "..." << endl;
			brush &OBrush = Group.Brushes[o];
			int Ofcount = OBrush.t_faces;
			if (OBrush.valid)
			{
				for (int r = 0; r<res; r++) 									// generated brush loop (x = res)
				{
					if (dev) cout << "      Entering generated Brush #" << b << "..." << endl;
					brush &Brush = bGroup[g].Brushes[b];
					Brush.Faces  = new face[Ofcount];								// NEW FACES - Amount: [X] (amount of faces of original brush)
					Brush.t_faces = Ofcount;
					Brush.IsDivisible = OBrush.IsDivisible;
					Brush.entID = OBrush.entID;
					if (dev) cout << "      Created " << Ofcount << "new faces for this Brush." << endl;
					Brush.SecID = r;
					Brush.SegID = o;
					Brush.SegID2 = seg2;
					Brush.name = "GEN_sec" + to_string(r) + "_seg" + to_string(o) + "_fc" + to_string(Ofcount);
					Brush.gID = g;
					
					if (dev) cout << "  Brush #"<<b<< ", Seg #"<<Brush.SegID<< ", Sec #" << Brush.SecID << ", Tfaces: " << Brush.t_faces << ", Ofcount: " << Ofcount<< endl;
					
					for (int f = 0; f<Brush.t_faces; f++) 					// face loop (x = faces per brush)
					{
						if (dev) cout << "        Entering Face #" << f << "..." << endl;
						face &Face = Brush.Faces[f];
						
						if (f==0||f==1)	// base and head faces
						{
							Face.vcount = Ofcount-2;
							Face.Vertices = new vertex[Face.vcount];
							if (f==0) Face.fID=0;
							else Face.fID=1;
							if (dev) cout << "        Created " << Face.vcount << " new vertices for this face. Face is BASE/HEAD." << endl;
						}
						else // Body faces
						{
							Face.vcount = 4;
							Face.Vertices = new vertex[4];
							Face.fID=2;
							if (dev) cout << "        Created " << Face.vcount << " new vertices for this face. Face is BODY." << endl;
						}
					}
					b++;
				}
				seg2++;
			}
		}
	}
}

void file::LoadPaths(int g)
{
	bool dev = 0;
	int &t_arcs = mGroup->t_arcs;
	if (PathList.size()==0)
		PathList.resize(t_arcs);
	
	if (dev) cout << "Loading Path #"<<g+1<<"..." << endl; if (dev) getch();
	
	if (cTable[g].type==2)
	{
		if (dev) cout << "  Scanning map dir for valid path files ("<<cTable[g].path<<")..." << endl;
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
			filename = p_path + name + "_path" + to_string(g+1) +".map";
		
		if (dev) cout << "  Path-File " << filename << endl;
		
		str_path = LoadTextFile(filename);
		
		if (str_path!="ERR")
		{
			if (dev) cout << "  Pathfile " << filename << " exists and is now being interpreted..." << endl;
			
			interpretPathFile(str_path, PathList[g]);
			PathList[g].preverse = cTable[g].preverse;
			PathList[g].cornerFix = cTable[g].cornerfix;
			PathList[g].type = cTable[g].type;
			analyzePath(PathList[g]);
			
			if (PathList[g].valid) {
				cout << "|    [INFO] Path #"<<g+1<<" ("<<filename<<") successfully loaded!"<< endl;
				cTable[g].res = PathList[g].t_corners-PathList[g].t_paths+PathList[g].Gaps;
				if (dev) cout << " Path res now " << cTable[g].res << " (total corners "<<PathList[g].t_corners<< " - total paths "<<PathList[g].t_paths<<" + Gaps "<<PathList[g].Gaps<<")" << endl;if (dev)  getch();
			} else {
				cout << "|    [ERROR] Pathfile " << filename << " does NOT seem to contain path_corner entities!"<< endl;
				cTable[g].type = 0;
				cTable[g].res = 12;
				if (cTable[g].ramp>0&&cTable[g].height>0) cTable[g].tri = 1;
			}
		}
		else
		{
			PathList[g].valid = 0;
			cTable[g].type = 0;
			cTable[g].res = 12;
			if (cTable[g].ramp>0&&cTable[g].height>0) cTable[g].tri = 1;
			cout << "|    [ERROR] Pathfile " << filename << " does NOT exist!" << endl;
		}
	}
}

void interpretPathFile(string pFile, path_set &PathList)
{
	bool dev = 0;
	//circleset PathList;
	string p_corner = "\"classname\" \"path_corner\"";
	string str_origin = "\"origin\""; // +10
	string str_name = "\"targetname\""; // +14
	string str_target = "\"target\""; // +10
	string number = "-0123456789";
	string spacer = " ";
	string newline = "\n";
	string qmark = "\"";
	
	if (pFile.length()>0&&pFile.find(p_corner)!=-1)
	{
		// count path_corner
		if(dev) cout << "count path_corner..." << endl;
		int pcount=0;
		int find = 0, last = 0;
		while (find!=-1)
		{
			find = pFile.find(p_corner, last);
			if (find!=-1) {pcount++; last = find+1;}
			else break;
			//cout << "    find " << find << endl;
		}
		path_corner pCorner[pcount];
		
		if(dev) cout << "total path_corner: " << pcount << endl;
		PathList.t_corners = pcount;
		if(dev) getch();
		
		// extract path_corner properties
		if(dev) cout << "extract path_corner properties..." << endl;
		for (int i = 0, s=0,e=0,l=0; i<pcount; i++)
		{
			string str_x,str_y,str_z;
			int x_start,x_end,y_start,y_end,z_start,z_end,name_start,name_end,tar_start,tar_end;
			s = pFile.find(p_corner,l);
			e = pFile.find("}",s);
			
			x_start = pFile.find(str_origin, s)+10;
			x_end = pFile.find(spacer,x_start);
			str_x = pFile.substr(x_start,x_end-x_start);
			
			y_start = x_end+1;
			y_end = pFile.find(spacer,y_start);
			str_y = pFile.substr(y_start,y_end-y_start);
			
			z_start = y_end+1;
			z_end = pFile.find(qmark,z_start);
			str_z = pFile.substr(z_start,z_end-z_start);
			
			pCorner[i].pos.x = stof(str_x);
			pCorner[i].pos.y = stof(str_y);
			pCorner[i].pos.z = stof(str_z);
			
			name_start = pFile.find(str_name, s)+14;
			name_end = pFile.find(qmark, name_start);
			pCorner[i].name = pFile.substr(name_start,name_end-name_start);
			
			tar_start = pFile.find(str_target, l)+14;
			tar_end = pFile.find(qmark, tar_start);
			if (tar_start<e)
			pCorner[i].target = pFile.substr(tar_start,tar_end-tar_start);
			
			if(dev) cout << "   path "<<i<<" name " << pCorner[i].name << " coords" << pCorner[i].pos << " target " << pCorner[i].target << endl;
			
			l = e+1;
		}
		if(dev) getch();
		
		// assign path ID to paths, if there is more than one path (first pathname without counter (01) identifies a new path)
		if(dev) cout << "assign path ID to paths..." << endl;
		string c_phrase = pCorner[0].name;
		int c_phrase_len = pCorner[0].name.length();
		int c_pID = 0;
		for (int i = 0; i<pcount; i++)
		{
			int c_name_len = pCorner[i].name.length();
			int c_phrase_pos = pCorner[i].name.find(c_phrase); // current phrase found pos
			
			// looking for "path1" (could find it in path11, path1101, path11101, etc.)
			if (c_phrase_pos==-1 || (c_phrase_pos==0&&c_name_len==c_phrase_len+1) )
			// if current search phrase (path1) wasnt found (path2) OR it was found, but current name is 1 digit longer than search phrase (e.g. path11), this is probably a new path
			{
				c_phrase = pCorner[i].name;
				c_phrase_len = pCorner[i].name.length();
				c_pID++;
				pCorner[i].pID = c_pID;
			}
			// result belongs to e.g. path1 if result is at least 2 digits longer than path1, e.g. path101 (1. point), path1102 (102. points), etc.
			else if (c_phrase_pos==0 && c_name_len>=c_phrase_len+2 ) // if current phrase (e.g. "path1") is found at Pos0 and its length is at least 2 digits longer than current phrase (e.g. "path101")
			{
				pCorner[i].pID = c_pID;
			}
			if(dev) cout << "    path " << i << " name " << pCorner[i].name << " ID " << pCorner[i].pID << endl;
		}
		if(dev) getch();
		
		int tpaths = c_pID+1;
		int vcount[tpaths]; for (int i = 0; i<tpaths; i++) vcount[i] = 0;
		// count vertices of each path ID
		if(dev) cout << "count vertices of each path ID..." << endl;
		for (int i = 0; i<pcount; i++)
		{
			vcount[pCorner[i].pID]++;
			if(dev) cout << "    current vertex counter " << vcount[pCorner[i].pID] << endl;
		}
		
		if(dev)
		for (int i = 0; i<c_pID+1; i++) {
			if(dev) cout << "    path " << i << " vertex amount " << vcount[i] << endl; }
		if(dev) getch();
		
		// finally fill the path corners into the files circleset
		if(dev) cout << "finally fill the path corners into the files circleset..." << endl;
		PathList.t_paths = c_pID+1;
		PathList.Paths = new path[c_pID+1];
		for (int s = 0, p=0; s<PathList.t_paths; s++) // circle loop
		{
			path &Path = PathList.Paths[s];
			Path.t_corners = vcount[s];
			Path.Corners = new path_corner[vcount[s]];
			for (int v = 0; v<Path.t_corners; v++)
			{
				Path.Corners[v] = pCorner[p];
				p++;
			}
		}
		
		// get height of all corners and create a height table from it
		for (int s = 0, h=0; s<PathList.t_paths; s++) // path loop
		{
			path &Path = PathList.Paths[s];
			for (int v = 0; v<Path.t_corners; v++) // corner loop
			{
				path_corner &Corner = Path.Corners[v];
				if (v==0) 	PathList.heightTable.push_back(Corner.pos.z);
				else		PathList.heightTable.push_back(Corner.pos.z-Path.Corners[v-1].pos.z);
				if(dev) cout << " Path #" << s << " Corner#" << v << " relative height: " << PathList.heightTable[h] << endl;
				h++;
			}
		}
		PathList.valid = 1;
		
		if(dev) getch();
		//return true;
	}
	else PathList.valid = 0;
	//else return false;
}


void file::analyzePath(path_set &List)
{
	//cout << " Analyzing Path Set with " << List.t_paths<< " paths in it... " << endl;
	for (int p = 0; p<List.t_paths; p++) // path loop
	{
		path &Path = List.Paths[p];
		
		// determine starting direction of this path (not needed ATM)
		//float StartAngle = GetVecAlign( GetVector(Path.Corners[0].pos, Path.Corners[1].pos) ,0);
		//if ( StartAngle>90&&StartAngle<270) {Path.direct = 0;} //cout << " Path direction is backwards!" << endl;}
		//else cout << " Path direction is forwards! " << endl;
		
		if (List.preverse) {Path.reverse();}// cout << " Path IS being reversed! " << endl; } else cout << " Path NOT reversed! " << endl;
		
		// determine angle of all sections
		for (int c = 0; c<Path.t_corners; c++) // corner loop
		{
			if (c<Path.t_corners-1)
			{
				path_corner &Corner1 = Path.Corners[c];
				path_corner &Corner2 = Path.Corners[c+1];
				
				// Angle
				gvector Edge = GetVector(Corner1.pos, Corner2.pos);
				Corner1.Yaw = GetVecAlign(Edge, 0);
				//cout << " Corner #"<<c<< " Name " << Corner1.name << " Yaw " << Corner1.Yaw << endl;
				
				// relative Height
				Corner1.step = Corner2.pos.z-Corner1.pos.z;
				//cout << " Path " << p << " Corner " << c << " of "<<Path.t_corners<<" step " << Corner1.step << " (Corner2.z "<<Corner2.pos.z<<" - Corner1.z "<<Corner1.pos.z<<")" << endl;
				
				if (c<Path.t_corners-2)
				{
					path_corner &Corner3 = Path.Corners[c+2];
					gvector Edge2 = GetVector(Corner2.pos, Corner3.pos);
					//cout << " Edge Angle " << GetVecAng(Edge, Edge2) << endl;
					
					// Determine whether next section is heading left or right
					gvector Vec_Yaw = Edge2;
					Vec_Yaw.rotate(0,0,-(Corner1.Yaw));
					float Check_Yaw = GetVecAlign(Vec_Yaw,0);
					if (Check_Yaw>180) Corner1.NextIsCW = 1;
					//cout << "    Check_Yaw " <<Check_Yaw; if (Corner1.NextIsCW) cout << " Clockwise! " << endl; else cout << " Counter Clockwise! " << endl;
				}
			}
		}
		
		// determine section-align (0,90,180,270)
		if (List.type==2)
		for (int c = 0; c<Path.t_corners; c++) // corner loop
		{
			// 45-315	0	UP
			// 225-315	270	RIGHT
			// 135-225	180 DOWN
			// 45-135	90	LEFT
			path_corner &Corner = Path.Corners[c];
			int CornerYaw = round(Corner.Yaw);
			if 		(CornerYaw>=315||CornerYaw<45)  /*UP*/    {Corner.Align = 0;}
			else if (CornerYaw>=225&&CornerYaw<315) /*RIGHT*/ {Corner.rot1 = 270; Corner.rot2 = 270; Corner.Align = 1;}
			else if (CornerYaw>=135&&CornerYaw<225) /*DOWN*/  {Corner.rot1 = 180; Corner.rot2 = 180; Corner.Align = 2;}
			else if (CornerYaw>=45 &&CornerYaw<135) /*LEFT*/  {Corner.rot1 = 90;  Corner.rot2 = 90;  Corner.Align = 3;}
		}
		
		// get worldalign of each section - changes if aligns of 2 sections differ by 90 or 180 degree
		//cout << "Fix cutting edge rotations and mark gaps..." << endl;
		if (List.type==2)
		for (int c = 0; c<Path.t_corners-2; c++) // corner loop
		{
			path_corner &Corner = Path.Corners[c];
			path_corner &CornerN = Path.Corners[c+1];
			path_corner &CornerN2 = Path.Corners[c+2];
			int CornerYaw = round(Corner.Yaw);
			int CornerNYaw = round(CornerN.Yaw);
			
			bool AngleIs45 = 0;  if (CornerYaw==45||CornerYaw==135||CornerYaw==225||CornerYaw==315) AngleIs45=1;
			bool AngleNIs45 = 0; if (CornerNYaw==45||CornerNYaw==135||CornerNYaw==225||CornerNYaw==315) AngleNIs45=1;
			//cout << " Section #"<<c+1<<" has "; if (!AngleNIs45) cout <<"NO "; cout<<" 45 degree Angle! ("<< CornerNYaw <<")" << endl;
			
			float YawDiff = CornerYaw-CornerNYaw; if (YawDiff<0) YawDiff*=-1;
			bool YawDiffIsClean = 0; if (YawDiff==90||YawDiff==270||YawDiff==45) YawDiffIsClean = 1;
			int Diff = CornerN.Align-Corner.Align; Diff = sqrt(pow(Diff,2)); // difference in world align
			
			// Hor Length between these two Corners (must be bigger than mesh size for clean 45 degree Corner)
			vertex FlatPos 		= Corner.pos; FlatPos.z = 0;
			vertex FlatPosN 	= CornerN.pos; FlatPosN.z = 0;
			vertex FlatPosN2 	= CornerN2.pos; FlatPosN2.z = 0;
			float CornerDist  	= GetVecLen(GetVector( FlatPos, FlatPosN ));
			float CornerNDist 	= GetVecLen(GetVector( FlatPosN, FlatPosN2 ));
			float size = mGroup->SizeY;
			bool CornerDistIsValid  = 0; if (pow(CornerDist,2)/2>=pow(size,2)) CornerDistIsValid = 1;
			bool CornerNDistIsValid = 0; if (pow(CornerNDist,2)/2>=pow(size,2)) CornerNDistIsValid = 1;
			bool Plain = 0; if (Corner.pos.z==CornerN.pos.z) Plain = 1;
			bool PlainN = 0; if (CornerN.pos.z==CornerN2.pos.z) PlainN = 1;
			//cout << "    Size: " << size << " CornerDist " << CornerDist << "( " << FlatPos << ", " << FlatPosN << ")" << " CornerDistIsValid " << CornerDistIsValid << " ( "<<pow(CornerDist,2)/2<< " >= " << pow(size,2)<<")" << endl;
			//cout << "  Corner #" << c << " Align " << Corner.Align <<" Angle " << static_cast<int>(Corner.Yaw)<< " Align Next "<<CornerN.Align<<" Angle Next "<<CornerN.Yaw<<" Difference "<< Diff <<" rot1 " << Corner.rot1 << " rot2 " << Corner.rot2;
			//cout << "  Corner #" << c << " Yaw "<< static_cast<int>(Corner.Yaw);
			//cout << " #"<<c<<" YawDiff " << static_cast<int>(YawDiff) << " Yaw " << static_cast<int>(Corner.Yaw) << " AngleIs45 " << AngleIs45 << " YawN " << static_cast<int>(CornerN.Yaw) << " AngleNIs45 " << AngleNIs45 << endl;
			//cout << "    Section #"<<c<< " Name " << Corner.name << " Diff " << Diff << " ("<<CornerN.Align<<"-"<<Corner.Align<<") " << " has "; if (!AngleIs45) cout <<"NO "; cout<<" 45 degree Angle! ("<< CornerYaw <<") Next is "; if (Corner.NextIsCW) cout<< " CLOCKWISE! " << endl; else cout <<" NOT CLOCKWISE!" << endl;
			if (Diff==1||Diff==3)
			{
				/*
				if (AngleIs45&&AngleNIs45&&YawDiffIsClean)
				{
					if (!Corner.NextIsCW)
					CornerN.rot1 -= 90;
					else
					Corner.rot2 -= 90;
				}
				else if (!AngleIs45&&AngleNIs45&&!Corner.NextIsCW)
				{
					CornerN.rot1 -= 90;
				}
				//else if (!AngleIs45&&AngleNIs45&&Corner.NextIsCW)
				//{
				//	CornerN.rot1 += 90;
				//}
				else if (AngleIs45&&!AngleNIs45&&Corner.NextIsCW)
				{
					Corner.rot2 -= 90;
				}
				else if (!AngleIs45&&!AngleNIs45)
				{
					if (List.cornerFix || (!List.cornerFix&&Corner.NextIsCW))
					{
						// if world align of 2 sections differs by 90 degree
						Corner.NextIsGap90 = 1;
						List.Gaps++;
						//cout << "  Corner #"<<c<<" Diff 90 deg, ADDING GAP! Gaps now: " << List.Gaps << " cornerfix" << List.cornerFix << endl;
					}
				}
				else
				{
					if (List.cornerFix || (!List.cornerFix&&Corner.NextIsCW))
					{
						Corner.NextIsGap90 = 1;
						List.Gaps++;
						//cout << "  Corner #"<<c<<" Diff 90 deg, ADDING GAP! Gaps now: " << List.Gaps << " cornerfix" << List.cornerFix << endl;
					}
				}*/
				
				if (AngleIs45&&AngleNIs45&&CornerDistIsValid&&Plain&&PlainN)
				{
					if (Corner.NextIsCW)
					{
						//cout << "    C45 N45 CW" << endl;
						Corner.rot2  -= 90;
					}
					else 
					{
						//cout << "    C45 N45 CCW" << endl;
						CornerN.rot1 -= 90;
					}
				}
				else if (!AngleIs45&&AngleNIs45&&!Corner.NextIsCW&&CornerNDistIsValid&&PlainN)
				{
					/*if (Corner.NextIsCW)
					{
						cout << "    N45 CW" << endl;
						CornerN.rot1 += 90;
					}
					else*/
					{
						//cout << "    N45 CCW CornerNDist " << CornerNDist << " size " << size <<  endl;
						CornerN.rot1 -= 90;
					}
				}
				else if (AngleIs45&&!AngleNIs45&&CornerDistIsValid&&Plain&&Corner.NextIsCW)
				{
					//if (Corner.NextIsCW)
					{
						//cout << "    C45 CW" << endl;
						Corner.rot2 -= 90;
					}
					/*else 
					{
						cout << "    C45 CCW" << endl;
						Corner.rot1 += 90;
					}*/
				}
				else if (List.cornerFix || (!List.cornerFix&&Corner.NextIsCW))
				{
					Corner.NextIsGap90 = 1;
					List.Gaps++;
					//cout << "    Gap 90" << endl;
				}
				//else cout << "    Rule 90 - NO RULE applied!!!" << endl;
			}
			else if (Diff==2)
			{
				if (AngleIs45&&AngleNIs45&&Plain&&PlainN&&CornerDistIsValid)
				{
					if (Corner.NextIsCW)
					{
						//cout << "    Rule 180 - C45 N45 CW" << endl;
						Corner.rot2  -= 90;
					}
					else 
					{
						//cout << "    Rule 180 - C45 N45 CCW" << endl;
						Corner.rot2  += 90;
						CornerN.rot1 -= 90;
					}
				}
				else if (AngleIs45&&!AngleNIs45&&Plain&&CornerDistIsValid&&Corner.NextIsCW)
				{
					//if (Corner.NextIsCW)
					{
						//cout << "    Rule 180 - C45 CW" << endl;
						Corner.NextIsGap90 = 1;
						List.Gaps++;
						Corner.rot2  -= 90;
					}
					/*else 
					{
						cout << "    Rule 180 - C45 CCW" << endl;
						Corner.rot2  += 90;
						CornerN.rot1 -= 90;
					}*/
				}
				// if world align of 2 sections differs by 180 degree
				else if (List.cornerFix || (!List.cornerFix&&Corner.NextIsCW))
				{
					Corner.NextIsGap180 = 1;
					List.Gaps+=2;
					//cout << "  Corner #"<<c<<" Diff 180 deg, ADDING GAP! Gaps now: " << List.Gaps << " cornerfix" << List.cornerFix << endl;
				}
				//else cout << "    Rule 180 - NO Rule applied!" << endl;
			}
			//else cout << endl;
		}
		//cout << endl;
	}
}

void path::reverse() {
	for (int i = 0; i<t_corners/2; i++)
		swap(Corners[i],Corners[t_corners-1-i]);
}

void file::roundCoords(int g)
{
	if (cTable[g].round==0&&cTable[g].transit_round>0)
	bGroup[g].GetTransitVertices();
	
	if (cTable[g].round>0||cTable[g].transit_round>0)
	bGroup[g].RoundBrushVertices(0);
}

// Create a Ramp & adjust Face Texture Vectors accordingly
void file::RampIt(int g)
{
	bool dev = 0;
	
	if ((cTable[g].ramp>0&&cTable[g].height>0)||cTable[g].type==2)
	for (int b=0; b<bGroup[g].t_brushes; b++)
	{
		brush &Brush = bGroup[g].Brushes[b];
		if (dev) cout << " creating Ramp of arc " << g << " brush " << b << " type " << cTable[g].type << " height " << cTable[g].height << " ramp " << cTable[g].ramp << endl;
		
		if (Brush.valid&&Brush.draw)
		if (Brush.Tri==nullptr)
		{
			if (dev) cout << "  Triangulation inactive!" << endl;
			for (int f = 0; f<Brush.t_faces; f++)
			{
				face &Face = Brush.Faces[f];
				
				if (Face.draw)
				Face.CreateRamp(g, b, f, Brush.SecID, Brush.IsWedge2, Brush.step);
			}
		}
		else
		{
			if (dev) cout << "  Triangulation active! Tri Count " << Brush.t_tri << endl;
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

void file::Triangulate(int g)
{
	bGroup[g].Triangulate();
}


void file::createFramework(int g)
{
	bool dev = 0;
	if (dev) cout << "Creating Construction Framework..."<<endl;
	//for (int a = 0; a<mGroup->t_arcs; a++) // arc/circleset loop
	{
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
				if(dev) cout << "Circle Set #" << g << ", total arcs " << mGroup->t_arcs << ", rad: " << cTable[g].rad << ", res: " << cTable[g].res << ", type: " << cTable[g].type << endl;
				
				// Every set of circle gets x circles (x=brushfaces-2)
				if(dev) cout << "   Creating " << tcircs << " circles for Brush " << b << endl;
				Brush.cset->c = new circle[tcircs];
				circleset &Set = *Brush.cset;
				Set.tcircs = tcircs;
				
				if(dev) cout << "   Creating Circle Vertices..." << endl;
				for (int c = 0; c<tcircs; c++) // circle loop
				{
					if (dev) cout << "     basevertID " << Brush.vlist[c] << endl;
					if (dev) cout << "     smallest Angle " << Brush.Faces[Brush.vlist[c]].vAngle_s << endl;
					
					circle &Circle = Set.c[c];
					Circle.tverts = res;
					vertex basevert = Brush.Faces[Brush.vlist[c]].Vertices[ Brush.Faces[Brush.vlist[c]].vAngle_s ];
					Circle.SrcFace = Brush.vlist[c];
					float yb = Group.Dimensions.yb;
					float ys = Group.Dimensions.ys;
					
					// Every Circle gets x vertices (x=resolution of current arc)
					float indrad = 0.0; // individual rad, based on basevert y pos and new rad
					
					if (rad == yb)
						indrad = basevert.y; // individual rad, based on basevert y pos and new rad
					else
						indrad = basevert.y + (rad - yb); // individual rad, based on basevert y pos and new rad
					
					float height = basevert.z;
					if(dev) cout << "circle-loop #" << c  << ", res: " << res<< ", basevert: " << Brush.Faces[ Brush.vlist[c] ].Vertices[ Brush.Faces[Brush.vlist[c]].vAngle_s ] << ", yb: " << yb <<  endl;
					
					// generate Circles
					if (type==0)
						Circle.build_circlePi(res,indrad,height);
					else if (type==1)
					{
						//cout << "Attempting to build Grid Circle... (res,rad,height)" << res << "," << indrad << "," << height << endl;
						Circle.build_circleGrid(res,indrad,height);
					}
					else if (type==2)
					{
						//cout << " building grid path of arc #" << a << " path " << c << endl;
						path_set &PSet = PathList[g];
						Circle.build_pathGrid(g, indrad, height, PSet); // 08.05.2019 changed basevert.y to indrad
					}
					if(dev) cout << " Circle " << Circle << endl;
			}
			}
		}
	}
	
	/*
	for (int b = 0; b<mGroup->t_brushes; b++) // brush loop
	{
		brush &Brush = mGroup->Brushes[b];
		if (Brush.valid)
		{
			// every Brush gets x sets of circles (x=total arcs)
			Brush.cset = new circleset[mGroup->t_arcs];
			
			for (int a = 0; a<mGroup->t_arcs; a++) // arc/circleset loop
			{
				int tcircs = Brush.t_faces-2;
				int res = cTable[a].res;
				float rad = cTable[a].rad;
				int type = cTable[a].type;
				//cout << "Circle Set #" << a << ", total arcs " << mGroup->t_arcs << ", rad: " << cTable[a].rad << ", res: " << cTable[a].res << ", type: " << cTable[a].type << endl;
				
				// Every set of circle gets x circles (x=brushfaces-2)
				Brush.cset[a].c = new circle[tcircs];
				circleset &Set = Brush.cset[a];
				Set.tcircs = tcircs;
				
				for (int c = 0; c<tcircs; c++) { // circle loop
					
					circle &Circle = Set.c[c];
					Circle.tverts = res;
					vertex basevert = Brush.Faces[Brush.vlist[c]].Vertices[ Brush.Faces[Brush.vlist[c]].vAngle_s ];
					Circle.SrcFace = Brush.vlist[c];
					float biggesty = mGroup->biggestY;
					float smallesty = mGroup->smallestY;
					
					// Every Circle gets x vertices (x=resolution of current arc)
					float indrad = 0.0; // individual rad, based on basevert y pos and new rad
					
					if (rad == biggesty)
						indrad = basevert.y; // individual rad, based on basevert y pos and new rad
					else
						indrad = basevert.y + (rad - biggesty); // individual rad, based on basevert y pos and new rad
					
					float height = basevert.z;
					//if (cTable[a].type==2) height+= 
					//cout << "circle-loop #" << c  << ", res: " << res<< ", basevert: " << Brush.Faces[ Brush.vlist[c] ].Vertices[ Brush.Faces[Brush.vlist[c]].vAngle_s ] << ", biggesty: " << biggesty <<  endl;
					
					// generate Circles
					if (type==0)
						Circle.build_circlePi(res,indrad,height);
					else if (type==1)
					{
						//cout << "Attempting to build Grid Circle... (res,rad,height)" << res << "," << indrad << "," << height << endl;
						Circle.build_circleGrid(res,indrad,height);
					}
					else if (type==2)
					{
						//cout << " building grid path of arc #" << a << " path " << c << endl;
						path_set &PSet = PathList[a];
						Circle.build_pathGrid(indrad, height, PSet); // 08.05.2019 changed basevert.y to indrad
					}
					//if (Circle.Vertices[0].x == 0 && Circle.Vertices[0].y == 0) Set.IsTriang = 1;
					
					// Check final Circle for Decimal Places
					/*
					int &Dplaces = cTable[a].DeciPlaces;
					int NewDeci = GetDeciPlaces(height);
					if (NewDeci > Dplaces) Dplaces = NewDeci;
					for (int j = 0; j<res; j++)
					{
						NewDeci = GetDeciPlaces(Circle.Vertices[j].x);
						
						if (NewDeci > Dplaces)
							Dplaces = NewDeci;
					}*/
					
					//cout << "Deciplaces now: " << Dplaces << endl;
					//cout << "[New Circle] Brush #" << b << ", Arc #" << a << ", Circle #" << c << ", res: " << res << ", rad: " << rad << ", indrad: " << indrad << ", height: " << height << endl;
					//for (int i = 0; i < res; i++) cout << "#" << i << Circle.Vertices[i] << endl;
					//cout << endl;
					//cout << "Circle Done!" << endl;
				/*
				}
			}
		}
	}
	*/
}

// calculate the resulting settings for each arc
void file::createTableC()
{
	bool dev = 0;
	if (dev) cout << "Creating " << mGroup->t_arcs << " Construction Tables..." << endl;
	
	// Create construction Tables
	int &t_arcs = mGroup->t_arcs;
	if (cTable!=nullptr) {delete[] cTable; if (dev) cout << " Deleting existing cTable, containing " << t_arcs << " objects and creating a new one..." << endl; }
	cTable = new ctable[t_arcs];
	
	// Fill Construction Tables with previously loaded Settings
	if (dev) cout << "   Filling " << t_arcs << " Construction Tables with previously loaded Settings..." << endl;
	for (int a = 0; a<t_arcs; a++) {
		cTable[a] = sTable[a];
		if (dev) cout << "     Arc " << a << " rad: " << sTable[a].rad << "\t offset: " << sTable[a].offset << "\t type: " << sTable[a].type << "\t res: " << sTable[a].res << "\t shift: " << sTable[a].shift << "\t height: " << sTable[a].height << endl;
	}
	
	// export settings
	if (cTable[0].target!="UNSET") target = cTable[0].target; else target = dTable[0].target;
	if (cTable[0].append!=-1) append = cTable[0].append; else append = dTable[0].append;
	
	if (dev) cout << "   Evaluate settings..." << endl;
	// Evaluate settings
	for (int a = 0; a<t_arcs; a++) // arc loop
	{
		// path
		if 		(a>0&&cTable[a].path=="UNSET") 		cTable[a].path = cTable[a-1].path;
		
		// type
		if 		(a==0&&cTable[0].type<0) 	cTable[0].type = dTable[0].type;	// if first type is 0, set to PI
		else if (a>0&&cTable[a].type<0) 	cTable[a].type = cTable[a-1].type; // if x-th type is 0, type is same as previous type
		
		// obj
		if 		(a==0&&cTable[0].obj<0) 	cTable[0].obj = dTable[0].obj;	// if first type is 0, set to PI
		else if (a>0&&cTable[a].obj<0) 		cTable[a].obj = cTable[a-1].obj; // if x-th type is 0, type is same as previous type
		
		// rad & offset
		if 		(cTable[a].rad==0) 			cTable[a].rad = sGroup[a].Dimensions.yb+cTable[a].offset;	// if first rad is 0, set rad to biggest map-y coord + offset
		else 								cTable[a].rad += cTable[a].offset;
		float size = sGroup[a].Dimensions.yb - sGroup[a].Dimensions.ys;
		
		// if (cTable[a].type<2) // prevent arc radius smaller than 0 or object size if arc type is PI or GRID Circle (paths can have negative source-object coordinates)
		if ( cTable[a].rad-size <= 0) cTable[a].rad = size; //+cTable[a].offset
		if ( cTable[a].type==2 )
		{
			cTable[a].rad = size+cTable[a].offset;
			//cout << " FIXING Radius of Grid Path Type source map... Rad currently " << cTable[a].rad << " map Y size " << size << " offset " << cTable[a].offset << endl;
			if ( cTable[a].rad < size) { cTable[a].rad = size; } //cout << "   Rad bigger than size+offset ("<<size+cTable[a].offset<<") Rad now " << cTable[a].rad << endl; }
		}
		sGroup[a].SizeY = size;
		
		// original radius offset
		cTable[a].offset_NO = (cTable[a].rad-(sGroup[a].SizeY/2)) - (mGroup->Dimensions.yb-(mGroup->SizeY/2));   //cTable[a].rad - (sGroup[a].SizeY/2); //mGroup->Dimensions.yb; // sGroup[a].Origin.y
		
		//res
		if (cTable[a].type==0) { // PI Circle Type
			if 		(a==0&&cTable[0].res<=0) cTable[0].res = dTable[0].res;	// if first res is 0, set to minimum of 8
			else if (a>0&&cTable[a].res<=0) {
				float m = cTable[a].rad/cTable[a-1].rad; //cout << "multiplier: " << m << endl;
				int temp = (m*cTable[a-1].res)/4.0; //cout << "temp (last res/4): " << m << endl;
				int temp2 = temp*4; //cout << "temp2 (temp*4): " << m << endl;
				cTable[a].res = temp2; // if x-th res is 0, set it to value based on the current and last radius+offset
			} else if (a==0&&cTable[0].res>0) {
				int temp = cTable[a].res/4.0;
				cTable[a].res = temp*4;
			}
			if (cTable[a].res <= 4) cTable[a].res = 4; // always set res to a minimum of 4
		}
		else if (cTable[a].type==1) { // grid Circle Type
		
			if (a==0&&cTable[a].res<=0)
				cTable[a].res = 12;							// if first res is 0, set to minimum of 12

			//else if (a>0&&cTable[a].res==-1) cTable[a].res = cTable[a-1].res;
				
			else if (a>0&&cTable[a].res<=0)					// if x-th res is 0, get res based on previous res
			{
				float m = cTable[a].rad/cTable[a-1].rad;
				int temp = ((m/2)*cTable[a-1].res);
				CheckFixRes(temp, 2);
				cTable[a].res = temp;						// if x-th res is 0, set it to value based on the current and last radius+offset
			}
			else if (cTable[a].res>0&&cTable[a].res<7) 		// res 1..6 (12..384)
			{
				if (cTable[a].res>1) {						// res 2..6 (24..384)
					int m = 12;
					for (int i = 0; i<cTable[a].res-1; i++) {
						m *= 2;
					}
					cTable[a].res = m;
				}
				else cTable[a].res = 12;						// res 1 (12)
				
			}
			else if (cTable[a].res>=7&&cTable[a].res<=384)		// res 12..384
				CheckFixRes(cTable[a].res, 2);
		}
		
		// bound
		if 		(a==0&&cTable[0].bound<0) 	cTable[0].bound = dTable[0].bound;
		else if (a>0&&cTable[a].bound<0) 	cTable[a].bound = cTable[a-1].bound;
		
		// skipnull
		if 		(a==0&&cTable[0].skipnull<0) 	cTable[0].skipnull = dTable[0].skipnull;
		else if (a>0&&cTable[a].skipnull<0) 	cTable[a].skipnull = cTable[a-1].skipnull;
		
		//shift
		if 		(a==0&&cTable[0].shift<0) 	cTable[0].shift = dTable[0].shift;
		else if (a>0&&cTable[a].shift<0) 	cTable[a].shift = cTable[a-1].shift;
		
		//ramp
		if 		(a==0&&cTable[0].ramp<0) 	cTable[0].ramp = dTable[0].ramp;
		else if (a>0&&cTable[a].ramp<0) 	cTable[a].ramp = cTable[a-1].ramp;
		
		//height
		if 		(a==0&&cTable[0].height<0) 	cTable[0].height = dTable[0].height;
		else if (a>0&&cTable[a].height<0) 	cTable[a].height = cTable[a-1].height;
		
		// tri
		if 		(a==0&&cTable[0].tri<0) 	cTable[0].tri = dTable[0].tri;
		else if (a>0&&cTable[a].tri<0) 		cTable[a].tri = cTable[a-1].tri;
		// in case ramp is being generated, always use triangulation
		if (cTable[a].ramp>0&&cTable[a].height>0&&cTable[a].type!=2) cTable[a].tri = 1;
		
		// round
		if 		(a==0&&cTable[0].round<0) 	cTable[0].round = dTable[0].round;
		else if (a>0&&cTable[a].round<0) 	cTable[a].round = cTable[a-1].round;
		
		// cornerfix (if type == 2 - Grid Path - fixes overlapping inner corners)
		if 		(a==0&&cTable[0].cornerfix<0) cTable[0].cornerfix = dTable[0].cornerfix;
		else if (a>0&&cTable[a].cornerfix<0)  cTable[a].cornerfix = cTable[a-1].cornerfix;
		
		// reverse path direction
		if 		(a==0&&cTable[0].preverse<0) cTable[0].preverse = dTable[0].preverse;
		else if (a>0&&cTable[a].preverse<0)  cTable[a].preverse = cTable[a-1].preverse;
		
		// ramptex - set horizontal texturing mode for ramps
		if 		(a==0&&cTable[0].ramptex<0) cTable[0].ramptex = dTable[0].ramptex;
		else if (a>0&&cTable[a].ramptex<0)  cTable[a].ramptex = cTable[a-1].ramptex;
		
		// split - determine whether a path gets split into little pieces or stays whole on export
		if 		(a==0&&cTable[0].psplit<0) cTable[0].psplit = dTable[0].psplit;
		else if (a>0&&cTable[a].psplit<0)  cTable[a].psplit = cTable[a-1].psplit;
		
		//range_start
		if 		(a==0&&cTable[0].range_start==-1) 	cTable[0].range_start = dTable[0].range_start;
		else if (a>0&&cTable[a].range_start==-1) 	cTable[a].range_start = cTable[a-1].range_start;
		
		//range_end
		if 		(a==0&&cTable[0].range_end==-1) 	cTable[0].range_end = dTable[0].range_end;
		else if (a>0&&cTable[a].range_end==-1) 		cTable[a].range_end = cTable[a-1].range_end;
		
		// range fix
		if 		(cTable[a].range_start>cTable[a].range_end) 	cTable[a].range_start=0;
		if 		(cTable[a].range_end<cTable[a].range_start) 	cTable[a].range_end=100;
		
		//transit_tri
		if 		(a==0&&cTable[0].transit_tri<0) cTable[0].transit_tri = dTable[0].transit_tri;
		else if (a>0&&cTable[a].transit_tri<0)  cTable[a].transit_tri = cTable[a-1].transit_tri;
		
		//transit_round
		if 		(a==0&&cTable[0].transit_round<0) cTable[0].transit_round = dTable[0].transit_round;
		else if (a>0&&cTable[a].transit_round<0)  cTable[a].transit_round = dTable[a-1].transit_round; // ??????
		
		// gaps
		if 		(a==0&&cTable[0].gaps<0) 	cTable[0].gaps = dTable[0].gaps;
		else if (a>0&&cTable[a].gaps<0) 	cTable[a].gaps = cTable[a-1].gaps;
		
		// scale
		if 		(a==0&&!cTable[0].scale.IsSet) 	cTable[0].scale = dTable[0].scale;
		else if (a>0&&!cTable[a].scale.IsSet) 	cTable[a].scale = dTable[0].rot_src;//cTable[a-1].scale;
		
		// scale_src
		if 		(a==0&&!cTable[0].scale_src.IsSet) 	cTable[0].scale_src = dTable[0].scale_src;
		else if (a>0&&!cTable[a].scale_src.IsSet) 	cTable[a].scale_src = dTable[0].rot_src;//cTable[a-1].scale_src;
		
		// rot
		if 		(a==0&&!cTable[0].rot.IsSet) 	cTable[0].rot = dTable[0].rot;
		else if (a>0&&!cTable[a].rot.IsSet) 	cTable[a].rot = dTable[0].rot_src;//cTable[a-1].rot;
		
		// rot_src
		if 		(a==0&&!cTable[0].rot_src.IsSet) 	cTable[0].rot_src = dTable[0].rot_src;
		else if (a>0&&!cTable[a].rot_src.IsSet) 	cTable[a].rot_src = dTable[0].rot_src;//cTable[a-1].rot_src;
		
		// move
		if 		(a==0&&!cTable[0].move.IsSet) 	cTable[0].move = dTable[0].move;
		else if (a>0&&!cTable[a].move.IsSet) 	cTable[a].move = dTable[0].rot_src;//cTable[a-1].move;
		
		// gaplen
		if 		(a==0&&cTable[0].gaplen<0) 	cTable[0].gaplen = dTable[0].gaplen;
		else if (a>0&&cTable[a].gaplen<0) 	cTable[a].gaplen = cTable[a-1].gaplen;
		
		// d_enable
		if 		(a==0&&cTable[0].d_enable<0) 	cTable[0].d_enable = dTable[0].d_enable;
		else if (a>0&&cTable[a].d_enable<0) 	cTable[a].d_enable = cTable[a-1].d_enable;
		
		// d_autoyaw
		if 		(a==0&&cTable[0].d_autoyaw<0) 	cTable[0].d_autoyaw = dTable[0].d_autoyaw;
		else if (a>0&&cTable[a].d_autoyaw<0) 	cTable[a].d_autoyaw = cTable[a-1].d_autoyaw;
		
		// d_autopitch
		if 		(a==0&&cTable[0].d_autopitch<0) 	cTable[0].d_autopitch = dTable[0].d_autopitch;
		else if (a>0&&cTable[a].d_autopitch<0) 		cTable[a].d_autopitch = cTable[a-1].d_autopitch;
		
		// d_pos
		if 		(a==0&&cTable[0].d_pos<0) 	cTable[0].d_pos = dTable[0].d_pos;
		else if (a>0&&cTable[a].d_pos<0) 		cTable[a].d_pos = cTable[a-1].d_pos;
		
		// d_separate
		if 		(a==0&&cTable[0].d_separate<0) 		cTable[0].d_separate = dTable[0].d_separate;
		else if (a>0&&cTable[a].d_separate<0) 		cTable[a].d_separate = cTable[a-1].d_separate;
		
		// d_autoname
		if 		(a==0&&cTable[0].d_autoname<0) 		cTable[0].d_autoname = dTable[0].d_autoname;
		else if (a>0&&cTable[a].d_autoname<0) 		cTable[a].d_autoname = cTable[a-1].d_autoname;
		
		// nulltex
		if 		(a==0&&cTable[0].nulltex=="UNSET") 	cTable[0].nulltex = dTable[0].nulltex;
		if 		(a>0&&cTable[a].nulltex=="UNSET") 	cTable[a].nulltex = cTable[a-1].nulltex;
		
		// spike_height
		if 		(a==0&&cTable[0].spike_height<0) 	cTable[0].spike_height = dTable[0].spike_height;
		else if (a>0&&cTable[a].spike_height<0) 	cTable[a].spike_height = cTable[a-1].spike_height;
		
		// d_pos_rand
		if 		(a==0&&!cTable[0].d_pos_rand.IsSet) 	cTable[0].d_pos_rand = dTable[0].d_pos_rand;
		else if (a>0&&!cTable[a].d_pos_rand.IsSet) 	cTable[a].d_pos_rand = cTable[a-1].d_pos_rand;
		
		// d_rotz_rand
		if 		(a==0&&!cTable[0].d_rotz_rand.IsSet) 	cTable[0].d_rotz_rand = dTable[0].d_rotz_rand;
		else if (a>0&&!cTable[a].d_rotz_rand.IsSet) 	cTable[a].d_rotz_rand = cTable[a-1].d_rotz_rand;
		
		// d_movey_rand
		if 		(a==0&&!cTable[0].d_movey_rand.IsSet) 	cTable[0].d_movey_rand = dTable[0].d_movey_rand;
		else if (a>0&&!cTable[a].d_movey_rand.IsSet) 	cTable[a].d_movey_rand = cTable[a-1].d_movey_rand;
		
		// d_draw
		if 		(a==0&&cTable[0].d_draw<0) 	cTable[0].d_draw = dTable[0].d_draw;
		else if (a>0&&cTable[a].d_draw<0) 	cTable[a].d_draw = cTable[a-1].d_draw;
		
		// d_skip
		if 		(a==0&&cTable[0].d_skip<0) 	cTable[0].d_skip = dTable[0].d_skip;
		else if (a>0&&cTable[a].d_skip<0) 	cTable[a].d_skip = cTable[a-1].d_skip;

		// d_draw_rand
		if 		(a==0&&cTable[0].d_draw_rand<0) cTable[0].d_draw_rand = dTable[0].d_draw_rand;
		else if (a>0&&cTable[a].d_draw_rand<0) 	cTable[a].d_draw_rand = cTable[a-1].d_draw_rand;
		
		if (dev) cout << "     Arc " << a << " rad: " << cTable[a].rad << "\t offset: " << cTable[a].offset << "\t type: " << cTable[a].type << "\t res: " << cTable[a].res << "\t shift: " << cTable[a].shift << "\t height: " << cTable[a].height /*<< "\t tri: " << cTable[a].tri << "\t ramp: " << cTable[a].ramp << "\t round: " << cTable[a].round*/ << endl;
	}
}


void file::LoadMap_GetEntities()
{
	bool dev = 0;
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
			if (dev) cout << " Entity detected - ID " << E.eID << endl;
		}
	}
	
	// determine end pos of each entity and save content to new string
	for (int i = 0, t_ents = EntityList.size(); i<EntityList.size(); i++)
	{
		entity &Entity = EntityList[i];
		if (i==t_ents-1)	Entity.pos_end = str_map.length();
		else				Entity.pos_end = EntityList[i+1].pos_start;
		
		Entity.content = str_map.substr(Entity.pos_start, Entity.pos_end-Entity.pos_start);
	}
	
	// determine entity types
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
	}
	
	// determine internal head end pos
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
	
	// check if entity is part of a detail group
	for (int i = 0, c=0; i<EntityList.size(); i++)
	{
		entity &Entity = EntityList[i];
		
		int find = 0, skip1 = 0, skip2 = 0;
		string str_skip1 = "info_curve";
		find = Entity.content.find("\"m2c_d_group\" \"");
		skip1 = Entity.content.find(str_skip1);
		if (skip1==-1)
		if (find!=-1)
		{
			find += 15; // put cursor to beginning of value
			int groupname_end = Entity.content.find("\"",find);
			if ( groupname_end!=-1 && groupname_end-find>0 )
			{
				Entity.groupname = Entity.content.substr(find, groupname_end-find);
				Entity.IsDetail = 1;
			}
		} else {
			if(Entity.type==2) {
				Entity.IsDetail = 1;
				Entity.groupname = "custom_dg0"+to_string(c);
				if (dev)cout << " Found empty DetailGroup? Name [" << Entity.groupname << "] type [" << Entity.type << "]" <<endl;
				c++;
			}
		}
		if (dev)cout << " Found DetailGroup? " << Entity.IsDetail << " Name " << Entity.groupname << " type " << Entity.type <<endl;
	}
	
	// Get specific Keys and KeyValues of all Solid and Point Entities
	for (int i = 0; i<EntityList.size(); i++)
	{
		entity &Entity = EntityList[i];
		Entity.GetKeyValues();
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
		if (dev)cout << " count point entities without detail group name as individual detail group ..." << endl;
		if (Entity.IsDetail && Entity.type==2 && gname.size()==0) {
			t_dgroups++;
			if (dev)cout << "   found one! Entity #" << i << " class " << Entity.key_classname << endl;
		}
	}
	t_dgroups--; // always subtract 1 from total group amount, because empty group names (empty groupname = non-detail brush) dont count, but are included during search too
	
	if (dev) {
	cout << " Total unique detail groups " << t_dgroups << endl;
	for (int i = 0; i<EntityList.size(); i++) {
		entity &Entity = EntityList[i];
		cout << "   #" << i << " " << Entity.groupname << endl;}
	}
	
	dmGroup = new group[t_dgroups]; // create original detail object group
	if (dev) cout << "Created " << t_dgroups << " detail object groups! " << endl;
	for (int i = 0; i<t_dgroups; i++)
		dmGroup[i].IsSrcMap = 1;
	
	// get total amount of map brushes
	for (int i = 0; i<EntityList.size(); i++)
	{
		entity &Entity = EntityList[i];
		
		if (!Entity.IsDetail)
		mGroup->t_brushes += Entity.t_brushes;
	}
	if (dev) cout << " Total Map Brushes " << mGroup->t_brushes << endl;
	
	// create Brushes of each Entity (World and Solid)
	if (dev) cout << " Creating Brushes for "<<EntityList.size()<<" Entities... " << endl;
	for (int i = 0; i<EntityList.size(); i++)
	{
		entity &Entity = EntityList[i];
		Entity.CreateBrushes();
	}
	
	// assign detail group ID to each entity brush and Entity
	if (dev) cout << "  Assigning detail group ID to each entity brush... " << endl;
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
					if (dev) cout << " Entity #" << e << " dID " << Entity.dID << " type " << Entity.type << endl;
					for (int b = 0; b<Entity.t_brushes; b++)
					{
						brush &Brush = Entity.Brushes[b];
						Brush.dID = i-1;
						if (dev) cout << "    Brush #" << b << " dID " << Brush.dID << endl;
					}
				} else if (Entity.groupname=="") {
					Entity.dID = i-1;
					if (dev) cout << " Entity #" << e << " dID " << Entity.dID << " type " << Entity.type << " Name " << Entity.groupname << " class " << Entity.key_classname <<endl;
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
		group &dGroup = dmGroup[dg];
		for (int i = 0; i<EntityList.size(); i++)
		{
			entity &Entity = EntityList[i];
			int dID = Entity.dID;
			
			if (Entity.IsDetail && Entity.dID==dg)
			dmGroup[dID].t_brushes += Entity.t_brushes;
			
			if (Entity.type==2 && Entity.IsDetail && Entity.dID==dg && Entity.key_classname!="info_detailgroup" && Entity.key_classname!="info_curve")
			dmGroup[dID].t_ents++;
		}
		if (dev) cout << " Total Brushes for Detail Group " << dg << " - " << dmGroup[dg].t_brushes << endl;
		if (dev) cout << " Total Entities for Detail Group " << dg << " - " << dmGroup[dg].t_ents << endl;
	}
	
	// create Point Entities for Detail Groups
	for (int dg = 0; dg<t_dgroups; dg++)
	{
		group &dGroup = dmGroup[dg];
		dGroup.Entities = new entity[dGroup.t_ents];
	}
	
	// copy point entities that are part of a detail group to the respective detail group
	for (int dg = 0, ed = 0; dg<t_dgroups; dg++)
	{
		group &dGroup = dmGroup[dg];
		for (int e = 1; e<EntityList.size(); e++)
		{
			entity &Source = EntityList[e];
			int dID = Source.dID;
			if (dev) cout << " DGroup " << dg << " Entity " << e << " List #" << ed<< endl;
			if ( Source.type==2 && Source.IsDetail && Source.dID==dg && Source.key_classname!="info_detailgroup")
			{
				//for (int ed = 0; ed<dGroup.t_ents; ed++)
				{
					if (dev) cout << "   Copying point entity " << e << " of dgroup " << dg << " to this dgroups Entity list (current #" << ed<<")"<< endl;
					entity &Entity = dGroup.Entities[ed];
					Entity.CopySimple(Source);
					ed++;
				}
			}
		}
		ed=0;
	}

	// after getting values from info_detailgroup, they can be cut from the entity list
	/*for (int i = 0; i<EntityList.size(); i++)
	{
		entity &Entity = EntityList[i];
		if (Entity.key_classname=="info_detailgroup") {
			Entity.IsDetail=0;
		}
	}*/
	
	// scan all solid entities (brush eID) and point entities of a detail group for property keyvalues (starting angle, pitch, yaw, separation, etc.)
	// double values are being overwritten, there can only be one command/entity that controls the whole detail group
	if (dev) cout << "  Scanning all entities of a detail group for property keyvalues..." << endl;
	for (int d = 0; d<t_dgroups; d++)
	{
		group &dGroup = dmGroup[d];
		if (dev) cout << "     Detail Group #" << d << " total point ents " << dGroup.t_ents << " total brushes " << dGroup.t_brushes << endl;
		for (int i = 0; i<EntityList.size(); i++)
		{
			entity &Entity = EntityList[i];
			if (Entity.type>=1 && Entity.IsDetail && Entity.dID==d)
			{
				if (dev) cout << "     Entity #" << i << " class: " << Entity.key_classname << " Keys: enable " << Entity.d_enable << " angle " << Entity.d_pos << " pitch " << Entity.d_autopitch << " yaw " << Entity.d_autoyaw << " sep " << Entity.d_separate << " d_autoname " << Entity.d_autoname  <<endl;
				if (Entity.d_pos>0) 		dGroup.d_pos 		= Entity.d_pos; 		//else 								dGroup.d_pos = 0;
				if (Entity.d_autopitch>0) 	dGroup.d_autopitch 	= Entity.d_autopitch;		else if (Entity.d_autopitch==-1) 	dGroup.d_autopitch 	= 0;
				if (Entity.d_autoyaw>0) 	dGroup.d_autoyaw 	= Entity.d_autoyaw;			else if (Entity.d_autoyaw==-1) 		dGroup.d_autoyaw 	= 0;
				if (Entity.d_separate>0) 	dGroup.d_separate 	= Entity.d_separate;		else if (Entity.d_separate==-1) 	dGroup.d_separate 	= 0;
				if (Entity.d_autoname>0) 	dGroup.d_autoname 	= Entity.d_autoname;		else if (Entity.d_autoname==-1) 	dGroup.d_autoname 	= 0;
				if (Entity.d_enable>0) 		dGroup.d_enable 	= Entity.d_enable;			else if (Entity.d_enable==-1) 		dGroup.d_enable 	= 0;
				if (Entity.d_pos_rand.IsSet) 	dGroup.d_pos_rand = Entity.d_pos_rand;
				if (Entity.d_rotz_rand.IsSet) 	dGroup.d_rotz_rand = Entity.d_rotz_rand;
				if (Entity.d_movey_rand.IsSet) 	dGroup.d_movey_rand = Entity.d_movey_rand;
				if (Entity.d_draw>0) 		dGroup.d_draw 		= Entity.d_draw;		else if (Entity.d_draw==-1) 		dGroup.d_draw 		= 0;
				if (Entity.d_skip>0) 		dGroup.d_skip 		= Entity.d_skip;		else if (Entity.d_skip==-1) 		dGroup.d_skip 		= 0;
				if (Entity.d_draw_rand>0) 	dGroup.d_draw_rand 	= Entity.d_draw_rand;	else if (Entity.d_draw_rand==-1) 	dGroup.d_draw_rand 	= 0;
				if (dev) cout << "     dGroup #" << d << " Name: " << Entity.groupname << " angle: " << dGroup.d_pos << " pitch " << dGroup.d_autopitch << " yaw " << dGroup.d_autoyaw << " separate " << dGroup.d_separate << " enable " << dGroup.d_enable << " d_autoname " << dGroup.d_autoname <<endl;
			}
		}
	}

	/*for (int i = 0; i<EntityList.size(); i++)
	{
		entity &Entity = EntityList[i];
		cout << " Entity #" << i << endl;
		cout << "   Type: \t" << Entity.type << endl;
		cout << "   Start: \t" << Entity.pos_start << endl;
		cout << "   End: \t" << Entity.pos_end << endl;
		cout << "   Head: \t" << Entity.head_end << endl;
		cout << "   Brush: \t" << Entity.t_brushes << endl;
	}
	
	getch();*/
}


void file::TransformSource()
{
	bool dev = 0;
	if (dev) cout << " Applying custom Source transformations..." << endl;
	if (dev) getch();
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
			if (dev) cout << "   Group "<<g<<"..." <<endl;
			for (int b = 0; b<Group.t_brushes; b++)
			{
				brush &Brush = Group.Brushes[b];
				if (Brush.valid)
				{
					if (dev&&b==0) cout << "     Scale "  << scale_x << endl;
					if (cTable[g].scale_src.IsSet)
					Brush.ScaleOrigin(scale_x, Group.Origin);
					
					if (dev&&b==0) cout << "     Rot "  << rot_x << "," << rot_y << "," <<rot_z << endl;
					if (cTable[g].rot_src.IsSet)
					Brush.RotOrigin(rot_x,rot_y,rot_z, Group.Origin);
				}
			}
			//if (dev) sGroup[g].ExportGroupToMap(p_path+name+"_transformed"+to_string(g+1)+".map");
			
			if (cTable[g].rot_src.y!=0||cTable[g].rot_src.z!=0)
			Group.RoundBrushVertices(1);
			
			// Detail Objects
			group_set &Set = dSet[g];
			for (int d = 0; d<t_dgroups; d++)
			{
				group &dGroup = Set.Groups[d];
				vertex nOrigin = Group.Origin;
				//nOrigin.x = 0;
				
				for (int b = 0; b<dGroup.t_brushes; b++)
				{
					brush &Brush = dGroup.Brushes[b];
					
					if (cTable[g].scale_src.IsSet)	Brush.ScaleOrigin(scale_x, nOrigin);
					if (cTable[g].rot_src.IsSet)	Brush.RotOrigin(rot_x,rot_y,rot_z, nOrigin);
				}
				for (int e = 0; e<dGroup.t_ents; e++)
				{
					entity &Entity = dGroup.Entities[e];
					
					if (cTable[g].scale_src.IsSet)	Entity.ScaleOrigin(scale_x,nOrigin);
					if (cTable[g].rot_src.IsSet)	Entity.RotateOrigin(rot_x,rot_y,rot_z, nOrigin);
					Euler RotEuler(rot_x,rot_y,rot_z);
					if (cTable[g].rot_src.IsSet)	Entity.RotateEntity(RotEuler,1);
				}
				dGroup.GetDimensions(1);
				
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
			if (dev) cout << "   Getting Dimensions..." <<endl;
			Group.GetDimensions(1);
			if (dev) cout << "   Checking Brush Validity..." <<endl;
			Group.CheckBrushValidity();
			if (dev) cout << "   Getting Face Orients..." <<endl;
			Group.GetBrushFaceOrients();
			if (dev) cout << "   Getting Texture Aligns..." <<endl;
			Group.GetBrushTVecAligns();
			if (dev) cout << "   Updating Vertex List..." <<endl;
			Group.GetGroupVertexList();
			if (dev) cout << "   Checking Brush Divisibility..." <<endl;
			Group.CheckBrushDivisibility();
		}
	}
}


void file::TransformFinal(int g)
{
	bool dev = 0;
	float scale_x = cTable[g].scale.x;
	//float scale_y = cTable[g].scale.y;
	//float scale_z = cTable[g].scale.z;
	
	float move_x = cTable[g].move.x;
	float move_y = cTable[g].move.y;
	float move_z = cTable[g].move.z;
	
	float rot_x = cTable[g].rot.x;
	float rot_y = cTable[g].rot.y;
	float rot_z = cTable[g].rot.z;
	
	if (dev) cout << " ######## Final Transformation: ######## " << cTable[g].rot << endl;
	// Apply custom transformations
	for (int b = 0; b<bGroup[g].t_brushes; b++)
	{
		brush &Brush = bGroup[g].Brushes[b];
		
		if (cTable[g].scale.IsSet) {
			Brush.Scale(scale_x);
			//Brush.Scale(scale_y);
			//Brush.Scale(scale_z);
		}
		
		if (cTable[g].rot.IsSet)
			Brush.Rot(rot_x,rot_y,rot_z);
			
		if (cTable[g].move.IsSet)
			Brush.Move(move_x,move_y,move_z,1);
		
		if (Brush.Tri!=nullptr)
		{
			for (int bt = 0; bt<Brush.t_tri; bt++)
			{
				brush &TriBrush = Brush.Tri[bt];
				if (cTable[g].scale.IsSet)	TriBrush.Scale(scale_x);
				if (cTable[g].rot.IsSet)	TriBrush.Rot(rot_x,rot_y,rot_z);
				if (cTable[g].move.IsSet)	TriBrush.Move(move_x,move_y,move_z,1);
			}
		}
	}
	
	// detail groups
	for (int d = 0; d<t_dgroups; d++)
	{
		group &Group = dSet[g].Groups[d];
		for (int b = 0; b<Group.t_brushes; b++)
		{
			brush &Brush = Group.Brushes[b];
			
			if (cTable[g].scale.IsSet)	Brush.Scale(scale_x);
			if (cTable[g].rot.IsSet)	Brush.Rot(rot_x,rot_y,rot_z);
			if (cTable[g].move.IsSet)	Brush.Move(move_x,move_y,move_z,1);
		}
		for (int e = 0; e<Group.t_ents; e++)
		{
			entity &Entity = Group.Entities[e];
			
			if (cTable[g].scale.IsSet)	Entity.ScaleOrigin(scale_x, Zero);
			if (cTable[g].rot.IsSet)	Entity.RotateOrigin(rot_x,rot_y,rot_z, Zero);
			Euler RotEuler(rot_x,rot_y,rot_z);
			if (cTable[g].rot.IsSet)	Entity.RotateEntity(RotEuler,1);
			if (cTable[g].move.IsSet)	Entity.Origin.move(move_x,move_y,move_z);
		}
	}
}


void file::createGroupMap()
{
	bool dev = 0;
	
	mGroup = new group;
	mGroup->IsSrcMap = 1;
	int &tbc = mGroup->t_brushes;
	int &tfc = mGroup->t_faces;
	
	if (dev) cout << " LoadMap_GetEntities..."<<endl;
	LoadMap_GetEntities();
	
	// copy entity brushes that are not of a detail type into curve source object
	mGroup->Brushes = new brush[tbc];
	if (dev) cout << "Created " << tbc << " Map Brushes!" << endl;
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
	
	if (dev) cout << "  GetDimensions..." << endl;
	mGroup->GetDimensions(0);
	
	if (dev) getch();
	/*cout << " Dimensions: x " << mGroup->Dimensions.xs << " " << mGroup->Dimensions.xb << endl;
	cout << " Dimensions: y " << mGroup->Dimensions.ys << " " << mGroup->Dimensions.yb << endl;
	cout << " Dimensions: z " << mGroup->Dimensions.zs << " " << mGroup->Dimensions.zb << endl;
	cout << " Origin: " << mGroup->Origin << endl;*/
}


void file::LoadMap_GetTexInfo()
{
	bool dev = 0;
	string WadFileString = "";
	int &tbc = mGroup->t_brushes;
	
	// look for Textures and WADs used in Map File
	if (dev) cout << " Looking for Textures and WADs used in Map File..." << endl;
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
		cout << "|    [INFO] Found " << WadListMap.size() << " WAD files used in this map file:" << endl;
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
				cout << " of them were found!" << endl;
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
	if(dev) cout << "Loading texture informations from WAD3 files..." << endl;
	vector<string> fail_tex;
	for (int i = 0; i<tTable_name.size(); i++) 
	{
		string &texn = tTable_name[i];
		int &texw = tTable_width[i];
		int &texh = tTable_height[i];
		
		int tID = -1;
		int wID = 0;
		if(dev) cout << " looking for texture " << texn << " in previously loaded wad files... " << endl;
		for (int j=0; j<WADFiles.size(); j++)
		{
			tID = WADFiles[j].FindTexture(texn);
			if(dev) cout << " found tID " << tID << " WAD #" << j << " ("<<WADFiles[j].FilePath<<")" <<endl;
			if (tID!=-1) { wID=j; break; }
		}
		// when texture name was found, add its width and height to the current texture list entry
		if (tID!=-1)
		{
			WADFiles[wID].GetTexInfo(tID, texw, texh);
		}
		else // if texture name wasnt found, use default size
		{
			if(dev) cout << "  Texture name " << texn << " not found..." << endl;
			fail_tex.push_back(texn);
			//texw = 128;
			//texh = 128;
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
	
	if(dev) cout << "Final Texture List and Info:" << endl;
	if(dev)
	for (int i = 0; i<tTable_name.size(); i++) 
	{
		string &texn = tTable_name[i];
		int &texw = tTable_width[i];
		int &texh = tTable_height[i];
		
		cout << " Tex #" << i << " Name " << texn << " width " << texw << " height " << texh << endl;
	}
	if(dev) getch();
}

// scan all faces for Textures and add those to the files texture list
void file::LoadMap_GetTexInfoScanBrush(brush &Brush)
{
	bool dev = 0;
	
	if(dev) cout << "Scanning all faces for Textures and add those to the files texture list..." << endl;
	for(int f = 0; f<Brush.t_faces; f++)
	{
		face &Face = Brush.Faces[f];
		int tex_count = tTable_name.size();
		if(dev) cout << " Checking Texture "<<Face.Texture<<" of Face " << f << " tTable_name.size: " << tex_count << endl;
		if (tex_count==0)
		{
			tTable_name.push_back(Face.Texture);
			tTable_width.push_back(128);
			tTable_height.push_back(128);
			if(dev) Face.tID = 0;
		}
		else
		{
			for (int j = 0; j<tex_count; j++)
			{
				if (Face.Texture==tTable_name[j]) // current Face Texture is already in the list
				{
					Face.tID=j;
					if(dev) cout << "   Face.Texture("<<Face.Texture<<")==tTable_name["<<j<<"]("<<tTable_name[j]<<")... Aborting! Face.tID " << Face.tID << endl;
					break;
				}
				else if (j==tex_count-1)
				{
					Face.tID=j+1;
					if(dev) cout << "   Reached end of tTable_name list. Adding Face.Texture ("<<Face.Texture<<") to tTable_name. Face.tID " << Face.tID << endl;
					tTable_name.push_back(Face.Texture);
					tTable_width.push_back(128);
					tTable_height.push_back(128);
				}
			}
			if(dev) cout << endl;
		}
		//cout << "   Final Face Texture ID " << Face.tID << endl; //////////////////////////
	}
	if(dev) cout << " Texture List size now " << tTable_name.size() << "!" << endl;
	//getch();
}

void file::LoadMap_ConvertWorld2Face()
{
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
				//cout << " Brush " << b << " Face " << f << " Tex" << Face.Texture << endl;
				if (Face.fID==2&&Face.HasWorldAlign)
				{
					gvector &Vec = *Face.VecV;
					gvector EdgeEquiv;
					
					//check if Face Edge v0/v1 is relevant for this Tex Vector, flip it if not
					gvector Edge = GetVector( Face.VerticesC[0],Face.VerticesC[1] );
					gvector EdgeN = Normalize(Edge);
					//cout << "     Getting Dot of Edge(N) " << EdgeN << " and Vec " << Vec;
					float Dot = GetDot(EdgeN, Vec);
					//cout << " Dot " << Dot;
					if (Dot<0) { Edge.flip(); EdgeN.flip(); } /*EdgeN.flip(); Dot = GetDot(EdgeN, Vec); cout << " - Flipped Edge! Dot now " << Dot << endl; } else cout << endl;*/
					
					//cout << "     TVec Old " << Vec << " Edge " << Edge << endl;
					
					// fix Texture Scale
					float TexLenO = GetAdjaLen (Edge, Vec);
					float TexLenN = GetVecLen (Edge);
					float m = TexLenO/TexLenN;
					
					//cout << "     TexLenO " << TexLenO << " TexLenN " << TexLenN << " Mod " << m << endl << endl;
					
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

void file::createDetailGroup()
{
	bool dev = 0;
	
	// create final detail object group
	if (dev) cout << " Creating final detail object group..." << endl;
	dSet = new group_set[mGroup->t_arcs];
	if (dev) cout << "   " << mGroup->t_arcs << " Group Sets created!" << endl;
	for (int s = 0; s<mGroup->t_arcs; s++)
	{
		group_set &Set = dSet[s];
		Set.Groups = new group[t_dgroups];
		Set.t_groups = t_dgroups;
		if (dev) cout << "     " << t_dgroups << " Detail groups created inside Set #"<<s<<"!" << endl;
		
		for (int d = 0; d<Set.t_groups; d++)
		{
			if (dev) cout << "       brushcount is res "<< cTable[s].res <<" * " << " dmGroup.t_brushes " << dmGroup[d].t_brushes << " = " << cTable[s].res * dmGroup[d].t_brushes << endl;
			group &dGroup = Set.Groups[d];
			
			// detail group properties
			dGroup.CopyProps(dmGroup[d]);
			
			// solid entities
			int brushcount = cTable[s].res * dmGroup[d].t_brushes;
			dGroup.Brushes = new brush[ brushcount ];
			if (dev) cout << "       " << brushcount << " Brushes created inside Detail Group #"<<d<<"!" << endl;
			dGroup.t_brushes = brushcount;
			
			// point entities
			int entcount = cTable[s].res * dmGroup[d].t_ents;
			dGroup.Entities = new entity[ entcount ];
			dGroup.t_ents = entcount;
			dGroup.Dimensions = dmGroup[d].Dimensions;
			dGroup.Origin = dmGroup[d].Origin;
		}
	}
	if (dev) getch();
	// fill final detail object group
	if (dev) cout << endl << " Filling final detail object group..." << endl;
	for (int a = 0; a<mGroup->t_arcs; a++)
	{
		group_set &Set = dSet[a];
		
		for (int d = 0; d<Set.t_groups; d++)
		{
			group &dGroup = Set.Groups[d];
			if (dev) cout << "   " << t_dgroups << " Filling Brushes of Detail Group #"<<d<<"!" << endl;
			
			// Solid Brushes of this detail group
			for (int b = 0, sb=0, sec=0; b<dGroup.t_brushes; b++)
			{
				brush &Brush = dGroup.Brushes[b];
				Brush.Copy(dmGroup[d].Brushes[sb]);
				Brush.SecID = sec;
				if (dev) cout << "     Brush "<<b<<" source Brush " << sb << " (of max " << dmGroup[d].t_brushes << ")" << " sec " << sec << " (of max " << cTable[a].res << ")" << endl;
				
				sec++;
				if (sec==cTable[a].res) {
					if (dev) cout << "       Final Section reached! Resetting Sec to 0. Increasing source brush to " << sb+1 << " of max " << dmGroup[d].t_brushes << endl;
					sec=0;
					sb++;
				}
			}
			
			// Point Entities of this detail group
			for (int e = 0, se=0, sec=0; e<dGroup.t_ents; e++)
			{
				entity &Entity = dGroup.Entities[e];
				Entity.CopySimple(dmGroup[d].Entities[se]);
				Entity.SecID = sec;
				if (dev) cout << "     Entity "<<e<<" source Entity " << se << " type "<< Entity.Keys[0].value << " (of max "<<dmGroup[d].t_ents<<")" << " sec " << sec << " (of max " << cTable[a].res << ") Euler " << Entity.Angles << endl;
				
				sec++;
				if (sec==cTable[a].res) {
					if (dev) cout << "       Final Section reached! Resetting Sec to 0. Increasing source entity to " << se+1 << " (of max "<< dmGroup[d].t_ents <<")" << endl;
					sec=0;
					se++;
				}
			}
		}
	}
	
	// apply draw, skip or random draw settings
	for (int a = 0; a<mGroup->t_arcs; a++)
	{
		bool dev = 0;
		group_set &Set = dSet[a];
		int G_DRAW = 0; if (cTable[a].d_draw>0) G_DRAW = cTable[a].d_draw;
		int G_SKIP = 0; if (cTable[a].d_skip>0) G_SKIP = cTable[a].d_skip;
		bool G_DRAW_RAND = 0; if (cTable[a].d_draw_rand==1) G_DRAW_RAND = 1;
		
		if (dev) cout << " G_DRAW " <<  G_DRAW << " G_SKIP " << G_SKIP << " G_DRAW_RAND " << G_DRAW_RAND << endl;
		for (int d = 0; d<Set.t_groups; d++)
		{
			group &dGroup = Set.Groups[d];
			int L_DRAW = 0; if (dGroup.d_draw>0) L_DRAW = dGroup.d_draw;
			int L_SKIP = 0; if (dGroup.d_skip>0) L_SKIP = dGroup.d_skip;
			bool L_DRAW_RAND = 0; if (dGroup.d_draw_rand==1||(G_DRAW_RAND&&dGroup.d_draw_rand<0)) L_DRAW_RAND = 1;
			
			int F_DRAW = G_DRAW; if (L_DRAW>0) F_DRAW = L_DRAW;
			int F_SKIP = G_SKIP; if (L_SKIP>0) F_SKIP = L_SKIP;
			if (dev) cout << " L_DRAW " <<  L_DRAW << " L_SKIP " << L_SKIP << " L_DRAW_RAND " << L_DRAW_RAND << " F_DRAW " << F_DRAW << " F_SKIP " << F_SKIP << endl;
			
			bool R_DRAW[cTable[a].res]; for (int i=0; i<cTable[a].res; i++) R_DRAW[i] = 1;
			if ( (G_DRAW_RAND&&L_DRAW_RAND) || (!G_DRAW_RAND&&L_DRAW_RAND) )
			for (int i=0; i<cTable[a].res; i++) {
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
				
				if ( (G_DRAW_RAND&&L_DRAW_RAND) || (!G_DRAW_RAND&&L_DRAW_RAND) ) {
					Entity.draw = R_DRAW[sec];
					if (dev) cout << " sec " << sec << " random draw is " << R_DRAW[sec] <<endl;
				} else {
					if (F_DRAW>0 && sec!=draw_ctr) {
						Entity.draw = 0;
						if (dev) cout << " sec " << sec << " is != draw_ctr " << draw_ctr << " Entity gets SKIPPED! " <<endl;
					} else if (F_DRAW>0 && sec==draw_ctr) {
						if (dev) cout << " sec " << sec << " is == draw_ctr " << draw_ctr << " Entity gets DRAWN! draw_ctr now " << draw_ctr+F_DRAW <<endl;
						draw_ctr += F_DRAW;
					}
					
					if (F_SKIP>0 && sec==skip_ctr) {
						Entity.draw = 0;
						if (dev) cout << " sec " << sec << " is = skip_ctr " << skip_ctr << " Entity gets SKIPPED! skip_ctr now " << skip_ctr+F_SKIP<< endl;
						skip_ctr += F_SKIP;
					}
				}
			}
		}
	}
	
	
	if (dev) getch();
}

void file::createGroupSource()
{
	bool dev = 0;
	// count rad-commands from settings list to get max amount of indivdual arcs to be generated
	if (dev) cout << " count rad-commands from settings list..." << endl;
	int &t_arcs = mGroup->t_arcs;
	if (dev) cout << " t_arcs " << t_arcs << endl;
	for (int i = 0; i<settings.size(); i++) {
		if (settings[i]=="rad") t_arcs++;
	}
	if (t_arcs==0) t_arcs = 1;
	if (InternalMapSettings) t_arcs = t_iarcs;
	//cout << "File contains " << t_arcs << " rad commands." << endl; getch();
	if (dev) cout << " t_arcs " << t_arcs << endl;
	sGroup = new group[t_arcs];
	for (int g = 0; g<t_arcs; g++)
	{
		if (mGroup->t_brushes-mGroup->invalids>0) {
		if (dev) cout << " #"<<g<<" Copying mGroup... invalids " << mGroup->invalids << " valids " << mGroup->t_brushes  << endl;
		sGroup[g].Copy(*mGroup);
		sGroup[g].IsSrcMap = 1;
		sGroup[g].gID = g;
		if (dev) cout << " mGroup copied to sGroup"<<g<<" invalids " << sGroup[g].invalids << " valids " << sGroup[g].t_brushes  << endl;
		//sGroup[g].ExportGroupToMap(p_path+name+"_fixed"+to_string(g+1)+".map"); getch();
		}
		else
		{
			cout << "|    [WARNING] Something went horribly wrong! The Source Map doesn't contain valid brushes ("<< mGroup->t_brushes-mGroup->invalids << ")!" << endl;
			getch(); 
		}
	}
	if (dev) getch();
}

void file::LoadMap()
{
	bool dev = 0;
	
	if (dev) cout << " createGroupMap..."<<endl;
	createGroupMap();
	
	if (dev) cout << " LoadMap_GetTexInfo..."<<endl;
	// get dimensions (width and height) of all textures that were used in the source map
	LoadMap_GetTexInfo();
	
	if (dev) cout << " Create Detail Objects..."<<endl;
	LoadMap_DetailObj();
	
	if (dev) cout << " CheckBrushValidity..."<<endl;
	mGroup->CheckBrushValidity(); // can brushes actually be turned into curves?
	mGroup->GetBrushFaceOrients(); // determine base/head and body faces
	
	if (dev) cout << " GetBrushTVecAligns..."<<endl;
	mGroup->GetBrushTVecAligns(); // see if texture vectors are aligned correctly and check their orientation
	
	if (dev) cout << " ReconstructMap..."<<endl;
	mGroup->ReconstructMap(); // get all of the missing vertices for further calculations (map-files only save 3 vertices per plane)
	
	if (dev) cout << " CheckBrushDivisibility..."<<endl;
	mGroup->CheckBrushDivisibility(); // check whether a brush can be triangulated or not
	
	if (dev) cout << " GetRconBrushShifts..."<<endl;
	mGroup->GetBrushShifts(); // get original texture shifts
	//mGroup->GetRconBrushShifts(); // get original texture shifts from recunstructed vertices
	
	if (dev) cout << " LoadMap_ConvertWorld2Face..."<<endl;
	LoadMap_ConvertWorld2Face(); // turn world alignment of faces into face alignment, so the new vectors can easily be created from a generated mesh 1:1
	
	//if (dev) cout << "  Exporting Raw Source to Mapfile..." << endl;
	//mGroup->ExportGroupToMap(p_path+name+"_source.map");
}

void file::LoadMap_DetailObj()
{
	bool dev = 0;
	// reconstruct original detail brushes and get their texute offsets
	if (dev) cout << "  Reconstructing original detail brushes..." << endl;
	if (dev) getch();
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
	
	// get original detail brushes Texture Shifts
	if (dev) cout << "  Getting original detail brushes Texture Shifts..." << endl;
	if (dev) getch();
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
		group &Group = dmGroup[dg];
		Group.Brushes = new brush[Group.t_brushes];
		if (dev) cout << "Created " << Group.t_brushes << " Brushes for Detail Group "<<dg<<"!" << endl;
		for (int e = 0; e<EntityList.size(); e++)
		{
			entity &Entity = EntityList[e];
			if (Entity.IsDetail && Entity.dID==dg)
			{
				for (int eb = 0; eb<Entity.t_brushes; eb++)
				{
					brush &Brush = dmGroup[dg].Brushes[b];
					
					Brush.Copy(Entity.Brushes[eb]);
					b++;
				}
			}
		}
		b = 0;
	}
	
	for (int dg = 0; dg<t_dgroups; dg++)
		dmGroup[dg].GetDimensions(1);
	
	// fix position of basic detail objects
	/*for (int i = 0; i<t_dgroups; i++)
	{
		group &Group = dmGroup[i];
		// get offset, if X position of objects origin is not 0
		if (Group.Origin.x!=0)
		{
			for (int b = 0; b<Group.t_brushes; b++)
			{
				brush &Brush = Group.Brushes[b];
				Brush.Move(-(Group.Origin.x),0,0,1);
			}
			for (int k = 0; k<Group.t_ents; k++)
			{
				entity &Entity = Group.Entities[k];
				Entity.Origin.x -= Group.Origin.x;
			}
		}
		Group.Origin.x = 0;
	}*/
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




/*void file::WriteFace(face &Face)
{
	ofstream mapfile;
	mapfile.open(path+name+"_arc.map", ofstream::app);
	
	mapfile << setprecision(8) << fixed;
	mapfile << Face.Vertices[0] << Face.Vertices[1] << Face.Vertices[2];
	
	mapfile << setprecision(8) << fixed;
	
	mapfile << Face.Texture << " ";
	mapfile << " [ ";
	mapfile << Face.VecX.x << " ";
	mapfile << Face.VecX.y << " ";
	mapfile << Face.VecX.z << " ";
	mapfile << Face.ShiftX << " ";
	mapfile << "] [ ";
	mapfile << Face.VecY.x << " ";
	mapfile << Face.VecY.y << " ";
	mapfile << Face.VecY.z << " ";
	mapfile << Face.ShiftY << " ";
	mapfile << "] ";
	mapfile << Face.Rot << " ";
	mapfile << Face.ScaleX << " ";
	mapfile << Face.ScaleY << " ";
	mapfile << endl;
	
	mapfile.close();
}*/


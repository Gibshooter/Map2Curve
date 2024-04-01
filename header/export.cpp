#include "file.h"
//#include "settings.h"
#include "group.h"
#include "utils.h"
#include "face.h"
#include "RMF.h"

#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <iomanip> // precision

using namespace std;

//enum keytype;
extern ctable *cTable;
extern group *mGroup;
extern group *sGroup;
extern group *bGroup;
extern group_set *DetailSet;
extern bool G_DEV;
extern string ROOT;
extern bool ISROOTED;


/* ===== FILE METHODS EXPORT ===== */


// Export Brush to .rmf-File (Valve Hammer Editors Rich Map Format)
void file::ExportToRMF()
{
	#if DEBUG > 0
	bool dev = 0;
	if (dev) cout << " Exporting Brush to .rmf-File..." << endl;
	#endif
	
	// custom output file
	string OutputFilePath;
	if (target!="ERR"&&target!="UNSET"&&target.length()>0) {
		OutputFilePath = target;
		
		// if user defined output path + filename has no RMF file extension, add one by replacing the existing file extension
		if(OutputFilePath.substr(OutputFilePath.length()-3,3)!="rmf")
		OutputFilePath.replace(OutputFilePath.length()-3,3,"rmf");
	}
	else
		OutputFilePath = p_path+name+"_curved.rmf";
	
	// Console Info Message
	cout << OutputFilePath;
	
	// Skipping Brushes with only Nullfaces
	for (int g = 0; g < mGroup->t_arcs; g++)
	{
		group &Group = bGroup[g];
		if (cTable[g].skipnull>0 && cTable[g].rmf>0)
		Group.CheckNULLBrushes();
	}
	
	// reset exported brush variable (from when curve was supported to map-file recently, which would otherwise lead to no exported brushes for type 2/3)
	for (int g = 0; g < mGroup->t_arcs; g++)
	{
		group &Group = bGroup[g];
		for (int b = 0; b < Group.t_brushes; b++)
		{
			brush &Brush = Group.Brushes[b];
			if (Group.IsSecInRange(Brush.SecID) && Brush.draw)
			{
				Brush.exported = 0;
				if (Brush.Tri!=nullptr)
				{
					for (int bt=0; bt<Brush.t_tri; bt++)
					Brush.Tri[bt].exported = 0;
				}
			}
		}
	}
	
	// Export to file
	RMF RMF_Buffer(OutputFilePath);
	
	// Write RMF File Header
	#if DEBUG > 0
	if (dev) cout << " Writing RMF File Header..." << endl;
	#endif

	RMF_Buffer.WriteHeader();

	// Write Vis Groups
	RMF_Buffer.WriteVisGroups((int)0);
	
	// Write Objects
	int n_obj = 0;
	unsigned long pos_n_obj = RMF_Buffer.WriteCounterGetPos(); // write counter int and get its position in the output stream to modify it later

	#if DEBUG > 0
	if (dev) cout << " Writing Objects. Counter atm 0. Saving counter pos for later update: " << pos_n_obj << endl;
	
	// Write world brushes of all curves
	if (dev) cout << " Writing World Curve Brushes..." << endl;
	#endif
	
	for (int g = 0; g < mGroup->t_arcs; g++)
	{
		group &Group = bGroup[g];
		
		if(cTable[g].rmf>0 && cTable[g].c_enable>0)
		{
			// Brushes
			for (int b = 0; b < Group.t_brushes; b++)
			{
				brush &Brush = Group.Brushes[b];
				int sec = Brush.SecID;
				
				if (Brush.entID==0 && Group.IsSecInRange(sec) && Brush.draw)  // custom range (e.g. 0 to 100%) determined by range_start/end command
				{
					if (Brush.Tri==nullptr)
					{
						RMF_Buffer.WriteSolid(Brush);
						n_obj++;
					}
					else
					{
						for (int bt=0; bt<Brush.t_tri; bt++)
						{
							brush &TriBrush = Brush.Tri[bt];
							if (TriBrush.draw)
							{
								RMF_Buffer.WriteSolid(TriBrush);
								n_obj++;
							}
						}
					}
					
					// Gap of current Brush
					if (cTable[g].gaps>0&&Brush.Gap!=nullptr)
					{
						#if DEBUG > 0
						if (dev) cout << "   Writing Gap Brush..." << endl;
						#endif
						
						brush &Gap = *Brush.Gap;
						RMF_Buffer.WriteSolid(Gap);
						n_obj++;
					}
				}
			}
		}
	}
	
	// write entity curve brushes
	#if DEBUG > 0
	if (dev) cout << " Writing Entity Curve Brushes..." << endl;
	#endif
	
	for (int g = 0; g < mGroup->t_arcs; g++)
	{
		group &Group = bGroup[g];
		
		if ( cTable[g].rmf>0 && cTable[g].c_enable>0 )
		if ( cTable[g].type!=2 && cTable[g].type!=3 )
		{
			for (int e = 0; e < EntityList.size(); e++) // entity loop
			{
				entity &Entity = EntityList[e];
				
				// current solid entity header
				if ( Entity.type==1 && !Entity.IsDetail )
				{
					RMF_Buffer.WriteEntityHeader(Entity);
					unsigned long pos_n_solids = RMF_Buffer.WriteCounterGetPos();
					int n_solids = 0;
					
					// Iterate Brushes
					for (int b = 0; b < bGroup[g].t_brushes; b++)
					{
						brush &Brush = bGroup[g].Brushes[b];
						int sec = Brush.SecID;
						
						if (Brush.entID==e && Group.IsSecInRange(sec) && Brush.draw)
						{
							if (Brush.Tri==nullptr)
							{
								RMF_Buffer.WriteSolid(Brush);
								n_solids++;
							}
							else
							{
								for (int bt=0; bt<Brush.t_tri; bt++)
								{
									brush &TriBrush = Brush.Tri[bt];
									if (TriBrush.draw)
									{
										RMF_Buffer.WriteSolid(TriBrush);
										n_solids++;
									}
								}
							}
							
							// Gap of current Brush
							if (cTable[g].gaps>0&&Brush.Gap!=nullptr)
							{
								brush &Gap = *Brush.Gap;
								RMF_Buffer.WriteSolid(Gap);
								n_solids++;
							}
						}
					}
					RMF_Buffer.WriteEntityFooter(Entity); // finish current solid entity
					RMF_Buffer.UpdateAtPosAndReturn(pos_n_solids, n_solids); // Update number of Entity Brushes
					n_obj++;
				}
			}
		}
		else if ( cTable[g].type==2||cTable[g].type==3 )
		{
			int t_orients 	= 1;
			int t_paths 	= PathList[g].t_paths;
			// count total orientations and assign orient ID to each Brush
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
				}
			}
			
			// write spline brushes
			for (int o = 0; o < t_orients; o++) // orientation loop
			for (int p = 0; p < t_paths; p++) // paths loop
			for (int e = 0; e < EntityList.size(); e++) // entity loop
			{
				entity &Entity = EntityList[e];
				
				// current solid entity header
				if (Entity.type==1)
				{
					int n_solids = 0;
					unsigned long pos_n_solids = 0;
					bool wrote_head = 0;
					bool wrote_foot = 0;
					// Iterate Brushes
					for (int b = 0; b < bGroup[g].t_brushes; b++)
					{
						brush &Brush = bGroup[g].Brushes[b];
						int sec = Brush.SecID;
						
						if (!Brush.exported && Brush.entID==e && ((cTable[g].psplit==1&&Brush.oID==o)||(cTable[g].psplit==0)) && Brush.pID==p && Group.IsSecInRange(sec) && Brush.draw)
						{
							if (!wrote_head)
							{
								RMF_Buffer.WriteEntityHeader(Entity);
								pos_n_solids = RMF_Buffer.WriteCounterGetPos();
								wrote_head = 1;
							}
							
							if (Brush.Tri==nullptr)
							{
								RMF_Buffer.WriteSolid(Brush);
								n_solids++;
							}
							else
							{
								for (int bt=0; bt<Brush.t_tri; bt++)
								{
									brush &TriBrush = Brush.Tri[bt];
									if (TriBrush.draw)
									{
										RMF_Buffer.WriteSolid(TriBrush);
										n_solids++;
									}
								}
							}
							
							Brush.exported = 1;
						}
						if (b==bGroup[g].t_brushes-1&&!wrote_foot&&wrote_head)
						{
							RMF_Buffer.WriteEntityFooter(Entity); // finish current solid entity
							RMF_Buffer.UpdateAtPosAndReturn(pos_n_solids, n_solids); // Update number of World Brushes
							wrote_foot = 1;
							n_solids = 0;
							n_obj++;
						}
					}
					// finish current solid entity
				}
			}
		}
	}

	// Bounding Boxes
	#if DEBUG > 0
	if (dev) cout << " Bounding Boxes..." << endl;
	#endif
	
	for (int g = 0; g < mGroup->t_arcs; g++)
	{
		/*if (cTable[g].bound==1 && cTable[g].rmf>0) // old: boundbox 1 is a solid entity
		{
			brush &Box = *bGroup[g].boundBox;
			entity Entity;
			Entity.key_classname = "func_detail";
			RMF_Buffer.WriteEntityHeader(Entity);
			RMF_Buffer.WriteByte((int)1);
			RMF_Buffer.WriteSolid(Box);
			RMF_Buffer.WriteEntityFooter(Entity);
			n_obj++;
		}*/
		if (cTable[g].bound==1 && cTable[g].rmf>0)
		{
			unsigned char col[3] = {252, 186, 3};
			RMF_Buffer.WriteSolid(*bGroup[g].boundBox);
			n_obj++;
		}
		else if (cTable[g].bound==2 && cTable[g].rmf>0)
		{
			unsigned char col[3] = {252, 186, 3};
			RMF_Buffer.WriteGroup(6, col );
			for(int b=0; b<6; b++)
				RMF_Buffer.WriteSolid(bGroup[g].boundBox[b]);
			n_obj++;
		}
	}
	
	// DEV Assets
	if(G_DEV)
	for (int g = 0; g < mGroup->t_arcs; g++)
	{
		group &Group = bGroup[g];
		
		// current solid entity header
		entity Entity;
		Entity.key_classname = "func_detail";
		RMF_Buffer.WriteEntityHeader(Entity);
		int n_solids = 0;
		unsigned long pos_n_solids = RMF_Buffer.WriteCounterGetPos();
		
		// Iterate Brushes
		for (int b = 0; b < bGroup[g].DevAssets.size(); b++)
		{
			brush &Brush = *bGroup[g].DevAssets[b];
			int sec = Brush.SecID;
			
			if ( Group.IsSecInRange(sec) ) // ????????????
			{
				RMF_Buffer.WriteSolid(Brush);
				n_solids++;
			}
		}
		RMF_Buffer.WriteEntityFooter(Entity); // finish current solid entity
		RMF_Buffer.UpdateAtPosAndReturn(pos_n_solids, n_solids); // Update number of World Brushes
		n_obj++;
	}
	
	// Detail Objects
	#if DEBUG > 0
	if (dev) cout << " Detail Objects..." << endl;
	#endif
	
	for (int g = 0; g < mGroup->t_arcs; g++)
	{
		group_set &Set = DetailSet[g];
		group &Group = bGroup[g];
		
		if ( cTable[g].d_enable && cTable[g].rmf>0 )
		for (int dg = 0; dg < Set.t_groups; dg++)
		{
			group &dGroup = Set.Groups[dg];
			
			if ( dGroup.d_enable==1 )
			{
				for (int e = 0; e < EntityList.size(); e++) // entity loop
				{
					entity &Entity = EntityList[e];
					
					if ( Entity.dID==dg && Entity.type == 1 && Entity.t_brushes>0 && Entity.IsDetail )
					{
						// export detail objects as whole objects
						if ( dGroup.d_separate==0 )
						{
							// current solid entity header
							RMF_Buffer.WriteEntityHeader(Entity);
							int n_solids = 0;
							unsigned long pos_n_solids = RMF_Buffer.WriteCounterGetPos();
							
							// Iterate Brushes
							for (int b = 0; b < dGroup.t_brushes; b++)
							{
								brush &Brush = dGroup.Brushes[b];
								int sec = Brush.SecID;
								if ( Brush.entID==e && Group.IsSecInRange(sec) && Brush.draw && !Brush.IsOrigin )
								{
									#if DEBUG > 0
									if(dev) cout << " EntityGroup " << dg << " Brush " << b << " entID " << Brush.entID << " n_solids " << n_solids << endl;
									#endif
									
									RMF_Buffer.WriteSolid(Brush);
									n_solids++;
								}
							}
							
							RMF_Buffer.WriteEntityFooter(Entity); // finish current solid entity
							RMF_Buffer.UpdateAtPosAndReturn(pos_n_solids, n_solids); // Update number of World Brushes
							n_obj++;
						}
						else // export detail objects as individual solid objects
						{
							for (int s=0; s<bGroup[g].sections; s++)
							{
								entity EntityN;
								EntityN.CopySimple(Entity);
								
								if ( dGroup.d_autoname>0 ) {
									if( Entity.key_target!="" ) {
										string k_target_name = "target";
										string k_target_value = Entity.key_target+"_"+to_string(s);
										ReplaceKeyInList(EntityN.Keys_Original, k_target_name, k_target_value);
									}
									if( Entity.key_targetname!="" ) {
										string k_targetname_name = "targetname";
										string k_targetname_value = Entity.key_targetname+"_"+to_string(s);
										ReplaceKeyInList(EntityN.Keys_Original, k_targetname_name, k_targetname_value);
									}
								}
								
								RMF_Buffer.WriteEntityHeader(Entity);
								int n_solids = 0;
								unsigned long pos_n_solids = RMF_Buffer.WriteCounterGetPos();
								
								for (int b = 0; b < dGroup.t_brushes; b++)
								{
									brush &Brush = dGroup.Brushes[b];
									int sec = Brush.SecID;
									
									if (sec==s && Brush.entID==e && Group.IsSecInRange(sec) && Brush.draw && !Brush.IsOrigin)
									{
										RMF_Buffer.WriteSolid(Brush);
										n_solids++;
									}
								}
								
								RMF_Buffer.WriteEntityFooter(Entity); // finish current solid entity
								RMF_Buffer.UpdateAtPosAndReturn(pos_n_solids, n_solids); // Update number of World Brushes
								n_obj++;
							}
						}
					}
				}
				// Export Point Entities
				for (int e = 0; e < dGroup.t_ents; e++)
				{
					entity &Entity = dGroup.Entities[e];
					int sec = Entity.SecID;
					if ( Group.IsSecInRange(sec) && Entity.draw && !Entity.IsOrigin )
					{
						entity EntityN;
						EntityN.CopySimple(Entity);
						
						//cout << " Exporting copy of Entity " << Entity << endl;
						
						// origin
						string k_origin_name = "origin";
						string k_origin_value = to_string(Entity.Origin.x) + " " + to_string(Entity.Origin.y) + " " + to_string(Entity.Origin.z);
						ReplaceKeyInList(EntityN.Keys_Original, k_origin_name, k_origin_value);
						
						// angles
						if( Entity.IsKeyType(KT_ANGLES) ) {
							string k_angles_name = "angles";
							string k_angles_value = to_string(Entity.Angles.Pitch) + " " + to_string(Entity.Angles.Yaw) + " " + to_string(Entity.Angles.Roll);
							ReplaceKeyInList(EntityN.Keys_Original, k_angles_name, k_angles_value);
						}
						
						// scale
						if( dGroup.d_scale_rand.x==1 && Entity.IsKeyType(KT_SCALE) ) {
							string k_scale_name = "scale";
							string k_scale_value = to_string(Entity.key_scale);
							ReplaceKeyInList(EntityN.Keys_Original, k_scale_name, k_scale_value);
						}
						
						// targetname & target
						if ( dGroup.d_autoname>0 ) {
							if( Entity.key_target!="" ) {
								string k_target_name = "target";
								string k_target_value = Entity.key_target+"_"+to_string(sec);
								ReplaceKeyInList(EntityN.Keys_Original, k_target_name, k_target_value);
							}
							if( Entity.key_targetname!="" ) {
								string k_targetname_name = "targetname";
								string k_targetname_value = Entity.key_targetname+"_"+to_string(sec);
								ReplaceKeyInList(EntityN.Keys_Original, k_targetname_name, k_targetname_value);
							}
						}
						RMF_Buffer.WriteEntityHeader(EntityN);
						RMF_Buffer.WriteByte((int)0); // no solids since this is a point entity
						
						RMF_Buffer.WriteEntityFooter(EntityN); // finish current solid entity
						n_obj++;
					}
				}
			}
		}
	}
	
	#if DEBUG > 0
	if (dev) cout << " Pos: " << RMF_Buffer.RMFBuffer.tellp() <<" Updating Counter at pos " << pos_n_obj << " with new object number: " << n_obj << "..." << endl;
	#endif
	
	RMF_Buffer.UpdateAtPosAndReturn(pos_n_obj, n_obj); // Update number of World Objects

	#if DEBUG > 0
	if (dev) cout << " Returning to pos " << RMF_Buffer.RMFBuffer.tellp() << "..." << endl;
	
	if (dev) cout << " Writing World Spawn Entity and Paths..." << endl;
	#endif

	RMF_Buffer.WriteWorldSpawn(EntityList[0]);
	RMF_Buffer.WritePaths((int)0);
	RMF_Buffer.CloseFile();
}



// Export Brush to .map-File
void file::ExportToMap()
{
	#if DEBUG > 0
	bool dev = 0;
	if (dev) cout << " Exporting Brush to .map-File..." << endl;
	#endif
	
	ofstream mapfile;
	
	// custom export file
	#if DEBUG > 0
	if (dev) cout << " Custom export file..." << endl;
	#endif
	
	string OutputFilePath;
	if (target!="ERR"&&target!="UNSET"&&target.length()>0) {
		if (target!=path_map)
		OutputFilePath = target;
		else { // if source filename equals target filename, add _curved to it, to prevent overwriting source file (this can only happen for MAP files)
			int i = target.find(".map");
			if (i!=-1)
			OutputFilePath = target.substr(0, i) + "_curved.map";
		}
	}
	else
		OutputFilePath = p_path+name+"_curved.map";
	
	// Console Info Message
	#if DEBUG > 0
	if (dev) cout << " Console Info Message..." << endl;
	#endif
	cout << OutputFilePath;
	
	// append
	string OutputFileContent;
	string OutputFileContentWorld;
	string OutputFileContentEntities;
	if (append&&CheckIfFileExists(OutputFilePath))
	{
		OutputFileContent 			= LoadTextFile(OutputFilePath);
		OutputFileContentWorld 		= GetMapWorld(OutputFileContent);
		OutputFileContentEntities 	= GetMapEnts(OutputFileContent);
	} else append = 0;
	
	#if DEBUG > 0
	if (dev) cout << " append " << cTable[0].append << " target len " << OutputFileContent.length() << " worldspawn len " << OutputFileContentWorld.length() << " entities len " << OutputFileContentEntities.length() << endl;
	#endif
	
	// Skipping Brushes with only Nullfaces
	for (int g = 0; g < mGroup->t_arcs; g++)
	{
		if (cTable[g].skipnull>0 && cTable[g].map>0)
		bGroup[g].CheckNULLBrushes();
	}
	
	// Export to file
	mapfile.open(OutputFilePath);
	
	// Write worldspawn header
	#if DEBUG > 0
	if (dev) cout << " Worldspawn header..." << endl;
	#endif

	if (append)
	mapfile << OutputFileContentWorld;
	else
	mapfile << EntityList[0].content.substr(0, EntityList[0].head_end) << endl;
	
	// Write all world brushes first (all arcs)
	#if DEBUG > 0
	if (dev) cout << " Writing all world brushes first (all arcs)..." << endl;
	#endif

	// Bounding Boxes
	for (int g = 0; g < mGroup->t_arcs; g++)
	{
		if (cTable[g].map>0)
		if (cTable[g].bound == 1)
		{
			brush &Box = *bGroup[g].boundBox;
			mapfile << "{\n";
			
			for (int f=0; f<6; f++)
			{
				face &Face = Box.Faces[f];
				WriteFaceMAP(mapfile, Face);
			}
			mapfile << "}\n";
		} else if (cTable[g].bound == 2)
		{
			for (int b=0; b<6; b++)
			{
				brush &Box = bGroup[g].boundBox[b];
				mapfile << "{\n";
				
				for (int f=0; f<6; f++)
				{
					face &Face = Box.Faces[f];
					WriteFaceMAP(mapfile, Face);
				}
				mapfile << "}\n";
			}
		}
	}

	
	for (int g = 0; g < mGroup->t_arcs; g++)
	{
		group &Group = bGroup[g];
		
		if(cTable[g].map>0 && cTable[g].c_enable>0)
		{
			// arc settings
			int secs = bGroup[g].sections;
			
			// Iterate Brushes
			for (int b = 0; b < bGroup[g].t_brushes; b++)
			{
				brush &Brush = bGroup[g].Brushes[b];
				int sec = Brush.SecID;
				
				if (Brush.entID==0 && Group.IsSecInRange(sec) && Brush.draw)  // custom range (e.g. 0 to 100%) determined by range_start/end command
				{
					#if DEBUG > 0
					if (dev) cout << "  Writing Brush " << b << "..." << endl;
					#endif
					
					if (Brush.Tri==nullptr)
					{
						mapfile << "{" << endl;
						
						// Iterate Faces
						for (int f = 0; f < Brush.t_faces; f++)
						{
							face &Face = Brush.Faces[f];
							
							if (Face.draw)
							WriteFaceMAP(mapfile, Face);
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
									WriteFaceMAP(mapfile, FaceTB);
								}
								
								mapfile << "}" << endl;
							}
						}
					}
					
					// Gap of current Brush
					if (cTable[g].gaps>0&&Brush.Gap!=nullptr)
					{
						#if DEBUG > 0
						if (dev) cout << "   Writing Gap Brush..." << endl;
						#endif
						
						brush &Gap = *Brush.Gap;
						mapfile << "{" << endl;
						for (int f = 0; f < Gap.t_faces; f++)
						{
							face &GFace = Gap.Faces[f];
							
							if (GFace.draw)
							WriteFaceMAP(mapfile, GFace);
						}
						mapfile << "}" << endl;
					}
				}
			}
		}
	}
	mapfile << "}" << endl; // finish world brushes
	
	if (append)
	mapfile << OutputFileContentEntities;

	// write solid entities
	#if DEBUG > 0
	if (dev) cout << " Writing solid entities..." << endl;
	#endif
	
	for (int g = 0; g < mGroup->t_arcs; g++)
	{
		group &Group = bGroup[g];
		
		if (cTable[g].type!=2&&cTable[g].type!=3 && cTable[g].map>0 && cTable[g].c_enable>0)
		for (int e = 0; e < EntityList.size(); e++) // entity loop
		{
			entity &Entity = EntityList[e];
			
			// current solid entity header
			if ( Entity.type==1 && !Entity.IsDetail )
			{
				mapfile << Entity.content.substr(0, Entity.head_end) << "\n";
				//cout << "Arc #"<<g<<" Entity ID " << e << " Entity Type " << Entity.type << endl;
				
				// Iterate Brushes
				for (int b = 0; b < Group.t_brushes; b++)
				{
					brush &Brush = Group.Brushes[b];
					int sec = Brush.SecID;
					
					if (Brush.entID==e && Group.IsSecInRange(sec) && Brush.draw)
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
								WriteFaceMAP(mapfile, Face);
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
										WriteFaceMAP(mapfile, FaceTB);
									}
									
									mapfile << "}" << endl;
								}
							}
						}
						
						// Gap of current Brush
						if (cTable[g].gaps>0&&Brush.Gap!=nullptr)
						{
							#if DEBUG > 0
							if (dev) cout << "   Writing Gap Brush..." << endl;
							#endif
							
							brush &Gap = *Brush.Gap;
							mapfile << "{" << endl;
							for (int f = 0; f < Gap.t_faces; f++)
							{
								face &GFace = Gap.Faces[f];
								
								if (GFace.draw)
								WriteFaceMAP(mapfile, GFace);
							}
							mapfile << "}" << endl;
						}
					}
				}
				mapfile << "}" << endl; // finish current solid entity
			}
		}
		else if ( (cTable[g].type==2||cTable[g].type==3) && cTable[g].map>0 && cTable[g].c_enable>0)
		{
			int t_orients 	= 1;
			int t_paths 	= PathList[g].t_paths;
			// count total orientations and assign orient ID to each Brush
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
				}
			}
			
			// write spline brushes
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
						
						#if DEBUG > 0
						if(dev)cout << " eID " << Brush.entID << " pID " << Brush.pID << " oID " << Brush.oID << " Draw " << Brush.draw << endl;
						#endif
						
						if (!Brush.exported && Brush.entID==e && ((cTable[g].psplit==1&&Brush.oID==o)||(cTable[g].psplit==0)) && Brush.pID==p && Group.IsSecInRange(sec) && Brush.draw)
						{
							if (!wrote_head)
							{
								mapfile << Entity.content.substr(0, Entity.head_end) << "\n";
								wrote_head = 1;
								//cout << "    Wrote Head!"<< endl;
							}
							
							//mapfile << "{" << endl;
							if (Brush.Tri==nullptr)
							{
								mapfile << "{" << endl;
								
								// Iterate Faces
								for (int f = 0; f < Brush.t_faces; f++)
								{
									face &Face = Brush.Faces[f];
									
									if (Face.draw)
									WriteFaceMAP(mapfile, Face);
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
											WriteFaceMAP(mapfile, FaceTB);
										}
										
										mapfile << "}" << endl;
									}
								}
							}

							Brush.exported = 1;
						}
						if (b==bGroup[g].t_brushes-1&&!wrote_foot&&wrote_head)
						{
							mapfile << "}" << endl;
							wrote_foot = 1;
						}
					}
				}
			}
		}
	}

	// Bounding Boxes
	#if DEBUG > 0
	if (dev) cout << " Bounding Boxes..." << endl;
	#endif
	for (int g = 0; g < mGroup->t_arcs; g++)
	{
		group &Group = bGroup[g];
		if (cTable[g].map>0)
		if (cTable[g].bound == 2)
		{
			vertex O;
			
			dimensions AllCombined;
			if (t_dgroups>0&&bGroup[g].t_brushes>0) AllCombined = DimensionCombine(bGroup[g].Dimensions, DetailSet[g].Dimensions);
			else if(bGroup[g].t_brushes==0&&t_dgroups>0) AllCombined = DetailSet[g].Dimensions;
			else AllCombined = bGroup[g].Dimensions;
			
			O.x = (AllCombined.xb+AllCombined.xs)/2.0;
			O.y = (AllCombined.yb+AllCombined.ys)/2.0;
			O.z = AllCombined.zs;
			string O_PS = to_string(O.x) + " " + to_string(O.y) + " " + to_string(O.z);
			O.x +=64;
			string O_LE = to_string(O.x+64) + " " + to_string(O.y) + " " + to_string(O.z);
			
			// PlaYER sTART
			mapfile << "{\n";
			mapfile << "\"classname\" \"info_player_start\"\n";
			mapfile << "\"angles\" \"0 0 0\"\n";
			mapfile << "\"origin\" \""<<O_PS<<"\"\n";
			mapfile << "}\n";
			
			// light_environment
			mapfile << "{\n";
			mapfile << "\"classname\" \"light_environment\"\n";
			mapfile << "\"angles\" \"0 135 0\"\n";
			mapfile << "\"_light\" \"255 255 128 150\"\n";
			mapfile << "\"_fade\" \"1.0\"\n";
			mapfile << "\"pitch\" \"-55\"\n";
			mapfile << "\"origin\" \""<<O_LE<<"\"\n";
			mapfile << "}\n";
		}
	}
	
	// DEV Assets
	if(G_DEV)
	for (int g = 0; g < mGroup->t_arcs; g++)
	{
		mapfile << "{\n\"classname\" \"func_detail\"\n";
		
		// Iterate Brushes
		for (int b = 0; b < bGroup[g].DevAssets.size(); b++)
		{
			brush &DevBrush = *bGroup[g].DevAssets[b];
			
			if( bGroup[g].IsSecInRange(DevBrush.SecID) )
			{
				mapfile << "{\n";
				// Iterate Faces
				for (int f = 0; f < DevBrush.t_faces; f++)
				{
					face &Face = DevBrush.Faces[f];
					WriteFaceMAP(mapfile, Face);
				}
				mapfile << "}\n";
			}
		}
		mapfile << "}\n";
	}
	
	// Detail Objects
	#if DEBUG > 0
	if (dev) cout << " Detail Objects..." << endl;
	#endif
	
	for (int g = 0; g < mGroup->t_arcs; g++)
	{
		group_set &Set = DetailSet[g];
		group &Group = bGroup[g];
		
		if ( cTable[g].d_enable && cTable[g].map>0 )
		for (int dg = 0; dg < Set.t_groups; dg++)
		{
			group &dGroup = Set.Groups[dg];
			
			if ( dGroup.d_enable==1 )
			{
				for (int e = 0; e < EntityList.size(); e++) // entity loop
				{
					entity &Entity = EntityList[e];
					
					if ( Entity.dID==dg && Entity.type == 1 && Entity.t_brushes>0 && Entity.IsDetail )
					{
						// export detail objects as whole objects
						if ( dGroup.d_separate==0 )
						{
							// current solid entity header
							mapfile << Entity.content.substr(0, Entity.head_end) << "\n";
							
							// Iterate Brushes
							for (int b = 0; b < dGroup.t_brushes; b++)
							{
								brush &Brush = dGroup.Brushes[b];
								int sec = Brush.SecID;
								if ( Brush.entID==e && Group.IsSecInRange(sec) && Brush.draw && !Brush.IsOrigin )
								{
									mapfile << "{" << endl;
									
									// Iterate Faces
									for (int f = 0; f < Brush.t_faces; f++)
									{
										face &Face = Brush.Faces[f];
										
										if (Face.draw)
										WriteFaceMAP(mapfile, Face);
									}
									
									mapfile << "}" << endl;
								}
							}
							mapfile << "}" << endl; // finish current solid entity
						}
						// export detail objects as individual solid objects
						else
						{
							for (int s=0; s<Group.sections; s++)
							{
								mapfile << Entity.content.substr(0, Entity.head_end) << "\n";
								
								#if DEBUG > 0
								if (dev) {
								cout << endl << " Writing Point Entities of dGroup..." << endl;
								cout << "   Global Autoname:" << cTable[g].d_autoname << endl;
								cout << "   Local Autoname:" << dGroup.d_autoname << endl;
								system("pause");}
								#endif
								
								if ( dGroup.d_autoname>0 )
								{
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
									
									if (sec==s && Brush.entID==e && Group.IsSecInRange(sec) && Brush.draw && !Brush.IsOrigin)
									{
										mapfile << "{" << endl;
										// Iterate Faces
										for (int f = 0; f < Brush.t_faces; f++) {
											face &Face = Brush.Faces[f];
											if (Face.draw)
											WriteFaceMAP(mapfile, Face);
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
					if ( Group.IsSecInRange(sec) && Entity.draw && !Entity.IsOrigin )
					{
						mapfile << Entity.content.substr(0,Entity.head_end-2);
						
						// origin
						mapfile << "\"origin\" \"" << Entity.Origin.x << " " << Entity.Origin.y << " " << Entity.Origin.z << "\"" << endl;
						
						// angles
						if( Entity.IsKeyType(KT_ANGLES) )
						mapfile << "\"angles\" \"" << Entity.Angles.Pitch << " " << Entity.Angles.Yaw << " " << Entity.Angles.Roll << "\"" << endl;
						
						// scale
						if( dGroup.d_scale_rand.x==1 && Entity.IsKeyType(KT_SCALE) ) {
							mapfile << "\"scale\" \"" << to_string(Entity.key_scale) << "\"" << endl;
						}
						
						// targetname & target
						if ( dGroup.d_autoname>0 ) {
							if( Entity.key_target!="" ) {
								mapfile << "\"target\" \"" << Entity.key_target+"_"+to_string(sec) << "\"" << endl;
							}
							if( Entity.key_targetname!="" ) {
								mapfile << "\"targetname\" \"" << Entity.key_targetname+"_"+to_string(sec) << "\"" << endl;
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
/*void file::ExportToMapO(string p)
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
}*/




void file::ExportToObj()
{
	for (int g = 0; g < mGroup->t_arcs; g++)
	{
		group &Group = bGroup[g];
		
		if (cTable[g].obj&&Group.valid) // wether or not this arc is being exported to an obj file
		{
			ofstream objfile;
			objfile.open(p_path+name+"_"+to_string(g+1)+".obj");
			
			for (int b = 0; b < Group.t_brushes; b++)
			{
				brush &Brush = Group.Brushes[b];
				int sec = Brush.SecID;
				
				if (Group.IsSecInRange(sec) && Brush.draw)  // custom range (e.g. 0 to 90 degree) determined by arc command in settings file
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
									objfile << "v " << Face.Vertices[v].x << " " << Face.Vertices[v].y << " " << Face.Vertices[v].z << " ";
									if(v==0) objfile << "0 1 0" << endl;
									else if(v==1) objfile << "1 1 0" << endl;
									else if(v==2) objfile << "1 0 0" << endl;
									else objfile << endl;
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
			
			// Detail Objects
			group_set &Set = DetailSet[g];
			for (int d=0; d<Set.t_groups; d++)
			{
				group &dGroup = Set.Groups[d];
				for (int b = 0; b < dGroup.t_brushes; b++)
				{
					brush &Brush = dGroup.Brushes[b];
					int sec = Brush.SecID;
					
					if ( Group.IsSecInRange(sec) && Brush.draw )
					{
						objfile << "g brush_export" << b << endl;
						
						for (int f = 0; f < Brush.t_faces; f++)
						{
							face &Face = Brush.Faces[f];
							int tverts = Face.vcount;
							
							if (Face.draw)
							{
								for(int v = 0; v < tverts; v++) {
									objfile << "v " << Face.Vertices[v].x << " " << Face.Vertices[v].y << " " << Face.Vertices[v].z << " ";
									if(v==0) objfile << "0 1 0" << endl;
									else if(v==1) objfile << "1 1 0" << endl;
									else if(v==2) objfile << "1 0 0" << endl;
									else objfile << endl;
								}
								
								objfile << "f";
								
								for(int i = 0; i < tverts; i++)
									objfile << " -" << i+1;
								
								objfile << endl << endl;
							}
						}
					}
				}
			}
			
			// DEV Assets
			if(G_DEV)
			for (int b=0; b < Group.DevAssets.size(); b++)
			{
				brush &Brush = *Group.DevAssets[b];
				int sec = Brush.SecID;
				
				if ( Group.IsSecInRange(sec) )
				{
					objfile << "g brush_export" << b << endl;
					
					for (int f = 0; f < Brush.t_faces; f++)
					{
						face &Face = Brush.Faces[f];
						int tverts = Face.vcount;
						
						for(int v = 0; v < tverts; v++)
						{
							objfile << "v " << Face.Vertices[v].x << " " << Face.Vertices[v].y << " " << Face.Vertices[v].z << " ";
							if(v==0) objfile << "0 1 0" << endl;
							else if(v==1) objfile << "1 1 0" << endl;
							else if(v==2) objfile << "1 0 0" << endl;
							else objfile << endl;
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

			// Gaps
			if (cTable[g].gaps>0)
			for (int b = 0; b < Group.t_brushes; b++)
			{
				if(Group.Brushes[b].Gap!=nullptr)
				{
					brush &Gap = *Group.Brushes[b].Gap;
					int sec = Gap.SecID;
	
					if ( Group.IsSecInRange(sec) )
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












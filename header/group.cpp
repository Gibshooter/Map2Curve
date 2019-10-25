#include "group.h"
#include "brush.h"
#include "settings.h"
#include "file.h"
#include "utils.h"

#include <conio.h> // getch
#include <iostream>
#include <fstream>
#include <iomanip> // precision

using namespace std;

extern ctable *cTable;
extern group *bGroup;
extern group *mGroup;
extern group *sGroup;
extern file *gFile;

/* ===== GROUP METHODS ===== */

void group::GetBrushFaceVertexSE()
{
	group &Group = *this;
	for (int b = 0; b<Group.t_brushes; b++)
	{
		brush &Brush = Group.Brushes[b];
		
		if (Brush.valid)
		Brush.GetFaceVertexSE();
	}
}

void group::GetBrushVertexListSE()
{
	group &Group = *this;
	for (int b = 0; b<Group.t_brushes; b++)
	{
		brush &Brush = Group.Brushes[b];
		
		if (Brush.valid)
		Brush.GetVertexListSE();
	}
}

void group::GetBrushVertexList()
{
	group &Group = *this;
	for (int b = 0; b<Group.t_brushes; b++)
	{
		brush &Brush = Group.Brushes[b];
		
		if (Brush.valid)
		Brush.GetVertexList();
	}
}

void group::GetRconBrushVertices()
{
	group &Group = *this;
	for (int b = 0; b<Group.t_brushes; b++)
	{
		brush &Brush = Group.Brushes[b];
		
		if (Brush.valid)
		Brush.GetRconVertices();
	}
}

void group::ConvertRconBrushVertices()
{
	group &Group = *this;
	for (int b = 0; b<Group.t_brushes; b++)
	{
		brush &Brush = Group.Brushes[b];
		
		if (Brush.valid)
		Brush.ConvertVerticesC2V();
	}
	Group.RCON = 1;
}

void group::GetBrushFaceCentroids()
{
	group &Group = *this;
	for (int b = 0; b<Group.t_brushes; b++)
	{
		brush &Brush = Group.Brushes[b];
		
		if (Brush.valid)
		Brush.GetFaceCentroids();
	}
}

void group::GetBrushFaceCentroidsC()
{
	group &Group = *this;
	for (int b = 0; b<Group.t_brushes; b++)
	{
		brush &Brush = Group.Brushes[b];
		
		if (Brush.valid)
		Brush.GetFaceCentroidsC();
	}
}


// get angle of all vertices relative to brush(profile) center
void group::GetBrushVertexAngles()
{
	bool dev = 0;
	if(dev) cout<< endl << " Getting Vertex Angles..." << endl;
	
	group &Group = *this;
	for (int b = 0, i = 0; b<Group.t_brushes; b++)
	{
		brush &Brush = Group.Brushes[b];
		
		if (Brush.valid)
		{
			float &sangle = Brush.vAngle_s;
			float &bangle  = Brush.vAngle_b;
			if(dev) cout << "  Brush Centroid " << Brush.centroid << endl;
			for (int f = 0; f<Brush.t_faces; f++)
			{
				face &Face = Brush.Faces[f];
				if(dev) cout<< "   Face " << f << endl;
				if (Face.fID==2)
				{
					for (int v = 0; v<Face.vcount; v++) // changed from Face.vcount to 3
					{
						vertex ev = Face.Vertices[v];
						if(dev) cout << "     Vertex " << v << ev << endl;
						ev.x = 0.0;
						vertex &cv = Brush.centroid;
						gvector vec = GetVector(ev, cv);
						
						gvector achecker(0.0,0.0,-1.0); //angle checker vector
						float &vangle = Face.Vertices[v].angle;
						float angle = GetVecAng(achecker, vec);
						
						if (ev.y>=cv.y)	vangle = angle;
						else 			vangle = 360.0-angle;
						
						if(dev) cout << "     vangle " << vangle << " angle " << angle << endl;
						
						// for comparison, save smallest and largest vertex angle as floating point number to each brush
						if (i==0) sangle = vangle;
						if (vangle < sangle)
						{
							if(dev) cout << "     angle ("<<vangle<<") is smaller than smallest ("<<sangle<<"). angle is new smallest!" << endl;
							sangle = vangle;
						}
						if (vangle > bangle)
						{
							if(dev) cout << "     angle ("<<vangle<<") is bigger  than biggest  ("<<bangle<<"). angle is new biggest!" << endl;
							bangle = vangle;
						}
	
						if(dev) cout << "     i: "<<i<<", Brush "<<b<<", Face " << f << ", Vertex " << v<< "\t " <<ev << "\t Angle: " << Face.Vertices[v].angle << endl;
						i++;
					}
				}
			}
			if(dev) cout << "  smallest angle of this brush: " << sangle << ", biggest: " << bangle << endl;
			i = 0;
		}
	}
	
	if(dev) cout<< endl << " All Vertex Angles:" << endl;
	if(dev)
	for (int b = 0, i = 0; b<Group.t_brushes; b++)
	{
		brush &Brush = Group.Brushes[b];
		if (Brush.valid)
		{
			for (int f = 0; f<Brush.t_faces; f++)
			{
				face &Face = Brush.Faces[f];
				if (Face.fID==2)
				{
					for (int v = 0; v<3; v++)
					{
						vertex &V = Face.Vertices[v];
						if(dev) cout << " Brush "<<b<<", Face " << f << ", Vertex " << v << V << "\t Angle: " << Face.Vertices[v].angle << endl;
					}
				}
			}
		}
	}
	if(dev) getch();
}

// reconstruct a source map to be able to calculate the original texture shifts/offsets
void group::GetGroupVertexList()
{
	bool dev = 0;
	group &Group = *this;
	if (dev) cout << " GetBrushSimpleCentroid..." << endl;
	Group.GetBrushSimpleCentroid(); // used to get vertex list
	if (dev) cout << " ClearBrushVertexList..." << endl;
	Group.ClearBrushVertexList();
	if (dev) cout << " GetBrushVertexAngles..." << endl;
	Group.GetBrushVertexAngles();
	if (dev) cout << " GetBrushFaceVertexSE..." << endl;
	Group.GetBrushFaceVertexSE();
	if (dev) cout << " GetBrushVertexListSE..." << endl;
	Group.GetBrushVertexListSE();
	if (dev) cout << " GetBrushVertexList..." << endl;
	Group.GetBrushVertexList();
}

// reconstruct a source map to be able to calculate the original texture shifts/offsets
void group::ReconstructMap()
{
	bool dev = 0;
	// Vertex List #1
	if (dev) cout << " Vertex List #1..." << endl;
	mGroup->GetGroupVertexList();
	
	// Reconstruct Brush Vertices
	if (dev) cout << " Reconstruct Brush Vertices..." << endl;
	mGroup->GetRconBrushVertices();

	// Face Centroids
	if (dev) cout << " Face Centroids..." << endl;
	mGroup->GetBrushFaceCentroidsC();
	
	// Get Missing Vertices from RCON Mesh
	if (dev) cout << " Get Missing Vertices from RCON Mesh..." << endl;
	mGroup->ConvertRconBrushVertices();

	// Face Centroids
	//mGroup->GetBrushFaceCentroids();
	
	// Vertex List #2 based on Reconstructed Vertices
	if (dev) cout << " Vertex List #2 based on Reconstructed Vertices..." << endl;
	mGroup->GetGroupVertexList();
}

// reconstruct working source objects that were copied from the previously reconstructed source map
void group::Reconstruct()
{
	bool dev = 0;
	group &Group = *this;
	if(dev) cout << " Reconstructing Group " << Group.gID << "..." << endl;

	if(dev) cout << "   Getting Vertex List... " << endl;
	Group.GetGroupVertexList();
	
	if(dev) cout << "   GetBrushFaceCentroidsC..." << endl;
	Group.GetBrushFaceCentroids(); // unnecessary???
}

// get GetBaseEdges, Baseshift and Original Texture Offset
void group::GetBrushShifts()
{
	// get BaseShifts and Original Offsets
	group &Group = *this;
	for (int b = 0; b<Group.t_brushes; b++)
	{
		brush &Brush = Group.Brushes[b];
		if (Brush.valid)
		{
			for (int f = 0; f<Brush.t_faces; f++)
			{
				face &Face = Brush.Faces[f];
				GetBaseEdges(Face);
				GetBaseShift(Face, 0, 1, 0);
				GetTexOffset(Face, 0);
			}
		}
	}
}

void group::ClearBrushVertexList()
{
	group &Group = *this;
	for (int b = 0; b<Group.t_brushes; b++)
	{
		brush &Brush = Group.Brushes[b];
		if (Brush.valid)
			Brush.ClearVertexList();
	}
}

void group::ExportGroupToMap(string p)
{
	group &Group = *this;

	ofstream mapfile;
	mapfile.open(p);

	// worldspawn header
	mapfile << "{\n\"classname\" \"worldspawn\"\n\"mapversion\" \"220\"\n";
	
	for (int b = 0; b < Group.t_brushes; b++)
	{
		brush &Brush = Group.Brushes[b];
		if (Brush.valid)
		{
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



// get GetBaseEdges, Baseshift and Original Texture Offset
void group::GetRconBrushShifts()
{
	// get BaseShifts and Original Offsets from reconstructed Source Map Faces
	// they need to be reconstructed, because Map-file faces are stored with 3 vertices at max, which makes figuring out texture shifts impossible
	group &Group = *this;
	for (int b = 0; b<Group.t_brushes; b++)
	{
		brush &Brush = Group.Brushes[b];
		if (Brush.valid)
		{
			for (int f = 0; f<Brush.t_faces; f++)
			{
				face &Face = Brush.Faces[f];
				GetBaseEdgesC(Face);
				GetBaseShift(Face, 0, 1, 1);
				GetTexOffset(Face, 0);
			}
		}
	}
}

// get "simple" centroid of each brush (brush means brush-profile)
void group::GetBrushSimpleCentroid()
{
	group &Group = *this;
	for (int b = 0; b<Group.t_brushes; b++)
	{
		brush &Brush = Group.Brushes[b];
		
		if (Brush.valid)
		Brush.GetSimpleCentroid();
	}
}

void group::CheckBrushDivisibility()
{
	//cout << " Checking Brush Divisibility... total Brushes " << t_brushes << endl;
	group &Group = *this;
	for (int b = 0; b<Group.t_brushes; b++)
	{
		brush &Brush = Group.Brushes[b];
		
		if (Brush.valid)
		Brush.CheckDivisibility();
	}
}

void group::CreateBrushGaps()
{
	group &Group = *this;
	int g = Group.gID;
	if (cTable[g].gaps>0&&cTable[g].type<=1)
	for (int b = 0; b<Group.t_brushes; b++)
	{
		brush &Brush = Group.Brushes[b];
		
		if (Brush.valid)
		Brush.CreateGap(g);
	}
}

void group::ArrangeGaps()
{
	bool dev = 0;
	if (dev) cout << " Arranging Gaps... " << endl;
	group &Group = *this;
	int g = Group.gID;
	//gvector Arranger;
	gvector GapEdge;
	gvector SecEdge;
	/*
		1. Gapss of current section stay as they are, GapEdge of this section is being created
		2. Brush of this Gap and ALL following Brushes and their Gaps are being moved according to the GapEdge vector
		3. GapEdge is being added to Arranger-Vector, which constantly evolves to reflect the travelled distance
	*/
	if (cTable[g].gaps>0&&cTable[g].type<=1)
	for (int sec = 0; sec<Group.sections; sec++)
	{
		if (dev) cout << "  H Source Face is " << Group.SecBaseFace[sec] << endl;
		face &HSrcFace = *Group.SecBaseFace[sec];
		//HSrcFace.Texture = "RED";
		SecEdge = HSrcFace.EdgeH; //GetVector(HSrcFace.Vertices[1],HSrcFace.Vertices[2]);
		if (dev) cout << "  Section " << sec << " HSrcFace Edge " << SecEdge << " Length " << GetVecLen(SecEdge) << endl;
		
		for (int b = 0; b<Group.t_brushes; b++)
		{
			brush &Brush = Group.Brushes[b];
			brush &Gap = *Group.Brushes[b].Gap;
			int bsec = Brush.SecID;
			
			// Arrange all Brushes and their Gaps in Order, start with second Brush
			if (Brush.valid&&bsec==sec)
			{
				GapEdge = GetVector(Gap.Faces[2].Vertices[1],Gap.Faces[2].Vertices[2]);
				if (dev) cout << "    Brush " << b << " GapEdge " << GapEdge << " Length " << GetVecLen(GapEdge) << endl;;
				Brush.Move(GapEdge.x,GapEdge.y,0,0);
				
				//Arranger = SecEdge;
				//if (dev) cout << " Arranger " << Arranger << " + GapEdge = ";
				//Arranger.AddVec(GapEdge);
				//if (dev) cout << Arranger << " Length " << GetVecLen(Arranger) << endl;
				if (Brush.Tri!=nullptr)
				{
					for (int bt = 0; bt<Brush.t_tri; bt++)
					{
						brush &BrushTri = Brush.Tri[bt];
						BrushTri.Move(GapEdge.x,GapEdge.y,0,0);
					}
				}
			}
		}
		
		// Move all following Brushes and Gaps, too, if not of this or prior sections
		for (int bn = 0; bn<Group.t_brushes; bn++)
		{
			brush &BrushN = Group.Brushes[bn];
			brush &GapN = *Group.Brushes[bn].Gap;
			int bnsec = BrushN.SecID;
			
			if (BrushN.valid&&bnsec>sec)
			{
				BrushN.Move(GapEdge.x,GapEdge.y,0,0);
				GapN.Move(GapEdge.x,GapEdge.y,0,0);
				
				if (BrushN.Tri!=nullptr)
				{
					for (int bt = 0; bt<BrushN.t_tri; bt++)
					{
						brush &BrushTri = BrushN.Tri[bt];
						BrushTri.Move(GapEdge.x,GapEdge.y,0,0);
					}
				}
			}
		}
	}
}

void group::RoundBrushVertices(bool Override)
{
	group &Group = *this;
	int g = Group.gID;
	if (Override||cTable[g].round>0||cTable[g].transit_round>0)
	{
		if (Override||cTable[g].round>0)
		for (int b = 0; b<Group.t_brushes; b++)
		{
			brush &Brush = Group.Brushes[b];
			Brush.SetRound(1);
			if (Brush.Gap!=nullptr)
			Brush.Gap->SetRound(1);
			
			if (Brush.Tri!=nullptr)
			for (int bt = 0; bt<Brush.t_tri; bt++)
			{
				brush &BrushTri = Brush.Tri[bt];
				BrushTri.SetRound(1);
			}
		}

		for (int b = 0; b<Group.t_brushes; b++)
		{
			brush &Brush = Group.Brushes[b];
			Brush.RoundVertices();
			
			if (Brush.Tri!=nullptr)
			for (int bt = 0; bt<Brush.t_tri; bt++)
			{
				brush &BrushTri = Brush.Tri[bt];
				BrushTri.RoundVertices();
			}
		}
	}
}

// determine body Face Lengths for Texture Shift calculation
void group::GetBrushBodyFaceLengths()
{
	group &Group = *this;
	int g = Group.gID;
	//cout << "determine Face Lengths..." << endl;
	// PI/GRID original and new face lengths
	for (int b = 0; b < Group.t_brushes; b++)
	{
		brush &Brush = Group.Brushes[b];
		if (Brush.valid)
		{
			int sec = Brush.SecID;
			int seg = Brush.SegID;
			int seg2 = Brush.SegID2;
			
			for (int f = 0; f < Brush.t_faces; f++)
			{
				face &Face = Brush.Faces[f];
				// identify current source brush for original Face Length calculation
				//cout << " calculating OFbrushID: (seg "<<seg<<"*cTable[g].res "<<cTable[g].res<<")+(cTable[g].res/4 "<<cTable[g].res/4<<")-1"<< endl;
				int OFbrushID = (seg2*cTable[g].res)+(cTable[g].res/4)-1;
				float OFaceLen = GetFaceLen(Group.Brushes[OFbrushID].Faces[f]);
				//cout << " OFaceLen: " << OFaceLen << " OFbrushID: " << OFbrushID << " brush: " << b << endl;
				
				if (Face.fID==2)
				{
					if (cTable[g].type==1)
						Face.LengthO = OFaceLen;
					else
						Face.LengthO = GetVecLen( GetVector(  Face.Vertices[0], Face.Vertices[1]  ));
					Face.LengthN = GetFaceLen(Face);
					//if (Brush.IsWedge) cout << "   Brush # "<<b<<" Face #" <<f<< " Tex: "<<Face.Texture << " Face Len " << Face.LengthN << endl;
					
					//if (b==0)
					//cout << "Face #" <<f<< " Tex: "<<Face.Texture << "\nVecO " << VecO << "\nPitchO: " << Face.PitchO << "\nVecN " << Cross2 <<"\nPitchN: " << Face.PitchN << "\nDifference O/N: " << Face.PitchN-Face.PitchO << endl << endl;
				}
			}
		}
	}
}

void group::GetBrushFacePlanarity()
{
	group &Group = *this;
	int g = Group.gID;
	for (int b = 0; b < Group.t_brushes; b++)
	{
		brush &Brush = Group.Brushes[b];
		
		if (Brush.valid&&Brush.draw)
		Brush.GetFacePlanarity();
	}
}

void group::Triangulate()
{
	group &Group = *this;
	int g = Group.gID;
	
	// first, mark brushes for triangulation, if tri is active for all brushes
	if (cTable[g].tri>0)
	{
		for (int b = 0; b < Group.t_brushes; b++)
		{
			brush &Brush = Group.Brushes[b];
			Brush.DoSplit = 1;
		}
	}
	if ( cTable[g].tri==0 && cTable[g].transit_tri>0 )
	{
		for (int b = 0; b < Group.t_brushes; b++)
		{
			brush &Brush = Group.Brushes[b];
			if (Brush.SecID==Group.range_start || Brush.SecID==Group.range_end-1)
			Brush.DoSplit = 1;
		}
	}
	if (  cTable[g].tri==0 && cTable[g].transit_tri==0 && cTable[g].transit_round>0 )
	{
		for (int b = 0; b < Group.t_brushes; b++)
		{
			brush &Brush = Group.Brushes[b];
			if ( !Brush.IsDivisible && (Brush.SecID==Group.range_start || Brush.SecID==Group.range_end-1) )
			Brush.DoSplit = 1;
		}
	}
	if ( cTable[g].tri==0 && cTable[g].round>0 )
	{
		for (int b = 0; b < Group.t_brushes; b++)
		{
			brush &Brush = Group.Brushes[b];
			if ( !Brush.IsDivisible || ( Brush.IsDivisible&&Brush.t_faces==5 ) )
			Brush.DoSplit = 1;
		}
	}

	for (int b = 0; b < Group.t_brushes; b++)
	{
		brush &Brush = Group.Brushes[b];
		
		if (Brush.valid&&Brush.draw&&Brush.DoSplit)
		Brush.Triangulate();
	}
}

void group::GetHeadVertices()
{
	group &Group = *this;
	int g = Group.gID;
	
	for (int b = 0; b < Group.t_brushes; b++)
	{
		brush &Brush = Group.Brushes[b];
		
		if (Brush.valid&&Brush.draw)
		Brush.MarkFaceVertices(Brush.Faces[1], 1, 0);
		
		//if (Brush.Tri!=nullptr)
	}
}

void group::GetTransitVertices()
{
	bool dev = 0;
	group &Group = *this;
	int g = Group.gID;
	
	if (dev) cout << " Getting Transit Vertices for Group "<<g<<"..." << endl;
	for (int b = 0; b < Group.t_brushes; b++)
	{
		brush &Brush = Group.Brushes[b];
		int sec = Brush.SecID;
		
		if (Brush.valid&&Brush.draw)
		{
			if (sec==Group.range_start) {
			Brush.MarkFaceVertices(Brush.Faces[0], 0, 0); if (dev) cout << "  Brush Section is range start " << Group.range_start << endl; }
			
			else if (sec==Group.range_end-1) {
			Brush.MarkFaceVertices(Brush.Faces[1], 0, 0); if (dev) cout << "  Brush Section is range end " << Group.range_end << endl; }
			
			if (Brush.Tri!=nullptr)
			for (int bt = 0; bt<Brush.t_tri; bt++)
			{
				brush &BrushTri = Brush.Tri[bt];
				
				if (sec==Group.range_start) {
				BrushTri.MarkFaceVertices(Brush.Faces[0], 0, 0); if (dev) cout << "  Brush Section is range start " << Group.range_start << endl; }
				
				else if (sec==Group.range_end-1) {
				BrushTri.MarkFaceVertices(Brush.Faces[1], 0, 0); if (dev) cout << "  Brush Section is range end " << Group.range_end << endl; }
			}
		}
	}
}

void group::CheckNULLBrushes()
{
	group &Group = *this;
	int g = Group.gID;
	
	for (int b = 0; b < Group.t_brushes; b++)
	{
		brush &Brush = Group.Brushes[b];
		
		if (Brush.valid&&Brush.draw)
		{
			if (Brush.Tri==nullptr)
			Brush.CheckNULLFaces();
			else
			for (int bt = 0; bt < Brush.t_tri; bt++)
			{
				brush &TriBrush = Brush.Tri[bt];
				TriBrush.CheckNULLFaces();
			}
		}
	}
}

void group::GetFaceGroups()
{
	bool dev = 0;
	group &Group = *this;
	int g = Group.gID;
	
	int last = 1;
	vector<string> uniqTexList;
	if (cTable[g].shift!=0)
	{
		if (cTable[g].shift==3||cTable[g].shift==5)
		for (int b = 0; b <Group.t_brushes; b++)
		{
			brush &Brush =Group.Brushes[b];
			if (Brush.valid)
			{
				int &sec = Brush.SecID;
				int &seg = Brush.SegID;
				
				if (Brush.SecID==0) // list unique textures of this brush, if shift mode is 3 (per brush texture)
				{
					for (int f = 2; f<Brush.t_faces; f++)
					{
						face &Face = Brush.Faces[f];
						if (Face.fID==2)
						{
							if (uniqTexList.size()==0) uniqTexList.push_back(Brush.Faces[f].Texture);
							else
							{
								for (int l = 0; l<uniqTexList.size(); l++)
								{
									if (Face.Texture==uniqTexList[l]) break; // if current faces texture is already in the list, stop the comparison
									
									if (l==uniqTexList.size()-1) {
										uniqTexList.push_back(Face.Texture); // if end of texture list is reached and comparison loop wasnt stopped yet, texture is unique and is being added to list
										if(dev) cout << " Test Loop #"<<l<<" added new texture to list -> " << Face.Texture << " list size now " << uniqTexList.size() << endl;
									}
								}
							}
						}
					}
					Brush.t_tgroups = uniqTexList.size();
					/*
					// assign basic group ID to face, based on texture only
					for (int f = 0; f<Brush.t_faces; f++)
					{
						face &Face = Brush.Faces[f];
						if (Face.fID==2)
						{
							for (int l = 0; l<uniqTexList.size(); l++)
							{
								if (Face.Texture==uniqTexList[l])
								{
									Face.group = l;
									if(dev) cout << "  uniqTexList: " << uniqTexList[l] << " Face.group: " << Face.group << " Tex " << Face.Texture << endl;
								}
							}
						}
					}*/
				}
			}
		}
		
		if(dev) cout << " unique textures found: " << uniqTexList.size() << endl; if(dev) getch();
		
		/*for (int b = 0; b<Group.t_brushes; b++)
		{
			brush &Brush = Group.Brushes[b];
			for (int f = 2; f<Brush.t_faces; f++)
			{
				face &Face = Brush.Faces[f];
				if(dev) cout << " Brush ("<<b<< ") Face ("<<f<<") Sec " << Brush.SecID << " eID " << Brush.entID << " Tex " << Face.Texture << endl;	
		}}
		 if(dev) getch();*/
		 
		// define group ID for Shift 5 (group texture)
		if(dev) cout << " solid entities found in source object: " << gFile->t_solids << endl;
		if (cTable[g].shift==5)
		for (int sec = 0, fgroup = 0; sec<Group.sections; sec++)
		{
			bool foundCheck = 0;
			//if(dev) cout << " Section " << sec << endl;
			for (int e = 0; e<gFile->t_solids+3; e++) // changed e = 1 to  e = 0 to enable world spawn brushes to have tex group shift. apparently works, yay!
			{
				//if(dev) cout << "  Entity " << e << endl;
				bool foundEnt = 0;
				for (int t = 0; t<uniqTexList.size(); t++)
				{
					bool foundTex = 0;
					//if(dev) cout << "   Tex " << t << " " << uniqTexList[t] << endl;
					for (int b = 0; b<Group.t_brushes; b++)
					{
						brush &Brush = Group.Brushes[b];
						//if(dev) cout << "    Brush " << b << endl;
						if (Brush.SecID==sec&&Brush.entID==e&&Brush.valid)
						{
							for (int f = 2; f<Brush.t_faces; f++)
							{
								face &Face = Brush.Faces[f];
								//if(dev) cout << " Section " << sec << " Entity " << e << " Tex " << t << " Brush " << b << " Face " << f << endl;
								if (Face.fID==2&&Face.Texture==uniqTexList[t])
								{
									Face.group = fgroup;
									foundEnt = 1;
									foundTex = 1;
									foundCheck = 1;
									if(dev) cout << "      Brush ("<<b<< ") Face ("<<f<<") Sec " << Brush.SecID << " eID " << Brush.entID << " Group is: " << Face.group << " Tex " << Face.Texture << endl;
								}
							}
						}
					}
					if (foundTex&&foundCheck) { fgroup++; foundCheck = 0; if(dev) cout << "   End of Tex Loop - Facegroup Increased to " << fgroup << endl; }
				}
				if (foundEnt&&foundCheck) { fgroup++; foundCheck = 0; if(dev) cout << "  End of Ent Loop - Facegroup Increased to " << fgroup << endl; }
			}
			if(dev) cout << " End of Sec Loop - Facegroup Increased to" << fgroup << endl;
			if (foundCheck) { fgroup++; foundCheck = 0;}
			if (sec==Group.sections-1) Group.hGroupsCount = fgroup+1;
		}
		
		// define group ID for every face
		if (cTable[g].shift<5)
		for (int b = 0; b < Group.t_brushes; b++)
		{
			brush &Brush = Group.Brushes[b];
			if (Brush.valid)
			{
				int &sec = Brush.SecID;
				int &seg = Brush.SegID;
				int biggest = 0;
				for (int f = 0; f<Brush.t_faces; f++)
				{
					face &Face = Brush.Faces[f];
					if (Face.fID==2)
					{
						if (cTable[g].shift==1||cTable[g].shift==4) // per section
						{
							Face.group = sec;
							if (b==Group.t_brushes-1&&f==Brush.t_faces-1) Group.hGroupsCount = Face.group+1;
						}
						
						else if (cTable[g].shift==2) // per brush
						{
							Face.group = b;
							if (b==Group.t_brushes-1&&f==Brush.t_faces-1) Group.hGroupsCount = Face.group+1;
						}
						
						else if (cTable[g].shift==3) // per brush texture
						{
							Face.group += last;
							if (Face.group>biggest) biggest = Face.group;
							if (f==Brush.t_faces-1) {last = biggest; last++; biggest=0;}
							
							if (b==Group.t_brushes-1&&f==Brush.t_faces-1) Group.hGroupsCount = last;
						}
						
						if(dev) cout << " Brush ("<<b<< ") Face ("<<f<<") Sec " << Brush.SecID << " eID " << Brush.entID << " Group is: " << Face.group << " Tex " << Face.Texture << endl;
					}
				}
			}
		}
		if(dev) cout  << "  TOTAL GROUP COUNT: " << Group.hGroupsCount << endl; if(dev) getch();
		
		/*if(dev)
		for (int b = 0; b < Group.t_brushes; b++)
		{
			brush &Brush = Group.Brushes[b];
			for (int f = 0; f<Brush.t_faces; f++)
			{
				face &Face = Brush.Faces[f];
				//Face.Texture = to_string(Brush.entID);
				if (Face.group!=0)
				Face.Texture = to_string(Face.group);
				else
				Face.Texture = "RED";
			}
		}*/
	}
}

// add custom height to this brush
void group::AddBrushHeights()
{
	group &Group = *this;
	int g = Group.gID;
	for (int b = 0; b<Group.t_brushes; b++)
	{
		brush &Brush = Group.Brushes[b];
		int sec = Brush.SecID;
		if (Brush.valid && cTable[g].height>0 && cTable[g].type!=2)
		{
			float step = cTable[g].height;
			float height = cTable[g].height * sec;
			int start = bGroup[g].range_start;
			
			for (int f = 0; f<Brush.t_faces; f++)
			{
				face &Face = Brush.Faces[f];
				if (cTable[g].ramp==2&&sec>=start) { height = bGroup[g].heightTable[sec-start-1]; }
				Face.AddHeight(height);
				//cout << " brush " << b << " face " << f << " sec " << sec << " height " << height << endl;
			}
		}
	}
}

void group::RotateVectors()
{
	group &Group = *this;
	int g = Group.gID;
	// rotation steps for PI circles
	float steps = -(360.0/cTable[g].res);
	
	// rotation steps for GRID circles
	float steps_grid_body[cTable[g].res];
	float steps_grid_base[cTable[g].res];
	float steps_grid_head[cTable[g].res];
	if (cTable[g].type==1)
	{
		circle circleA; circleA.Vertices = new vertex[cTable[g].res]; circleA.build_circleGrid(cTable[g].res, 16, 0);
		circle circleB; circleB.Vertices = new vertex[cTable[g].res]; circleA.build_circleGrid(cTable[g].res, 32, 0);
		
		for (int i = 0; i<cTable[g].res; i++)
		{
			float base_rot = GetVecAlign(GetVector(circleB.Vertices[i],circleA.Vertices[i]), 0);
			if (base_rot<90)
			steps_grid_base[i] = -(90 - base_rot);
			else
			steps_grid_base[i] = -(450 - base_rot);
			
			if (i==cTable[g].res-1)
			steps_grid_body[i] = GetVecAlign(GetVector(circleA.Vertices[i], circleA.Vertices[0]), 0);
			else
			steps_grid_body[i] = GetVecAlign(GetVector(circleA.Vertices[i], circleA.Vertices[i+1]), 0);
		}
		//for (int i = 0; i<cTable[g].res; i++)
		//cout << "steps_grid_body["<<i<<"]" << steps_grid_body[i] << endl;
		//for (int i = 0; i<cTable[g].res; i++)
		//cout << "steps_grid_base["<<i<<"]" << steps_grid_base[i] << endl;
	}
	
	// rotation steps for GRID PATH circles
	if (cTable[g].type==2)
	{
		// Get Rotation for Body Faces from one Body Face of this Brush that is suited (has 4 vertices)
		for (int b = 0, v=0; b<Group.t_brushes; b++) // path loop
		{
			brush &Brush = Group.Brushes[b];
			if (Brush.SegID==0)
			{
				// Check for QUAD Face to get vec align from first
				int sourceFaceID = 0;
				//vertex *P1ptr, *P2ptr;
				for (int f = 0; f<Brush.t_faces; f++)
				{
					if (Brush.Faces[f].fID==2&&Brush.Faces[f].draw==1)
					{
						sourceFaceID = f;
						break;
					}
				}
				//cout << " source face ID " << sourceFaceID << " tex " << Brush.Faces[sourceFaceID].Texture;
				//cout << "  Get Vec Rotation of Face "<< sourceFaceID <<" Brush " << b << P1 << P2 << endl;
				
				steps_grid_body[v] = GetVecAlign( Brush.Faces[sourceFaceID].EdgeH ,0); //GetVecAlign( GetVector( V1, V2 ),0 );
				//cout << " steps_grid_body["<<v<<"] " << steps_grid_body[v] << " from EdgeH " << Brush.Faces[sourceFaceID].EdgeH << endl;
				
				// BASE / HEAD Faces now too
				Brush.Faces[0].GetNormal();
				Brush.Faces[1].GetNormal();
				steps_grid_base[v] = GetVecAlign( Brush.Faces[0].Normal,0 )+180;
				steps_grid_head[v] = GetVecAlign( Brush.Faces[1].Normal,0 );
				//cout << " steps_grid_base["<<v<<"] " << steps_grid_base[v] << endl;
				v++;
			}
		}
	}
	
	// apply final rotation
	for (int b = 0; b < Group.t_brushes; b++)
	{
		brush &Brush = Group.Brushes[b];
		brush &Gap = *Group.Brushes[b].Gap;
		float deg = 0;
		int sec = Brush.SecID;
		int secs = Group.sections;
		
		for (int f = 0; f < Brush.t_faces; f++)
		{
			int tbf = Brush.t_faces-2;
			face &Face = Brush.Faces[f];
			
			//if (Face.fID<=1) cout << "Brush "  << b << " Face " << f << " sec " << sec ;
			//if (Face.fID<=1) Face.Texture = to_string(sec);
			
			if (cTable[g].type==0) // PI CIRCLES
			{
				if (Face.fID<=1)
				{
					if (Face.fID==0) { // BASE FACES
						//cout << "Face " << f <<" ID == 0" << ", texture is: " << Face.Texture << endl;
						if (sec>0) deg = steps*sec;
						Face.VecX.rotate(0,0,deg);
						Face.VecY.rotate(0,0,deg);
					} else { // HEAD FACES
						//cout << "Face " << f <<" ID == 1" << ", texture is: " << Face.Texture << endl;
						deg = steps*(sec+1);
						Face.VecX.rotate(0,0,deg);
						Face.VecY.rotate(0,0,deg);
						//Face.Centroid.rotate(0,0,deg);
					}
					
					// Gaps
					if (cTable[g].gaps>0) {
						face &GapFace = Gap.Faces[f];
						if (sec>0) deg = steps*sec;
						GapFace.VecX.rotate(0,0,deg);
						GapFace.VecY.rotate(0,0,deg);
					}
				} else {// BODY FACES
					//cout << "Face " << f <<" ID == 2" << endl;
					if (sec==0) { deg = steps/2.0; }
					else 		{ deg = (steps*sec) + (steps/2.0); }
					Face.VecH->rotate(0,0,deg);
					Face.VecV->rotate(0,0,deg);
					
					// Gaps
					if (cTable[g].gaps>0) {
						face &GapFace = Gap.Faces[f];
						deg = steps*sec;
						GapFace.VecX.rotate(0,0,deg);
						GapFace.VecY.rotate(0,0,deg);
					}
				}
			}
			else if (cTable[g].type==1) // GRID CIRCLES
			{
				if (Face.fID<=1)
				{
					if (Face.fID==0) { // BASE FACES
						deg = steps_grid_base[sec];
						//cout << "  base sec: " << sec << " deg " << deg << endl;
						//cout << "Face " << f <<" Rot(deg): "<< deg << " Base Face" << ", tex: " << Face.Texture << endl;
						Face.VecX.rotate(0,0,deg);
						Face.VecY.rotate(0,0,deg);
						
					} else { // HEAD FACES
						if (sec==secs-1) deg = steps_grid_base[0];
						else 			 deg = steps_grid_base[sec+1];
						//cout << "  head sec: " << sec << " deg " << deg << endl;
						//cout << "Face " << f <<" Rot(deg): "<< deg << " Head Face" << ", tex: " << Face.Texture << endl;
						Face.VecX.rotate(0,0,deg);
						Face.VecY.rotate(0,0,deg);
					}
				} else {// BODY FACES
					//cout << "  body sec: " << sec << " deg " << deg << endl;
					deg = steps_grid_body[sec];
					//cout << "Face " << f <<" Rot(deg): "<< deg << " Body Face" << ", tex: " << Face.Texture << endl;
					Face.VecH->rotate(0,0,deg);
					Face.VecV->rotate(0,0,deg);
				}
				
				// Gaps
				// dont need seperate rotations
				if (cTable[g].gaps>0) {
					face &GapFace = Gap.Faces[f];
					deg = steps_grid_base[sec];
					GapFace.VecX.rotate(0,0,deg);
					GapFace.VecY.rotate(0,0,deg);
				}
			}
			else if (cTable[g].type==2) // GRID PATH CIRCLES
			{
				if (Face.fID<=1)
				{
					if (Face.fID==0) { // BASE FACES
						deg = steps_grid_base[sec];
						Face.VecX.rotate(0,0,deg);
						Face.VecY.rotate(0,0,deg);
						
					} else { // HEAD FACES
						deg = steps_grid_head[sec];
						Face.VecX.rotate(0,0,deg);
						Face.VecY.rotate(0,0,deg);
						//cout << " Rotating Tex Vec of Face "<<f<<" Brush " << b << " by " << deg << " degree" << " sec " << sec << " secs-1 " << secs-1 << endl;
					}
				} else {// BODY FACES
					//cout << " Rotating Tex Vec of Face "<<f<<" Brush " << b << " by " << deg << " degree" << endl;
					deg = steps_grid_body[sec];
					Face.VecH->rotate(0,0,deg);
					Face.VecV->rotate(0,0,deg);
				}
			}
			
			Face.RefreshEdges();
			CheckFaceAlign(Face);
			// Fix Vertical Texture Vector
			if (Face.fID==2&&Face.HasWorldAlign)
			{
				gvector &VecV = *Face.VecV;
				vertex &V0 = Face.Vertices[0];
				vertex &V1 = Face.Vertices[1];
				vertex &V2 = Face.Vertices[2];
				gvector Edge1 = GetVector(V1,V2);
				if (Brush.IsWedge&&Face.IsWedgeDown) Edge1 = GetVector(V0,V2);
				gvector Cross = GetCross( Face.Normal, Edge1);
				//cout << " new vector " << Cross << " from V1 " << V1 << " V2 " << V2 << " Edge1 " << Edge1;
				Cross.flip();
				Cross.Normalize();
				float Dot = GetDot(Cross,VecV);
				if (Dot<0) { Cross.flip(); }//cout << " Dot " << Dot << " Flipping Vector " << endl; } else cout << " Dot " << Dot << endl;
				//cout << " VecV before Fix " << VecV;
				VecV.CopyCoords( Cross );
				//Face.Texture = "RED";
				//cout << " and after Fix " << VecV << endl;
			}
		}
	}
}

// get groups for horizontal shifts
void group::GetHorLengths()
{
	bool dev = 0;
	if(dev) cout << "Getting groups for horizontal shifts..." << endl;
	
	group &Group = *this;
	int g = Group.gID;
	
	if(dev) cout << " GetFaceGroups..." << endl;
	if (cTable[g].shift==0) Group.hGroupsCount = Group.sections;
	Group.GetFaceGroups();

	// get longest edge lengths for texture groups
	if(dev) cout << " get longest edge lengths for texture groups..." << endl;
	//if (cTable[g].shift!=0)
	{
		int hgc = Group.hGroupsCount;
		Group.hEdgeLen_temp.resize(hgc);
		Group.hEdgeLen_temp2.resize(hgc);
		Group.hSourceFace.resize(hgc);
		Group.hSourceFace2.resize(hgc);
		
		for (int b = 0; b < Group.t_brushes; b++)
		{
			brush &Brush = Group.Brushes[b];
			if (Brush.valid)
			{
				int &sec = Brush.SecID;
				int &seg = Brush.SegID;
				for (int f = 0; f<Brush.t_faces; f++)
				{
					face &Face = Brush.Faces[f];
					if (Face.fID==2)
					{
						vertex &V0 = Face.Vertices[0];
						vertex &V1 = Face.Vertices[1];
						vertex &V2 = Face.Vertices[2];
						float Edge1Len = 0;
						float Edge2Len = 0;
						if (Face.vcount>3) // Face is QUAD
						{
							vertex &V3 = Face.Vertices[3];
							Edge1Len = GetVecLen(GetVector(V1, V2));
							Edge2Len = GetVecLen(GetVector(V0, V3));
						}
						else // Brush is a TRIANGLE
						{
							if (Face.IsWedgeDown)
							{
								Edge1Len = GetVecLen(GetVector(V0, V2));
								Edge2Len = 0;
							}
							else
							{
								Edge1Len = GetVecLen(GetVector(V1, V2));
								Edge2Len = 0;
							}
							if(dev) cout << " Brush is Wedge. Edge1Len " << Edge1Len << " Edge2Len " << Edge2Len << " Face is WedgeDown " << Face.IsWedgeDown << endl;
						}
						Face.EdgeLenL = Edge1Len; if (Edge2Len>Edge1Len) Face.EdgeLenL = Edge2Len;
						Face.EdgeLenS = Edge1Len; if (Edge2Len<Edge1Len) Face.EdgeLenS = Edge2Len;
						
						// Long Edge
						if (Edge1Len>Group.hEdgeLen_temp[Face.group])
						{
							Group.hEdgeLen_temp[Face.group] = Edge1Len;
							Group.hSourceFace[Face.group] = &Face;
							//Group.SecBaseFace[sec] = &Face;
							if(dev) cout << "  Edge-Length of Brush "<<b<<" Face " << f << " Tex-Group " << Face.group << " -> " << Group.hEdgeLen_temp[Face.group] << " Face Tex " <<Face.Texture << endl; 
						}
						if (Edge2Len>Group.hEdgeLen_temp[Face.group])
						{
							Group.hEdgeLen_temp[Face.group] = Edge2Len;
							Group.hSourceFace[Face.group] = &Face;
							//Group.SecBaseFace[sec] = &Face;
							if(dev) cout << "  Edge-Length of Brush "<<b<<" Face " << f <<" Tex-Group " << Face.group << " -> " << Group.hEdgeLen_temp[Face.group] << " Face Tex " <<Face.Texture << endl; 
						}
						
						// new since 2019.06.06
						if (Group.SecBaseFace[sec]==nullptr) Group.SecBaseFace[sec] = &Face;
						face &SecBase = *Group.SecBaseFace[sec];
						float SecBaseLen = SecBase.EdgeLenL;
						if (Edge1Len>SecBaseLen||Edge2Len>SecBaseLen)
						Group.SecBaseFace[sec] = &Face;
						
						// Short Edge
						if (Edge1Len<Group.hEdgeLen_temp2[Face.group]||Group.hEdgeLen_temp2[Face.group]==0)
						{
							Group.hEdgeLen_temp2[Face.group] = Edge1Len;
							Group.hSourceFace2[Face.group] = &Face;
							//Group.SecBaseFace[sec] = &Face;
						}
						if (Edge2Len<Group.hEdgeLen_temp2[Face.group])
						{
							Group.hEdgeLen_temp2[Face.group] = Edge2Len;
							Group.hSourceFace2[Face.group] = &Face;
							//Group.SecBaseFace[sec] = &Face;
						}
						
						if(dev) cout << " Base Face for Section " << sec << " is now " << &Group.SecBaseFace[sec] << " Tex " <<  Group.SecBaseFace[sec]->Texture << " stored in Pointer " << Group.SecBaseFace[sec] << endl;
					}
				}
			}
		}
		if (dev) getch();
		
		// make Section Base Faces identify themselfs as what they are
		if(dev) cout << " make Section Base Faces identify themselfs as what they are..." << endl;
		for (int i = 0; i<cTable[g].res; i++)
		{
			//cout << " SecBaseFace " << i << " Adress " << *SecBaseFace[i] << " " << &SecBaseFace[i] << endl;
			Group.SecBaseFace[i]->IsSecBase = 1;
			if(dev) cout << " Section " << i << " Base Face fID " << Group.SecBaseFace[i]->fID << " Tex " << Group.SecBaseFace[i]->Texture << " EdgeLenL " << Group.SecBaseFace[i]->EdgeLenL << endl;
		}
		
		// check for nullpointer lengths
		if(dev) cout << " check for nullpointer lengths..." << endl;
		for (int i = 0; i<Group.hSourceFace.size(); i++)
		{
			if (Group.hSourceFace[i]==nullptr)  Group.hSourceFace[i] = Group.hSourceFace2[i];
			if (Group.hSourceFace2[i]==nullptr) Group.hSourceFace2[i] = Group.hSourceFace[i];
		}
		
		if(dev)
		for (int i = 0; i<Group.hEdgeLen_temp.size(); i++)
			cout << "  Tex-Group "<< i <<" Edge-Length Long " << Group.hEdgeLen_temp[i] << "("<<Group.hSourceFace[i]<<")" << " Short " << Group.hEdgeLen_temp2[i] << "("<<Group.hSourceFace2[i]<<")"<< endl;
		
		// copy source face information to all faces
		if(dev) cout << " copy source face information to all faces..." << endl;
		for (int b = 0; b < Group.t_brushes; b++)
		{
			brush &Brush = Group.Brushes[b];
			if (Brush.valid)
			{
				int &sec = Brush.SecID;
				int &seg = Brush.SegID;
				for (int f = 0; f<Brush.t_faces; f++)
				{
					face &Face = Brush.Faces[f];
					if (Face.fID==2)
					{
						Face.HSourceL = Group.hSourceFace[Face.group];
						Face.HSourceS = Group.hSourceFace2[Face.group];
						if(dev) cout << "  Brush "<<b<<" Face "<<f<<" facegroup "<<Face.group<<" sec "<<sec<<" Source Face H-Shift (Adress) Long " << Face.HSourceL <<" Short " << Face.HSourceS << endl;
					}
				}
			}
		}
		
		// horizontal face shifts for source faces
		if(dev) cout << " horizontal face shifts for source faces..." << endl;
		for (int b = 0; b < Group.t_brushes; b++)
		{
			brush &Brush = Group.Brushes[b];
			if (Brush.valid)
			{
				int &sec = Brush.SecID;
				brush *LBrush = nullptr;
				if (sec>0) LBrush = &Group.Brushes[b-1];
				for (int f = 0; f<Brush.t_faces; f++)
				{
					face &Face = Brush.Faces[f];
					if (Face.draw)
					{
						face *LSrcFaceL = nullptr, *LSrcFaceS = nullptr;
						if (sec>0)
						{
							LSrcFaceL = LBrush->Faces[f].HSourceL; if(dev) cout << " LSrcFaceL " << LSrcFaceL << endl;
							LSrcFaceS = LBrush->Faces[f].HSourceS; if(dev) cout << " LSrcFaceS " << LSrcFaceS << endl;
						}
						face &SrcFaceL = *Face.HSourceL;
						face &SrcFaceS = *Face.HSourceS;
						if (Face.fID==2)
						{
							int tg = Face.group;
							if (sec==0) {
								Face.HShiftL = SrcFaceL.EdgeLenL; if(dev) cout << " Face.HShiftL " << Face.HShiftL << endl;
								Face.HShiftS = SrcFaceS.EdgeLenS; if(dev) cout << " Face.HShiftS " << Face.HShiftS << endl;
							} else {
								Face.HShiftL = LSrcFaceL->HShiftL + SrcFaceL.EdgeLenL; if(dev) cout << " Face.HShiftL " << Face.HShiftL << endl;
								Face.HShiftS = LSrcFaceS->HShiftS + SrcFaceS.EdgeLenS; if(dev) cout << " Face.HShiftS " << Face.HShiftS << endl;
							}
						}
					}
				}
			}
		}
		
		// final horizontal face shifts
		if(dev) cout << " final horizontal face shifts..." << endl;
		for (int b = 0; b < Group.t_brushes; b++)
		{
			brush &Brush = Group.Brushes[b];
			if (Brush.valid)
			{
				int &sec = Brush.SecID;
				int &seg = Brush.SegID;
				brush *LBrush = nullptr;
				if (sec>0) LBrush = &Group.Brushes[b-1];
				for (int f = 0; f<Brush.t_faces; f++)
				{
					face &Face = Brush.Faces[f];
					face *LSrcFaceL = nullptr, *LSrcFaceS = nullptr;
					if (sec>0)
					{
						LSrcFaceL = LBrush->Faces[f].HSourceL;
						LSrcFaceS = LBrush->Faces[f].HSourceS;
					}
					face &SrcFaceL = *Face.HSourceL;
					face &SrcFaceS = *Face.HSourceS;
					if (Face.fID==2)
					{
						int tg = Face.group;
						if (sec==0) {
							Face.HShiftL = SrcFaceL.EdgeLenL;
							Face.HShiftS = SrcFaceS.EdgeLenS;
						} else {
							Face.HShiftL = LSrcFaceL->HShiftL + SrcFaceL.EdgeLenL;
							Face.HShiftS = LSrcFaceS->HShiftS + SrcFaceS.EdgeLenS;
						}
						if(dev) { cout << "  B "<<b<<" F "<<f<< " sec " <<sec<< " FGroup "<< Face.group <<" H-Shift L " << Face.HShiftL << " S " << Face.HShiftS << " SrcFaceL.EdgeLenL " << SrcFaceL.EdgeLenL;
						if (sec>0)cout << " LSrcFaceL->HShiftL " << LSrcFaceL->HShiftL << " SrcFaceS.EdgeLenS " << SrcFaceS.EdgeLenS << endl; else cout << endl; }
					}
				}
			}
		}
	}
}

// create height Table for smooth ramp generation
void group::CreateHeightTableSmooth()
{
	bool dev = 0;
	group &Group = *this;
	int g = Group.gID;
	if (cTable[g].ramp==2&&cTable[g].type!=2)
	{
		int range_end   = Group.range_end;
		int range_start = Group.range_start;
		int range		= range_end-range_start;
		if (range<=1) range = 2;
		if (dev) cout << " range_start " << range_start << " range_end " << range_end << " range " << range << endl;
		//create source circle
			// circle segment
			float height_max = cTable[g].height * (range);
			float height_half = (cTable[g].height * (range))/2;
			float chord  = 4*height_half;
			float rad    = (  (4*pow(height_half,2)) + pow(chord,2)  ) / (8 * height_half);
		if (dev) cout << " height_max\t" << height_max << endl;
		if (dev) cout << " height_half\t" << height_half << endl;
		if (dev) cout << " chord\t\t" << chord << endl;
		if (dev) cout << " rad\t\t" << rad << endl;
		
		float x_step = cTable[g].height*2;
		if (dev) cout << " x_step\t\t" << x_step << endl;
		
		Group.heightTable.resize(range);
		for (int i = 0; i<Group.heightTable.size(); i++)
		Group.heightTable[i] = height_max;
		
		// Fill first Half of the Height Table
		for (int i = 0; i<ceil(range/2.0); i++)
		{
			float x = 0, y = 0, y_fixed = 0;
			
			x = (height_max/(range/2.0))*(i+1);
			y = GetIsectCircleLine(rad, x);
			y_fixed = rad - y;
			//Group.heightTable.push_back(y_fixed);
			Group.heightTable[i] = y_fixed;
			
			if (dev) cout << "  i "<<i<< " x " << x << " isect " << y << " y_fixed " << y_fixed << endl;
		}
		
		if (dev)
		for (int i = 0; i<Group.heightTable.size(); i++)
			cout << " height table #" << i << " y " << round(Group.heightTable[i]) << endl;
		
		// Fill second Half by subtracting the first Half from the max height
		for (int i = 0, j = Group.heightTable.size()-2; i<ceil(range/2); i++)
		{
			float y = height_max - Group.heightTable[i];
			if (dev) cout << " Height Table #" << i << " j " << j << " of range " << range/2 << " height " << y << " = height_max " << height_max << " - heightTable[j] " << heightTable[j] << endl;
			Group.heightTable[j] = y;
			j--;
			//Group.heightTable.push_back(y);
		}
		
		for (int i = 0; i< Group.heightTable.size(); i++)
			Group.heightTable[i] = round(Group.heightTable[i]);
		
		if (dev)
		for (int i = 0; i<Group.heightTable.size(); i++)
		{
			cout << " height table #" << i << " y " << round(Group.heightTable[i]) << " difference ";
			if (i>0) cout << round(Group.heightTable[i] - Group.heightTable[i-1]) << endl;
			else cout << round(Group.heightTable[i] - 0) << endl;
		}
		if (dev) getch();
	}
	
	// create height Table path (type 2)
	/*if (cTable[g].type==2)
	{
		for (int i = 0; i<PathList[g].heightTable.size(); i++) {
			Group.heightTable.push_back(PathList[g].heightTable[i]);
			//cout << " new heightTable #" << i << " is " <<Group.heightTable[i] << endl;
		}
	}*/
	//getch();
}


// Generate Curve Object from loaded Settings, Paths and Map Source Object
void group::Build()
{
	bool dev = 0;
	
	group &Group = *this;
	int g = Group.gID;
	group &SrcGroup = sGroup[g];
	for (int b=0; b<Group.t_brushes; b++)
	{
		brush &Brush = Group.Brushes[b];
		int seg = Brush.SegID;
		int sec = Brush.SecID;
		brush &SrcBrush = SrcGroup.Brushes[seg];
		if (SrcBrush.valid)
		{
			if (dev)cout << "  Entering Brush #" << b+1 << ", Total Faces: " << Brush.t_faces << ", section #" << sec+1 << ", segment #" << seg+1 << endl;
			//Brush.DoSplit = SrcBrush.cset->c[0].Vertices[sec].DoSplit;
			
			// circle vertex interator
			int cIDa = sec; if (cTable[g].type==2) cIDa=sec*2;
			int cIDb = cIDa+1;
			Brush.step = SrcBrush.cset->c[0].Vertices[cIDa].step;
			Brush.pID = SrcBrush.cset->c[0].Vertices[cIDa].pID;
			Brush.Align = SrcBrush.cset->c[0].Vertices[cIDa].Align;
			if (!SrcBrush.valid) Brush.valid=0;
			
			for (int f = 0, c=0; f<Brush.t_faces; f++)
			{
				if (dev)cout << "    Entering Face #" << f+1 << "..." << endl;
				face &Face  = Brush.Faces[f];
				if (f==0||f==1) // if base/head Face
				{
					if (dev)cout << "        SrcBrush BaseID " << SrcBrush.BaseID << " SrcBrush HeadID " << SrcBrush.HeadID << endl;
					face &BaseSrc = SrcBrush.Faces[SrcBrush.BaseID];
					face &HeadSrc = SrcBrush.Faces[SrcBrush.HeadID];
					
					if (dev)cout << "        Copying Base and Head Faces...";
					if 		(f==0) Face.CopyFace(BaseSrc,0);
					else if (f==1) Face.CopyFace(HeadSrc,0);
					if (dev)cout << "Done!" << endl;
					
					for (int v = 0; v<Face.vcount; v++)
					{
						if (f==0) {// Base Face
							if (dev)cout << "        Generating Vertex #" << v+1 << " of Base Face, mG->B["<<seg<<"].cset->c["<<v<<"].V["<<cIDa<<"] :" << SrcGroup.Brushes[seg].cset->c[v].Vertices[cIDa] << endl;
							Face.Vertices[v] = SrcGroup.Brushes[seg].cset->c[v].Vertices[cIDa];
						}
						else { // Head Face
							//int secb;
							if (dev)cout << "        Generating Vertex #" << v+1 << " of Head Face, mG->B["<<seg<<"].cset->c["<<v<<"].V["<<cIDb<<"] :" << SrcGroup.Brushes[seg].cset->c[v].Vertices[cIDb] << endl;
							if (cIDa==Group.sections-1&&cTable[g].type!=2) cIDb=0; else cIDb=cIDa+1;
							Face.Vertices[v] = SrcGroup.Brushes[seg].cset->c[v].Vertices[cIDb];
						}
					}
					if (f==0) Face.RevOrder(0);
				}
				else // if Body Face
				{
					if (dev)cout << "        Body Face " << f << " Brush " << b << endl;
					face &BodySrc = SrcBrush.Faces[SrcBrush.cset->c[c].SrcFace];
					Face.CopyFace(BodySrc,0);
					if (dev)cout << "        Tex Offsets PRE X " << Face.OffsetX << " Y " << Face.OffsetY << " BaseVertexX " << Face.BaseX << " BaseVertexY " << Face.BaseY << " VecX "<<Face.VecX<<" VecY "<<Face.VecY<<" Centroid " << Face.Centroid << endl;
					if (dev)cout << "          copying Bodysource... Facealign now: " << Face.FaceAlign  << endl;
					int cb; //int secb = sec+1;
					if (c==SrcGroup.Brushes[seg].cset->tcircs-1) cb=0; else cb=c+1;
					
					vertex &V0 	= SrcGroup.Brushes[seg].cset->c[c].Vertices[cIDa];
					vertex &V1 	= SrcGroup.Brushes[seg].cset->c[cb].Vertices[cIDa];
					vertex &V2 	= SrcGroup.Brushes[seg].cset->c[cb].Vertices[cIDb];
					vertex &V3 	= SrcGroup.Brushes[seg].cset->c[c].Vertices[cIDb];
					bool W01 = 0; if (CompareVerticesR(V0,V1)) W01 = 1;
					bool W03 = 0; if (CompareVerticesR(V0,V3)) W03 = 1;
					bool W12 = 0; if (CompareVerticesR(V1,V2)) W12 = 1;
					
					/*cout << " Brush " <<b<<" Face " << f <<" V0 " << V0 <<" V1 " << V1<<" V2 " << V2 <<" V3 " << V3;
					if (W01) cout << " 01 Match!"  << " /t";
					if (W03) cout << " 03 Match!"  << " /t";
					if (W12) cout << " 12 Match!"  << " /t";*/
					//cout << " Brush " <<b<<" Face "<<f<<" V0 [" << V0.x << "|" << V0.y << "] V1 [" << V1.x << "|" << V1.y << "] V2 [" << V2.x << "|" << V2.y << "] V3 [" << V3.x << "|" << V3.y << "] \t";
					
					if (V0.x==V1.x&&V0.y==V1.y&&V0.x==V2.x&&V0.y==V2.y) { //if (V0.x==0&&V0.y==0&&V1.x==0&&V1.y==0) {
					if (dev)cout << "          INVALID" << endl;
					Face.vcount = 0;
					Face.draw = 0;
					} else if (W01) { //V0.x==V1.x&&V0.y==V1.y&&V0.z==V1.z) {
					if (dev)cout << "          WEDGE V0/V1" << endl;
					Face.Vertices[0] = V0;
					Face.Vertices[1] = V2;
					Face.Vertices[2] = V3;
					Face.SetEdges(1,2,1,0,1,0);
					Face.vcount = 3;
					Brush.IsWedge = 1;
					} else if (W03) { //V0.x==V3.x&&V0.y==V3.y&&V0.z==V3.z) {
					if (dev)cout << "          WEDGE V0/V3" << endl;
					Face.Vertices[0] = V0;
					Face.Vertices[1] = V1;
					Face.Vertices[2] = V2;
					Face.SetEdges(1,2,1,0,1,0);
					Face.vcount = 3;
					Brush.IsWedge = 1;
					} else if (W12) { //V1.x==V2.x&&V1.y==V2.y&&V1.z==V2.z) {
					if (dev)cout << "          WEDGE V1/V2" << endl;
					Face.Vertices[0] = V0;
					Face.Vertices[1] = V1;
					Face.Vertices[2] = V3;
					Face.SetEdges(0,2,1,0,1,0);
					Face.IsWedgeDown = 1;
					Face.vcount = 3;
					Brush.IsWedge = 1;
					} else {
					if (dev)cout << "          QUAD" << endl;
					Face.Vertices[0] = V0;
					Face.Vertices[1] = V1;
					Face.Vertices[2] = V2;
					Face.Vertices[3] = V3;
					Face.SetEdges(1,2,1,0,1,3);
					}
					if (V0.DevTex!="") { Face.Texture=V0.DevTex; }
					//cout << "      Generating Vertex #1 of Body Face, mG->B["<<seg<<"].cset->c["<<c<<"].V["<<sec<<"] :" << SrcGroup.Brushes[seg].cset->c[c].Vertices[sec] << endl;
					//cout << "      Generating Vertex #2 of Body Face, mG->B["<<seg<<"].cset->c["<<cb<<"].V["<<sec<<"] :" << SrcGroup.Brushes[seg].cset->c[cb].Vertices[sec] << endl;
					//cout << "      Generating Vertex #3 of Body Face, mG->B["<<seg<<"].cset->c["<<cb<<"].V["<<secb<<"] :" << SrcGroup.Brushes[seg].cset->c[cb].Vertices[secb] << endl;
					//cout << "      Generating Vertex #4 of Body Face, mG->B["<<seg<<"].cset->c["<<c<<"].V["<<secb<<"] :" << SrcGroup.Brushes[seg].cset->c[c].Vertices[secb] << endl;
					Face.GetNormal();
					if (dev) cout << "        Face Normal " << Face.Normal << endl;
					c++;
				}
			}
		}
	}
}

//determine Face Orientation and check for invalid Brushes
void group::CheckBrushValidity()
{
	group &Group = *this;
	for (int b = 0; b<Group.t_brushes; b++)
	{
		brush &Brush = Group.Brushes[b];
		
		bool valid = Brush.CheckValidity();
		if (!valid) Group.invalids++;
	}
	
	if (Group.t_brushes-Group.invalids<=0)
	Group.valid = 0;
}

void group::GetBrushFaceOrients()
{
	group &Group = *this;
	for (int b = 0; b<Group.t_brushes; b++)
	{
		brush &Brush = Group.Brushes[b];
		
		if (Brush.valid)
		Brush.GetFaceOrients();
	}
}

void group::GetBrushTVecAligns()
{
	group &Group = *this;
	for (int b = 0; b<Group.t_brushes; b++)
	{
		brush &Brush = Group.Brushes[b];
		
		if (Brush.valid)
		Brush.GetTVecAligns();
	}
}

// Limited Copy Function for entire Groups
void group::Copy(group &Source)
{
	bool dev = 0;
	if(dev) cout << " copying group ..." << endl;
	t_arcs		= Source.t_arcs;
	t_brushes 	= Source.t_brushes-Source.invalids;
	t_ents		= Source.t_ents;
	sections 	= Source.sections;
	segments	= Source.segments;
	SizeY 		= Source.SizeY;
	SizeZ 		= Source.SizeZ;
	biggestY 	= Source.biggestY;
	smallestY 	= Source.smallestY;
	biggestZ 	= Source.biggestZ;
	smallestZ 	= Source.smallestZ;
	Dimensions  = Source.Dimensions;
	Origin		= Source.Origin;
	if (t_brushes>0)
	Brushes		= new brush[t_brushes];
	if (t_ents>0)
	Entities	= new entity[t_ents];
	valid		= Source.valid;
	RCON		= Source.RCON;
	if(dev) cout << " created "<< t_brushes << " new brushes!" << endl;

	d_pos		= Source.d_pos;
	d_autopitch	= Source.d_autopitch;
	d_autoyaw	= Source.d_autoyaw;
	d_enable	= Source.d_enable;
	d_separate	= Source.d_separate;
	d_autoname	= Source.d_autoname;
	d_draw		= Source.d_draw;
	d_skip		= Source.d_skip;
	d_draw_rand	= Source.d_draw_rand;
	d_pos_rand= Source.d_pos_rand;
	d_rotz_rand = Source.d_rotz_rand;
	d_movey_rand= Source.d_movey_rand;
	
	for (int b=0, s=0; b<Source.t_brushes; b++)
	{
		brush &SrcBrush = Source.Brushes[b];
		brush &Brush = Brushes[s];
		if (SrcBrush.valid)
		{
			if(dev) cout << " Creating Brush #" << s << " by copying source Brush #" << b << endl;
			Brush.Copy(SrcBrush);
			s++;
		}
	}
	if(dev) cout << " Finished copying!" << endl;
}

void group::CopyProps(group &Source)
{
	d_pos		= Source.d_pos;
	d_autopitch	= Source.d_autopitch;
	d_autoyaw	= Source.d_autoyaw;
	d_enable	= Source.d_enable;
	d_separate	= Source.d_separate;
	d_autoname	= Source.d_autoname;
	d_draw		= Source.d_draw;
	d_skip		= Source.d_skip;
	d_draw_rand	= Source.d_draw_rand;
	d_pos_rand= Source.d_pos_rand;
	d_rotz_rand = Source.d_rotz_rand;
	d_movey_rand= Source.d_movey_rand;
}

void group::GetOrigin()
{
	dimensions &D = Dimensions;
	Origin.x = D.xb-((D.xb-D.xs)/2);
	Origin.y = D.yb-((D.yb-D.ys)/2);
	Origin.z = D.zb-((D.zb-D.zs)/2);
}

void group_set::GetDimensions(bool Overwrite)
{
	bool setBox = 0;
	dimensions &DS = Dimensions;
	for(int d=0; d<t_groups; d++)
	{
		group &dGroup = Groups[d];
		
		dGroup.GetDimensions(Overwrite);
		dimensions &D = dGroup.Dimensions;
		
		if (!setBox) { DS.set(D.xs,D.ys,D.zs); setBox = 1; } // set initial dimensions to first group vertex per default
		else
		{
			if (D.xs < DS.xs) DS.xs = D.xs;
			if (D.xb > DS.xb) DS.xb = D.xb;
			if (D.ys < DS.ys) DS.ys = D.ys;
			if (D.yb > DS.yb) DS.yb = D.yb;
			if (D.zs < DS.zs) DS.zs = D.zs;
			if (D.zb > DS.zb) DS.zb = D.zb;
		}
	}
}

void group::GetDimensions(bool Overwrite)
{
	bool dev = 0;
	dimensions &D = Dimensions;
	bool setBox = 0;
	for (int b = 0; b<t_brushes; b++)
	{
		brush &Brush = Brushes[b];
		int sec = Brush.SecID;
		if ((!IsSrcMap && Brush.draw && Brush.valid && sec<range_end && sec>=range_start ) || IsSrcMap || Overwrite )
		{
			for (int f = 0; f<Brush.t_faces; f++)
			{
				face &Face = Brush.Faces[f];
				if (Face.draw)
				{
					for (int v = 0; v<Face.vcount; v++)
					{
						vertex &V = Face.Vertices[v];
						
						if (!setBox) { D.set(V.x,V.y,V.z); setBox = 1; } // set initial dimensions to first group vertex per default
						else
						{
							if (V.x < D.xs) D.xs = V.x;
							if (V.x > D.xb) D.xb = V.x;
							if (V.y < D.ys) D.ys = V.y;
							if (V.y > D.yb) D.yb = V.y;
							if (V.z < D.zs) D.zs = V.z;
							if (V.z > D.zb) D.zb = V.z;
						}
						//cout << " dimensions taken from v" << v << V << " : " << D.xs << ", " << D.xb << ", " << D.ys << ", " << D.yb << ", " << D.zs << ", " << D.zb << endl;
					}
				}
			}
		}
	}
	for (int e = 0; e<t_ents; e++)
	{
		entity &Entity = Entities[e];
		int sec = Entity.SecID;
		if ((!IsSrcMap && sec<range_end && sec>=range_start ) || IsSrcMap || Overwrite )
		{
			vertex &V = Entity.Origin;
			
			if (!setBox) { D.set(V.x,V.y,V.z); setBox = 1; }
			else
			{
				if (V.x < D.xs) D.xs = V.x;
				if (V.x > D.xb) D.xb = V.x;
				if (V.y < D.ys) D.ys = V.y;
				if (V.y > D.yb) D.yb = V.y;
				if (V.z < D.zs) D.zs = V.z;
				if (V.z > D.zb) D.zb = V.z;
			}
		}
	}
	Origin.x = D.xb-((D.xb-D.xs)/2);
	Origin.y = D.yb-((D.yb-D.ys)/2);
	Origin.z = D.zb-((D.zb-D.zs)/2);
	SizeY = D.yb-D.ys;
	SizeZ = D.zb-D.zs;
	if (IsSrcMap) IsSrcMap = 0;
	if(dev) cout << " Origin of this object " << Origin << endl;
}
	

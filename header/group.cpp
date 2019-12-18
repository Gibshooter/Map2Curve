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

struct brush;
struct entity;

extern ctable *cTable;
extern group *bGroup;
extern group *mGroup;
extern group *sGroup;
extern file *gFile;
extern string def_nulltex;

/* ===== GROUP METHODS ===== */

ostream &operator<<(ostream &ostr, group &g)
{
	ostr << endl << " Group ID " << g.gID << " secs " << g.sections << " segs " << g.segments << " brushes " << g.t_brushes << " entities " << g.t_ents << endl;
	ostr << "   Origin " << g.Origin << " sizeY " << g.SizeY << endl;
	ostr << "   Brushes:" << endl;
	for (int b = 0; b<g.t_brushes; b++) // Brushes
	{
		brush &Brush = g.Brushes[b];
		ostr << "     Brush " << b << " sec " << Brush.SecID << " seg " << Brush.SegID << " dID " << Brush.dID << " entID " << Brush.entID << endl;
	}
	ostr << "   Entities:" << endl;
	for (int e = 0; e<g.t_ents; e++) // Entities
	{
		entity &Entity = g.Entities[e];
		ostr << "     Entity " << e << " Origin " << Entity.Origin << " Euler " << Entity.Angles << " sec " << Entity.SecID << " dID " << Entity.dID << " entID " << Entity.eID << endl;
	}
	ostr << endl;
}

void group::Move(float x, float y, float z, bool LockBrushShifts)
{
	for (int b = 0; b<t_brushes; b++) // Brushes
	{
		brush &Brush = Brushes[b];
		Brush.Move(x,y,z,LockBrushShifts);
	}
	for (int e = 0; e<t_ents; e++) // Entities
	{
		entity &Entity = Entities[e];
		Entity.Origin.move(x,y,z);
	}
}

void group::MoveSecs(vector<gvector> &Move, bool LockBrushShifts)
{
	for (int b = 0; b<t_brushes; b++) // Brushes
	{
		brush &Brush = Brushes[b];
		int sec = Brush.SecID;
		Brush.Move( Move[sec].x, Move[sec].y, Move[sec].z, LockBrushShifts );
	}
	for (int e = 0; e<t_ents; e++) // Entities
	{
		entity &Entity = Entities[e];
		int sec = Entity.SecID;
		Entity.Origin.move(Move[sec].x, Move[sec].y, Move[sec].z);
	}
}

void group::RotOriginSecs(vector<float> &RotX, vector<float> &RotY, vector<float> &RotZ, vector<vertex> &Origin, bool LockBrushShifts)
{
	for (int b = 0; b<t_brushes; b++) // Brushes
	{
		brush &Brush = Brushes[b];
		int sec = Brush.SecID;
		Brush.RotOrigin( RotX[sec], RotY[sec], RotZ[sec], Origin[sec] );
	}
	for (int e = 0; e<t_ents; e++) // Entities
	{
		entity &Entity = Entities[e];
		int sec = Entity.SecID;
		Entity.Origin.rotateOrigin(RotX[sec], RotY[sec], RotZ[sec], Origin[sec]);
		Euler RotAngles(RotX[sec], RotY[sec], RotZ[sec]);
		Entity.RotateEntity(RotAngles,1);
	}
}

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
	bool dev = 0;
	group &Group = *this;
	int g = Group.gID;
	if(dev)cout << "determine Face Lengths..." << endl;
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
				if(dev)cout << " calculating OFbrushID: (seg "<<seg<<"*cTable[g].res "<<cTable[g].res<<")+(cTable[g].res/4 "<<cTable[g].res/4<<")-1"<< endl;
				int OFbrushID; 
				if (cTable[gID].type!=3) OFbrushID = (seg2*cTable[g].res)+(cTable[g].res/4)-1;
				else OFbrushID = b;
				
				float OFaceLen = GetFaceLen(Group.Brushes[OFbrushID].Faces[f]);
				if(dev)cout << " OFaceLen: " << OFaceLen << " OFbrushID: " << OFbrushID << " brush: " << b << endl;
				
				if (Face.fID==2)
				{
					if (cTable[g].type==1)
						Face.LengthO = OFaceLen;
					else
						Face.LengthO = GetVecLen( GetVector(  Face.Vertices[0], Face.Vertices[1]  ));
					
					if (cTable[g].type!=3)
					Face.LengthN = GetFaceLen(Face);
					else
					Face.LengthN = Face.LengthO;
					//if (Brush.IsWedge) cout << "   Brush # "<<b<<" Face #" <<f<< " Tex: "<<Face.Texture << " Face Len " << Face.LengthN << endl;
					
					if(dev)if (b==0) cout << "Face #" <<f<< " Tex: "<<Face.Texture << "\nPitchO: " << Face.PitchO << "\nPitchN: " << Face.PitchN << "\nDifference O/N: " << Face.PitchN-Face.PitchO << endl << endl;
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
	if ( cTable[g].tri==0 && cTable[g].ramp>0 && cTable[g].type!=2 )
	{
		for (int b = 0; b < Group.t_brushes; b++)
		{
			brush &Brush = Group.Brushes[b];
			int sec = Brush.SecID;
			
			if ( Group.heightTableSteps[sec]!=0 )
			{
				if (cTable[g].type==3)
				{
					if (sec==0) // brush is first
					{
						brush &Next = Group.Brushes[b+1];
						if ( Brush.Yaw!=Next.Yaw )
							Brush.DoSplit = 1;
					}
					else if (sec>0&&sec<Group.sections-1) // brush isnt first or last
					{
						brush &Next = Group.Brushes[b+1];
						brush &Last = Group.Brushes[b-1];
						if ( Brush.Yaw!=Next.Yaw || Brush.Yaw!=Last.Yaw )
							Brush.DoSplit = 1;
					}
					else // brush is last
					{
						brush &Last = Group.Brushes[b-1];
						if ( Brush.Yaw!=Last.Yaw )
							Brush.DoSplit = 1;
					}
				}
				else Brush.DoSplit = 1;
			}
			//cout << " Brush " << b << " sec " << sec << " Split? " << Brush.DoSplit << " Step " << Group.heightTableSteps[sec] << endl;
		}
	}
	
	for (int b = 0; b < Group.t_brushes; b++)
	{
		brush &Brush = Group.Brushes[b];
		//cout << " Tri! Brush #" << b << " Split it? "; if (Brush.DoSplit) cout << " YES!" << endl; else cout << " NO!" << endl;
		
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
	bool dev = 0;
	group &Group = *this;
	int g = Group.gID;
	for (int b = 0; b<Group.t_brushes; b++)
	{
		brush &Brush = Group.Brushes[b];
		int sec = Brush.SecID;
		if (Brush.valid && (cTable[g].height!=0||cTable[g].heightmode==2))
		{
			float step = bGroup[g].heightTableSteps[sec];
			float height = bGroup[g].heightTable[sec];
			if(dev) cout << " Brush " << b << " sec " << sec << " start " << bGroup[g].range_start << " Height " << height << " Step " << step << " (bGroup["<<g<<"].heightTable["<<sec<<"]) " << endl;
			for (int f = 0; f<Brush.t_faces; f++)
			{
				face &Face = Brush.Faces[f];
				Face.AddHeight(height);
				//cout << " brush " << b << " face " << f << " sec " << sec << " height " << height << endl;
			}
		}
	}
}

void group::ShearVectors()
{
	group &Group = *this;
	int g = Group.gID;
	
	Group.MarkInsideSecBrushes();
	
	for (int b = 0; b<Group.t_brushes; b++) {
		brush &Brush = Group.Brushes[b];
		Brush.GetSourceFaces();
	}
	for (int b = 0; b<Group.t_brushes; b++)
	{
		brush &Brush = Group.Brushes[b];
		int sec = Brush.SecID;
		if( Brush.Tri!=nullptr&&sec>=Group.range_start&&sec<Group.range_end ) // Brush.IsDivisible&&!Brush.IsWedge&&Brush.t_faces==6
		{
			for (int bt = 0; bt<Brush.t_tri; bt++)
			for (int f = 0; f<Brush.Tri[bt].t_faces; f++)
			{
				face &Face = Brush.Tri[bt].Faces[f];
				//cout << " Brush " <<b<<" Tri " << bt << " Face " << f << " NULL " << Face.IsNULL << " ID " << Face.fID << " Orient " << Face.Orient << endl;
				bool reverse = 0; if( cTable[g].preverse && (cTable[g].type==2||cTable[g].type==3) ) reverse = 1;
				if(!Face.IsNULL&&Face.draw&&Face.fID==2&&Face.Orient!=6&&Face.Texture!="NULL"&&Face.Texture!=def_nulltex)
					Face.ConvertToSheared(Brush.Tri[bt].IsWedge2, IsSecInside[sec], reverse, Brush);
			}
		}
	}
}

void group::MarkInsideSecBrushes()
{
	// determins whether the brushes section is facing to the inside (longest edge is inside) or the outside (longest edge outside)
	// depends on where the first section is facing
	group &Group = *this;
	int g = Group.gID;
	IsSecInside.resize(Group.sections);
	fill(IsSecInside.begin(), IsSecInside.end(), 1);
	
	for (int b = 0; b<Group.t_brushes; b++) {
		brush &Brush = Group.Brushes[b];
		int sec = Brush.SecID;
		int seg = Brush.SegID;
		
		//if(seg==0)
		for (int f = 0; f<Brush.t_faces; f++)
		{
			face &Face = Brush.Faces[f];
			if(!Face.IsNULL&&Face.draw&&Face.fID==2&&(Face.Orient==4||Face.Orient==5))
			{
				// 2 = Top, 3 = Down, 4 = Front, 5 = Back
				if( Face.Orient==5 && Face.EdgeLenL==Face.HSourceL->EdgeLenL )
				{
					//cout << " Face " << Face.name << " Orient " << Face.Orient << " LenL " << Face.EdgeLenL << " LenS " << Face.EdgeLenS << " Source " << Face.HSourceL->EdgeLenL << endl;
					IsSecInside[sec] = 0;
					break;
				}
				else if( Face.Orient==4 && Face.EdgeLenS==Face.HSourceS->EdgeLenS )
				{
					//cout << " Face " << Face.name << " Orient " << Face.Orient << " LenL " << Face.EdgeLenL << " LenS " << Face.EdgeLenS << " Source " << Face.HSourceS->EdgeLenS << endl;
					IsSecInside[sec] = 0;
					break;
				}
			}
		}
	}
	
	/*cout << " Sections Facing..." << endl;
	for (int i=0;i<IsSecInside.size(); i++)
	{
		cout << " #" << i << " " << IsSecInside[i] << endl;
	}*/
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
	if (cTable[g].type==2||cTable[g].type==3)
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
			else if (cTable[g].type==2||cTable[g].type==3) // GRID PATH CIRCLES
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
						
						//if(dev) cout << " Base Face for Section " << sec << " is now " << Group.SecBaseFace[sec]->name << " Tex " <<  Group.SecBaseFace[sec]->Texture << " stored in Pointer " << Group.SecBaseFace[sec] << endl;
					}
				}
			}
		}
		if (dev) getch();
		
		// make Section Base Faces identify themselfs as what they are
		if(dev) cout << endl << " make Section Base Faces identify themselfs as what they are..." << endl;
		for (int i = 0; i<cTable[g].res; i++)
		{
			//cout << " SecBaseFace " << i << " Adress " << *SecBaseFace[i] << " " << &SecBaseFace[i] << endl;
			Group.SecBaseFace[i]->IsSecBase = 1;
			if(dev) cout << " Section " << i << " Base Face fID " << Group.SecBaseFace[i]->name << " Tex " << Group.SecBaseFace[i]->Texture << " EdgeLenL " << Group.SecBaseFace[i]->EdgeLenL << endl;
		}
		if (dev) getch();
		
		// check for nullpointer lengths
		if(dev) cout << endl << " check for nullpointer lengths..." << endl;
		for (int i = 0; i<Group.hSourceFace.size(); i++)
		{
			if (Group.hSourceFace[i]==nullptr)  {Group.hSourceFace[i] = Group.hSourceFace2[i]; if (dev) cout << " HSrcFace " << i << " is NULLPTR! changed to HSrcFace2." << endl; }
			if (Group.hSourceFace2[i]==nullptr) {Group.hSourceFace2[i] = Group.hSourceFace[i]; if (dev) cout << " HSrcFace2 " << i << " is NULLPTR! changed to HSrcFace." << endl; }
		}
		
		if(dev) cout << endl << " Edgelengths for all TexGroups so far..." << endl;
		if(dev)
		for (int i = 0; i<Group.hEdgeLen_temp.size(); i++) {
			if (Group.hSourceFace[i]!=nullptr)
			cout << "  Tex-Group "<< i <<" Edge-Length Long " << Group.hEdgeLen_temp[i] << "("<<Group.hSourceFace[i]->name<<")" << " Short " << Group.hEdgeLen_temp2[i] << "("<<Group.hSourceFace2[i]->name<<")"<< endl;
			else
			cout << "  Tex-Group "<< i <<" NULLPTR!" << endl;
		}
		if (dev) getch();
		
		// copy source face information to all faces
		if(dev) cout << endl << " copy source face information to all faces..." << endl;
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
						if(dev) {cout << "  Brush "<<b<<" Face "<<f<<" facegroup "<<Face.group<<" sec "<<sec<<" Source Face H-Shift Long " << Face.HSourceL->name <<" Short " << Face.HSourceS->name; if (Face.IsSecBase) cout<<"[X]"<<endl; else cout << endl;}
					}
				}
			}
		}
		if (dev) getch();
		
		// horizontal face shifts for source faces
		if(dev) cout << endl << " horizontal face shifts for source faces..." << endl;
		for (int sec = 0; sec < cTable[g].res; sec++)
		for (int b = 0; b < Group.t_brushes; b++)
		{
			brush &Brush = Group.Brushes[b];
			if (Brush.valid&&Brush.SecID==sec)
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
							LSrcFaceL = LBrush->Faces[f].HSourceL; //if(dev&&f==2) {cout << " LSrcFaceL "; if(LSrcFaceL!=nullptr) cout << LSrcFaceL->name << endl; else cout << " NULL!" << endl;}
							LSrcFaceS = LBrush->Faces[f].HSourceS; //if(dev&&f==2) {cout << " LSrcFaceS "; if(LSrcFaceS!=nullptr) cout << LSrcFaceS->name << endl; else cout << " NULL!" << endl;}
						}
						face &SrcFaceL = *Face.HSourceL;
						face &SrcFaceS = *Face.HSourceS;
						if (Face.fID==2)
						{
							int tg = Face.group;
							if (sec==0) {
								Face.HShiftL = SrcFaceL.EdgeLenL; if(dev&&f==2) cout << " b " << b << " sec " << sec << " Face.HShiftL " << Face.HShiftL << " \t( EdgeLenL "<< SrcFaceL.EdgeLenL <<" )" << endl;
								Face.HShiftS = SrcFaceS.EdgeLenS; //if(dev&&f==2) cout << " Face.HShiftS " << Face.HShiftS << endl;
							} else {
								Face.HShiftL = LSrcFaceL->HShiftL + SrcFaceL.EdgeLenL; if(dev&&f==2) cout << " b " << b << " sec " << sec << " Face.HShiftL " << Face.HShiftL << " \t( LHShift_L " << LSrcFaceL->HShiftL <<" \t["<<LSrcFaceL->name<<"] + \tEdgeLenL "<< SrcFaceL.EdgeLenL <<" \t["<<SrcFaceL.name<<"] )" << endl;
								Face.HShiftS = LSrcFaceS->HShiftS + SrcFaceS.EdgeLenS; //if(dev&&f==2) cout << " Face.HShiftS " << Face.HShiftS << endl;
							}
						}
					}
				}
			}
		}
		if (dev) getch();
		
		// final horizontal face shifts
		/*if(dev) cout << endl << " final horizontal face shifts..." << endl;
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
						if(dev&&f==2) { cout << "  B "<<b<<" F "<<f<< " sec " <<sec<< " FGroup "<< Face.group <<" H-Shift L " << Face.HShiftL << " \tS " << Face.HShiftS << " \tEdgeL " << SrcFaceL.EdgeLenL << " \tEdgeS " << SrcFaceS.EdgeLenS;
						if (sec>0)cout << " \tLSrcFaceL->HShiftL " << LSrcFaceL->HShiftL << " \t[ "<<LSrcFaceL->name<<" ]" << endl; else cout << endl; }
					
						//if(f==2) Face.Texture = Face.HSourceL->name;
						//else Face.Texture="{LARGE#S"+to_string(f-2);
					}
				}
			}
		}*/
	}
}

// create height Table for smooth ramp generation
void group::CreateHeightTable()
{
	bool dev = 0;
	group &Group = *this;
	int g = Group.gID;
	
	// create GapList
	Group.CreateGapList();
	
	int res = cTable[gID].res;
	int start = range_start;
	int end = range_end;
	int range = range_end-range_start;

	vector<float> &List = heightTable; List.resize(res+1);
	vector<float> &ListRel = heightTableSteps; ListRel.resize(res+1);
	vector<float> Slope;
	vector<float> Steps;
	
	// 0 = Linear, 1 = Smooth, 2 = Spline, 3 = Random Jagged, 4 = Random Smooth
	if (cTable[g].heightmode==0||(cTable[g].path=="UNSET"&&cTable[g].heightmode==2))
	{
		if (cTable[g].heightmode==2) {
			cout << "|    [WARNING] Heightmode set to \"Spline\", but no valid spline file found!" << endl;
			cout << "|              Using linear heightmode instead..."<<endl;
			cTable[g].heightmode=0;
		}
		if (dev) cout << " Creating Linear Heighttable... Step " << cTable[g].height << " range " << range << endl; 
		CreateSlopeLinear(cTable[g].height, cTable[g].res, Slope);
	}
	else if (cTable[g].heightmode==1)
	{
		if (dev) cout << " Creating Smooth Heighttable... Step " << cTable[g].height << " range " << range << endl; 
		CreateSlopeSmooth(cTable[g].height, range, Slope);
	}
	else if (cTable[g].heightmode==2)
	{
		if (dev) cout << " Creating Spline Heighttable... Step " << cTable[g].height << " range " << range << endl; 
		CreateSlopeSpline(gFile->PathList[g], range, Slope, Steps);
	}
	else if (cTable[g].heightmode==3)
	{
		if (dev) cout << " Creating Random Heighttable... Step " << cTable[g].height << " range " << range << endl; 
		CreateSlopeRandom(cTable[g].height, cTable[g].res, Slope);
	}
	
	// round heights
	if (cTable[g].heightmode!=2)
	for (int i=0; i<Slope.size(); i++)
		Slope[i] = round(Slope[i]);
	
	/*
	if(dev) {
	cout << endl << " Slope List (original) size " << Slope.size() << endl;
	for (int i=0; i<Slope.size(); i++)
		cout << " Slope["<<i<<"] " << Slope[i] << endl; getch();}
	
	// if heightmode 2 (spline), cut height info of last corner first
	if (cTable[g].heightmode==2&&cTable[g].type!=2)
	{
		path_set &Set = gFile->PathList[g];
		int erase = Set.Paths[0].t_corners-1;
		if(dev) cout << " Heightmode 2 -> Cleaning Slope List... first to be deleted: " << erase << endl;
		for (int i=0, p=1; i<Slope.size(); i++) {
			if(dev) cout << "   Entry #" << i+1 << "/"<<Slope.size()<< " " << Slope[i] << " erase " << erase << endl;
			if (i==erase) {
				if (dev) cout << "      ERASED!" << endl;
				Slope.erase(Slope.begin()+i);
				erase+= Set.Paths[p].t_corners-1;
				p++;
			}
		}
	}*/
	
	if(dev) {
	cout << endl << " Slope List (after cleansing) size " << Slope.size() << endl;
	for (int i=0; i<Slope.size(); i++)
		cout << " Slope["<<i<<"] " << Slope[i] << endl; getch();}
	
	// create Heightlist and StepList
	int off=0; if(cTable[g].heightmode!=2) off=1;
	for (int i=0, j=0; i<res+off; i++)
	{
		if(dev) cout << " List #" << i+1 << "/"<<res<< " start-offset " << start << " Heightlist #" << j+1 << "/"<<Slope.size()<<" Gap " << GapList[i] << endl;
		List[i] = 0;
		ListRel[i] = 0;
		if (i>=start) {
			if (cTable[g].heightmode==2) {
				List[i] = Slope[j];
				ListRel[i] = Steps[j];
				j++;
			} else {
				if (!GapList[i]) {
					List[i] = Slope[j];
					ListRel[i] = Slope[j+1]-Slope[j];
					j++;
				} else {
					List[i] = Slope[j];
					ListRel[i] = 0;
				}
			}
		}
		/*if (i>=start) {
			if (!GapList[i]||(cTable[g].type==2&&cTable[g].heightmode==2)) {
				List[i] = Slope[j];
				ListRel[i] = Slope[j+1]-Slope[j];
				j++;
			} else {
				List[i] = Slope[j];
				ListRel[i] = 0;
			}
		}*/
	}
	if(dev)getch();
	
	// if random, connect last with first
	if (cTable[g].heightmode==3)
		ListRel[List.size()-2] = List[0] - List[List.size()-2];
	
	// fill out-of-range entries with 0
	for (int i=0; i<List.size(); i++)
	{
		if(i<start||i>end) {
			List[i] = 0;
			ListRel[i] = 0;
		}
	}
	
	if (dev) { cout << " FINAL height table:" << endl; for (int i = 0; i<List.size(); i++) cout << "   #" << i << " " << List[i] << " Rel " << ListRel[i] << endl; getch(); }
}

// create GapList for Height Slope Generation
void group::CreateGapList()
{
	bool dev = 0;
	group &Group = *this;
	int g = Group.gID;
	GapList.resize(cTable[g].res);
	for (int i=0; i<GapList.size(); i++)
		GapList[i] = 0;
		
	if (cTable[gID].type==2)
	{
		path_set &Set = gFile->PathList[g]; //mGroup->Brushes.cset[g];
		
		for (int p=0,i=0; p<Set.t_paths; p++) {
			for (int c=0; c<Set.Paths[p].t_corners-1; c++) {
				path_corner &Corner = Set.Paths[p].Corners[c];
				if (Corner.NextIsGap90) {
					Group.GapList[i+1] = 1;
					i++;
				}
				else if (Corner.NextIsGap180) {
					Group.GapList[i+1] = 1;
					Group.GapList[i+2] = 1;
					i+=2;
				}
				if (dev) cout << " GapList["<<i<<"] " << Group.GapList[i] << endl;
				i++;
			}
		}
	}
	
	if (dev)
	for (int i=0; i<Group.GapList.size(); i++) {
		cout << " GapList["<<i<<"] " << Group.GapList[i] << endl;
	}
}

bool group::CheckForSlopes()
{
	bool dev = 0;
	
	group &Group = *this;
	for (int b=0; b<Group.t_brushes; b++)
	{
		brush &Brush = Group.Brushes[b];
		if (Brush.step>0) return true;
	}
	for (int i=0; i<Group.heightTable.size(); i++)
	{
		if (Group.heightTable[i]>0) return true;
	}
}

// Generate Curve Object from loaded Settings, Paths and Map Source Object
void group::Build()
{
	bool dev = 0;
	
	group &Group = *this;
	int g = Group.gID;
	group &SrcGroup = sGroup[g];
	
	bool SHOW_LINES = 0;
	canvas *Canva;
	if (SHOW_LINES) Canva = new canvas(200, 50);
	
	if (dev)cout << " Building "<<Group.t_brushes<<" brushes | res " << cTable[Group.gID].res << " | sections " << Group.sections << " | segments " << Group.segments << endl;
	for (int b=0; b<Group.t_brushes; b++)
	{
		brush &Brush = Group.Brushes[b];
		int seg = Brush.SegID;
		int sec = Brush.SecID;
		brush &SrcBrush = SrcGroup.Brushes[seg];
		if (SrcBrush.valid)
		{
			if (dev)cout << "  Entering Brush #" << b+1 << " | Faces: " << Brush.t_faces << " | sec #" << sec+1 << "/"<<Group.sections<<" | seg #" << seg+1<<"/"<<Group.segments << endl;
			
			// circle vertex iterator
			int cIDa = sec; if (cTable[g].type==2||cTable[g].type==3) cIDa=sec*2;
			int cIDb = cIDa+1;
			Brush.step = SrcBrush.cset->c[0].Vertices[cIDa].step;
			Brush.pID = SrcBrush.cset->c[0].Vertices[cIDa].pID;
			Brush.Align = SrcBrush.cset->c[0].Vertices[cIDa].Align;
			Brush.DoSplit = SrcBrush.cset->c[0].Vertices[cIDa].DoTri;
			Brush.IsGap = SrcBrush.cset->c[0].Vertices[cIDa].IsGap;
			Brush.Yaw = SrcBrush.cset->c[0].Vertices[cIDa].Yaw;
			Brush.Pitch = SrcBrush.cset->c[0].Vertices[cIDa].Pitch;
			Brush.IsCCW = SrcBrush.cset->c[0].Vertices[cIDa].IsCCW;
			if (!SrcBrush.valid) Brush.valid=0;
			
			if (0) {
				cout << "  cIDa: " << cIDa << "(sec(*2 if type=2/3))" << endl;
				cout << "  cIDb: " << cIDb << "(sec+1)" << endl;
				cout << "  Step: " << Brush.step << endl;
				cout << "  pID: " << Brush.pID << endl;
				cout << "  Align: " << Brush.Align << endl;
				cout << "  DoSplit: " << Brush.DoSplit << endl;
				cout << "  IsGap: " << Brush.IsGap << endl << endl;
				cout << "  Contruction Frame: " << endl;
				getch();
				cout << SrcBrush.cset[seg] << endl;
			}
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
							if (cIDa==Group.sections-1&&cTable[g].type!=2&&cTable[g].type!=3) cIDb=0; else cIDb=cIDa+1;
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
				
				if(SHOW_LINES)
				for (int i=0;i<Face.vcount-1; i++) 
					 { Canva->CreateLine(0.1, Face.Vertices[i], Face.Vertices[i+1]); }
			}
			if(SHOW_LINES) Canva->Print();
			if (dev) getch();
		}
	}
	if(SHOW_LINES) delete Canva;
	// DEV INFO
	if (0)
	for (int b=0; b<Group.t_brushes; b++)
	{
		brush &Brush = Group.Brushes[b];
		{
			cout << " Brush #" << b << " sec " << Brush.SecID << " seg " << Brush.SegID << endl;
			cout << "  Step: " << Brush.step << endl;
			cout << "  pID: " << Brush.pID << endl;
			cout << "  Align: " << Brush.Align << endl;
			cout << "  DoSplit: " << Brush.DoSplit << endl << endl;
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
	
	if (Group.t_brushes-Group.invalids<=0) {
		Group.valid = 0;
		Group.ValidMesh = 0;
	}
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
			if (Brush.Gap!=nullptr)
			{
				brush &GapBrush = *Brush.Gap;
				for (int f = 0; f<GapBrush.t_faces; f++)
				{
					face &Face = GapBrush.Faces[f];
					if (Face.draw)
					{
						for (int v = 0; v<Face.vcount; v++)
						{
							vertex &V = Face.Vertices[v];
							
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
	

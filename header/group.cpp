#include "group.h"
#include "brush.h"
#include "settings.h"
#include "file.h"
#include "utils.h"

#include <iostream>
#include <fstream>
#include <iomanip> // precision

#define DEBUG 0

using namespace std;

struct brush;
struct entity;

extern ctable *cTable;
extern group *bGroup;
extern group *mGroup;
extern group *sGroup;
extern file *gFile;
extern string def_nulltex;
extern bool G_DEV;

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

bool group::IsSecInRange(int secID)
{
	group &Group = *this;
	
	if (secID<Group.range_end && secID>=Group.range_start)
	{
		//cout << " ++++++ TRUE +++++ IsSecInRange Group " << Group.gID << " sec " << secID << " range_end " << Group.range_end << " range_start " << Group.range_start << endl;
		return 1;
	}
	else
	{
		//cout << " ++++++ FALSE+++++ IsSecInRange Group " << Group.gID << " sec " << secID << " range_end " << Group.range_end << " range_start " << Group.range_start << endl;
		return 0;
	}
}

void group::WeldGroupVertices(bool WeldGaps)
{
	#if DEBUG > 0
	bool dev = 0;
	if(dev) cout << " Welding Brushes " << WeldGaps << endl;
	#endif
	
	group &Group = *this;
	int g = Group.gID;
	
	if(!WeldGaps)
	{
		vector<vertex> agents;
		// if sec ID isnt odd numbered, add all brush vertices to agent list
		for(int b=0; b<Group.t_brushes; b++)
		{
			brush &Brush = Group.Brushes[b];
			if(  Brush.SecID % 2 == 0  ) // only even numbered section brushes
			for(int f=0; f<Brush.t_faces; f++)
			{
				face &Face = Brush.Faces[f];
				for(int v=0; v<Face.vcount; v++)
				{
					vertex &V = Face.Vertices[v];
					V.SecID = Brush.SecID;
					V.SegID = Brush.SegID;
					if(V.carved)
					agents.push_back(V);
				}
			}
		}
		
		// detail brushes
		// weld all vertices to agents, if they share the same position (precision!)
		for(int b=0; b<Group.t_brushes; b++)
		{
			brush &Brush = Group.Brushes[b];
			if( Brush.SecID % 2 != 0 )
			for(int f=0; f<Brush.t_faces; f++)
			{
				face &Face = Brush.Faces[f];
				for(int v=0; v<Face.vcount; v++)
				{
					vertex &V = Face.Vertices[v];
					for (int a=0; a<agents.size(); a++)
					{
						vertex &A = agents[a];
						if(( A.SegID==Brush.SegID ) && ( A.SecID==Brush.SecID-1||A.SecID==Brush.SecID+1 ))
						if( CompareVerticesDeci(V, A, 2) )
						{
							#if DEBUG > 0
							if(dev) cout << setprecision(8) << " Welding Detail Brush " <<b<<" F " << f << " V " << v << V << " sec " << Brush.SecID << " seg " << Brush.SegID << " to Agent " << a << A << " sec " << A.SecID << " seg " << A.SegID << endl;
							#endif
							
							V.x = A.x;
							V.y = A.y;
							V.z = A.z;
							break;
						}
					}
				}
			}
		}
	}
	else
	{
		vector<vertex> agents;
		// if gaps are active, every section is an agent, all gaps will be welded
		for(int b=0; b<Group.t_brushes; b++)
		{
			brush &Brush = Group.Brushes[b];
			for(int f=0; f<Brush.t_faces; f++)
			{
				face &Face = Brush.Faces[f];
				for(int v=0; v<Face.vcount; v++)
				{
					vertex &V = Face.Vertices[v];
					V.SecID = Brush.SecID;
					V.SegID = Brush.SegID2;
					agents.push_back(V);
				}
			}
		}
		
		// weld gaps to agents
		for(int b=0; b<Group.t_brushes; b++)
		{
			brush &Brush = Group.Brushes[b];
			if( Brush.Gap!=nullptr )
			{
				brush &Gap = *Brush.Gap;
				for(int f=0; f<Gap.t_faces; f++)
				{
					face &Face = Gap.Faces[f];
					for(int v=0; v<Face.vcount; v++)
					{
						vertex &V = Face.Vertices[v];
						for (int a=0; a<agents.size(); a++)
						{
							vertex &A = agents[a];
							if(( A.SegID==Gap.SegID ) && ( A.SecID==Gap.SecID||A.SecID==Gap.SecID-1 ))
							if( !CompareVertices(V,A) )
							if( CompareVerticesDeci(V, A, 2) )
							{
								#if DEBUG > 0
								if(dev) cout << " Welding Gap  Brush " << setprecision(8) <<b<<" F " << f << " V " << v << V << " sec " << Gap.SecID << " seg " << Gap.SegID << " to Agent " << a << A << " sec " << A.SecID << " seg " << A.SegID << endl;
								#endif
								
								V.x = A.x;
								V.y = A.y;
								V.z = A.z;
								break;
							}
						}
					}
				}
			}
		}
	}
}

void group::FillUnsetKeySettings()
{
	group &Group = *this;
	int g = Group.gID;
	
	if(d_autoyaw<0)     Group.d_autoyaw 		= cTable[g].d_autoyaw;
	if(d_autopitch<0) 	Group.d_autopitch		= cTable[g].d_autopitch;
	if(d_enable<0) 		Group.d_enable			= cTable[g].d_enable;
	if(d_circlemode<0) 	Group.d_circlemode		= cTable[g].d_circlemode;
	
	if(d_pos<0) 		Group.d_pos				= cTable[g].d_pos;
	if(d_separate<0) 	Group.d_separate		= cTable[g].d_separate;
	if(d_autoname<0) 	Group.d_autoname		= cTable[g].d_autoname;
	if(d_draw<0) 		Group.d_draw			= cTable[g].d_draw;
	if(d_draw_rand<0) 	Group.d_draw_rand		= cTable[g].d_draw_rand;
	if(d_skip<0) 		Group.d_skip			= cTable[g].d_skip;
	if(d_carve<0) 		Group.d_carve			= cTable[g].d_carve;
	
	if (!Group.d_pos_rand.IsSet) 	Group.d_pos_rand	= cTable[g].d_pos_rand;
	if (!Group.d_rotz_rand.IsSet) 	Group.d_rotz_rand	= cTable[g].d_rotz_rand;
	if (!Group.d_movey_rand.IsSet) 	Group.d_movey_rand	= cTable[g].d_movey_rand;
	if (!Group.d_scale_rand.IsSet) 	Group.d_scale_rand	= cTable[g].d_scale_rand;
}

void group::MarkGroupOriginObjects()
{
	group &Group = *this;
	for(int b=0; b<Group.t_brushes; b++)
	{
		brush &Brush = Brushes[b];
		Brush.IsOriginBrush();
		if(Brush.IsOrigin) Group.HasOrigin=1;
	}
	for(int e=0; e<Group.t_ents; e++)
	{
		entity &Entity = Entities[e];
		Entity.IsOriginEntity();
		if(Entity.IsOrigin) Group.HasOrigin=1;
	}
}

void group::GetGroupOriginCustom()
{
	group &Group = *this;
	for(int b=0; b<Group.t_brushes; b++)
	{
		brush &Brush = Brushes[b];
		if(Brush.IsOrigin)
		{
			Brush.GetBrushDimensions(1);
			Group.Origin = Brush.Origin;
			break;
		}
	}
	for(int e=0; e<Group.t_ents; e++)
	{
		entity &Entity = Entities[e];
		if(Entity.IsOrigin)
		{
			Group.Origin = Entity.Origin;
			break;
		}
	}
}

void group::Move(float x, float y, float z, bool LockBrushShifts)
{
	for (int b = 0; b<t_brushes; b++) // Brushes
	{
		brush &Brush = Brushes[b];
		Brush.Move(x,y,z,LockBrushShifts, gID);
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
		Brush.Move( Move[sec].x, Move[sec].y, Move[sec].z, LockBrushShifts, gID);
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
		Brush.RotOrigin( RotX[sec], RotY[sec], RotZ[sec], Origin[sec], gID);
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

void group::ScaleOriginSecs(vector<float> &Scale, vector<vertex> &Origin)
{
	for (int b = 0; b<t_brushes; b++) // Brushes
	{
		brush &Brush = Brushes[b];
		int sec = Brush.SecID;
		Brush.ScaleOrigin(Scale[sec], Origin[sec], gID);
	}
	for (int e = 0; e<t_ents; e++) // Entities
	{
		entity &Entity = Entities[e];
		int sec = Entity.SecID;
		Entity.Origin.ScaleOrigin(Scale[sec], Origin[sec]);
		Entity.key_scale *= Scale[sec];
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
	#if DEBUG > 0
	bool dev = 0;
	if(dev) cout<< endl << " Getting Vertex Angles..." << endl;
	#endif
	
	group &Group = *this;
	for (int b = 0, i = 0; b<Group.t_brushes; b++)
	{
		brush &Brush = Group.Brushes[b];
		
		if (Brush.valid)
		{
			float &sangle = Brush.vAngle_s;
			float &bangle  = Brush.vAngle_b;
			
			#if DEBUG > 0
			if(dev) cout << "  Brush Centroid " << Brush.centroid << endl;
			#endif
			
			for (int f = 0; f<Brush.t_faces; f++)
			{
				face &Face = Brush.Faces[f];
				#if DEBUG > 0
				if(dev) cout<< "   Face " << f << endl;
				#endif
				
				if (Face.fID==2)
				{
					for (int v = 0; v<Face.vcount; v++) // changed from Face.vcount to 3
					{
						vertex ev = Face.Vertices[v];
						#if DEBUG > 0
						if(dev) cout << "     Vertex " << v << ev << endl;
						#endif
						
						ev.x = 0.0;
						vertex &cv = Brush.centroid;
						gvector vec = GetVector(ev, cv);
						
						gvector achecker(0.0,0.0,-1.0); //angle checker vector
						float &vangle = Face.Vertices[v].angle;
						float angle = GetVecAng(achecker, vec);
						
						if (ev.y>=cv.y)	vangle = angle;
						else 			vangle = 360.0-angle;
						
						#if DEBUG > 0
						if(dev) cout << "     vangle " << vangle << " angle " << angle << endl;
						#endif
						
						// for comparison, save smallest and largest vertex angle as floating point number to each brush
						if (i==0) sangle = vangle;
						if (vangle < sangle)
						{
							#if DEBUG > 0
							if(dev) cout << "     angle ("<<vangle<<") is smaller than smallest ("<<sangle<<"). angle is new smallest!" << endl;
							#endif
							
							sangle = vangle;
						}
						if (vangle > bangle)
						{
							#if DEBUG > 0
							if(dev) cout << "     angle ("<<vangle<<") is bigger  than biggest  ("<<bangle<<"). angle is new biggest!" << endl;
							#endif
							
							bangle = vangle;
						}
	
						#if DEBUG > 0
						if(dev) cout << "     i: "<<i<<", Brush "<<b<<", Face " << f << ", Vertex " << v<< "\t " <<ev << "\t Angle: " << Face.Vertices[v].angle << endl;
						#endif
						
						i++;
					}
				}
			}
			#if DEBUG > 0
			if(dev) cout << "  smallest angle of this brush: " << sangle << ", biggest: " << bangle << endl;
			#endif
			
			i = 0;
		}
	}
	
	#if DEBUG > 0
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
	if(dev) system("pause");
	#endif
}

// reconstruct a source map to be able to calculate the original texture shifts/offsets
void group::GetGroupVertexList()
{
	#if DEBUG > 0
	bool dev = 0;
	#endif
	group &Group = *this;
	#if DEBUG > 0
	if (dev) cout << " GetBrushSimpleCentroid..." << endl;
	#endif
	Group.GetBrushSimpleCentroid(); // used to get vertex list
	#if DEBUG > 0
	if (dev) cout << " ClearBrushVertexList..." << endl;
	#endif
	Group.ClearBrushVertexList();
	#if DEBUG > 0
	if (dev) cout << " GetBrushVertexAngles..." << endl;
	#endif
	Group.GetBrushVertexAngles();
	#if DEBUG > 0
	if (dev) cout << " GetBrushFaceVertexSE..." << endl;
	#endif
	Group.GetBrushFaceVertexSE();
	#if DEBUG > 0
	if (dev) cout << " GetBrushVertexListSE..." << endl;
	#endif
	Group.GetBrushVertexListSE();
	#if DEBUG > 0
	if (dev) cout << " GetBrushVertexList..." << endl;
	#endif
	Group.GetBrushVertexList();
}

// reconstruct a source map to be able to calculate the original texture shifts/offsets
void group::ReconstructMap()
{
	#if DEBUG > 0
	bool dev = 0;
	#endif
	// Vertex List #1
	#if DEBUG > 0
	if (dev) cout << " Vertex List #1..." << endl;
	#endif
	mGroup->GetGroupVertexList();
	
	// Reconstruct Brush Vertices
	#if DEBUG > 0
	if (dev) cout << " Reconstruct Brush Vertices..." << endl;
	#endif
	mGroup->GetRconBrushVertices();

	// Face Centroids
	#if DEBUG > 0
	if (dev) cout << " Face Centroids..." << endl;
	#endif
	mGroup->GetBrushFaceCentroidsC();
	
	// Get Missing Vertices from RCON Mesh
	#if DEBUG > 0
	if (dev) cout << " Get Missing Vertices from RCON Mesh..." << endl;
	#endif
	mGroup->ConvertRconBrushVertices();

	// Face Centroids
	//mGroup->GetBrushFaceCentroids();
	
	// Vertex List #2 based on Reconstructed Vertices
	#if DEBUG > 0
	if (dev) cout << " Vertex List #2 based on Reconstructed Vertices..." << endl;
	#endif
	mGroup->GetGroupVertexList();
	
	for(int b=0; b<mGroup->t_brushes; b++)
		for(int f=0; f<mGroup->Brushes[b].t_faces; f++) {
			face &Face = mGroup->Brushes[b].Faces[f];
			//Face.GetNormal(); // not necessary
			Face.SortVertices(Face.Normal);
		}
}

// reconstruct working source objects that were copied from the previously reconstructed source map
void group::Reconstruct()
{
	#if DEBUG > 0
	bool dev = 0;
	#endif
	group &Group = *this;
	#if DEBUG > 0
	if(dev) cout << " Reconstructing Group " << Group.gID << "..." << endl;
	#endif

	#if DEBUG > 0
	if(dev) cout << "   Getting Vertex List... " << endl;
	#endif
	Group.GetGroupVertexList();
	
	#if DEBUG > 0
	if(dev) cout << "   GetBrushFaceCentroidsC..." << endl;
	#endif
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
	#if DEBUG > 0
	bool dev = 0;
	if (dev) cout << " Arranging Gaps... " << endl;
	#endif
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
		#if DEBUG > 0
		if (dev) cout << "  H Source Face is " << Group.SecBaseFace[sec] << endl;
		#endif
		
		face &HSrcFace = *Group.SecBaseFace[sec];
		//HSrcFace.Texture = "RED";
		SecEdge = HSrcFace.EdgeH; //GetVector(HSrcFace.Vertices[1],HSrcFace.Vertices[2]);
		
		#if DEBUG > 0
		if (dev) cout << "  Section " << sec << " HSrcFace Edge " << SecEdge << " Length " << GetVecLen(SecEdge) << endl;
		#endif
		
		for (int b = 0; b<Group.t_brushes; b++)
		{
			brush &Brush = Group.Brushes[b];
			brush &Gap = *Group.Brushes[b].Gap;
			int bsec = Brush.SecID;
			
			// Arrange all Brushes and their Gaps in Order, start with second Brush
			if (Brush.valid&&bsec==sec)
			{
				GapEdge = GetVector(Gap.Faces[2].Vertices[1],Gap.Faces[2].Vertices[2]);
				
				#if DEBUG > 0
				if (dev) cout << "    Brush " << b << " GapEdge " << GapEdge << " Length " << GetVecLen(GapEdge) << endl;;
				#endif
				
				Brush.Move(GapEdge.x,GapEdge.y,0,0,gID);
				
				if (Brush.Tri!=nullptr)
				{
					for (int bt = 0; bt<Brush.t_tri; bt++)
					{
						brush &BrushTri = Brush.Tri[bt];
						BrushTri.Move(GapEdge.x,GapEdge.y,0,0,gID);
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
				BrushN.Move(GapEdge.x,GapEdge.y,0,0,g);
				GapN.Move(GapEdge.x,GapEdge.y,0,0,g);
				
				if (BrushN.Tri!=nullptr)
				{
					for (int bt = 0; bt<BrushN.t_tri; bt++)
					{
						brush &BrushTri = BrushN.Tri[bt];
						BrushTri.Move(GapEdge.x,GapEdge.y,0,0,g);
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
	#if DEBUG > 0
	bool dev = 0;
	if(dev)cout << "determine Face Lengths..." << endl;
	#endif
	
	group &Group = *this;
	int g = Group.gID;
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
				#if DEBUG > 0
				if(dev)cout << " calculating OFbrushID: (seg "<<seg<<"*cTable[g].res "<<cTable[g].res<<")+(cTable[g].res/4 "<<cTable[g].res/4<<")-1"<< endl;
				#endif
				
				int OFbrushID; 
				if (cTable[gID].type!=3) OFbrushID = (seg2*cTable[g].res)+(cTable[g].res/4)-1;
				else OFbrushID = b;
				
				float OFaceLen = GetFaceLen(Group.Brushes[OFbrushID].Faces[f]);
				#if DEBUG > 0
				if(dev)cout << " OFaceLen: " << OFaceLen << " OFbrushID: " << OFbrushID << " brush: " << b << endl;
				#endif
				
				if (Face.fID==2)
				{
					if (cTable[g].type==1||(cTable[g].type==0&&cTable[g].flatcircle==1))
						Face.LengthO = OFaceLen;
					else
						Face.LengthO = GetVecLen( GetVector(  Face.Vertices[0], Face.Vertices[1]  ));
					
					if (cTable[g].type!=3)
					Face.LengthN = GetFaceLen(Face);
					else
					Face.LengthN = Face.LengthO;
					
					#if DEBUG > 0
					if(dev)if (b==0) cout << "Face #" <<f<< " Tex: "<<Face.Texture << "\nPitchO: " << Face.PitchO << "\nPitchN: " << Face.PitchN << "\nDifference O/N: " << Face.PitchN-Face.PitchO << endl << endl;
					#endif
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
	
	/*for (int b = 0; b < Group.t_brushes; b++)
	{
		brush &Brush = Group.Brushes[b];
		Brush.DoSplit = 1;
	}*/
	
	for (int b = 0; b < Group.t_brushes; b++)
	{
		brush &Brush = Group.Brushes[b];
		//cout << " Tri! Brush #" << b << " Split it? "; if (Brush.DoSplit) cout << " YES!" << endl; else cout << " NO!" << endl;
		
		if (Brush.valid&&Brush.draw&&Brush.DoSplit&& IsSecInRange(Brush.SecID))
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
	group &Group = *this;
	int g = Group.gID;

	#if DEBUG > 0
	bool dev = 0;
	if (dev) cout << " Getting Transit Vertices for Group "<<g<<"..." << endl;
	#endif
	
	for (int b = 0; b < Group.t_brushes; b++)
	{
		brush &Brush = Group.Brushes[b];
		int sec = Brush.SecID;
		
		if (Brush.valid&&Brush.draw)
		{
			if (sec==Group.range_start)
			{
				Brush.MarkFaceVertices(Brush.Faces[0], 0, 0);
				
				#if DEBUG > 0
				if (dev) cout << "  Brush Section is range start " << Group.range_start << endl;
				#endif
			}
			else if (sec==Group.range_end-1)
			{
				Brush.MarkFaceVertices(Brush.Faces[1], 0, 0);
				
				#if DEBUG > 0
				if (dev) cout << "  Brush Section is range end " << Group.range_end << endl;
				#endif
			}
			
			if (Brush.Tri!=nullptr)
			for (int bt = 0; bt<Brush.t_tri; bt++)
			{
				brush &BrushTri = Brush.Tri[bt];
				
				if (sec==Group.range_start)
				{
					BrushTri.MarkFaceVertices(Brush.Faces[0], 0, 0);
					
					#if DEBUG > 0
					if (dev) cout << "  Brush Section is range start " << Group.range_start << endl;
					#endif
				}
				else if (sec==Group.range_end-1)
				{
					BrushTri.MarkFaceVertices(Brush.Faces[1], 0, 0);
					
					#if DEBUG > 0
					if (dev) cout << "  Brush Section is range end " << Group.range_end << endl;
					#endif
				}
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
		
		if (Brush.valid&&Brush.draw&& Group.IsSecInRange(Brush.SecID))
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

// add custom height to this brush
void group::AddBrushHeights()
{
	#if DEBUG > 0
	bool dev = 0;
	#endif
	
	group &Group = *this;
	int g = Group.gID;
	for (int b = 0; b<Group.t_brushes; b++)
	{
		brush &Brush = Group.Brushes[b];
		int sec = Brush.SecID;
		if (Brush.valid && (cTable[g].height!=0||cTable[g].heightmode==1))
		{
			float step = bGroup[g].heightTableSteps[sec];
			float height = bGroup[g].heightTable[sec];
			
			if ( !IsValid(step) ) step = 0;
			
			#if DEBUG > 0
			if(dev) cout << " Brush " << b << " sec " << sec << " start " << bGroup[g].range_start << " Height " << height << " Step " << step << " (bGroup["<<g<<"].heightTable["<<sec<<"]) " << endl;
			#endif
			
			for (int f = 0; f<Brush.t_faces; f++)
			{
				face &Face = Brush.Faces[f];
				Face.AddHeight(height);
			}
		}
	}
}

void group::ShearVectors()
{
	group &Group = *this;
	int g = Group.gID;
	
	for (int b = 0; b<Group.t_brushes; b++)
	{
		brush &Brush = Group.Brushes[b];
		Brush.GetSourceFaces();
	}

	for (int b = 0; b<Group.t_brushes; b++)
	{
		brush &Brush = Group.Brushes[b];
		int sec = Brush.SecID;
		if( Brush.Tri!=nullptr && Group.IsSecInRange(sec) )
		{
			for (int bt = 0; bt<Brush.t_tri; bt++)
			for (int f = 0; f<Brush.Tri[bt].t_faces; f++)
			{
				face &Face = Brush.Tri[bt].Faces[f];
				//cout << " Brush " <<b<<" Tri " << bt << " Face " << f << " NULL " << Face.IsNULL << " ID " << Face.fID << " Orient " << Face.Orient << endl;
				if ( Face.FaceIsValid(2,1,1,1) && Face.Orient!=6 )
					Face.ConvertToShearedTri(Brush.Tri[bt].IsWedge2, 0, 0, Brush, g);
			}
		}
		else if( Brush.Tri==nullptr && Group.IsSecInRange(sec) )
		{
			for (int f = 0; f<Brush.t_faces; f++)
			{
				face &Face = Brush.Faces[f];
				if ( Face.FaceIsValid(2,1,1,1) && Face.Orient!=6 )
					Face.ConvertToSheared(g);
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
	
	// rotation steps for simple splines
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
				float step_half = 0;
				if(cTable[g].flatcircle==1) step_half = steps/2;
				if (Face.fID<=1)
				{
					if (Face.fID==0) { // BASE FACES
						//cout << "Face " << f <<" ID == 0" << ", texture is: " << Face.Texture << endl;
						if (sec>0) deg = (steps*sec)-step_half;
						Face.VecX.rotate(0,0,deg);
						Face.VecY.rotate(0,0,deg);
					} else { // HEAD FACES
						//cout << "Face " << f <<" ID == 1" << ", texture is: " << Face.Texture << endl;
						deg = (steps*(sec+1))-step_half;
						Face.VecX.rotate(0,0,deg);
						Face.VecY.rotate(0,0,deg);
						//Face.Centroid.rotate(0,0,deg);
					}
					
					// Gaps
					if (cTable[g].gaps>0) {
						face &GapFace = Gap.Faces[f];
						if (sec>0) deg = (steps*sec)-step_half;
						GapFace.VecX.rotate(0,0,deg);
						GapFace.VecY.rotate(0,0,deg);
					}
				} else {// BODY FACES
					//cout << "Face " << f <<" ID == 2" << endl;
					if (sec==0) { deg = (steps/2.0)-step_half; }
					else 		{ deg = ((steps*sec) + (steps/2.0)) - step_half; }
					Face.VecH->rotate(0,0,deg);
					Face.VecV->rotate(0,0,deg);
					
					// Gaps
					if (cTable[g].gaps>0) {
						face &GapFace = Gap.Faces[f];
						deg = (steps*sec)-step_half;
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

void group::GroupTexturize()
{
	#if DEBUG > 0
	bool dev = 0;
	#endif
	
	group &Group 		= *this;
	int g 				= Group.gID;
	bool UseLongEdge 	= cTable[g].hshiftsrc;
	
	#if DEBUG > 0
	if(dev)system("pause");
	if(dev) cout << "Adding basic Texture Shifts" << endl;
	#endif
	
	// Get BaseShift before further texture composition
	for (int b=0; b<Group.t_brushes; b++)
	{
		brush &Brush 		= Group.Brushes[b];
		int sec 			= Brush.SecID;
		
		if (Group.IsSecInRange(sec))
		{
			for (int f = 0; f<Brush.t_faces; f++)
			{
				face &Face  = Brush.Faces[f];
				int Axis = 1;
				if (!Face.VecX.IsHor) Axis = 2;
				
				if ( Face.FaceIsValid(2,1,1,1) )
				{
					if(cTable[g].type!=2)
					{
						if(Brush.IsInside)
						GetBaseShift(Face,Axis,1,0);
						else if(!Brush.IsInside)
						GetBaseShift(Face,Axis,0,0);
					}
					else
					{
						GetBaseShift(Face,Axis,UseLongEdge,0);
					}
					#if DEBUG > 0
					if (dev) {cout << " Brush " << b << "Face " << f << " Orient " << Face.Orient << " BaseShift " << setw(9) << Face.BaseShiftX << " Adress " << &Face << " HsourceL " << Face.HSourceL << " HsourceS " << Face.HSourceS; if (&Face==Face.HSourceL) cout << " XXX L "; else if(&Face==Face.HSourceS) cout << " XXX S "; cout << endl;}
					#endif
				}
			}
		}
	}
	
	#if DEBUG > 0
	if(dev)system("pause");
	if (dev) cout << "Adding basic Texture Shifts" << endl;
	#endif
	
	// Fix Texture Scale and Shift for Body Faces that are smaller than before (happens on curve generation)
	for (int b=0; b<Group.t_brushes; b++)
	{
		brush &Brush 		= Group.Brushes[b];
		int seg 			= Brush.SegID;
		int sec 			= Brush.SecID;
		
		if (Group.IsSecInRange(sec))
		{
			#if DEBUG > 0
			if (dev) cout << endl << " #### BRUSH #" << b << " SECTION #" << sec << endl;
			#endif
			
			// horizontal face scale calculation NEW as of v0.8 Dec 2023
			for (int f = 0; f<Brush.t_faces; f++)
			{
				face &Face  = Brush.Faces[f];
				
				// Fix vertical Body Face Scales, if New Vertical Face Lengths differ from old ones
				float mx = 1.0;
				float my = 1.0;
				
				if ( Face.FaceIsValid(2,1,1,1) )
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
				if (Face.VecX.IsHor)  	msh = msx;
				else 					msh = msy;
				
				// Compose Face Shift
				Face.ShiftX = Face.BaseShiftX * mx;
				Face.ShiftY = Face.BaseShiftY * my;
				Face.ScaleX /= mx;
				Face.ScaleY /= my;
				
				// Add Shifts to horizontal Body Faces
				if ( Face.FaceIsValid(2,1,1,1) )
				{
					face *hFace = nullptr;
					face *LhFace = nullptr;
					float Temp_BaseShift = 0; 
					float Temp_EdgeLen = 0;
	
					if (cTable[g].shift==0)
					{
						if (Face.VecX.IsHor) 	{ Face.ShiftX = 0; Face.OffsetX = 0; }
						else 				 	{ Face.ShiftY = 0; Face.OffsetY = 0; }
					}
					else if (cTable[g].shift>0 && cTable[g].shift<6)
					{
						#if DEBUG > 0
						if(dev&&Face.FaceIsValid(-1,1,1,1)) {
						cout << " Brush " << setw(2) << b;
						//cout << " BrushInside " << Brush.IsInside;
						//cout << " SecInside " << Group.IsSecInside[sec];
						cout << " Face " 		<< setw(2) 	<< f;
						cout << " FG " 			<< setw(2) 	<< Face.group;
						cout << " msh " 		<< setw(2) 	<< msh;
						cout << " Orient " 		<< Face.Orient;
						cout << " BaseShift " 	<< setw(5)<< setprecision(0) << Temp_BaseShift;
						cout << " EdgeLen "  	<< setw(5)<< setprecision(0) << Temp_EdgeLen;
						cout << " ShiftX " 	  	<< setw(5)<< setprecision(0) << Face.ShiftX;
						cout << " FAdress " 	<< &Face;
						cout << " hFace " 		<< hFace;
						cout << " LhFace " 		<< LhFace;
						cout << " Tex " 		<< Face.Texture<< endl;
						}
						#endif
						
						if (UseLongEdge)		hFace = Face.HSourceL;
						else					hFace = Face.HSourceS;
						
						if(sec>0) {
						if (UseLongEdge)		LhFace = Face.LHSourceL;
						else					LhFace = Face.LHSourceS; }
						
						if (hFace->VecX.IsHor) 	Temp_BaseShift = hFace->BaseShiftX;
						else 					Temp_BaseShift = hFace->BaseShiftY;
						
						if (sec>0) {
						if (UseLongEdge)		Temp_EdgeLen = LhFace->HShiftL * msh;
						else					Temp_EdgeLen = LhFace->HShiftS * msh; }
						
						if(G_DEV) {
						if (UseLongEdge&& 		&Face==Group.hSourceFace[Face.group]) 	Face.Texture = "RED";
						if (!UseLongEdge && 	&Face==Group.hSourceFace2[Face.group]) 	Face.Texture = "YELLOW"; }
						
						#if DEBUG > 0
						/*if (sec>0 && Group.Brushes[b].IsGap && (&Face==Group.hSourceFace[Face.group] || &Face==Group.hSourceFace2[Face.group]) )
						{
							if(UseLongEdge) Face.Texture="RED";
							else Face.Texture="YELLOW";
							
							if(Face.LHSourceL!=nullptr) {
							Face.LHSourceL->Texture="{BLUE"; }
							
							if(Face.LHSourceS!=nullptr) {
							Face.LHSourceS->Texture="{BLUE"; }
						}*/
						#endif
						
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
							else 				 	{ Face.ShiftY = Temp_BaseShift; Face.OffsetY = 0; }
					}
				}
				
				// Add Offset to vertical body and head/base Face Shift
				if (Face.fID==2) {
					if (Face.VecX.IsHor) 	Face.ShiftY += Face.OffsetY;
					else 				 	Face.ShiftX += Face.OffsetX;
				} else {
											Face.ShiftY += Face.OffsetY;
											Face.ShiftX += Face.OffsetX;
				}
				
				Face.MiniShift();
			}
			
			#if DEBUG > 0
			if(dev) cout << " - - - - - - - - - - - - - - - - - -" << endl<< endl;
			#endif
		}
	}
	
	
	if(G_DEV) {
	cout << " +++++++++++++++++ FINAL TEXTURE SHIFTS +++++++++++++++++ " << endl;
	for (int b=0; b<Group.t_brushes; b++)
	{
		brush &Brush = Group.Brushes[b];
		CoutBrushFacesDevInfo(Brush, b, g, IsSecInRange(Brush.SecID) );
	}}
	
	// fix Gap Textures
	if (cTable[g].gaps>0&&cTable[g].type<=1)
	for (int b=0; b<Group.t_brushes; b++)
	{
		brush &Gap = *Group.Brushes[b].Gap;
		for (int f = 0; f<Gap.t_faces; f++)
		{
			face &Face  = Gap.Faces[f];
			GetBaseEdges(Face);
			GetBaseShift(Face,0,1,0);
			
			Face.ShiftX = Face.BaseShiftX + Face.OffsetX;
			Face.ShiftY = Face.BaseShiftY + Face.OffsetY;
			Face.MiniShift();
			
			#if DEBUG > 0
			//cout << " Gap Face BaseShiftX " << Face.BaseShiftX << " + OffsetX " << Face.OffsetX << " = ShiftX " << Face.ShiftX << endl;
			//cout << " Gap Face BaseShiftY " << Face.BaseShiftY << " + OffsetY " << Face.OffsetY << " = ShiftY " << Face.ShiftY << endl;
			#endif
		}
	}
}

void group::GroupTexturizeHStretch()
{
	#if DEBUG > 0
	bool dev = 0;
	if (dev) cout << "Stretching textures of hor-sourcefaces horizontally..." << endl;
	if (dev) system("pause");
	#endif
	
	group &Group 		= *this;
	int g 				= Group.gID;
	bool UseLongEdge 	= cTable[g].hshiftsrc;
	
	for (int b=0; b<Group.t_brushes; b++)
	{
		brush &Brush 		= Group.Brushes[b];
		int sec 			= Brush.SecID;
		int tsecs 			= Group.sections;
		int l_hstretchamt 	= cTable[g].hstretchamt;
		
		// horizontal face scale calculation NEW as of v0.8 Dec 2023
		if( Group.IsSecInRange(sec) )
		for (int f = 0; f<Brush.t_faces; f++)
		{
			face &Face  = Brush.Faces[f];
			
			// new hstretch and hstretchamt command
			// hstretchamt=0 	Automatic stretch to current export range, based on hor tex-size and custom scale
			// hstretchamt>0 	Stretch X times to current export range, ignores custom scale, only uses tex-size
			if ( Face.FaceIsValid(2,1,1,1) && ( ( &Face==Group.hSourceFace[Face.group] && UseLongEdge ) || ( &Face==Group.hSourceFace2[Face.group] && !UseLongEdge ) ) )
			{
				float horScale 			= 1.0; if (Face.VecX.IsHor) horScale = Face.ScaleX; else horScale = Face.ScaleY; 										// horizontal texture scale
				if (horScale<0) horScale*=-1;
				float horSize  			= 128; if (Face.VecX.IsHor) horSize = gFile->tTable_width[Face.tID]; else horSize = gFile->tTable_height[Face.tID]; 	// horizontal texture size
				int rsecs 				= Group.range_end - Group.range_start;
				int secBrushEnd   		= ((ceil((b+1.0)/tsecs)*tsecs)-(tsecs-Group.range_end-1))-2;
				int secBrushStart		= secBrushEnd-rsecs+1;
				
				float hRangeStart		= 0; if (Group.range_start>0) hRangeStart = Group.Brushes[secBrushStart-1].Faces[f].HShiftL;
				if (!UseLongEdge) {
				hRangeStart				= 0; if (Group.range_start>0) hRangeStart = Group.Brushes[secBrushStart-1].Faces[f].HShiftS; }
				
				float hRangeEnd   		= Group.Brushes[secBrushEnd].Faces[f].HShiftL;
				if (!UseLongEdge) {
				hRangeEnd   			= Group.Brushes[secBrushEnd].Faces[f].HShiftS; }
				
				float currentMaxHRange 	= hRangeEnd-hRangeStart;
				
				int roundStretchAmt 	= 0;
				float newHScale 		= 1.0;
				if (l_hstretchamt==0)
				{
					// rounded Amount of times the current texture can be tiled along the whole output range with the current scale
					roundStretchAmt = currentMaxHRange / (horScale * horSize);
					if (roundStretchAmt<=0) roundStretchAmt = 1;
					newHScale = currentMaxHRange / roundStretchAmt / horSize; // new horizontal texture scale
				}
				else if (l_hstretchamt>0)
				{
					// new h scale based on a fixed amount of tiles along the whole output range
					newHScale = currentMaxHRange / l_hstretchamt / horSize;
				}
				if (Face.VecX.IsHor) {
					if (Face.ScaleX<0 && newHScale>0) newHScale*=-1;
					Face.ScaleX = newHScale;
				} else {
					if (Face.ScaleY<0 && newHScale>0) newHScale*=-1;
					Face.ScaleY = newHScale;
				}
				
				#if DEBUG > 0
				if (dev) {
					cout << " B " <<setw(3)<< b
					<< " Sec "<<setw(2)<<sec
					<< " Secs T: "<<rsecs
					<< " hor Scale: "<<setw(8)<< horScale
					<< " hor Size: "<< setw(3)<< horSize
					<< " Range S: " << Group.range_start
					<< " Range R: " << Group.range_end
					<< " Brush S: "<<setw(2)<< secBrushStart
					<< " Brush E: "<<setw(2)<< secBrushEnd
					<< " Range S: "<< setw(8)<< hRangeStart
					<< " Range E: "<< setw(8)<< hRangeEnd
					<< " Range T: "<< setw(8)<< currentMaxHRange
					<< " Tex: "<< setw(13)<< gFile->tTable_name[Face.tID]
					<< " stretch: "<< roundStretchAmt
					<< " newHScale: "<< newHScale <<endl;
				}
				#endif
				
				GetBaseShift(Face, 1, 1, 0);
			}
		}
	}
	
	#if DEBUG > 0
	if (dev) cout << "Copying stretched hor-sourceface base-shifts to their children..." << endl;
	if(dev)system("pause");
	#endif
	
	// Copying stretched hor-sourceface base-shifts to their children
	for (int b=0; b<Group.t_brushes; b++)
	{
		brush &Brush 	= Group.Brushes[b];
		int sec			= Brush.SecID;
		
		if( Group.IsSecInRange(sec) )
		for (int f = 0; f<Brush.t_faces; f++)
		{
			face &Face  = Brush.Faces[f];
			
			if (    Face.FaceIsValid(2,1,1,1) && ( ( &Face!=Group.hSourceFace[Face.group] && UseLongEdge ) || ( &Face!=Group.hSourceFace2[Face.group] && !UseLongEdge ) )    )
			{
				if(UseLongEdge) {
					if (Face.VecX.IsHor) Face.ScaleX = Group.hSourceFace[Face.group]->ScaleX; else Face.ScaleY = Group.hSourceFace[Face.group]->ScaleY;
					if (Face.VecX.IsHor) Face.ShiftX = Group.hSourceFace[Face.group]->ShiftX; else Face.ShiftY = Group.hSourceFace[Face.group]->ShiftY;
				} else {
					if (Face.VecX.IsHor) Face.ScaleX = Group.hSourceFace2[Face.group]->ScaleX; else Face.ScaleY = Group.hSourceFace2[Face.group]->ScaleY;
					if (Face.VecX.IsHor) Face.ShiftX = Group.hSourceFace2[Face.group]->ShiftX; else Face.ShiftY = Group.hSourceFace2[Face.group]->ShiftY;
				}
			}
		}
	}
}

// get groups for horizontal shifts
void group::GetFaceGroups()
{
	#if DEBUG > 0
	bool dev = 0;
	#endif
	
	group &Group = *this;
	int g = Group.gID;
	
	int last = 1;
	vector<string> uniqTexList;
	vector<float> uniqTexListScale;
	vector<float> uniqTexListShift;
	if (Group.t_brushes>0 && Group.valid && cTable[g].shift!=0)
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
						if ( Face.fID==2 )
						{
							if (uniqTexList.size()==0)
							{
								uniqTexList.push_back(Brush.Faces[f].Texture);
								
								if (Face.VecX.IsHor)
								{
									uniqTexListScale.push_back(Brush.Faces[f].ScaleX);
									uniqTexListShift.push_back(Brush.Faces[f].ShiftX);
								} else {
									uniqTexListScale.push_back(Brush.Faces[f].ScaleY);
									uniqTexListShift.push_back(Brush.Faces[f].ShiftY);
								}
								
								#if DEBUG > 0
								if(dev) cout << " Test Loop FIRST added new texture to list -> " << Face.Texture << "\t/Sc " << uniqTexListScale[0] << "\t/Sh " << uniqTexListShift[0] <<  "\tlist size now " << uniqTexList.size() << endl;
								#endif
							}
							else
							{
								for (int l = 0; l<uniqTexList.size(); l++)
								{
									if (Face.Texture==uniqTexList[l] &&
									((Face.VecX.IsHor && Face.ScaleX == uniqTexListScale[l]) || (!Face.VecX.IsHor && Face.ScaleY == uniqTexListScale[l])) &&
									((Face.VecX.IsHor && Face.ShiftX == uniqTexListShift[l]) || (!Face.VecX.IsHor && Face.ShiftY == uniqTexListShift[l]))
									) break; // if current faces texture is already in the list, stop the comparison
									
									if (l==uniqTexList.size()-1)
									{
										uniqTexList.push_back(Face.Texture); // if end of texture list is reached and comparison loop wasnt stopped yet, texture is unique and is being added to list
										
										if (Face.VecX.IsHor) {
											uniqTexListScale.push_back(Brush.Faces[f].ScaleX);
											uniqTexListShift.push_back(Brush.Faces[f].ShiftX);
										} else {
											uniqTexListScale.push_back(Brush.Faces[f].ScaleY);
											uniqTexListShift.push_back(Brush.Faces[f].ShiftY);
										}
										
										#if DEBUG > 0
										if(dev) cout << " Test Loop #"<<l<<" added new texture to list -> " << Face.Texture << "\t/Sc " << uniqTexListScale[l] << "\t/Sh " << uniqTexListShift[l] <<  "\tlist size now " << uniqTexList.size() << endl;
										#endif
									}
								}
							}
						}
					}
					Brush.t_tgroups = uniqTexList.size();
				}
			}
		}
		
		#if DEBUG > 0
		if(dev) cout << " unique textures found: " << uniqTexList.size() << endl; if(dev) system("pause");
		if(dev) cout << " solid entities found in source object: " << gFile->t_solids << endl;
		#endif
		 
		// define group ID for Shift 5 (group texture)
		if (cTable[g].shift==5)
		for (int sec = 0, fgroup = 0; sec<Group.sections; sec++)
		{
			bool foundCheck = 0;
			for (int e = 0; e<gFile->t_solids+3; e++) // changed e = 1 to  e = 0 to enable world spawn brushes to have tex group shift. apparently works, yay!
			{
				bool foundEnt = 0;
				for (int t = 0; t<uniqTexList.size(); t++)
				{
					bool foundTex = 0;
					for (int b = 0; b<Group.t_brushes; b++)
					{
						brush &Brush = Group.Brushes[b];
						if (Brush.SecID==sec&&Brush.entID==e&&Brush.valid)
						{
							for (int f = 2; f<Brush.t_faces; f++)
							{
								face &Face = Brush.Faces[f];
								
								if ( Face.fID==2 && Face.Texture==uniqTexList[t] &&  //Face.FaceIsValid(2,0,0,1)
								((Face.VecX.IsHor && Face.ScaleX == uniqTexListScale[t]) || (!Face.VecX.IsHor && Face.ScaleY == uniqTexListScale[t])) &&
								((Face.VecX.IsHor && Face.ShiftX == uniqTexListShift[t]) || (!Face.VecX.IsHor && Face.ShiftY == uniqTexListShift[t]))
								)
								{
									Face.group = fgroup;
									Face.ugroup = t;
									foundEnt = 1;
									foundTex = 1;
									foundCheck = 1;
									
									#if DEBUG > 0
									if(dev) cout << "      Brush ("<<b<< ") Face ("<<f<<") Sec " << Brush.SecID << " eID " << Brush.entID << " Group is: " << Face.group << " Tex " << Face.Texture << " Scale " << uniqTexListScale[t] <<  " Shift " << uniqTexListShift[t] << endl;
									#endif
								}
							}
						}
					}
					if (foundTex&&foundCheck)
					{
						fgroup++;
						foundCheck = 0;
						
						#if DEBUG > 0
						if(dev) cout << "   End of Tex Loop - Facegroup Increased to " << fgroup << endl;
						#endif
					}
				}
				if (foundEnt&&foundCheck)
				{
					fgroup++;
					foundCheck = 0;
					
					#if DEBUG > 0
					if(dev) cout << "  End of Ent Loop - Facegroup Increased to " << fgroup << endl;
					#endif
				}
			}
			#if DEBUG > 0
			if(dev) cout << " End of Sec Loop - Facegroup Increased to" << fgroup << endl;
			#endif
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
					if ( Face.fID==2 )
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
						
						#if DEBUG > 0
						if(dev) cout << " Brush ("<<b<< ") Face ("<<f<<") Sec " << Brush.SecID << " eID " << Brush.entID << " Group is: " << Face.group << " Tex " << Face.Texture << endl;
						#endif
					}
				}
			}
		}
		
		#if DEBUG > 0
		if(dev) cout  << "  TOTAL GROUP COUNT: " << Group.hGroupsCount << endl;
		if(dev) system("pause");
		#endif
	}
}


void group::GetHorLengths()
{
	#if DEBUG > 0
	bool dev = 0;
	if(dev) cout << "Getting groups for horizontal shifts..." << endl;
	#endif
	
	group &Group = *this;
	int g = Group.gID;
	bool UseLongEdge = cTable[g].hshiftsrc;
	
	#if DEBUG > 0
	if(dev) cout << " GetFaceGroups..." << endl;
	#endif
	
	if (cTable[g].shift==0) Group.hGroupsCount = Group.sections;
	Group.GetFaceGroups();
	
	// NEW 2024 get a list of faces that are pointing inside and outside the most to easier find the best source-edge for hor-shift later
	int hgc = Group.hGroupsCount;
	vector <float>hPitch_temp;
	vector <float>hPitch_temp2; 
	if (Group.valid && Group.t_brushes>0)
	{
		hPitch_temp.resize(hgc);
		hPitch_temp2.resize(hgc);
		fill(hPitch_temp.begin(), hPitch_temp.end(), 0);
		fill(hPitch_temp2.begin(), hPitch_temp2.end(), 0);
		
		for (int b = 0; b < Group.t_brushes; b++)
		{
			brush &Brush = Group.Brushes[b];
			if (Brush.valid)
			{
				int &sec = Brush.SecID;
				for (int f = 0; f<Brush.t_faces; f++)
				{
					face &Face = Brush.Faces[f];
					if ( Face.FaceIsValid(2,1,1,1) )
					{
						// Long Edge
						if ( Face.PitchO<hPitch_temp[Face.group] )
							hPitch_temp[Face.group] = Face.PitchO;
						if ( Face.PitchO>hPitch_temp2[Face.group] )
							hPitch_temp2[Face.group] = Face.PitchO;
						
						//cout << " Brush " << setw(2) << b << " Face " << f << " sec " << setw(2) << sec << " Pitch is " << setw(5) << setprecision(1) << Face.PitchO << " Face.Group " << setw(2) << Face.group << " longest pitch " << hPitch_temp[Face.group] << " shortest pitch " << hPitch_temp2[Face.group] <<endl;
					}
				}
			}
		}
	}
	
	// get edge lengths for texture groups
	#if DEBUG > 0
	if(dev) cout << " get edge lengths for texture groups..." << endl;
	#endif
	if (Group.valid && Group.t_brushes>0)
	{
		Group.hEdgeLen_temp.resize(hgc);
		Group.hEdgeLen_temp2.resize(hgc);
		Group.hSourceFace.resize(hgc);
		Group.hSourceFace2.resize(hgc);
		
		fill(hEdgeLen_temp2.begin(), hEdgeLen_temp2.end(), -1);
		
		// NEW 2024 list of longest and shortest face-group edge-vectors for DEV purposes
		Group.fGroupVecL.resize(hgc);
		Group.fGroupVecS.resize(hgc);
		
		for (int b = 0; b < Group.t_brushes; b++)
		{
			brush &Brush = Group.Brushes[b];
			if (Brush.valid)
			{
				int &sec = Brush.SecID;
				for (int f = 0; f<Brush.t_faces; f++)
				{
					face &Face = Brush.Faces[f];
					bool IsPointIn = 0;
					
					if ( Face.FaceIsValid(2,1,1,1) )
					{
						gvector Vec1;
						gvector Vec2;
						
						vertex &V0 = Face.Vertices[0];
						vertex &V1 = Face.Vertices[1];
						vertex &V2 = Face.Vertices[2];
						float Edge1Len = 0;
						float Edge2Len = 0;
						
						if (Face.vcount>3) // Face is QUAD
						{
							vertex &V3 = Face.Vertices[3];
							Vec1.set(V1, V2, V1);
							Vec2.set(V0, V3, V0);
							Edge1Len = GetVecLen(Vec1); // OLD pre 2024: Edge1Len = GetVecLen(GetVector(V1, V2));
							Edge2Len = GetVecLen(Vec2); // OLD pre 2024: Edge2Len = GetVecLen(GetVector(V0, V3));
							
							//devFaceEdges.push_back(Vec1); // DEV
							//devFaceEdges.push_back(Vec2); // DEV
						}
						else // Brush is a TRIANGLE
						{
							if (Face.IsWedgeDown)
							{
								Vec1.set(V0, V2, V0);
								Vec2.set(V0, V0, V0);
								Edge1Len = GetVecLen(Vec1); // OLD pre 2024: GetVecLen(GetVector(V0, V2));
								Edge2Len = 0;
								//devFaceEdges.push_back(Vec1); // DEV
							}
							else
							{
								Vec1.set(V1, V2, V1);
								Vec2.set(V0, V0, V0);
								Edge1Len = GetVecLen(Vec1); // OLD pre 2024: GetVecLen(GetVector(V1, V2));
								Edge2Len = 0;
								//devFaceEdges.push_back(Vec1); // DEV
							}
						}
						#if DEBUG > 0
						if(dev) {cout << " Brush "<<setw(2)<<b<<" FG "<<setw(2)<<Face.group<<" Face "<<f<<" sec "<<setw(2)<< sec;
						if (Face.vcount>3) cout <<" SQUARE!"; else cout <<" TRIANG!";
						cout << " Eg1Len "<<fixed<<setprecision(0) <<setw(5) << Edge1Len << " Eg2Len " <<setw(5)<< Edge2Len << " WedgeDn " << Face.IsWedgeDown;}
						#endif
						
						Face.EdgeLenL = Edge1Len; if (Edge2Len>Edge1Len) Face.EdgeLenL = Edge2Len;
						Face.EdgeLenS = Edge1Len; if (Edge2Len<Edge1Len) Face.EdgeLenS = Edge2Len;
						
						bool GPATH = 0; if (cTable[g].type==2) GPATH = 1;
						bool GAP = 0; if (Brush.IsGap) GAP = 1;
						bool PO_L = 0; if (Face.PitchO==hPitch_temp[Face.group])  PO_L = 1;
						bool PO_S = 0; if (Face.PitchO==hPitch_temp2[Face.group]) PO_S = 1;
						
						// Im Falle von Grid Path (type=2) sind alle Kanten üblicherweise gleich lang. Das Finden des richtigen Quell-Faces ist daher nicht mit der normalen Methode möglich.
						// Alle Kanten sind grundsätzlich gleich lang. Unterschiedlich lange oder Null-Kanten treten nur bei Gap-Brushes auf.
						// Auf die Normale Methode verzichten und bei nicht-Gap-Brushes nur den jeweiligen kleinsten oder größten Pitch suchen, um HSource zu ermitteln.
						// Bei Gap-Brushes dann die normale Methode nutzen.
						
						// Normale Methode: über kantenlänge && (Type!=2 || type==2 && GAP)
						// Pitch Methode: Type==2 && !GAP
						
						// Long Edge
						if ( (Edge1Len>Group.hEdgeLen_temp[Face.group] && ((GPATH&&GAP)||(!GPATH))) || (PO_L&&GPATH&&!GAP) )
						{
							Group.hEdgeLen_temp[Face.group] = Edge1Len;
							Group.hSourceFace[Face.group] = &Face;
							
							Group.fGroupVecL[Face.group] = Vec1;// NEW 2024
							
							#if DEBUG > 0
							if(dev) cout << " HIT  LONG-Edge1";
							#endif
						}
						if ( (Edge2Len>Group.hEdgeLen_temp[Face.group] && ((GPATH&&GAP)||(!GPATH))) || (PO_L&&GPATH&&!GAP) )
						{
							Group.hEdgeLen_temp[Face.group] = Edge2Len;
							Group.hSourceFace[Face.group] = &Face;
							
							Group.fGroupVecL[Face.group] = Vec2;// NEW 2024
							
							#if DEBUG > 0
							if(dev) cout << " HIT  LONG-Edge2";
							#endif
						}
						
						// new since 2019.06.06
						if (Group.SecBaseFace[sec]==nullptr) Group.SecBaseFace[sec] = &Face;
						face &SecBase = *Group.SecBaseFace[sec];
						float SecBaseLen = SecBase.EdgeLenL;
						if (Edge1Len>SecBaseLen||Edge2Len>SecBaseLen)
						Group.SecBaseFace[sec] = &Face;
						
						// Short Edge
						if ( ((Edge1Len<Group.hEdgeLen_temp2[Face.group] && ((GPATH&&GAP)||(!GPATH))) || (PO_S&&GPATH&&!GAP)) || Group.hEdgeLen_temp2[Face.group]==-1 )
						{
							Group.hEdgeLen_temp2[Face.group] = Edge1Len;
							Group.hSourceFace2[Face.group] = &Face;
							
							Group.fGroupVecS[Face.group] = Vec1;// NEW 2024
							
							#if DEBUG > 0
							if(dev) cout << " HIT SHORT-Edge1 ( ("<<Edge1Len<<"<"<<Group.hEdgeLen_temp2[Face.group]<<"||"<<Group.hEdgeLen_temp2[Face.group]<<"==0 ) &&"<<Face.PitchO<<"=="<<hPitch_temp2[Face.group]<<")";
							#endif
						}
						if ( ((Edge2Len<Group.hEdgeLen_temp2[Face.group] && ((GPATH&&GAP)||(!GPATH))) || (PO_S&&GPATH&&!GAP)) || Group.hEdgeLen_temp2[Face.group]==-1 )
						{
							Group.hEdgeLen_temp2[Face.group] = Edge2Len;
							Group.hSourceFace2[Face.group] = &Face;
							
							Group.fGroupVecS[Face.group] = Vec2;// NEW 2024
							
							#if DEBUG > 0
							if(dev) cout << " HIT SHORT-Edge2 ("<<Edge2Len<<"<"<<Group.hEdgeLen_temp2[Face.group]<<"&&"<<Face.PitchO<<"=="<<hPitch_temp2[Face.group]<<")";
							#endif
						}
						
						#if DEBUG > 0
						if(dev) cout << endl;
						#endif
						//#if DEBUG > 0
						//if(dev) cout << " Base Face for Section " << sec << " is now " << Group.SecBaseFace[sec]->name << " Tex " <<  Group.SecBaseFace[sec]->Texture << " stored in Pointer " << Group.SecBaseFace[sec] << endl;
						//#endif
					}
				}
			}
		}
		
		#if DEBUG > 0
		if(dev) {
		for (int b = 0; b < Group.t_brushes; b++)
		{
			brush &Brush = Group.Brushes[b];
			CoutBrushFacesDevInfo(Brush,b,g, Group.IsSecInRange(Brush.SecID) );
		}system("pause"); }
		#endif
		
		//#if DEBUG > 0
		//if (dev) system("pause");
		//if(dev) cout << endl << " make Section Base Faces identify themselfs as what they are..." << endl;
		//#endif
		
		// make Section Base Faces identify themselfs as what they are // OBSOLETE?! used for gap brushes
		for (int i = 0; i<cTable[g].res; i++)
		{
			Group.SecBaseFace[i]->IsSecBase = 1;
			
			//#if DEBUG > 0
			//if(dev) cout << " Section " << i << " Base Face fID " << Group.SecBaseFace[i]->name << " Tex " << Group.SecBaseFace[i]->Texture << " EdgeLenL " << Group.SecBaseFace[i]->EdgeLenL << endl;
			//#endif
		}






		#if DEBUG > 0
		if(dev) cout << endl << " 0 0 0 0 check for nullpointer lengths..." << endl;
		if (dev) system("pause");
		#endif
		
		// check for nullpointer lengths
		for (int i = 0; i<Group.hSourceFace.size(); i++)
		{
			if (Group.hSourceFace[i]==nullptr)
			{
				Group.hSourceFace[i] = Group.hSourceFace2[i];
				Group.fGroupVecL[i] = Group.fGroupVecS[i];// NEW 2024
				
				#if DEBUG > 0
				if (dev) cout << " HSrcFaceL #" << i << " is NULLPTR! changed to HSrcFaceS" << endl;
				#endif
			}
			if (Group.hSourceFace2[i]==nullptr)
			{
				Group.hSourceFace2[i] = Group.hSourceFace[i];
				Group.fGroupVecS[i] = Group.fGroupVecL[i];// NEW 2024
				
				#if DEBUG > 0
				if (dev) cout << " HSrcFaceS #" << i << " is NULLPTR! changed to HSrcFaceL" << endl;
				#endif
			}
		}
		
		#if DEBUG > 0
		if(dev) cout << endl << " Edgelengths for all TexGroups so far..." << endl;
		if(dev)
		for (int i = 0; i<Group.hEdgeLen_temp.size(); i++) {
			if (Group.hSourceFace[i]!=nullptr)
			cout << "  Tex-Group "<< i <<" Edge-Length Long " << Group.hEdgeLen_temp[i] << "("<<Group.hSourceFace[i]<<")" << " Short " << Group.hEdgeLen_temp2[i] << "("<<Group.hSourceFace2[i]<<")"<< endl;
			else
			cout << "  Tex-Group "<< i <<" NULLPTR!" << endl;
		}
		if (dev) system("pause");
		#endif
		
		
		
		
		
		// copy source face information to all faces
		#if DEBUG > 0
		if(dev) cout << endl << " copy source face information to all faces..." << endl;
		#endif
		for (int b = 0; b < Group.t_brushes; b++)
		{
			brush &Brush = Group.Brushes[b];
			if (Brush.valid)
			{
				int &sec = Brush.SecID;
				for (int f = 0; f<Brush.t_faces; f++)
				{
					face &Face = Brush.Faces[f];
					if ( Face.FaceIsValid(2,1,1,1) )
					{
						Face.HSourceL = Group.hSourceFace[Face.group];
						Face.HSourceS = Group.hSourceFace2[Face.group];
						
						if( sec>0 && (cTable[g].type!=2) )
						{
							Face.LHSourceL = Group.Brushes[b-1].Faces[f].HSourceL;
							Face.LHSourceS = Group.Brushes[b-1].Faces[f].HSourceS;
						}
						
						#if DEBUG > 0
						if(dev) {cout << "  Brush ["<<setw(3)<<b<<"] Face ["<<setw(3)<<f<<"] facegroup ["<<setw(3)<<Face.group<<"] sec ["<<setw(3)<<sec<<"] Orient [" << Face.Orient << "] Source Face H-Shift Long [" << Face.HSourceL <<"] Short [" << Face.HSourceS << "]"; if (&Face==Face.HSourceL) cout<<"[XXX L]"; else if (&Face==Face.HSourceS) cout<<"[XXX S]"; cout << endl; }
						#endif
					}
				}
			}
		}
		
		
		
		
		
		#if DEBUG > 0
		if (dev) cout << endl << " look for inside brushes and switch hor source faces/lengths..." << endl;
		if (dev) system("pause");
		#endif
		
		// look for inside brushes and switch hor source faces/lengths (only happens on path extrusion)
		if(cTable[g].type!=2)
		Group.MarkInsideSecBrushes();
		
		
		
		
		
		#if DEBUG > 0
		if(dev) cout << endl << " LAST horizontal face shifts for source faces..." << endl;
		if(dev) system("pause");
		#endif
		
		// find source faces (L/S) of last section; important if last section simply has no respective source face (e.g. type=2 and last brush is Wedge Brush(happens in generated corners) )
		// new method (v0.8 2024), replaces LastBrush-Method
		if(cTable[g].type==2) // will only happen on path extrusions
		for (int b=Group.t_brushes-1; b>=0; b--)
		{
			brush &Brush = Group.Brushes[b];
			int sec 	 = Brush.SecID;
			
			if (  sec>0 && Brush.valid && Brush.draw && Group.IsSecInRange(sec)  ) // range added for v0.8 Update Dec 2023
			{
				for (int f = 0; f<Brush.t_faces; f++)
				{
					face &Face = Brush.Faces[f];
					
					if ( Face.FaceIsValid(2,1,1,1) ) // previously: Face.draw
					{
						#if DEBUG > 0
						if(dev)cout << " Brush " << b << " sec " << sec << " Face " << f << " UFGroup" << Face.ugroup << " FGroup " << Face.group << endl;
						#endif
						// compare all brushes of recent section until last hsoruce face is found
						bool foundLast = 0;
						
						for (int bb = b-1; bb>=0; bb--) // start with current brush and decent from here, because others are not relevant
						{
							brush &LBrush = Group.Brushes[bb];
							
							//if (  LBrush.SecID<sec )
							for (int ff = 0; ff<LBrush.t_faces; ff++)
							{
								face &LFace = LBrush.Faces[ff];
								if ( LFace.FaceIsValid(2,1,1,1) && LFace.ugroup==Face.ugroup ) //&& Face.LHSourceL==nullptr && Face.LHSourceS==nullptr
								{
									// Bei der Suche nach dem letzten gültigen hSourceFace sollen wedges mit Edgelen 0 bei Hsrcshift0 übersprungen werden
									//if(!LBrush.IsGap || ( cTable[g].hshiftsrc==1 && LBrush.IsGap))
									{
										Face.LHSourceL = LFace.HSourceL;
										Face.LHSourceS = LFace.HSourceS;
										
										#if DEBUG > 0
										if(dev)cout << " FOUND!!! Brush " << bb << " sec " << sec << " Face " << ff << " UFGroup" << Face.ugroup << " FGroup " << Face.group << endl;
										#endif
										
										foundLast=1;
									}
								}
								if(foundLast) {
								#if DEBUG > 0
								if(dev)cout << " Breaking face loop..."<<endl;
								#endif
								break;
								}
							}
							if(foundLast) {
							#if DEBUG > 0
							if(dev)cout << " Breaking Brush loop..."<<endl;
							#endif
							break;
							}
						}
					}
				}
			}
		}

		#if DEBUG > 0
		if(dev) cout << endl << " List last source faces..." << endl;
		if(dev) system("pause");
		// List last source faces
		if(dev)
		for (int b = 0; b < Group.t_brushes; b++)
		{
			brush &Brush = Group.Brushes[b];
			for (int f = 0; f<Brush.t_faces; f++)
			{
				face &Face = Brush.Faces[f];
				if ( Face.FaceIsValid(2,1,1,1) )
				{
					int lsec=-1;
					cout << " Brush "<<b<<" sec "<<Brush.SecID<<" f "<<f<< " ufg " << Face.ugroup << " fg " << Face.group << " Tex "<< Face.Texture << " lfaceL " << Face.LHSourceL << " lfaceS " << Face.LHSourceS<< endl;
				}
			}
		}
		if(dev) cout << endl << " horizontal face shifts for source faces..." << endl;
		if(dev) system("pause");
		#endif
		
		
		
		
		
		// horizontal face shifts for source faces
		for (int secc = 0; secc < cTable[g].res; secc++)
		for (int b = 0; b < Group.t_brushes; b++)
		{
			brush &Brush = Group.Brushes[b];
			if (  Brush.valid && Brush.SecID==secc && Group.IsSecInRange(secc)  ) // range added for v0.8 Update Dec 2023
			{
				int &sec = Brush.SecID;
				for (int f = 0; f<Brush.t_faces; f++)
				{
					face &Face = Brush.Faces[f];
					
					if ( Face.FaceIsValid(2,1,1,1) )
					{
						face &HSL = *Face.HSourceL;
						face &HSS = *Face.HSourceS;
						
						if (sec==0)
						{
							Face.HShiftL = HSL.EdgeLenL;
							Face.HShiftS = HSS.EdgeLenS;
						}
						else
						{
							Face.HShiftL = Face.LHSourceL->HShiftL + HSL.EdgeLenL;
							Face.HShiftS = Face.LHSourceS->HShiftS + HSS.EdgeLenS;
						}
						
						#if DEBUG > 0
						if(dev) cout << " b " << b << " sec " << sec << " Face.HShiftL " << Face.HShiftL << " Face.HShiftS " << Face.HShiftS << endl;
						#endif
					}
				}
			}
		}
		
		
		
		#if DEBUG > 0
		if (dev) {
		cout << " horizontal face shifts for source faces ...."<<endl;
		for (int b = 0; b < Group.t_brushes; b++)
		{
			brush &Brush = Group.Brushes[b];
			CoutBrushFacesDevInfo(Brush,b,g, Group.IsSecInRange(Brush.SecID) );
		}
		system("pause");
		}
		#endif
	}
}

void group::AddCustomShiftOffset()
{
	group &Group = *this;
	int &g = Group.gID;
	
	if (Group.valid && Group.t_brushes>0)
	{
		// add horizontal offset 
		for (int b = 0; b < Group.t_brushes; b++)
		{
			brush &Brush = Group.Brushes[b];
			int &sec = Brush.SecID;
			
			if (  Group.Brushes[b].valid && Group.IsSecInRange(Group.Brushes[b].SecID)  )
			{
				if (Brush.Tri==nullptr)
				{
					for (int f = 0; f<Brush.t_faces; f++)
					{
						face &Face = Brush.Faces[f];
						if ( Face.FaceIsValid(2,1,1,1) )
						{
							if (  cTable[g].hshiftoffset!=0  )
							{
								if (Face.VecX.IsHor) { 	Face.ShiftX += cTable[g].hshiftoffset;
								} else { 				Face.ShiftY += cTable[g].hshiftoffset; }
							}
						}
					}
				}
				else
				{
					for (int bt = 0; bt<Brush.t_tri; bt++)
					{
						brush &TBrush = Brush.Tri[bt];
						for (int f = 0; f<TBrush.t_faces; f++)
						{
							face &Face = TBrush.Faces[f];
							if ( Face.FaceIsValid(2,1,1,1) )
							{
								if (  cTable[g].hshiftoffset!=0  )
								{
									if (Face.VecX.IsHor) { 	Face.ShiftX += cTable[g].hshiftoffset;
									} else { 				Face.ShiftY += cTable[g].hshiftoffset; }
								}
							}
						}
					}
				}
				
			}
		}
	}
}

void group::MarkInsideSecBrushes()
{
	#if DEBUG > 0
	bool dev = 0;
	#endif
	
	// determins whether the brushes section is facing to the inside (longest edge is inside) or the outside (longest edge outside)
	// depends on where the first section is facing
	group &Group = *this;
	int g = Group.gID;
	IsSecInside.resize(Group.sections); // UNNESESSARY ATM??
	//fill(IsSecInside.begin(), IsSecInside.end(), 1); // UNNESESSARY ATM??
	
	vector<bool> hGroupIsInside;
	hGroupIsInside.resize(Group.hGroupsCount);
	//fill(hGroupIsInside.begin(), hGroupIsInside.end(), 1);
	
	/*for (int i=0; i<Group.hGroupsCount; i++)
	{
		brush *SrcBrush = nullptr; if(Group.hSourceFace[i]->Mother!=nullptr) SrcBrush = Group.hSourceFace[i]->Mother;
		if (SrcBrush->IsCCW&&SrcBrush->SecID>0) {
			hGroupIsInside[i] = 0;
			IsSecInside[SrcBrush->SecID] = 0;
		}
		SrcBrush->IsInside = IsSecInside[SrcBrush->SecID];
	}*/
	
	for (int b = 0; b<Group.t_brushes; b++)
	{
		brush &Brush = Group.Brushes[b];
		int sec = Brush.SecID;
		for (int f = 0; f<Brush.t_faces; f++)
		{
			face &Face = Brush.Faces[f];
			if (Brush.IsCCW&&sec>0) { 
				hGroupIsInside[Face.group] = 1;
				IsSecInside[sec] = 1;
			} else {
				hGroupIsInside[Face.group] = 0;
				IsSecInside[sec] = 0;
			}
			
			/*if(  Face.fID==2 && ( Face.Orient==4 || Face.Orient==5 )  ) //Face.FaceIsValid(2,1,1,0)
			{
				// 2 = Top, 3 = Down, 4 = Front, 5 = Back
				if( Face.Orient==5 && Face.EdgeLenL==Face.HSourceL->EdgeLenL )
				{
					IsSecInside[sec] = 0; // UNNESESSARY ATM??
					hGroupIsInside[Face.group] = 0;
					break;
				}
				else if( Face.Orient==4 && Face.EdgeLenS==Face.HSourceS->EdgeLenS )
				{
					IsSecInside[sec] = 0; // UNNESESSARY ATM??
					hGroupIsInside[Face.group] = 0;
					break;
				}
			}*/
		}
		Brush.IsInside = IsSecInside[sec]; // UNNESESSARY ATM??
	}
	
	// INSIDE SWITCH for all face-groups Edge Lens L/S, Source Face and Edge Vectors (used for path extrusions, if long side is actually short side)
	for(int i=0; i<Group.hGroupsCount; i++)
	{
		if(hGroupIsInside[i]&&Group.hSourceFace[i]!=nullptr)
		{
			swap (hSourceFace[i], hSourceFace2[i]);
			swap (hSourceFace[i]->EdgeLenL, hSourceFace[i]->EdgeLenS);
			swap (hSourceFace2[i]->EdgeLenL, hSourceFace2[i]->EdgeLenS);
			swap (fGroupVecL[i], fGroupVecS[i]); // ONLY FOR DEV PURPOSES
		}
	}
	
	// INSIDE SWITCH for all faces H-Source L/S (used for path extrusions, if long side is actually short side)
	for (int b = 0; b<Group.t_brushes; b++)
	{
		brush &Brush = Group.Brushes[b];
		for (int f = 0; f<Brush.t_faces; f++)
		{
			face &Face = Brush.Faces[f];
			if (hGroupIsInside[Face.group])
			{
				Face.HSourceL = hSourceFace[Face.group];
				Face.HSourceS = hSourceFace2[Face.group];
			}
		}
	}
	
	#if DEBUG > 0
	if(dev) {
	cout << "  FINAL INSIDE SEC LIST:" << Group.hGroupsCount << endl;
	for(int i=0; i<Group.hGroupsCount; i++)
	{cout << "     Face-Group "<< i << " hGroupIsInside " << hGroupIsInside[i] << endl;}
	}
	#endif
}

// Intersection planes for each group section
void group::CreateIsects()
{
	#if DEBUG > 0
	bool dev = 0;
	#endif
	
	group &Group = *this;
	int g = Group.gID;
	int size = cTable[g].res;

	#if DEBUG > 0
	if(dev) cout << endl << " Creating Intersection Plane List #"<<g<<" size "<< size << "..." << endl;
	#endif
	
	vector<gvector> &List = SecIsect;
	List.resize(size*2);
	
	// Pi Circle
	if (cTable[g].type==0)
	{
		float step = 360.0/size;
		float step_half = 0; if(cTable[g].flatcircle==1) step_half = step/2;
		for (int i=0,j=0; i<size; i++)
		{
			gvector Vec1(-1,0,0);
			gvector Vec2(1,0,0);
			Vec1.rotate(0,0,(-step*i)+step_half);
			Vec2.rotate(0,0,(-step*(i+1))+step_half);
			
			List[j] = Vec1;
			List[j+1] = Vec2;
			
			#if DEBUG > 0
			if(dev) cout << " Sec " <<i<< " List #"<<j<<" "<< List[j] << " | " << List[j+1] << endl;
			#endif
			
			j+=2;
		}
	}
	// Grid Circle
	else if (cTable[g].type==1)
	{
	}
	// Custom Spline Simple
	else if (cTable[g].type==2)
	{
	}
	// Custom Spline Intersect
	else if (cTable[g].type==3)
	{
	}
	
	#if DEBUG > 0
	if(dev) {
		cout << " Final Intersection Plane List: " << endl;
		for(int i=0; i<List.size(); i+=2)
			cout << " #" <<i << " "<< List[i] << " \t(Yaw " << GetVecAlign(List[i],0) << ") \t| " << List[i+1] << " \t(Yaw " << GetVecAlign(List[i+1],0) << ") " << endl;
		system("pause");
	}
	#endif
}

void group::CarveGroupSections()
{
	#if DEBUG > 0
	bool dev = 0;
	#endif
	
	group &Group = *this;
	int g = Group.gID;
	
	// Brushes
	#if DEBUG > 0
	if(dev) cout << endl << "###### GROUP Carving Brushes..." << endl;
	#endif
	
	for(int b=0; b<t_brushes; b++)
	{
		brush &Brush = Brushes[b];
		int sec = Brush.SecID;
		if (bGroup[g].IsSecInRange(sec) && Brush.draw)
		{
			#if DEBUG > 0
			if(dev) cout << "###### GROUP Brush " << b << " sec " << sec << " Plane #" << sec*2 << " " << SecIsect[sec*2] << endl;
			#endif
			
			Brush.CarveBrush(SecIsect[sec*2]);
			
			// in case first carving was successful, do the second
			if(Brush.draw)
			{
				#if DEBUG > 0
				if(dev) cout << "###### GROUP Brush " << b << " sec " << sec << " Plane #" << (sec*2)+1 << " " << SecIsect[(sec*2)+1] << endl;
				#endif
				
				Brush.CarveBrush(SecIsect[(sec*2)+1]);
			}
		}
	}
	
	// entities
	for(int e=0; e<t_ents; e++)
	{
		entity &Entity = Entities[e];
		int sec = Entity.SecID;
		
		Entity.CarveEntity(SecIsect[sec*2]);
		
		if(Entity.draw) // in case first carving was successful, do the second
		Entity.CarveEntity(SecIsect[(sec*2)+1]);
	}
	#if DEBUG > 0
	if(dev) { cout << "###### GROUP END " << endl<<endl; system("pause"); }
	#endif
}

// create height Table for smooth ramp generation
void group::CreateHeightTable()
{
	#if DEBUG > 0
	bool dev = 0;
	#endif
	
	group &Group = *this;
	int g = Group.gID;
	
	// create GapList
	Group.CreateGapList();
	
	int res = cTable[gID].res; 			// |<-------res------>|
	int start = range_start;   			// |  rs<------->     |
	int end = range_end;       			// |    <------->re   |
	int range = range_end-range_start; 	// |    <-range->     |
	
	vector<float> &List = heightTable; List.resize(res+1);
	vector<float> &ListRel = heightTableSteps; ListRel.resize(res+1);
	vector<float> Slope;
	vector<float> Steps;
	vector<float> Lengths;
	//if (cTable[g].type==2)
	//Lengths.resize( gFile->PathList[g].t_corners-gFile->PathList[g].t_paths );
	//else
	Lengths.resize(res+1);
	
	// create Section Length-List for Type 2/3 to get accurate height for each step
	if (cTable[g].type>1)
	{
		path_set &Paths = gFile->PathList[g];

		for (int p=0, cc=0; p<Paths.t_paths; p++)
		{
			path Path = Paths.Paths[p];
			for (int c=0; c<Path.t_corners; c++)
			{
				path_corner &Corner = Path.Corners[c];

				if (cc==0)
					Lengths[cc] = Corner.length;
				else
					Lengths[cc] = Corner.length+Lengths[cc-1];
				
				//#if DEBUG > 0
				//cout << " Lengths["<<cc<<"] " << Lengths[cc] << endl;
				//#endif
				
				cc++;
			}
		}
	}
	Lengths.insert(Lengths.begin(), 0.0);
	
	#if DEBUG > 0
	if(dev) {
	for(int i=0; i<Lengths.size(); i++)
		cout << " Lengths #"<< i << " " << Lengths[i] << endl;
	system("pause");
	}
	#endif
	
	// 0 = Linear, 1 = Spline, 2 = Random Jagged, >=3 = Easings
	if (cTable[g].heightmode==0||(cTable[g].path=="UNSET"&&cTable[g].heightmode==1))
	{
		if (cTable[g].heightmode==1) {
			cout << "|    [WARNING] Heightmode set to \"Spline\", but no valid spline file found!" << endl;
			cout << "|              Using linear heightmode instead..."<<endl;
			cTable[g].heightmode=0;
		}
		CreateSlopeLinear(cTable[g].height, cTable[g].res, Slope, Lengths, g);
	}
	else if (cTable[g].heightmode>=3)
	{
		CreateSlopeEasings(cTable[g].height, cTable[g].res, Slope, Lengths, g);
	}
	else if (cTable[g].heightmode==1)
	{
		CreateSlopeSpline(gFile->PathList[g], range, Slope, Steps);
	}
	else if (cTable[g].heightmode==2)
	{
		CreateSlopeRandom(cTable[g].height, cTable[g].res, Slope, g);
	}
	
	// round heights
	if (cTable[g].heightmode!=1)
	for (int i=0; i<Slope.size(); i++)
		Slope[i] = round(Slope[i]);

	#if DEBUG > 0
	if(dev) {
	cout << endl << " Slope List (after rounding) size " << Slope.size() << endl;
	for (int i=0; i<Slope.size(); i++)
		cout << " Slope["<<i<<"] " << Slope[i] << endl; system("pause");}
	#endif
	
	// create Heightlist and StepList
	int off=0; if(cTable[g].heightmode!=1) off=1;
	for (int i=0, j=0; i<res+off; i++)
	{
		#if DEBUG > 0
		if(dev) cout << " List #" << i << "/"<<res<< " start-offset " << start << " Heightlist #" << j+1 << "/"<<Slope.size()<<" Gap " << GapList[i] << endl;
		#endif
		
		List[i] = 0;
		ListRel[i] = 0;
		//if (i>=start)
		{
			if (cTable[g].heightmode==1)
			{
				List[i] = Slope[j];
				ListRel[i] = Steps[j];
				j++;
			}
			else
			{
				if (!GapList[i])
				{
					List[i] = Slope[j];
					ListRel[i] = Slope[j+1]-Slope[j];
					j++;
				}
				else
				{
					List[i] = Slope[j];
					ListRel[i] = 0;
				}
			}
		}
		
		// quick fix invalid numbers (nan inf)
		if ( !IsValid(List[i]) ) List[i] = 0;
		if ( !IsValid(ListRel[i]) ) ListRel[i] = 0;
	}
	
	// if random, connect last with first
	if (cTable[g].heightmode==2)
		ListRel[List.size()-2] = List[0] - List[List.size()-2];  // NOT CORRECT!!!!!! but who cares
	
	#if DEBUG > 0
	if (dev) { cout << " FINAL height table:" << endl; for (int i = 0; i<List.size(); i++) cout << "   #" << i << " " << List[i] << " Rel " << ListRel[i] << endl; system("pause"); }
	#endif
}

// create GapList for Height Slope Generation
void group::CreateGapList()
{
	#if DEBUG > 0
	bool dev = 0;
	#endif
	
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
				i++;
				
				#if DEBUG > 0
				if (dev) cout << " GapList["<<i<<"] " << Group.GapList[i] << endl;
				#endif
			}
		}
	}
	
	#if DEBUG > 0
	if (dev)
	for (int i=0; i<Group.GapList.size(); i++) {
		cout << " GapList["<<i<<"] " << Group.GapList[i] << endl;
	}
	#endif
}

bool group::CheckForSlopes()
{
	#if DEBUG > 0
	bool dev = 0;
	#endif
	
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
	#if DEBUG > 0
	bool dev = 0;
	#endif
	
	group &Group = *this;
	int g = Group.gID;
	group &SrcGroup = sGroup[g];
	
	bool SHOW_LINES = 0;
	
	#if DEBUG > 0
	if (dev)cout << " Building "<<Group.t_brushes<<" brushes | res " << cTable[Group.gID].res << " | sections " << Group.sections << " | segments " << Group.segments << endl;
	#endif
	
	for (int b=0; b<Group.t_brushes; b++)
	{
		brush &Brush = Group.Brushes[b];
		int seg = Brush.SegID;
		int sec = Brush.SecID;
		brush &SrcBrush = SrcGroup.Brushes[seg];
		if (SrcBrush.valid)
		{
			#if DEBUG > 0
			if (dev)cout << "  Entering Brush #" << b+1 << " | Faces: " << Brush.t_faces << " | sec #" << sec+1 << "/"<<Group.sections<<" | seg #" << seg+1<<"/"<<Group.segments << endl;
			#endif
			
			// circle vertex iterator
			int cIDa = sec; if (cTable[g].type==2||cTable[g].type==3) cIDa=sec*2;
			int cIDb = cIDa+1;
			Brush.step 		= SrcBrush.cset->c[0].Vertices[cIDa].step;
			Brush.pID 		= SrcBrush.cset->c[0].Vertices[cIDa].pID;
			Brush.Align 	= SrcBrush.cset->c[0].Vertices[cIDa].Align;
			Brush.DoSplit 	= SrcBrush.cset->c[0].Vertices[cIDa].DoTri;
			Brush.IsGap 	= SrcBrush.cset->c[0].Vertices[cIDa].IsGap;
			Brush.Yaw 		= SrcBrush.cset->c[0].Vertices[cIDa].Yaw;
			Brush.Pitch 	= SrcBrush.cset->c[0].Vertices[cIDa].Pitch;
			Brush.IsCCW 	= SrcBrush.cset->c[0].Vertices[cIDa].IsCCW;
			if (!SrcBrush.valid) Brush.valid=0;
			
			#if DEBUG > 0
			if (dev) {
				cout << "  cIDa: " << cIDa << "(sec(*2 if type=2/3))" << endl;
				cout << "  cIDb: " << cIDb << "(sec+1)" << endl;
				cout << "  Step: " << Brush.step << endl;
				cout << "  pID: " << Brush.pID << endl;
				cout << "  Align: " << Brush.Align << endl;
				cout << "  DoSplit: " << Brush.DoSplit << endl;
				cout << "  IsGap: " << Brush.IsGap << endl << endl;
				cout << "  Contruction Frame: " << endl;
				system("pause");
				cout << SrcBrush.cset[seg] << endl; }
			#endif
			
			for (int f = 0, c=0; f<Brush.t_faces; f++)
			{
				#if DEBUG > 0
				if (dev)cout << "    Entering Face #" << f+1 << "..." << endl;
				#endif
				
				face &Face  = Brush.Faces[f];
				
				if (f==0||f==1) // if base/head Face
				{
					#if DEBUG > 0
					if (dev)cout << "        SrcBrush BaseID " << SrcBrush.BaseID << " SrcBrush HeadID " << SrcBrush.HeadID << endl;
					#endif
					
					face &BaseSrc = SrcBrush.Faces[SrcBrush.BaseID];
					face &HeadSrc = SrcBrush.Faces[SrcBrush.HeadID];
					
					#if DEBUG > 0
					if (dev)cout << "        Copying Base and Head Faces...";
					#endif
					
					if 		(f==0) Face.CopyFace(BaseSrc,0);
					else if (f==1) Face.CopyFace(HeadSrc,0);
					
					#if DEBUG > 0
					if (dev)cout << "Done!" << endl;
					#endif
					
					for (int v = 0; v<Face.vcount; v++)
					{
						// Base Face
						if (f==0)
						{
							#if DEBUG > 0
							if (dev)cout << "        Generating Vertex #" << v+1 << " of Base Face, mG->B["<<seg<<"].cset->c["<<v<<"].V["<<cIDa<<"] :" << SrcGroup.Brushes[seg].cset->c[v].Vertices[cIDa] << endl;
							#endif
							Face.Vertices[v] = SrcGroup.Brushes[seg].cset->c[v].Vertices[cIDa];
						}
						else // Head Face
						{
							#if DEBUG > 0
							if (dev)cout << "        Generating Vertex #" << v+1 << " of Head Face, mG->B["<<seg<<"].cset->c["<<v<<"].V["<<cIDb<<"] :" << SrcGroup.Brushes[seg].cset->c[v].Vertices[cIDb] << endl;
							#endif
							
							if (cIDa==Group.sections-1&&cTable[g].type!=2&&cTable[g].type!=3) cIDb=0; else cIDb=cIDa+1;
							Face.Vertices[v] = SrcGroup.Brushes[seg].cset->c[v].Vertices[cIDb];
						}
					}
					if (f==0) Face.RevOrder(0);
				}
				else // if Body Face
				{
					#if DEBUG > 0
					if (dev)cout << "        Body Face " << f << " Brush " << b << endl;
					#endif
					
					face &BodySrc = SrcBrush.Faces[SrcBrush.cset->c[c].SrcFace];
					Face.CopyFace(BodySrc,0);

					#if DEBUG > 0
					if (dev)cout << "        Tex Offsets PRE X " << Face.OffsetX << " Y " << Face.OffsetY << " BaseVertexX " << Face.BaseX << " BaseVertexY " << Face.BaseY << " VecX "<<Face.VecX<<" VecY "<<Face.VecY<<" Centroid " << Face.Centroid << endl;
					if (dev)cout << "          copying Bodysource... Facealign now: " << Face.FaceAlign  << endl;
					#endif
					
					int cb;
					if (c==SrcGroup.Brushes[seg].cset->tcircs-1) cb=0; else cb=c+1;
					
					vertex &V0 	= SrcGroup.Brushes[seg].cset->c[c].Vertices[cIDa];
					vertex &V1 	= SrcGroup.Brushes[seg].cset->c[cb].Vertices[cIDa];
					vertex &V2 	= SrcGroup.Brushes[seg].cset->c[cb].Vertices[cIDb];
					vertex &V3 	= SrcGroup.Brushes[seg].cset->c[c].Vertices[cIDb];
					bool W01 = 0; if (CompareVerticesR(V0,V1)) W01 = 1;
					bool W03 = 0; if (CompareVerticesR(V0,V3)) W03 = 1;
					bool W12 = 0; if (CompareVerticesR(V1,V2)) W12 = 1;
					
					if (V0.x==V1.x&&V0.y==V1.y&&V0.x==V2.x&&V0.y==V2.y)
					{
						#if DEBUG > 0
						if (dev)cout << "          INVALID" << endl;
						#endif
						
						Face.vcount = 0;
						Face.draw = 0;
					}
					else if (W01)
					{
						#if DEBUG > 0
						if (dev)cout << "          WEDGE V0/V1" << endl;
						#endif
						
						Face.Vertices[0] = V0;
						Face.Vertices[1] = V2;
						Face.Vertices[2] = V3;
						Face.SetEdges(1,2,1,0,1,0);
						Face.vcount = 3;
						Brush.IsWedge = 1;
					}
					else if (W03)
					{
						#if DEBUG > 0
						if (dev)cout << "          WEDGE V0/V3" << endl;
						#endif
						
						Face.Vertices[0] = V0;
						Face.Vertices[1] = V1;
						Face.Vertices[2] = V2;
						Face.SetEdges(1,2,1,0,1,0);
						Face.vcount = 3;
						Brush.IsWedge = 1;
					}
					else if (W12)
					{
						#if DEBUG > 0
						if (dev)cout << "          WEDGE V1/V2" << endl;
						#endif
						
						Face.Vertices[0] = V0;
						Face.Vertices[1] = V1;
						Face.Vertices[2] = V3;
						Face.SetEdges(0,2,1,0,1,0);
						Face.IsWedgeDown = 1;
						Face.vcount = 3;
						Brush.IsWedge = 1;
					}
					else
					{
						#if DEBUG > 0
						if (dev)cout << "          QUAD" << endl;
						#endif
						
						Face.Vertices[0] = V0;
						Face.Vertices[1] = V1;
						Face.Vertices[2] = V2;
						Face.Vertices[3] = V3;
						Face.SetEdges(1,2,1,0,1,3);
					}
					
					Face.GetNormal();
					c++;
					
					#if DEBUG > 0
					if (dev) cout << "        Face Normal " << Face.Normal << endl;
					#endif
				}
				Face.Mother = &Brush;
			}
			#if DEBUG > 0
			if (dev) system("pause");
			#endif
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
	
	if (Group.t_brushes>0&&Group.t_brushes-Group.invalids<=0) {
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
	#if DEBUG > 0
	bool dev = 0;
	if(dev) cout << " copying group ..." << endl;
	#endif
	
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

	#if DEBUG > 0
	if(dev) cout << " created "<< t_brushes << " new brushes!" << endl;
	#endif

	d_pos		= Source.d_pos;
	d_autopitch	= Source.d_autopitch;
	d_autoyaw	= Source.d_autoyaw;
	d_enable	= Source.d_enable;
	d_separate	= Source.d_separate;
	d_autoname	= Source.d_autoname;
	d_draw		= Source.d_draw;
	d_skip		= Source.d_skip;
	d_draw_rand	= Source.d_draw_rand;
	d_pos_rand	= Source.d_pos_rand;
	d_rotz_rand = Source.d_rotz_rand;
	d_movey_rand= Source.d_movey_rand;
	d_scale_rand= Source.d_scale_rand;
	d_carve		= Source.d_carve;
	d_circlemode= Source.d_circlemode;
	
	for (int b=0, s=0; b<Source.t_brushes; b++)
	{
		brush &SrcBrush = Source.Brushes[b];
		brush &Brush = Brushes[s];
		if (SrcBrush.valid)
		{
			#if DEBUG > 0
			if(dev) cout << " Creating Brush #" << s << " by copying source Brush #" << b << endl;
			#endif
			
			Brush.Copy(SrcBrush);
			s++;
		}
	}
	
	#if DEBUG > 0
	if(dev) cout << " Finished copying!" << endl;
	#endif
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
	d_scale_rand= Source.d_scale_rand;
	d_carve		= Source.d_carve;
	d_circlemode= Source.d_circlemode;
}

void group::GetGroupOrigin()
{
	dimensions &D = Dimensions;
	Origin.x = D.xb-((D.xb-D.xs)/2);
	Origin.y = D.yb-((D.yb-D.ys)/2);
	Origin.z = D.zb-((D.zb-D.zs)/2);
}

void group_set::GetGroupSetDimensions(bool Overwrite)
{
	#if DEBUG > 0
	bool dev = 0;
	if(dev) cout << endl << " Getting Group SET Dimensions..." << endl;
	#endif
	
	bool setBox = 0;
	dimensions &DS = Dimensions;
	for(int d=0; d<t_groups; d++)
	{
		group &dGroup = Groups[d];
		
		dGroup.GetGroupDimensions(Overwrite,0);
		dimensions &D = dGroup.Dimensions;
		
		if (!setBox)
		{
			DS.set(D.xs,D.xb,D.ys,D.yb,D.zs,D.zb); setBox = 1;
			
			#if DEBUG > 0
			if(dev) cout << " No dimensions, adding first " << D << endl;
			#endif
		}
		else // set initial dimensions to first group vertex per default
		{
			if (D.xs < DS.xs)
			{
				DS.xs = D.xs;
				#if DEBUG > 0
				if(dev) cout << " new s X: " << DS.xs << " DS.xs " << DS.xs << endl;
				#endif
			}
			if (D.xb > DS.xb) 
			{
				DS.xb = D.xb;
				#if DEBUG > 0
				if(dev) cout << " new b X: " << DS.xb << " DS.xb " << DS.xb << endl;
				#endif
			}
			if (D.ys < DS.ys) 
			{
				DS.ys = D.ys;
				#if DEBUG > 0
				if(dev) cout << " new s Y: " << DS.ys << " DS.ys " << DS.ys << endl;
				#endif
			}
			if (D.yb > DS.yb) 
			{
				DS.yb = D.yb;
				#if DEBUG > 0
				if(dev) cout << " new b Y: " << DS.yb << " DS.yb " << DS.yb << endl;
				#endif
			}
			if (D.zs < DS.zs) 
			{
				DS.zs = D.zs;
				#if DEBUG > 0
				if(dev) cout << " new s Z: " << DS.zs << " DS.zs " << DS.zs << endl;
				#endif
			}
			if (D.zb > DS.zb) 
			{
				DS.zb = D.zb;
				#if DEBUG > 0
				if(dev) cout << " new b Z: " << DS.zb << " DS.zb " << DS.zb << endl;
				#endif
			}
		}
	}
	#if DEBUG > 0
	if(dev) cout << endl;
	#endif
}

void group::GetGroupDimensions(bool Overwrite, bool CustomOrigin)
{
	#if DEBUG > 0
	bool dev = 0;
	if(dev) cout << endl << " Getting GROUP Dimensions..." << endl;
	#endif
	
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
						
						if (!setBox)
						{
							D.set(V.x,V.y,V.z);
							setBox = 1;
							
							#if DEBUG > 0
							if(dev) cout << " No dimensions, adding first " << V << endl;
							#endif
						}
						else // set initial dimensions to first group vertex per default
						{
							if (V.x < D.xs)
							{
								D.xs = V.x;
								
								#if DEBUG > 0
								if(dev) cout << " new s X: " << D.xs << " v " << v << V << endl;
								#endif
							}
							if (V.x > D.xb)
							{
								D.xb = V.x;
								
								#if DEBUG > 0
								if(dev) cout << " new b X: " << D.xb << " v " << v << V << endl;
								#endif
							}
							if (V.y < D.ys)
							{
								D.ys = V.y;

								#if DEBUG > 0
								if(dev) cout << " new s Y: " << D.ys << " v " << v << V << endl;
								#endif
							}
							if (V.y > D.yb)
							{
								D.yb = V.y;

								#if DEBUG > 0
								if(dev) cout << " new b Y: " << D.yb << " v " << v << V << endl;
								#endif
							}
							if (V.z < D.zs)
							{
								D.zs = V.z;

								#if DEBUG > 0
								if(dev) cout << " new s Z: " << D.zs << " v " << v << V << endl;
								#endif
							}
							if (V.z > D.zb)
							{
								D.zb = V.z;
								
								#if DEBUG > 0
								if(dev) cout << " new b Z: " << D.zb << " v " << v << V << endl;
								#endif
							}
						}
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
	if(CustomOrigin&&HasOrigin)
	{
		GetGroupOriginCustom();
	}
	else
	{
		Origin.x = D.xb-((D.xb-D.xs)/2);
		Origin.y = D.yb-((D.yb-D.ys)/2);
		Origin.z = D.zb-((D.zb-D.zs)/2);
	}
	SizeY = D.yb-D.ys;
	SizeZ = D.zb-D.zs;
	if (IsSrcMap) IsSrcMap = 0;
	
	#if DEBUG > 0
	if(dev) cout << Dimensions << endl;
	if(dev) cout << " Origin of this object " << Origin << endl << endl;
	#endif
}
	

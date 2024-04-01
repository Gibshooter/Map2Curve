#include "brush.h"
#include "face.h"
#include "settings.h"
#include "vertex.h"
#include "dimensions.h"
#include "LSE.h"
#include "file.h"

#include <string>
#include <iomanip> // precision
#include <fstream>

#define DEBUG 0

using namespace std;

struct face;
struct circleset;
struct vertex;
struct dimensions;
struct LSE;

extern ctable *cTable;
extern float def_spikesize;
extern string def_nulltex;
extern file *gFile;

/* ===== BRUSH METHODS ===== */

brush::~brush()
{
	delete[] Faces;
	delete[] vlist;
	if(Tri!=nullptr) delete[] Tri;
	if(Gap!=nullptr) delete Gap;
	if(cset!=nullptr) delete cset;
}

brush::brush(int tf, int tv)
{
	t_faces = tf;
	Faces = new face[tf];
	
	for (int f = 0; f<tf; f++) {
		face &Face = Faces[f];
		for (int v = 0; v<tv; v++) {
			Face.vcount = tv;
			Face.Vertices = new vertex[tv];
		}
	}
}

void brush::GetBrushDimensions(bool Overwrite)
{
	brush &Brush = *this;
	bool IsSet = 0;
	if (( Brush.draw && Brush.valid ) || Overwrite )
	{
		for (int f = 0; f<Brush.t_faces; f++)
		{
			face &Face = Brush.Faces[f];
			if (Face.draw)
			{
				for (int v = 0; v<Face.vcount; v++)
				{
					vertex &V = Face.Vertices[v];
					
					if (!IsSet) { D.set(V.x,V.y,V.z); IsSet = 1; } // set initial dimensions to first group vertex per default
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
	Origin.x = D.xb-((D.xb-D.xs)/2);
	Origin.y = D.yb-((D.yb-D.ys)/2);
	Origin.z = D.zb-((D.zb-D.zs)/2);
}

bool brush::IsOriginBrush()
{
	brush &B = *this;
	int OTex=0;
	for (int f=0; f<B.t_faces; f++)
	{
		face &F = Faces[f];
		if(F.Texture=="ORIGIN")
		OTex++;
	}
	if(OTex==B.t_faces) { IsOrigin = 1; return 1; }
	else return 0;
}

void brush::CarveBrush(gvector Plane)
{
	brush &Brush = *this;
	
	#if DEBUG > 0
	bool dev = 0;
	if(dev) cout << endl << "BRUSH Carving Brush [" << Brush.name << "] with Plane " << Plane << " (yaw " <<GetVecAlign(Plane,0) <<")" <<endl;
	#endif
	
	bool DoCarve = 0;
	int FacesBeyond = 0;

	#if DEBUG > 0
	if(dev) cout << "BRUSH Checking Face status..." << endl;
	#endif
	
	for(int f=0; f<t_faces; f++)
	{
		face &Face = Faces[f];
		if(Face.draw) {
			int Status = Face.CarveFace(Plane);
			if(Status==2) DoCarve = 1;
			if(Status==1) FacesBeyond++;
			
			#if DEBUG > 0
			if(dev) {cout << "BRUSH Face "<< f<<" "; if(Status==0) cout << " is inside of bounds!"<<endl; else if(Status==1) cout << " is out of bounds!"<<endl; else cout << " has been CARVED!"<<endl;  }
			#endif
		}
	}
	
	#if DEBUG > 0
	if(dev) cout << "BRUSH Faces Beyond: " << FacesBeyond << endl;
	if(dev) system("pause");
	#endif
	
	// since Faces of this brush were carved, create a new face of all the new vertices to fill the hole
	if(DoCarve)
	{
		#if DEBUG > 0
		if(dev) cout << "BRUSH Creating missing face. Faces atm " << t_faces <<"..." << endl;
		#endif
		
		// check which vertices are on the cutting plane
		vector<vertex> V_New;
		for(int f=0; f<t_faces; f++)
		{
			face &Face = Faces[f];
			if(Face.draw)
			for(int v=0; v<Face.vcount; v++)
			{
				vertex &V = Face.Vertices[v];
				if(IsVertexOnPlane(Plane, V, 2))
					V_New.push_back(V);
			}
		}
		
		#if DEBUG > 0
		if(dev) cout << "BRUSH   Vertices on Plane: "  << V_New.size() << endl;
		#endif
		
		// get rid of double vertices
		vector<vertex> V_Unique;
		for(int v=0; v<V_New.size(); v++)
		{
			vertex &S = V_New[v];
			if(  !IsVertexInList(S, V_Unique, 1, 1)  )
				V_Unique.push_back(S);
		}
		
		#if DEBUG > 0
		if(dev) cout << "BRUSH   Unique new Vertices: "  << V_Unique.size() << endl;
		#endif
		
		// create new Face
		face Filler;
		Filler.vcount = V_Unique.size();
		Filler.Vertices = new vertex[Filler.vcount];
		for(int v=0; v<Filler.vcount; v++)
		{
			vertex &S = V_Unique[v];
			vertex &T = Filler.Vertices[v];
			T = S;
			T.carved = 1;
		}
		Filler.SortVertices(Plane);
		
		#if DEBUG > 0
		if(dev) for(int v=0; v<Filler.vcount; v++) cout << "BRUSH   Final Vertex #" << v  << Filler.Vertices[v] << endl;
		#endif
		
		Filler.Normal = Plane;
		
		#if DEBUG > 0
		if(dev) cout << "BRUSH   Filler normal " << Filler.Normal << endl;
		#endif
		
		Filler.Texture = def_nulltex;
		AlignToWorld(Filler);
		
		// add new face to brush
		face *F_New = new face[t_faces+1];
		for(int f=0; f<t_faces; f++)
		{
			face &Target = F_New[f];
			face &Source = Faces[f];
			Target.CopyFace(Source,1);
		}
		F_New[t_faces].CopyFace(Filler, 1);
		delete[] Faces;
		Faces = F_New;
		t_faces++;
		
		#if DEBUG > 0
		if(dev) cout << "BRUSH   Faces now "  << t_faces << endl;
		#endif
		
		// fix borderliner (e.g. 15.9999 = 16.0)
		for(int f=0; f<t_faces; f++)
		{
			face &Face = Faces[f];
			for(int v=0; v<Face.vcount; v++)
			{
				vertex &V = Face.Vertices[v];
				if(IsBorderliner(V.x,2)==1) V.x = round(V.x);
				if(IsBorderliner(V.y,2)==1) V.y = round(V.y);
				if(IsBorderliner(V.z,2)==1) V.z = round(V.z);
			}
		}
		
		// care for identical vertex X/Y coordinates on different heights (X 1 Y 1)!=(X 1.001 Y 0.999) if there are differences
		vector<vertex> agents;
		for(int f=0; f<t_faces; f++)
		{
			face &Face = Faces[f];
			for(int v=0; v<Face.vcount; v++)
			{
				vertex &V = Face.Vertices[v];
				if(V.carved && !IsVertexXYInList(V, agents, 1, 2)) // only add carved vertices and those which arent already in there
					agents.push_back(V);
			}
		}
		
		// compare agents with all other carved vertices
		for(int f=0; f<t_faces; f++)
		{
			face &Face = Faces[f];
			for(int v=0; v<Face.vcount; v++)
			{
				vertex &V = Face.Vertices[v];
				for(int a=0; a<agents.size(); a++)
				{
					vertex &A = agents[a];
					if(V.carved) // only check carved vertices
					{
						if( !CompareVerticesXY(V,A) ) // precise difference
						if( CompareVerticesXYDeci(V,A, 2) ) // lower precision difference
						{
							// Weld the vertex to its Agent
							
							#if DEBUG > 0
							if(dev) cout << setprecision(8) << " welding carved vertex " << v << V << " to agent " << a << A << endl;
							#endif
							
							V.x = A.x;
							V.y = A.y;
						}
					}
				}
			}
		}
	}
	// all faces of this brush are beyond plane. Brush is completely out of bound and can therefor be discarded!
	else if(!DoCarve&&FacesBeyond==t_faces)
	{
		#if DEBUG > 0
		if(dev) cout << "BRUSH All faces of this brush are beyond plane!" << endl;
		#endif
		
		Brush.draw = 0;
	}
	
	#if DEBUG > 0
	if(dev) { cout << "BRUSH END" << endl<<endl; system("pause"); }
	#endif
}

void brush::Scale(float n)
{
	brush &Brush = *this;
	if (n!=0)
	for (int f = 0; f<Brush.t_faces; f++)
	{
		face &Face = Brush.Faces[f];
		if (Face.draw)
		{
			if(n<0) Face.RevOrder(0);
			for (int v = 0; v<Face.vcount; v++)
			{
				vertex &V = Face.Vertices[v];
				
				V.x*=n;
				V.y*=n;
				V.z*=n;
			}
			Face.ScaleX *= n;
			Face.ScaleY *= n;
		}
	}
}

void brush::ScaleOrigin(float n, vertex Origin, int g)
{
	if (n!=0)
	{
		this->Move(-Origin.x, -Origin.y, -Origin.z,1,g);
		
		for (int f = 0; f<t_faces; f++)
		{
			face &Face = Faces[f];
			if (Face.draw)
			{
				for (int v = 0; v<Face.vcount; v++)
				{
					vertex &V = Face.Vertices[v];
					
					V.scale(n);
				}
				Face.Centroid.scale(n);
				Face.ScaleX *= n;
				Face.ScaleY *= n;
			}
		}
		
		this->Move(Origin.x, Origin.y, Origin.z,1,g);
	}
}

void brush::Move(float x, float y, float z, bool fixShifts, int g)
{
	#if DEBUG > 0
	bool dev = 0;
	#endif
	
	//bool UseLongEdge = cTable[g].hshiftsrc;
	
	if(x!=0||y!=0||z!=0)
	for (int f = 0; f<t_faces; f++)
	{
		face &Face = Faces[f];
		if (Face.draw)
		{
			#if DEBUG > 0
			if (dev) cout << " Attempt to move Face " << f << " Tex " << Face.Texture << " draw " << Face.draw << endl;
			#endif
			
			if (fixShifts)
			{
				if (Face.BaseListX[0]==0&&Face.BaseListX[3]==0)
				{GetBaseEdges(Face);}
				GetBaseShift(Face,0,1,0);
				GetTexOffset(Face,0);
			}
			for (int v = 0; v<Face.vcount; v++)
			{
				vertex &V = Face.Vertices[v];
				
				#if DEBUG > 0
				if (dev) cout << "    Moving vertex " << v << V << " by " << x << ", " << y << ", " << z << endl;
				#endif
				
				V.move(x,y,z);
			}
			
			if (fixShifts)
			{
				GetBaseShift(Face,0,1,0);
				Face.ShiftX = Face.BaseShiftX + Face.OffsetX;
				Face.ShiftY = Face.BaseShiftY + Face.OffsetY;
			}
		}
	}
}

void brush::Rot(float x, float y, float z)
{
	if(x!=0||y!=0||z!=0)
	for (int f = 0; f<t_faces; f++)
	{
		face &Face = Faces[f];
		if (Face.draw)
		{
			for (int v = 0; v<Face.vcount; v++)
			{
				vertex &V = Face.Vertices[v];
				
				V.rotate(x,y,z);
			}
			Face.VecX.rotate(x,y,z);
			Face.VecY.rotate(x,y,z);
		}
	}
}

void brush::RotOrigin(float x, float y, float z, vertex Origin, int g)
{
	#if DEBUG > 0
	bool dev = 0;
	#endif
	
	//bool UseLongEdge = cTable[g].hshiftsrc;
	
	if(x!=0||y!=0||z!=0)
	for (int f = 0; f<t_faces; f++)
	{
		face &Face = Faces[f];
		if (Face.draw)
		{
			#if DEBUG > 0
			if (dev) cout << " Face " << f << " Tex " << Face.Texture << " Fixing Shift after Rotation..." << endl;
			if (dev) cout << "   Old BaseShiftX " << Face.BaseShiftX << endl;
			if (dev) cout << "   Old BaseShiftY " << Face.BaseShiftY << endl;
			if (dev) cout << "   Rotating Vertices... ("<<Face.vcount<<")" << endl;
			#endif
			
			for (int v = 0; v<Face.vcount; v++){
				vertex &V = Face.Vertices[v];
				
				V.rotateOrigin(x,y,z, Origin);
			}
			
			#if DEBUG > 0
			if (dev) cout << "   Rotating VerticesC... ("<<Face.vcountC<<")" << endl;
			#endif
			
			for (int v = 0; v<Face.vcountC; v++)
			{
				vertex &VC = Face.VerticesC[v];
				VC.rotateOrigin(x,y,z, Origin);
			}
			
			#if DEBUG > 0
			if (dev) cout << "   Rotating Centroid... " << Face.Centroid << endl;
			#endif
			
			Face.Centroid.rotateOrigin(x,y,z, Origin);
			
			#if DEBUG > 0
			if (dev) cout << "   Centroid now " << Face.Centroid << endl;
			if (dev) cout << "   Rotating Tex Vectors... " << endl;
			#endif
			
			Face.VecX.rotate(x,y,z);
			Face.VecY.rotate(x,y,z);
			
			#if DEBUG > 0
			if (dev) cout << "   Getting Edges and BaseShift... " << endl;
			#endif
			
			GetBaseShift(Face,0,1,0);

			#if DEBUG > 0
			if (dev) cout << "   New BaseShiftX " << Face.BaseShiftX << endl;
			if (dev) cout << "   New BaseShiftY " << Face.BaseShiftY << endl;
			if (dev) cout << "   OffsetX " << Face.OffsetX << endl;
			if (dev) cout << "   OffsetY " << Face.OffsetY << endl;
			#endif
			
			Face.ShiftX = Face.BaseShiftX + Face.OffsetX;
			Face.ShiftY = Face.BaseShiftY + Face.OffsetY;

			#if DEBUG > 0
			if (dev) cout << "   New ShiftX " << Face.ShiftX << endl;
			if (dev) cout << "   New ShiftY " << Face.ShiftY << endl;
			#endif
		}
	}
}


void brush::Copy(brush &Source)
{
	t_faces = Source.t_faces;
	SecID 	= Source.SecID;
	SegID 	= Source.SegID;
	BaseID 	= Source.BaseID;
	HeadID 	= Source.HeadID;
	valid 	= Source.valid;
	IsDivisible = Source.IsDivisible;
	IsEnt 	= Source.IsEnt;
	entID 	= Source.entID;
	Faces 	= new face[t_faces];
	draw 	= Source.draw;
	IsWedge = Source.IsWedge;
	DoSplit	= Source.DoSplit;
	IsGap	= Source.IsGap;
	step 	= Source.step;
	pID 	= Source.pID;
	Align	= Source.Align;
	name	= Source.name + "_copy";
	IsSpike	= Source.IsSpike;
	RCON	= Source.RCON;
	gID		= Source.gID;
	dID		= Source.dID;
	entID 	= Source.entID;
	bID		= Source.bID;
	IsOrigin= Source.IsOrigin;
	
	vlist = new int[t_faces-2];
	if (Source.vlist!=nullptr)
	for (int i = 0; i < t_faces-2; i++) {
		vlist[i] = Source.vlist[i]; //cout << " copied vList [" << i << "] " << vlist[i] << " Source " << Source.vlist[i] << " Face Vertex " << endl;
	}
	else
	for (int i = 0; i < t_faces-2; i++)
		vlist[i] = -1;
	
	if (Source.Gap!=nullptr) {
		Gap = new brush;
		Gap->CopySimple(*Source.Gap);
	}
	
	for (int f = 0; f<t_faces; f++)
	{
		face &OFace = Source.Faces[f];
		face &Face = Faces[f];
		
		Face.vcount 	= OFace.vcount;
		Face.vcountC 	= OFace.vcountC;
		Face.Texture 	= OFace.Texture;
		Face.ShiftX 	= OFace.ShiftX;
		Face.ShiftY 	= OFace.ShiftY;
		Face.Rot 		= OFace.Rot;
		Face.ScaleX 	= OFace.ScaleX;
		Face.ScaleY 	= OFace.ScaleY;
		Face.VecX 		= OFace.VecX;
		Face.VecY 		= OFace.VecY;
		if (!Face.VecX.IsHor) {
		Face.VecH = &Face.VecY;
		Face.VecV = &Face.VecX; }
		Face.fID 		= OFace.fID;
		Face.FaceAlign 	= OFace.FaceAlign;
		Face.OffsetX 	= OFace.OffsetX;
		Face.OffsetY 	= OFace.OffsetY;
		Face.BaseX		= OFace.BaseX;
		Face.BaseY		= OFace.BaseY;
		Face.BaseX2		= OFace.BaseX2;
		Face.BaseY2		= OFace.BaseY2;
		Face.tID		= OFace.tID;
		Face.Orient 	= OFace.Orient;
		Face.Vertices 	= new vertex[Face.vcount];
		Face.VerticesC 	= new vertex[Face.vcountC];
		Face.draw		= OFace.draw;
		Face.BaseShiftX = OFace.BaseShiftX;
		Face.BaseShiftY = OFace.BaseShiftY;
		Face.Centroid   = OFace.Centroid;
		Face.EdgeH 		= OFace.EdgeH;
		Face.EdgeV 		= OFace.EdgeV;
		for (int i = 0; i<6; i++)
		Face.EdgeIDs[i] = OFace.EdgeIDs[i];
		Face.IsSecBase	= OFace.IsSecBase;
		Face.Hypo		= OFace.Hypo;
		Face.vAngle_s	= OFace.vAngle_s;
		Face.vAngle_b	= OFace.vAngle_b;
		Face.Normal		= OFace.Normal;
		Face.IsPlanar	= OFace.IsPlanar;
		Face.IsNULL		= OFace.IsNULL;
		Face.TentID		= OFace.TentID;
		Face.name		= OFace.name;
		Face.HSourceL	= OFace.HSourceL;
		Face.HSourceS	= OFace.HSourceS;
		Face.Mother		= this;
		Face.PitchO		= OFace.PitchO;
		Face.group		= OFace.group;
		Face.BaseListX.resize(4);
		Face.BaseListY.resize(4);
		Face.LHSourceL = OFace.LHSourceL;
		Face.LHSourceS = OFace.LHSourceS;
		Face.ugroup	= OFace.ugroup;
		for (int i = 0; i<4; i++)
		{
			Face.BaseListX[i] = OFace.BaseListX[i];
			Face.BaseListY[i] = OFace.BaseListY[i];
			//cout << " Face.BaseListX["<<i<<"] " << Face.BaseListX[i]<< endl;
			//cout << " Face.BaseListY["<<i<<"] " << Face.BaseListY[i]<< endl;
		}
		for (int v = 0; v<Face.vcount; v++) {
			vertex &OVert = OFace.Vertices[v];
			vertex &Vert = Face.Vertices[v];
			Vert = OVert;
		}
		for (int v = 0; v<Face.vcountC; v++) {
			vertex &OVertC = OFace.VerticesC[v];
			vertex &VertC = Face.VerticesC[v];
			VertC = OVertC;
		}
	}
}

void brush::CopySimple(brush &Source)
{
	t_faces = Source.t_faces;
	SecID 	= Source.SecID;
	SegID 	= Source.SegID;
	valid 	= Source.valid;
	IsEnt 	= Source.IsEnt;
	entID 	= Source.entID;
	Faces 	= new face[t_faces];
	draw 	= Source.draw;
	IsSpike	= Source.IsSpike;
	RCON	= Source.RCON;
	gID 	= Source.gID;
	dID		= Source.dID;
	bID		= Source.bID;
	IsOrigin= Source.IsOrigin;
	IsWedge = Source.IsWedge;
	
	for (int f = 0; f<t_faces; f++)
	{
		face &OFace = Source.Faces[f];
		face &Face = Faces[f];
		
		Face.vcount 	= OFace.vcount;
		Face.Texture 	= OFace.Texture;
		Face.ShiftX 	= OFace.ShiftX;
		Face.ShiftY 	= OFace.ShiftY;
		Face.Rot 		= OFace.Rot;
		Face.ScaleX 	= OFace.ScaleX;
		Face.ScaleY 	= OFace.ScaleY;
		Face.VecX 		= OFace.VecX;
		Face.VecY 		= OFace.VecY;
		if (!Face.VecX.IsHor) {
		Face.VecH = &Face.VecY;
		Face.VecV = &Face.VecX; }
		Face.fID 		= OFace.fID;
		Face.tID		= OFace.tID;
		Face.Orient 	= OFace.Orient;
		Face.Vertices 	= new vertex[Face.vcount];
		Face.draw		= OFace.draw;
		Face.IsNULL		= OFace.IsNULL;
		for (int i = 0; i<6; i++)
		Face.EdgeIDs[i] = OFace.EdgeIDs[i];
		Face.EdgeH 		= OFace.EdgeH;
		Face.EdgeV 		= OFace.EdgeV;
		Face.Hypo 		= OFace.Hypo;
		Face.TentID		= OFace.TentID;
		Face.HSourceL	= OFace.HSourceL;
		Face.HSourceS	= OFace.HSourceS;
		Face.name		= OFace.name;
		Face.Mother		= this;
		Face.PitchO		= OFace.PitchO;
		Face.group		= OFace.group;
		Face.LHSourceL 	= OFace.LHSourceL;
		Face.LHSourceS 	= OFace.LHSourceS;
		Face.ugroup		= OFace.ugroup;

		for (int v = 0; v<Face.vcount; v++)
		{
			vertex &OVert = OFace.Vertices[v];
			vertex &Vert = Face.Vertices[v];
			Vert = OVert;
		}
	}
}

void brush::MakeCuboid(dimensions D, string Tex)
{
	brush &Brush = *this;
	
	Brush.Faces = new face[6];
	Brush.t_faces = 6;
	for(int f=0; f<6; f++)
	{
		face &Face = Brush.Faces[f];
		Face.Vertices = new vertex[4];
		Face.vcount = 4;
	}
	Faces[0].Vertices[3].setall(D.xs,D.ys,D.zs);
	Faces[0].Vertices[2].setall(D.xs,D.yb,D.zs);
	Faces[0].Vertices[1].setall(D.xb,D.yb,D.zs);
	Faces[0].Vertices[0].setall(D.xb,D.ys,D.zs);
	
	Faces[1].Vertices[0].setall(D.xs,D.yb,D.zb);
	Faces[1].Vertices[1].setall(D.xb,D.yb,D.zb);
	Faces[1].Vertices[2].setall(D.xb,D.ys,D.zb);
	Faces[1].Vertices[3].setall(D.xs,D.ys,D.zb);
	
	Faces[2].Vertices[0].setall(D.xs,D.ys,D.zs);
	Faces[2].Vertices[1].setall(D.xs,D.ys,D.zb);
	Faces[2].Vertices[2].setall(D.xb,D.ys,D.zb);
	Faces[2].Vertices[3].setall(D.xb,D.ys,D.zs);
	
	Faces[3].Vertices[0].setall(D.xb,D.ys,D.zs);
	Faces[3].Vertices[1].setall(D.xb,D.ys,D.zb);
	Faces[3].Vertices[2].setall(D.xb,D.yb,D.zb);
	Faces[3].Vertices[3].setall(D.xb,D.yb,D.zs);
	
	Faces[4].Vertices[0].setall(D.xb,D.yb,D.zs);
	Faces[4].Vertices[1].setall(D.xb,D.yb,D.zb);
	Faces[4].Vertices[2].setall(D.xs,D.yb,D.zb);
	Faces[4].Vertices[3].setall(D.xs,D.yb,D.zs);
	
	Faces[5].Vertices[0].setall(D.xs,D.yb,D.zs);
	Faces[5].Vertices[1].setall(D.xs,D.yb,D.zb);
	Faces[5].Vertices[2].setall(D.xs,D.ys,D.zb);
	Faces[5].Vertices[3].setall(D.xs,D.ys,D.zs);
	
	for (int f = 0; f<6; f++) {
		face &Face = Faces[f];
		Face.Texture = Tex;
		AlignToWorld(Face);
	}
}

brush* MakeBoxHollow(dimensions D, float wall, string Tex)
{
	brush *Hollow = new brush[6];
	
	dimensions D_BrushL(D.xs, D.xs+wall, D.ys+wall, D.yb-wall, D.zs+wall, D.zb-wall);
	dimensions D_BrushR(D.xb-wall, D.xb, D.ys+wall, D.yb-wall, D.zs+wall, D.zb-wall);
	dimensions D_BrushU(D.xs, D.xb, D.ys, D.yb, D.zb-wall, D.zb);
	dimensions D_BrushD(D.xs, D.xb, D.ys, D.yb, D.zs, D.zs+wall);
	dimensions D_BrushF(D.xs, D.xb, D.ys, D.ys+wall, D.zs+wall, D.zb-wall);
	dimensions D_BrushB(D.xs, D.xb, D.yb-wall, D.yb, D.zs+wall, D.zb-wall);
	
	Hollow[0].MakeCuboid(D_BrushL, Tex);
	Hollow[1].MakeCuboid(D_BrushR, Tex);
	Hollow[2].MakeCuboid(D_BrushU, Tex);
	Hollow[3].MakeCuboid(D_BrushD, Tex);
	Hollow[4].MakeCuboid(D_BrushF, Tex);
	Hollow[5].MakeCuboid(D_BrushB, Tex);
	
	return Hollow;
}

void brush::MakeCube(float size, string Tex)
{
	float half = size/2;
	Faces[0].Vertices[2].setall(-half,half,-half);
	Faces[0].Vertices[1].setall(half,half,-half);
	Faces[0].Vertices[0].setall(half,-half,-half);
	
	Faces[1].Vertices[0].setall(-half,half,half);
	Faces[1].Vertices[1].setall(half,half,half);
	Faces[1].Vertices[2].setall(half,-half,half);
	
	Faces[2].Vertices[0].setall(-half,-half,-half);
	Faces[2].Vertices[1].setall(-half,-half,half);
	Faces[2].Vertices[2].setall(half,-half,half);
	
	Faces[3].Vertices[0].setall(half,-half,-half);
	Faces[3].Vertices[1].setall(half,-half,half);
	Faces[3].Vertices[2].setall(half,half,half);
	
	Faces[4].Vertices[0].setall(half,half,-half);
	Faces[4].Vertices[1].setall(half,half,half);
	Faces[4].Vertices[2].setall(-half,half,half);
	
	Faces[5].Vertices[0].setall(-half,half,-half);
	Faces[5].Vertices[1].setall(-half,half,half);
	Faces[5].Vertices[2].setall(-half,-half,half);
	
	for (int f = 0; f<6; f++) {
		face &Face = Faces[f];
		Face.Texture = Tex;
		AlignToWorld(Face);
	}
}

void brush::GetSimpleCentroid()
{
	#if DEBUG > 0
	bool dev = 0;
	#endif
	
	brush &Brush = *this;
	if (Brush.valid)
	{
		float sy=0, hy=0, hz=0, sz=0;
		
		// get centroid for each brush
		for (int f = 0; f<Brush.t_faces; f++)
		{
			face &Face = Brush.Faces[f];
			// get smallest and biggest Y/Z vertex coordinate for each brush
			for (int v = 0; v<3; v++)
			{
				vertex &ev = Face.Vertices[v];

				// smallest/biggest X/Y
				if (f==0&&v==0) {sy=ev.y; hy=ev.y; sz=ev.z; hz=ev.z; }
				else {
					if (ev.y<sy) {sy = ev.y; }
					if (ev.y>hy) {hy = ev.y; }
					if (ev.z<sz) {sz = ev.z; }
					if (ev.z>hz) {hz = ev.z; }
				}
			}
		}
		Brush.centroid.y = hy-((hy-sy)/2);
		Brush.centroid.z = hz-((hz-sz)/2);
		
		#if DEBUG > 0
		if(dev) { cout << "centroid: " << Brush.centroid << ", from hy/sy/hz/sz " << hy << "," << sy << "," << hz << "," << sz << endl; }
		#endif
	}
}

// find smallest and biggest vertex angle of each face
void brush::GetFaceVertexSE()
{
	#if DEBUG > 0
	bool dev = 0;
	if(dev) cout << endl << " Smallest and Biggest Face Vertex Angle..." << endl;
	#endif
	
	brush &Brush = *this;
	
	if (Brush.valid)
	{
		for (int f = 0; f<Brush.t_faces; f++) // get smallest vertex angle of each face and store it in sv.Angle
		{
			face &Face = Brush.Faces[f];
			int &sv_ID = Face.vAngle_s; // vertex ID with smallest vertex angle
			int &sv_ID2 = Face.vAngle_sbak; // backup
			int &bv_ID = Face.vAngle_b; // vertex ID with biggest  vertex angle
			vertex &face_smallest = Face.Vertices[sv_ID]; // vertex that has smallest angle face-wide (at beginning always 0)
			vertex &face_biggest  = Face.Vertices[bv_ID]; // vertex that has biggest  angle face-wide (at beginning always 0)
			
			if (Face.fID==2) // only body faces
			{
				for (int v = 1; v<Face.vcount; v++) // vertex loop // changed from Face.vcount to vcount 3
				{
					vertex &candidate = Face.Vertices[v]; // current candidate vertex
					
					if (candidate.angle < Face.Vertices[sv_ID].angle)
					{
						#if DEBUG > 0
						if(dev) cout << "    Face "<<f<<" tex "<< Face.Texture << " candidate angle (" << round(candidate.angle) << ") is smaller than smallest angle of face (" << round(Face.Vertices[sv_ID].angle) << ")" << endl;
						#endif
						
						sv_ID = v;
						sv_ID2 = v;
					}
					if (candidate.angle > Face.Vertices[bv_ID].angle)
					{
						#if DEBUG > 0
						if(dev) cout << "    Face "<<f<<" tex "<< Face.Texture << " candidate angle (" << round(candidate.angle) << ") is bigger than biggest angle of face (" << round(Face.Vertices[bv_ID].angle) << ")" << endl;
						#endif
						
						bv_ID = v;
					}
					
				}
				
				#if DEBUG > 0
				if(dev) cout << "  smallest vertex of brush " << Brush.SegID << ", face " << f << "\t is: " << sv_ID <<"\t ("<<round(Face.Vertices[sv_ID].angle)<<"),\t biggest: " << bv_ID << "\t (angle " << round(Face.Vertices[bv_ID].angle) << ")" << endl; 
				#endif
			}
		}
		
		#if DEBUG > 0
		if(dev)
		for (int f = 0; f<Brush.t_faces; f++)
		{
			face &Face = Brush.Faces[f];
			if (Face.fID==2)
			cout << "  FINAL: Face " << f << " tex "<< Face.Texture << " smallest vertex: " << Face.vAngle_s << Face.Vertices[Face.vAngle_s] << " biggest vertex: " << Face.vAngle_b << Face.Vertices[Face.vAngle_b] << endl;
		}
		#endif
	}
	
	#if DEBUG > 0
	if(dev) system("pause");
	#endif
}


// find first and last vertex for Brush vertex sorting list
void brush::GetVertexListSE()
{
	#if DEBUG > 0
	bool dev = 0;
	if(dev) cout<< endl << " First and Last Vertex of a whole Brush..." << endl;
	#endif
	
	brush &Brush = *this;
	
	if (Brush.valid)
	{
		int &first_entry = Brush.vlist[0];
		int &last_entry  = Brush.vlist[Brush.t_faces-3];
		
		float &sangle = Brush.vAngle_s; // smallest vertex angle of whole brush
		float &bangle = Brush.vAngle_b; // biggest  vertex angle of whole brush
		
		#if DEBUG > 0
		if(dev) cout << "Brush " << Brush.SegID << ", smallest vertex angle: " << sangle << ", biggest: " << bangle << endl;
		#endif
		
		for (int f = 0; f<Brush.t_faces; f++)
		{
			face &Face = Brush.Faces[f];
			vertex &sfv = Face.Vertices[Face.vAngle_s]; // vertex of this face that has smallest angle
			vertex &bfv  = Face.Vertices[Face.vAngle_b]; // ... biggest angle
			
			if (Face.fID==2) // only body faces
			{
				#if DEBUG > 0
				if(dev) cout << " Face " << f << ", smallest angle: " << Face.Vertices[Face.vAngle_s].angle << ", vertex "<< Face.vAngle_s << Face.Vertices[Face.vAngle_s] << ", biggest angle: " << Face.Vertices[Face.vAngle_b].angle << ", vertex "<< Face.vAngle_b << Face.Vertices[Face.vAngle_b] << endl;
				#endif
				
				// a brush has 2 faces, that share the smallest vertex angle, but only one face can be the first, the other one is the last
				if		(sfv.angle == sangle && bfv.angle < bangle) // the first face cant have the biggest vertex angle in it
				first_entry = f;
				
				else if (sfv.angle == sangle && bfv.angle == bangle) // only the last face can have the biggest vertex angle in it
				{
					last_entry = f;
					Face.vAngle_s = Face.vAngle_b;
				}
			}
		}
		
		#if DEBUG > 0
		if(dev) cout << "first vList ID " << first_entry << endl;
		if(dev) cout << "last  vList ID " << last_entry << endl;
		if(dev) cout << "first entry of this brush is face " << first_entry << " tex "<< Brush.Faces[first_entry].Texture <<" vertex " << Brush.Faces[first_entry].Vertices[Brush.Faces[first_entry].vAngle_s] << " with a vertex angle of " << Brush.Faces[first_entry].Vertices[Brush.Faces[first_entry].vAngle_s].angle << endl;
		if(dev) cout << "last  entry of this brush is face " << last_entry  << " tex "<< Brush.Faces[last_entry].Texture <<" vertex " << Brush.Faces[last_entry].Vertices[Brush.Faces[last_entry].vAngle_b] << " with a vertex angle of " << Brush.Faces[last_entry].Vertices[Brush.Faces[last_entry].vAngle_s].angle << endl;
		#endif
	}
	
	#if DEBUG > 0
	if(dev) system("pause");
	#endif
}


// sort the rest of the vertices (only one vertex per face left)
void brush::GetVertexList()
{
	#if DEBUG > 0
	bool dev = 0;
	if(dev) cout << endl << " Sorting Vertices..." << endl;
	#endif
	
	brush &Brush = *this;
		
	if (Brush.vlist[0]<0||Brush.vlist[Brush.t_faces-3]<0)
	{
		cout << "|    [ERROR] There was a problem reconstructing a Brush, skipping..." << endl;
		Brush.valid = 0;
	}
	
	if (Brush.valid)
	{
		// print vlist
		#if DEBUG > 0
		if(dev) cout << "   Vertex List Pointer " << Brush.vlist << " Array Size " << sizeof(*Brush.vlist)/sizeof(int) << endl;
		if(dev)
		for (int s = 0; s<Brush.t_faces-2; s++) {
			cout << "    VList #" << s << " " << Brush.vlist[s] << endl;
		}
		#endif
		
		// eventually clear vlist
		for (int s = 1; s<Brush.t_faces-3; s++) {
			Brush.vlist[s] = -1;
		}
		
		for (int s = 1; s<Brush.t_faces-3; s++) // list loop starts at second vertex entry and stops 1 before end, since first anf last vertex is already known
		{
			//cout << "  List Entry #" << s << endl;
			int &centry = Brush.vlist[s];
			int &lentry = Brush.vlist[s-1]; // last list entry

			vertex &last = Brush.Faces[lentry].Vertices[  Brush.Faces[lentry].vAngle_s  ];
			
			#if DEBUG > 0
			if(dev) cout << "    Last Vertex: (Face "<<lentry<<" Vertex "<<  Brush.Faces[lentry].vAngle_s << ")" << Brush.Faces[lentry].Vertices[Brush.Faces[lentry].vAngle_s].angle << endl;
			#endif
			
			for (int f = 0; f<Brush.t_faces; f++) // face loop
			{
				face &Face = Brush.Faces[f];
				//cout << "    Face #" << f+1 << endl;
				if (Face.fID==2)
				{
					#if DEBUG > 0
					if(dev) cout << "    Current List Entry # " << s << ", Candidate Face#" << f << ", Vertex #" << Face.vAngle_s << ", Angle: " << Face.Vertices[Face.vAngle_s].angle << endl;
					#endif
					
					vertex &candidate = Face.Vertices[Face.vAngle_s];

					#if DEBUG > 0
					if(dev) cout << "    Current Vertex: " << candidate << endl;
					#endif
					
					if (centry==-1 && candidate.angle > last.angle)
					// if current list entry is UNSET and current vertex angle is greater than first vertex angle, set current list entry to Face f and Vertex v
					{
						centry = f;
						
						#if DEBUG > 0
						if(dev) cout << "      New Entry (First): " << candidate.angle << " is > last entry: " << last.angle << endl;
						#endif
					}
					else if (centry!=-1 && candidate.angle < Brush.Faces[centry].Vertices[  Brush.Faces[centry].vAngle_s  ].angle && candidate.angle > last.angle)
					// if current list entry is UNSET and current vertex angle is SMALLER than currently "in-list" stored vertex BUT GREATER than last vertex angle, set current list entry to Face f and Vertex v
					{
						centry = f;
						
						#if DEBUG > 0
						if(dev) cout << "      New Entry (xth): "<< candidate.angle << " is < candidate: " << Brush.Faces[centry].Vertices[Brush.Faces[centry].vAngle_s].angle << " but not > last entry: " << last.angle <<endl;
						#endif
					}
				}
			}
		}
		
		#if DEBUG > 0
		if(dev) cout << "Sorted vertex list for brush #" << Brush.SegID << endl;
		if(dev)
		for (int i = 0; i<Brush.t_faces-2; i++) { // list loop
			face &Face = Brush.Faces[Brush.vlist[i]];
			int &svert = Face.vAngle_s;
			cout << "#" << i << " = Vertex #" << Face.vAngle_s << " of Face #" << Brush.vlist[i] << ", Angle: " << Face.Vertices[svert].angle << ", Vertex " << Brush.Faces[Brush.vlist[i]].Vertices[Brush.Faces[Brush.vlist[i]].vAngle_s] <<  endl;
		}
		#endif
	}
	
	#if DEBUG > 0
	if(dev) cout << "Press any Button to continue!" << endl;
	if(dev) system("pause");
	#endif
}

void brush::ConvertVerticesC2V()
{
	#if DEBUG > 0
	bool dev = 0;
	if (dev) cout << " Converting RCON Vertices to normal Vertices..." << endl;
	#endif
	
	brush &Brush = *this;
	for(int f = 0; f<Brush.t_faces; f++)
	{
		face &Face = Brush.Faces[f];
		
		#if DEBUG > 0
		if (dev) cout << "   Face " << f << " Tex " << Face.Texture << " V " << Face.vcount << " VC " << Face.vcountC << " ID " << Face.fID << " Or " << Face.Orient << endl;
		#endif
		
		Face.vcount = Face.vcountC;
		
		if (Face.fID==2)
		{
			#if DEBUG > 0
			if (dev) cout << "     Body Face" << endl;
			#endif
			
			int Missing = -1;
			// maintain first 3 old vertices and just add the missing one
			vertex *NewBody = new vertex[4];
			for (int v = 0; v<3; v++) {
				vertex &VO = Face.Vertices[v];
				vertex &VN = NewBody[v];
				VN = VO;
				
				#if DEBUG > 0
				if (dev) cout << "     Vertex " << v << VN << endl;
				#endif
			}
			// Look for last missing Vertex
			for (int c = 3; c>=0&&Missing==-1; c--)
			for (int v = 0, fail = 0; v<3; v++)
			{
				vertex &VC = Face.VerticesC[c];
				vertex &VO = Face.Vertices[v];
				
				#if DEBUG > 0
				if (dev) cout << "        Comparing Candidate #"<< c << VC << " and Old #" << v << VO << endl;
				#endif
				
				if (!CompareVertices(VC,VO)) { fail++;}
				if (v==2&&fail==3) { Missing = c; break; }
			}
			NewBody[3] = Face.VerticesC[Missing];
			
			#if DEBUG > 0
			if (dev) cout << "     Vertex 4 " << NewBody[3] << endl;
			#endif
			
			delete[] Face.Vertices;
			Face.Vertices = NewBody;
		}
		else
		{
			#if DEBUG > 0
			if (dev) cout << "     Head/Base Face" << endl;
			#endif
			
			delete[] Face.Vertices;
			Face.Vertices = new vertex[Face.vcountC];
			
			for (int v = 0; v<Face.vcountC; v++) {
				Face.Vertices[v] = Face.VerticesC[v];
				
				#if DEBUG > 0
				if (dev) cout << "     Vertex " << v << Face.Vertices[v] << endl;
				#endif
			}
		}
		Face.GetVertexOrder();
		
		if (!Face.Clockwise)
		Face.RevOrder(0);
	}
	
	#if DEBUG > 0
	if (dev) system("pause");
	#endif
}

// create new vertices for all faces to be able to calculate their X/Y Texture Offsets
void brush::GetRconVertices()
{
	#if DEBUG > 0
	bool dev = 0;
	if(dev) cout << "Reconstructing non-existent brush vertices..." << endl;
	#endif
	
	brush &Brush = *this;
	
	if (Brush.valid)
	{
		// get smallest and biggest x coord for this brush
		float sx = 0;
		float bx = 0;
		sx = Brush.Faces[ Brush.BaseID ].Vertices[0].x;
		bx = Brush.Faces[ Brush.HeadID ].Vertices[0].x;
		
		#if DEBUG > 0
		if(dev) cout << "  smallest/biggest X: " << sx << " / " << bx << endl;
		#endif
		
		// new head/base faces
		#if DEBUG > 0
		if(dev) cout << "  Head/Base Faces... Base ID " << Brush.BaseID << " Head ID " << Brush.HeadID << endl;
		#endif
		
		for (int f = 0; f<2; f++)
		{
			int FaceID = 0;
			if (f==0) 	FaceID = Brush.BaseID;
			else 		FaceID = Brush.HeadID;
			face &Face = Brush.Faces[FaceID];
			Face.VerticesC = new vertex[Brush.t_faces-2];
			Face.vcountC = Brush.t_faces-2;
			
			#if DEBUG > 0
			if(dev) cout << "    New Head Base Vertices..." << endl;
			#endif
			
			for (int v = 0; v<Brush.t_faces-2; v++)
			{
				#if DEBUG > 0
				if(dev) cout << "      Vertex " << v << " Brush.vlist " << Brush.vlist[v] << " Angle " << Brush.Faces[Brush.vlist[v]].vAngle_s << endl;
				#endif
				
				Face.VerticesC[v] = Brush.Faces[  Brush.vlist[v]  ].Vertices[  Brush.Faces[Brush.vlist[v]].vAngle_s  ];
				
				if (f==0) Face.VerticesC[v].x = sx;
				if (f==1) Face.VerticesC[v].x = bx;
			}
			
			#if DEBUG > 0
			if(dev) {
			for (int v = 0; v<Brush.t_faces-2; v++) cout << "    Face "<<f<<" new vertex " << v << Face.VerticesC[v] << endl;
			for (int v = 0; v<3; v++) cout << "    Face "<<f<<" old vertex " << v << Face.Vertices[v] << endl; }
			#endif
		}
		
		// new body faces
		#if DEBUG > 0
		if(dev) cout << "  Body Faces..." << endl;
		#endif
		
		for (int e = 0; e<Brush.t_faces-2; e++)
		{
			int f = Brush.vlist[e]; // first vlist entry face
			
			face &Face = Brush.Faces[f];
			Face.VerticesC = new vertex[4];
			Face.vcountC = 4;
			
			Face.VerticesC[0] = Face.Vertices[ Face.vAngle_sbak ];
			Face.VerticesC[0].x = bx;
			Face.VerticesC[1] = Face.Vertices[ Face.vAngle_b ];
			Face.VerticesC[1].x = bx;
			Face.VerticesC[2] = Face.Vertices[ Face.vAngle_b ];
			Face.VerticesC[2].x = sx;
			Face.VerticesC[3] = Face.Vertices[ Face.vAngle_sbak ];
			Face.VerticesC[3].x = sx;
			
			#if DEBUG > 0
			if(dev) {
			for (int v = 0; v<4; v++) cout << "    Face "<<f<<" new vertex " << v << Face.VerticesC[v] << " Face.vAngle_sbak " << Face.vAngle_sbak << " Face.vAngle_b " << Face.vAngle_b << endl;
			for (int v = 0; v<3; v++) cout << "    Face "<<f<<" old vertex " << v << Face.Vertices[v] << endl; }
			#endif
		}
		Brush.RCON = 1;
	}
	
	#if DEBUG > 0
	if(dev) cout << "Press any Button to continue!" << endl;
	if(dev) system("pause");
	#endif
}


// Determine Texture Alignment of all Faces and fix invalid Texture alignments
void brush::GetTVecAligns()
{
	#if DEBUG > 0
	bool dev = 0;
	if(dev) cout << "Determine Alignment of all Faces and fix invalid Face alignments..." << endl;
	#endif
	
	brush &Brush = *this;
	if (Brush.valid)
	{
		for (int f = 0; f<Brush.t_faces; f++)
		{
			face &Face = Brush.Faces[f];
			
			if (Face.fID==2)
			{
				bool IsValid = IsFaceAlignValid(Face);
				
				//if (Face.FaceAlign==1)
				if (!IsValid) // Check if Face-Align of this Face is valid
				{
					//cout << "Face vector " << Face.VecX<< " is Hor("<<Face.VecX.IsHor<<") " << Face.VecY << " is Hor(" << Face.VecY.IsHor <<")" << endl;
					
					cout << "|    [WARNING] Face #" << f+1 << " of Brush #"<<Brush.SegID+1<<" (Tex: " << Face.Texture << ")"<< endl;
					cout << "|              has no valid align for arc generation."<<endl;
					cout << "|              World Align is being applied." << endl;
					cout << "|" << endl;
					AlignToWorld(Face);
				}
				
				// get orientation of horizontal vectors for correct baseshift calculation
				if (Face.VecH->x<0) {
					Face.VecH->IsNeg = 1;
					//cout << "  Hor Vector " << *Face.VecH << " of Face " << f << " Brush " << b << " has negative X coord!" << endl;
				} else
					Face.VecH->IsNeg = 0;
				//float PitchN = GetVecAlign(Face.Normal,1);
				float PitchV = GetVecAlign(*Face.VecV,1);
				//cout << " Face " << f << " Tex " << Face.Texture << " VecV" << *Face.VecV << endl;
				//cout << "   PitchN " << PitchN << " PitchV " << PitchV;// << " PitchDiff " << PitchDiff;
				if (/*Face.VecV->z>0|| ( Face.VecV->z==0 && Face.VecV->y>0 &&  ) */ /*PitchDiff<180*/ PitchV<=90||PitchV>270 ) {
					Face.VecV->IsNeg = 1;
					//Face.Texture = "RED";
					//cout << " V Vector is NEGATIVE!" << endl; 
				} //else cout << " V Vector is Positive!" << endl;
				else Face.VecV->IsNeg = 0;
				//cout << endl;
			}
		}
	}
	
	#if DEBUG > 0
	if(dev) cout << "Press ANY BUTTON to continue! ..." << endl;
	if(dev) system("pause");
	#endif
}

void brush::GetFaceNormals()
{
	if (valid)
	{
		for (int f = 0; f<t_faces; f++)
		{
			face &Face = Faces[f];
			if ( Face.draw && Face.vcount>2 )
				Face.GetNormal();
		}
	}
}

void brush::GetFaceShifts()
{
	if (valid)
	{
		for (int f = 0; f<t_faces; f++)
		{
			face &Face = Faces[f];
			GetBaseEdges(Face);
			GetBaseShift(Face, 0, 1, 0);
			GetTexOffset(Face, 0);
		}
	}
}

// get exact centroids of reconstructed Faces
void brush::GetFaceCentroids()
{
	#if DEBUG > 0
	bool dev = 0;
	if(dev) cout << "Getting exact centroid of Brush " << this->SecID << " Faces..." << endl;
	#endif
	
	brush &Brush = *this;
	
	if (Brush.valid)
	{
		for (int f = 0; f<Brush.t_faces; f++)
		{
			face &Face = Brush.Faces[f];
			Face.GetCentroid();
			
			#if DEBUG > 0
			if (dev) cout << "  Face " << f << " Centroid is " << Face.Centroid << endl;
			#endif
		}
	}
}

// get exact centroids of reconstructed Faces
void brush::GetFaceCentroidsC()
{
	#if DEBUG > 0
	bool dev = 0;
	if(dev) cout << "Getting exact centroid of Brush " << this->SecID << " Recon-Faces..." << endl;
	#endif
	
	brush &Brush = *this;
	
	if (Brush.valid)
	{
		for (int f = 0; f<Brush.t_faces; f++)
		{
			face &Face = Brush.Faces[f];
			Face.GetCentroidC();
			
			#if DEBUG > 0
			if (dev) cout << "  Face " << f << " Centroid is " << Face.Centroid << endl;
			#endif
		}
	}
}

void brush::CheckDivisibility()
{
	// Mark Cuboid Brushes for potential triangulation
	#if DEBUG > 0
	bool dev = 0;
	if(dev) cout << " Checking for Cuboid Brushes..." << endl;
	#endif
	
	brush &Brush = *this;
	if (Brush.valid)
	{
		// differ between potential rectangle- and polygone type brushes
		#if DEBUG > 0
		if(dev) cout << " Brush " << Brush.SegID << " total faces " << Brush.t_faces << " base ID " << Brush.BaseID << " head ID " << Brush.HeadID << endl;
		#endif
		
		if (Brush.t_faces==6||Brush.t_faces==5)
		{
			// a brush qualifies for "simple" (filesize efficient) triangulation, when there is an upright front and back face and the brush has 6 faces altogether
			bool foundBack = 0;
			bool foundFront = 0;
			
			for (int f = 0; f<Brush.t_faces; f++)
			{
				face &Face = Brush.Faces[f];
				if (Face.fID==2)
				{
					if 		(Face.Orient==4)
					{
						foundFront = 1;
						
						#if DEBUG > 0
						if(dev)cout << "   Found Front! Face Orient " << Face.Orient << endl;
						#endif
					}
					else if (Face.Orient==5)
					{
						foundBack  = 1;
						
						#if DEBUG > 0
						if(dev)cout << "   Found Back!  Face Orient " << Face.Orient << endl;
						#endif
					}
				}
			}
			if (foundFront&&foundBack&&Brush.t_faces==6) Brush.IsDivisible = 1;
			else if ((foundFront||foundBack)&&Brush.t_faces==5) Brush.IsDivisible = 1;
			else Brush.IsDivisible = 0;
		}
		
		#if DEBUG > 0
		if (dev) {
		if (Brush.IsDivisible) cout << " Brush seg "<< Brush.SegID <<" sec " << Brush.SecID <<" IS divisible easily!" << endl;
		else cout << " Brush seg "<< Brush.SegID <<" sec " << Brush.SecID<<" tex ("<<Brush.Faces[2].Texture<<") IS NOT divisible easily!" << endl; }
		#endif
	}
}

void brush::CreateGap(int g)
{
	#if DEBUG > 0
	bool dev = 0;
	if (dev) cout << " Creating Gap Brush from Brush #" <<this->SecID<<"..." << endl;
	#endif
	
	brush &Source = *this;
	Gap = new brush;
	Gap->Copy(Source);
	
	Gap->Faces[0].GetNormal();
	gvector nEdge = Gap->Faces[0].Normal;
	nEdge.mult(cTable[g].gaplen); // make the gap brush 256 units long
	nEdge.flip();
	
	// Fix Head Faces
	#if DEBUG > 0
	if (dev) cout << "   Fixing Head Face..." << endl;
	#endif
	
	face &Base = Gap->Faces[0];
	face &Head = Gap->Faces[1];
	for (int v = 0; v<Base.vcount; v++)
	{
		vertex &VB = Base.Vertices[v];
		vertex &VH = Head.Vertices[v];
		VH = Add(VB,nEdge);
		
		#if DEBUG > 0
		if (dev) cout << "     Head Vertex "<<v<< " now " << VH << " = Base Vertex " << VB << " + nEdge " << nEdge << endl;
		#endif
	}
	Head.RevOrder(0);
	
	// Fix Body Faces
	for (int f = 2; f<Gap->t_faces; f++)
	{
		face &Face = Gap->Faces[f];
		vertex &V0 = Face.Vertices[0];
		vertex &V1 = Face.Vertices[1];
		vertex &V2 = Face.Vertices[2];
		vertex &V3 = Face.Vertices[3];
		V2 = Add(V1, nEdge);
		V3 = Add(V0, nEdge);
		
		#if DEBUG > 0
		if (dev) cout << "     Body Vertex 3 now " << V3 << " = V0 " << V0 << " + nEdge " << nEdge << endl;
		if (dev) cout << "     Body Vertex 2 now " << V2 << " = V1 " << V1 << " + nEdge " << nEdge << endl;
		#endif
	}
}

void brush::FixBorderliner(int prec)
{
	#if DEBUG > 0
	bool dev = 0;
	#endif
	
	for(int f=0; f<t_faces; f++)
	{
		face &Face = Faces[f];
		
		#if DEBUG > 0
		if(dev)cout << setprecision(8);
		if(dev)cout << Face;
		#endif
		
		for(int v=0; v<Face.vcount; v++)
		{
			vertex &V = Face.Vertices[v];
			if(IsBorderliner(V.x,prec)==1) { V.x = round(V.x);}
			if(IsBorderliner(V.y,prec)==1) { V.y = round(V.y);}
			if(IsBorderliner(V.z,prec)==1) { V.z = round(V.z);}
		}
		
		#if DEBUG > 0
		if(dev){cout << Face; cout << setprecision(0); system("pause");}
		#endif
	}
}

void brush::Reconstruct()
{
	#if DEBUG > 0
	bool dev = 0;
	#endif
	
	brush &Brush = *this;
	bool DoRcon[Brush.t_faces]; // which faces are to be reconstructed
	for (int f = 0; f<t_faces; f++)
		DoRcon[f] = 1;
	vector< vector<int> > ConList(t_faces); // connected Faces index list

	ConList.resize(t_faces);
	// when a brush is being imported from a map file, each of its faces consists of 3 vertices
	// in order to be able to get their original texture offsets, their remaining vertices have to be calculated
	
	//Brush.FixBorderliner(2);
	Brush.GetFaceNormals();
	
	// get some missing vertices from the 3 existing face vertices, by checking the distance of each brush vertex to each brush plane (when dist is 0, vertex is on plane!)
	#if DEBUG > 0
	if (dev) cout << " Getting missing face vertices from the 3 existing face vertices..." << endl;
	if (dev) system("pause");
	#endif
	
	for (int f = 0; f<t_faces; f++)
	{
		face &Face = Brush.Faces[f];
		vector<vertex> NewFaceVerts;
		// Fill new vertex list with existing face vertices
		for (int i = 0; i<Face.vcount; i++)
			NewFaceVerts.push_back(Face.Vertices[i]);
		
		// Vertices of all Faces are being checked
		#if DEBUG > 0
		if (dev) cout << "  Face " << f << " - Vertices of all Faces are being checked..." << endl;
		#endif
		
		for (int fc = 0; fc<t_faces; fc++)
		{
			face &CFace = Brush.Faces[fc];
			if (f!=fc) // skip if candidate is current face itself
			{
				for (int v = 0; v<CFace.vcount; v++)
				{
					vertex &V = CFace.Vertices[v];
					
					if (IsVertexOnFace(Face,V,1))
					{
						NewFaceVerts.push_back(CFace.Vertices[v]);
						
						#if DEBUG > 0
						if (dev) cout << "   Match! V " << v << V << " of F " << fc << " ("<<CFace.Texture <<") is on Face " << f << " ("<<Face.Texture<<") NVec "<< Face.Normal << " Listsize " << NewFaceVerts.size() << endl;
						#endif
					}
					else
					{
						#if DEBUG > 0
						if (dev) cout << "   NO Match! Vertex " << v << V << " of Face " << fc << " ("<<CFace.Texture <<") wasnt found on Face " << f << " ("<<Face.Texture<<")!" << endl;
						#endif
					}
				}
			}
		}
		#if DEBUG > 0
		if (dev) system("pause");
		#endif
		
		//replace existing vertex array of this face and fill it with the newly generated vertex list
		int new_vcount = NewFaceVerts.size();
		int old_vcount = Face.vcount;
		if (new_vcount>Face.vcount)
		{
			#if DEBUG > 0
			if (dev) cout << "  New vcount " << new_vcount << " - Creating new Vertex array for Face "<<Face.Texture<<"..." << endl;
			#endif
			
			delete[] Face.Vertices;
			Face.Vertices = new vertex[new_vcount];
			Face.vcount = old_vcount; // usually 3
			
			for (int v = 0, vb=0; v<new_vcount; v++)
			{
				vertex &V = Face.Vertices[vb];
				vertex &VN = NewFaceVerts[v];
				// first check if new vertex is a copy of existing vertices or an actual new vertex
				if (v>=old_vcount)
				{
					if (  !IsVertexInList(VN, Face.Vertices, Face.vcount, 1, 2)  )
					{
						V = VN;
						
						#if DEBUG > 0
						if (dev) cout << "   Face " << f << " Vcount O/N ("<<Face.vcount<<"/"<<Face.vcount+1<<") NewVert "<< v << VN << " added to VList ("<<vb<<"). V-Index increased to " << vb+1 << endl;
						#endif
						
						Face.vcount++;
						vb++;
					}
					else
					{
						//...
					}
				}
				else if (v<old_vcount)
				{
					V = VN;
					vb++;
				}
			}
		}
		#if DEBUG > 0
		if (dev) system("pause");
		#endif
	}
	
	#if DEBUG > 0
	if (dev) cout << "  New vertex lists..." << endl;
	if (dev)
	for (int f = 0; f<t_faces; f++)
	{
		face &Face = Brush.Faces[f];
		for (int v = 0; v<Face.vcount; v++)
		{
			vertex &V = Face.Vertices[v];
			cout << "    Face " << f<< " Tex " << Face.Texture << " v " << v << V << endl;
		}
	}
	#endif
	
	// sort face vertices
	for (int f = 0; f<t_faces; f++)
	{
		face &Face = Brush.Faces[f];
		Face.SortVertices(Face.Normal);
		Face.GetNormal();
	}
	
	#if DEBUG > 0
	if (dev) cout << " FINISH!" << endl;
	if (dev) system("pause");
	#endif
	// Skip reconstruction entirely, if brush is only made of triangles! (but this isnt possible. theres no way to tell if thats true without further checking)
	// solution: if every face of a brush has 2 connected faces initially, the brush consists only of triangles (correct?? NO!!)
}

void brush::CheckForHoles(vector<int> &Neighbors)
{
	// there is a hole in the brush, if any face does not share at least 3 full edges (each has 2 vertices) with other faces
	#if DEBUG > 0
	bool dev = 0;
	if(dev) cout << endl << " Brush Faces " << t_faces << " - Checking for Holes..." << endl;
	#endif
	
	brush &Brush = *this;
	vector<int> Temp;
	
	// check for unique edges
	#if DEBUG > 0
	if(dev) cout << endl << "  Checking Edges..." << endl;
	#endif
	
	for (int f=0; f<Brush.t_faces; f++)
	{
		face &Face = Faces[f];
		for (int v=0; v<Face.vcount; v++)
		{
			vertex *V1; vertex *V2;
			if(v==Face.vcount-1) {
				V1 = &Face.Vertices[v];
				V2 = &Face.Vertices[0];
			} else {
				V1 = &Face.Vertices[v];
				V2 = &Face.Vertices[v+1];
			}
			
			// check if face does not share one of its edges with other faces; if it does not, mark it as "Neighbor" of a hole
			if( !IsEdgeInBrush(*V1, *V2, f) )
			{
				Temp.push_back(f);
				
				#if DEBUG > 0
				if(dev) cout << "   Edge #"<<v<< *V1 << *V2 <<" of Face " <<f<< " Tex " << Face.Texture << " NOT found in this Brush again! (indicates a hole)" << endl;
				if(dev) { V1->DoSplit=1; V2->DoSplit=1; }
				#endif
				
				break;
			}
		}
	}
	
	// DEV PURPOSES: EXPORT BRUSH TO OBJ FILE
	//string Output = gFile->p_path+gFile->name;
	//ExportBrushToOBJ(Output, Brush);
	
	// 3 connected Neighbors are necessary to get the missing vertex; otherwise ignore hole ¯\_(")_/¯
	#if DEBUG > 0
	if(dev) cout << "  3 connected Neighbors are necessary to get the missing vertex..." << endl;
	#endif
	
	if(Temp.size()>3)
	{
		#if DEBUG > 0
		if(dev) cout << "   More neighbors found than necessary: " << Temp.size() << endl;
		#endif
		
		face &N1 = Faces[Temp[0]];
		for(int c=1; c<Temp.size(); c++)
		{
			face &C = Faces[Temp[c]];
			if( !DoFacesShareVertices( N1, C ) )
			{
				Temp[c] = -1;
			}
			else
			{
				#if DEBUG > 0
				if(dev) cout << "    Connected Face #"<<c<< " Tex " << Faces[Temp[c]].Texture<<" for Neighbor Face #0 (" << Temp[0]<< " Tex " << Faces[0].Texture << ") -> " << Temp[c] << endl;
				#endif
			}
		}
		
		if(Temp.size()>=3)
		for(int n=0; n<3; n++)
			if(Temp[n]!=-1)
			{
				#if DEBUG > 0
				if(dev) cout << "    #" << Temp.size() << " " << Temp[n] << endl;
				#endif
				
				Neighbors.push_back(Temp[n]);
			}	
		
		#if DEBUG > 0
		if(dev) cout << "   Returning " << Neighbors.size() << " Neighbor Faces..." << endl;
		#endif
	}
	else if(Temp.size()==3)
	{
		#if DEBUG > 0
		if(dev) cout << "  Exactely 3 Neighbor Faces found..." << endl;
		#endif
		
		Neighbors = Temp;
	}
}

bool brush::IsEdgeInBrush(vertex &E1, vertex &E2, int Exclude)
{
	brush &Brush = *this;
	
	#if DEBUG > 0
	bool dev = 0;
	if(dev) cout << endl << " Searching Edge " << E1 << E2 << " in Brush. Excluding Face "<< Exclude << " tex " << Faces[Exclude].Texture << endl;
	#endif
	
	for (int f=0; f<Brush.t_faces; f++)
	{
		face &Face = Faces[f];
		if(f!=Exclude)
		for (int v=0; v<Face.vcount; v++)
		{
			vertex *V1, *V2, *V1s, *V2s;
			if(v==Face.vcount-1)
			{
				V1 = &Face.Vertices[v];
				V2 = &Face.Vertices[0];
				V1s = &Face.Vertices[0]; // also check inverted edge
				V2s = &Face.Vertices[v];
			}
			else
			{
				V1 = &Face.Vertices[v];
				V2 = &Face.Vertices[v+1];
				V1s = &Face.Vertices[v+1]; // also check inverted edge
				V2s = &Face.Vertices[v];
			}
			
			#if DEBUG > 0
			if(dev) cout << "   Face " << f << " tex " << Face.Texture << " Edge v#" << v << *V1 << " v#" << v+1 << *V2;
			#endif
			
			if( (CompareVerticesR(*V1,E1) && CompareVerticesR(*V2,E2)) || (CompareVerticesR(*V1s,E1) && CompareVerticesR(*V2s,E2)) )
			{
				#if DEBUG > 0
				if(dev) cout << " [MATCH] Face " << f << " Tex "<<Face.Texture <<" contains wanted Edge!" << endl;
				#endif
				
				return 1;
			}
			else
			{
				#if DEBUG > 0
				if(dev) cout << endl;
				#endif
			}
		}
	}
	#if DEBUG > 0
	if(dev) cout << endl;
	#endif
	
	return 0;
}
	

void brush::FixHoles()
{
	#if DEBUG > 0
	bool dev = 0;
	if(dev) cout << endl << " Fixing Holes..."<< endl;
	#endif
	brush &Brush = *this;
	
	vector<int> Neighbors;
	CheckForHoles(Neighbors);
	if( Neighbors.size()==3 )
	{
		face &F1 = Faces[Neighbors[0]];
		face &F2 = Faces[Neighbors[1]];
		face &F3 = Faces[Neighbors[2]];
		
		#if DEBUG > 0
		if(dev) cout << "   Getting intersection point of 3 faces..."<< endl;
		if(dev) for(int i=0;i<3; i++) cout << "    Neighbor Face #" << i << " [" << Neighbors[i] << "]" << endl << Faces[Neighbors[i]] << endl;
		#endif
		
		double mat[3][4];
		SetMat(mat, F1,F2,F3);
		vertex Isect;
		
		if(gaussianElimination(mat, Isect))
		{
			// add new vertex to each of the faces, that surround the hole
			F1.AddNewVertex(Isect);
			F2.AddNewVertex(Isect);
			F3.AddNewVertex(Isect);
		}
		else
		{
			cout << "|    [WARNING] Hole found in imported Brush could not be fixed!"<< endl;
			
			#if DEBUG > 0
			if(dev) {
			F1.Texture = "RED";
			F2.Texture = "RED";
			F3.Texture = "RED";}
			if(dev) for(int i=0;i<3; i++) cout << "    Neighbor Face #" << i << " [" << Neighbors[i] << "]" << endl << Faces[Neighbors[i]] << endl;
			if(dev) cout << " F1.Normal " << F1.Normal << " F2.Normal " << F2.Normal << " F3.Normal " << F3.Normal << endl;
			#endif
		}
	}
}

void brush::CreateTent()
{
	#if DEBUG > 0
	bool dev = 0;
	#endif
	brush &Brush = *this;
	
	Brush.Faces[0].GetCentroid();
	vertex TentBase = Brush.Faces[0].Centroid;
	Brush.Faces[0].GetNormal();
	gvector Normal = Brush.Faces[0].Normal.flip();
	Normal.mult(def_spikesize);
	TentBase.Add(Normal);

	#if DEBUG > 0
	if (dev) cout << "  Created Tent Vertex " << TentBase << " from centroid " << Brush.Faces[0].Centroid << " and flipped Normal " << Normal <<endl;
	if (dev) cout << "  Creating Tent NULL Faces now..." <<endl;
	if (dev) cout << " Creating Tent for Triangle Brush..." << endl;
	#endif
	
	for (int f = 1, v = 0; f<Brush.t_faces; f++)
	{
		face &Face = Brush.Faces[f];
		Face.Vertices = new vertex[3];
		Face.vcount = 3;
		vertex &V0 = Face.Vertices[0];
		vertex &V1 = Face.Vertices[1];
		vertex &V2 = Face.Vertices[2];
		
		#if DEBUG > 0
		if (dev) cout << "      created 3 new vertices for face " << f << endl;
		#endif
		
		V0 = Brush.Faces[0].Vertices[v];
		V1 = TentBase;
		if (f==Brush.t_faces-1)
		V2 = Brush.Faces[0].Vertices[0];
		else
		V2 = Brush.Faces[0].Vertices[v+1];

		#if DEBUG > 0
		if (dev) cout << "      V0 " << V0 << V1 << V2 << endl;
		#endif
		
		Face.Texture = def_nulltex;
		Face.IsNULL = 1;
		Face.fID=2;
		AlignToWorld(Face);
		Face.TentID = 1;
		
		v++;
	}
}

void brush::ClearVertexList()
{
	brush &Brush = *this;
	
	Brush.vAngle_b = 0;
	Brush.vAngle_s = 0;
	if (Brush.vlist!=nullptr)
	{
		delete Brush.vlist;
		Brush.vlist = new int[Brush.t_faces-2];
		for (int i = 0; i<Brush.t_faces-2; i++)
			Brush.vlist[i] = -1;
	}
		
	for (int f = 0; f<Brush.t_faces; f++)
	{
		face &Face = Brush.Faces[f];
		
		Face.vAngle_b = 0;
		Face.vAngle_s = 0;
		Face.vAngle_sbak = 0;
		for (int i = 0; i<4; i++)
		{
			Face.BaseListX[i] = 0;
			Face.BaseListY[i] = 0;
		}
		for (int v = 0; v<Face.vcount; v++)
		{
			vertex &V = Face.Vertices[v];
			V.angle = 0;
		}
	}
}

void brush::RoundVertices()
{
	brush &Brush = *this;
	
	if (   Brush.valid && Brush.draw  )
	for (int f = 0; f<Brush.t_faces; f++)
	{
		face &Face = Brush.Faces[f];
		Face.RoundVertices();
		
		// Gaps
		if(Brush.Gap!=nullptr)
		{
			face &Face = Brush.Gap->Faces[f];
			Face.RoundVertices();
		}
	}
}

void brush::GetFacePlanarity()
{
	brush &Brush = *this;
	
	if (Brush.valid&&Brush.draw)
	for (int f = 0; f<Brush.t_faces; f++)
	{
		face &Face = Brush.Faces[f];
		if (Face.fID==2)
		Face.GetPlanarity();
	}
}

// Mark all vertices of a brush that match a certain faces vertices; Mode 0 = DoRound; Mode 1 = DoAddHeight
void brush::MarkFaceVertices(face &Candidate, int Mode, bool Overwrite)
{
	#if DEBUG > 0
	bool dev = 0;
	#endif

	brush &Brush = *this;
	
	if (Brush.valid&&Brush.draw)
	for (int f = 0; f<Brush.t_faces; f++)
	{
		face &Face = Brush.Faces[f];
		if (Overwrite)
		for (int v = 0; v<Face.vcount; v++)
		{
			vertex &V = Face.Vertices[v];
			V.DoAddHeight = 0;
			V.DoRound = 0;
		}

		#if DEBUG > 0
		if(dev) cout << " Marking Vertices of Brush sec " << Brush.SecID << " Face " << f << " mode " << Mode << " comparing to " << Candidate.vcount << " vertices " << endl;
		#endif
		
		for (int c = 0; c<Candidate.vcount; c++)
		{
			vertex &C = Candidate.Vertices[c];
			
			#if DEBUG > 0
			if(dev) cout << "   Comparing with Candidate Vertex " << c << C << endl;
			#endif
			
			for (int v = 0; v<Face.vcount; v++)
			{
				vertex &V = Face.Vertices[v];
				
				#if DEBUG > 0
				if(dev) cout << "     Vertex " << v << V << endl;
				#endif
				
				if (CompareVerticesXY(C,V))
				{
					#if DEBUG > 0
					if(dev) cout << "       Match Found!" << endl;
					#endif
					
					if (Mode==1)	V.DoAddHeight = 1;
					if (Mode==0)	V.DoRound = 1;
				}
			}
		}
	}
	#if DEBUG > 0
	if(dev) system("pause");
	#endif
}

void brush::CheckNULLFaces()
{
	brush &Brush = *this;
	int Ncount = 0;
	
	#if DEBUG > 0
	bool dev = 0;
	if (dev) cout << " Checking Null Count of Brush " << Brush.name << " with face count " << Brush.t_faces << endl;
	#endif
	
	if (Brush.valid&&Brush.draw)
	for (int f = 0; f<Brush.t_faces; f++)
	{
		face &Face = Brush.Faces[f];
		if (!Face.draw||Face.IsNULL||Face.Texture=="NULL"||Face.Texture=="SOLIDHINT"||Face.Texture==def_nulltex)
		{
			Ncount++;
			#if DEBUG > 0
			if (dev) cout << "  Face is NULL Face! NUll Counter now " << Ncount << endl;
			#endif
		}
	}
	
	#if DEBUG > 0
	if (dev) cout << "  Final Null Count of this Brush " << Ncount << " of total " << Brush.t_faces << endl<< endl;
	#endif
	
	if (Ncount == Brush.t_faces)
	Brush.draw = 0;
}

void brush::SetRound(bool State)
{
	brush &Brush = *this;
	
	if (Brush.valid&&Brush.draw)
	for (int f = 0; f<Brush.t_faces; f++)
	{
		face &Face = Brush.Faces[f];
		for (int v = 0; v<Face.vcount; v++)
		{
			vertex &V = Face.Vertices[v];
			V.DoRound = State;
		}
	}
}

void brush::RefreshSpikeTents()
{
	brush &Brush = *this;
	
	#if DEBUG > 0
	bool dev = 0;
	if (dev) cout << " Refreshing Tent Vertex of Brush " << Brush.name << " IsSpike " << Brush.IsSpike << " total faces " << Brush.t_faces << endl;
	#endif
	
	if (Brush.valid&&Brush.draw&&Brush.IsSpike)
	{
		face &Base = Brush.Faces[0];
		Base.GetCentroid();
		Base.GetNormal();
		vertex newTent;
		gvector Normal = Base.Normal.flip();
		Normal.mult(def_spikesize);
		newTent = Add(Base.Centroid, Normal);
		
		for (int f = 1; f<Brush.t_faces; f++)
		{
			face &Face = Brush.Faces[f];
			
			#if DEBUG > 0
			if (dev) cout << "   Face " << f << " isNULL " << Face.IsNULL << endl;
			#endif
			
			if (Face.IsNULL)
			Face.Vertices[Face.TentID] = newTent;
		}
	}
}

void brush::Triangulate()
{
	#if DEBUG > 0
	bool dev = 0;
	#endif
	
	brush &Brush = *this;
	
	if (Brush.valid&&Brush.draw)
	{
		// check which kind of triangulation can be applied to this brush
		// 1. Efficient Mode for Trapezoids and Wedge Profiles with an upright back and/or front face
		// 2. Full (I dont give a shit) Mode for complex brushes with more than 6 faces
		//    - special case: upright (planar) front/back faces can still stay squares. they dont need to be triangulated
		//      condition: Face is head/base face or front/back body face
		
		if (Brush.IsDivisible&&!Brush.IsWedge)
		{
			if (Brush.t_faces==6)
			{
				#if DEBUG > 0
				if (dev) cout << " Triangulating 6 sided Div Brush..." << endl;
				#endif
				
				Brush.TriTrapezoid();
			}
			else if (Brush.t_faces==5)
			{
				#if DEBUG > 0
				if (dev) cout << " Triangulating 5 sided Div Brush..." << endl;
				#endif
				
				Brush.TriTriangle();
			}
		}
		else if (Brush.IsDivisible&&Brush.IsWedge)
		{
			// Do nothing...
			Brush.Tri = new brush[1];
			Brush.Tri[0].CopySimple(Brush);
			Brush.t_tri = 1;
			
			#if DEBUG > 0
			if (dev) cout << " Brush already is a Wedge. Leaving it as it is!" << endl;
			#endif
		}
		else
		{
			// Brush isnt divisible effectively, so it has to be split up into as many triangle-brushes as necessary, but not more, to keep it efficient
			#if DEBUG > 0
			if (dev) cout << " Triangulating Complex Brush..." << endl;
			#endif
			
			Brush.TriComplex();
		}
	}
}

// triangulate a Trapezoid brush that has 4 sides of which 2 (front and back face) are parallel
void brush::TriTrapezoid()
{
	#if DEBUG > 0
	bool dev = 0;
	#endif
	
	brush &Brush = *this;
	
	if (Brush.valid&&Brush.draw&&Brush.IsDivisible&&!Brush.IsWedge&&Brush.t_faces==6)
	{
		#if DEBUG > 0
		if(dev) cout << "   Triangulating Trapezoid Brush - fcount " << Brush.t_faces << endl;
		#endif
		
		Brush.Tri = new brush[2];
		Brush.t_tri = 2;
		brush &Wedge1 = Brush.Tri[0];
		brush &Wedge2 = Brush.Tri[1];
		face *Top[2] 	= {nullptr,nullptr};
		face *Btm[2] 	= {nullptr,nullptr};
		face *Front[2] 	= {nullptr,nullptr};
		face *Back[2] 	= {nullptr,nullptr};
		Wedge1.Copy(Brush);
		Wedge2.Copy(Brush);
		
		for (int f = 0; f<6; f++)
		{
			face &Face1 = Wedge1.Faces[f];
			face &Face2 = Wedge2.Faces[f];
			
			if 		(Face1.Orient==2) {Top[0] = &Face1;}
			else if (Face1.Orient==3) {Btm[0] = &Face1;}
			else if (Face1.Orient==5) {Back[0] = &Face1;}
			
			if 		(Face2.Orient==2) {Top[1] = &Face2;}
			else if (Face2.Orient==3) {Btm[1] = &Face2;}
			else if (Face2.Orient==4) {Front[1] = &Face2;}
		}
		face &Wedge1_Top 	= *Top[0];
		face &Wedge1_Btm 	= *Btm[0];
		face &Wedge1_Back 	= *Back[0];
		face &Wedge2_Top 	= *Top[1];
		face &Wedge2_Btm 	= *Btm[1];
		face &Wedge2_Front 	= *Front[1];
		
		// fix wedge 1
		#if DEBUG > 0
		if(dev) cout << "   Fixing Wedge 1... this includes Face1 " << Wedge1.Faces[1].Texture << " BACK " << Wedge1_Back.Texture << " Wedge1_Top " << Wedge1_Top.Texture << " Wedge1_Btm " << Wedge1_Btm.Texture << endl;
		#endif
		
		Wedge1.Faces[1].draw = 0; // remove relevant cutting edge face to form a triangle of this brush
		Wedge1_Back.Vertices[2]	= Wedge1.Faces[1].Vertices[2];
		Wedge1_Back.Vertices[3]	= Wedge1.Faces[1].Vertices[3];
		swap(Wedge1_Top.Vertices[2], Wedge1_Top.Vertices[3]); // swap vertices to fix vertex order
		Wedge1_Top.vcount = 3;
		Wedge1_Btm.vcount = 3;
		
		// fix wedge 2
		#if DEBUG > 0
		if(dev) cout << "   Fixing Wedge 2..." << endl;
		#endif
		
		Wedge2.IsWedge2 = 1;
		Wedge2.Faces[0].draw = 0;
		Wedge2_Front.Vertices[0]= Wedge2.Faces[0].Vertices[2];
		Wedge2_Front.Vertices[1]= Wedge2.Faces[0].Vertices[3];
		Wedge2_Top.pushVerts(3);
		Wedge2_Btm.pushVerts(2);
		Wedge2_Top.vcount = 3;
		Wedge2_Btm.vcount = 3;
		
		// new tri faces textures and tex align
		#if DEBUG > 0
		if(dev) cout << "   new tri faces textures and tex align..." << endl;
		#endif
		
		Wedge1_Back.fID = 2;
		Wedge1_Back.Orient = 6;
		Wedge1_Back.Texture = def_nulltex;
		Wedge1_Back.IsNULL = 1;
		AlignToWorld(Wedge1_Back);
		
		Wedge2_Front.fID = 2;
		Wedge2_Front.Orient = 6;
		Wedge2_Front.Texture = def_nulltex;
		Wedge2_Front.IsNULL = 1;
		AlignToWorld(Wedge2_Front);
		
		// new Face Edge IDs
		#if DEBUG > 0
		if(dev) cout << "   new Face Edge IDs..." << endl;
		#endif
		
		Wedge1_Top.SetEdges(0,2,0,1,2,1);
		Wedge2_Top.SetEdges(1,0,1,2,0,2);
		Wedge1_Btm.SetEdges(1,2,1,0,2,0);
		Wedge2_Btm.SetEdges(1,2,1,0,2,0);
		
		Wedge1.MarkFaceVertices(Brush.Faces[1], 1, 1);
		Wedge2.MarkFaceVertices(Brush.Faces[1], 1, 1);
		
		#if DEBUG > 0
		if(dev) cout << "   delete Wedge Gaps..." << endl;
		#endif
		
		delete Wedge1.Gap; Wedge1.Gap = nullptr;
		delete Wedge2.Gap; Wedge2.Gap = nullptr;
	}
}

// triangulate a Triangle Brush that has 3 sides of which one is !upright!
void brush::TriTriangle()
{
	brush &Brush = *this;
	
	if (Brush.valid&&Brush.draw&&Brush.IsDivisible&&!Brush.IsWedge&&Brush.t_faces==5)
	{
		Brush.Tri = new brush[2];
		Brush.t_tri = 2;
		brush &Wedge1 = Brush.Tri[0];
		brush &Wedge2 = Brush.Tri[1];
		face *Top[2] 	= {nullptr,nullptr};
		face *Btm[2] 	= {nullptr,nullptr};
		face *Front[2] 	= {nullptr,nullptr};
		face *Back[2] 	= {nullptr,nullptr};
		Wedge1.Copy(Brush);
		Wedge2.Copy(Brush);
		bool HasFront = 0;
		bool HasBack = 0;
		
		for (int f = 0; f<5; f++)
		{
			face &Face1 = Wedge1.Faces[f];
			face &Face2 = Wedge2.Faces[f];
			
			if 		(Face1.Orient==2) {Top[0] = &Face1;}
			else if (Face1.Orient==3) {Btm[0] = &Face1;}
			else if (Face1.Orient==5) {Back[0] = &Face1; HasBack=1; Wedge1.HasBack=1; Wedge2.HasBack=1;}
			else if (Face1.Orient==4) {Front[0] = &Face1;}
			
			if 		(Face2.Orient==2) {Top[1] = &Face2;}
			else if (Face2.Orient==3) {Btm[1] = &Face2;}
			else if (Face2.Orient==4) {Front[1] = &Face2; HasFront=1;}
		}
		face &Wedge1_Top 	= *Top[0];
		face &Wedge1_Btm 	= *Btm[0];
		face &Wedge1_Back 	= *Back[0];
		face &Wedge1_Front 	= *Front[0];
		face &Wedge2_Top 	= *Top[1];
		face &Wedge2_Btm 	= *Btm[1];
		face &Wedge2_Front 	= *Front[1];
		
		if (HasFront)
		{
			// fix wedge 1
			Wedge1.Faces[1].Vertices[0] = Wedge1.Faces[0].Vertices[2];
			swap(Wedge1_Top.Vertices[2],Wedge1_Top.Vertices[3]);
			Wedge1_Top.vcount = 3;
			Wedge1_Btm.vcount = 3;
			
			// fix wedge 2
			Wedge2.IsWedge2 = 1;
			Wedge2_Front.draw = 0;
			Wedge2.Faces[0].Vertices[0] = Wedge2.Faces[1].Vertices[2];
			Wedge2.Faces[0].Vertices[1] = Wedge2.Faces[1].Vertices[1];
			Wedge2_Top.pushVerts(3);
			Wedge2_Btm.pushVerts(2);
			Wedge2_Top.vcount = 3;
			Wedge2_Btm.vcount = 3;
			
			// new tri faces textures and tex align
			Wedge1.Faces[1].fID = 2;
			Wedge1.Faces[1].Texture = def_nulltex;
			Wedge1.Faces[1].Orient = 6;
			AlignToWorld(Wedge1.Faces[1]);
			
			Wedge2.Faces[0].fID = 2;
			Wedge2.Faces[0].Texture = def_nulltex;
			Wedge2.Faces[0].Orient = 6;
			AlignToWorld(Wedge2.Faces[0]);
		}
		
		if (HasBack)
		{
			// fix wedge 1
			Wedge1.Faces[1].draw = 0; // remove relevant cutting edge face to form a triangle of this brush
			Wedge1_Back.Vertices[2]	= Wedge1.Faces[1].Vertices[2];
			swap(Wedge1_Top.Vertices[2], Wedge1_Top.Vertices[3]); // swap vertices to fix vertex order
			Wedge1_Top.vcount = 3;
			Wedge1_Btm.vcount = 3;
			Wedge1_Back.vcount = 3;
			
			// fix wedge 2
			Wedge2.IsWedge2 = 1;
			Wedge2.Faces[0].Vertices[0] = Wedge2.Faces[1].Vertices[2];
			Wedge2_Top.pushVerts(3);
			Wedge2_Btm.pushVerts(2);
			Wedge2_Top.vcount = 3;
			Wedge2_Btm.vcount = 3;
			
			// new tri faces textures and tex align
			Wedge1_Back.fID = 2;
			Wedge1_Back.Texture = def_nulltex;
			Wedge1_Back.Orient = 6;
			AlignToWorld(Wedge1_Back);

			Wedge2.Faces[0].fID = 2;
			Wedge2.Faces[0].Texture = def_nulltex;
			Wedge2.Faces[0].Orient = 6;
			AlignToWorld(Wedge2.Faces[0]);
		}
		
		// new Face Edge IDs
		Wedge1_Top.SetEdges(0,2,0,1,2,1);
		Wedge2_Top.SetEdges(1,0,1,2,0,2);
		Wedge1_Btm.SetEdges(1,2,1,0,2,0);
		Wedge2_Btm.SetEdges(1,2,1,0,2,0);
		
		Wedge1.MarkFaceVertices(Brush.Faces[1], 1, 1);
		Wedge2.MarkFaceVertices(Brush.Faces[1], 1, 1);
		
		delete Wedge2.Gap; Wedge2.Gap = nullptr;
	}
}

void brush::TriComplex()
{
	#if DEBUG > 0
	bool dev = 0;
	if (dev) cout << " Now triangulating Complex Brush..." << endl;
	#endif
	
	brush &Brush = *this;
	
	if (Brush.valid&&Brush.draw)
	{
		// triangulate this brush no matter what its made of
		// count triangles
		int newBrushCount = 0; // minimum 2 brushes for the head and base faces
		for (int f = 0; f<Brush.t_faces; f++)
		{
			face &Face = Brush.Faces[f];
			if (Face.fID==0||Face.fID==1)
			{
				if (!cTable[gID].round || (cTable[gID].round&&Face.vcount==3) ) {
					newBrushCount += 1;
				} else {
					newBrushCount += Face.vcount-2;
				}
			}
			if (Face.fID==2)
			{
				newBrushCount+=2;
			}
		}
		
		#if DEBUG > 0
		if (dev) { cout << "  New Brush Count: " << newBrushCount << endl; }
		#endif
		
		Brush.Tri = new brush[newBrushCount];
		Brush.t_tri = newBrushCount;
		Brush.Faces[0].GetCentroid();
		Brush.Faces[1].GetCentroid();
		
		// Head and Base Face
		#if DEBUG > 0
		if (dev) { cout << "  Turning Head and Base Face into Brushes..." << endl; }
		#endif
		
		if (!cTable[gID].round||Brush.Faces[0].vcount==3) {
			brush* Base = Face2Brush(Brush.Faces[0]);
			brush* Head = Face2Brush(Brush.Faces[1]);
			Brush.Tri[0].CopySimple(*Base);
			Brush.Tri[1].CopySimple(*Head);
			delete Base;
			delete Head;
		}
		else
		{
			// if round = 1 base and head faces are turned into a "triangle fan", consisting of many brush spikes instead of just one spike brush
			#if DEBUG > 0
			if (dev) { cout << "    Round is ON. Turning Head Face into triangle bridge..." << endl; }
			#endif
			
			for (int tb = 0, ov = 0; ov<Brush.Faces[0].vcount-1; ov++) {
				if (ov!=1)
				{
					#if DEBUG > 0
					if (dev) { cout << "      Tri Brush " << tb << " Original Face Vertex " << ov << " of total " << Brush.Faces[0].vcount << endl; }
					#endif
					
					brush* BaseFan = Face2BrushTriBridge(Brush.Faces[0], ov);
					Brush.Tri[tb].CopySimple(*BaseFan);
					delete BaseFan;
					tb++;
				}
			}
			for (int tb = Brush.Faces[0].vcount-2, ov=0; ov<Brush.Faces[1].vcount-1; ov++) {
				if (ov!=1)
				{
					#if DEBUG > 0
					if (dev) { cout << "      Tri Brush " << tb << " Original Face Vertex " << ov << " of total " << Brush.Faces[1].vcount << endl; }
					#endif
					
					brush* HeadFan = Face2BrushTriBridge(Brush.Faces[1], ov);
					Brush.Tri[tb].CopySimple(*HeadFan);
					delete HeadFan;
					tb++;
				}
			}
		}
		
		// Body Faces
		#if DEBUG > 0
		if (dev) { cout << "  Turning Body Faces into Brushes..." << endl; }
		#endif
		
		int v = 2;
		if (cTable[gID].round&&Brush.Faces[0].vcount!=3) v=(Brush.Faces[0].vcount-2)*2;
		for (int f = 2; f<Brush.t_faces; f++)
		{
			face &Face = Brush.Faces[f];
			#if DEBUG > 0
			if (dev) cout << "   Face " << f << " of total " << Brush.t_faces << endl;
			#endif
			
			if (Face.fID==2)
			{
				#if DEBUG > 0
				if (dev) { cout << "     Tri Brush " << v << " ( of" << Brush.t_tri << ") Creating 2 Triangle Brushes..." << endl; }
				#endif
				
				brush* Wedge1 = Face2BrushTri(Brush.Faces[f], 0);
				
				#if DEBUG > 0
				if (dev) { cout << "       created Wedge 1 " << endl; }
				#endif
				
				brush* Wedge2 = Face2BrushTri(Brush.Faces[f], 1);
				
				#if DEBUG > 0
				if (dev) { cout << "       created Wedge 2 " << endl; }
				#endif

				#if DEBUG > 0
				if (dev) { cout << "     Copying Simple..." << endl; }
				#endif
				
				Brush.Tri[v].CopySimple  ( *Wedge1 );
				Brush.Tri[v+1].CopySimple( *Wedge2 );

				#if DEBUG > 0
				if (dev) { cout << "     Marking Vertices..." << endl; }
				#endif
				
				Brush.Tri[v].MarkFaceVertices(Brush.Faces[1], 1, 1);
				Brush.Tri[v+1].MarkFaceVertices(Brush.Faces[1], 1, 1);
				
				#if DEBUG > 0
				if (dev) { cout << "     Defining Wedge..." << Brush.Tri[v].Faces[0].EdgeH << " - " << Brush.Tri[v+1].Faces[0].EdgeH << endl; }
				#endif
				
				if(GetVecLen(Brush.Tri[v].Faces[0].EdgeH) < GetVecLen(Brush.Tri[v+1].Faces[0].EdgeH))
				Brush.Tri[v+1].IsWedge2 = 1;
				else
				Brush.Tri[v].IsWedge2 = 1;
				
				delete Wedge1;
				delete Wedge2;
				
				v+=2;
				
				#if DEBUG > 0
				if (dev) { cout << "     Finished, now Brush " << v << endl; }
				#endif
			}
		}
	}
}

void brush::GetSourceFaces()
{
	brush &Brush = *this;
	
	#if DEBUG > 0
	bool dev = 0;
	if (dev) cout << " Getting SOurce Faces of Brush " << Brush.name << endl;
	#endif
	
	for (int f=0,i=0; f<Brush.t_faces; f++)
	{
		face &Face = Brush.Faces[f];
		if(Face.fID==2)
		{
			#if DEBUG > 0
			if (dev) cout << " Face "<<f<<" LenL " << Face.EdgeLenL<<" LenS " << Face.EdgeLenS << endl;
			#endif
			
			if(i==0) { Brush.HSourceL = &Face; Brush.HSourceS = &Face; i++; }
			else
			{
				if(Face.EdgeLenL > Brush.HSourceL->EdgeLenL) { Brush.HSourceL = &Face; }
				if(Face.EdgeLenS < Brush.HSourceS->EdgeLenS) { Brush.HSourceS = &Face; }
			}
		}
	}
	
	#if DEBUG > 0
	if (dev) cout << "  HSourceL " << Brush.HSourceL->EdgeLenL << endl;
	if (dev) cout << "  HSourceS " << Brush.HSourceS->EdgeLenS << endl << endl;
	#endif
}

void brush::GetFaceOrients()
{
	//determine Orientation of all body Faces

	#if DEBUG > 0
	bool dev = 0;
	bool devtex = 0;
	if (dev) cout << " determine Orientation of all body Faces..." << endl;
	#endif
	
	// 0 = Left, 1 = Right, 2 = Top, 3 = Down, 4 = Front, 5 = Back
	brush &Brush = *this;
	if (Brush.valid)
	{
		for (int f = 0; f<Brush.t_faces; f++)
		{
			face &Face = Brush.Faces[f];
			
			// NEW2024 Face-Pitches for face-groups source-edge detection later on
			vertex RV0;
			vertex RV1; RV1.setall(0,-1,0);
			gvector Vec1; Vec1.set(RV0,RV1);
			Face.PitchO = GetDot(Face.Normal,Vec1); //1=Front, -1=Back, 0=Up/Down
			
			#if DEBUG > 0
			if (dev) cout << " Face " << f << " Face.PitchO " << Face.PitchO << " Face.PitchN " << Face.PitchN << " Tex "<< Face.Texture << " Orient " << Face.Orient << " Normal " << Face.Normal << endl;
			#endif
			
			if (Face.fID==2)
			{
				if (Face.Normal.z>0.001)
				{
					Face.Orient = 2;
					
					#if DEBUG > 0
					if (devtex) Face.Texture+="_UP";
					#endif
				}
				else if (Face.Normal.z<-0.001) // Top / Down Faces
				{
					Face.Orient = 3;
					
					#if DEBUG > 0
					if (devtex) Face.Texture+="_DN";
					#endif
				}
				else if (Face.Normal.y>0.999) // Front Back Faces
				{
					Face.Orient = 5;
					
					#if DEBUG > 0
					if (devtex) Face.Texture+="_BK";
					#endif
				}
				else if (Face.Normal.y<-0.999)
				{
					Face.Orient = 4;
					
					#if DEBUG > 0
					if (devtex) Face.Texture+="_FT";
					#endif
				}
				
				#if DEBUG > 0
				if (dev) cout << "     New Orient [" << Face.Orient << "]" << endl;
				if (dev) cout << "     Normal " << Face.Normal << endl << endl;
				#endif
			}
		}
		// if top and down faces cant be identified but front/back faces were and brush qualifies as a potential Cuboid (6 faces):
	}
}

bool brush::CheckValidity()
{
	#if DEBUG > 0
	bool dev = 0;
	bool devtex = 0;
	#endif
	
	brush &Brush = *this;
	Brush.BaseID=-1;
	Brush.HeadID=-1;

	#if DEBUG > 0
	if(dev) cout << "  Checking Validity of Brush " << Brush.SegID<< endl;
	#endif
	
	int ctr_base = 0;
	int ctr_head = 0;
	int ctr_body = 0;
	
	// look for base and head faces. everything else must be a body face (supposedly)
	// also check of faces are planar, because Face Normals are calculated of only 3 vertices
	for(int f = 0; f<Brush.t_faces; f++)
	{
		face &Face = Brush.Faces[f];
		Face.fID = 0;
		Face.Orient = 0;
		Face.GetNormal();
		Face.GetPlanarity();

		#if DEBUG > 0
		if(dev) cout << "   Face " << f << " Tex " << Face.Texture << " Normal " << Face.Normal << " Planarity " << Face.IsPlanar << endl;
		#endif
		
		if (Face.Normal.x>0.999&&Face.IsPlanar)
		{
			Face.Orient = 1;
			Face.fID = 1;
			ctr_head++;
			Brush.HeadID = f;

			#if DEBUG > 0
			if (devtex)
			Face.Texture += "_RT";
			#endif
		}
		else if (Face.Normal.x<-0.999&&Face.IsPlanar)
		{
			Face.Orient = 0;
			Face.fID = 0;
			ctr_base++;
			Brush.BaseID = f;
			
			#if DEBUG > 0
			if (devtex)
			Face.Texture += "_LF";
			#endif
		}
		else if (Face.Normal.x>-0.001&&Face.Normal.x<0.001&&Face.IsPlanar)
		{
			ctr_body++;
			Face.fID = 2;
			
			#if DEBUG > 0
			if (devtex)
			Face.Texture += "_BODY";
			#endif
		}
	}
	
	#if DEBUG > 0
	if(dev) cout << " Brush " << Brush.name << " Body Faces " << ctr_body << " HeadID " << Brush.HeadID << " BaseID " << Brush.BaseID << endl << endl;
	if(dev) system("pause");
	#endif
	
	// check if base and headface vertices are mirroring each other (only works when Brush Faces have been reconstructed)
	bool Mirror = 0;
	if (Brush.BaseID!=-1&&Brush.HeadID!=-1&&Brush.BaseID!=Brush.HeadID)
	{
		face &Base = Brush.Faces[Brush.BaseID];
		face &Head = Brush.Faces[Brush.HeadID];
		
		#if DEBUG > 0
		if(dev) cout << "  Checking if " << Base.vcount << " base and " << Head.vcount <<" head vertices of Brush " << Brush.name << " RCON " << Brush.RCON << " are mirroring each other..." << endl;
		#endif
		
		if (Brush.RCON && Base.vcount==Head.vcount)
		{
			int mCtr = 0;
			for (int b = 0; b<Base.vcount; b++)
			{
				vertex &B = Base.Vertices[b];
				for (int h = 0; h<Base.vcount; h++)
				{
					vertex &H = Head.Vertices[h];
					if (B.x!=H.x&&B.y==H.y&&B.z==H.z) {
						mCtr++;
						
						#if DEBUG > 0
						if(dev) cout << "  Checking Base Vert " << b << B << " and Head Vert " << h << H << " MATCH! Ctr now " << mCtr << endl;
						#endif
					}
					else
					{
						#if DEBUG > 0
						if(dev) cout << "  Checking Base Vert " << b << B << " and Head Vert " << h << H << endl;
						#endif
					}
				}
			}
			if (mCtr==Base.vcount) Mirror = 1;
		}
		else if (Brush.RCON && Base.vcount!=Head.vcount) Mirror = 0;
		else if (!RCON) Mirror = 1; // if Brushes arent reconstructed yet, theres no point in checking for mirrored vertices
	}
	#if DEBUG > 0
	if (dev) system("pause");
	if(dev) {
		cout << " ctr_head " << ctr_head << " ctr_base " << ctr_base << " ctr_body " << ctr_body << " tfaces " << Brush.t_faces << " Mirror " << Mirror << endl;
	}
	#endif
	
	if (ctr_head!=1||ctr_base!=1||ctr_body!=Brush.t_faces-2||!Mirror)
	{
		Brush.valid = 0;
		cout << "|  [WARNING] Brush ["<<Brush.bID<<"] of Entity ["<<Brush.entID<<"] seems to have an invalid mesh and won't be processed." << endl;
		cout << "|" << endl;
		return false;
	}
	else return true;
}




/* ===== BRUSH FUNCTIONS ===== */


// this turns planar faces with any amount of vertices into a single brush
// by keeping that one face and spanning up a tent of NULL faces behind it
// used for head or base faces
brush* Face2Brush(face &SrcFace)
{
	#if DEBUG > 0
	bool dev = 0;
	if (dev) cout << " Converting a Face to a Brush (Spike)..." << endl;
	#endif
	
	brush* BrushPtr = new brush;
	brush& Brush = *BrushPtr;
	
	#if DEBUG > 0
	if (dev) cout << "  Creating "<<SrcFace.vcount+1<<" new faces for this Spike..." << endl;
	#endif
	
	Brush.IsSpike = 1;
	Brush.Faces = new face[SrcFace.vcount+1];
	Brush.t_faces = SrcFace.vcount+1;

	#if DEBUG > 0
	if (dev) cout << "  Copying Source Face ("<<Brush.Faces[0].Texture<<") into first Spike face..." << endl;
	#endif
	
	Brush.Faces[0].CopyFace(SrcFace,1);
	
	#if DEBUG > 0
	if (dev) cout << "  Creating Tent Faces (NULL-Faces) now..." << endl;
	#endif
	
	Brush.CreateTent();
	
	return BrushPtr;
}

brush* Face2BrushTri(face &SrcFace, int Side)
{
	#if DEBUG > 0
	bool dev = 0;
	if (dev) { cout << " Converting a Face to a Triangle Brush..." << endl; }
	#endif
	
	brush* BrushPtr = new brush;
	brush& Brush = *BrushPtr;
	
	Brush.IsSpike = 1;
	Brush.Faces = new face[4];
	Brush.t_faces = 4;
	
	#if DEBUG > 0
	if (dev) { cout << "    Copying Face..." << endl; }
	#endif
	
	Brush.Faces[0].CopyFace(SrcFace,1);
	
	if (Side==0)
	{
		if(Brush.Faces[0].vcount==4)
		swap(Brush.Faces[0].Vertices[2],Brush.Faces[0].Vertices[3]);
		
		Brush.Faces[0].SetEdges(2,0,1,0,2,1);
	}
	else
	{
		Brush.Faces[0].pushVerts(3);
		Brush.Faces[0].SetEdges(1,0,1,2,2,0);
	}
	#if DEBUG > 0
	if (dev) cout << "  Set Face Edges EdgeH " << Brush.Faces[0].EdgeH << " EdgeV " << Brush.Faces[0].EdgeV << " Hypo " << Brush.Faces[0].Hypo <<endl;
	#endif
	
	Brush.Faces[0].vcount = 3;
	Brush.CreateTent();
	
	#if DEBUG > 0
	if (dev) cout << endl;
	#endif
	
	return BrushPtr;
}

brush* Face2BrushTriBridge(face &SrcFace, int VID)
{
	#if DEBUG > 0
	bool dev = 0;
	if (dev) cout << " Converting a Face with fID "<< SrcFace.fID << " to a Triangle Bridge Brush..." << endl;
	#endif
	
	brush* BrushPtr = new brush;
	brush& Brush = *BrushPtr;
	
	Brush.IsSpike = 1;
	Brush.Faces = new face[4];
	Brush.t_faces = 4;
	
	Brush.Faces[0].CopyFace(SrcFace,0);
	Brush.Faces[0].Vertices = new vertex[3];
	Brush.Faces[0].vcount = 3;
	vertex &VO0 = Brush.Faces[0].Vertices[0];
	vertex &VO1 = Brush.Faces[0].Vertices[1];
	vertex &VO2 = Brush.Faces[0].Vertices[2];
	
	VO0 = SrcFace.Vertices[0];
	if (VID==0) {
	VO1 = SrcFace.Vertices[1];
	VO2 = SrcFace.Vertices[2];
	} else if (VID>=2) {
	VO1 = SrcFace.Vertices[VID];
	VO2 = SrcFace.Vertices[VID+1];
	}
	
	Brush.CreateTent();
	
	#if DEBUG > 0
	if (dev) cout << endl;
	#endif
	
	return BrushPtr;
}

brush* Face2BrushTriFan(face &SrcFace, int VID)
{
	#if DEBUG > 0
	bool dev = 0;
	if (dev) cout << " Converting a Face with fID "<< SrcFace.fID << " to a Triangle Fan Brush..." << endl;
	#endif
	
	brush* BrushPtr = new brush;
	brush& Brush = *BrushPtr;
	
	Brush.IsSpike = 1;
	Brush.Faces = new face[4];
	Brush.t_faces = 4;
	
	Brush.Faces[0].CopyFace(SrcFace,0);
	Brush.Faces[0].Vertices = new vertex[3];
	Brush.Faces[0].vcount = 3;
	vertex &VO0 = Brush.Faces[0].Vertices[0];
	vertex &VO1 = Brush.Faces[0].Vertices[1];
	vertex &VO2 = Brush.Faces[0].Vertices[2];
	
	SrcFace.GetCentroid();
	if (VID==SrcFace.vcount-1) {
	VO0 = SrcFace.Vertices[VID];
	VO1 = SrcFace.Vertices[0];
	} else {
	VO0 = SrcFace.Vertices[VID];
	VO1 = SrcFace.Vertices[VID+1];
	}
	VO2 = SrcFace.Centroid;
	
	Brush.CreateTent();
	
	#if DEBUG > 0
	if (dev) cout << endl;
	#endif
	
	return BrushPtr;
}



void ExportBrushToOBJ(string Output, brush &Brush)
{
	ofstream objfile;
	objfile.open(Output+"_Brush_"+Brush.name+".obj");
	
	objfile << "g brush_export" << endl;
	
	for (int f = 0; f < Brush.t_faces; f++)
	{
		face &Face = Brush.Faces[f];
		int tverts = Face.vcount;
		
		if (Face.draw)
		{
			for(int v = 0; v < tverts; v++)
			{
				vertex &V = Face.Vertices[v];
				objfile << "v " << V.x << " " << V.y << " " << V.z << " ";
				
				// FOR DEV PURPOSES: color this vertex red
				if(V.DoSplit) objfile << "1 0 0" << endl;
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
	
	objfile.close();
}



brush* CreateCube(int size)
{
	brush *Testcube_Ptr = new brush;
	brush &Testcube = *Testcube_Ptr;
	
	Testcube.Faces = new face[6];
	Testcube.t_faces = 6;
	for (int f = 0; f<6; f++) {
		face &Face = Testcube.Faces[f];
		Face.vcount = 4;
		Face.Vertices = new vertex[4];
		Face.Texture = "NULL";
	}
	float size_half = size/2;
	
	Testcube.Faces[0].Vertices[3].setall(-size_half,-size_half,0);
	Testcube.Faces[0].Vertices[2].setall(-size_half,size_half,0);
	Testcube.Faces[0].Vertices[1].setall(size_half,size_half,0);
	Testcube.Faces[0].Vertices[0].setall(size_half,-size_half,0);
	
	Testcube.Faces[1].Vertices[0].setall(-size_half,size_half,size);
	Testcube.Faces[1].Vertices[1].setall(size_half,size_half,size);
	Testcube.Faces[1].Vertices[2].setall(size_half,-size_half,size);
	Testcube.Faces[1].Vertices[3].setall(-size_half,-size_half,size);
	
	Testcube.Faces[2].Vertices[0].setall(-size_half,-size_half,0);
	Testcube.Faces[2].Vertices[1].setall(-size_half,-size_half,size);
	Testcube.Faces[2].Vertices[2].setall(size_half,-size_half,size);
	Testcube.Faces[2].Vertices[3].setall(size_half,-size_half,0);
	
	Testcube.Faces[3].Vertices[0].setall(size_half,-size_half,0);
	Testcube.Faces[3].Vertices[1].setall(size_half,-size_half,size);
	Testcube.Faces[3].Vertices[2].setall(size_half,size_half,size);
	Testcube.Faces[3].Vertices[3].setall(size_half,size_half,0);
	
	Testcube.Faces[4].Vertices[0].setall(size_half,size_half,0);
	Testcube.Faces[4].Vertices[1].setall(size_half,size_half,size);
	Testcube.Faces[4].Vertices[2].setall(-size_half,size_half,size);
	Testcube.Faces[4].Vertices[3].setall(-size_half,size_half,0);
	
	Testcube.Faces[5].Vertices[0].setall(-size_half,size_half,0);
	Testcube.Faces[5].Vertices[1].setall(-size_half,size_half,size);
	Testcube.Faces[5].Vertices[2].setall(-size_half,-size_half,size);
	Testcube.Faces[5].Vertices[3].setall(-size_half,-size_half,0);
	
	for (int f = 0; f<6; f++) {
		face &Face = Testcube.Faces[f];
		AlignToWorld(Face);
	}
	
	return Testcube_Ptr;
}


void brush::VecToBrush(gvector &Vec, gvector Normal, string Tex)
{
	#if DEBUG > 0
	bool dev = 0;
	#endif
	
	Normal.mult(4);
	
	brush &Brush = *this;
	vertex Origin(Vec.px,Vec.py,Vec.pz);
	
	Brush.Faces = new face[6];
	Brush.t_faces = 6;
	
	face &F1 = Brush.Faces[0];
	face &F2 = Brush.Faces[1];
	face &F3 = Brush.Faces[2];
	face &F4 = Brush.Faces[3];
	face &F5 = Brush.Faces[4];
	face &F6 = Brush.Faces[5];
	
	for (int f = 0; f<6; f++) {
		face &Face = Brush.Faces[f];
		Face.vcount = 4;
		Face.Vertices = new vertex[4];
		Face.Texture = Tex;
		
		#if DEBUG > 0
		if (dev) cout << " face #" << f<< " tex " << Face.Texture << endl;
		#endif
	}
	#if DEBUG > 0
	if (dev) cout << " Creating Face 0 Vertices..." << endl;
	#endif
	
	// Base Face
	vertex &V1 = F1.Vertices[0];
	vertex &V2 = F1.Vertices[1];
	vertex &V3 = F1.Vertices[2];
	vertex &V4 = F1.Vertices[3];
	
	V1 = Origin + Normal;
	V3 = Origin - Normal;
	gvector Cross1 = Normalize( GetCross( Vec, Normal ) );
	Cross1.mult(4);
	V2 = Origin + Cross1;
	V4 = Origin - Cross1;

	#if DEBUG > 0
	if (dev) {
	cout << " V1 = Origin " << Origin << " + Normal " << Normal << " = " << V1 << endl;
	cout << " V2 = Origin " << Origin << " - Normal " << Normal << " = " << V2 << endl;
	cout << " V3 = Origin " << Origin << " + Cross1 " << Cross1 << " = " << V3 << endl;
	cout << " V4 = Origin " << Origin << " - Cross1 " << Cross1 << " = " << V4 << endl;
	cout << " Align Face 0 to World..." << endl;
	system("pause");}
	#endif
	
	AlignToWorld(F1);
	
	#if DEBUG > 0
	if (dev) { cout << " Copy Face 0 to Face 1 and move it by Distance of Vec " << Vec << "..." << endl;
	system("pause"); }
	#endif
	
	// Head Face
	F2.CopyFace(F1,1);
	F2.Move(Vec);
	AlignToWorld(F2);
	
	vertex &V1_2 = F2.Vertices[0];
	vertex &V2_2 = F2.Vertices[1];
	vertex &V3_2 = F2.Vertices[2];
	vertex &V4_2 = F2.Vertices[3];
	
	#if DEBUG > 0
	if (dev) {cout << " Create Face 2 to 5 from Face 0 and 1...." << endl;
	system("pause");}
	#endif
	
	// Body Faces
	vertex F3_List[4] = { V1, V2, V2_2, V1_2 };
	vertex F4_List[4] = { V2, V3, V3_2, V2_2 };
	vertex F5_List[4] = { V3, V4, V4_2, V3_2 };
	vertex F6_List[4] = { V4, V1, V1_2, V4_2 };
	
	F3.SetFace(4,F3_List,Tex);
	F4.SetFace(4,F4_List,Tex);
	F5.SetFace(4,F5_List,Tex);
	F6.SetFace(4,F6_List,Tex);
	
	F3.RevOrder(0);
	F4.RevOrder(0);
	F5.RevOrder(0);
	F6.RevOrder(0);
	
	AlignToWorld(F3);
	AlignToWorld(F4);
	AlignToWorld(F5);
	AlignToWorld(F6);
	
	// Fix Vertex Order of Head Face
	F2.RevOrder(0);
}





ostream &operator<<(ostream &ostr, brush &Brush)
{
	ostr << endl << " ******** Printing Brush ********" << endl;
	ostr << "  Name: " << Brush.name << endl;
	ostr << "  SecID: " << Brush.SecID << endl;
	ostr << "  SegID: " << Brush.SegID << endl;
	ostr << "  Draw: " << Brush.draw << endl;
	ostr << "  Faces: " << Brush.t_faces << endl;
	for(int f=0; f<Brush.t_faces; f++) {
		ostr << "   #" << f << " " << Brush.Faces[f] << endl;
	}
	
	return ostr;
}




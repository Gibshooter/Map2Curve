#include "brush.h"
#include "face.h"
#include "settings.h"
#include "vertex.h"

#include <string>
#include <conio.h> // getch

using namespace std;

struct face;
struct circleset;
struct vertex;

extern ctable *cTable;
extern float def_spikesize;
extern string def_nulltex;

/* ===== DIMENSIONS METHODS & FUNCTIONS ===== */

void dimensions::set(float a, float b, float c)
{
	xs = a;
	xb = a;
	ys = b;
	yb = b;
	zs = c;
	zb = c;
}

void dimensions::expand(int size)
{
	xs = (floor(xs/size)*size)-size;
	xb = (ceil(xb/size)*size)+size;
	ys = (floor(ys/size)*size)-size;
	yb = (ceil(yb/size)*size)+size;
	zs = (floor(zs/size)*size)-size;
	zb = (ceil(zb/size)*size)+size;
}

dimensions DimensionCombine(dimensions D1, dimensions D2)
{
	dimensions D3;
	
	if (D1.xs < D2.xs) D3.xs = D1.xs; else D3.xs = D2.xs;
	if (D1.xb > D2.xb) D3.xb = D1.xb; else D3.xb = D2.xb;
	if (D1.ys < D2.ys) D3.ys = D1.ys; else D3.ys = D2.ys;
	if (D1.yb > D2.yb) D3.yb = D1.yb; else D3.yb = D2.yb;
	if (D1.zs < D2.zs) D3.zs = D1.zs; else D3.zs = D2.zs;
	if (D1.zb > D2.zb) D3.zb = D1.zb; else D3.zb = D2.zb;
	
	return D3;
}

ostream &operator<<(ostream &ostr, dimensions &D)
{
	return ostr << "( XS " << D.xs << " XB " << D.xb << " YS " << D.ys << " YB " << D.yb << " ZS " << D.zs << " ZB " << D.zb << " )";
}

/* ===== BRUSH METHODS ===== */

brush::~brush()
{
	delete[] Faces;
	delete[] vlist;
	delete[] Tri;
	delete Gap;
	delete cset;
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

void brush::Scale(float n)
{
	brush &Brush = *this;
	for (int f = 0; f<Brush.t_faces; f++)
	{
		face &Face = Brush.Faces[f];
		if (Face.draw)
		{
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

void brush::ScaleOrigin(float n, vertex Origin)
{
	this->Move(-Origin.x, -Origin.y, -Origin.z,1);
	
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
	
	this->Move(Origin.x, Origin.y, Origin.z,1);
}

void brush::Move(float x, float y, float z, bool fixShifts)
{
	bool dev = 0;
	
	for (int f = 0; f<t_faces; f++)
	{
		face &Face = Faces[f];
		if (Face.draw)
		{
			if (dev) cout << " Attempt to move Face " << f << " Tex " << Face.Texture << " draw " << Face.draw << endl;
			/*for (int v = 0; v<Face.vcount; v++)
			{
				vertex &V = Face.Vertices[v];
				cout << "   Vertex " << v << V << endl;
			}*/
			if (fixShifts) {
			if (Face.BaseListX[0]==0&&Face.BaseListX[3]==0)
			{GetBaseEdges(Face);}
			//cout << "    New Base Edges: BaseX1 " << Face.BaseListX[0] << " BaseX2 " << Face.BaseListX[3] << " BaseY1 " << Face.BaseListY[0] << " BaseY2 " << Face.BaseListY[3] << endl;
			GetBaseShift(Face,0,1,0);
			//cout << "    New BaseShifts " << Face.BaseShiftX << ", " << Face.BaseShiftY << endl;
			GetTexOffset(Face,0);
			//cout << "    New Offsets " << Face.OffsetX << ", " << Face.OffsetY << endl;
			}
			for (int v = 0; v<Face.vcount; v++)
			{
				vertex &V = Face.Vertices[v];
				
				if (dev) cout << "    Moving vertex " << v << V << " by " << x << ", " << y << ", " << z << endl;
				V.move(x,y,z);
			}
			//cout << "    Get BaseShift... old (" << Face.BaseShiftX << ", " << Face.BaseShiftY << ")" << endl;
			if (fixShifts) {
			GetBaseShift(Face,0,1,0);
			Face.ShiftX = Face.BaseShiftX + Face.OffsetX;
			Face.ShiftY = Face.BaseShiftY + Face.OffsetY;
			//cout << "    New Face Shifts " << Face.ShiftX << ", " << Face.ShiftY << ")" << endl << endl;
			//getch();
			}
		}
	}
}

void brush::Rot(float x, float y, float z)
{
	for (int f = 0; f<t_faces; f++)
	{
		face &Face = Faces[f];
		if (Face.draw)
		{
			//GetTexOffset(Face,0);
			for (int v = 0; v<Face.vcount; v++)
			{
				vertex &V = Face.Vertices[v];
				
				V.rotate(x,y,z);
			}
			Face.VecX.rotate(x,y,z);
			Face.VecY.rotate(x,y,z);
			//GetBaseShift(Face,0,1,0);
			//Face.ShiftX = Face.BaseShiftX + Face.OffsetX;
			//Face.ShiftY = Face.BaseShiftY + Face.OffsetY;
		}
	}
}

void brush::RotOrigin(float x, float y, float z, vertex Origin)
{
	//this->Move(-Origin.x, -Origin.y, -Origin.z);
	bool dev = 0;
	
	for (int f = 0; f<t_faces; f++)
	{
		face &Face = Faces[f];
		if (Face.draw)
		{
			if (dev) cout << " Face " << f << " Tex " << Face.Texture << " Fixing Shift after Rotation..." << endl;
			if (dev) cout << "   Old BaseShiftX " << Face.BaseShiftX << endl;
			if (dev) cout << "   Old BaseShiftY " << Face.BaseShiftY << endl;
			if (dev) cout << "   Rotating Vertices... ("<<Face.vcount<<")" << endl;
			for (int v = 0; v<Face.vcount; v++){
				vertex &V = Face.Vertices[v];
				//cout << "   Brush " << this->name << "Face " << f << " Vertex OLD " << v << V;
				V.rotateOrigin(x,y,z, Origin);
				//cout << " NEW " << V << endl;
			}
			if (dev) cout << "   Rotating VerticesC... ("<<Face.vcountC<<")" << endl;
			for (int v = 0; v<Face.vcountC; v++){
				vertex &VC = Face.VerticesC[v];
				VC.rotateOrigin(x,y,z, Origin);
			}
			if (dev) cout << "   Rotating Centroid... " << Face.Centroid << endl;
			Face.Centroid.rotateOrigin(x,y,z, Origin);
			if (dev) cout << "   Centroid now " << Face.Centroid << endl;
			if (dev) cout << "   Rotating Tex Vectors... " << endl;
			Face.VecX.rotate(x,y,z);
			Face.VecY.rotate(x,y,z);
			
			if (dev) cout << "   Getting Edges and BaseShift... " << endl;
			GetBaseShift(Face,0,1,0);
			if (dev) cout << "   New BaseShiftX " << Face.BaseShiftX << endl;
			if (dev) cout << "   New BaseShiftY " << Face.BaseShiftY << endl;
			if (dev) cout << "   OffsetX " << Face.OffsetX << endl;
			if (dev) cout << "   OffsetY " << Face.OffsetY << endl;
			Face.ShiftX = Face.BaseShiftX + Face.OffsetX;
			Face.ShiftY = Face.BaseShiftY + Face.OffsetY;
			if (dev) cout << "   New ShiftX " << Face.ShiftX << endl;
			if (dev) cout << "   New ShiftY " << Face.ShiftY << endl;
		}
	}
	//this->Move(Origin.x, Origin.y, Origin.z);
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
	step 	= Source.step;
	pID 	= Source.pID;
	Align	= Source.Align;
	name	= Source.name + "_copy";
	IsSpike	= Source.IsSpike;
	RCON	= Source.RCON;
	gID		= Source.gID;
	dID		= Source.dID;
	entID 	= Source.entID;
	
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
		
		Face.BaseListX.resize(4);
		Face.BaseListY.resize(4);
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

		for (int v = 0; v<Face.vcount; v++)
		{
			vertex &OVert = OFace.Vertices[v];
			vertex &Vert = Face.Vertices[v];
			Vert = OVert;
		}
	}
}

void brush::MakeCuboid(dimensions Box, string Tex)
{
	Faces[0].Vertices[2].setall(Box.xs,Box.yb,Box.zs);
	Faces[0].Vertices[1].setall(Box.xb,Box.yb,Box.zs);
	Faces[0].Vertices[0].setall(Box.xb,Box.ys,Box.zs);
	
	Faces[1].Vertices[0].setall(Box.xs,Box.yb,Box.zb);
	Faces[1].Vertices[1].setall(Box.xb,Box.yb,Box.zb);
	Faces[1].Vertices[2].setall(Box.xb,Box.ys,Box.zb);
	
	Faces[2].Vertices[0].setall(Box.xs,Box.ys,Box.zs);
	Faces[2].Vertices[1].setall(Box.xs,Box.ys,Box.zb);
	Faces[2].Vertices[2].setall(Box.xb,Box.ys,Box.zb);
	
	Faces[3].Vertices[0].setall(Box.xb,Box.ys,Box.zs);
	Faces[3].Vertices[1].setall(Box.xb,Box.ys,Box.zb);
	Faces[3].Vertices[2].setall(Box.xb,Box.yb,Box.zb);
	
	Faces[4].Vertices[0].setall(Box.xb,Box.yb,Box.zs);
	Faces[4].Vertices[1].setall(Box.xb,Box.yb,Box.zb);
	Faces[4].Vertices[2].setall(Box.xs,Box.yb,Box.zb);
	
	Faces[5].Vertices[0].setall(Box.xs,Box.yb,Box.zs);
	Faces[5].Vertices[1].setall(Box.xs,Box.yb,Box.zb);
	Faces[5].Vertices[2].setall(Box.xs,Box.ys,Box.zb);
	
	for (int f = 0; f<6; f++) {
		face &Face = Faces[f];
		Face.Texture = Tex;
		AlignToWorld(Face);
	}
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
	bool dev = 0;
	brush &Brush = *this;
	if (Brush.valid)
	{
		//int fVertices[2] = {-1,-1}; // first vertex for sorting
		float sy=0, hy=0, hz=0, sz=0;
		
		// get centroid for each brush
		for (int f = 0; f<Brush.t_faces; f++)
		{
			face &Face = Brush.Faces[f];
			// get smallest and biggest Y/Z vertex coordinate for each brush
			for (int v = 0; v<3; v++)
			{
				vertex &ev = Face.Vertices[v];
				//cout << "    Vertex #" << v << " " << ev;

				// smallest/biggest X/Y
				if (f==0&&v==0) {sy=ev.y; hy=ev.y; sz=ev.z; hz=ev.z; /*cout << " (is first)";*/ }
				else {
					if (ev.y<sy) {sy = ev.y; /*cout << " , smallest Y now: " << sy;*/ }
					if (ev.y>hy) {hy = ev.y; /*cout << " , highest  Y now: " << hy;*/ }
					if (ev.z<sz) {sz = ev.z; /*cout << " , smallest Z now: " << sz;*/ }
					if (ev.z>hz) {hz = ev.z; /*cout << " , highest Z now: " << hz;*/ }
				}
			}
		}
		Brush.centroid.y = hy-((hy-sy)/2);
		Brush.centroid.z = hz-((hz-sz)/2);
		if(dev) { cout << "centroid: " << Brush.centroid << ", from hy/sy/hz/sz " << hy << "," << sy << "," << hz << "," << sz << endl; }
	}
}

// find smallest and biggest vertex angle of each face
void brush::GetFaceVertexSE()
{
	bool dev = 0;
	if(dev) cout << endl << " Smallest and Biggest Face Vertex Angle..." << endl;
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
						if(dev) cout << "    Face "<<f<<" tex "<< Face.Texture << " candidate angle (" << round(candidate.angle) << ") is smaller than smallest angle of face (" << round(Face.Vertices[sv_ID].angle) << ")" << endl;
						sv_ID = v;
						sv_ID2 = v;
					}
					if (candidate.angle > Face.Vertices[bv_ID].angle)
					{
						if(dev) cout << "    Face "<<f<<" tex "<< Face.Texture << " candidate angle (" << round(candidate.angle) << ") is bigger than biggest angle of face (" << round(Face.Vertices[bv_ID].angle) << ")" << endl;
						bv_ID = v;
					}
					
				}
				if(dev) cout << "  smallest vertex of brush " << Brush.SegID << ", face " << f << "\t is: " << sv_ID <<"\t ("<<round(Face.Vertices[sv_ID].angle)<<"),\t biggest: " << bv_ID << "\t (angle " << round(Face.Vertices[bv_ID].angle) << ")" << endl; 
			}
		}
		
		if(dev)
		for (int f = 0; f<Brush.t_faces; f++)
		{
			face &Face = Brush.Faces[f];
			if (Face.fID==2)
			cout << "  FINAL: Face " << f << " tex "<< Face.Texture << " smallest vertex: " << Face.vAngle_s << Face.Vertices[Face.vAngle_s] << " biggest vertex: " << Face.vAngle_b << Face.Vertices[Face.vAngle_b] << endl;
		}
	}
	if(dev) getch();
}


// find first and last vertex for Brush vertex sorting list
void brush::GetVertexListSE()
{
	bool dev = 0;
	if(dev) cout<< endl << " First and Last Vertex of a whole Brush..." << endl;
	brush &Brush = *this;
	
	if (Brush.valid)
	{
		int &first_entry = Brush.vlist[0];
		int &last_entry  = Brush.vlist[Brush.t_faces-3];
		
		float &sangle = Brush.vAngle_s; // smallest vertex angle of whole brush
		float &bangle = Brush.vAngle_b; // biggest  vertex angle of whole brush
		
		if(dev) cout << "Brush " << Brush.SegID << ", smallest vertex angle: " << sangle << ", biggest: " << bangle << endl;
		
		for (int f = 0; f<Brush.t_faces; f++)
		{
			face &Face = Brush.Faces[f];
			vertex &sfv = Face.Vertices[Face.vAngle_s]; // vertex of this face that has smallest angle
			vertex &bfv  = Face.Vertices[Face.vAngle_b]; // ... biggest angle
			
			if (Face.fID==2) // only body faces
			{
				if(dev) cout << " Face " << f << ", smallest angle: " << Face.Vertices[Face.vAngle_s].angle << ", vertex "<< Face.vAngle_s << Face.Vertices[Face.vAngle_s] << ", biggest angle: " << Face.Vertices[Face.vAngle_b].angle << ", vertex "<< Face.vAngle_b << Face.Vertices[Face.vAngle_b] << endl;
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
		if(dev) cout << "first vList ID " << first_entry << endl;
		if(dev) cout << "last  vList ID " << last_entry << endl;
		if(dev) cout << "first entry of this brush is face " << first_entry << " tex "<< Brush.Faces[first_entry].Texture <<" vertex " << Brush.Faces[first_entry].Vertices[Brush.Faces[first_entry].vAngle_s] << " with a vertex angle of " << Brush.Faces[first_entry].Vertices[Brush.Faces[first_entry].vAngle_s].angle << endl;
		if(dev) cout << "last  entry of this brush is face " << last_entry  << " tex "<< Brush.Faces[last_entry].Texture <<" vertex " << Brush.Faces[last_entry].Vertices[Brush.Faces[last_entry].vAngle_b] << " with a vertex angle of " << Brush.Faces[last_entry].Vertices[Brush.Faces[last_entry].vAngle_s].angle << endl;
	}
	if(dev) getch();
}


// sort the rest of the vertices (only one vertex per face left)
void brush::GetVertexList()
{
	bool dev = 0;
	if(dev) cout << endl << " Sorting Vertices..." << endl;
	brush &Brush = *this;
		
	if (Brush.vlist[0]<0||Brush.vlist[Brush.t_faces-3]<0)
	{
		cout << "|    [ERROR] There was a problem reconstructing a Brush, skipping..." << endl;
		Brush.valid = 0;
	}
	
	if (Brush.valid)
	{
		// print vlist
		if(dev) cout << "   Vertex List Pointer " << Brush.vlist << " Array Size " << sizeof(*Brush.vlist)/sizeof(int) << endl;
		if(dev)
		for (int s = 0; s<Brush.t_faces-2; s++) {
			cout << "    VList #" << s << " " << Brush.vlist[s] << endl;
		}
		
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
			if(dev) cout << "    Last Vertex: (Face "<<lentry<<" Vertex "<<  Brush.Faces[lentry].vAngle_s << ")" << Brush.Faces[lentry].Vertices[Brush.Faces[lentry].vAngle_s].angle << endl;
			
			for (int f = 0; f<Brush.t_faces; f++) // face loop
			{
				face &Face = Brush.Faces[f];
				//cout << "    Face #" << f+1 << endl;
				if (Face.fID==2)
				{
					if(dev) cout << "    Current List Entry # " << s << ", Candidate Face#" << f << ", Vertex #" << Face.vAngle_s << ", Angle: " << Face.Vertices[Face.vAngle_s].angle << endl;
					vertex &candidate = Face.Vertices[Face.vAngle_s];
					if(dev) cout << "    Current Vertex: " << candidate << endl;
					
					if (centry==-1 && candidate.angle > last.angle)
					// if current list entry is UNSET and current vertex angle is greater than first vertex angle, set current list entry to Face f and Vertex v
					{
						centry = f;
						if(dev) cout << "      New Entry (First): " << candidate.angle << " is > last entry: " << last.angle << endl;
					}
					else if (centry!=-1 && candidate.angle < Brush.Faces[centry].Vertices[  Brush.Faces[centry].vAngle_s  ].angle && candidate.angle > last.angle)
					// if current list entry is UNSET and current vertex angle is SMALLER than currently "in-list" stored vertex BUT GREATER than last vertex angle, set current list entry to Face f and Vertex v
					{
						centry = f;
						if(dev) cout << "      New Entry (xth): "<< candidate.angle << " is < candidate: " << Brush.Faces[centry].Vertices[Brush.Faces[centry].vAngle_s].angle << " but not > last entry: " << last.angle <<endl;
					}
				}
			}
		}
		
		if(dev) cout << "Sorted vertex list for brush #" << Brush.SegID << endl;
		if(dev)
		for (int i = 0; i<Brush.t_faces-2; i++) { // list loop
			face &Face = Brush.Faces[Brush.vlist[i]];
			int &svert = Face.vAngle_s;
			cout << "#" << i << " = Vertex #" << Face.vAngle_s << " of Face #" << Brush.vlist[i] << ", Angle: " << Face.Vertices[svert].angle << ", Vertex " << Brush.Faces[Brush.vlist[i]].Vertices[Brush.Faces[Brush.vlist[i]].vAngle_s] <<  endl;
		}
	}
	if(dev) cout << "Press any Button to continue!" << endl;
	if(dev) getch();
}

void brush::ConvertVerticesC2V()
{
	bool dev = 0;
	brush &Brush = *this;
	if (dev) cout << " Converting RCON Vertices to normal Vertices..." << endl;
	for(int f = 0; f<Brush.t_faces; f++)
	{
		face &Face = Brush.Faces[f];
		if (dev) cout << "   Face " << f << " Tex " << Face.Texture << " V " << Face.vcount << " VC " << Face.vcountC << " ID " << Face.fID << " Or " << Face.Orient << endl;
		Face.vcount = Face.vcountC;
		
		if (Face.fID==2)
		{
			if (dev) cout << "     Body Face" << endl;
			int Missing = -1;
			// maintain first 3 old vertices and just add the missing one
			vertex *NewBody = new vertex[4];
			for (int v = 0; v<3; v++) {
				vertex &VO = Face.Vertices[v];
				vertex &VN = NewBody[v];
				VN = VO;
				if (dev) cout << "     Vertex " << v << VN << endl;
			}
			// Look for last missing Vertex
			for (int c = 3; c>=0&&Missing==-1; c--)
			for (int v = 0, fail = 0; v<3; v++)
			{
				vertex &VC = Face.VerticesC[c];
				vertex &VO = Face.Vertices[v];
				if (dev) cout << "        Comparing Candidate #"<< c << VC << " and Old #" << v << VO << endl;
				if (!CompareVertices(VC,VO)) { fail++;}
				if (v==2&&fail==3) { Missing = c; break; }
				//else if (v==2&&fail<3) fail = 0;
			}
			NewBody[3] = Face.VerticesC[Missing];
			if (dev) cout << "     Vertex 4 " << NewBody[3] << endl;
			delete[] Face.Vertices;
			Face.Vertices = NewBody;
		}
		else
		{
			if (dev) cout << "     Head/Base Face" << endl;
			delete[] Face.Vertices;
			Face.Vertices = new vertex[Face.vcountC];
			for (int v = 0; v<Face.vcountC; v++) {
				Face.Vertices[v] = Face.VerticesC[v];
				if (dev) cout << "     Vertex " << v << Face.Vertices[v] << endl;
			}
		}
		//delete[] Face.Vertices;
		//Face.Vertices = new vertex[Face.vcountC];
		/*
		for (int v = 0; v<Face.vcountC; v++)
		{
			Face.Vertices[v] = Face.VerticesC[v];
		}
		*/
		Face.GetVertexOrder();
		
		if (!Face.Clockwise)
		Face.RevOrder(0);
	}
	if (dev) getch();
}

// create new vertices for all faces to be able to calculate their X/Y Texture Offsets
void brush::GetRconVertices()
{
	bool dev = 0;
	if(dev) cout << "Reconstructing non-existent brush vertices..." << endl;
	brush &Brush = *this;
	
	if (Brush.valid)
	{
		// get smallest and biggest x coord for this brush
		float sx = 0;
		float bx = 0;
		sx = Brush.Faces[ Brush.BaseID ].Vertices[0].x;
		bx = Brush.Faces[ Brush.HeadID ].Vertices[0].x;
		if(dev) cout << "  smallest/biggest X: " << sx << " / " << bx << endl;
		
		// new head/base faces
		if(dev) cout << "  Head/Base Faces... Base ID " << Brush.BaseID << " Head ID " << Brush.HeadID << endl;
		for (int f = 0; f<2; f++)
		{
			int FaceID = 0;
			if (f==0) 	FaceID = Brush.BaseID;
			else 		FaceID = Brush.HeadID;
			face &Face = Brush.Faces[FaceID];
			Face.VerticesC = new vertex[Brush.t_faces-2];
			Face.vcountC = Brush.t_faces-2;
			
			if(dev) cout << "    New Head Base Vertices..." << endl;
			for (int v = 0; v<Brush.t_faces-2; v++)
			{
				if(dev) cout << "      Vertex " << v << " Brush.vlist " << Brush.vlist[v] << " Angle " << Brush.Faces[Brush.vlist[v]].vAngle_s << endl;
				Face.VerticesC[v] = Brush.Faces[  Brush.vlist[v]  ].Vertices[  Brush.Faces[Brush.vlist[v]].vAngle_s  ];
				
				if (f==0) Face.VerticesC[v].x = sx;
				if (f==1) Face.VerticesC[v].x = bx;
			}
			//if (f==0) Face.RevOrderC(1);  // Reversing Vertex Order unnecessary?!
			if(dev) {
			for (int v = 0; v<Brush.t_faces-2; v++) cout << "    Face "<<f<<" new vertex " << v << Face.VerticesC[v] << endl;
			for (int v = 0; v<3; v++) cout << "    Face "<<f<<" old vertex " << v << Face.Vertices[v] << endl;
			}
		}
		
		// new body faces
		if(dev) cout << "  Body Faces..." << endl;
		for (int e = 0; e<Brush.t_faces-2; e++)
		{
			int f = Brush.vlist[e]; // first vlist entry face
			//int fb = 0; if (vl==Brush.t_faces-3) fb=0; else fb=f+1;
			//int vl2 = Brush.vlist[fb]; // next vlist entry face
			
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
			
			if(dev) {
			for (int v = 0; v<4; v++) cout << "    Face "<<f<<" new vertex " << v << Face.VerticesC[v] << " Face.vAngle_sbak " << Face.vAngle_sbak << " Face.vAngle_b " << Face.vAngle_b << endl;
			for (int v = 0; v<3; v++) cout << "    Face "<<f<<" old vertex " << v << Face.Vertices[v] << endl;
			}
		}
		Brush.RCON = 1;
	}
	if(dev) cout << "Press any Button to continue!" << endl;
	if(dev) getch();
}


// Determine Texture Alignment of all Faces and fix invalid Texture alignments
void brush::GetTVecAligns()
{
	bool dev = 0;
	if(dev) cout << "Determine Alignment of all Faces and fix invalid Face alignments..." << endl;
	
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
	if(dev) cout << "Press ANY BUTTON to continue! ..." << endl;
	if(dev) getch();
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
	bool dev = 0;
	if(dev) cout << "Getting exact centroid of Brush " << this->SecID << " Faces..." << endl;
	brush &Brush = *this;
	
	if (Brush.valid)
	{
		for (int f = 0; f<Brush.t_faces; f++)
		{
			face &Face = Brush.Faces[f];
			Face.GetCentroid();
			if (dev) cout << "  Face " << f << " Centroid is " << Face.Centroid << endl;
		}
	}
}

// get exact centroids of reconstructed Faces
void brush::GetFaceCentroidsC()
{
	bool dev = 0;
	if(dev) cout << "Getting exact centroid of Brush " << this->SecID << " Recon-Faces..." << endl;
	brush &Brush = *this;
	
	if (Brush.valid)
	{
		for (int f = 0; f<Brush.t_faces; f++)
		{
			face &Face = Brush.Faces[f];
			Face.GetCentroidC();
			if (dev) cout << "  Face " << f << " Centroid is " << Face.Centroid << endl;
		}
	}
}

void brush::CheckDivisibility()
{
	// Mark Cuboid Brushes for potential triangulation
	bool dev = 0;
	if(dev) cout << " Checking for Cuboid Brushes..." << endl;
	brush &Brush = *this;
	if (Brush.valid)
	{
		//face &Base = Brush.Faces[Brush.BaseID];
		// differ between potential rectangle- and polygone type brushes
		if(dev) cout << " Brush " << Brush.SegID << " total faces " << Brush.t_faces << " base ID " << Brush.BaseID << " head ID " << Brush.HeadID << endl;
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
					if 		(Face.Orient==4) { foundFront = 1; if(dev)cout << "   Found Front! Face Orient " << Face.Orient << endl; }
					else if (Face.Orient==5) { foundBack  = 1; if(dev)cout << "   Found Back!  Face Orient " << Face.Orient << endl; }
				}
			}
			if (foundFront&&foundBack&&Brush.t_faces==6) Brush.IsDivisible = 1;
			else if ((foundFront||foundBack)&&Brush.t_faces==5) Brush.IsDivisible = 1;
			else Brush.IsDivisible = 0;
		}
		if (dev) {
		if (Brush.IsDivisible) cout << " Brush seg "<< Brush.SegID <<" sec " << Brush.SecID <<" IS divisible easily!" << endl;
		else cout << " Brush seg "<< Brush.SegID <<" sec " << Brush.SecID<<" tex ("<<Brush.Faces[2].Texture<<") IS NOT divisible easily!" << endl; }
	}
}

void brush::CreateGap(int g)
{
	bool dev = 0;
	if (dev) cout << " Creating Gap Brush from Brush #" <<this->SecID<<"..." << endl;
	
	brush &Source = *this;
	Gap = new brush;
	Gap->Copy(Source);
	
	Gap->Faces[0].GetNormal();
	gvector nEdge = Gap->Faces[0].Normal;
	nEdge.mult(cTable[g].gaplen); // make the gap brush 256 units long
	nEdge.flip();
	
	// Fix Head Faces
	if (dev) cout << "   Fixing Head Face..." << endl;
	face &Base = Gap->Faces[0];
	face &Head = Gap->Faces[1];
	for (int v = 0; v<Base.vcount; v++)
	{
		vertex &VB = Base.Vertices[v];
		vertex &VH = Head.Vertices[v];
		VH = Add(VB,nEdge);
		if (dev) cout << "     Head Vertex "<<v<< " now " << VH << " = Base Vertex " << VB << " + nEdge " << nEdge << endl;
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
		if (dev) cout << "     Body Vertex 2 now " << V2 << " = V1 " << V1 << " + nEdge " << nEdge << endl;
		if (dev) cout << "     Body Vertex 3 now " << V3 << " = V0 " << V0 << " + nEdge " << nEdge << endl;
		
		//if(!Face.GetVertexOrder()) Face.RevOrder(0);
	}
}

void brush::Reconstruct()
{
	bool dev = 0;
	brush &Brush = *this;
	bool DoRcon[Brush.t_faces]; // which faces are to be reconstructed
	for (int f = 0; f<t_faces; f++)
		DoRcon[f] = 1;
	vector< vector<int> > ConList(t_faces); // connected Faces index list

	ConList.resize(t_faces);
	// when a brush is being imported from a map file, each of its faces consists of 3 vertices
	// in order to be able to get their original texture offsets, their remaining vertices have to be calculated somehow
	// this can be done by intersecting all faces (that are not parallel) with each other (always 3 at a time) to get the missing intersection points
	
	// intersecting all faces with each other will produce many copies of the same vertex, which is actually unnecessary, as is intersecting parallel faces
	// a quick check on parallelism could be to compare the Normal vectors of 2 faces, where one of both was inverted before:  x-->  <--x (as there can never be 2 of the same Normal Vectors in a brush anyway)
	// theres no way I can think of that avoids the unnecessary calculation of already existing vertices, so for the moment, I will just calculate them all (no I will not, because that would be insane)
	// one way I thought of is checking which faces are "connected" with each other and only calculate the intersections of those
	
	// check for parallelism and "vertical" face-align
	
	// maybe try to find 2 faces that act as top and bottom of the brush?
	
	Brush.GetFaceNormals();
	
	// get some missing vertices from the 3 existing face vertices, by checking the distance of each brush vertex to each brush face (when dist is 0, vertex is face vertex!)
	if (dev) cout << " Getting missing face vertices from the 3 existing face vertices..." << endl;
	if (dev) getch();
	for (int f = 0; f<t_faces; f++)
	{
		face &Face = Brush.Faces[f];
		vector<vertex> NewFaceVerts;
		// Fill new vertex list with existing face vertices
		for (int i = 0; i<Face.vcount; i++)
			NewFaceVerts.push_back(Face.Vertices[i]);
		
		// Vertices of all Faces are being checked
		if (dev) cout << "  Face " << f << " - Vertices of all Faces are being checked..." << endl;
		for (int fc = 0; fc<t_faces; fc++)
		{
			face &CFace = Brush.Faces[fc];
			if (f!=fc) // skip if candidate is current face itself
			{
				for (int v = 0; v<CFace.vcount; v++)
				{
					vertex &V = CFace.Vertices[v];
					if (IsVertexOnFace(Face,V,2))
					{
						NewFaceVerts.push_back(CFace.Vertices[v]);
						if (dev) cout << "   Match! V " << v << V << " of F " << fc << " ("<<CFace.Texture <<") is on Face " << f << " ("<<Face.Texture<<") NVec "<< Face.Normal << " Listsize " << NewFaceVerts.size() << endl;
					}
					else
					{
						if (dev) cout << "   NO Match! Vertex " << v << V << " of Face " << fc << " ("<<CFace.Texture <<") wasnt found on Face " << f << " ("<<Face.Texture<<")!" << endl;
					}
				}
			}
		}
		if (dev) getch();
		
		//replace existing vertex array of this face and fill it with the newly generated vertex list
		int new_vcount = NewFaceVerts.size();
		int old_vcount = Face.vcount;
		if (new_vcount>Face.vcount)
		{
			if (dev) cout << "  New vcount " << new_vcount << " - Creating new Vertex array for Face "<<Face.Texture<<"..." << endl;
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
					if (  !IsVertexInList(VN, Face.Vertices, Face.vcount, 0, 0)  )
					{
						V = VN;
						if (dev) cout << "   Face " << f << " Vcount O/N ("<<Face.vcount<<"/"<<Face.vcount+1<<") NewVert "<< v << VN << " added to VList ("<<vb<<"). V-Index increased to " << vb+1 << endl;
						Face.vcount++;
						vb++;
					}
					else
					{
						//if (dev) cout << "   Face " << f << " Vcount decreased by 1 to " << Face.vcount-1 << endl;
						//Face.vcount--;
					}
				}
				else if (v<old_vcount)
				{
					V = VN;
					vb++;
				}
			}
		}
		if (dev) getch();
	}
	
	if (dev) cout << "  New vertex lists..." << endl;
	if (dev)
	for (int f = 0; f<t_faces; f++)
	{
		face &Face = Brush.Faces[f];
		for (int v = 0; v<Face.vcount; v++)
		{
			vertex &V = Face.Vertices[v];
			cout << "    Face " << f << " v " << v << V << endl;
		}
	}
	
	
	// find faces that are "connected" to each other (share the same vertices)
	/*if (dev) cout << " Finding faces that are connected to each other..." << endl;
	if (dev) getch();
	for (int f = 0; f<t_faces; f++)
	{
		face &Face = Brush.Faces[f];
		//ConList.push_back(vector<int>());
		for (int fc = 0; fc<t_faces; fc++)
		{
			face &CFace = Brush.Faces[fc];
			if (f!=fc) // dont compare current face to itself
			{
				//if (dev) cout << "   Current Face " << f << " Candidate " << fc << endl;
				if (DoFacesShareVerts(Face,CFace,2))
				{
					if (dev) cout << "   Face " << f<<" (" << Face.Texture << ") CFace " << fc <<" (" << CFace.Texture << ") MATCH! Adding ID "<<fc<<" to ConList "<<f<<"..." << endl;
					ConList[f].push_back(fc);
					//if (dev) getch();
				}
			}
		}
	}
	
	if (dev) cout << " Final ConList:" << endl;
	if (dev) getch();
	if (dev)
	for (int f = 0; f<t_faces; f++)
	{
		int size = ConList[f].size();
		for (int c = 0; c<size; c++)
		{
			cout << " ConList["<<f<<"]["<<c<<"] " << ConList[f][c] << "("<<Brush.Faces[f].Texture<<") connects to ("<<Brush.Faces[ConList[f][c]].Texture<<")" << endl;
		}
	}*/
	
	if (dev) cout << " FINISH!" << endl;
	if (dev) getch();
	// Skip reconstruction entirely, if brush is only made of triangles! (but this isnt possible. theres no way to tell if thats true without further checking)
	// solution: if every face of a brush has 2 connected faces initially, the brush consists only of triangles (correct?? NO!!)
}

void brush::CreateTent()
{
	bool dev = 0;
	brush &Brush = *this;
	
	//if (dev) cout << "  Created 3 textured faces vertices" << endl << "      V0 " << VO0 << endl << "      V1 " << VO1 << endl << "      V2 (Center from SrcFace Centroid " <<SrcFace.Centroid<<") " << VO2 << endl;
	Brush.Faces[0].GetCentroid();
	vertex TentBase = Brush.Faces[0].Centroid;
	Brush.Faces[0].GetNormal();
	gvector Normal = Brush.Faces[0].Normal.flip();
	Normal.mult(def_spikesize);
	TentBase.Add(Normal);
	if (dev) cout << "  Created Tent Vertex " << TentBase << " from centroid " << Brush.Faces[0].Centroid << " and flipped Normal " << Normal <<endl;
	if (dev) cout << "  Creating Tent NULL Faces now..." <<endl;
	
	if (dev) cout << " Creating Tent for Triangle Brush..." << endl;
	for (int f = 1, v = 0; f<Brush.t_faces; f++)
	{
		face &Face = Brush.Faces[f];
		Face.Vertices = new vertex[3];
		Face.vcount = 3;
		vertex &V0 = Face.Vertices[0];
		vertex &V1 = Face.Vertices[1];
		vertex &V2 = Face.Vertices[2];
		if (dev) cout << "      created 3 new vertices for face " << f << endl;
		
		V0 = Brush.Faces[0].Vertices[v];
		V1 = TentBase;
		if (f==Brush.t_faces-1)
		V2 = Brush.Faces[0].Vertices[0];
		else
		V2 = Brush.Faces[0].Vertices[v+1];
		if (dev) cout << "      V0 " << V0 << V1 << V2 << endl;
		
		Face.Texture = def_nulltex;
		Face.IsNULL = 1;
		Face.fID=2;
		AlignToWorld(Face);
		Face.TentID = 1;
		//Face.RevOrder(0);
		
		v++;
	}
}

void brush::ClearVertexList()
{
	bool dev = 0;
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
		//Face.BaseX = 0;
		//Face.BaseX2 = 0;
		//Face.BaseY = 0;
		//Face.BaseY2 = 0;
		Face.vAngle_b = 0;
		Face.vAngle_s = 0;
		Face.vAngle_sbak = 0;
		for (int i = 0; i<4; i++) {
			Face.BaseListX[i] = 0;
			Face.BaseListY[i] = 0;
		}
		for (int v = 0; v<Face.vcount; v++) {
			vertex &V = Face.Vertices[v];
			V.angle = 0;
		}
	}
}

void brush::RoundVertices()
{
	bool dev = 0;
	brush &Brush = *this;
	
	if (   Brush.valid && Brush.draw  ) // &&   ( Brush.IsDivisible && ( Brush.t_faces==6 || Brush.t_faces==5 ) )
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
	bool dev = 0;
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
	bool dev = 0;
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

		if(dev) cout << " Marking Vertices of Brush sec " << Brush.SecID << " Face " << f << " mode " << Mode << " comparing to " << Candidate.vcount << " vertices " << endl;
		for (int c = 0; c<Candidate.vcount; c++)
		{
			vertex &C = Candidate.Vertices[c];
			if(dev) cout << "   Comparing with Candidate Vertex " << c << C << endl;
			for (int v = 0; v<Face.vcount; v++)
			{
				vertex &V = Face.Vertices[v];
				if(dev) cout << "     Vertex " << v << V << endl;
				if (CompareVerticesXY(C,V))
				{
					if(dev) cout << "       Match Found!" << endl;
					if (Mode==1)	V.DoAddHeight = 1;
					if (Mode==0)	V.DoRound = 1;
				}
			}
		}
	}
	if(dev) getch();
}

void brush::CheckNULLFaces()
{
	bool dev = 0;
	brush &Brush = *this;
	int Ncount = 0;
	if (dev) cout << " Checking Null Count of Brush " << Brush.name << " with face count " << Brush.t_faces << endl;
	
	if (Brush.valid&&Brush.draw)
	for (int f = 0; f<Brush.t_faces; f++)
	{
		face &Face = Brush.Faces[f];
		if (!Face.draw||Face.IsNULL||Face.Texture=="NULL"||Face.Texture==def_nulltex) {
			Ncount++;
			if (dev) cout << "  Face is NULL Face! NUll Counter now " << Ncount << endl;
		}
	}
	if (dev) cout << "  Final Null Count of this Brush " << Ncount << " of total " << Brush.t_faces << endl<< endl;
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
	bool dev = 0;
	brush &Brush = *this;
	if (dev) cout << " Refreshing Tent Vertex of Brush " << Brush.name << " IsSpike " << Brush.IsSpike << " total faces " << Brush.t_faces << endl;
	
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
			if (dev) cout << "   Face " << f << " isNULL " << Face.IsNULL << endl;
			if (Face.IsNULL)
			Face.Vertices[Face.TentID] = newTent;
			//Face.RefreshTent(Base);
		}
	}
}

void brush::Triangulate()
{
	bool dev = 0;
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
			if (Brush.t_faces==6) { if (dev) cout << " Triangulating 6 sided Div Brush..." << endl;
				Brush.TriTrapezoid(); }
			else if (Brush.t_faces==5) { if (dev) cout << " Triangulating 5 sided Div Brush..." << endl;
				Brush.TriTriangle(); }
		}
		else if (Brush.IsDivisible&&Brush.IsWedge)
		{
			// Do nothing...
			if (dev) cout << " Brush already is a Wedge. Leaving it as it is!" << endl;
		}
		else
		{
			// Brush isnt divisible effectively, so it has to be split up into as many triangle-brushes as necessary, but not more, to keep it efficient
			if (dev) cout << " Triangulating Complex Brush..." << endl;
			Brush.TriComplex();
		}
	}
}

// triangulate a Trapezoid brush that has 4 sides of which 2 (front and back face) are parallel
void brush::TriTrapezoid()
{
	bool dev = 0;
	brush &Brush = *this;
	
	if (Brush.valid&&Brush.draw&&Brush.IsDivisible&&!Brush.IsWedge&&Brush.t_faces==6)
	{
		if(dev) cout << "   Triangulating Trapezoid Brush - fcount " << Brush.t_faces << endl;
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
		if(dev) cout << "   Fixing Wedge 1... this includes Face1 " << Wedge1.Faces[1].Texture << " BACK " << Wedge1_Back.Texture << " Wedge1_Top " << Wedge1_Top.Texture << " Wedge1_Btm " << Wedge1_Btm.Texture << endl;
		Wedge1.Faces[1].draw = 0; // remove relevant cutting edge face to form a triangle of this brush
		Wedge1_Back.Vertices[2]	= Wedge1.Faces[1].Vertices[2];
		Wedge1_Back.Vertices[3]	= Wedge1.Faces[1].Vertices[3];
		swap(Wedge1_Top.Vertices[2], Wedge1_Top.Vertices[3]); // swap vertices to fix vertex order
		Wedge1_Top.vcount = 3;
		Wedge1_Btm.vcount = 3;
		
		// fix wedge 2
		if(dev) cout << "   Fixing Wedge 2..." << endl;
		Wedge2.IsWedge2 = 1;
		Wedge2.Faces[0].draw = 0;
		Wedge2_Front.Vertices[0]= Wedge2.Faces[0].Vertices[2];
		Wedge2_Front.Vertices[1]= Wedge2.Faces[0].Vertices[3];
		Wedge2_Top.pushVerts(3);
		Wedge2_Btm.pushVerts(2);
		Wedge2_Top.vcount = 3;
		Wedge2_Btm.vcount = 3;
		
		// new tri faces textures and tex align
		if(dev) cout << "   new tri faces textures and tex align..." << endl;
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
		if(dev) cout << "   new Face Edge IDs..." << endl;
		Wedge1_Top.SetEdges(0,2,0,1,2,1);
		Wedge2_Top.SetEdges(1,0,1,2,0,2);
		Wedge1_Btm.SetEdges(1,2,1,0,2,0);
		Wedge2_Btm.SetEdges(1,2,1,0,2,0);
		
		Wedge1.MarkFaceVertices(Brush.Faces[1], 1, 1);
		Wedge2.MarkFaceVertices(Brush.Faces[1], 1, 1);
		
		if(dev) cout << "   delete Wedge Gaps..." << endl;
		delete Wedge1.Gap; Wedge1.Gap = nullptr;
		delete Wedge2.Gap; Wedge2.Gap = nullptr;
	}
}

// triangulate a Triangle Brush that has 3 sides of which one is !upright!
void brush::TriTriangle()
{
	bool dev = 0;
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
	bool dev = 0;
	brush &Brush = *this;
	if (dev) cout << " Now triangulating Complex Brush..." << endl;
	
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
		
		if (dev) cout << "  New Brush Count: " << newBrushCount << endl;
		Brush.Tri = new brush[newBrushCount];
		Brush.t_tri = newBrushCount;
		Brush.Faces[0].GetCentroid();
		Brush.Faces[1].GetCentroid();
		// Head and Base Face
		if (dev) cout << "  Turning Head and Base Face into Brushes..." << endl;
		if (!cTable[gID].round||Brush.Faces[0].vcount==3) {
			brush* Base = Face2Brush(Brush.Faces[0]);
			brush* Head = Face2Brush(Brush.Faces[1]);
			Brush.Tri[0].CopySimple(*Base);
			Brush.Tri[1].CopySimple(*Head);
			delete Base;
			delete Head;
		} else {
			// if round = 1 base and head faces are turned into a "triangle fan", consisting of many brush spikes instead of just one spike brush
			if (dev) cout << "    Round is ON. Turning Head Face into triangle bridge..." << endl;
			for (int tb = 0, ov = 0; ov<Brush.Faces[0].vcount-1; ov++) {
				if (ov!=1)
				{
					if (dev) cout << "      Tri Brush " << tb << " Original Face Vertex " << ov << " of total " << Brush.Faces[0].vcount << endl;
					brush* BaseFan = Face2BrushTriBridge(Brush.Faces[0], ov);
					Brush.Tri[tb].CopySimple(*BaseFan);
					delete BaseFan;
					tb++;
				}
			}
			for (int tb = Brush.Faces[0].vcount-2, ov=0; ov<Brush.Faces[1].vcount-1; ov++) {
				if (ov!=1)
				{
					if (dev) cout << "      Tri Brush " << tb << " Original Face Vertex " << ov << " of total " << Brush.Faces[1].vcount << endl;
					brush* HeadFan = Face2BrushTriBridge(Brush.Faces[1], ov);
					Brush.Tri[tb].CopySimple(*HeadFan);
					delete HeadFan;
					tb++;
				}
			}
		}
		
		// Body Faces
		if (dev) cout << "  Turning Body Faces into Brushes..." << endl;
		int v = 2;
		if (cTable[gID].round&&Brush.Faces[0].vcount!=3) v=(Brush.Faces[0].vcount-2)*2;
		for (int f = 2; f<Brush.t_faces; f++)
		{
			face &Face = Brush.Faces[f];
			if (dev) cout << "   Face " << f << endl;
			if (Face.fID==2)
			{
				if (dev) cout << "     Tri Brush " << v << " Creating 2 Triangle Brushes..." << endl;
				brush* Wedge1 = Face2BrushTri(Brush.Faces[f], 0);
				brush* Wedge2 = Face2BrushTri(Brush.Faces[f], 1);
				
				Brush.Tri[v].CopySimple  ( *Wedge1 );
				Brush.Tri[v+1].CopySimple( *Wedge2 );

				Brush.Tri[v].MarkFaceVertices(Brush.Faces[1], 1, 1);
				Brush.Tri[v+1].MarkFaceVertices(Brush.Faces[1], 1, 1);
				
				delete Wedge1;
				delete Wedge2;
				
				v+=2;
			}
		}
	}
}

void brush::GetFaceOrients()
{
	bool dev = 0;
	bool devtex = 0;
	//determine Orientation of all body Faces
	if (dev) cout << " determine Orientation of all body Faces..." << endl;
	// 0 = Left, 1 = Right, 2 = Top, 3 = Down, 4 = Front, 5 = Back
	brush &Brush = *this;
	if (Brush.valid)
	{
		for (int f = 0; f<Brush.t_faces; f++)
		{
			face &Face = Brush.Faces[f];
			if (dev) cout << " Face " << f << " Tex "<< Face.Texture << " Orient " << Face.Orient << " Normal " << Face.Normal << endl;
			Face.GetNormal();
			if (Face.fID==2)
			{
				if 		(Face.Normal.z>0.001) 	{Face.Orient = 2; if (devtex) Face.Texture+="_UP";} // Top / Down Faces
				else if (Face.Normal.z<-0.001) 	{Face.Orient = 3; if (devtex) Face.Texture+="_DN";}
				else if (Face.Normal.y>0.999) 	{Face.Orient = 5; if (devtex) Face.Texture+="_BK";} // Front Back Faces
				else if (Face.Normal.y<-0.999) 	{Face.Orient = 4; if (devtex) Face.Texture+="_FT";}
				
				if (dev) cout << "     New Orient [" << Face.Orient << "]" << endl;
				if (dev) cout << "     Normal " << Face.Normal << endl << endl;
			}
		}
		// if top and down faces cant be identified but front/back faces were and brush qualifies as a potential Cuboid (6 faces):
	}
}

bool brush::CheckValidity()
{
	bool dev = 0;
	bool devtex = 0;
	
	brush &Brush = *this;
	Brush.BaseID=-1;
	Brush.HeadID=-1;
	if(dev) cout << "  Checking Validity of Brush " << Brush.SegID<< endl;
	
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
		if(dev) cout << "   Face " << f << " Tex " << Face.Texture << " Normal " << Face.Normal << " Planarity " << Face.IsPlanar << endl;
		
		if (Face.Normal.x>0.999&&Face.IsPlanar) {
			Face.Orient = 1;
			Face.fID = 1;
			ctr_head++;
			Brush.HeadID = f;
			if (devtex)
			Face.Texture += "_RT";
		}
		else if (Face.Normal.x<-0.999&&Face.IsPlanar) {
			Face.Orient = 0;
			Face.fID = 0;
			ctr_base++;
			Brush.BaseID = f;
			if (devtex)
			Face.Texture += "_LF";
		}
		else if (Face.Normal.x>-0.001&&Face.Normal.x<0.001&&Face.IsPlanar) {
			ctr_body++;
			Face.fID = 2;
			if (devtex)
			Face.Texture += "_BODY";
		}
	}
	if(dev) cout << " Brush " << Brush.name << " Body Faces " << ctr_body << " HeadID " << Brush.HeadID << " BaseID " << Brush.BaseID << endl << endl;
	if(dev) getch();
	
	// check if base and headface vertices are mirroring each other (only works when Brush Faces have been reconstructed)
	bool Mirror = 0;
	if (Brush.BaseID!=-1&&Brush.HeadID!=-1&&Brush.BaseID!=Brush.HeadID)
	{
		face &Base = Brush.Faces[Brush.BaseID];
		face &Head = Brush.Faces[Brush.HeadID];
		if(dev) cout << "  Checking if " << Base.vcount << " base and " << Head.vcount <<" head vertices of Brush " << Brush.name << " RCON " << Brush.RCON << " are mirroring each other..." << endl;
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
						if(dev) cout << "  Checking Base Vert " << b << B << " and Head Vert " << h << H << " MATCH! Ctr now " << mCtr << endl;
					} else {
						if(dev) cout << "  Checking Base Vert " << b << B << " and Head Vert " << h << H << endl;
					}
				}
			}
			if (mCtr==Base.vcount) Mirror = 1;
		}
		else if (Brush.RCON && Base.vcount!=Head.vcount) Mirror = 0;
		else if (!RCON) Mirror = 1; // if Brushes arent reconstructed yet, theres no point in checking for mirrored vertices
	}
	if (dev) getch();
	
	if (ctr_head!=1||ctr_base!=1||ctr_body!=Brush.t_faces-2||!Mirror)
	{
		Brush.valid = 0;
		cout << "|  [WARNING] Brush ["<<Brush.SegID+1<<"] seems to have an invalid Mesh and won't be processed." << endl;
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
	bool dev = 0;
	if (dev) cout << " Converting a Face to a Brush (Spike)..." << endl;
	
	brush* BrushPtr = new brush;
	brush& Brush = *BrushPtr;
	
	if (dev) cout << "  Creating "<<SrcFace.vcount+1<<" new faces for this Spike..." << endl;
	Brush.IsSpike = 1;
	Brush.Faces = new face[SrcFace.vcount+1];
	Brush.t_faces = SrcFace.vcount+1;
	if (dev) cout << "  Copying Source Face ("<<Brush.Faces[0].Texture<<") into first Spike face..." << endl;
	Brush.Faces[0].CopyFace(SrcFace,1);
	
	if (dev) cout << "  Creating Tent Faces (NULL-Faces) now..." << endl;
	Brush.CreateTent();
	
	if (dev) cout << "  returning BrushPtr..." << endl;
	return BrushPtr;
}

brush* Face2BrushTri(face &SrcFace, int Side)
{
	bool dev = 0;
	if (dev) cout << " Converting a Face to a Triangle Brush..." << endl;
	
	brush* BrushPtr = new brush;
	brush& Brush = *BrushPtr;
	
	Brush.IsSpike = 1;
	Brush.Faces = new face[4];
	Brush.t_faces = 4;
	
	Brush.Faces[0].CopyFace(SrcFace,1);
	
	if (Side==0)
	{
		swap(Brush.Faces[0].Vertices[2],Brush.Faces[0].Vertices[3]);
		Brush.Faces[0].SetEdges(2,0,1,0,2,1);
	}
	else
	{
		Brush.Faces[0].pushVerts(3);
		Brush.Faces[0].SetEdges(1,0,1,2,2,0);
	}
	if (dev) cout << "  Set Face Edges EdgeH " << Brush.Faces[0].EdgeH << " EdgeV " << Brush.Faces[0].EdgeV << " Hypo " << Brush.Faces[0].Hypo <<endl;
	
	Brush.Faces[0].vcount = 3;
	Brush.CreateTent();
	
	if (dev) cout << endl;
	return BrushPtr;
}

brush* Face2BrushTriBridge(face &SrcFace, int VID)
{
	bool dev = 0;
	if (dev) cout << " Converting a Face with fID "<< SrcFace.fID << " to a Triangle Bridge Brush..." << endl;
	
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
	
	if (dev) cout << endl;
	//if (dev) getch();
	return BrushPtr;
}

brush* Face2BrushTriFan(face &SrcFace, int VID)
{
	bool dev = 0;
	if (dev) cout << " Converting a Face with fID "<< SrcFace.fID << " to a Triangle Fan Brush..." << endl;
	
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
	
	if (dev) cout << endl;
	//if (dev) getch();
	return BrushPtr;
}









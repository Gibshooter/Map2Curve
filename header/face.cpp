#include "face.h"
#include "vertex.h"
#include "utils.h"
#include "settings.h"
#include "file.h"
#include "group.h"

#include <math.h>
#include <iostream>
#include <iomanip> // precision

#define DEBUG 0

#define PI 3.14159265

using namespace std;

struct gvector;
struct vertex;
struct cTable;
struct vGroup;
struct file;
enum axis;

extern vertex Zero;
extern file *gFile;
extern ctable *cTable;
extern float def_spikesize;
extern string def_nulltex;
extern group *bGroup;

/* ===== FACE METHODS ===== */

void CoutBrushFacesDevInfo(brush &Brush, int b, int g, bool IsInRange)
{
	if ( IsInRange )
	{
		int tf = Brush.t_faces;
		
		cout << "|==================\\" << endl;
		cout << "|Brush " << b << " curve " << g << endl;
		cout << "| F";
		cout << "|O";
		cout << "| G";
		cout << "|Sec";
		cout << "|" << setw(11) << "  Texture  ";
		cout << "|" << setw(5) << "ScalX";
		cout << "|" << setw(5) << "ScalY";
		cout << "|" << setw(7) << "ShiftX";
		cout << "|" << setw(7) << "ShiftY";
		cout << "|" << setw(7) << "BShiftX";
		cout << "|" << setw(7) << "BShiftY";
		cout << "|" << setw(7) << "HShiftL";
		cout << "|" << setw(7) << "HShiftS";
		cout << "|" << setw(8) << "  HSrcL ";
		cout << "|" << setw(8) << "  HSrcS ";
		cout << "|IsSrcL";
		cout << "|IsSrcS";
		cout << "|ELenL";
		cout << "|ELenS";
//		cout << "|     EdgeH    ";
//		cout << "|     EdgeV    ";
		cout << "|SecBas";
		cout << "|WedgeD";
		cout << "|BWedge";
		cout << "|Gap";
		cout << "|Align";
		cout << endl;
		
		for (int f = 0; f<tf; f++)
		{
			face &Face = Brush.Faces[f];
			if (Face.FaceIsValid(-1,1,1,1))
			{
				CoutFacesDevInfo(Brush,Face,f);
			}
		}
		cout << ios_base::unsetf << endl;
		cout << endl;
	}
}

void CoutFacesDevInfo(brush &Brush, face &Face, int f)
{
	cout << fixed;
	cout << "|" << setw(2) << f;
	cout << "|" << Face.Orient;
	cout << "|" << setw(2) << Face.group;
	cout << "|" << setw(3) << Brush.SecID;
	cout << "|" << setw(11) << Face.Texture;
	cout << "|" << setw(5) << setprecision(1) << Face.ScaleX;
	cout << "|" << setw(5) << setprecision(1) << Face.ScaleY;
	cout << "|" << setw(7) << setprecision(0) << round(Face.ShiftX);
	cout << "|" << setw(7) << setprecision(0) << round(Face.ShiftY);
	cout << "|" << setw(7) << setprecision(0) << round(Face.BaseShiftX);
	cout << "|" << setw(7) << setprecision(0) << round(Face.BaseShiftY);
	cout << "|" << setw(7) << setprecision(0) << round(Face.HShiftL);
	cout << "|" << setw(7) << setprecision(0) << round(Face.HShiftS);
	cout << "|" << setw(8) << Face.HSourceL;
	cout << "|" << setw(8) << Face.HSourceS;
	if (&Face==Face.HSourceL) cout << "|XXXXXX"; else cout << "|      ";
	if (&Face==Face.HSourceS) cout << "|XXXXXX"; else cout << "|      ";
	cout << "|" << setw(5) << round(Face.EdgeLenL);
	cout << "|" << setw(5) << round(Face.EdgeLenS);
	//cout << "|" << setw(4) << setprecision(0) << Face.EdgeH.x << "," << setw(4) << Face.EdgeH.y << "," << setw(4) << Face.EdgeH.z;
	//cout << "|" << setw(4) << setprecision(0) << Face.EdgeV.x << "," << setw(4) << Face.EdgeV.y << "," << setw(4) << Face.EdgeV.z;
	if (Face.IsSecBase) 	cout << "|XXXXXX"; else cout << "|      ";
	if (Face.IsWedgeDown) 	cout << "|XXXXXX"; else cout << "|      ";
	if (Brush.IsWedge) 	cout << "|X 1  X"; else if(Brush.IsWedge2) cout << "|X 2  X"; else cout << "|      ";
	if (Brush.IsGap) 	cout << "| X "; else cout << "|   ";
	cout << "|" << setw(5) << Brush.Align;
	cout << endl;
}


bool face::FaceIsValid(int CheckForID, bool CheckIsDraw, bool CheckIsNotNull, bool CheckIsNotNullHintTex)
{
	face &Face = *this;
	if (
		( ( Face.fID == CheckForID )		 || CheckForID == -1 ) &&
		( ( Face.draw && CheckIsDraw )		 || !CheckIsDraw ) &&
		( ( !Face.IsNULL && CheckIsNotNull)	 || !CheckIsNotNull ) &&
		(	(
			Face.Texture != "NULL" &&
			Face.Texture != "SOLIDHINT" &&
			Face.Texture != def_nulltex &&
			CheckIsNotNullHintTex
			) || !CheckIsNotNullHintTex
		)
	) {
		//if(CheckForID==-1&&!CheckIsDraw&&!CheckIsNotNull)cout << "++++++ TRUE +++++++ fID " << Face.fID << " CHK(" << CheckForID << ") Face.draw " << Face.draw << " CHK(" << CheckIsDraw << ") !Face.Null " << Face.IsNULL << " CHK(" << CheckIsNotNull << ") Face.Tex " << Face.Texture << " CHK(" << CheckIsNotNullHintTex <<")" << endl;
		return 1;
	}
	else
	{
		//if(CheckForID==-1&&!CheckIsDraw&&!CheckIsNotNull)cout << "++++++ FALSE ++++++ fID " << Face.fID << " CHK(" << CheckForID << ") Face.draw " << Face.draw << " CHK(" << CheckIsDraw << ") !Face.Null " << Face.IsNULL << " CHK(" << CheckIsNotNull << ") Face.Tex " << Face.Texture << " CHK(" << CheckIsNotNullHintTex <<")" << endl;
		return 0;
	}
}

void face::Move(gvector &Vec)
{
	for (int v = 0; v<vcount; v++)
	{ Vertices[v].move(Vec.x,Vec.y,Vec.z); }
}

void face::AddNewVertex(vertex N)
{
	// create new vertex array
	vertex *NewVerts = new vertex[vcount+1];
	
	// copy old array
	for (int v=0; v<vcount; v++)
	{
		NewVerts[v] = Vertices[v];
	}
	NewVerts[vcount] = NewVerts[vcount-1]; // copy last original vertex, to copy possible settings
	NewVerts[vcount].setall(N.x,N.y,N.z); // copy coords of new vertex
	
	delete[] Vertices;
	Vertices = NewVerts;
	vcount++;
	
	SortVertices(Normal);
}

void face::MiniShift()
{
	#if DEBUG > 0
	bool dev = 0;
	if(dev) cout << " Minishift()..." << endl;
	#endif
	
	int tex_w = gFile->tTable_width[tID];
	int tex_h = gFile->tTable_height[tID];
	
	for (int a=0; a<2; a++)
	{
		if ((a==0&&ShiftX!=0)||(a==1&&ShiftY!=0))
		{
			float *Shift;
			if (a==0) Shift = &ShiftX;
			else      Shift = &ShiftY;
			
			int TSize = 0;
			if(a==0) TSize = tex_w;
			else     TSize = tex_h;
			
			int m = *Shift / TSize;
			
			#if DEBUG > 0
			if (dev) cout << " Shift " << *Shift << " TSize " << TSize << " m " << m;
			#endif
			
			if (*Shift < 0) *Shift += TSize*m;
			if (*Shift >= TSize) *Shift -= TSize*m;
			
			#if DEBUG > 0
			if (dev) cout << " Shift New " << *Shift << endl;
			#endif
		}
	}
}

void face::RoundVertices()
{
	for (int v = 0; v<vcount; v++)
	{
		vertex &V = Vertices[v];
		if (V.DoRound)
		{
			V.x = round(V.x);
			V.y = round(V.y);
			V.z = round(V.z);
		}
	}
}

void face::pushVerts (int i)
{
	vertex last, temp;
	for (int j = 0; j<i; j++)
	for (int v = 0; v<vcount; v++)
	{
		if (v==0)
		{
			last = Vertices[0];
			Vertices[0] = Vertices[vcount-1];
		}
		else
		{
			temp = Vertices[v];
			Vertices[v] = last;
			last = temp;
		}
	}
}

void face::GetNormal()
{
	vertex p1, p2, p3;
	p1 = Vertices[1];
	p2 = Vertices[0];
	p3 = Vertices[2];
	gvector Vec1 = GetVector(p1,p2);
	gvector Vec2 = GetVector(p1,p3);
	Normal = Normalize(GetCross(Vec1, Vec2));
	
	//cout << "Normalvector from points: " << p1 << p2 << p3 << " and vectors " << Vec1 << Vec2 << " is " << Normal << endl;
}

// check whether or not this face is planar
bool face::GetPlanarity()
{
	face &Face = *this;
	
	#if DEBUG > 0
	bool dev = 0;
	if(dev) cout << " Checking Planarity of Face Tex " << Face.Texture << " vcount " << Face.vcount << endl;
	#endif
	
	int vc = Face.vcount;
	gvector Edges[vc];
	gvector Normals[vc];
	float Scalar[vc];
	
	// create NormalVectors from all Face-Edges
	for (int i = 0; i<vc; i++)
	{
		if (i==vc-1)
		Edges[i] = GetVector(Face.Vertices[i],Face.Vertices[0]);
		else
		Edges[i] = GetVector(Face.Vertices[i],Face.Vertices[i+1]);

		#if DEBUG > 0
		if (dev) cout << "  Edge " << i << " " << Edges[i] << " Vertices " << Face.Vertices[i] << Face.Vertices[i+1] << endl;
		#endif
	}
	for (int i = 0; i<vc; i++)
	{
		if (i==vc-1)
		Normals[i] = Normalize(GetCross(Edges[i],Edges[0]));
		else
		Normals[i] = Normalize(GetCross(Edges[i],Edges[i+1]));
		
		#if DEBUG > 0
		if(dev) cout << "  Normal " << i << Normals[i] << endl;
		#endif
	}
	//get scalars of all vectors and vector 0
	for (int i = 0; i<vc-1; i++)
	{
		Scalar[i] = GetDot(Normals[i+1], Normals[0]);
		Scalar[i] = ceilf(Scalar[i] * 100) / 100;
		
		#if DEBUG > 0
		if(dev) cout << "  Scalar " << i << " " << Scalar[i] << " from Normal " << Normals[i+1] << " and " << Normals[0] << endl;
		#endif
	}
	//compare all normals with first one 
	for (int i = 1; i<vc-1; i++)
	{
		if (Scalar[i]!=Scalar[0])
		{
			#if DEBUG > 0
			if(dev) cout << "   Scalar " << i << " (" << Scalar[i] << ") differs from Scalar 0 " << Scalar[0] << " Face is COplanar!" << endl;
			#endif
			
			Face.IsPlanar = 0;
			
			return false;
		}
		else
		{
			#if DEBUG > 0
			if(dev) cout << "   Scalar " << i << " (" << Scalar[i] << ") is equal to Scalar 0 " << Scalar[0] << " Face is planar so far!" << endl;
			#endif
		}
	}
	return true;
}

float face::GetLenHor(bool UseEdge)
{
	#if DEBUG > 0
	bool dev = 0;
	if(dev) cout << "      LLLLLLLLLLLLL GetLenHor --- Option useEdge: " << UseEdge << endl;
	#endif
	
	face &Face = *this;
	gvector Vec;
	Face.LenHO = Face.LenH;
	
	if (!UseEdge)	Vec = *Face.VecH;
	else			Vec = Face.EdgeH;
	
	Face.LenH = GetAdjaLen(Face.Hypo, Vec);

	#if DEBUG > 0
	if(dev) cout << "      LLLLLLLLLLLLL        Face.LenHO ["<< setw(8) << Face.LenHO << "] Vec "<< Vec << " Face.LenH ["<< setw(8) << Face.LenH << "] Face.Hypo " << Face.Hypo <<endl;
	#endif
	
	return Face.LenH;
}

float face::GetLenVer(bool UseEdge)
{
	face &Face = *this;
	gvector Vec;
	Face.LenVO = Face.LenV;
	
	if (!UseEdge)	Vec = *Face.VecH;
	else			Vec = Face.EdgeH;
	
	Face.LenV = GetOppoLen(Face.Hypo, Vec);
	
	return Face.LenV;
}

void face::RotateVertices(float x, float y, float z)
{
	face &Face = *this;
	
	for (int i = 0; i<vcount; i++)
	{
		vertex &V = Face.Vertices[i];
		V.rotate(x,y,z);
	}
}

void face::RefreshTent(face &Base)
{
	#if DEBUG > 0
	bool dev = 0;
	#endif
	
	face &Face = *this;
	if (Face.IsNULL)
	{
		#if DEBUG > 0
		if (dev) cout << "   Face Normal " << Base.Normal << " Centroid " << Base.Centroid << " IsNULL " << Face.IsNULL << " Tent ID " << Face.TentID << " new Tent ";
		#endif
		
		vertex &Tent = Face.Vertices[Face.TentID];
		Tent = Base.Centroid;
		gvector Normal = Base.Normal.flip();
		Normal.mult(def_spikesize);
		Tent.Add(Normal);
		
		#if DEBUG > 0
		if (dev) cout << Tent << endl;
		#endif
	}
}

void face::SortVertices(gvector nVec)
{
	face &Face = *this;
	
	#if DEBUG > 0
	bool dev = 0;
	if(dev) cout<< " Sorting Face Vertices - Tex " << Face.Texture << " nVec "<< nVec << endl;
	#endif
	
	Face.Normal = nVec;
	
	face Face2; Face2.CopyFace(Face,1);
	float Yaw = GetVecAlign(Normal,0);
	Face2.RotateVertices(0,0,-Yaw);
	Face2.Normal.rotate(0,0,-Yaw);
	
	float Pitch = GetVecAlign(Face2.Normal,1);
	Face2.RotateVertices(0,Pitch,0);
	
	#if DEBUG > 0
	if(dev) Face2.Normal.rotate(0,Pitch,0);
	#endif
	
	Face2.RotateVertices(0,-90,0);

	#if DEBUG > 0
	if(dev) Face2.Normal.rotate(0,-90,0);
	if(dev) cout<< "  Face Yaw " << Yaw << " Pitch " << Pitch << endl;
	#endif
	
	Face2.GetCentroidClassic();

	#if DEBUG > 0
	if(dev) cout << "   Face2 Normal " << Face2.Normal << " Yaw " << GetVecAlign(Face2.Normal,0) << " Pitch " << GetVecAlign(Face2.Normal,1) << endl;
	if(dev) cout << "   Face2 Centroid " << Face2.Centroid << endl;
	#endif
	
	vector<int> Sorted; Sorted.resize(vcount,0);

	// Get Yaw Angles
	#if DEBUG > 0
	if(dev) cout<<"   Getting Yaw Angles..." << endl;
	#endif
	
	vector<float> ListYaw; ListYaw.resize(vcount,0);
	for(int v=0; v<vcount; v++)
	{
		vertex &V = Face2.Vertices[v];
		gvector Vec = GetVector(Face2.Centroid,V);
		ListYaw[v] = GetVecAlign(Vec,0);
		
		#if DEBUG > 0
		if(dev) cout<< "      vertex #" << v << " Yaw " << ListYaw[v] << endl;
		#endif
	}
	
	int small = 0, big = 0;
	for(int v=1; v<vcount; v++) if(ListYaw[v]<ListYaw[small]) small = v;
	for(int v=1; v<vcount; v++) if(ListYaw[v]>ListYaw[big]) big = v;
	Sorted[0] = small;
	Sorted[Sorted.size()-1] = big;
	fill(Sorted.begin()+1,Sorted.end()-1,big);
	
	// get sorted Vertex index list based on Yaw
	for(int s=1; s<vcount-1; s++)
	{
		int &Ind_C = Sorted[s];
		int &Ind_L = Sorted[s-1];
		float &Ang_L = ListYaw[ Ind_L ]; // the current minimum yaw
		// check all yaws for one that is NOT smaller than current minimum and yet the smallest of all the remaining ones
		for(int v=0; v<vcount; v++)
		{
			float &Candi = ListYaw[v];
			if(Candi>Ang_L && Candi<ListYaw[ Sorted[s] ])
			{
				Ind_C = v;
				
				#if DEBUG > 0
				if(dev)cout << "        Changing Current Index to " << v << "("<< ListYaw[ Sorted[s] ] <<")" <<endl;
				#endif
			}
			
			#if DEBUG > 0
			if(dev)cout << "        Sorted #" << s << " Ind_C " << Ind_C << " LastYaw " << Ang_L << " CurYaw " << ListYaw[ Sorted[s] ] << " Candi #" << v << " " << Candi << endl;
			#endif
		}
	}
	
	#if DEBUG > 0
	if(dev) {
	cout << "     Sorted Vertex Index List..." << endl;
	for(int v=0; v<Sorted.size(); v++) { int &V = Sorted[v]; cout << "        #" << v << " Index " << Sorted[v] << " Yaw " << ListYaw[ Sorted[v] ] << endl; } }
	#endif
	
	// create new vertex array and replace old one
	vertex *V_New = new vertex[vcount];
	for(int i=0; i<vcount; i++)
	{
		vertex &V = V_New[i];
		vertex &N = Vertices[ Sorted[i] ];
		V = N;
	}
	delete[] Vertices;
	Vertices = V_New;
	
	//if(Face2.Normal.z>0)
	Face.RevOrder(0);
	
	#if DEBUG > 0
	if(dev) {
	cout << "     Final Vertex List..." << endl;
	for(int v=0; v<vcount; v++) { cout << "        #" << v << " " << Vertices[v] << endl; } system("pause"); cout << endl; }
	system("pause");
	#endif
}

void face::ConvertToSheared(int g)
{
	#if DEBUG > 0
	bool dev = 0;
	#endif
	
	face &Face = *this;
	bool UseLongEdge = cTable[g].hshiftsrc;
	
	GetBaseEdges(Face);
	GetBaseShift(Face,0,1,0);
	
	if(UseLongEdge)
	Face.OffsetX = Face.HSourceL->OffsetX;
	else
	Face.OffsetX = Face.HSourceS->OffsetX;
		
	Face.RefreshEdges();
	Face.GetLenHor(0);
	
	#if DEBUG > 0
	if(dev) {cout << " Shearing Face " << Face.name << endl << " Tex " << Face.Texture << endl << " Orient " << Face.Orient << endl << " Offset ";
	if (Face.VecX.IsHor) cout << OffsetX << endl; else cout << OffsetY << endl;
	cout << " Shift "; 		if (Face.VecX.IsHor) cout << ShiftX << endl; else cout << ShiftY << endl;
	cout << " BaseShift ";	if (Face.VecX.IsHor) cout << BaseShiftX << endl; else cout << BaseShiftY << endl; }
	#endif
	
	if(Face.Orient==2||Face.Orient==3)
	{
		// get new Face Vectors
		gvector VecH_Old = *Face.VecH;
		Face.GetNormal();
		gvector Cross = Normalize( GetCross(Face.Normal, Face.EdgeV) );
		Face.VecH->CopyCoords(Cross);
		if(GetDot(*Face.VecH, VecH_Old)<0) Face.VecH->flip();
		
		// get old Hor Lengths
		float HLenO = EdgeLenL;
		
		// get new lengths, texture, scale and shift
		float HLenN=0;
		if		(vcount==4&&Face.Orient==3) HLenN = GetAdjaLen(GetVector(Vertices[1],Vertices[3]), *Face.VecH);
		else if (vcount==4&&Face.Orient==2) HLenN = GetAdjaLen(GetVector(Vertices[0],Vertices[2]), *Face.VecH);
		else if (vcount==3) HLenN = GetAdjaLen(GetVector(Vertices[0],Vertices[2]), *Face.VecH);
		
		float m = HLenO/HLenN;
		if(Face.VecX.IsHor) {
			Face.ScaleX /= m; } else {
			Face.ScaleY /= m; }
		
		GetBaseShift(Face,0,1,0);
		
		if(Face.VecX.IsHor)
		{ Face.ShiftX = Face.BaseShiftX + Face.OffsetX; }
		else
		{ Face.ShiftY = Face.BaseShiftY + Face.OffsetY; }
		
		#if DEBUG > 0
		if(dev){
			cout << " Hypo " << Hypo << " (Len " << GetVecLen(Hypo) << ")" << endl;
			cout << " VecH " << *VecH << endl;
			cout << " VecH_OLD " << VecH_Old << endl;
			cout << " EdgeH " << EdgeH << " (Len " << GetVecLen(EdgeH) << ")" << endl;
			cout << " HSourceL " << Face.HSourceL->EdgeH << " name " << Face.HSourceL->name <<" (Len " << Face.HSourceL->EdgeLenL << ")" << endl;
			cout << " HSourceS " << Face.HSourceS->EdgeH << " name " << Face.HSourceS->name <<" (Len " << Face.HSourceS->EdgeLenS << ")" << endl;
			cout << " HLenO " << HLenO << "(made of Face.HSourceL->EdgeLenL ("<<Face.HSourceL->EdgeLenL<<"))" << endl;
			cout << " HLenN " << HLenN << endl;
			cout << " m " << m << endl;
			cout << " Scale "; if(Face.VecX.IsHor) cout << ScaleX<<endl; else cout << ScaleY << endl;
			cout << " BaseShift "; if(Face.VecX.IsHor) cout << BaseShiftX<<endl; else cout << BaseShiftY << endl;
			cout << " Shift "; if(Face.VecX.IsHor) cout << ShiftX<<endl; else cout << ShiftY << endl;
			cout << " Hor Vec? "; if(Face.VecX.IsHor) cout <<"X"<<endl; else cout << "Y" << endl;
		}
		#endif
	}
	else
	{
		// get old Hor Face Lengths
		float HLenO = Face.HSourceL->EdgeLenL;
		float HLenN = Face.EdgeLenL;
		float m = HLenO / HLenN;
		
		#if DEBUG > 0
		if(dev) cout << " HLenO " << HLenO << " HLenN " << HLenN << " m " << m << endl;
		#endif
		
		GetBaseShift(Face,0,1,0);
		
		if(Face.VecX.IsHor)
		{ Face.ShiftX = Face.BaseShiftX + Face.OffsetX; }
		else
		{ Face.ShiftY = Face.BaseShiftY + Face.OffsetY; }
	}
	
	#if DEBUG > 0
	if(dev) system("pause");
	if(dev) cout << endl;
	#endif
}

void face::ConvertToShearedTri(bool IsLongEdge, bool IsInside, bool Reverse, brush &Brush, int g)
{
	face &Face = *this;

	#if DEBUG > 0
	bool dev = 0;
	#endif
	
	bool UseLongEdge = cTable[g].hshiftsrc;
	// Determine whether this wedge is the long or the short one in the case of a reversed spline extrusion
	GetBaseEdges(Face);
	GetBaseShift(Face,0,1,0);
	
	if(UseLongEdge)
	Face.OffsetX = Face.HSourceL->OffsetX;
	else
	Face.OffsetX = Face.HSourceS->OffsetX;
	
	Face.RefreshEdges();
	
	#if DEBUG > 0
	if(dev) {cout << endl << " Shearing Face Tex " << Face.Texture << " Orient " << Face.Orient << " IsWedge " << Brush.IsWedge;
	if (IsLongEdge) cout << " LONG Wedge"; else cout <<" SHORT Wedge";
	if (IsInside) cout << " INSIDE" << endl; else cout <<" OUTSIDE"; cout << endl;
	}
	#endif
	
	if(Face.Orient==2||Face.Orient==3) // new Vectors, Scales and Shifts for up/down faces
	{
		// get new Face Vectors
		gvector VecH_Old = *Face.VecH;
		Face.GetNormal();
		gvector Cross = Normalize( GetCross(Face.Normal, Face.EdgeV) );
		Face.VecH->CopyCoords(Cross);
		if(GetDot(*Face.VecH, VecH_Old)<0) Face.VecH->flip();
		
		// get old Hor Lengths
		float 			 HLenO = Face.HSourceL->EdgeLenL;
		if(!UseLongEdge) HLenO = Face.HSourceS->EdgeLenS;

		// get new lengths, texture, scale and shift
		float HLenN=0;
		if(!Brush.IsWedge) 		HLenN = Face.GetLenHor(0);
		else if(Brush.IsWedge)	HLenN = GetAdjaLen(GetVector(Vertices[0],Vertices[2]), *Face.VecH);
		//HLenN = Face.GetLenHor(0);
		if(HLenN<0) HLenN=-HLenN;
		
		float m = HLenO/HLenN;
		
		if(Face.VecX.IsHor) {
			Face.ScaleX /= m; } else {
			Face.ScaleY /= m; }
		
		GetBaseShift(Face,0,1,0);
		
		if(Face.VecX.IsHor)
		{ Face.ShiftX = Face.BaseShiftX + Face.OffsetX; }
		else
		{ Face.ShiftY = Face.BaseShiftY + Face.OffsetY; }
		
		#if DEBUG > 0
		if(dev) cout << "     HLenO " << HLenO << " HLenN " << HLenN <<  " OffsetX " << Face.OffsetX << " BaseShiftX " << Face.BaseShiftX << " ShiftX " << Face.ShiftX << " m " << m << " ScaleX " << Face.ScaleX << endl;
		if(dev) cout << " Sec " << Brush.SecID << " Orient " << Face.Orient << " OffsetX " << setw(5) << round(Face.OffsetX*100)/100.0 << " BaseShiftX " << setw(5) << round(Face.BaseShiftX*100)/100.0 << " ShiftX " << setw(5) << round(Face.ShiftX*100)/100.0 << " m " << setw(3) << round(m*10)/10.0 << " ScaleX " << setw(4) << round(Face.ScaleX*10)/10.0 << endl;
		#endif
	}
	else // Upright front and back faces don't need new vectors, only new Scales and Shifts
	{
		// get difference between new and old horizontal face/texture lengths
		float HLenO 			= Face.HSourceL->EdgeLenL;
		if(!UseLongEdge) HLenO 	= Face.HSourceS->EdgeLenS;
		float HLenN 			= Face.GetLenHor(0);
		
		float m 			= HLenO / HLenN;
		if(m<0) m			= -m;
		
		#if DEBUG > 0
		if(dev) cout << " HLenO " << HLenO << " HLenN " << HLenN << " m " << m << endl;
		if(dev) cout << " ScaleX OLD " << setw(7) << Face.ScaleX << " ScaleY OLD " << setw(7) << Face.ScaleY << " ShiftX OLD " << setw(7) << Face.ShiftX << " ShiftY OLD " << setw(7) << Face.ShiftY  << endl;
		#endif
		
		// compose new horizontal texture Scale and Shift
		if(Face.VecX.IsHor)
		{ Face.ScaleX /= m; }
		else
		{ Face.ScaleY /= m; }
		
		GetBaseShift(Face,0,1,0);
		
		if(Face.VecX.IsHor)
		{ Face.ShiftX = Face.BaseShiftX + Face.OffsetX; }
		else
		{ Face.ShiftY = Face.BaseShiftY + Face.OffsetY; }
		
		#if DEBUG > 0
		if(dev) cout << " ScaleX NEW " << setw(7) << Face.ScaleX << " ScaleY NEW " << setw(7) << Face.ScaleY << " ShiftX NEW " << setw(7) << Face.ShiftX << " ShiftY NEW " << setw(7) << Face.ShiftY  << endl<<endl;
		if(dev) cout << " Sec " << setw(2) << Brush.SecID << " Orient " << Face.Orient << " OffsetX " << setw(5) << round(Face.OffsetX*100)/100.0 << " BaseShiftX " << setw(5) << round(Face.BaseShiftX*100)/100.0 << " ShiftX " << setw(5) << round(Face.ShiftX*100)/100.0 << " m " << setw(2) << round(m*10)/10.0 << " ScaleX " << setw(4) << round(Face.ScaleX*10)/10.0 << endl;
		#endif
	}
	
	#if DEBUG > 0
	//if(dev) system("pause");
	if(dev) cout << endl;
	#endif
}

int face::IsFaceBeyondPlane(gvector nVec)
{
	#if DEBUG > 0
	bool dev = 0;
	if(dev) cout << "    +----" << endl << "    | IsFaceBeyondPlane?..." << endl;
	#endif
	
	int Invalids = 0;
	// for each vertex of the face, check if it is beyond the given plane
	for(int v=0; v<vcount; v++)
	{
		vertex &V = Vertices[v];
		vertex Origin(nVec.px,nVec.py,0);
		vertex Pos(V.x,V.y,0);
		gvector Hypo(Origin, Pos);
		float AdjaLen = GetAdjaLen(Hypo,nVec);
		
		#if DEBUG > 0
		if(dev) cout << "    |   #"<<v<<" AdjaLen " << AdjaLen << " Origin " << Origin << " V " << V << " HypoLen " << GetVecLen(Hypo) << endl;
		#endif
		
		if(AdjaLen>0) { V.IsValid = 0; Invalids++; }
	}
	if(Invalids==vcount)
	{
		// All Vertices of this face are beyond Plane. Face is completely out of bound and can be discarded!
		#if DEBUG > 0
		if(dev) cout << "    | 111111111111 All Vertices of this face are beyond Plane!" << endl << "    +----" << endl << endl;
		if(dev) system("pause");
		#endif
		
		draw = 0;
		return 1;
	}
	else if(Invalids==0)
	{
		// No Vertex of this face is beyond Plane. Face wont be carved at all!
		#if DEBUG > 0
		if(dev) cout << "    | 000000000000 No Vertex of this face is beyond Plane!" << endl << "    +----" << endl << endl;
		if(dev) system("pause");
		#endif
		
		return 0;
	}
	else
	{
		// Some vertices of this face are on the one and some on the other side of the Plane. Face will be carved!
		#if DEBUG > 0
		if(dev) cout << "    | 222222222222 Face will be carved!" << endl << "    +----" << endl << endl;
		if(dev) system("pause");
		#endif
		
		return 2;
	}
}

int face::CarveFace(gvector Plane)
{
	face &Face = *this;
	
	#if DEBUG > 0
	bool dev = 0;
	if(dev) cout << endl << "FACE Carving Face (tex " << Face.Texture << " nVec " << Normal << " vcount " <<Face.vcount<<") with Plane " << Plane << endl;
	#endif
	
	Face.GetNormal();
	int IsBeyond = Face.IsFaceBeyondPlane(Plane);
	if(IsBeyond==2)
	{
		Plane.rotate(0,0,-90); // cutting plane vector is currently a normal vector, so rotate it by 90 degree to get an intersection line
		vector<vertex> V_New;
		// carve relevant edges (one vertex in front of plane and one behind)
		for(int v=0; v<vcount; v++)
		{
			vertex &V1 = Vertices[v];
			vertex *V2_Ptr = nullptr;
			if(v<vcount-1)	V2_Ptr = &Vertices[v+1];
			else			V2_Ptr = &Vertices[0];
			vertex &V2 = *V2_Ptr;
			
			if( (V1.IsValid&&!V2.IsValid) || (!V1.IsValid&&V2.IsValid) )
			{
				// make everything flat, because this way it's easier to intersect
				gvector Line = GetVector(V1,V2);
				gvector Line_Flat = Line; Line_Flat.z = 0;
				vertex Plane_Origin(Plane.px,Plane.py,0);
				vertex V1_Flat(V1.x,V1.y,0);
				vertex Isect = GetLineIsect(V1_Flat, Plane_Origin, Line_Flat, Plane);
				
				// now get height of 2D intersection
				if(Line.z!=0)
				{
					float AdjaLen = GetAdjaLen(Line, Line_Flat);
					gvector Section = GetVector(V1_Flat, Isect);
					float Sec_Len = GetVecLen(Section);
					float m = Sec_Len/AdjaLen;
					gvector Hypo_New(Line); Hypo_New.mult(m);
					Isect.z = V1.z + Hypo_New.z;
				}
				else Isect.z = V1.z;
				
				#if DEBUG > 0
				if(dev) cout << "FACE  #"<<v<<" Line_F " << Line_Flat << " V1_F " << V1_Flat << " V2 " << V2 << " Plane " << Plane << "(yaw "<<GetVecAlign(Plane,0)<<") Plane_O " << Plane_Origin << " Isect " << Isect << endl;
				#endif
				
				V_New.push_back(Isect);
			}
		}
		// delete obsolete and add new vertices from intersection
		vector<vertex> V_Clean;
		for(int v=0; v<vcount; v++)
		{
			vertex &V = Vertices[v];
			
			if(V.IsValid)
			{
				V_Clean.push_back(V);
				
				#if DEBUG > 0
				if(dev) cout << "FACE    Clean Vertex #" << v << V << endl;
				#endif
			}
		}
		
		for(int v=0; v<V_New.size(); v++)
		{
			vertex &V = V_New[v];
			V_Clean.push_back(V);
			
			#if DEBUG > 0
			if(dev) cout << "FACE    New Vertex #" << v << V << endl;
			#endif
		}
		
		vertex *V_Final = new vertex[V_Clean.size()];
		
		for(int v=0; v<V_Clean.size(); v++)
		{
			V_Final[v] = V_Clean[v];
			
			#if DEBUG > 0
			if(dev) cout << "FACE    Final Vertex #" << v << V_Final[v] << endl;
			#endif
		}
		
		delete[] Vertices;
		Vertices = V_Final;
		vcount = V_Clean.size();
		
		// sort final vertices
		Face.SortVertices(Face.Normal);
		
		#if DEBUG > 0
		if(dev) cout << endl << "FACE END (vcount now " <<Face.vcount<<")" << endl;
		#endif
		
		return 2;
	}
	else if(IsBeyond==1) return 1;
	else if(IsBeyond==0) return 0;
}

void face::CreateRamp(int g, int b, int f, int SecID, bool IsWedge2, float Bstep)
{
	bool Tri = cTable[g].tri;
	bool UseLongEdge = cTable[g].hshiftsrc;
	face &Face = *this;
	
	#if DEBUG > 0
	bool dev = 0;
	if(dev) {cout << endl << " Creating Ramp from Face Tex " << Face.Texture << " Orient " << Face.Orient << " IsNull " << Face.IsNULL << " Wedge " << IsWedge2 << endl;}
	#endif
	
	int sec = SecID;
	if ( !Face.FaceIsValid(-1,0,0,1) ) Face.IsNULL=1;
	
	vertex *V0ptr=nullptr, *V1ptr=nullptr, *V2ptr=nullptr, *V3ptr=nullptr;
	V0ptr = &Face.Vertices[0];
	V1ptr = &Face.Vertices[1];
	V2ptr = &Face.Vertices[2];
	if (Face.vcount>3)
	V3ptr = &Face.Vertices[3];
	vertex &V0 = *V0ptr, &V1 = *V1ptr, &V2 = *V2ptr, &V3 = *V3ptr;
	
	/*=========================================================================++
	|| Height Tables, steps
	++=========================================================================++*/
	#if DEBUG > 0
	if (dev) cout << "/....................     Height Tables..." << endl;
	#endif
	
	float step 		= bGroup[g].heightTableSteps[SecID];
	int start 		= bGroup[g].range_start;
	
	if (step!=0&&Face.draw)
	{
		gvector EdgeH;
		gvector EdgeV;
		gvector OldNormal = *Face.VecH;
		
		float HLengthO = 1;
		float VLengthO = 1;
		float HLengthN = 1;
		float VLengthN = 1;
			
		if (Face.fID==0) {}
		else if (Face.fID==1)
		{
			GetBaseShift(Face,0,1,0);
			GetTexOffset(Face, 0);
			Face.AddHeight(step);
			GetBaseShift(Face,0,1,0);
			Face.ShiftX = Face.BaseShiftX + Face.OffsetX;
			Face.ShiftY = Face.BaseShiftY + Face.OffsetY;
		}
		else if (Face.fID==2&&!Face.IsNULL)
		{
			#if DEBUG > 0
			if(dev)
			{
				cout << "/....................     Brush " <<b<<" Face "<<f<<" Sec " << sec << " Tex "<<Face.Texture << " ScaleX " << Face.ScaleX << " Scale Y " <<  Face.ScaleY << " Wedge " << IsWedge2;
				if (Face.Orient==2) {cout << " Orient UP"; }
				if (Face.Orient==3) {cout << " Orient DN"; }
				if (Face.Orient==4) {cout << " Orient FT"; }
				if (Face.Orient==5) {cout << " Orient BK"; }
				if (Face.Orient==6) {cout << " Orient NULL"; }
				cout << endl;
			}
			#endif
			
			/*=========================================================================++
			|| Get previously calculated Offsets, Centroid, BaseEdges and BaseShift
			++=========================================================================++*/
			#if DEBUG > 0
			if (dev) cout << "/....................     Get previously calculated Offsets, Centroid, BaseEdges and BaseShift..." << endl;
			#endif
			
			Face.GetCentroid();
			
			#if DEBUG > 0
			if(dev)cout << "/....................     Tex Offsets PRE X " << Face.OffsetX << " Y " << Face.OffsetY << " BaseVertexX " << Face.BaseX << " BaseVertexY " << Face.BaseY << " VecX "<<Face.VecX<<" VecY "<<Face.VecY<<" Centroid " << Face.Centroid << endl;
			#endif
			
			GetBaseEdges(Face);
			GetBaseShift(Face,0,1,0);
			GetTexOffset(Face, 0);
			
			#if DEBUG > 0
			if(dev)cout << "/....................     Tex Offsets AFT X " << Face.OffsetX << " Y " << Face.OffsetY << " BaseVertexX " << Face.BaseX << " BaseVertexY " << Face.BaseY << endl;
			#endif
			
			/*=========================================================================++
			|| Get Original Face Lengths first
			++=========================================================================++*/
			#if DEBUG > 0
			if (dev) cout << "/....................     Get Original Face Lengths first..." << endl;
			#endif
			
			if (cTable[g].ramptex==0)
			{
				if(cTable[g].texmode==1) // texture shearing on
				{
					if (Face.Orient!=4&&Face.Orient!=5)
					{
						HLengthO = Face.GetLenHor(0);
						VLengthO = Face.GetLenVer(1);
					}
					else VLengthO = Face.GetLenVer(0);
				}
				else // texture shearing off
				{
					if (Face.Orient!=4&&Face.Orient!=5)
					HLengthO = Face.GetLenHor(0);
					VLengthO = Face.GetLenVer(0);
				}
				
				Face.LengthO = HLengthO;
			}
			else if (cTable[g].ramptex==1)
			{
				if (Face.Orient!=4&&Face.Orient!=5)
				HLengthO = Face.GetLenHor(1);
				VLengthO = Face.GetLenVer(1);
				Face.LengthO = HLengthO;
			}
			#if DEBUG > 0
			if(dev)cout << "/....................     OLD Lengths: H " << HLengthO << " V " << VLengthO << endl;
			#endif
		}
		
		if (Face.fID==2)
		{
			/*=========================================================================++
			|| Add Height
			++=========================================================================++*/
			for (int v = 0; v<Face.vcount; v++)
			{
				vertex &V = Face.Vertices[v];
				
				if (V.DoAddHeight) V.z+=step;
			}
		}
			
		if (Face.fID==2&&!Face.IsNULL)
		{
			Face.RefreshEdges(); // Important!!
			Face.GetNormal();
			
			gvector Normal = Face.Normal;
			gvector Hor, Ver, Cross, Cross2;
			gvector ZSpan(0,0,1);
			float DotH = 0, DotV = 0;
			
			/*=========================================================================++
			|| fix texture vector align in case of Ramp // 2 = Up, 3 = Down
			++=========================================================================++*/
			//cout << " fix texture vector align in case of Ramp..." << endl; system("pause");
			if (cTable[g].ramptex==0)
			{
				if(cTable[g].texmode==1) // texture shearing on
				{
					if (Face.Orient==2||Face.Orient==3)
					{
						if (!Tri) {
							EdgeH = Face.EdgeH;
							EdgeV = Face.EdgeV;
							EdgeH.flip();
							EdgeV.flip();
						} else {
							if (!IsWedge2) {
								EdgeH = Face.EdgeH;
								EdgeV = Face.EdgeV;
								EdgeH;
								EdgeV.flip();
							} else {
								EdgeH = Face.EdgeH;
								EdgeV = Face.EdgeV;
								EdgeH.flip();
								EdgeV;
							}
						}
						
						Cross  = GetCross( Normal, EdgeH );
						Cross2 = GetCross( Normal, EdgeV );
						Hor = Normalize( Cross2 );
						Ver = Normalize( Cross );
						DotH = GetDot (Hor, *Face.VecH); if (DotH<0) { Hor.flip(); }
						DotV = GetDot (Ver, *Face.VecV); if (DotV<0) { Ver.flip(); }
						Face.VecH->CopyCoords( Hor );
						Face.VecV->CopyCoords( Ver );
					}
					if (Face.Orient==4||Face.Orient==5) // Back/Front Faces
					{
						gvector EdgeH = Face.EdgeH;
						gvector EdgeV = Face.EdgeV;
						gvector Cross =  GetCross (Face.Normal, EdgeV);
						gvector Cross2 = GetCross (Face.Normal, EdgeH);
						
						gvector Hor = Normalize(Cross);
						gvector Ver = Normalize(Cross2);
						DotH = GetDot (Hor, *Face.VecH); if (DotH<0) { Hor.flip(); }
						DotV = GetDot (Ver, *Face.VecV); if (DotV<0) { Ver.flip(); }
						
						Face.VecH->CopyCoords( Hor );
						Face.VecV->CopyCoords( Ver );
					}
				}
				else // texture shearing off
				{
					if (Face.Orient==2) // new Texture Vectors for sloped top faces
					{
						if (!Tri) {
							EdgeH = Face.EdgeH;
							EdgeV = Face.EdgeV;
							EdgeH.flip();
							EdgeV.flip();
						} else {
							if (!IsWedge2) {
								EdgeH = Face.EdgeH;
								EdgeV = Face.EdgeV;
								EdgeH.flip();
								EdgeV.flip();
							} else {
								EdgeH = Face.EdgeH;
								EdgeV = Face.EdgeV;
							}
						}
						Cross  = GetCross( Normal, EdgeH );
						Cross2 = GetCross( Normal, *Face.VecH );
						Hor = Normalize( GetCross( Normal, Cross2 ) );
						Ver = Normalize( Cross );
						DotH = GetDot (Hor, *Face.VecH); if (DotH<0) { Hor.flip(); }
						DotV = GetDot (Ver, *Face.VecV); if (DotV<0) { Ver.flip(); }
						Face.VecH->CopyCoords( Hor );
						Face.VecV->CopyCoords( Ver );
					}
					if (Face.Orient==3) // down faces
					{
						if (!Tri) {
							EdgeH = Face.EdgeH;
							EdgeV = Face.EdgeV;
						} else {
							if (!IsWedge2) {
								EdgeH = Face.EdgeH;
								EdgeV = Face.EdgeV;
							} else {
								EdgeH = Face.EdgeH;
								EdgeV = Face.EdgeV;
								EdgeH.flip();
								EdgeV.flip();
							}
						}
						Cross  = GetCross( Normal, EdgeH );
						Cross2 = GetCross( Normal, *Face.VecH );
						Hor = Normalize( GetCross( Normal, Cross2 ) );
						Ver = Normalize( Cross );
						DotH = GetDot (Hor, *Face.VecH); if (DotH<0) { Hor.flip(); }
						DotV = GetDot (Ver, *Face.VecV); if (DotV<0) { Ver.flip(); }
						Face.VecH->CopyCoords( Hor );
						Face.VecV->CopyCoords( Ver );
					}
					if (Face.Orient==4||Face.Orient==5) // Back/Front Faces
					{
						gvector EdgeH = Face.EdgeH;
						gvector EdgeV = Face.EdgeV;
						gvector Cross =  GetCross (Face.Normal, EdgeV);
						gvector Cross2 = GetCross (Face.Normal, EdgeH);
						
						gvector Hor = Normalize(Cross);
						gvector Ver = Normalize(Cross2);
						DotH = GetDot (Hor, *Face.VecH); if (DotH<0) { Hor.flip(); }
						DotV = GetDot (Ver, *Face.VecV); if (DotV<0) { Ver.flip(); }
						
						Face.VecH->CopyCoords( Hor );
						Face.VecV->CopyCoords( Ver );
					}
				}
			}
			else if (cTable[g].ramptex == 1)
			{
				if (Face.Orient==2)
				{
					// new Texture Vectors for sloped top/down faces
					if (!Tri)
					{
						EdgeH = GetVector(V0, V3);
						EdgeV = GetVector(V0, V1);
						EdgeH.flip();
						EdgeV.flip();
					}
					else
					{
						if (!IsWedge2)
						{
							EdgeH = GetVector(V0, V2);
							EdgeV = GetVector(V0, V1);
							EdgeH.flip();
							EdgeV.flip();
						}
						else
						{
							EdgeH = GetVector(V1, V0);
							EdgeV = GetVector(V1, V2);
						}
					}
					Normal = GetCross(EdgeV, EdgeH);
					Ver = Normalize(GetCross(EdgeH, Normal));
					Hor = Normalize(GetCross(Ver, Normal));
					float DotH = GetDot (Hor, *Face.VecH);
					float DotV = GetDot (Ver, *Face.VecV);
					if (DotH<0) { Hor.flip(); }
					if (DotV<0) { Ver.flip(); }
					Face.VecH->CopyCoords( Hor );
					Face.VecV->CopyCoords( Ver );
				}
				if (Face.Orient==3)
				{
					if (!Tri)
					{
						EdgeH = GetVector(V1, V2);
						EdgeV = GetVector(V1, V0);
					}
					else
					{
						if (!IsWedge2)
						{
							EdgeH = GetVector(V1, V2);
							EdgeV = GetVector(V1, V0);
						}
						else
						{
							EdgeH = GetVector(V1, V2);
							EdgeV = GetVector(V1, V0);
							EdgeH.flip();
							EdgeV.flip();
						}
					}
					Normal = GetCross(EdgeV, EdgeH);
					Ver = Normalize(GetCross(EdgeH, Normal));
					Hor = Normalize(GetCross(Ver, Normal));
					Hor.flip();
					Ver.flip();
					float DotH = GetDot (Hor, *Face.VecH);
					float DotV = GetDot (Ver, *Face.VecV);
					if (DotH<0) { Hor.flip(); }
					if (DotV<0) { Ver.flip(); }
					Face.VecH->CopyCoords( Hor );
					Face.VecV->CopyCoords( Ver );
				}
				if (Face.Orient==4||Face.Orient==5) // Back/Front Faces
				{
					Face.GetNormal();
					gvector EdgeH = GetVector(V1, V2);
					gvector EdgeV = GetVector(V0, V1);
					Cross = GetCross (Face.Normal, EdgeH);
					Cross2 = GetCross (Face.Normal, EdgeV);
					gvector Hor = Normalize(EdgeH);
					gvector Ver = Normalize(Cross);
					Hor.flip();
					if (Face.Orient==4) Ver.flip();
					if (Face.VecH->IsNeg) { Hor.flip(); }
					if (Face.VecV->IsNeg) { Ver.flip(); }
					Face.VecH->CopyCoords( Hor );
					Face.VecV->CopyCoords( Ver );
				}
			}
			
			/*=========================================================================++
			|| Get new Face Lengths
			++=========================================================================++*/
			if (cTable[g].ramptex==0)
			{
				if (Face.Orient!=4&&Face.Orient!=5)
				HLengthN = Face.GetLenHor(0);
				VLengthN = Face.GetLenVer(1);
				Face.LengthN = VLengthN;
			}
			else if (cTable[g].ramptex==1)
			{
				if (Face.Orient!=4&&Face.Orient!=5)
				HLengthN = Face.GetLenHor(1);
				VLengthN = Face.GetLenVer(1);
				Face.LengthN = VLengthN;
			}
			#if DEBUG > 0
			if(dev)cout << "/....................     NEW Lengths: H " << HLengthN << " V " << VLengthN << endl;
			#endif

			/*=========================================================================++
			|| Re-compose textures of Body Faces
			++=========================================================================++*/
			float w2mh;
			if (IsWedge2&&cTable[g].ramptex==1)
				w2mh = bGroup[g].Brushes[b-1].Faces[f].LengthO / bGroup[g].Brushes[b-1].Faces[f].LengthN;

			#if DEBUG > 0
			if(dev)cout << "/....................     w2mh " << w2mh <<  endl;
			#endif
			
			float mh = HLengthO/HLengthN;
			float mv = VLengthO/VLengthN;
			float mx, my;
			
			if (Face.VecX.IsHor) 	{mx = mh; my = mv;}
			else 					{mx = mv; my = mh;}
			
			#if DEBUG > 0
			if(dev)cout << "/....................     mx " << mx << " my " << my << endl;
			if(dev)cout << "/....................     ScaleX (" << Face.ScaleX << ") /= mx (" << mx << ") -> " << Face.ScaleX / mx << endl;
			if(dev)cout << "/....................     ScaleY (" << Face.ScaleY << ") /= my (" << my << ") -> " << Face.ScaleY / my << endl;
			#endif
			
			// Fix Scales 
			Face.ScaleX /= mx;
			Face.ScaleY /= my;
			
			if (  IsWedge2 && (  Face.Orient==2 || Face.Orient==3  ) && cTable[g].ramptex==1  )
			{
				if (Face.VecX.IsHor) 	{ Face.ScaleX *= w2mh; }
				else 					{ Face.ScaleY *= w2mh; }
			}
			
			#if DEBUG > 0
			if(dev)cout << "/....................     Face Tex BaseShifts PRE X " << Face.BaseShiftX << " Y " << Face.BaseShiftY << endl;
			#endif
			
			GetBaseShift(Face,0,1,0);

			#if DEBUG > 0
			if(dev)cout << "/....................     Face Tex BaseShifts AFT X [" << Face.BaseShiftX << "] IsHor? (" << Face.VecX.IsHor << ") Y [" << Face.BaseShiftY << "] IsHor? (" << Face.VecY.IsHor << ")" <<endl;
			#endif
			
			/*--------------------------------------------------+
			| Compose Face Shift
			+---------------------------------------------------+*/
			
			#if DEBUG > 0
			if(dev)cout << "/....................     ShiftX (" << Face.ShiftX << ") = BaseShiftX " << Face.BaseShiftX << " + OffsetX (" << Face.OffsetX << ") = " << Face.BaseShiftX + Face.OffsetX << endl;
			if(dev)cout << "/////////////////////     ShiftY (" << Face.ShiftY << ") = BaseShiftY " << Face.BaseShiftY << " + OffsetY (" << Face.OffsetY << ") = " << Face.BaseShiftY + Face.OffsetY  << endl;
			#endif
			
			Face.ShiftX = Face.BaseShiftX;
			Face.ShiftY = Face.BaseShiftY;
			
			if (cTable[g].ramptex==0)
			{
				Face.ShiftX += Face.OffsetX;
				Face.ShiftY += Face.OffsetY;
			}
			else if (cTable[g].ramptex==1)
			{
				if (Face.VecX.IsHor)
				Face.ShiftY += Face.OffsetY;
				else
				Face.ShiftX += Face.OffsetX;
			}
		}
	}
	
	#if DEBUG > 0
	if(dev)cout << " END Ramp Face Generation " << endl << endl;
	#endif
}

void face::CopyFace(face &Source, bool CopyVertices)
{
	Texture = Source.Texture;
	ShiftX 	= Source.ShiftX;
	ShiftY 	= Source.ShiftY;
	Rot 	= Source.Rot;
	ScaleX 	= Source.ScaleX;
	ScaleY 	= Source.ScaleY;
	VecX 	= Source.VecX;
	VecY 	= Source.VecY;
	if (Source.VecY.IsHor)
	{
		VecH = &VecY;
		VecV = &VecX;
	}
	fID 	= Source.fID;
	FaceAlign = Source.FaceAlign;
	OffsetX = Source.OffsetX;
	OffsetY = Source.OffsetY;
	BaseX	= Source.BaseX;
	BaseY	= Source.BaseY;
	BaseX2	= Source.BaseX2;
	BaseY2	= Source.BaseY2;
	tID		= Source.tID;
	Orient 	= Source.Orient;
	EdgeH 	= Source.EdgeH;
	EdgeV 	= Source.EdgeV;
	IsSecBase = Source.IsSecBase;
	IsPlanar= Source.IsPlanar;
	IsNULL	= Source.IsNULL;
	Centroid= Source.Centroid;
	Normal	= Source.Normal;
	TentID	= Source.TentID;
	HSourceL = Source.HSourceL;
	HSourceS = Source.HSourceS;
	LHSourceL = Source.LHSourceL;
	LHSourceS = Source.LHSourceS;
	name 	= Source.name;
	draw	= Source.draw;
	Mother	= Source.Mother;
	PitchO	= Source.PitchO;
	group	= Source.group;
	ugroup	= Source.ugroup;
	if (CopyVertices)
	{
		vcount = Source.vcount;
		Vertices = new vertex[vcount];
		
		for (int v = 0; v<vcount; v++) {
			Vertices[v] = Source.Vertices[v];
		}
		// copy Edge Information too
		for (int i = 0; i<6; i++) {
			EdgeIDs[i] = Source.EdgeIDs[i];
		}
		EdgeH.CopyCoords(Source.EdgeH);
		EdgeV.CopyCoords(Source.EdgeV);
	}
}

void face::RevOrder(bool C = 0)
{
	if (!C)
	for (int v = 0, l = vcount-1; v < vcount/2; v++) {
		swap (Vertices[v], Vertices[l-v]);
	}
	else 
	for (int v = 0, l = vcountC-1; v < vcountC/2; v++) {
		swap (VerticesC[v], VerticesC[l-v]);
	}
}

void face::AddHeight(float height)
{
	for (int i = 0; i<vcount; i++)
	Vertices[i].z += height;
}

void face::GetCentroid()
{
	face &Face = *this;
    Face.GetNormal();
	
	#if DEBUG > 0
	bool dev = 0;
	if(dev) {cout << " ====================== GET Centroid of Face Normal.... " << Face.Normal<< " ======================" << endl;}
	#endif
   
    if (Face.Normal.z<0.999&&Face.Normal.z>-0.999) // If Face is not already Flat, rotate it first, get 2D-centroid, rotate it back
    {
		face FFace; FFace.CopyFace(Face, 1);
		float yaw = GetVecAlign(FFace.Normal,0); //Yaw
		float pitch = GetVecAlign(FFace.Normal,1); //Pitch
		
		#if DEBUG > 0
		if(dev) {
		cout << "	   Face is not flat" << endl;
		cout << "	   Face Normal ORIGINAL: " << Face.Normal << " Pitch " << GetVecAlign(FFace.Normal,1) << " Yaw " << GetVecAlign(FFace.Normal,0) << endl;
		cout << Face << endl;}
		#endif

		FFace.RotateVertices(0,0,-yaw);
		FFace.GetNormal();
		
		#if DEBUG > 0
		if(dev) {cout << "	   Yaw fixed - New Normal: " << FFace.Normal << " Pitch " << GetVecAlign(FFace.Normal,1) << " Yaw " << GetVecAlign(FFace.Normal,0) << endl;}
		#endif
		
		pitch = GetVecAlign(FFace.Normal,1);
		if(pitch!=0)
		{
			FFace.RotateVertices(0,pitch,0);
			
			#if DEBUG > 0
			if(dev) { cout << "	   Pitch fixed - New Normal: " << FFace.Normal << " Pitch now " << GetVecAlign(FFace.Normal,1) << endl; }
			#endif
		}
		
		FFace.RotateVertices(0,-90,0);
		FFace.GetNormal();
		
		#if DEBUG > 0
		if(dev) { if (FFace.Normal.z<0.999) cout << " !!!!!!!!!!!!  WARNING Face Rotation didn't work!!!!!!!!!!!! Pitch now " << GetVecAlign(FFace.Normal,1) << " yaw now " << GetVecAlign(FFace.Normal,0) << endl; }
		#endif
		
		FFace.Centroid = GetCentroid2024(FFace.Vertices, Face.vcount,0);
		FFace.Centroid.z = FFace.Vertices[0].z;
		
		// Rotate FFAce back to check if transformation was correct
		#if DEBUG > 0
		if(dev) {
			FFace.RotateVertices(0,90,0);
			if(pitch!=0)
			FFace.RotateVertices(0,-pitch,0);
			FFace.RotateVertices(0,0,yaw);
			FFace.GetNormal();
		}
		#endif
		
		#if DEBUG > 0
		if(dev) { cout << "	   Flattened Face " << FFace.Normal << " Centroid: " << FFace.Centroid << endl; }
		#endif
		
		FFace.Centroid.rotate(0,90,0);
		if(pitch!=0)
		FFace.Centroid.rotate(0,-pitch,0);
		FFace.Centroid.rotate(0,0,yaw);
		
		#if DEBUG > 0
		if(dev) { cout << FFace << endl; }
		#endif
		
		Centroid = FFace.Centroid;
	}
	else // Face is already flat
	{
		#if DEBUG > 0
		if(dev) {
		cout << "	   Face is flat" << endl;
		cout << "	   Face Normal ORIGINAL: " << Face.Normal << " Pitch " << GetVecAlign(Face.Normal,1) << " Yaw " << GetVecAlign(Face.Normal,0) << endl;
		cout << Face << endl;}
		#endif
		
		Centroid = GetCentroid2024(Face.Vertices, Face.vcount,1);
		Centroid.z = Face.Vertices[0].z;
	}
	
	#if DEBUG > 0
	if(dev) {
	if ( !IsValid(Centroid.x) || !IsValid(Centroid.y) || !IsValid(Centroid.z) ) { cout << "           CENTROID HAS INVALID COORDINATES!" << Centroid << endl; system("pause"); }
	}
	#endif
	
	if ( !IsValid(Centroid.x) ) Centroid.x=0;
	if ( !IsValid(Centroid.y) ) Centroid.y=0;
	if ( !IsValid(Centroid.z) ) Centroid.z=0;
	
	#if DEBUG > 0
	if(dev) {
	for (int i=0; i<Face.vcount; i++) {
		gvector Vec = GetVector(Face.Centroid, Face.Vertices[i]);
		cout << " Distance between Centroid and vertex " << i << " -> " << GetVecLen(Vec) << endl;
	}
	cout << " --------------------------- Transformed Centroid --------------------------- " << Face.Centroid << endl << endl;
	}
	#endif
}


// the old centroid function works better for an unsorted vertex list
// it is only being used once at face::SortVertices(gvector nVec)
void face::GetCentroidClassic()
{
	face &Face = *this;
    Face.GetNormal();
    
	vGroup Temp(vcount);
	for (int v = 0; v<vcount; v++)
	{
		Temp.Vertices[v] = Vertices[v];
	}
	vGroup Final = GetCentroidV(Temp);
	Centroid = Final.Vertices[0];
}



void face::GetCentroidC()
{
	vGroup Temp(vcountC);
	for (int v = 0; v<vcountC; v++)
	{
		Temp.Vertices[v] = VerticesC[v];
	}
	vGroup Final = GetCentroidV(Temp);
	Centroid 	 = Final.Vertices[0];
}

void face::SetEdges(int V0, int V1, int V2, int V3, int V4, int V5) // Horizontal, Vertical and Hypotenuse Edge
{
	#if DEBUG > 0
	bool dev = 0;
	if (dev)cout << "|||||||||||||| Setting new edges... " << V0 << ", " << V1 << ", " << V2 << ", " << V3 << ", " << V4 << ", " << V5 << endl;
	#endif
	
	EdgeIDs[0] = V0;
	EdgeIDs[1] = V1;
	EdgeIDs[2] = V2;
	EdgeIDs[3] = V3;
	EdgeIDs[4] = V4;
	EdgeIDs[5] = V5;
	
	EdgeH = GetVector( Vertices[V0] , Vertices[V1] );
	EdgeV = GetVector( Vertices[V2] , Vertices[V3] );
	Hypo  = GetVector( Vertices[V4] , Vertices[V5] );
	
	#if DEBUG > 0
	if (dev) cout << "||||||||||||||     Edge Vectors " << EdgeH << EdgeV << Hypo << endl;
	#endif
}

void face::RefreshEdges()
{
	#if DEBUG > 0
	bool dev = 0;
	if (dev)cout << "|||||||||||||| REFRESHING new edges... " << endl;
	#endif
	
	EdgeH = GetVector( Vertices[EdgeIDs[0]] , Vertices[EdgeIDs[1]] );
	EdgeV = GetVector( Vertices[EdgeIDs[2]] , Vertices[EdgeIDs[3]] );
	Hypo  = GetVector( Vertices[EdgeIDs[4]] , Vertices[EdgeIDs[5]] );
	
	#if DEBUG > 0
	if (dev) cout << "||||||||||||||     Edge Vectors " << EdgeH << EdgeV << Hypo << endl;
	#endif
}

bool face::GetVertexOrder()
{
	face &Face = *this;
	
	#if DEBUG > 0
	bool dev = 0;
	if (dev) cout << " Getting Vertex Order of Face with Tex "<<Face.Texture << " Orient " << Face.Orient << " ID " << Face.fID <<" (CW or CCW)..." << endl;
	#endif
	
	gvector Normal = Face.Normal;
	gvector Edge1 = GetVector(Face.Vertices[0],Face.Vertices[1]);
	gvector Edge2 = GetVector(Face.Vertices[1],Face.Vertices[2]);
	
	// Scalar of Face NVector and ZChecker has to be >0 to be able to determine the relative vector YAW. just rotate everything until this applies
	gvector ZChecker(0,0,1);
	
	#if DEBUG > 0
	if (dev) cout << "     Getting Scalar of Checker "<<ZChecker<<" and Face Normal " << Normal << endl;
	#endif
	
	float Scalar = GetDot(ZChecker,Normal);

	#if DEBUG > 0
	if (dev) cout << "       Scalar " << Scalar << " Normal " << Normal << endl;
	#endif
	
	while (Scalar<0.01)
	{
		Normal.rotate(90,0,0);
		Edge1.rotate(90,0,0);
		Edge2.rotate(90,0,0);
		Scalar = GetDot(ZChecker,Normal);
		
		#if DEBUG > 0
		if (dev) cout << "       Scalar " << Scalar << " Normal " << Normal << endl;
		#endif
		
		if (Scalar<0.01)
		{
			Normal.rotate(0,90,0);
			Edge1.rotate(0,90,0);
			Edge2.rotate(0,90,0);
			Scalar = GetDot(ZChecker,Normal);
			
			#if DEBUG > 0
			if (dev) cout << "       Scalar " << Scalar << " Normal " << Normal << endl;
			#endif
			
			if (Scalar>0.01) break;
		}
		else break;
	}
	
	// Check if Edge 2 turns Clockwise
	int Edge1_Yaw = GetVecAlign(Edge1,0);
	gvector Vec_Yaw = Edge2;
	Vec_Yaw.rotate(0,0,-(Edge1_Yaw));
	float Check_Yaw = GetVecAlign(Vec_Yaw,0);
	if (Check_Yaw>=180) Face.Clockwise = 1;
	
	#if DEBUG > 0
	if (dev) cout << "       Edge1_Yaw " << Edge1_Yaw << " Check_Yaw " << Check_Yaw << endl;
	
	if (Face.Clockwise)
	{if (dev) cout << "     Face Clockwise!" << endl << endl;}
	else
	{if (dev) cout << "     Face Counter-Clockwise!" << endl << endl;}
	#endif
	
	return Face.Clockwise;
}





/* ===== FACE FUNCTIONS ===== */

// check if 2 faces are parallel to each other
bool IsFaceParallel(face &F1, face &F2)
{
	// use vertex as container for compare-function
	vertex V1, V2;
	V1.x = F1.Normal.x*10;
	V1.y = F1.Normal.y*10;
	V1.z = F1.Normal.z*10;
	V2.x = -(F2.Normal.x*10);
	V2.y = -(F2.Normal.y*10);
	V2.z = -(F2.Normal.z*10);
	
	if (CompareVerticesR(V1,V2))
		return true;
	else
		return false;
}

// Checks if a Face has a Face Alignment in terms of Editor Face/World Align
void CheckFaceAlign(face &Face)
{
	gvector &Vec = *Face.VecV;
	gvector &Normal = Face.Normal;
	
	float Dot = GetDot (Vec, Normal);
	if (Dot>0.0001||Dot<-0.0001) { Face.HasWorldAlign = 1; }
}

bool DoFacesShareVerts(face &F1, face &F2, int deciplaces)
{
	#if DEBUG > 0
	bool dev = 0;
	if (dev) cout << "    Checking if F1 ("<<F1.Texture<<") and F2 ("<<F2.Texture<<") share vertices..." << endl;
	#endif
	
	for (int v = 0; v<F1.vcount; v++)
	{
		vertex &V = F1.Vertices[v];
		for (int vc = 0; vc<F2.vcount; vc++)
		{
			vertex &VC = F2.Vertices[vc];
			if (CompareVerticesDeci(V,VC,deciplaces))
				return true;
		}
	}
	return false;
}

bool IsVertexOnPlane(gvector &Normal, vertex &V, int deci)
{
	int p = pow(10,deci); // precision
	if (p==0) p=1;
	
	// distance
	double d = GetDistPlaneVertex(Normal,V);
	
	#if DEBUG > 0
	bool dev = 0;
	if(dev) cout << " Distance d " << d << " rounded by deci " << deci << " = " << (floorf(d*p)/p) << endl;
	if(dev) system("pause");
	#endif
	
	int d_rounded = floorf(d*p);
	if ( d_rounded/p==0 ) return true;
	else return false;
}

double GetDistPlaneVertex(gvector &Normal, vertex &V)
{
	vertex Origin(Normal.px, Normal.py, Normal.pz);
	double d = GetAdjaLen(  GetVector( Zero, Origin ), Normal  );
	float x1 = Normal.x;
	float x2 = Normal.y;
	float x3 = Normal.z;
	float p1 = V.x;
	float p2 = V.y;
	float p3 = V.z;
	
	double tempA = (x1*p1)+(x2*p2)+(x3*p3)-d;
	double tempB = sqrt( pow(x1,2) + pow(x2,2) + pow(x3,2) );
	
	double result = tempA / tempB;
	
	#if DEBUG > 0
	bool dev = 0;
	if(dev) cout << " Distance Plane " << Normal << " Plane Pos " <<Origin << " V " << V << " d " << d << " tempA " << tempA << " tempB " << tempB << " result " << result << endl;
	#endif
	
	return result;
}

bool IsVertexOnFace(face &Face, vertex &V, int deci)
{
	int p = pow(10,deci); // precision
	if (p==0) p=1;
	
	// distance
	double d = GetDistFaceVertex(Face,V);
	int d_rounded = round(d);
	float d_multi = d_rounded*p;
	if ( d_multi/p==0 ) return true;
	else return false;
}

double GetDistFaceVertex(face &Face, vertex &V)
{
	// get distance (0,0,0)|<-->|Face
	double d = GetAdjaLen(  GetVector( Zero, Face.Vertices[0] ), Face.Normal  );
	
	#if DEBUG > 0
	bool dev = 0;
	if (dev) cout << "      Getting Distance between Vertex " <<V<< " and Face " << Face.Texture << endl;
	if (dev) cout << "         Getting Dist Face<->(0|0|0) from Zero " << Zero << " and Face-Vertex[0] " << Face.Vertices[0] << " = " << d << endl;
	#endif
	
	float x1 = Face.Normal.x;
	float x2 = Face.Normal.y;
	float x3 = Face.Normal.z;
	float p1 = V.x;
	float p2 = V.y;
	float p3 = V.z;
	
	#if DEBUG > 0
	if (dev) cout << "         Getting Results from Face Normal " << Face.Normal << " and Test-Vertex " << V << endl;
	#endif
	
	double tempA = (x1*p1)+(x2*p2)+(x3*p3)-d;
	double tempB = sqrt( pow(x1,2) + pow(x2,2) + pow(x3,2) );

	#if DEBUG > 0
	if (dev) cout << "         A " << tempA << endl;
	if (dev) cout << "         B " << tempB << endl;
	#endif
	
	double result = tempA / tempB;

	#if DEBUG > 0
	if (dev) cout << "         result " << result << endl;
	#endif
	
	return result;
}

// Get Base Edge
void GetBaseEdges(face &Face)
{
	#if DEBUG > 0
	bool dev = 0;
	if (dev) cout << "Getting Base Edges of Face with Tex " << Face.Texture << endl;
	#endif
	
	// this function will determine the base edge (bottom edge) for the 2 Texture Vectors
	// by checking the angles between all vertices and the relevant T-Vector. the smallest Angle will lead to the Face vertex, that acts as the base edge
	gvector *TVec = nullptr;
	int *EdgeID = nullptr;
	int *EdgeID2 = nullptr;
	vector<int> *BaseList = nullptr;
	for (int vec = 0; vec<2; vec++)
	{
		vector<float> baseList_SCAL;
		#if DEBUG > 0
		if (dev&&vec==0) cout << "  Getting Base Edge of Horizontal Texture Vector" << Face.VecX << endl;
		if (dev&&vec==1) cout << "  Getting Base Edge of Vertical Texture Vector" << Face.VecY << endl;
		#endif
		
		if 	(vec==0) 	TVec = &Face.VecX;
		else 			TVec = &Face.VecY;
		if (vec==0)		{EdgeID = &Face.BaseX; EdgeID2 = &Face.BaseX2; BaseList = &Face.BaseListX;}
		else 			{EdgeID = &Face.BaseY; EdgeID2 = &Face.BaseY2; BaseList = &Face.BaseListY;}
		
		vertex &cen = Face.Centroid;
		
		#if DEBUG > 0
		if (dev) cout << "   Current Centroid is " << Face.Centroid << endl;
		#endif
		
		float sList[Face.vcount]; // scalar list
		int s = 0, b = 0, s2 = 0, b2 = 0;
		
		// create vectors from vertices and centroid of current Face and then scalar products of these vectors and the texture vector to find the base edge (rather base "vertex")
		for (int v = 0; v<Face.vcount; v++)
		{
			// Get Dot Product
			gvector VecC = GetVector(cen, Face.Vertices[v]);
			sList[v] = GetDot(*TVec, VecC);
			
			#if DEBUG > 0
			if (dev) cout << "     Current Vertex " << v << Face.Vertices[v] << " Scalar: " << sList[v] << endl;
			#endif
			
			// Get Adjacent Length
			if (sList[v]>sList[s]) {s = v;}
			if (sList[v]<sList[b]) {b = v;}
			
		}
		if (vec==0)
		{
			if (s==0) s2 = 1; else if (s==1) s2 = 0; else if (s==2) s2 = 3; else if (s==3) s2 = 2;
			if (b==0) b2 = 1; else if (b==1) b2 = 0; else if (b==2) b2 = 3; else if (b==3) b2 = 2;
		}
		else
		{
			if (s==0) s2 = 3; else if (s==1) s2 = 2; else if (s==2) s2 = 1; else if (s==3) s2 = 0;
			if (b==0) b2 = 3; else if (b==1) b2 = 2; else if (b==2) b2 = 1; else if (b==3) b2 = 0;
		}
		BaseList->at(0) = s;
		BaseList->at(1) = s2;
		BaseList->at(2) = b2;
		BaseList->at(3) = b;
		
		#if DEBUG > 0
		if (dev) cout << "  Biggest Scalar and therefor Base Edge is vertex " << s << Face.Vertices[s] << endl;
		if (dev) cout << "  Smallest Scalar and therefor Counter-Base Edge is vertex " << b << Face.Vertices[b] << endl;
		#endif
		
		*EdgeID = s;
		*EdgeID2 = b;
		
	}
	
	#if DEBUG > 0
	if (dev)
	{
		cout << "  Final Base Vertex List: " << endl;
		for (int i = 0; i<4; i++)
			cout << "    #" <<i <<" BaseX " << Face.BaseListX[i] << "\t BaseY " << Face.BaseListY[i] << endl;
		cout << endl;
	}
	#endif
}

// Get Base Edge
void GetBaseEdgesC(face &Face)
{
	#if DEBUG > 0
	bool dev = 0;
	if (dev) cout << "Getting Base Edges of Face with Tex " << Face.Texture << endl;
	#endif
	
	// this function will determine the base edge (bottom edge) for the 2 Texture Vectors
	// by checking the angles between all vertices and the relevant T-Vector. the smallest Angle will lead to the Face vertex, that acts as the base edge
	gvector *TVec = nullptr;
	int *EdgeID = nullptr;
	for (int vec = 0; vec<2; vec++)
	{
		#if DEBUG > 0
		if (dev&&vec==0) cout << "  Getting Base Edge of Horizontal Texture Vector" << Face.VecX << endl;
		if (dev&&vec==1) cout << "  Getting Base Edge of Vertical Texture Vector" << Face.VecY << endl;
		#endif
		
		if 	(vec==0) 	TVec = &Face.VecX;
		else 			TVec = &Face.VecY;
		if (vec==0)		EdgeID = &Face.BaseX;
		else 			EdgeID = &Face.BaseY;
		
		vertex &cen = Face.Centroid;
		
		#if DEBUG > 0
		if (dev) cout << "   Current Centroid is " << Face.Centroid << endl;
		#endif
		
		float sList[Face.vcountC];
		int s = 0;
		// create vectors from vertices and centroid of current Face and then scalar products of these vectors and the texture vector to find the base edge (rather base "vertex")
		for (int v = 0; v<Face.vcountC; v++)
		{
			gvector VecC = Normalize(GetVector(cen, Face.VerticesC[v]));
			sList[v] = GetDot(*TVec, VecC);
			
			#if DEBUG > 0
			if (dev) cout << "     Current Vertex " << v << Face.VerticesC[v] << " Scalar: " << sList[v] << endl;
			#endif
			
			if (sList[v]>sList[s]) s = v;
		}
		
		#if DEBUG > 0
		if (dev) cout << "  Biggest Scalar and therefor Base Edge is vertex " << s << Face.VerticesC[s] << endl;
		#endif
		
		*EdgeID = s;
	}
}

// Get Original Texture Offsets from this face
void GetTexOffset(face &Face, int mode = 0)
{
	// mode 0 = both Vectors, mode 1 = X Vector, mode 2 = Y Vector
	#if DEBUG > 0
	bool dev = 0;
	if (dev) cout << "Getting Offset for Face with Tex " << Face.Texture << endl;
	#endif
	
	int texw = gFile->tTable_width[Face.tID];
	int texh = gFile->tTable_height[Face.tID];
	if (texw<16) texw = 16;
	if (texh<16) texh = 16;
	
	#if DEBUG > 0
	if (dev) cout << " Tex Size " << texw << " * " << texh << endl;
	#endif
	
	float *Scale = nullptr;
	float *Shift = nullptr;
	float *Offset = nullptr;
	float *BaseShift = nullptr;
	float tex_size = 0;
	
	#if DEBUG > 0
	if (dev) cout << " Current Face Info | Texture " << Face.Texture << " TexID " << Face.tID << " texw " << texw << " texh " << texh << endl;
	#endif
	
	for (int vec = 0; vec<2; vec++)
	{
		//vertex BaseVertex;
		if (vec==0)
		{
			Shift		= &Face.ShiftX;
			Scale		= &Face.ScaleX;
			Offset 		= &Face.OffsetX;
			BaseShift 	= &Face.BaseShiftX;
			tex_size	= texw;
			
			#if DEBUG > 0
			if (dev) cout << "  Tex size is " << tex_size << "(tex width ["<<texw<<"] XXX ScaleX ["<<Face.ScaleX<<"] )"<< endl;
			#endif
		}
		if (vec==1)
		{
			Shift		= &Face.ShiftY;
			Scale		= &Face.ScaleY;
			Offset 		= &Face.OffsetY;
			BaseShift 	= &Face.BaseShiftY;
			tex_size	= texh;
			
			#if DEBUG > 0
			if (dev) cout << "  Tex size is " << tex_size << "(tex height ["<<texh<<"] XXX ScaleY ["<<Face.ScaleY<<"] )"<< endl;
			#endif
		}
		
		if ((vec==0&&(mode==0||mode==1))||(vec==1&&(mode==0||mode==2)))
		{
			float B = *BaseShift;
			float S = *Shift;
			int i = 0;
			if (B>S) while (B>S)
			{
				B -= tex_size;
				
				#if DEBUG > 0
				if (dev) cout << "    BaseShift bigger than "<<S<<" - now: " << B << endl;
				#endif
				
				if(i>100) break; // emergency 
				i++;
			}
			
			#if DEBUG > 0
			if (dev) cout << "     BaseShift now: " << B << " Shift now: " << S << endl;
			#endif
			
			*Offset = S - B;
		}
	}
	
	#if DEBUG > 0
	if (dev) cout << "  Offset X is " << Face.OffsetX << endl;
	if (dev) cout << "  OFfset Y is " << Face.OffsetY << endl << endl;
	#endif
}


// axis 0 = X+Y Baseshift, axis 1 = X only, axis 2 = Y only
// mode 0 = longest , 1 = shortest
void GetBaseShift(face &Face, int axis = 0, bool longEdge = 1, bool C = 0)
{
	#if DEBUG > 0
	bool dev = 0;
	if (dev) cout << "Getting BaseShift for Face with Tex " << Face.Texture << endl;
	#endif
	
	gvector Vec;
	float *BaseShift = nullptr;
	float *Scale = nullptr;
	
	#if DEBUG > 0
	if (dev&&C) cout << " BaseX1 " << Face.BaseX << " BaseX2 " << Face.BaseX2 << " BaseY1 " << Face.BaseY << " BaseY2 " << Face.BaseY2 << endl;
	if (dev&&!C) cout << " BaseX1 " << Face.BaseListX[0] << " BaseX2 " << Face.BaseListX[3] << " BaseY1 " << Face.BaseListY[0] << " BaseY2 " << Face.BaseListY[3] << endl;
	#endif
	
	for (int vec = 0; vec<2; vec++)
	{
		vertex BaseVertex;
		vertex BaseVertex2;
		if (vec==0)
		{
			Vec = Face.VecX;
			if (longEdge) {
				if (C) 	{BaseVertex = Face.VerticesC[Face.BaseX];}
				else 	{BaseVertex =  Face.Vertices[Face.BaseListX[0]];  BaseVertex2 = Face.Vertices[Face.BaseListX[3]];}
			} else {
						{BaseVertex =  Face.Vertices[Face.BaseListX[1]];  BaseVertex2 = Face.Vertices[Face.BaseListX[2]];}
			}
			Scale = &Face.ScaleX;
			BaseShift = &Face.BaseShiftX;
			
			#if DEBUG > 0
			if (dev) cout << "  Base-Vertex is " << BaseVertex << " Tex Vec is " << Vec <<  endl;
			#endif
		}
		if (vec==1)
		{
			Vec = Face.VecY;
			if (longEdge)
			{
				if (C) 	{BaseVertex = Face.VerticesC[Face.BaseY];}
				else 	{BaseVertex =  Face.Vertices[Face.BaseListY[0]];  BaseVertex2 = Face.Vertices[Face.BaseListY[3]];}
			}
			else
			{
						{BaseVertex =  Face.Vertices[Face.BaseListY[1]];  BaseVertex2 = Face.Vertices[Face.BaseListY[2]];}
			}
			Scale = &Face.ScaleY;
			BaseShift = &Face.BaseShiftY;
			
			#if DEBUG > 0
			if (dev) cout << "  Base-Vertex is " << BaseVertex << " Tex Vec is " << Vec <<  endl;
			#endif
		}
				
		if ((vec==0&&(axis==0||axis==1))||(vec==1&&(axis==0||axis==2)))
		{
			// check vec pitch (specifies wether BaseShift is positive or negative)
			vertex BVertex;
			if (Vec.IsHor)
			{
				if (Vec.IsNeg) BVertex = BaseVertex; else BVertex = BaseVertex2;
			}
			else BVertex = BaseVertex; // ????????
			
			gvector Hypo = GetVector(Zero, BVertex);
			
			#if DEBUG > 0
			if (dev) cout << "  Vector VecBN of Base-Vertex and NULL: " << Hypo <<  endl;
			#endif
			
			float Hypo_Len = GetVecLen(Hypo);
			
			#if DEBUG > 0
			if (dev) cout << "  Length: " << Hypo_Len <<  endl;
			#endif
			
			float alpha = GetAngleCos(Hypo, Vec);
			float alpha_deg = acos(alpha)*180/PI;
			
			#if DEBUG > 0
			if (dev) cout << "  Angle between VecBN and VecTV: " << alpha_deg <<  endl;
			#endif
			
			float m = 1.0 / *Scale; // Texture Scale Modifier, because Texture Shift depends on Texture Scale
			*BaseShift = -((alpha * Hypo_Len)*m);
			if (!IsValid(*BaseShift)) *BaseShift = 0;
		}
	}
	
	#if DEBUG > 0
	if (dev) cout << "  BaseShift X is " << Face.BaseShiftX << endl;
	if (dev) cout << "  BaseShift Y is " << Face.BaseShiftY << endl << endl;
	#endif
}

void GetBaseShiftCustom(face &Face, int axis, vertex Custom)
{
	gvector Vec;
	float *BaseShift = nullptr;
	float *Scale = nullptr;
	for (int vec = 0; vec<2; vec++)
	{
		vertex BaseVertex;
		vertex BaseVertex2;
		if (vec==0)
		{
			Vec = Face.VecX;
			BaseVertex =  Custom;
			Scale = &Face.ScaleX;
			BaseShift = &Face.BaseShiftX;
		}
		if (vec==1)
		{
			Vec = Face.VecY;
			BaseVertex =  Custom;
			Scale = &Face.ScaleY;
			BaseShift = &Face.BaseShiftY;
		}
				
		if ((vec==0&&(axis==0||axis==1))||(vec==1&&(axis==0||axis==2)))
		{
			// check vec pitch (specifies wether BaseShift is positive or negative)
			vertex BVertex = BaseVertex;
			gvector Hypo = GetVector(Zero, BVertex);
			float Hypo_Len = GetVecLen(Hypo);
			float alpha = GetAngleCos(Hypo, Vec);
			float alpha_deg = acos(alpha)*180/PI;
			
			float m = 1.0 / *Scale; // Texture Scale Modifier, because Texture Shift depends on Texture Scale
			*BaseShift = -((alpha * Hypo_Len)*m);
			if (!IsValid(*BaseShift)) *BaseShift = 666.666;
		}
	}
}

// Checks if the Face Alignment of this Face is valid in Terms of "usable by Map2Curve"
bool IsFaceAlignValid(face &Face)
{
	// First check which one is the vertical vector
	bool VvecFound = 0;
	
	// Vertical Vector has the following coordinate restrictions: X has to be always 0
	gvector *Vec1 = nullptr;
	gvector *Vec2 = nullptr;
	for (int i = 0; i<2; i++)
	{
		if (i==0) 	{ Vec1 = &Face.VecX; Vec2 = &Face.VecY; }
		else 		{ Vec1 = &Face.VecY; Vec2 = &Face.VecX; }
		
		if 	(Vec1->x==0||(Vec1->x<0.01&&Vec1->x>-0.01)) { VvecFound = 1; Face.VecV=Vec1; Face.VecH=Vec2; Face.VecH->IsHor=1; Face.VecV->IsHor=0;
		}
	}
	
	// check if other Vector is valid horizontal vector
	// Horizontal Vector has the following coordinate restrictions: Z has to be always 0
	gvector *Hvec = Face.VecH;
	if (VvecFound)
	{
		if (Hvec->z==0 || (Hvec->z<0.01&&Hvec->z>-0.01))
		{
			return true;
		}
		else
		{
			return false;
		}
	}
	else
	{
		return false;
	}
}

axis GetWorldAlign(face &Face) {
	
	gvector Vec1, Vec2, NVec, BaseVec(0,0,64);
	
	// Check Pitch Angle
	Vec1 = GetVector(Face.Vertices[0], Face.Vertices[1]);
	Vec2 = GetVector(Face.Vertices[0], Face.Vertices[2]);
	NVec = GetCross(Vec1, Vec2);
	
	float PitchAngle = GetVecAng(BaseVec, NVec);
	
	// Align to Z-Axis
	if (PitchAngle <= 45 || PitchAngle >= 135)
	{
		return Z;
	}
	else
	{
		BaseVec.x = 64;
		BaseVec.y = 0;
		BaseVec.z = 0;
		NVec.z = 0;
		
		// Check Yaw Angle
		float YawAngle = GetVecAng(BaseVec, NVec);
		
		// Align to X or Y-Axis
		if (YawAngle > 45 && YawAngle < 135)
		{
			return X;
			
		} else if (YawAngle <= 45 || YawAngle >= 135) {

			return Y;
		}
	}
}

void AlignToWorld(face &Face)
{
	axis WAlign = GetWorldAlign(Face);
	
	Face.VecX.set(0);
	Face.VecY.set(0);
	Face.Rot = 0;
	
	// Align to X or Y-Axis
	if (WAlign == Z)
	{
		Face.VecX.x = 1;
		Face.VecY.y = -1;
		//Face.WAlign = 0; // Top-Down World Align
		
	} else if (WAlign == X)
	{
		Face.VecX.x = 1;
		Face.VecY.z = -1;
		//Face.WAlign = 1; // Front/Back World Align
		
	} else if (WAlign == Y)
	{
		Face.VecX.y = 1;
		Face.VecY.z = -1;
		//Face.WAlign = 2; // Side (L/R) World Align
	}
	Face.FaceAlign = 0;
}


float GetFaceLen(face &Face)
{
	vertex p0, p1, p2;
	gvector Vec1, Vec2;
	float AlphaCos = 0;
	
	if (Face.IsWedgeDown)
	{
		p0 = Face.Vertices[0];
		p1 = Face.Vertices[1];
		p2 = Face.Vertices[2];
	}
	else
	{
		p0 = Face.Vertices[1];
		p1 = Face.Vertices[0];
		p2 = Face.Vertices[2];
	}
	Vec1 = GetVector(p0, p1);
	Vec2 = GetVector(p0, p2);
	
	AlphaCos = sin(acos(GetAngleCos(Vec2, Vec1)));
	
	float result = AlphaCos * GetVecLen(Vec1);
	if (!IsValid(result)) result = 0;
	
	return result;
}


void WriteFaceMAP(ostream &ostr, face &Face)
{
	ostr << setprecision(8) << fixed;
	ostr << Face.Vertices[0] << Face.Vertices[1] << Face.Vertices[2];
	ostr << setprecision(8) << fixed;
	
	ostr << " " << Face.Texture;
	ostr << " [ ";
	ostr << Face.VecX.x << " ";
	ostr << Face.VecX.y << " ";
	ostr << Face.VecX.z << " ";
	ostr << Face.ShiftX << " ";
	ostr << "] [ ";
	ostr << Face.VecY.x << " ";
	ostr << Face.VecY.y << " ";
	ostr << Face.VecY.z << " ";
	ostr << Face.ShiftY << " ";
	ostr << "] ";
	ostr << Face.Rot << " ";
	ostr << Face.ScaleX << " ";
	ostr << Face.ScaleY << " ";
	ostr << endl;
	
	ostr << setprecision(0) << fixed;
}


ostream &operator<<(ostream &ostr, face &Face)
{
	ostr << endl << " ### Printing Face ###" << endl;
	ostr << "  Vertices: " << Face.vcount << endl;
	for(int v=0; v<Face.vcount; v++) {
		ostr << "   #" << v << " " << Face.Vertices[v] << endl;
	}
	ostr << "  Normal " << Face.Normal << endl;
	ostr << "  Texture " << Face.Texture << endl;
	ostr << "  VecX " << Face.VecX << endl;
	ostr << "  VecY " << Face.VecY << endl;
	ostr << "  ShiftX " << Face.ShiftX << endl;
	ostr << "  ShiftY " << Face.ShiftY << endl;
	ostr << "  ScaleX " << Face.ScaleX << endl;
	ostr << "  ScaleY " << Face.ScaleY << endl;
	ostr << endl;
	
	return ostr;
}

bool DoFacesShareVertices(face &F1, face &F2)
{
	for(int v1=0; v1<F1.vcount; v1++)
	{
		for(int v2=0; v2<F2.vcount; v2++)
		{
			if( CompareVerticesR(F1.Vertices[v1], F2.Vertices[v2]) )
			{
				return 1;
			}
		}
	}
	return 0;
}





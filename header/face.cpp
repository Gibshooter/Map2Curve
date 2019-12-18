#include "face.h"
#include "vertex.h"
#include "utils.h"
#include "settings.h"
#include "file.h"
#include "group.h"

#include <math.h>
#include <iostream>
#include <iomanip> // precision

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

void face::MiniShift()
{
	bool dev = 0;
	if(dev) cout << " Minishift()..." << endl;
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
			if (dev) cout << " Shift " << *Shift << " TSize " << TSize << " m " << m;
			if (*Shift < 0) *Shift += TSize*m;
			if (*Shift >= TSize) *Shift -= TSize*m;
			if (dev) cout << " Shift New " << *Shift << endl;
			
			/*while (*Shift < 0)      { if(dev) cout << " TSize "<< TSize <<" Mod " <<  <<" Shift<0: " << *Shift << endl; *Shift += TSize; }
			while (*Shift >= TSize) { if(dev) cout << " TSize "<< TSize <<" Shift>=0: " << *Shift << endl; *Shift -= TSize;
			if (*Shift==0) break; } */
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
	bool dev = 0;
	face &Face = *this;
	if(dev) cout << " Checking Planarity of Face Tex " << Face.Texture << " vcount " << Face.vcount << endl;
	int vc = Face.vcount;
	gvector Edges[vc];
	gvector Normals[vc];
	float Scalar[vc];
	// create NormalVectors from all Face-Edges
	for (int i = 0; i<vc; i++) {
		if (i==vc-1)
		Edges[i] = GetVector(Face.Vertices[i],Face.Vertices[0]);
		else
		Edges[i] = GetVector(Face.Vertices[i],Face.Vertices[i+1]);
		if (dev) cout << "  Edge " << i << " " << Edges[i] << " Vertices " << Face.Vertices[i] << Face.Vertices[i+1] << endl;
	}
	for (int i = 0; i<vc; i++) {
		if (i==vc-1)
		Normals[i] = Normalize(GetCross(Edges[i],Edges[0]));
		else
		Normals[i] = Normalize(GetCross(Edges[i],Edges[i+1]));
		if(dev) cout << "  Normal " << i << Normals[i] << endl;
	}
	//get scalars of all vectors and vector 0
	for (int i = 0; i<vc-1; i++) {
		Scalar[i] = GetDot(Normals[i+1], Normals[0]);
		Scalar[i] = ceilf(Scalar[i] * 100) / 100;
		if(dev) cout << "  Scalar " << i << " " << Scalar[i] << " from Normal " << Normals[i+1] << " and " << Normals[0] << endl;
	}
	//compare all normals with first one 
	for (int i = 1; i<vc-1; i++) {
		if (Scalar[i]!=Scalar[0])
		{
			if(dev) cout << "   Scalar " << i << " (" << Scalar[i] << ") differs from Scalar 0 " << Scalar[0] << " Face is COplanar!" << endl;
			Face.IsPlanar = 0;
			return false;
		} else {
			if(dev) cout << "   Scalar " << i << " (" << Scalar[i] << ") is equal to Scalar 0 " << Scalar[0] << " Face is planar so far!" << endl;
		}
	}
	return true;
}

float face::GetLenHor(bool UseEdge)
{
	face &Face = *this;
	gvector Vec;
	Face.LenHO = Face.LenH;
	
	if (!UseEdge)	Vec = *Face.VecH;
	else			Vec = Face.EdgeH;
	
	Face.LenH = GetAdjaLen(Face.Hypo, Vec);
	
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
	bool dev = 0;
	face &Face = *this;
	if (Face.IsNULL)
	{
		if (dev) cout << "   Face Normal " << Base.Normal << " Centroid " << Base.Centroid << " IsNULL " << Face.IsNULL << " Tent ID " << Face.TentID << " new Tent ";
		vertex &Tent = Face.Vertices[Face.TentID];
		Tent = Base.Centroid;
		gvector Normal = Base.Normal.flip();
		Normal.mult(def_spikesize);
		Tent.Add(Normal);
		
		if (dev) cout << Tent << endl;
	}
}

void face::ConvertToSheared(bool IsLongEdge, bool IsInside, bool Reverse, brush &Brush)
{
	bool dev = 0;
	face &Face = *this;
	
	// Determine whether this wedge is the long or the short one in the case of a reversed spline extrusion
	if(Reverse) { if(IsLongEdge) IsLongEdge=0; else IsLongEdge=1; }
	GetBaseEdges(Face);
	GetBaseShift(Face,0,1,0);
	Face.OffsetX = Face.HSourceL->OffsetX;
	Face.RefreshEdges();
	
	if(dev) {cout << " Shearing Face " << Face.name << endl << " Tex " << Face.Texture << endl << " Orient " << Face.Orient << endl << " Offset ";
	if (Face.VecX.IsHor) cout << OffsetX << endl; else cout << OffsetY << endl;
	cout << " Shift "; 		if (Face.VecX.IsHor) cout << ShiftX << endl; else cout << ShiftY << endl;
	cout << " BaseShift ";	if (Face.VecX.IsHor) cout << BaseShiftX << endl; else cout << BaseShiftY << endl;
	if (IsLongEdge) cout << " LONG Wedge" << endl; else cout <<" SHORT Wedge" << endl;}
	
	//Face.Texture = Face.name;
	
	if(Face.Orient==2||Face.Orient==3)
	{
		// get new Face Vectors
		gvector VecH_Old = *Face.VecH;
		Face.GetNormal();
		gvector Cross = Normalize( GetCross(Face.Normal, Face.EdgeV) );
		Face.VecH->CopyCoords(Cross);
		if(GetDot(*Face.VecH, VecH_Old)<0) Face.VecH->flip();
		
		// get old Hor Lengths
		float HLenO = Face.HSourceL->EdgeLenL;
		//if(IsInside) HLenO = Face.HSourceS->EdgeLenS;
		
		// get new lengths, texture, scale and shift
		float HLenN=0;
		//if(IsInside)
		//	if(IsLongEdge) HLenN = GetAdjaLen(Brush.HSourceL->EdgeLenL, GetVecAng(Brush.HSourceS->EdgeH, *Face.VecH)); else HLenN = Face.GetLenHor(0); if(HLenN<0) HLenN=-HLenN;
		//else
		//	if(IsLongEdge) HLenN = Face.GetLenHor(0); else { HLenN = GetAdjaLen(Brush.HSourceS->EdgeLenS, GetVecAng(Brush.HSourceS->EdgeH, *Face.VecH)); if(HLenN<0) HLenN=-HLenN; } // necessary for spline extrusion ???
		HLenN = Face.GetLenHor(0); if(HLenN<0) HLenN=-HLenN;
		
		float m = HLenO/HLenN;
		if(Face.VecX.IsHor) {
			Face.ScaleX /= m; } else {
			Face.ScaleY /= m; }
		GetBaseShift(Face,0,1,0);
		if(Face.VecX.IsHor) {
			Face.ShiftX = Face.BaseShiftX + Face.OffsetX;
		} else {
			Face.ShiftY = Face.BaseShiftY + Face.OffsetY;
		}
		
		if(dev){
			cout << " Hypo " << Hypo << " (Len " << GetVecLen(Hypo) << ")" << endl;
			cout << " VecH " << *VecH << endl;
			cout << " VecH_OLD " << VecH_Old << endl;
			cout << " EdgeH " << EdgeH << " (Len " << GetVecLen(EdgeH) << ")" << endl;
			cout << " HSourceL " << Face.HSourceL->EdgeH << " name " << Face.HSourceL->name <<" (Len " << Face.HSourceL->EdgeLenL << ")" << endl;
			cout << " HSourceS " << Face.HSourceS->EdgeH << " name " << Face.HSourceS->name <<" (Len " << Face.HSourceS->EdgeLenS << ")" << endl;
			cout << " HLenO " << HLenO << "(made of Face.HSourceL->EdgeLenL ("<<Face.HSourceL->EdgeLenL<<"))" << endl;
			cout << " HLenN " << HLenN; if(IsLongEdge) cout << "(Face.GetLenHor(0))" << endl; else cout << "(made of Brush.HSourceS->EdgeLenS("<<Brush.HSourceS->EdgeLenS<<"))" << endl;
			cout << " m " << m << endl;
			cout << " Scale "; if(Face.VecX.IsHor) cout << ScaleX<<endl; else cout << ScaleY << endl;
			cout << " BaseShift "; if(Face.VecX.IsHor) cout << BaseShiftX<<endl; else cout << BaseShiftY << endl;
			cout << " Shift "; if(Face.VecX.IsHor) cout << ShiftX<<endl; else cout << ShiftY << endl;
			cout << " Hor Vec? "; if(Face.VecX.IsHor) cout <<"X"<<endl; else cout << "Y" << endl;
		}
	}
	else
	{
		// get old Hor Face Lengths
		float HLenO = Face.HSourceL->EdgeLenL;
		if(IsInside) HLenO = Face.HSourceS->EdgeLenS;
		float HLenN = Face.GetLenHor(0);
		float m = HLenO / HLenN; if(m<0) m=-m;
		if(dev) cout << " HLenO " << HLenO << " HLenN " << HLenN << " m " << m << endl;
		
		//if(IsInside) Face.Texture += "_IN"; else Face.Texture += "_OUT";
		//if(IsLongEdge) Face.Texture += "_L"; else Face.Texture += "_S";
		// get new texture scale and shift
		if(m!=1)
		{
			if(Face.VecX.IsHor) {
				Face.ScaleX /= m; } else {
				Face.ScaleY /= m;
			}
			GetBaseShift(Face,0,1,0);
			if(!IsLongEdge) {
				if(Face.VecX.IsHor)
				Face.ShiftX = Face.BaseShiftX + Face.OffsetX;
				else
				Face.ShiftY = Face.BaseShiftY + Face.OffsetY;
			}
			/*if(IsInside&&Face.Orient==5&&IsLongEdge)
			{
				//Face.Texture = "RED";
				if(Face.VecX.IsHor)
				Face.ShiftX = Face.BaseShiftX + Face.OffsetX;
				else
				Face.ShiftY = Face.BaseShiftY + Face.OffsetY;
			}*/
			/*if(Face.VecX.IsHor) {
				Face.ShiftX = Face.BaseShiftX + (Face.OffsetX / m); } else {
				Face.ShiftY = Face.BaseShiftY + (Face.OffsetY / m); }*/
			/*if(Face.VecX.IsHor) {
				Face.ShiftX *= m; } else {
				Face.ShiftY *= m; }*/
		}
	}
	if(dev) getch();
	if(dev) cout << endl;
}

void face::CreateRamp(int g, int b, int f, int SecID, bool IsWedge2, float Bstep)
{
	bool dev = 0;
	bool Tri = cTable[g].tri;
	face &Face = *this;
	if(dev) cout << " Creating Ramp from Face Tex " << Face.Texture << " Orient " << Face.Orient << " IsNull " << Face.IsNULL << " Wedge " << IsWedge2 << endl;
	int sec = SecID;
	if (Face.Texture=="NULL"||Face.Texture=="SOLIDHINT"||Face.Texture==def_nulltex) Face.IsNULL=1;
	
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
	if (dev) cout << " Height Tables..." << endl;
	float step = 0;
	float height = 0;

	step = cTable[g].height;
	height = cTable[g].height * sec;
	int start = bGroup[g].range_start;
	if (cTable[g].heightmode>0) {
		step = bGroup[g].heightTableSteps[sec];
		
		if (dev) cout << " step " << step << " start " << start << " sec " << sec << " height table " << bGroup[g].heightTable[sec-start] << " last " << bGroup[g].heightTable[sec-start-1] << endl;
	}
	
	if (step!=0&&Face.draw)
	{
		gvector EdgeH;
		gvector EdgeV;
		gvector Hypo;
		
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
			//dev = 0;
			//if (sec==0) dev=1;
			if(dev)
			{
				cout << " Brush " <<b<<" Face "<<f<<" Sec " << sec << " Tex "<<Face.Texture << " Wedge " << IsWedge2+1;
				if (Face.Orient==2) {cout << " Orient UP"; }
				if (Face.Orient==3) {cout << " Orient DN"; }
				if (Face.Orient==4) {cout << " Orient FT"; }
				if (Face.Orient==5) {cout << " Orient BK"; }
				if (Face.Orient==6) {cout << " Orient NULL"; }
				cout << endl;
			}
			/*=========================================================================++
			|| Get previously calculated Offsets, Centroid, BaseEdges and BaseShift
			++=========================================================================++*/
			if (dev) cout << " Get previously calculated Offsets, Centroid, BaseEdges and BaseShift..." << endl;
			Face.GetCentroid();
			if(dev)cout << "   Tex Offsets PRE X " << Face.OffsetX << " Y " << Face.OffsetY << " BaseVertexX " << Face.BaseX << " BaseVertexY " << Face.BaseY << " VecX "<<Face.VecX<<" VecY "<<Face.VecY<<" Centroid " << Face.Centroid << endl;
			GetBaseEdges(Face);
			GetBaseShift(Face,0,1,0);
			GetTexOffset(Face, 0);
			if(dev)cout << "   Tex Offsets AFT X " << Face.OffsetX << " Y " << Face.OffsetY << " BaseVertexX " << Face.BaseX << " BaseVertexY " << Face.BaseY << endl;
			
			/*=========================================================================++
			|| Get Original Face Lengths first
			++=========================================================================++*/
			if (dev) cout << " Get Original Face Lengths first..." << endl;
			
			if (cTable[g].ramptex==0)
			{
				if (Face.Orient!=4&&Face.Orient!=5)
				HLengthO = Face.GetLenHor(0);
				VLengthO = Face.GetLenVer(0);
				Face.LengthO = HLengthO;
			}
			else if (cTable[g].ramptex==1)
			{
				if (Face.Orient!=4&&Face.Orient!=5)
				HLengthO = Face.GetLenHor(1);
				VLengthO = Face.GetLenVer(1);
				Face.LengthO = HLengthO;
			}
			if(dev)cout << "   OLD Lengths: H " << HLengthO << " V " << VLengthO << endl;
		}
		
		if (Face.fID==2)
		{
			/*=========================================================================++
			|| Add Height
			++=========================================================================++*/
			for (int v = 0; v<Face.vcount; v++)
			{
				vertex &V = Face.Vertices[v];
					if(dev)cout << "   Adding Height "<<step<<" to Vertex " << v << " Z (" << V.z << ") = ";
				if (V.DoAddHeight) {
					V.z+=step;
				}
					if(dev)cout << V.z << endl;
			}
			
			// Fix "Tent" Vertices if Spike Brushes (created when triangulating complex brushes)
			for (int v = 0; v<Face.vcount; v++)
			{
				vertex &V = Face.Vertices[v];
				//if (V.IsTent) {
				//	V.MakeTent;
				//}
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
			//cout << " fix texture vector align in case of Ramp..." << endl; getch();
			if (cTable[g].ramptex==0)
			{
				if (Face.Orient==2)
				{
					// new Texture Vectors for sloped top/down faces
					if (!Tri) {
						EdgeH = Face.EdgeH; // GetVector(V0, V3);
						EdgeV = Face.EdgeV; // GetVector(V0, V1);
						EdgeH.flip();
						EdgeV.flip();
					} else {
						if (!IsWedge2) {
							EdgeH = Face.EdgeH; //GetVector(V0, V2);
							EdgeV = Face.EdgeV; //GetVector(V0, V1);
							EdgeH.flip();
							EdgeV.flip();
						} else {
							EdgeH = Face.EdgeH; // GetVector(V1, V0);
							EdgeV = Face.EdgeV; // GetVector(V1, V2);
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
				if (Face.Orient==3)
				{
					if (!Tri) {
						EdgeH = Face.EdgeH; // GetVector(V1, V2);
						EdgeV = Face.EdgeV; // GetVector(V1, V0);
					} else {
						if (!IsWedge2) {
							EdgeH = Face.EdgeH; // GetVector(V1, V2);
							EdgeV = Face.EdgeV; // GetVector(V1, V0);
						} else {
							EdgeH = Face.EdgeH; // GetVector(V1, V2);
							EdgeV = Face.EdgeV; // GetVector(V1, V0);
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
					gvector EdgeH = Face.EdgeH; // GetVector(V1, V2);
					gvector EdgeV = Face.EdgeV; // GetVector(V0, V1);
					gvector Cross =  GetCross (Face.Normal, EdgeV);
					gvector Cross2 = GetCross (Face.Normal, EdgeH);
					//if(dev) cout << "   Fixing TVec of brush " << b << " Orient " << Face.Orient << " VecV " << *Face.VecV << " EdgeH " << EdgeH << " EdgeHLen " << GetVecLen(EdgeH) <<" Normal " << Face.Normal <<endl;
					gvector Hor = Normalize(Cross);
					gvector Ver = Normalize(Cross2);
					DotH = GetDot (Hor, *Face.VecH); if (DotH<0) { Hor.flip(); }
					DotV = GetDot (Ver, *Face.VecV); if (DotV<0) { Ver.flip(); }
					//if (Face.Orient==4) Ver.flip();
					//if (Face.VecH->IsNeg) { Hor.flip(); }
					//if (Face.VecV->IsNeg) { Ver.flip(); }
					Face.VecH->CopyCoords( Hor );
					Face.VecV->CopyCoords( Ver );
					//cout << "    New VecH now " << *Face.VecH << " VecV now " << *Face.VecV << endl;
				}
			}
			else if (cTable[g].ramptex == 1)
			{
				if (Face.Orient==2)
				{
					// new Texture Vectors for sloped top/down faces
					if (!Tri) {
						EdgeH = GetVector(V0, V3);
						EdgeV = GetVector(V0, V1);
						EdgeH.flip();
						EdgeV.flip();
					} else {
						if (!IsWedge2) {
							EdgeH = GetVector(V0, V2);
							EdgeV = GetVector(V0, V1);
							EdgeH.flip();
							EdgeV.flip();
						} else {
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
					//if (Face.VecH->IsNeg) { Hor.flip(); }
					//if (Face.VecV->IsNeg) { Ver.flip(); }
					Face.VecH->CopyCoords( Hor );
					Face.VecV->CopyCoords( Ver );
				}
				if (Face.Orient==3)
				{
					if (!Tri) {
						EdgeH = GetVector(V1, V2);
						EdgeV = GetVector(V1, V0);
					} else {
						if (!IsWedge2) {
							EdgeH = GetVector(V1, V2);
							EdgeV = GetVector(V1, V0);
						} else {
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
					//if (Face.VecH->IsNeg) { Hor.flip(); }
					//if (Face.VecV->IsNeg) { Ver.flip(); }
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
					//if(dev) cout << "   Fixing TVec of brush " << b << " Orient " << Face.Orient << " VecV " << *Face.VecV << " EdgeH " << EdgeH << " EdgeHLen " << GetVecLen(EdgeH) <<" Normal " << Face.Normal <<endl;
					//gvector Hor = Normalize(EdgeH);
					gvector Hor = Normalize(EdgeH);
					gvector Ver = Normalize(Cross);
					Hor.flip();
					if (Face.Orient==4) Ver.flip();
					if (Face.VecH->IsNeg) { Hor.flip(); }
					if (Face.VecV->IsNeg) { Ver.flip(); }
					Face.VecH->CopyCoords( Hor );
					Face.VecV->CopyCoords( Ver );
					//cout << "    New VecH now " << *Face.VecH << " VecV now " << *Face.VecV << endl;
				}
			}
			//if(dev)cout << "   New Tex Vectors: VecX " << Face.VecX << " VecY " << Face.VecY << endl;
			
			/*=========================================================================++
			|| Get new Face Lengths
			++=========================================================================++*/
			//cout << " Get new Face Lengths..." << endl; getch();
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
			if(dev)cout << "   NEW Lengths: H " << HLengthN << " V " << VLengthN << endl;

			/*=========================================================================++
			|| Re-compose textures of Body Faces
			++=========================================================================++*/
			//cout << " Re-compose textures of Body Faces..." << endl; getch();
			float w2mh; if (IsWedge2&&cTable[g].ramptex==1) w2mh = bGroup[g].Brushes[b-1].Faces[f].LengthO/bGroup[g].Brushes[b-1].Faces[f].LengthN;
			if(dev)cout << "   w2mh " << w2mh <<  endl;
			
			float mh = HLengthO/HLengthN;
			float mv = VLengthO/VLengthN;
			float mx, my;
			
			if (Face.VecX.IsHor) 	{mx = mh; my = mv;}
			else 					{mx = mv; my = mh;}
			if(dev)cout << "   mx " << mx << " my " << my << endl;
			
			if(dev)cout << "   ScaleX (" << Face.ScaleX << ") /= mx (" << mx << ") -> " << Face.ScaleX / mx << endl;
			if(dev)cout << "   ScaleY (" << Face.ScaleY << ") /= my (" << my << ") -> " << Face.ScaleY / my << endl;
			
			// Fix Scales
			Face.ScaleX /= mx;
			Face.ScaleY /= my;
			if (IsWedge2&&(Face.Orient==2||Face.Orient==3)&&cTable[g].ramptex==1)
			{
				if (Face.VecX.IsHor) 	{ Face.ScaleX *= w2mh; }
				else 					{ Face.ScaleY *= w2mh; }
			}
			if(dev)cout << "   Face Tex BaseShifts PRE X " << Face.BaseShiftX << " Y " << Face.BaseShiftY << endl;
			
			GetBaseShift(Face,0,1,0);
			if(dev)cout << "   Face Tex BaseShifts AFT X [" << Face.BaseShiftX << "] IsHor? (" << Face.VecX.IsHor << ") Y [" << Face.BaseShiftY << "] IsHor? (" << Face.VecY.IsHor << ")" <<endl;
			
			/*--------------------------------------------------+
			| Compose Face Shift
			+---------------------------------------------------+*/
			//cout << " Compose Face Shift..." << endl; getch();
			if(dev)cout << "   ShiftX (" << Face.ShiftX << ") = BaseShiftX " << Face.BaseShiftX << " + OffsetX (" << Face.OffsetX << ") = " << Face.BaseShiftX + Face.OffsetX << endl;
			if(dev)cout << "   ShiftY (" << Face.ShiftY << ") = BaseShiftY " << Face.BaseShiftY << " + OffsetY (" << Face.OffsetY << ") = " << Face.BaseShiftY + Face.OffsetY  << endl;

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
	if(dev)cout << endl;
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
	if (Source.VecY.IsHor) {
	VecH = &VecY;
	VecV = &VecX; }
	//VecH	= Source.VecH;
	//VecV	= Source.VecV;
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
	name 	= Source.name;
	if (CopyVertices)
	{
		vcount = Source.vcount;
		Vertices = new vertex[vcount];
		for (int v = 0; v<vcount; v++) {
			Vertices[v] = Source.Vertices[v];
			//cout << " copying vertex " << v << Vertices[v] << " from source " << Source.Vertices[v] << endl;
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
	bool dev = 0;
	face &Face = *this;
    
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
	bool dev = 0;
	if (dev)cout << " Setting new edges... " << V0 << ", " << V1 << ", " << V2 << ", " << V3 << ", " << V4 << ", " << V5 << endl;
	EdgeIDs[0] = V0;
	EdgeIDs[1] = V1;
	EdgeIDs[2] = V2;
	EdgeIDs[3] = V3;
	EdgeIDs[4] = V4;
	EdgeIDs[5] = V5;
	//if (dev) cout << "   EdgeIDs " << EdgeIDs[0] << EdgeIDs[1] << EdgeIDs[2] << EdgeIDs[3] << EdgeIDs[4] << EdgeIDs[5] << endl;
	EdgeH = GetVector( Vertices[V0] , Vertices[V1] );
	EdgeV = GetVector( Vertices[V2] , Vertices[V3] );
	Hypo  = GetVector( Vertices[V4] , Vertices[V5] );
	if (dev) cout << "   Edge Vectors " << EdgeH << EdgeV << Hypo << endl;
}

void face::RefreshEdges()
{
	EdgeH = GetVector( Vertices[EdgeIDs[0]] , Vertices[EdgeIDs[1]] );
	EdgeV = GetVector( Vertices[EdgeIDs[2]] , Vertices[EdgeIDs[3]] );
	Hypo  = GetVector( Vertices[EdgeIDs[4]] , Vertices[EdgeIDs[5]] );
}

bool face::GetVertexOrder()
{
	bool dev = 0;
	face &Face = *this;
	if (dev) cout << " Getting Vertex Order of Face with Tex "<<Face.Texture << " Orient " << Face.Orient << " ID " << Face.fID <<" (CW or CCW)..." << endl;
	gvector Normal = Face.Normal;
	gvector Edge1 = GetVector(Face.Vertices[0],Face.Vertices[1]);
	gvector Edge2 = GetVector(Face.Vertices[1],Face.Vertices[2]);
	
	// Scalar of Face NVector and ZChecker has to be >0 to be able to determine the relative vector YAW. just rotate everything until this applies
	gvector ZChecker(0,0,1);
	if (dev) cout << "     Getting Scalar of Checker "<<ZChecker<<" and Face Normal " << Normal << endl;
	float Scalar = GetDot(ZChecker,Normal);
	if (dev) cout << "       Scalar " << Scalar << " Normal " << Normal << endl;
	while (Scalar<0.01)
	{
		Normal.rotate(90,0,0);
		Edge1.rotate(90,0,0);
		Edge2.rotate(90,0,0);
		Scalar = GetDot(ZChecker,Normal);
		if (dev) cout << "       Scalar " << Scalar << " Normal " << Normal << endl;
		if (Scalar<0.01)
		{
			Normal.rotate(0,90,0);
			Edge1.rotate(0,90,0);
			Edge2.rotate(0,90,0);
			Scalar = GetDot(ZChecker,Normal);
			if (dev) cout << "       Scalar " << Scalar << " Normal " << Normal << endl;
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
	if (dev) cout << "       Edge1_Yaw " << Edge1_Yaw << " Check_Yaw " << Check_Yaw << endl;
	
	if (Face.Clockwise)
	{if (dev) cout << "     Face Clockwise!" << endl << endl;}
	else
	{if (dev) cout << "     Face Counter-Clockwise!" << endl << endl;}
	
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
	//cout << " Face Tex " << Face.Texture << " VecV " << Vec << " Normal " << Normal << endl;
	if (Dot>0.0001||Dot<-0.0001) { Face.HasWorldAlign = 1; }//cout << " Face has Worldalign! " << endl; }
	//cout << "  Dot " << Dot << endl;
}

bool DoFacesShareVerts(face &F1, face &F2, int deciplaces)
{
	bool dev = 0;
	if (dev) cout << "    Checking if F1 ("<<F1.Texture<<") and F2 ("<<F2.Texture<<") share vertices..." << endl;
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

bool IsVertexOnFace(face &Face, vertex &V, int deci)
{
	int p = pow(10,deci); // precision
	if (p==0) p=1;
	
	// distance
	double d = GetDistFaceVertex(Face,V);
	
	if ( (floorf(d*p)/p)==0 ) return true;
	else return false;
}

double GetDistFaceVertex(face &Face, vertex &V)
{
	bool dev = 0;
	if (dev) cout << "      Getting Distance between Vertex " <<V<< " and Face " << Face.Texture << endl;
	// get distance (0,0,0)|<-->|Face
	double d = GetAdjaLen(  GetVector( Zero, Face.Vertices[0] ), Face.Normal  );
	if (dev) cout << "         Getting Dist Face<->(0|0|0) from Zero " << Zero << " and Face-Vertex[0] " << Face.Vertices[0] << " = " << d << endl;
	float x1 = Face.Normal.x;
	float x2 = Face.Normal.y;
	float x3 = Face.Normal.z;
	float p1 = V.x;
	float p2 = V.y;
	float p3 = V.z;
	
	if (dev) cout << "         Getting Results from Face Normal " << Face.Normal << " and Test-Vertex " << V << endl;
	double tempA = (x1*p1)+(x2*p2)+(x3*p3)-d;
	double tempB = sqrt( pow(x1,2) + pow(x2,2) + pow(x3,2) );
	if (dev) cout << "         A " << tempA << endl;
	if (dev) cout << "         B " << tempB << endl;
	
	double result = tempA / tempB;
	if (dev) cout << "         result " << result << endl;
	
	return result;
}

// Get Base Edge
void GetBaseEdges(face &Face)
{
	bool dev = 0;
	if (dev) cout << "Getting Base Edges of Face with Tex " << Face.Texture << endl;
	// this function will determine the base edge (bottom edge) for the 2 Texture Vectors
	// by checking the angles between all vertices and the relevant T-Vector. the smallest Angle will lead to the Face vertex, that acts as the base edge
	gvector *TVec = nullptr;
	int *EdgeID = nullptr;
	int *EdgeID2 = nullptr;
	vector<int> *BaseList = nullptr;
	for (int vec = 0; vec<2; vec++)
	{
		vector<float> baseList_SCAL;
		if (dev&&vec==0) cout << "  Getting Base Edge of Horizontal Texture Vector" << Face.VecX << endl;
		if (dev&&vec==1) cout << "  Getting Base Edge of Vertical Texture Vector" << Face.VecY << endl;
		
		if 	(vec==0) 	TVec = &Face.VecX;
		else 			TVec = &Face.VecY;
		if (vec==0)		{EdgeID = &Face.BaseX; EdgeID2 = &Face.BaseX2; BaseList = &Face.BaseListX;}
		else 			{EdgeID = &Face.BaseY; EdgeID2 = &Face.BaseY2; BaseList = &Face.BaseListY;}
		
		vertex &cen = Face.Centroid;
		if (dev) cout << "   Current Centroid is " << Face.Centroid << endl;
		float sList[Face.vcount]; // scalar list
		//float dList[Face.vcount]; // distance list
		int s = 0, b = 0, s2 = 0, b2 = 0;
		// create vectors from vertices and centroid of current Face and then scalar products of these vectors and the texture vector to find the base edge (rather base "vertex")
		for (int v = 0; v<Face.vcount; v++)
		{
			// Get Dot Product
			gvector VecC = GetVector(cen, Face.Vertices[v]);
			//VecC = Normalize(VecC);
			sList[v] = GetDot(*TVec, VecC);
			if (dev) cout << "     Current Vertex " << v << Face.Vertices[v] << " Scalar: " << sList[v] << endl;
			
			// Get Adjacent Length
			//dList[v] = GetAdjaLen(VecC, *TVec);
			//cout << "     Current Vertex " << v << Face.Vertices[v] << " Distance: " << dList[v] << endl;
			
			if (sList[v]>sList[s]) {s = v;}
			if (sList[v]<sList[b]) {b = v;}
			
		}
		if (vec==0) {
		if (s==0) s2 = 1; else if (s==1) s2 = 0; else if (s==2) s2 = 3; else if (s==3) s2 = 2;
		if (b==0) b2 = 1; else if (b==1) b2 = 0; else if (b==2) b2 = 3; else if (b==3) b2 = 2;
		} else {
		if (s==0) s2 = 3; else if (s==1) s2 = 2; else if (s==2) s2 = 1; else if (s==3) s2 = 0;
		if (b==0) b2 = 3; else if (b==1) b2 = 2; else if (b==2) b2 = 1; else if (b==3) b2 = 0;
		}
		BaseList->at(0) = s;
		BaseList->at(1) = s2;
		BaseList->at(2) = b2;
		BaseList->at(3) = b;
		
		
		
		/*
		for (int v = 0; v<Face.vcount; v++)
		{
			baseList_SCAL.push_back(sList[v]);
			BaseList->push_back(0);
		}
		sort (baseList_SCAL.begin(), baseList_SCAL.end(), comp);
		// Sorting Scalar List and Faces Vertex ID List
		for (int v = 0; v<Face.vcount; v++)
			for (int c = 0; c<Face.vcount; c++)
				if (sList[v]==baseList_SCAL[c]) BaseList->at(v) = c;
		
		cout << "Sorted Scalar of Face-Tex " << Face.Texture << endl;
		if (Face.fID==2)
		for (int v = 0; v<Face.vcount; v++)
			cout << " Basevertex " << v << " - " << BaseList->at(v) << endl;*/
		
		if (dev) cout << "  Biggest Scalar and therefor Base Edge is vertex " << s << Face.Vertices[s] << endl;
		if (dev) cout << "  Smallest Scalar and therefor Counter-Base Edge is vertex " << b << Face.Vertices[b] << endl;
		*EdgeID = s;
		*EdgeID2 = b;
		
	}
	
	if (dev) { cout << "  Final Base Vertex List: " << endl;
	for (int i = 0; i<4; i++) {
		cout << "    #" <<i <<" BaseX " << Face.BaseListX[i] << "\t BaseY " << Face.BaseListY[i] << endl;
	}
	cout << endl; }
}

// Get Base Edge
void GetBaseEdgesC(face &Face)
{
	bool dev = 0;
	if (dev) cout << "Getting Base Edges of Face with Tex " << Face.Texture << endl;
	// this function will determine the base edge (bottom edge) for the 2 Texture Vectors
	// by checking the angles between all vertices and the relevant T-Vector. the smallest Angle will lead to the Face vertex, that acts as the base edge
	gvector *TVec = nullptr;
	int *EdgeID = nullptr;
	for (int vec = 0; vec<2; vec++)
	{
		if (dev&&vec==0) cout << "  Getting Base Edge of Horizontal Texture Vector" << Face.VecX << endl;
		if (dev&&vec==1) cout << "  Getting Base Edge of Vertical Texture Vector" << Face.VecY << endl;
		
		if 	(vec==0) 	TVec = &Face.VecX;
		else 			TVec = &Face.VecY;
		if (vec==0)		EdgeID = &Face.BaseX;
		else 			EdgeID = &Face.BaseY;
		
		vertex &cen = Face.Centroid;
		if (dev) cout << "   Current Centroid is " << Face.Centroid << endl;
		float sList[Face.vcountC];
		int s = 0;
		// create vectors from vertices and centroid of current Face and then scalar products of these vectors and the texture vector to find the base edge (rather base "vertex")
		for (int v = 0; v<Face.vcountC; v++)
		{
			gvector VecC = Normalize(GetVector(cen, Face.VerticesC[v]));
			sList[v] = GetDot(*TVec, VecC);
			
			if (dev) cout << "     Current Vertex " << v << Face.VerticesC[v] << " Scalar: " << sList[v] << endl;
			
			if (sList[v]>sList[s]) s = v;
		}
		if (dev) cout << "  Biggest Scalar and therefor Base Edge is vertex " << s << Face.VerticesC[s] << endl;
		*EdgeID = s;
	}
}

// Get Original Texture Offsets from this face
void GetTexOffset(face &Face, int mode = 0)
{
	// mode 0 = both Vectors, mode 1 = X Vector, mode 2 = Y Vector
	bool dev = 0;
	if (dev) cout << "Getting Offset for Face with Tex " << Face.Texture << endl;
	
	int texw = gFile->tTable_width[Face.tID];
	int texh = gFile->tTable_height[Face.tID];
	if (texw<16) texw = 16;
	if (texh<16) texh = 16;
	
	if (dev) cout << " Tex Size " << texw << " * " << texh << endl;
	
	float *Scale = nullptr;
	float *Shift = nullptr;
	float *Offset = nullptr;
	float *BaseShift = nullptr;
	float tex_size = 0;
	
	if (dev) cout << " Current Face Info | Texture " << Face.Texture << " TexID " << Face.tID << " texw " << texw << " texh " << texh << endl;
	
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
			if (dev) cout << "  Tex size is " << tex_size << "(tex width ["<<texw<<"] XXX ScaleX ["<<Face.ScaleX<<"] )"<< endl;
		}
		if (vec==1)
		{
			Shift		= &Face.ShiftY;
			Scale		= &Face.ScaleY;
			Offset 		= &Face.OffsetY;
			BaseShift 	= &Face.BaseShiftY;
			tex_size	= texh;
			if (dev) cout << "  Tex size is " << tex_size << "(tex height ["<<texh<<"] XXX ScaleY ["<<Face.ScaleY<<"] )"<< endl;
		}
		
		if ((vec==0&&(mode==0||mode==1))||(vec==1&&(mode==0||mode==2)))
		{
			float B = *BaseShift;
			float S = *Shift;
			
			//if (B!=tex_size)
			//if (B<S*2)			while (B<S*2) 	{ B += tex_size; if (dev) cout << "    BaseShift smaller than "<<S*2<<" - now: " << B << endl;}
			if (B>S)				while (B>S) 	{ B -= tex_size; if (dev) cout << "    BaseShift bigger than "<<S<<" - now: " << B << endl; }
			
			//if (b!=tex_size)
			//if (b<tex_size)				while (b<tex_size) { b += tex_size; if (dev) cout << "    Shift smaller than "<<tex_size<<" - now: " << b << endl;}
			//else if (b>tex_size*2)		while (b>tex_size*2) { b -= tex_size; if (dev) cout << "    Shift bigger than "<<tex_size*2<<" - now: " << b << endl;}
			
			if (dev) cout << "     BaseShift now: " << B << " Shift now: " << S << endl;
			*Offset = S - B;
		}
	}
	if (dev) cout << "  Offset X is " << Face.OffsetX << endl;
	if (dev) cout << "  OFfset Y is " << Face.OffsetY << endl << endl;
}


// axis 0 = X+Y Baseshift, axis 1 = X only, axis 2 = Y only
// mode 0 = longest , 1 = shortest
void GetBaseShift(face &Face, int axis = 0, bool longEdge = 1, bool C = 0)
{
	bool dev = 0;
	if (dev) cout << "Getting BaseShift for Face with Tex " << Face.Texture << endl;
	
	gvector Vec;
	float *BaseShift = nullptr;
	float *Scale = nullptr;
	if (dev&&C) cout << " BaseX1 " << Face.BaseX << " BaseX2 " << Face.BaseX2 << " BaseY1 " << Face.BaseY << " BaseY2 " << Face.BaseY2 << endl;
	if (dev&&!C) cout << " BaseX1 " << Face.BaseListX[0] << " BaseX2 " << Face.BaseListX[3] << " BaseY1 " << Face.BaseListY[0] << " BaseY2 " << Face.BaseListY[3] << endl;
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
			if (dev) cout << "  Base-Vertex is " << BaseVertex << " Tex Vec is " << Vec <<  endl;
		}
		if (vec==1)
		{
			Vec = Face.VecY;
			if (longEdge) {
				if (C) 	{BaseVertex = Face.VerticesC[Face.BaseY];}
				else 	{BaseVertex =  Face.Vertices[Face.BaseListY[0]];  BaseVertex2 = Face.Vertices[Face.BaseListY[3]];}
			} else {
						{BaseVertex =  Face.Vertices[Face.BaseListY[1]];  BaseVertex2 = Face.Vertices[Face.BaseListY[2]];}
			}
			Scale = &Face.ScaleY;
			BaseShift = &Face.BaseShiftY;
			if (dev) cout << "  Base-Vertex is " << BaseVertex << " Tex Vec is " << Vec <<  endl;
		}
				
		if ((vec==0&&(axis==0||axis==1))||(vec==1&&(axis==0||axis==2)))
		{
			// check vec pitch (specifies wether BaseShift is positive or negative)
			/*float Vec_pitch = GetVecAlign(Vec, 1);
			if (!Vec.IsHor&&Face.fID==2)
			if (!dev) cout << "   Vec Pitch of Face Tex "<< Face.Texture << " is " << Vec_pitch << endl;*/
			vertex BVertex;
			if (Vec.IsHor)
			{
				if (Vec.IsNeg) BVertex = BaseVertex; else BVertex = BaseVertex2;
			}
			else BVertex = BaseVertex; // ????????
			
			gvector Hypo = GetVector(Zero, BVertex);		if (dev) cout << "  Vector VecBN of Base-Vertex and NULL: " << Hypo <<  endl;
			
			float Hypo_Len = GetVecLen(Hypo);				if (dev) cout << "  Length: " << Hypo_Len <<  endl;
			
			float alpha = GetAngleCos(Hypo, Vec);
			float alpha_deg = acos(alpha)*180/PI;			if (dev) cout << "  Angle between VecBN and VecTV: " << alpha_deg <<  endl;
			
			float m = 1.0 / *Scale; // Texture Scale Modifier, because Texture Shift depends on Texture Scale
			*BaseShift = -((alpha * Hypo_Len)*m);
			if (!IsValid(*BaseShift)) *BaseShift = 0;
		}
	}
	if (dev) cout << "  BaseShift X is " << Face.BaseShiftX << endl;
	if (dev) cout << "  BaseShift Y is " << Face.BaseShiftY << endl << endl;
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
	//cout << "FACE TEX: " << Face.Texture << endl;
	// Vertical Vector has the following coordinate restrictions: X has to be always 0
	gvector *Vec1 = nullptr;
	gvector *Vec2 = nullptr;
	//gvector *FaceVec1 = nullptr;
	//gvector *FaceVec2 = nullptr;
	for (int i = 0; i<2; i++)
	{
		if (i==0) 	{ Vec1 = &Face.VecX; Vec2 = &Face.VecY; /*FaceVec1 = Face.VecV; FaceVec2 = Face.VecH;*/ }
		else 		{ Vec1 = &Face.VecY; Vec2 = &Face.VecX; /*FaceVec1 = Face.VecH; FaceVec2 = Face.VecV;*/ }
		
		if 	(Vec1->x==0||(Vec1->x<0.01&&Vec1->x>-0.01)) { /*FaceVec1=Vec1; FaceVec2=Vec2; FaceVec2->IsHor=1;*/ VvecFound = 1; Face.VecV=Vec1; Face.VecH=Vec2; Face.VecH->IsHor=1; Face.VecV->IsHor=0;
			//cout << "  Found Vertical Vector! Vector " << *Vec1 << endl;
		}
	}
	
	// check if other Vector is valid horizontal vector
	// Horizontal Vector has the following coordinate restrictions: Z has to be always 0
	gvector *Hvec = Face.VecH;
	if (VvecFound) {
		if (Hvec->z==0 || (Hvec->z<0.01&&Hvec->z>-0.01))
		{
			//cout << "  Found Horizontal Vector, too! Vector " << *Hvec << endl;
			return true;
		}
		else
		{
			//cout << "  No Horizontal Vector found! Face (tex: "<<Face.Texture<<" vector "<< *Hvec<<") has Invalid Face Align!" << endl;
			return false;
		}
	}
	else
	{
		//cout << "  No Vertical Vector found! Face (tex: "<<Face.Texture<<") has Invalid Face Align!" << endl;
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
		if (YawAngle > 45 && YawAngle < 135) {
			
			return X;
			
		} else if (YawAngle <= 45 || YawAngle >= 135) {

			return Y;
		}
	}
}

void AlignToWorld(face &Face) {
	
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


float GetFaceLen(face &Face) {
	
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
	
	//if(Face.fID==2) cout << "Face Length: " << AlphaCos * GetVecLen(Vec1) << endl;
	//cout << " vertex " << p1 << p2 << p3 << " vectors: " << Vec1 << Vec2 << " alphacos: " << AlphaCos << endl;
	//cout << "Face Length: " << AlphaCos * GetVecLen(Vec1) << endl;
	
	float result = AlphaCos * GetVecLen(Vec1);
	if (!IsValid(result)) result = 0;
	
	return result;
}

ostream &operator<<(ostream &ostr, face &Face)
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
	
	return ostr;
}


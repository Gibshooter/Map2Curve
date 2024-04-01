#include "frames.h"
#include "vertex.h"
#include "utils.h"
#include "group.h"
#include "settings.h"

#include <iostream>

using namespace std;

extern group *sGroup;
extern group *mGroup;
extern ctable *cTable;
extern group *bGroup;

#define DEBUG 0

// relevant to constr. frame and path file handling:
// file::LoadSpline
// path_set::Analyze()
// createTableC
// group::Build
// group::RotateVectors()
// face::CreateRamp
// brush::Triangulate
// group::Triangulate


/* ===== PATH CLASSES & FUNCTIONS ===== */


ostream &operator<<(ostream &ostr, path &p)
{
	for(int i = 0; i<p.t_corners;i++)
	{
		ostr << "    Knot #"<<i<< p.Corners[i].pos << endl;
	}
	return ostr;
}

ostream &operator<<(ostream &ostr, path_set &ps)
{
	ostr << " Spline Set with " << ps.t_paths << " total splines and " << ps.t_corners << " total knots:" << endl;
	for (int i=0;i<ps.t_paths; i++)
	{
		ostr << "  Spline #" << i << endl;
		ostr << ps.Paths[i];
		ostr << endl;
	}
	return ostr;
}



void path::ScaleOrigin(tform n, vertex Origin)
{
	Move(-Origin.x, -Origin.y, -Origin.z);
	
	for (int i=0;i<t_corners;i++)
	{
		path_corner &K  = Corners[i];
		if(n.x!=0) K.pos.x *= n.x;
		if(n.y!=0) K.pos.y *= n.y;
		if(n.z!=0) K.pos.z *= n.z;
	}
	
	Move(Origin.x, Origin.y, Origin.z);
}

void path::Expand(float n)
{
	#if DEBUG > 0
	bool dev = 0;
	if(n!=0)
	if(dev) cout << " Expanding Spline by "<<n<<" units ..." << endl;
	#endif
	
	path_corner Back[t_corners];
	for (int i=0;i<t_corners;i++)
		Back[i] = Corners[i];
	
	bool IsCircle = 0;
	if (  CompareVertices( Corners[0].pos, Corners[t_corners-1].pos )  ) IsCircle = 1;
	
	for (int i=0;i<t_corners-1;i++)
	{
		path_corner &K  = Corners[i];
		
		#if DEBUG > 0
		if(dev) cout << "   Knot " << i << "/" << t_corners << endl;
		#endif
		
		if(i==0)
		{
			if(!IsCircle)
			{
				gvector zVec(0,0,1);
				gvector nCross = Normalize(  GetCross( GetVector(Back[1].pos,Back[0].pos), zVec )  );
				nCross.mult(n); nCross.z = 0; nCross.flip();
				Corners[0].pos.Add(nCross);
				
				#if DEBUG > 0
				if(dev) cout << "     First Knot! Old Pos " << Back[0].pos << " Adding " << nCross << " = " << Corners[0].pos << endl;
				#endif
			}
			else
			{
				gvector Vec1 = GetVector(Back[t_corners-2].pos, Back[t_corners-1].pos);
				gvector Vec2 = GetVector(Back[1].pos, Back[0].pos);
				gvector nVec = Normalize(  VecAdd(  Vec1, Vec2  )  );
				gvector Tester = Vec2; Tester.rotate(0,0,90);
				if (GetDot(nVec,Tester)<0) nVec.flip();
				nVec.mult(n); nVec.z = 0;
				Corners[0].pos.Add(nVec);
			}
		}
		else
		{
			gvector Vec1 = Normalize(GetVector(Back[i-1].pos, Back[i].pos));
			gvector Vec2 = Normalize(GetVector(Back[i+1].pos, Back[i].pos));
			gvector nVec = Normalize(  VecAdd(  Vec1, Vec2  )  );
			gvector Tester = Vec2; Tester.rotate(0,0,90);
			if (GetDot(nVec,Tester)<0) nVec.flip();
			nVec.mult(n); nVec.z = 0;
			Corners[i].pos.Add(nVec);
			
			#if DEBUG > 0
			if(dev) cout << "     Old Pos " << Back[i].pos << " Adding " << nVec << " = " << Corners[i].pos << endl;
			#endif
			
			if (i==t_corners-2)
			{
				if(!IsCircle)
				{
					gvector zVec(0,0,1);
					gvector nCross = Normalize(  GetCross( GetVector(Back[i+1].pos, Back[i].pos), zVec )  );
					nCross.mult(n); nCross.z = 0; nCross.flip();
					Corners[i+1].pos.Add(nCross);
					
					#if DEBUG > 0
					if(dev) cout << "     Last Knot! Old Pos " << Back[i+1].pos << " Adding " << nCross << " = " << Corners[i+1].pos << endl;
					#endif
				}
				else
				{
					Corners[i+1].pos = Corners[0].pos;
				}
			}
		}
	}
	
	#if DEBUG > 0
	if(dev) system("pause");
	#endif
}

void path::EvenOut()
{
	#if DEBUG > 0
	bool dev = 0;
	// 1. Get shortest Section Length
	if (dev) cout << " 1. Getting shortest Section Length... " << endl;
	#endif
	
	float SecLen[t_corners-1];
	gvector SecVec[t_corners-1];
	float sLen = 0;
	for (int k=0; k<t_corners-1; k++)
	{
		path_corner &Knot = Corners[k];
		path_corner &KnotN = Corners[k+1];
		gvector Vec = GetVector(Knot.pos, KnotN.pos);
		SecVec[k] = Vec;
		float Len = GetVecLen(Vec);
		SecLen[k] = Len;
		if(k==0) sLen = Len;
		else if(Len<sLen) sLen = Len;
	}
	float max = sLen * 1.75;

	#if DEBUG > 0
	if (dev) cout << "   Shortest Sec " << sLen << "(*1.2="<<sLen*1.2<<") Limit " << max << endl;
	#endif

	sLen *= 1.2; // make it a little longer maybe
	
	vector<path_corner> NewKnots;
	
	#if DEBUG > 0
	if (dev) cout << " 2. Beginning to create new knots... " << endl;
	#endif
	
	// 2. Begin to create new knots for sections that are at least 1.75 times bigger than the shortest(*1.2)
	for (int k=0; k<t_corners-1; k++)
	{
		path_corner &Knot = Corners[k];
		
		#if DEBUG > 0
		if (dev) cout << "   Old Knot #" << k << " Sec Length " << SecLen[k] << " Vec " << SecVec[k] << endl;
		#endif
		
		NewKnots.push_back(Knot);
		if(SecLen[k]>max) // section length greater than the limit
		{
			int subs = round( SecLen[k] / sLen ); // how many subdivisions?
			float subLen = SecLen[k] / subs;
			
			#if DEBUG > 0
			if (dev) cout << "     !!! Section length longer than the limit (" << max << ")" << endl;
			if (dev) cout << "     Creating " << subs-1 << " subdivisions for this section with length " << subLen << endl;
			#endif
			
			for(int s=0; s<subs-1; s++) // create new knots for this section based on the subdiv length
			{
				path_corner sub;
				sub.pos = Knot.pos;
				gvector addVec = Normalize(SecVec[k]);
				addVec.mult((s+1)*subLen);
				sub.pos.Add(addVec);
				NewKnots.push_back(sub);
				
				#if DEBUG > 0
				if (dev) cout << "      New Knot #" << s << " pos " << sub.pos << " addVec " << addVec << endl;
				#endif
			}
		}
	}
	// add last knot, or else it won't get a chance :(
	NewKnots.push_back(Corners[t_corners-1]);
	
	// 3. create path of new knots
	#if DEBUG > 0
	if (dev) cout << " 3. Creating path of new knots... " << endl;
	#endif
	
	path_corner *nKnotPtr = new path_corner[NewKnots.size()];
	for (int k=0; k<NewKnots.size(); k++)
	{
		path_corner &nKnot = NewKnots[k];
		path_corner &pKnot = nKnotPtr[k];
		pKnot = nKnot;
		
		#if DEBUG > 0
		if (dev) cout << "   #"<<k<<" of New Knot Array " << pKnot.pos << " made from new knot " << nKnot.pos << endl;
		#endif
	}
	
	// 4. replace old knots
	#if DEBUG > 0
	if (dev) cout << " 4. Replacing old knots... " << endl;
	#endif
	
	if (Corners!=nullptr) delete[] Corners;
	Corners = nKnotPtr;
	t_corners = NewKnots.size();
}

void path::reverse() {
	for (int i = 0; i<t_corners/2; i++)
		swap(Corners[i],Corners[t_corners-1-i]);
}

void path::Copy(path &Source)
{
	t_corners = Source.t_corners;
	t_length = Source.t_length;
	valid = Source.valid;
	Corners = new path_corner[t_corners];
	for (int i=0; i<t_corners; i++) {
		Corners[i] = Source.Corners[i];
	}
}

void path::Move(float x, float y, float z)
{
	for (int v = 0; v<t_corners; v++)
	{
		vertex &V = Corners[v].pos;
		
		if(x!=0) V.x += x;
		if(y!=0) V.y += y;
		if(z!=0) V.z += z;
	}
}



void path_set::Scale(tform n)
{
	GetSplineSetDimensions();
	
	for (int p=0;p<t_paths;p++)
	{
		Paths[p].ScaleOrigin(n, Origin);
	}
}

void path_set::GetSplineSetDimensions()
{
	bool setBox = 0;
	// get path origin
	for (int p=0;p<t_paths;p++)
	{
		for (int v = 0; v<Paths[p].t_corners; v++)
		{
			vertex &V = Paths[p].Corners[v].pos;
			
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
		}
	}
	Origin.x = D.xb-((D.xb-D.xs)/2);
	Origin.y = D.yb-((D.yb-D.ys)/2);
	Origin.z = D.zb-((D.zb-D.zs)/2);
}

int path_set::CountSections()
{
	int Secs=0;
	for (int p = 0; p<t_paths; p++) // path loop
	{
		path &Path = Paths[p];
		if (Path.t_corners==1) Secs++;
		else Secs += Path.t_corners-1;
	}
	return Secs;
}

void path_set::PathToDevAssets()
{
	path_set &List = *this;
	
	for (int p = 0; p<List.t_paths; p++) // path loop
	{
		path &Path = List.Paths[p];
		for (int c = 0; c<Path.t_corners-1; c++) // corner loop
		{
			path_corner &Corner1 = Path.Corners[c];
			path_corner &Corner2 = Path.Corners[c+1];
			gvector Edge; Edge.set(Corner1.pos, Corner2.pos, Corner1.pos);
			
			gvector Rot = Edge;
			Rot.rotate(0,0,90);
			Rot.Normalize();
			brush* Brush = new brush;
			
			Brush->VecToBrush(Edge, Rot, "{BLUE");
			
			bGroup[List.gID].DevAssets.push_back(Brush);
			
			//cout << " Curve " << List.gID << " Path " << p << " C1# " << c << Corner1.pos << " C2# " << c+1 << Corner2.pos << " Edge " <<  Edge << " Edge2 " << Rot << " bGroup["<<List.gID<<"].DevAssets.size " << bGroup[List.gID].DevAssets.size()<< endl;
			//system("pause");
		}
	}
}

void path_set::Analyze()
{
	#if DEBUG > 0
	bool dev = 0;
	#endif

	path_set &List = *this;
	
	for (int p = 0; p<List.t_paths; p++) // path loop
	{
		path &Path = List.Paths[p];
		if (cTable[gID].p_evenout>0) {
			Path.EvenOut();
			t_corners=0;
			for (int p=0;p<t_paths;p++) t_corners += Paths[p].t_corners;
		}
	}
	
	//cout << " Analyzing Path Set with " << List.t_paths<< " paths in it... " << endl;
	for (int p = 0; p<List.t_paths; p++) // path loop
	{
		path &Path = List.Paths[p];
		// determine starting direction of this path (not needed ATM)
		
		if (cTable[gID].p_expand!=0) Path.Expand(cTable[gID].p_expand);
		if (List.preverse) {Path.reverse();}
		
		// determine angle of all sections
		for (int c = 0; c<Path.t_corners; c++) // corner loop
		{
			if (c<Path.t_corners-1)
			{
				path_corner &Corner1 = Path.Corners[c];
				path_corner &Corner2 = Path.Corners[c+1];
				
				// flattened corners
				path_corner C1 = Corner1; C1.pos.z = 0;
				path_corner C2 = Corner2; C2.pos.z = 0;
				
				// Angle
				gvector Edge = GetVector(C1.pos, C2.pos);
				Corner1.Yaw = GetVecAlign(Edge, 0);
				Corner1.Pitch = GetVecAlign(Edge, 1);
				float EdgeLen = GetVecLen(Edge);
				Corner1.length = EdgeLen;
				Path.t_length += EdgeLen;
				
				#if DEBUG > 0
				if (dev) cout << " Path " << p<< " length " << EdgeLen << endl;
				#endif
				
				if (EdgeLen==0||!IsValid(EdgeLen))
				{
					valid=0;
					cout << "|    [ERROR] Spline contains invalid knots! Path #"<<p<< ", Knot #" << c << Corner1.pos << ", section has invalid length: " << EdgeLen << endl;
					break;
				}
				
				// relative Height
				Corner1.step = Corner2.pos.z-Corner1.pos.z;
				
				if (c<Path.t_corners-2)
				{
					path_corner &Corner3 = Path.Corners[c+2];
					gvector Edge2 = GetVector(Corner2.pos, Corner3.pos);
					
					// Determine whether next section is heading left or right
					gvector Vec_Yaw = Edge2;
					Vec_Yaw.rotate(0,0,-(Corner1.Yaw));
					float Check_Yaw = GetVecAlign(Vec_Yaw,0);
					if (Check_Yaw>180) Corner1.NextIsCW = 1;
				}
			}
			if (!valid) break;
		}
		t_length+=Path.t_length;

		#if DEBUG > 0
		if (dev) cout << "   PathSet total length " << t_length << endl;
		if (dev) system("pause");
		#endif
		
		// determine section-align (0,90,180,270)
		if ((List.type==2||List.type==3)&&valid)
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
		if (List.type==2&&valid)
		for (int c = 0; c<Path.t_corners-2; c++) // corner loop
		{
			path_corner &Corner = Path.Corners[c];
			path_corner &CornerN = Path.Corners[c+1];
			path_corner &CornerN2 = Path.Corners[c+2];
			int CornerYaw = round(Corner.Yaw);
			int CornerNYaw = round(CornerN.Yaw);
			
			bool AngleIs45 = 0;  if (CornerYaw==45||CornerYaw==135||CornerYaw==225||CornerYaw==315) AngleIs45=1;
			bool AngleNIs45 = 0; if (CornerNYaw==45||CornerNYaw==135||CornerNYaw==225||CornerNYaw==315) AngleNIs45=1;
			
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
			
			if (Diff==1||Diff==3)
			{
				if (AngleIs45&&AngleNIs45&&CornerDistIsValid&&Plain&&PlainN)
				{
					if (Corner.NextIsCW)
					{
						Corner.rot2  -= 90;
					}
					else 
					{
						CornerN.rot1 -= 90;
					}
				}
				else if (!AngleIs45&&AngleNIs45&&!Corner.NextIsCW&&CornerNDistIsValid&&PlainN)
				{
					CornerN.rot1 -= 90;
				}
				else if (AngleIs45&&!AngleNIs45&&CornerDistIsValid&&Plain&&Corner.NextIsCW)
				{
					Corner.rot2 -= 90;
				}
				else if (List.cornerFix || (!List.cornerFix&&Corner.NextIsCW))
				{
					Corner.NextIsGap90 = 1;
					List.Gaps++;
				}
			}
			else if (Diff==2)
			{
				if (AngleIs45&&AngleNIs45&&Plain&&PlainN&&CornerDistIsValid)
				{
					if (Corner.NextIsCW)
					{
						Corner.rot2  -= 90;
					}
					else 
					{
						Corner.rot2  += 90;
						CornerN.rot1 -= 90;
					}
				}
				else if (AngleIs45&&!AngleNIs45&&Plain&&CornerDistIsValid&&Corner.NextIsCW)
				{
					{
						Corner.NextIsGap90 = 1;
						List.Gaps++;
						Corner.rot2  -= 90;
					}
				}
				// if world align of 2 sections differs by 180 degree
				else if (List.cornerFix || (!List.cornerFix&&Corner.NextIsCW))
				{
					Corner.NextIsGap180 = 1;
					List.Gaps+=2;
				}
			}
		}
	}
}





/* ===== CIRCLE METHODS & FUNCTIONS ===== */

void circle::AddHeight(int g, vector<path_set> &Set)
{
	#if DEBUG > 0
	bool dev = 0;
	#endif
	
	vector<float> &height = bGroup[g].heightTable;
	vector<float> &step = bGroup[g].heightTableSteps;
	for (int sec=0, v=0; sec<cTable[g].res; sec++)
	{
		vertex &V = Vertices[v];
		vertex &VN = Vertices[v+1];
		float heightDiff = V.z - height[sec];
		float heightDiffN = VN.z - height[sec];
		if (cTable[g].type==0||cTable[g].type==1) {
			V.z += height[sec];
			if (sec==cTable[g].res-1)
			VN.z = V.z+step[sec];
			v++;
		} else if (cTable[g].type==2||cTable[g].type==3) {
			V.z += height[sec]; //heightDiff
			VN.z = V.z+step[sec];
			v+=2;
		}
		
		#if DEBUG > 0
		if(dev) cout << " Spline sec " << sec << " Height P1: " << V.z << " P2 " << VN.z << endl;
		#endif
	}
}

void circle::ConvertToSpline(int g)
{
	#if DEBUG > 0
	bool dev = 0;
	#endif

	int res = cTable[g].res;
	if(cTable[g].type==0||cTable[g].type==1)
	{
		#if DEBUG > 0
		if (dev) cout << " Converting Circle object (res: "<<tverts<<") to Spline (new res: "<< cTable[g].res*2 <<")..." << endl;
		#endif
		
		//create new vertex array
		vertex *NewVerts = new vertex[res*2];
		for(int vo=0,vn=0;vo<res;vo++) {
			vertex &VO = Vertices[vo];
			vertex &VON = Vertices[vo+1];
			vertex &VN = NewVerts[vn];
			vertex &VNN = NewVerts[vn+1];
			VN = VO;
			VNN = VON;
			vn+=2;
		}
		vertex *Backup = Vertices;
		delete[] Backup;
		Vertices = NewVerts;
		tverts = res*2;
		
		#if DEBUG > 0
		if(dev) system("pause");
		#endif
	}
}

void circle::GetInVec(int g)
{
	#if DEBUG > 0
	bool dev = 0;
	#endif
	
	bool IsSpline = 0;
	if(cTable[g].type==2||cTable[g].type==3) IsSpline = 1;
	InVec.resize(cTable[g].res);
	
	for (int v=0, sec=0; sec<cTable[g].res; sec++)
	{
		vertex &V = Vertices[v];
		vertex &VN = Vertices[v+1];
		gvector Vec = Normalize(GetVector( V, VN ));
		gvector Test = Vec; Test.rotate(0,0,90);
		
		// angle between this and last section
		if (sec>0)
		{
			vertex &VL = Vertices[v-2];
			gvector Vec2 = Normalize(GetVector(V, VL));
			if (round(V.Yaw)==round(VL.Yaw)) {
				InVec[sec] = Vec;
				InVec[sec].rotate(0,0,90);
			} else {
				InVec[sec] = Normalize(VecAdd(Vec, Vec2));
				if (GetDot(InVec[sec], Test)<0) InVec[sec].flip();
			}
			
			#if DEBUG > 0
			if(dev) cout << " #" <<sec<< " V1 " <<  V << endl << " V2 " << VN << endl << " V0 " << VL << endl << " Vec1 " << Vec << endl << " Vec2 " << Vec2 << " InVec " << InVec[sec] <<endl << endl;
			if(dev) system("pause");
			#endif
			
		} else {
			InVec[sec] = Vec;
			InVec[sec].rotate(0,0,90);
		}
		v+=2;
	}
}

void circle::GetAngles(int g)
{
	bool IsSpline = 0;
	if(cTable[g].type==2||cTable[g].type==3) IsSpline = 1;
	
	GetInVec(g);
	
	for (int v=0, sec=0; sec<cTable[g].res; sec++)
	{
		vertex &V = Vertices[v];
		vertex &VN = Vertices[v+1];
		gvector Vec = GetVector( V, VN );
		
		if (!CompareVerticesR(V,VN))
		{
			if(!IsSpline) V.Yaw = GetVecAlign(Vec, 0);
			if(sec>0) {
				vertex &VL = Vertices[v-2];
				gvector Vec2 = Normalize(GetVector( VL, V ));
				V.YawB = GetVecAlign(VecAdd(Normalize(Vec), Vec2),0);
			}
			gvector VecPitch(0,0,1);
			V.Pitch = GetVecAng(Vec, VecPitch)-90;
			v+=2;
		}
		else
		{
			if (sec>0) V.Yaw = Vertices[v-1].Yaw; else V.Yaw = 0;
			V.Pitch = 0;
		}
	}
}

void circle::reverse(int res) {
	for (int i = 0; i<res/2; i++)
		swap(Vertices[i],Vertices[res-1-i]);
}

void circle::build_circlePi(int res, float rad, float height, bool flat) {
	
	#if DEBUG > 0
	bool dev = 0;
	#endif
	
	Vertices = new vertex[res+1];
	
	float steps = 360.0/res;
	float step_half = 0; if(flat) step_half = steps/2;
	float alpha;
	
	for (int i = (res/4), j, k=0; i < res+(res/4)+1; i++) {
		
		alpha = (( (i * steps)+step_half ) *PI/180.0);
		if (k==0) j = res; else j = k-1;
		Vertices[j].x = rad*(cos(alpha));
		Vertices[j].y = rad*(sin(alpha));
		Vertices[j].z = height;
		k++;
	}
	
	if(flat&&step_half!=0)
	{
		// size difference between normal and "flat" circle
		float rad_flat = rad * cos(step_half*PI/180.0);
		float m = rad / rad_flat;
		
		#if DEBUG > 0
		if (dev) cout << " rad " << rad << " rad_flat " << rad_flat << " m " << m << endl;
		#endif
		
		// if starting angle > 0 scale circle to original radius again
		if(m!=0&&IsValid(m))
		for (int i = 0; i<res+1; i++)
		{
			#if DEBUG > 0
			if (dev) cout << " v " << i << Vertices[i];
			#endif
		
			Vertices[i].x *= m;
			Vertices[i].y *= m;
		
			#if DEBUG > 0
			if (dev) cout << " new " << Vertices[i] << endl;
			#endif
		}
	}
		
	reverse(res);
}

void circle::build_pathIntersect(int g, float posy, float height, path_set &Set)
{
	tverts = (Set.t_corners-Set.t_paths)*2;
	
	#if DEBUG > 0
	bool dev = 0;
	if(dev)cout << " total vertices of this vertex-path: " << tverts << " (total corners "<<Set.t_corners-Set.t_paths<<" *2 )" << endl;
	#endif
	
	Vertices = new vertex[tverts];
	float Size = sGroup[g].SizeY;
	
	// get intersection points between sections (except first and last sec)
	#if DEBUG > 0
	if(dev) cout << " Getting intersection points between sections... " << endl;
	#endif
	
	for (int p=0, v=0,sec=0; p < Set.t_paths; p++)
	{
		path &Path = Set.Paths[p];
		
		#if DEBUG > 0
		if(dev) cout << " path loop #" << p+1 << " of " << Set.t_paths<< endl;
		#endif
		
		bool IsCircle = CompareVertices(  Path.Corners[0].pos, Path.Corners[Path.t_corners-1].pos  );
		float Scal[Path.t_corners];
		gvector Vec[Path.t_corners];
		vertex pV[Path.t_corners];
		
		// Get Vec
		for (int c = 0; c < Path.t_corners-1; c++) {
			path_corner &Corner = Path.Corners[c];
			path_corner &CornerN = Path.Corners[c+1];
			Vec[c] = GetVector(Corner.pos, CornerN.pos);
			Vec[c].z = 0;
			
			#if DEBUG > 0
			if(dev) cout << " section vectors #" << c << ": " << Vec[c] << endl;
			#endif
		}
		// Get Pos Vertex
		for (int c = 0; c < Path.t_corners; c++)
		{
			path_corner &Corner = Path.Corners[c];
			// get position vertex for this section
			pV[c] = Corner.pos;
			pV[c].y += posy;
			
			#if DEBUG > 0
			if(dev) cout << "     PosV sec#" << c << ": " << pV[c] << " = Corner ("<<Corner.pos<<") + posy("<<posy<<")" << endl;
			#endif
			
			// rotate position vertex relevant to the current section align
			float Yaw = 0.0;
			if (c==Path.t_corners-1)
			{
				#if DEBUG > 0
				if (dev)cout << "          LAST Corner!"<<endl;
				#endif
				
				Yaw = Path.Corners[c-1].Yaw;
				pV[c].rotateOrigin(0,0,Yaw,Corner.pos);
			}
			else
			{
				Yaw = Corner.Yaw;
				pV[c].rotateOrigin(0,0,Yaw,Corner.pos);
			}
			#if DEBUG > 0
			if(dev) cout << "     PosV sec#" << c << " now " <<pV[c]<<" after Z-Rot by " << Yaw << " deg around Point " << Corner.pos << endl;
			#endif
		}
		
		#if DEBUG > 0
		if(dev) {
			cout << " Final Position Vectors ("<<Path.t_corners<<")" << endl;
			for (int c = 0; c < Path.t_corners; c++) {
				vertex &PV = pV[c];
				cout << " pV #" << c+1 << "/" << Path.t_corners << " " <<PV << endl;
 			}
		}
		#endif

		// Get Section Scalars
		for (int c = 0; c < Path.t_corners-2; c++) // corner loop
		{
			Scal[c] = GetDot(  Normalize(Vec[c]), Normalize(Vec[c+1])  );
			
			#if DEBUG > 0
			if(dev) cout << " Scalar #" <<c << " " << Scal[c] << endl;
			#endif
		}
		
		#if DEBUG > 0
		if (dev) system("pause");
		#endif
		
		int L_pV = Path.t_corners-1;
		int L_sec = Path.t_corners-2;
		for (int c = 0; c < Path.t_corners-1; c++) // corner loop
		{
			#if DEBUG > 0
			if(dev) cout << " corner #" << c << " of " << Path.t_corners << " L_sec " << L_sec << " L_pV " << L_pV << endl;
			#endif
			
			path_corner &Corner = Path.Corners[c];
			path_corner &CornerN = Path.Corners[c+1];
			vertex &V1 = Vertices[v];
			vertex &V2 = Vertices[v+1];
			if (Path.t_corners>2)
			{
				#if DEBUG > 0
				if(dev)cout << " #### path has more than 2 corners #### " << endl;
				#endif
				
				if (c==0)
				{
					#if DEBUG > 0
					if (dev)cout << "  FIRST Corner!" << endl;
					#endif
					
					if (!IsCircle)
					{
						#if DEBUG > 0
						if(dev) cout << "   NO Circle!!!" << endl;
						#endif
						
						V1 = pV[0];
					}
					else
					{
						#if DEBUG > 0
						if(dev) cout << "   Circle!!!" << endl;
						#endif
						
						if (  GetDot(  Normalize(Vec[0]), Normalize(Vec[L_sec]  )  ) >0.9999  )
						{
							#if DEBUG > 0
							if(dev) cout << "     Previous Section has same Align!" << endl;
							#endif
							
							V1 = pV[0];
						}
						else
						{
							#if DEBUG > 0
							if(dev) cout << "     Previous Section Align differs!" << endl;
							#endif
							
							V1 = GetLineIsect(pV[0], pV[L_sec], Vec[0], Vec[L_sec]);
						}
					}
					if (Scal[0]<0.9999) // next sec align differs
					{
						V2 = GetLineIsect(pV[0], pV[1], Vec[0], Vec[1]);
						
						#if DEBUG > 0
						if(dev) cout << "     V2 is now Line Intersect of following: " << pV[0] << pV[1] << Vec[0] << Vec[1] << endl;
						#endif
					}
					else // next sec align is same
					{
						V2 = pV[1];
						
						#if DEBUG > 0
						if(dev) cout << "     V2 is now pV[1] (#2)" << pV[1] << endl;
						#endif
					}
				}
				else if (c==Path.t_corners-2)
				{
					#if DEBUG > 0
					if (dev)cout << "  LAST Corner!" << endl;
					#endif
					
					if (Scal[c-1]<0.9999) // align differs
					V1 = GetLineIsect(pV[c], pV[c-1], Vec[c], Vec[c-1]);
					else // same align
					V1 = pV[c];
					
					if (!IsCircle) // NO Circle
					{
						#if DEBUG > 0
						if(dev) cout << "   NO Circle!!!" << endl;
						#endif
						
						V2 = pV[L_pV];
					}
					else // Circle!
					{
						#if DEBUG > 0
						if(dev) cout << "   Circle!!!" << endl;
						#endif
						
						if (  GetDot(  Normalize(Vec[0]) , Normalize(Vec[c])  ) >0.9999  ) // next section (circle-start) has same align as current
						{
							#if DEBUG > 0
							if(dev) cout << "     Next Section same Align!" << endl;
							#endif
							
							V2 = pV[L_pV];
						}
						else // next section has different align as current
						{
							#if DEBUG > 0
							if(dev) cout << "     Next Section Align differs!" << endl;
							#endif
							
							V2 = GetLineIsect(pV[0], pV[c], Vec[0], Vec[c]);
						}
					}
				}
				else if (c>0&&c<Path.t_corners-2)
				{
					#if DEBUG > 0
					if (dev)cout << "  MIDDLE Corner!" << endl;
					#endif
					
					V1 = Vertices[v-1];
					if (Scal[c]<0.9999) // align differs
					{
						V2 = GetLineIsect(pV[c], pV[c+1], Vec[c], Vec[c+1]);
					}
					else // same align
					{
						V2 = pV[c+1];
					}
				}
			}
			else if (Path.t_corners==2)// path has exactly 2 corners
			{
				#if DEBUG > 0
				if(dev)cout << " #### path has exactly 2 corners #### " << endl;
				#endif
				
				V1 = pV[0];
				V2 = pV[1];
			}
			
			#if DEBUG > 0
			if (dev) system("pause");
			#endif
			
			V1.pID = p;
			V1.Align = Corner.Align;
			
			V1.z = height;
			V2.z = height;
			V1.Yaw = Corner.Yaw;
			
			V1.Align = Corner.Align;
			if(c>0)
			V1.IsCCW = !Path.Corners[c-1].NextIsCW;
			
			#if DEBUG > 0
			if (dev) cout << "     Final Vertices V1 " << V1 << " V2 " << V2 << endl << endl;
			#endif
			
			v+=2;
		}
	}
	
	#if DEBUG > 0
	if (dev) { cout << endl; system("pause"); }
	#endif
}

void circle::build_pathGrid(int g, float posy, float height, path_set &Set)
{
	tverts = ((Set.t_corners-Set.t_paths)*2)+(Set.Gaps*2);
	Vertices = new vertex[tverts];
	float Size = sGroup[g].SizeY; // mGroup->biggestY; // Y-size of the original source object // changed 09.05.2019
	
	for (int p = 0,v=0; p < Set.t_paths; p++) // path loop
	{
		path &Path = Set.Paths[p];
		
		for (int c = 0; c < Path.t_corners-1; c++) // corner loop
		{
			path_corner &Corner = Path.Corners[c];
			path_corner &CornerN = Path.Corners[c+1];
			vertex &V1 = Vertices[v];
			vertex &V2 = Vertices[v+1];
			
			V1.pID = p;
			V1.Align = Corner.Align;
			V1.z = height;
			V2.z = height;
			V1.y = posy;
			V2.y = posy;
			V1.step = Corner.step;
			V1.rotate(0,0,Corner.rot1);
			V2.rotate(0,0,Corner.rot2);
			V1.x += Corner.pos.x;
			V1.y += Corner.pos.y;
			V2.x += CornerN.pos.x;
			V2.y += CornerN.pos.y;
			V1.Yaw = Corner.Yaw;
			
			if (!Corner.NextIsCW) // Next Edge turns CCW (left) - move last vertex to prevent overlapping meshes
			{
				if (Set.cornerFix)
				{
					// 0 = UP, 1 = RIGHT, 2 = DOWN, 3 = LEFT
					if (Corner.NextIsGap90||Corner.NextIsGap180)
					{
						if 		(Corner.Align==0) {V2.y -= Size; }
						else if (Corner.Align==1) {V2.x -= Size; }
						else if (Corner.Align==2) {V2.y += Size; }
						else if (Corner.Align==3) {V2.x += Size; }
					}
				}
			}
			
			if (c>0&&!Path.Corners[c-1].NextIsCW)  // Previous Edge turns CCW (left) - move first vertex to prevent overlapping meshes
			{
				if (Set.cornerFix)
				{
					if (Path.Corners[c-1].NextIsGap90||Path.Corners[c-1].NextIsGap180)
					{
						if 		(Corner.Align==0) {V1.y -= Size;}
						else if (Corner.Align==1) {V1.x -= Size;}
						else if (Corner.Align==2) {V1.y += Size;}
						else if (Corner.Align==3) {V1.x += Size;}
					}
				}
			}
			
			if ((Corner.NextIsGap90||Corner.NextIsGap180) && (Set.cornerFix || (!Set.cornerFix&&Corner.NextIsCW)) )
			{
				int GapRot = 0;
				if (Corner.NextIsCW) 	GapRot = -90; //CW
				else 					GapRot = 90;  //CCW
				
				vertex Origin;
				if (Corner.NextIsCW) 	Origin = CornerN.pos; //CW
				else 					Origin = CornerN.pos; //CornerN_PosNew; //CCW
				
				vertex &V3 = Vertices[v+2];
				vertex &V4 = Vertices[v+3];
				V3 = V2;
				V4 = V2;
				V3.pID = p;
				V3.Align = Corner.Align;
				V3.IsGap = 1;
				V3.z = height;//+CornerN.pos.z;
				V4.z = height;//+CornerN.pos.z;
				V4.rotateOrigin(0,0,GapRot,Origin);
				V3.Yaw = Corner.rot1 + (GapRot/2);
				
				if (Corner.NextIsGap180)
				{
					vertex &V5 = Vertices[v+4];
					vertex &V6 = Vertices[v+5];
					V5 = V4;
					V6 = V4;
					V5.pID = p;
					V5.Align = Corner.Align;
					V5.IsGap = 1;
					V5.z = height;
					V6.z = height;
					V6.rotateOrigin(0,0,GapRot,Origin);
					V5.Yaw = Corner.rot2 + (GapRot/2);
					v+=2;
				}
				v+=2;
			}
			v+=2;
		}
	}
}


// CIRCLE CONSTRUCTOR
void circle::build_circleGrid(int res, float rad, float height)
{
	// Pattern Segment Multiplier (e.g.1,2,4) - Base for pattern length calculation
	int resmulti = res/12;
	int res_stepdown = reduce(res);

	float radmulti = rad/128.0;
	
	// Vertex base modifier (e.g.32,8,2) - base step size for vertex-coordinate modification
	// decreases with increasing resolution by factor 4
	// increases with radius
	float vbmod = 128.0 / (n_pow(4,res_stepdown)) * radmulti;
	
	// Pattern Segment Length (e.g. bis res 12: "4,2,4,2") - calculates the length of the 4 segments, based on the segment multiplier
	// the greater the resolution, the longer the segments
	int pslen[2][5] = {
		{1*resmulti,4*resmulti,2*resmulti,4*resmulti,1*resmulti}, 
		{2*resmulti,2*resmulti,4*resmulti,2*resmulti,2*resmulti}
	};
	
	// Pattern Segment Modifier (e.g. "-32,0,32,0") - 4 base values for the pattern
	float basepat[2][5] = {
		{0,-vbmod,0,vbmod,0},
		{-vbmod,0,vbmod,0,-vbmod}
	};
	
	// Filled Pattern (e.g. res12,rad128: "-32 -32 -32 -32 0 0 32 32 32 32 0 0")
	oldcircle steps;
	
	for (int axis = 0; axis < 2; axis++)
	{
		int min, max;
		
		for (int pos = 0; pos < res; pos++)
		{
			max = pslen[axis][0];
			
			if (pos < max) {
				steps.coords[axis][pos] = basepat[axis][0];
			}
			
			max = pslen[axis][0]+pslen[axis][1];
			min = pslen[axis][0];
			
			if ((pos < max) && (pos >= min)) {
				steps.coords[axis][pos] = basepat[axis][1];
			}
			
			max = pslen[axis][0]+pslen[axis][1]+pslen[axis][2];
			min = pslen[axis][0]+pslen[axis][1];
			
			if ((pos < max) && pos >= min) {
				steps.coords[axis][pos] = basepat[axis][2];
			}
			
			max = pslen[axis][0]+pslen[axis][1]+pslen[axis][2]+pslen[axis][3];
			min = pslen[axis][0]+pslen[axis][1]+pslen[axis][2];
			
			if ((pos < max) && (pos >= min)) {
				steps.coords[axis][pos] = basepat[axis][3];
			}
			
			min = pslen[axis][0]+pslen[axis][1]+pslen[axis][2]+pslen[axis][3];
			
			if (pos >= min) {
				steps.coords[axis][pos] = basepat[axis][4];
			}
		}
	}
	
	// Final Circle Coordinates (e.g. res12,rad128: "32 96 128 128 96 32 -32 -96 -128 -128 -96 -32")
	oldcircle final;
	
	// First Coordinate X/Y
	final.coords[0][0] = 64.0 / n_pow(2,res_stepdown-1) * radmulti;
	final.coords[1][0] = -32.0 / n_pow(4,res_stepdown-1) * radmulti;
	
	// Final Steps
	float lastfinal, laststep;
	
	for (int axis = 0; axis < 2; axis++)
	{
		for (int pos = 1; pos < res; pos++)
		{
			lastfinal = final.coords[axis][pos-1];
			laststep = steps.coords[axis][pos];
			
			final.coords[axis][pos] = lastfinal + laststep;
		}
	}
	
	// Final Final Circle Coordinates
	Vertices = new vertex[res+1];
	
	Vertices[0].x = 32.0*radmulti/resmulti;
	Vertices[0].y = rad;
	Vertices[0].z = height;
	
	for (int c = 1; c < res+1; c++)
	{
		Vertices[c].x = Vertices[c-1].x + final.coords[0][c-1];
		Vertices[c].y = Vertices[c-1].y + final.coords[1][c-1];
		Vertices[c].z = height;
	}
}

ostream &operator<<(ostream &ostr, circle &c)
{
	for(int i = 0; i<c.tverts;i++)
	{
		ostr << "    Vertex #"<<i<<" ( " << c.Vertices[i].x << " " << c.Vertices[i].y << " " << c.Vertices[i].z << " )" << " Gap " << c.Vertices[i].IsGap << endl;
	}
	return ostr;
}

ostream &operator<<(ostream &ostr, circleset &cs)
{
	ostr << " CircleSet with " << cs.tcircs << " total circles" << endl;
	for (int i=0;i<cs.tcircs; i++)
	{
		ostr << "  Circle #" << i << endl;
		ostr << cs.c[i];
		ostr << endl;
	}
	return ostr;
}








/* ===== SPLINE FILE RELATED FUNCTIONS ===== */



void ParseCornerFile(string pFile, path_set &PathList)
{
	#if DEBUG > 0
	bool dev = 0;
	#endif
	
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
		#if DEBUG > 0
		if(dev) cout << "count path_corner..." << endl;
		#endif
		
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
		
		#if DEBUG > 0
		if(dev) cout << "total path_corner: " << pcount << endl;
		#endif
		
		PathList.t_corners = pcount;
		
		#if DEBUG > 0
		if(dev) system("pause");
		
		// extract path_corner properties
		if(dev) cout << "extract path_corner properties..." << endl;
		#endif
		
		for (int i = 0, s=0,e=0,l=0; i<pcount; i++)
		{
			string str_x,str_y,str_z;
			int x_start,x_end,y_start,y_end,z_start,z_end,name_start,name_end,tar_start,tar_end;
			s = pFile.find(p_corner,l);
			e = pFile.find("}",s);
			
			#if DEBUG > 0
			if (dev) cout << endl << "+----- Current Path_Corner Entity Content ("<<s<<"-"<<e<<"):" << endl << pFile.substr(s, e-s) << endl;
			#endif
			
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
			if (name_start!=-1&&name_start<e&&name_start>s)
			pCorner[i].name = pFile.substr(name_start,name_end-name_start);
			else
			pCorner[i].name = "UNSET";

			#if DEBUG > 0
			if (dev) cout << "     [!] Found Name at " << name_start << " (began search from "<<s<<") end " << name_end << " content " << pCorner[i].name << endl;
			#endif
			
			tar_start = pFile.find(str_target, s)+10;
			tar_end = pFile.find(qmark, tar_start);
			if (tar_start!=-1&&tar_start<e&&tar_start>s)
			pCorner[i].target = pFile.substr(tar_start,tar_end-tar_start);
			else
			pCorner[i].target = "UNSET";

			#if DEBUG > 0
			if(dev) cout << "     [!] Found Target at " << tar_start << " (began search from "<<s<<") end " << tar_end << " content " << pCorner[i].target << endl;
			
			if(dev) cout << "     path "<<i<<" name [" << pCorner[i].name << "] target [" << pCorner[i].target << "] coords [" << pCorner[i].pos << "] " << endl;
			#endif
			
			l = e+1;
		}
		#if DEBUG > 0
		if(dev) system("pause");
		#endif
		
		// assign path ID to corners, that have a valid target corner
		#if DEBUG > 0
		if(dev) cout << "assign path ID to generated path_corners..." << endl;
		#endif
		
		int c_pID = 0;
		for (int i = 1; i<pcount; i++)
		{
			path_corner &C = pCorner[i];
			path_corner &L = pCorner[i-1];
			string C_name = C.name;
			//string C_target = C.target;
			string L_name = L.name;
			string L_target = L.target;
			
			if ( L_name==C_name || (L_target==C_name&&L_target!="UNSET") )
			{
				C.pID = L.pID;
			}
			else
			{
				c_pID++;
				C.pID = c_pID;
			}
			
			#if DEBUG > 0
			if(dev) cout << "    corner " << i << " name [" << pCorner[i].name << "] target [" << pCorner[i].target << "] ID " << pCorner[i].pID << endl;
			#endif
		}
		#if DEBUG > 0
		if(dev) system("pause");
		#endif
		
		int tpaths = c_pID+1;
		int vcount[tpaths]; for (int i = 0; i<tpaths; i++) vcount[i] = 0;
		
		// count vertices of each path ID
		#if DEBUG > 0
		if(dev) cout << "count vertices of each path ID..." << endl;
		#endif
		
		for (int i = 0; i<pcount; i++)
		{
			vcount[pCorner[i].pID]++;
			
			#if DEBUG > 0
			if(dev) cout << "    current vertex counter " << vcount[pCorner[i].pID] << endl;
			#endif
		}
		
		#if DEBUG > 0
		if(dev)
		for (int i = 0; i<c_pID+1; i++) {
			if(dev) cout << "    path " << i << " vertex amount " << vcount[i] << endl; }
		if(dev) system("pause");
		#endif
		
		// get rid of single knots
		bool Discarded = 0;
		
		#if DEBUG > 0
		if(dev) cout << "Getting rid of single knots..." << endl;
		#endif
		
		vector<path_corner> CleanCorners;
		for (int i = 0, pIDOff=0; i<pcount; i++)
		{
			#if DEBUG > 0
			if(dev) cout << "  Corner " << i << " name " << pCorner[i].name << " pID " << pCorner[i].pID << " has members " << vcount[pCorner[i].pID] << endl;
			#endif
			
			if (vcount[pCorner[i].pID]>1)
			{
				pCorner[i].pID -= pIDOff;
				CleanCorners.push_back(pCorner[i]);
				
				#if DEBUG > 0
				if(dev) cout << "    Included! decreased pID by "<<pIDOff<<" to " << pCorner[i].pID << endl;
				#endif
			}
			else
			{
				Discarded=1;
				pIDOff++;
				tpaths--;
				
				#if DEBUG > 0
				if(dev) cout << "    Discarded! pIDOff now " << pIDOff << " tpaths now " << tpaths << "/" <<c_pID+1 << endl;
				#endif
			}
		}
		
		if (CleanCorners.size()>0)
		{
			if (Discarded) {
				cout << "|    [WARNING] Spline contains single knots which had been discarded!" << endl;
				//tpaths = 0;
				// fix pIDs in case of deleted knots
				/*for (int p=0, n_pID=0; p<c_pID+1; p++) {
					for (int c=0; c<CleanCorners.size(); c++) {
						path_corner &C = CleanCorners[c];
						bool found=0;
						if (C.pID==p) { C.pID=n_pID; found=1; }
						
						if (c==CleanCorners.size()-1&&found) { n_pID++; tpaths++; }
					}
				}*/
				for (int i=0; i<tpaths; i++) vcount[i]=0;
				
				// count total corners per path again
				for (int i = 0; i<CleanCorners.size(); i++)
					vcount[CleanCorners[i].pID]++;
				
				#if DEBUG > 0
				if(dev)
				for (int i = 0; i<tpaths; i++) {
					if(dev) cout << "    path " << i << "/"<<tpaths<<" knots " << vcount[i] << endl; }
				#endif
			}
			
			// finally fill the path corners into the files circleset
			#if DEBUG > 0
			if(dev) cout << "Filling cleaned path corners into files circleset..." << endl;
			#endif
			
			PathList.t_paths = tpaths;
			PathList.Paths = new path[tpaths];
			for (int p = 0, n=0; p<PathList.t_paths; p++) // circle loop
			{
				path &Path = PathList.Paths[p];
				
				#if DEBUG > 0
				if (dev) cout << " Path " << p << "/"<<tpaths<<" Corner " << n << "/"<<CleanCorners.size() <<" pID " << pCorner[n].pID << endl;
				#endif
				
				Path.t_corners = vcount[p];
				Path.Corners = new path_corner[vcount[p]];
				for (int v = 0; v<Path.t_corners; v++)
				{
					Path.Corners[v] = CleanCorners[n];
					//Path.Corners[v].pos.z = 0;
					n++;
				}
			}
			
			#if DEBUG > 0
			if (dev) {
				cout << endl << " Final Spline List:" << endl << endl;
				for (int p = 0; p<PathList.t_paths; p++) // circle loop
				{
					path &Path = PathList.Paths[p];
					cout << "  Path #" << p+1 << "/"<<PathList.t_paths<<" Knots " << Path.t_corners << " Length " << Path.t_length << endl;
					for (int c = 0; c<Path.t_corners; c++)
					{
						path_corner &C = Path.Corners[c];
						cout << "   Knot #" << c+1 << "/"<<Path.t_corners<<" Pos " << C.pos << " Name " << C.name << " pID " << C.pID << endl;
					}
				}
			}
			#endif
			
			PathList.valid = 1;
		}
		else
		{
			cout << "|    [ERROR] Spline only contains single knots!" << endl;
			PathList.valid = 0;
		}
		
		#if DEBUG > 0
		if(dev) system("pause");
		#endif
	}
	else PathList.valid = 0;
	
	#if DEBUG > 0
	if (dev) system("pause");
	#endif
}









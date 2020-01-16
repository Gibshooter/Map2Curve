#include "frames.h"
#include "vertex.h"
#include "utils.h"
#include "group.h"
#include "settings.h"

#include <iostream>
#include <conio.h>

using namespace std;

extern group *sGroup;
extern group *mGroup;
extern ctable *cTable;
extern group *bGroup;

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
	bool dev = 0;
	if(n!=0)
	if(dev) cout << " Expanding Spline by "<<n<<" units ..." << endl;
	path_corner Back[t_corners];
	for (int i=0;i<t_corners;i++)
		Back[i] = Corners[i];
	
	bool IsCircle = 0;
	if (  CompareVertices( Corners[0].pos, Corners[t_corners-1].pos )  ) IsCircle = 1;
	
	for (int i=0;i<t_corners-1;i++)
	{
		path_corner &K  = Corners[i];
		if(dev) cout << "   Knot " << i << "/" << t_corners << endl;
		if(i==0)
		{
			if(!IsCircle)
			{
				gvector zVec(0,0,1);
				gvector nCross = Normalize(  GetCross( GetVector(Back[1].pos,Back[0].pos), zVec )  );
				nCross.mult(n); nCross.z = 0; nCross.flip();
				Corners[0].pos.Add(nCross);
				if(dev) cout << "     First Knot! Old Pos " << Back[0].pos << " Adding " << nCross << " = " << Corners[0].pos << endl;
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
			if(dev) cout << "     Old Pos " << Back[i].pos << " Adding " << nVec << " = " << Corners[i].pos << endl;
			
			if (i==t_corners-2)
			{
				if(!IsCircle)
				{
					gvector zVec(0,0,1);
					gvector nCross = Normalize(  GetCross( GetVector(Back[i+1].pos, Back[i].pos), zVec )  );
					nCross.mult(n); nCross.z = 0; nCross.flip();
					Corners[i+1].pos.Add(nCross);
					if(dev) cout << "     Last Knot! Old Pos " << Back[i+1].pos << " Adding " << nCross << " = " << Corners[i+1].pos << endl;
				}
				else
				{
					/*gvector Vec1 = GetVector(Back[i].pos, Back[i+1].pos);
					gvector Vec2 = GetVector(Back[1].pos, Back[0].pos);
					gvector nVec = Normalize(  VecAdd(  Vec1, Vec2  )  );
					nVec.mult(n); nVec.z = 0;
					Corners[i+1].pos.Add(nVec);*/
					Corners[i+1].pos = Corners[0].pos;
				}
			}
		}
	}
	if(dev) getch();
}

void path::EvenOut()
{
	bool dev = 0;
	// 1. Get shortest Section Length
	if (dev) cout << " 1. Getting shortest Section Length... " << endl;
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
	if (dev) cout << "   Shortest Sec " << sLen << "(*1.2="<<sLen*1.2<<") Limit " << max << endl;
	sLen *= 1.2; // make it a little longer maybe
	
	vector<path_corner> NewKnots;
	if (dev) cout << " 2. Beginning to create new knots... " << endl;
	// 2. Begin to create new knots for sections that are at least 1.75 times bigger than the shortest(*1.2)
	for (int k=0; k<t_corners-1; k++)
	{
		path_corner &Knot = Corners[k];
		if (dev) cout << "   Old Knot #" << k << " Sec Length " << SecLen[k] << " Vec " << SecVec[k] << endl;
		
		NewKnots.push_back(Knot);
		if(SecLen[k]>max) // section length greater than the limit
		{
			int subs = round( SecLen[k] / sLen ); // how many subdivisions?
			float subLen = SecLen[k] / subs;
			if (dev) cout << "     !!! Section length longer than the limit (" << max << ")" << endl;
			if (dev) cout << "     Creating " << subs-1 << " subdivisions for this section with length " << subLen << endl;
			for(int s=0; s<subs-1; s++) // create new knots for this section based on the subdiv length
			{
				path_corner sub;
				sub.pos = Knot.pos;
				gvector addVec = Normalize(SecVec[k]);
				addVec.mult((s+1)*subLen);
				sub.pos.Add(addVec);
				NewKnots.push_back(sub);
				if (dev) cout << "      New Knot #" << s << " pos " << sub.pos << " addVec " << addVec << endl;
			}
		}
		//else NewKnots.push_back(Knot);
	}
	// add last knot, or else it won't get a chance :(
	NewKnots.push_back(Corners[t_corners-1]);
	
	// 3. create path of new knots
	if (dev) cout << " 3. Creating path of new knots... " << endl;
	path_corner *nKnotPtr = new path_corner[NewKnots.size()];
	for (int k=0; k<NewKnots.size(); k++)
	{
		path_corner &nKnot = NewKnots[k];
		path_corner &pKnot = nKnotPtr[k];
		pKnot = nKnot;
		if (dev) cout << "   #"<<k<<" of New Knot Array " << pKnot.pos << " made from new knot " << nKnot.pos << endl;
	}
	
	// 4. replace old knots
	if (dev) cout << " 4. Replacing old knots... " << endl;
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
			//cout << " dimensions taken from v" << v << V << " : " << D.xs << ", " << D.xb << ", " << D.ys << ", " << D.yb << ", " << D.zs << ", " << D.zb << endl;
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

void path_set::Analyze()
{
	bool dev = 0;
	path_set &List = *this;
	
	/*bool Recreate = 0;
	int valid_paths = List.t_paths;
	// Get rid of paths that only have one knot
	for (int p = 0; p<List.t_paths; p++) // path loop
	{
		path &Path = List.Paths[p];
		if (Path.t_corners<=1) {
			Path.valid=0;
			Recreate=1;
			valid_paths--;
		}
	}
	if (Recreate&&valid_paths>0)
	{
		cout << "|    [WARNING] Spline contains single knots which had been discarded!" << endl;
		path* CleanPaths = new path[valid_paths];
		for (int po=0, pn=0; po<List.t_paths; po++) // path loop
		{
			path &PathOld = Paths[po];
			if (PathOld.valid)
			{
				path &PathNew = CleanPaths[pn];
				PathNew.Copy(PathOld);
				pn++;
			}
		}
		delete[] Paths;
		Paths = CleanPaths; // replace old path pointer with new path pointer
		t_paths = valid_paths;
	} else if (valid_paths<=0) { 
		valid = 0;
		cout << "|    [ERROR] Spline only contains paths made of single knots!" << endl;
	}*/
	
	for (int p = 0; p<List.t_paths; p++) // path loop
	{
		path &Path = List.Paths[p];
		//cout << " Old corners: " << *this << endl;
		if (cTable[gID].p_evenout>0) {
			Path.EvenOut();
			t_corners=0;
			for (int p=0;p<t_paths;p++) t_corners += Paths[p].t_corners;
		}
		//cout << " new corners: " << *this << endl;
	}
	
	//cout << " Analyzing Path Set with " << List.t_paths<< " paths in it... " << endl;
	for (int p = 0; p<List.t_paths; p++) // path loop
	{
		path &Path = List.Paths[p];
		// determine starting direction of this path (not needed ATM)
		//float StartAngle = GetVecAlign( GetVector(Path.Corners[0].pos, Path.Corners[1].pos) ,0);
		//if ( StartAngle>90&&StartAngle<270) {Path.direct = 0;} //cout << " Path direction is backwards!" << endl;}
		//else cout << " Path direction is forwards! " << endl;
		
		if (cTable[gID].p_expand!=0) Path.Expand(cTable[gID].p_expand);
		if (List.preverse) {Path.reverse();}
		
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
				Corner1.Pitch = GetVecAlign(Edge, 1);
				float EdgeLen = GetVecLen(Edge);
				Path.t_length+=EdgeLen;
				if (dev) cout << " Path " << p<< " length " << EdgeLen << endl;
				if (EdgeLen==0||!IsValid(EdgeLen)) {
					valid=0;
					cout << "|    [ERROR] Spline contains invalid knots! Path #"<<p<< ", Knot #" << c << Corner1.pos << ", section has invalid length: " << EdgeLen << endl;
					break;
				}
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
			if (!valid) break;
		}
		t_length+=Path.t_length;
		if (dev) cout << "   PathSet total length " << t_length << endl;
		if (dev) getch();
		
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
		//cout << "Fix cutting edge rotations and mark gaps..." << endl;
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





/* ===== CIRCLE METHODS & FUNCTIONS ===== */

void circle::AddHeight(int g, vector<path_set> &Set)
{
	bool dev = 0;
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
		if(dev) cout << " Spline sec " << sec << " Height P1: " << V.z << " P2 " << VN.z << endl;
	}
}

void circle::ConvertToSpline(int g)
{
	bool dev = 0;
	int res = cTable[g].res;
	if(cTable[g].type==0||cTable[g].type==1)
	{
		if (dev) cout << " Converting Circle object (res: "<<tverts<<") to Spline (new res: "<< cTable[g].res*2 <<")..." << endl;
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
		if(dev) getch();
	}
}

void circle::GetInVec(int g)
{
	bool dev = 0;
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
			
			if(dev) cout << " #" <<sec<< " V1 " <<  V << endl << " V2 " << VN << endl << " V0 " << VL << endl << " Vec1 " << Vec << endl << " Vec2 " << Vec2 << " InVec " << InVec[sec] <<endl << endl;
			if(dev) getch();
			
		} else {
			InVec[sec] = Vec;
			InVec[sec].rotate(0,0,90);
		}
		v+=2;
	}
}

void circle::GetAngles(int g)
{
	bool dev = 0;
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
	
	bool dev = 0;
	Vertices = new vertex[res+1];
	//tverts = res+1;
	
	//cout << "Kreis Koordinaten mit PI konstruieren..." << endl;
	//cout << "Steps (360/" << res << ") = " << 360.0/res << endl;
	float steps = 360.0/res;
	float step_half = 0; if(flat) step_half = steps/2;
	float alpha;
	
	for (int i = (res/4), j, k=0; i < res+(res/4)+1; i++) {
		
		alpha = (( (i * steps)+step_half ) *PI/180.0);
		//cout << "i[" << i << "] step: " << i * steps << ", \tAlpha = " << alpha << endl;
		if (k==0) j = res; else j = k-1;
		Vertices[j].x = rad*(cos(alpha));
		Vertices[j].y = rad*(sin(alpha));
		Vertices[j].z = height;
		k++;
		/*if (roundV) {
		Vertices[j].x = round(Vertices[j].x);
		Vertices[j].y = round(Vertices[j].y);
		}*/
		//if (Vertices[i].x<0.4&&Vertices[i].x>-0.4) Vertices[i].x = round(Vertices[i].x);
		//if (Vertices[i].y<0.4&&Vertices[i].y>-0.4) Vertices[i].y = round(Vertices[i].y);
		//cout << "\t - X (" << Vertices[j].x << ")\t rad["<<rad<<"]*(cos(alpha)["<<cos(alpha)<<"])\t | Y (" << Vertices[j].y << ")\t rad["<<rad<<"]\t*(sin(alpha)["<<sin(alpha)<<"])" << endl;
	}
	
	if(flat&&step_half!=0)
	{
		// size difference between normal and "flat" circle
		float rad_flat = rad * cos(step_half*PI/180.0);
		float m = rad / rad_flat;
		if (dev) cout << " rad " << rad << " rad_flat " << rad_flat << " m " << m << endl;
		
		// if starting angle > 0 scale circle to original radius again
		if(m!=0&&IsValid(m))
		for (int i = 0; i<res+1; i++) {
			if (dev) cout << " v " << i << Vertices[i];
			Vertices[i].x *= m;
			Vertices[i].y *= m;
			if (dev) cout << " new " << Vertices[i] << endl;
		}
	}
		
	reverse(res);
	//for (int i = 0; i<res+1; i++)
	//	cout << " Vertices["<<i<<"]" << Vertices[i] << endl;
}

void circle::build_pathIntersect(int g, float posy, float height, path_set &Set)
{
	bool dev = 0;
	tverts = (Set.t_corners-Set.t_paths)*2;
	
	if(dev)cout << " total vertices of this vertex-path: " << tverts << " (total corners "<<Set.t_corners-Set.t_paths<<" *2 )" << endl;
	Vertices = new vertex[tverts];
	float Size = sGroup[g].SizeY;
	
	// get intersection points between sections (except first and last sec)
	if(dev) cout << " Getting intersection points between sections... " << endl;
	bool SHOW_LINES = 0;
	canvas *Canva;
	if (SHOW_LINES) Canva = new canvas(300, 50);
	
	for (int p=0, v=0,sec=0; p < Set.t_paths; p++)
	{
		path &Path = Set.Paths[p];
		if(dev) cout << " path loop #" << p+1 << " of " << Set.t_paths<< endl;
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
			if(dev) cout << " section vectors #" << c << ": " << Vec[c] << endl;
		}
		// Get Pos Vertex
		for (int c = 0; c < Path.t_corners; c++) {
			path_corner &Corner = Path.Corners[c];
			// get position vertex for this section
			pV[c] = Corner.pos;
			pV[c].y += posy;
			if(dev) cout << "     PosV sec#" << c << ": " << pV[c] << " = Corner ("<<Corner.pos<<") + posy("<<posy<<")" << endl;
			
			// rotate position vertex relevant to the current section align
			float Yaw = 0.0;
			if (c==Path.t_corners-1) {
				if (dev)cout << "          LAST Corner!"<<endl;
				Yaw = Path.Corners[c-1].Yaw;
				pV[c].rotateOrigin(0,0,Yaw,Corner.pos);
			} else {
				Yaw = Corner.Yaw;
				pV[c].rotateOrigin(0,0,Yaw,Corner.pos);
			}
			if(dev) cout << "     PosV sec#" << c << " now " <<pV[c]<<" after Z-Rot by " << Yaw << " deg around Point " << Corner.pos << endl;
		}
		if(dev) {
			cout << " Final Position Vectors ("<<Path.t_corners<<")" << endl;
			for (int c = 0; c < Path.t_corners; c++) {
				vertex &PV = pV[c];
				cout << " pV #" << c+1 << "/" << Path.t_corners << " " <<PV << endl;
 			}
		}

		// Get Section Scalars
		for (int c = 0; c < Path.t_corners-2; c++) // corner loop
		{
			Scal[c] = GetDot(  Normalize(Vec[c]), Normalize(Vec[c+1])  );
			if(dev) cout << " Scalar #" <<c << " " << Scal[c] << endl;
		}
		
		if (dev) getch();
		int L_pV = Path.t_corners-1;
		int L_sec = Path.t_corners-2;
		for (int c = 0; c < Path.t_corners-1; c++) // corner loop
		{
			if(dev) cout << " corner #" << c << " of " << Path.t_corners << " L_sec " << L_sec << " L_pV " << L_pV << endl;
			path_corner &Corner = Path.Corners[c];
			path_corner &CornerN = Path.Corners[c+1];
			vertex &V1 = Vertices[v];
			vertex &V2 = Vertices[v+1];
			if (Path.t_corners>2)
			{
				if(dev)cout << " #### path has more than 2 corners #### " << endl;
				if (c==0)
				{
					if (dev)cout << "  FIRST Corner!" << endl;
					
					if (!IsCircle) {
						if(dev) cout << "   NO Circle!!!" << endl;
						V1 = pV[0];
					}
					else {
						if(dev) cout << "   Circle!!!" << endl;
						if (  GetDot(  Normalize(Vec[0]), Normalize(Vec[L_sec]  )  ) >0.9999  ) {
							if(dev) cout << "     Previous Section has same Align!" << endl;
							V1 = pV[0];
						} else {
							if(dev) cout << "     Previous Section Align differs!" << endl;
							V1 = GetLineIsect(pV[0], pV[L_sec], Vec[0], Vec[L_sec]);
						}
					}
					if (Scal[0]<0.9999) { // next sec align differs
						V2 = GetLineIsect(pV[0], pV[1], Vec[0], Vec[1]);
						if(dev) cout << "     V2 is now Line Intersect of following: " << pV[0] << pV[1] << Vec[0] << Vec[1] << endl;
					} else {// next sec align is same
						V2 = pV[1];
						if(dev) cout << "     V2 is now pV[1] (#2)" << pV[1] << endl;
					}
				}
				else if (c==Path.t_corners-2)
				{
					if (dev)cout << "  LAST Corner!" << endl;
					
					if (Scal[c-1]<0.9999) // align differs
					V1 = GetLineIsect(pV[c], pV[c-1], Vec[c], Vec[c-1]);
					else // same align
					V1 = pV[c];
					
					if (!IsCircle) { // NO Circle
						if(dev) cout << "   NO Circle!!!" << endl;
						V2 = pV[L_pV];
					} else { // Circle!
						if(dev) cout << "   Circle!!!" << endl;
						if (  GetDot(  Normalize(Vec[0]) , Normalize(Vec[c])  ) >0.9999  ) { // next section (circle-start) has same align as current
							if(dev) cout << "     Next Section same Align!" << endl;
							V2 = pV[L_pV];
						} else { // next section has different align as current
							if(dev) cout << "     Next Section Align differs!" << endl;
							V2 = GetLineIsect(pV[0], pV[c], Vec[0], Vec[c]);
						}
					}
				}
				else if (c>0&&c<Path.t_corners-2)
				{
					if (dev)cout << "  MIDDLE Corner!" << endl;
					
					V1 = Vertices[v-1];
					if (Scal[c]<0.9999) { // align differs
						V2 = GetLineIsect(pV[c], pV[c+1], Vec[c], Vec[c+1]);
					} else { // same align
						V2 = pV[c+1];
					}
				}
			}
			else if (Path.t_corners==2)// path has exactly 2 corners
			{
				if(dev)cout << " #### path has exactly 2 corners #### " << endl;
				V1 = pV[0];
				V2 = pV[1];
			}
			/*else // path has 1 corner
			{
				if(dev)cout << " #### path has exactly 1 corner #### " << endl;
				V1 = pV[0];
				vertex VOrigin = Corner.pos;
				gvector VecY = GetVector(VOrigin, V1);
				gvector VecZ; VecZ.z+=1;
				gvector Cross = Normalize(GetCross(VecY, VecZ));
				Cross.mult(16);
				V2 = pV[0]; V2.Add(Cross);
			}*/
			if (dev) getch();
			if(SHOW_LINES) { Canva->CreateLine(0.05, V1, V2); Canva->Print(); }
			V1.pID = p;
			V1.Align = Corner.Align;
			
			//V1.step = Corner.step; //!!!!!!!!!!!!????????????
			V1.z = height;
			V2.z = height;
			V1.Yaw = Corner.Yaw;
			//V1.Pitch = Corner.Pitch;
			V1.Align = Corner.Align;
			if(c>0)
			V1.IsCCW = !Path.Corners[c-1].NextIsCW;
			//if (Corner.pos.z!=CornerN.pos.z) V1.DoTri = 1; //!!!!!!!!!!!!????????????
			if (dev) cout << "     Final Vertices V1 " << V1 << " V2 " << V2 << endl << endl;
			
			v+=2;
		}
	}
	if(SHOW_LINES) delete Canva;
	if (dev) { cout << endl; getch(); }
}

void circle::build_pathGrid(int g, float posy, float height, path_set &Set)
{
	tverts = ((Set.t_corners-Set.t_paths)*2)+(Set.Gaps*2);
	//cout << " total vertices of this path mesh: " << tverts << " (total corners "<<Set.t_corners-Set.t_paths<<" *2 + Gaps "<<Set.Gaps<<" *2 )" << endl;
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
			V1.z = height;//+Corner.pos.z;
			V2.z = height;//+Corner.pos.z;
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
			//V1.Pitch = Corner.Pitch;
			
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
				/*else
				{
					//if (Corner.NextIsGap90||Corner.NextIsGap180)
					{
						if 		(Corner.Align==0) {V2.y -= Size; }
						else if (Corner.Align==1) {V2.x -= Size; }
						else if (Corner.Align==2) {V2.y += Size; }
						else if (Corner.Align==3) {V2.x += Size; }
					}
				}*/
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
				/*else
				{
					//if (Corner.NextIsGap90||Corner.NextIsGap180)
					{
						if 		(Corner.Align==0) {V1.y -= Size;}
						else if (Corner.Align==1) {V1.x -= Size;}
						else if (Corner.Align==2) {V1.y += Size;}
						else if (Corner.Align==3) {V1.x += Size;}
					}
				}*/
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
				//V3.Pitch = 0;
				
				if (Corner.NextIsGap180)
				{
					vertex &V5 = Vertices[v+4];
					vertex &V6 = Vertices[v+5];
					V5 = V4;
					V6 = V4;
					V5.pID = p;
					V5.Align = Corner.Align;
					V5.IsGap = 1;
					V5.z = height;//+CornerN.pos.z;
					V6.z = height;//+CornerN.pos.z;
					V6.rotateOrigin(0,0,GapRot,Origin);
					V5.Yaw = Corner.rot2 + (GapRot/2);
					//V5.Pitch = 0;
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
	//Vertices = new vertex[res];

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
	
	for (int axis = 0; axis < 2; axis++) {
		
		int min, max;
		
		for (int pos = 0; pos < res; pos++) {
			
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
		
		//cout << "Final Circle Coordinate: " << Vertices[c] << endl;
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
			if (dev) cout << endl << "+----- Current Path_Corner Entity Content ("<<s<<"-"<<e<<"):" << endl << pFile.substr(s, e-s) << endl;
			
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
			if (dev) cout << "     [!] Found Name at " << name_start << " (began search from "<<s<<") end " << name_end << " content " << pCorner[i].name << endl;
			
			tar_start = pFile.find(str_target, s)+10;
			tar_end = pFile.find(qmark, tar_start);
			if (tar_start!=-1&&tar_start<e&&tar_start>s)
			pCorner[i].target = pFile.substr(tar_start,tar_end-tar_start);
			else
			pCorner[i].target = "UNSET";
			if (dev) cout << "     [!] Found Target at " << tar_start << " (began search from "<<s<<") end " << tar_end << " content " << pCorner[i].target << endl;
			
			if(dev) cout << "     path "<<i<<" name [" << pCorner[i].name << "] target [" << pCorner[i].target << "] coords [" << pCorner[i].pos << "] " << endl;
			
			l = e+1;
		}
		if(dev) getch();
		
		// assign path ID to corners, that have a valid target corner
		if(dev) cout << "assign path ID to generated path_corners..." << endl;
		int c_pID = 0;
		for (int i = 1; i<pcount; i++)
		{
			path_corner &C = pCorner[i];
			path_corner &L = pCorner[i-1];
			string C_name = C.name;
			//string C_target = C.target;
			string L_name = L.name;
			string L_target = L.target;
			
			if ( L_name==C_name || (L_target==C_name&&L_target!="UNSET") ) {
				C.pID = L.pID;
			} else {
				c_pID++;
				C.pID = c_pID;
			}
			if(dev) cout << "    corner " << i << " name [" << pCorner[i].name << "] target [" << pCorner[i].target << "] ID " << pCorner[i].pID << endl;
		}
		if(dev) getch();
		
		
		// assign path ID to paths, if there is more than one path (first pathname without counter (01) identifies a new path)
		/*if(dev) cout << "assign path ID to path_corners..." << endl;
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
			if(dev) cout << "    corner " << i << " name " << pCorner[i].name << " ID " << pCorner[i].pID << endl;
		}
		if(dev) getch();
		*/
		
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
		
		// get rid of single knots
		bool Discarded = 0;
		if(dev) cout << "Getting rid of single knots..." << endl;
		vector<path_corner> CleanCorners;
		for (int i = 0, pIDOff=0; i<pcount; i++)
		{
			if(dev) cout << "  Corner " << i << " name " << pCorner[i].name << " pID " << pCorner[i].pID << " has members " << vcount[pCorner[i].pID] << endl;
			if (vcount[pCorner[i].pID]>1) {
				pCorner[i].pID -= pIDOff;
				CleanCorners.push_back(pCorner[i]);
				if(dev) cout << "    Included! decreased pID by "<<pIDOff<<" to " << pCorner[i].pID << endl;
			}
			else { Discarded=1; pIDOff++; tpaths--; if(dev) cout << "    Discarded! pIDOff now " << pIDOff << " tpaths now " << tpaths << "/" <<c_pID+1 << endl; }
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
				
				if(dev)
				for (int i = 0; i<tpaths; i++) {
					if(dev) cout << "    path " << i << "/"<<tpaths<<" knots " << vcount[i] << endl; }
			}
			
			// finally fill the path corners into the files circleset
			if(dev) cout << "Filling cleaned path corners into files circleset..." << endl;
			PathList.t_paths = tpaths;
			PathList.Paths = new path[tpaths];
			for (int p = 0, n=0; p<PathList.t_paths; p++) // circle loop
			{
				path &Path = PathList.Paths[p];
				if (dev) cout << " Path " << p << "/"<<tpaths<<" Corner " << n << "/"<<CleanCorners.size() <<" pID " << pCorner[n].pID << endl;
				
				Path.t_corners = vcount[p];
				Path.Corners = new path_corner[vcount[p]];
				for (int v = 0; v<Path.t_corners; v++)
				{
					Path.Corners[v] = CleanCorners[n];
					//Path.Corners[v].pos.z = 0;
					n++;
				}
			}
			
			// get height of all corners and create a height table from it
			/*for (int s = 0, h=0; s<PathList.t_paths; s++) // path loop
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
			}*/
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
			
			
			PathList.valid = 1;
		}
		else
		{
			cout << "|    [ERROR] Spline only contains single knots!" << endl;
			PathList.valid = 0;
		}
		
		if(dev) getch();
	}
	else PathList.valid = 0;
	
	if (dev) getch();
}









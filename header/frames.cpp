#include "frames.h"
#include "vertex.h"
#include "utils.h"
#include "group.h"

#include <iostream>

using namespace std;

extern group *sGroup;

/* ===== CIRCLE METHODS & FUNCTIONS ===== */

void circle::reverse(int res) {
	for (int i = 0; i<res/2; i++)
		swap(Vertices[i],Vertices[res-1-i]);
}

void circle::build_circlePi(int res, float rad, float height) {
	
	Vertices = new vertex[res+1];
	//tverts = res+1;
	
	//cout << "Kreis Koordinaten mit PI konstruieren..." << endl;
	//cout << "Steps (360/" << res << ") = " << 360.0/res << endl;
	float steps = 360.0/res;
	float alpha;
	
	for (int i = (res/4), j, k=0; i < res+(res/4)+1; i++) {
		
		alpha = ((i * steps)*PI/180.0);
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
	
	reverse(res);
	//for (int i = 0; i<res+1; i++)
	//	cout << " Vertices["<<i<<"]" << Vertices[i] << endl;
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
			V1.z = height+Corner.pos.z;
			V2.z = height+Corner.pos.z;
			V1.y = posy;
			V2.y = posy;
			V1.step = Corner.step;
			V1.rotate(0,0,Corner.rot1);
			V2.rotate(0,0,Corner.rot2);
			V1.x += Corner.pos.x;
			V1.y += Corner.pos.y;
			V2.x += CornerN.pos.x;
			V2.y += CornerN.pos.y;
			
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
				V3.z = height+CornerN.pos.z;
				V4.z = height+CornerN.pos.z;
				V4.rotateOrigin(0,0,GapRot,Origin);
				
				if (Corner.NextIsGap180)
				{
					vertex &V5 = Vertices[v+4];
					vertex &V6 = Vertices[v+5];
					V5 = V4;
					V6 = V4;
					V5.pID = p;
					V5.Align = Corner.Align;
					V5.z = height+CornerN.pos.z;
					V6.z = height+CornerN.pos.z;
					V6.rotateOrigin(0,0,GapRot,Origin);
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
	string str;
	for(int i = 0; i<c.tverts;i++)
		ostr << "( " << c.Vertices[i].x << " " << c.Vertices[i].y << " " << c.Vertices[i].z << " )\n";
	ostr << str;
	return ostr;
}


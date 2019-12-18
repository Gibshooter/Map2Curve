#include "vertex.h"
#include "gvector.h"
#include "utils.h"
#include <math.h>
#include <iostream>

#define PI 3.14159265

using namespace std;

struct gvector;

/* ===== VERTEX METHODS ===== */

void vertex::set(float n)
{
	x = n;
	y = n;
	z = n;
}

void vertex::setall(float a, float b, float c)
{
	x = a;
	y = b;
	z = c;
}

void vertex::operator=(gvector Vec)
{
	x = Vec.x;
	y = Vec.y;
	z = Vec.z;
}

void vertex::Add(gvector Vec)
{
	x += Vec.x;
	y += Vec.y;
	z += Vec.z;
}

void vertex::Add(vertex V)
{
	x += V.x;
	y += V.y;
	z += V.z;
}

void vertex::scale(float n) 
{
	if (n!=0) {
		x*=n;
		y*=n;
		z*=n;
	}
}

void vertex::rotate(float degx, float degy, float degz)
{
	if(degx!=0||degy!=0||degz!=0)
	{
		// degree to rad
		float radx = degx * PI / 180.0;
		float rady = degy * PI / 180.0;
		float radz = degz * PI / 180.0;
		//cout << "rad [" << rad << "] deg (" << deg << ") ";
		
		if (degx!=0)
		{
			float newY = (y * cos(radx)) - (z * sin(radx));
			float newZ = (y * sin(radx)) + (z * cos(radx));
			y = newY;
			z = newZ;
		}
	
		if (degy!=0)
		{
			float newX = (x * cos(rady)) + (z * sin(rady));
			float newZ = (x * -sin(rady)) + (z * cos(rady));
			x = newX;
			z = newZ;
		}
	
		if (degz!=0)
		{
			float newX = (x * cos(radz)) - (y * sin(radz));
			float newY = (x * sin(radz)) + (y * cos(radz));
			x = newX;
			y = newY;
		}
		//cout << "X [" << newX << "]\t (" << x << ")\t Y [" << newY << "]\t (" << y << ")" << endl;
	}
}

void vertex::rotateOrigin(float degx, float degy, float degz, vertex orig)
{
	bool dev = 0;
	if(dev) cout << " Rotating Vertex " << *this << "..." <<endl;
	if(degx!=0||degy!=0||degz!=0)
	{
		// subtract Origin first
		x -= orig.x;
		y -= orig.y;
		z -= orig.z;
		
		// degree to rad
		float radx = degx * PI / 180.0;
		float rady = degy * PI / 180.0;
		float radz = degz * PI / 180.0;
		
		if (degx!=0)
		{
			float newY = (y * cos(radx)) - (z * sin(radx));
			float newZ = (y * sin(radx)) + (z * cos(radx));
			y = newY;
			z = newZ;
		}
		if (degy!=0)
		{
			float newX = (x * cos(rady)) + (z * sin(rady));
			float newZ = (x * -sin(rady)) + (z * cos(rady));
			x = newX;
			z = newZ;
		}
		if (degz!=0)
		{
			float newX = (x * cos(radz)) - (y * sin(radz));
			float newY = (x * sin(radz)) + (y * cos(radz));
			x = newX;
			y = newY;
		}
		
		if(dev) cout << "   New Vertex " << *this << endl;
		x += orig.x;
		y += orig.y;
		z += orig.z;
	}
}

void vertex::move(float moveX, float moveY, float moveZ)
{
	if(moveX!=0) x += moveX;
	if(moveY!=0) y += moveY;
	if(moveZ!=0) z += moveZ;
}

vertex::vertex (float a, float b, float c)
{
	x = a;
	y = b;
	z = c;
}



/* ===== VERTEX FUNCTIONS ===== */

ostream &operator<<(ostream &ostr, vertex &v)
{
	return ostr << "( " << v.x << " " << v.y << " " << v.z << " )";
}

bool IsVertexInList(vertex &V, vertex *VList, int vcount, bool UsePrecision, int deci)
{
	bool dev = 0;
	if (dev) cout << "      Checking if Vertex " << V << " is in VList with length " << vcount << ". Precision 0/1 (" << UsePrecision << ") up to " << deci << " places..." << endl;
	for (int v = 0; v<vcount; v++)
	{
		vertex &VC = VList[v];
		if (dev) cout << "          current VList Entry " << v << VC << endl;
		if (  UsePrecision  )
		{
			if (  CompareVerticesDeci(V, VC, deci)  )
			{
				if (dev) cout << "          MATCH! Vertex " << V << " is same as current Entry " << v << VC << endl;
				return true;
			}
		}
		else if (  !UsePrecision  )
		{
			if (  CompareVertices(V,VC)  )
			{
				if (dev) cout << "          MATCH! Vertex " << V << " is same as current Entry " << v << VC << endl;
				return true;
			}
		}
	}
	if (dev) cout << "        Vertex " << V << " was NOT found in this VList!" << endl;
	return false;
}

vertex Add(vertex V, gvector Vec)
{
	vertex N;
	N.x = V.x + Vec.x;
	N.y = V.y + Vec.y;
	N.z = V.z + Vec.z;
	return N;
}

vertex Add(vertex V1, vertex V2)
{
	vertex N;
	N.x = V1.x + V2.x;
	N.y = V1.y + V2.y;
	N.z = V1.z + V2.z;
	return N;
}

bool CompareVertices(vertex V0, vertex V1)
{
	if (
	V0.x==V1.x&&
	V0.y==V1.y&&
	V0.z==V1.z
	) return true;
	else return false;
}

bool CompareVerticesDeci(vertex V0, vertex V1, int deciplace)
{
	bool dev = 0;
	int d = pow(10, deciplace);
	if (d==0) d=1;
	if (dev) cout << "       Comparing Vertices Deciplaces "<<deciplace<<" ("<<d<<") V0 " << V0 << " V1 " << V1 << endl;
	if (
	floorf(V0.x*d)/d==floorf(V1.x*d)/d&&
	floorf(V0.y*d)/d==floorf(V1.y*d)/d&&
	floorf(V0.z*d)/d==floorf(V1.z*d)/d)
		return true;
	else
		return false;
}

bool CompareVerticesR(vertex V0, vertex V1)
{
	//cout << " Comparing vertices of V0 " << V0 << " and V1 " << V1;
	if (
	round(V0.x)==round(V1.x)&&
	round(V0.y)==round(V1.y)&&
	round(V0.z)==round(V1.z))
	{
		//cout << " MATCH!" << endl;
		return true;
	}
	else
	{
		//cout << " NO MATCH!" << endl;
		return false;
	}
}

bool CompareVerticesXY(vertex V0, vertex V1)
{
	if (
	V0.x==V1.x&&
	V0.y==V1.y
	) return true;
	else return false;
}

// Get Centroid of a Triangle
vertex GetCenTri(vertex v1, vertex v2, vertex v3)
{
	vertex c; // centroid
	
	c.x = (1.0/3.0)*(v1.x+v2.x+v3.x);
	c.y = (1.0/3.0)*(v1.y+v2.y+v3.y);
	c.z = (1.0/3.0)*(v1.z+v2.z+v3.z);
	//cout << "      centroid of this triangle is " << c << v1 << v2 << v3 << endl;
	
	return c;
}

vertex GetMVertex(vertex v1, vertex v2)
{
	vertex vn;
	vn.x = (v1.x+v2.x)/2;
	vn.y = (v1.y+v2.y)/2;
	vn.z = (v1.z+v2.z)/2;
	return vn;
}

float GetFaceLen3 (vertex p0, vertex p1, vertex p2)
{
	gvector Vec1, Vec2;
	float AlphaCos = 0;
	
	Vec1 = GetVector(p0, p1);
	Vec2 = GetVector(p0, p2);
	
	AlphaCos = sin(acos(GetAngleCos(Vec2, Vec1)));
	
	float result = AlphaCos * GetVecLen(Vec1);
	if (!IsValid(result)) result = 666.666;
	
	return result;
}



/* ===== VERTEX GROUP FUNCTIONS ===== */

// Get Centroid of a Bunch of Vertices
vGroup GetCentroidV(vGroup Origin)
{
	bool dev = 0;
	
	if (dev) cout << "Getting Centroid of Bunch of "<< Origin.size<< " Vertices..." << endl;
	if (dev)
	for (int i = 0; i<Origin.size; i++)
	cout << " V " << i << Origin.Vertices[i] << endl;
	
	vGroup Temp(Origin.size-2);
	vGroup Final;
	
	if (Origin.size>3)
	{
		if (dev) cout << "  More than 3 vertices!" << endl;
		for (int i = 0; i<Origin.size-2; i++)
		{
			Temp.Vertices[i] = GetCenTri(Origin.Vertices[0], Origin.Vertices[i+1], Origin.Vertices[i+2]);
		}
		Final = GetCentroidV(Temp);
		//Final.size = Temp.size-2;
		return Final;
	}
	else if (Origin.size==3)
	{
		if (dev) cout << "  Exactely 3 vertices!" << endl;
		Final.size = 1;
		Final.Vertices[0] = GetCenTri(Origin.Vertices[0], Origin.Vertices[1], Origin.Vertices[2]);
		return Final;
	}
	else if (Origin.size==2)
	{
		if (dev) cout << "  Exactely 2 vertices!" << endl;
		Final.Vertices[0] = GetMVertex(Origin.Vertices[0], Origin.Vertices[1]);
		if (dev) cout << "       centroid of these 2 vertices " << Origin.Vertices[0] << Origin.Vertices[1] <<" is " << Final.Vertices[0] << endl;
		Final.size = 1;
		return Final;
	}
	else if (Origin.size==1)
	{
		if (dev) cout << "  Exactely 1 vertices!" << endl;
		Final.size = 1;
		Final.Vertices[0] = Origin.Vertices[0];
		return Final;
	}
}



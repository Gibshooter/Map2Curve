#include "vertex.h"
#include "gvector.h"
#include "utils.h"
#include <math.h>
#include <iostream>

#define PI 3.14159265

#define DEBUG 0

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

void vertex::ScaleOrigin(float n, vertex Origin)
{
	if (n!=0)
	{
		this->move(-Origin.x, -Origin.y, -Origin.z);
		
		this->scale(n);
		
		this->move(Origin.x, Origin.y, Origin.z);
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
	}
}

void vertex::rotateOrigin(float degx, float degy, float degz, vertex orig)
{
	#if DEBUG > 0
	bool dev = 0;
	if(dev) cout << " Rotating Vertex " << *this << "..." <<endl;
	#endif
	
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
		
		#if DEBUG > 0
		if(dev) cout << "   New Vertex " << *this << endl;
		#endif
		
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
	#if DEBUG > 0
	bool dev = 0;
	if (dev) cout << "      Checking if Vertex " << V << " is in VList with length " << vcount << ". Precision 0/1 (" << UsePrecision << ") up to " << deci << " places..." << endl;
	#endif
	
	for (int v = 0; v<vcount; v++)
	{
		vertex &VC = VList[v];
		
		#if DEBUG > 0
		if (dev) cout << "          current VList Entry " << v << VC << endl;
		#endif
		
		if (  UsePrecision  )
		{
			if (  CompareVerticesDeci(V, VC, deci)  )
			{
				#if DEBUG > 0
				if (dev) cout << "          MATCH! Vertex " << V << " is same as current Entry " << v << VC << endl;
				#endif
				
				return true;
			}
		}
		else if (  !UsePrecision  )
		{
			if (  CompareVertices(V,VC)  )
			{
				#if DEBUG > 0
				if (dev) cout << "          MATCH! Vertex " << V << " is same as current Entry " << v << VC << endl;
				#endif
				
				return true;
			}
		}
	}
	
	#if DEBUG > 0
	if (dev) cout << "        Vertex " << V << " was NOT found in this VList!" << endl;
	#endif
	
	return false;
}

bool IsVertexInList(vertex &V, vector<vertex> VList, bool UsePrecision, int deci)
{
	int vcount = VList.size();
	
	#if DEBUG > 0
	bool dev = 0;
	if (dev) cout << "VINLIST Checking if Vertex " << V << " is in VList with length " << vcount << ". Precision 0/1 (" << UsePrecision << ") up to " << deci << " places..." << endl;
	#endif
	
	for (int v = 0; v<vcount; v++)
	{
		vertex &VC = VList[v];
		
		#if DEBUG > 0
		if (dev) cout << "VINLIST  current VList Entry " << v << VC << endl;
		#endif
		
		if (  UsePrecision  )
		{
			if (  CompareVerticesDeci(V, VC, deci)  )
			{
				#if DEBUG > 0
				if (dev) cout << "VINLIST   MATCH! Vertex " << V << " is same as current Entry " << v << VC << endl << endl;
				#endif
				
				return true;
			}
		}
		else if (  !UsePrecision  )
		{
			if (  CompareVertices(V,VC)  )
			{
				#if DEBUG > 0
				if (dev) cout << "VINLIST   MATCH! Vertex " << V << " is same as current Entry " << v << VC << endl << endl;
				#endif
				
				return true;
			}
		}
	}
	
	#if DEBUG > 0
	if (dev) cout << "VINLIST  Vertex " << V << " was NOT found in this VList!" << endl << endl;
	#endif
	
	return false;
}

bool IsVertexXYInList(vertex &V, vector<vertex> VList, bool UsePrecision, int deci)
{
	int vcount = VList.size();
	for (int v = 0; v<vcount; v++)
	{
		vertex &VC = VList[v];
		if (  UsePrecision  )
		{
			if (  CompareVerticesXYDeci(V, VC, deci)  )
				return true;
		}
		else if (  !UsePrecision  )
		{
			if (  CompareVerticesXY(V,VC)  )
				return true;
		}
	}
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
	#if DEBUG > 0
	bool dev = 0;
	#endif
	
	int d = pow(10, deciplace);
	if (d==0) d=1;
	int x1 = round(V0.x*d), x2 = round(V1.x*d);
	int y1 = round(V0.y*d), y2 = round(V1.y*d);
	int z1 = round(V0.z*d), z2 = round(V1.z*d);
	
	#if DEBUG > 0
	if (dev) cout << "VCOMPARE Deci "<<deciplace<<" ("<<d<<") V0 [" << x1 << "|" << y1 << "|" << z1 << "] V0 [" << x2 << "|" << y2 << "|" << z2 << "] " << endl;
	#endif
	
	if ( x1==x2 && y1==y2 && z1==z2)
		return true;
	else
		return false;
}

bool CompareVerticesXYDeci(vertex V0, vertex V1, int deciplace)
{
	#if DEBUG > 0
	bool dev = 0;
	#endif
	
	int d = pow(10, deciplace);
	if (d==0) d=1;
	int x1 = round(V0.x*d), x2 = round(V1.x*d);
	int y1 = round(V0.y*d), y2 = round(V1.y*d);

	#if DEBUG > 0
	if (dev) cout << "VCOMPARE Deci "<<deciplace<<" ("<<d<<") V0 [" << x1 << "|" << y1 << "] V0 [" << x2 << "|" << y2 << "] " << endl;
	#endif
	
	if ( x1==x2 && y1==y2)
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

// https://stackoverflow.com/questions/2792443/finding-the-centroid-of-a-polygon
vertex GetCentroid2024(vertex *vertices, int &vertexCount, bool devinfo)
{
	#if DEBUG > 0
	if (devinfo) cout << " Get Centroid 2024..." << endl;
	#endif
	
	vertex centroid;
	double cenx = 0, ceny=0;
	
	double signedArea = 0.0;
	double x0 = 0.0; // Current vertex X
	double y0 = 0.0; // Current vertex Y
	double x1 = 0.0; // Next vertex X
	double y1 = 0.0; // Next vertex Y
	double a = 0.0;  // Partial signed area

	// For all vertices except last
	int i=0;
	for (i=0; i<vertexCount-1; ++i)
	{
		x0 = vertices[i].x;
		y0 = vertices[i].y;
		x1 = vertices[i+1].x;
		y1 = vertices[i+1].y;
		a = x0*y1 - x1*y0;
		signedArea += a;
		cenx += (x0 + x1)*a; //centroid.x
		ceny += (y0 + y1)*a; //centroid.y
		#if DEBUG > 0
		if (devinfo) cout << fixed << "   loop#"<<i<<" x0 "<<x0<<" y0 "<<y0<<" x1 "<<x1<<" y1 "<<y1<<" a "<<a<<" signed Area "<<signedArea<<" cen.x "<<cenx<<" cen.y "<<ceny<<endl;
		#endif
	}

	// Do last vertex separately to avoid performing an expensive
	// modulus operation in each iteration.
	x0 = vertices[i].x;
	y0 = vertices[i].y;
	x1 = vertices[0].x;
	y1 = vertices[0].y;
	a = x0*y1 - x1*y0;
	signedArea += a;
	cenx += (x0 + x1)*a;
	ceny += (y0 + y1)*a;
	
	#if DEBUG > 0
	if (devinfo) cout << fixed << "   loop#"<<vertexCount<<" x0 "<<x0<<" y0 "<<y0<<" x1 "<<x1<<" y1 "<<y1<<" a "<<a<<" signed Area "<<signedArea<<" cen.x "<<cenx<<" cen.y "<<ceny<<endl;
	#endif

	signedArea *= 0.5;
	cenx /= (6.0*signedArea);
	ceny /= (6.0*signedArea);
	
	#if DEBUG > 0
	if (devinfo) cout << fixed << "   final - signed Area*=0.5 "<<signedArea<<" cen.x/=6.0*signedArea "<<cenx<<" cen.y/=6.0*signedArea "<<ceny<<endl;
	#endif

	centroid.x = cenx;
	centroid.y = ceny;

	return centroid;
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
	#if DEBUG > 0
	bool dev = 0;
	
	if (dev) cout << "Getting Centroid of Bunch of "<< Origin.size<< " Vertices..." << endl;
	if (dev)
	for (int i = 0; i<Origin.size; i++)
	cout << " V " << i << Origin.Vertices[i] << endl;
	#endif

	vGroup Temp(Origin.size-2);
	vGroup Final;
	
	if (Origin.size>3)
	{
		#if DEBUG > 0
		if (dev) cout << "  More than 3 vertices!" << endl;
		#endif
		
		for (int i = 0; i<Origin.size-2; i++)
		{
			Temp.Vertices[i] = GetCenTri(Origin.Vertices[0], Origin.Vertices[i+1], Origin.Vertices[i+2]);
		}
		Final = GetCentroidV(Temp);
		
		return Final;
	}
	else if (Origin.size==3)
	{
		#if DEBUG > 0
		if (dev) cout << "  Exactely 3 vertices!" << endl;
		#endif
		
		Final.size = 1;
		Final.Vertices[0] = GetCenTri(Origin.Vertices[0], Origin.Vertices[1], Origin.Vertices[2]);
		
		return Final;
	}
	else if (Origin.size==2)
	{
		#if DEBUG > 0
		if (dev) cout << "  Exactely 2 vertices!" << endl;
		#endif
		
		Final.Vertices[0] = GetMVertex(Origin.Vertices[0], Origin.Vertices[1]);
		
		#if DEBUG > 0
		if (dev) cout << "       centroid of these 2 vertices " << Origin.Vertices[0] << Origin.Vertices[1] <<" is " << Final.Vertices[0] << endl;
		#endif
		
		Final.size = 1;
		return Final;
	}
	else if (Origin.size==1)
	{
		#if DEBUG > 0
		if (dev) cout << "  Exactely 1 vertices!" << endl;
		#endif
		
		Final.size = 1;
		Final.Vertices[0] = Origin.Vertices[0];
		return Final;
	}
}

vertex operator+(vertex &V, gvector &Vec)
{
	vertex VN;
	VN.setall( V.x+Vec.x, V.y+Vec.y, V.z+Vec.z );
	
	return VN;
}

vertex operator-(vertex &V, gvector &Vec)
{
	vertex VN;
	VN.setall( V.x-Vec.x, V.y-Vec.y, V.z-Vec.z );
	
	return VN;
}



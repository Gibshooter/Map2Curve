#include "gvector.h"
#include "vertex.h"
#include "utils.h"
#include <math.h> // PI, sin, cos, pow, sqrt

#define PI 3.14159265

using namespace std;

struct vertex;

/* ===== GVECTOR FUNCTIONS ===== */

ostream &operator<<(ostream &ostr, gvector &v)
{
	return ostr << "( " << v.x << " " << v.y << " " << v.z << " )";
}

gvector GetVector(vertex p1, vertex p2) {
	
	gvector Vector;

	Vector.x = p2.x - p1.x;
	Vector.y = p2.y - p1.y;
	Vector.z = p2.z - p1.z;

	return Vector;
}

float GetAdjaLen (gvector Hypo, gvector Vec)
{
	float AlphaCos = GetAngleCos(Hypo, Vec);
	float HypoLen = GetVecLen (Hypo);
	//cout << " AlphaCos "<< AlphaCos << " HypoLen "<< HypoLen << " AdjaLen " << AlphaCos * HypoLen << endl;
	
	float result = AlphaCos * HypoLen;
	if (!IsValid(result)) result = 666.666;
	
	return result;
}

float GetOppoLen (gvector Hypo, gvector Vec)
{
	float AlphaSin = sin(acos(GetAngleCos(Hypo, Vec)));
	float HypoLen = GetVecLen (Hypo);
	
	float result = AlphaSin * HypoLen;
	if (!IsValid(result)) result = 666.666;
	
	return result;
}

gvector VecAdd (gvector Vec1, gvector Vec2)
{
	gvector VecA;
	
	VecA.x = Vec1.x + Vec2.x;
	VecA.y = Vec1.y + Vec2.y;
	VecA.z = Vec1.z + Vec2.z;
	
	return VecA;
}

float GetVecAlign (gvector Vector, int mode = 0) {
	
	Vector = Normalize(Vector);
	float yaw=0, pitch=0;
	gvector Base, VecYaw, VecPitch;
	Base.x = 1;
	
	if 		(Vector.x==1||(Vector.x==0&&Vector.y==0)||(Vector.z==1||Vector.z==-1)) 	{ yaw = 0; 		if (mode == 0) return yaw; }
	else if (Vector.y==1) 	{ yaw = 90; 	if (mode == 0) return yaw; }
	else if (Vector.x==-1) 	{ yaw = 180; 	if (mode == 0) return yaw; }
	else if (Vector.y==-1) 	{ yaw = 270; 	if (mode == 0) return yaw; }
	else
	{
		// Get Yaw Angle (mode = 0)
		VecYaw = Vector;
		VecYaw.z = 0;
		//VecYaw=Normalize(VecYaw);
		
		yaw = acos(GetAngleCos(VecYaw, Base)) * 180.0 / PI;
		if (VecYaw.y <= 0) yaw = 360 - yaw;
		if (yaw==360||yaw==NAN) yaw = 0;
		
		if (mode == 0) return yaw;
	}

	// Get Pitch Angle (mode = 1)
	//cout << "  Get Pitch of Vector " << Vector << endl;
	VecPitch = Vector;
	//Base = Vector;
	//Base.z = 0;
	
	if 		(VecPitch.z==1) 				{ pitch = 90; /*cout << "    Vec.z==1, pitch is 90 degree!"<<endl;*/ 		if (mode == 1) return pitch;}
	else if (VecPitch.z==-1) 				{ pitch = 270; /*cout << "    Vec.z==-1, pitch is 270 degree!"<<endl;*/ 		if (mode == 1) return pitch; }
	else if (VecPitch.z==0&&VecPitch.x==1) 	{ pitch = 0; /*cout << "    Vec.z==0;x==1, pitch is 0 degree!"<<endl;*/ 		if (mode == 1) return pitch; }
	//else if (VecPitch.z==0&&VecPitch.x==-1) { pitch = 180; /*cout << "    Vec.z==1;x==-1, pitch is 180 degree!"<<endl;*/ 		if (mode == 1) return pitch; }
	
	if (yaw!=0) {
		if (yaw<=180)
		VecPitch.rotate(0,0,-(yaw));
		else
		VecPitch.rotate(0,0,-(yaw-180));
		
		//cout << "        yaw = " << yaw << " PitchVec rotated by "<<-(yaw-180)<< endl;
	}
	
	pitch = acos(GetAngleCos(VecPitch, Base)) * 180.0 / PI;
	
	if (VecPitch.z<0) {
		pitch = 360 - pitch;
	}
	if (pitch>=360) pitch = 0;
	
	if (mode == 1) return pitch;
	else return 0;
}

float GetAngleCos(gvector Vector1, gvector Vector2) {
	
	float Dot, Vec1Len, Vec2Len;
	
	Dot = GetDot(Vector1, Vector2);
	Vec1Len = GetVecLen(Vector1);
	Vec2Len = GetVecLen(Vector2);
	
	float result = Dot / (Vec1Len * Vec2Len);
	if (!IsValid(result)) result = 666.666;
	
	return result;
}

gvector GetCross(gvector v1, gvector v2) {

	float &a1 = v1.x;
	float &a2 = v1.y;
	float &a3 = v1.z;
	
	float &b1 = v2.x;
	float &b2 = v2.y;
	float &b3 = v2.z;

	// Calculate Crossproduct for Vector 1 and 2
	gvector Crossproduct;
	Crossproduct.x = (a2*b3)-(a3*b2);
	Crossproduct.y = (a3*b1)-(a1*b3);
	Crossproduct.z = (a1*b2)-(a2*b1);
	
	return Crossproduct;
}

gvector Normalize(gvector Vector)
{
	// Get Length of Vector
	// extensions
	double ext1 = pow(Vector.x, 2) + pow(Vector.y, 2) + pow(Vector.z, 2);
	double ext2 = sqrt(ext1);
	
	// normalize Vector
	double ext3 = 1.0 / ext2;
	
	gvector nVector;
	
	nVector.x = Vector.x * ext3;
	nVector.y = Vector.y * ext3;
	nVector.z = Vector.z * ext3;

	return nVector;
}

float GetDot(gvector v1, gvector v2) {
	
	float dotproduct = (v1.x * v2.x) + (v1.y * v2.y) + (v1.z * v2.z);
	//cout << "dotproduct (vecA "<<v1<<" / vecB "<<v2<<"): " << dotproduct << endl;
	return dotproduct;
}

float GetVecLen(gvector Vector) {
	
	float step1 = pow(Vector.x, 2) + pow(Vector.y, 2) + pow(Vector.z, 2);
	float step2 = sqrt(step1);
	
	float result = step2;
	if (!IsValid(result)) result = 666.666;
	
	return result;
}

float GetVecAng(gvector Vec1, gvector Vec2) {
	
	double Dot = GetDot(Vec1, Vec2);
	
	double Vec1Len = GetVecLen(Vec1);
	double Vec2Len = GetVecLen(Vec2);
	
	double CosPhi = Dot / (Vec1Len * Vec2Len);
	
	float result = acos (CosPhi) * 180.0 / PI;
	if (!IsValid(result)) result = 0;
	
	return result;
}





/* ===== GVECTOR METHODS ===== */

void gvector::mult(float n)
{
	x *= n;
	y *= n;
	z *= n;
}

void gvector::div(float n)
{
	x /= n;
	y /= n;
	z /= n;
}

void gvector::set(float n)
{
	x = n;
	y = n;
	z = n;
}

float gvector::len()
{
	return sqrt(pow(x, 2) + pow(y, 2) + pow(z, 2));
}

void gvector::CopyCoords(gvector &Source)
{
	x = Source.x;
	y = Source.y;
	z = Source.z;
}

gvector gvector::flip()
{
	gvector Vec;
	Vec.x = x*-1.0; x *= -1.0;
	Vec.y = y*-1.0; y *= -1.0;
	Vec.z = z*-1.0; z *= -1.0;
	return Vec;
}

void gvector::rotate(float degx, float degy, float degz)
{
	// degree to rad
	float radx = degx * PI / 180.0;
	float rady = degy * PI / 180.0;
	float radz = degz * PI / 180.0;
	//cout << "degx (" << degx << ") [" << radx << "] degy (" << degy << ") [" << rady << "] degz (" << degz << ") [" << radz << "]" << endl;
	
	if (degx!=0)
	{
		float newY = (y * cos(radx)) - (z * sin(radx));
		float newZ = (y * sin(radx)) + (z * cos(radx));
		y = newY;
		z = newZ;
		//cout << " Y [" << newY << "]\t (" << y << ") Z [" << newZ << "]\t (" << z << ")" << endl;
	}
	if (degy!=0)
	{
		float newX = (x * cos(rady)) + (z * sin(rady));
		float newZ = (x * -sin(rady)) + (z * cos(rady));
		x = newX;
		z = newZ;
		//cout << "X [" << newX << "]\t (" << x << ")\t Z [" << newZ << "]\t (" << z << ")" << endl;
	}
	if (degz!=0)
	{
		float newX = (x * cos(radz)) - (y * sin(radz));
		float newY = (x * sin(radz)) + (y * cos(radz));
		x = newX;
		y = newY;
		//cout << "X [" << newX << "]\t (" << x << ")\t Y [" << newY << "]\t (" << y << ")" << endl;
	}
}

void gvector::Normalize()
{
	double ext1 = pow(x, 2) + pow(y, 2) + pow(z, 2);
	double ext2 = sqrt(ext1);
	
	double ext3 = 1.0 / ext2;
	
	x = x * ext3;
	y = y * ext3;
	z = z * ext3;
}

void gvector::AddVec(gvector AVec)
{
	x += AVec.x;
	y += AVec.y;
	z += AVec.z;
}


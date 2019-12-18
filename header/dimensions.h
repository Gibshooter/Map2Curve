#ifndef __DIMENSIONS_H_INCLUDED__
#define __DIMENSIONS_H_INCLUDED__

#include <string>
#include <vector>

using namespace std;


/* ===== DIMENSIONS CLASS ===== */

struct dimensions
{
	float xs = 0;
	float xb = 0;
	float ys = 0;
	float yb = 0;
	float zs = 0;
	float zb = 0;
	
	void set(float a, float b, float c);
	void expand(int size);
};

dimensions DimensionCombine(dimensions D1, dimensions D2);

ostream &operator<<(ostream &ostr, dimensions &D);


#endif

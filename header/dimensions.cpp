#include "dimensions.h"

#include <math.h>
#include <ostream>

using namespace std;


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


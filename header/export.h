#ifndef __EXPORT_H_INCLUDED__
#define __EXPORT_H_INCLUDED__

#include "group.h"

#include <string>

using namespace std;

//struct brush;


/* ===== FILE CLASS ===== */

// export final curve data
void ExportToMap();
void ExportToRMF();
void ExportToObj();

// DEVELOPER

void ExportToMapO();
void ExportGroupToMap(group &Group, string filename);
void ExportToObjDev();
void ExportGroupToObjDev(group_set &Set, string filename);
void ExportGroupToObjDev(group &Group, string filename);






#endif

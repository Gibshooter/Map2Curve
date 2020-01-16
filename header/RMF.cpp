#include "RMF.h"


using namespace std;


/* ===== RMF METHODS ===== */

void RMF::CloseFile()
{
	RMFBuffer.close();
}

// File Structure
void RMF::WriteHeader()
{
	unsigned char Header[7] = {205,204,12,64,82,77,70}; // "????RMF"
	RMFBuffer.write ((char*)&Header, sizeof(Header));
}

void RMF::WriteVisGroups(int n_vis)
{
	WriteByte(n_vis);
	// Write Vis Groups...
	
	WriteFixedTString("CMapWorld"); // Worldspawn object type
	
	unsigned char _BYTES_[7] = {0,0,0,0,220,220,220};
	RMFBuffer.write ((char*)&_BYTES_, 7); // ?? visgroup and color fields (not used by VHE)
}

void RMF::WriteGroup(int n_obj, unsigned char color[3])
{
	WriteFixedTString("CMapGroup");
	WriteByte((int)0); // vis group index
	WriteColor(color[0],color[1],color[2]);
	WriteByte(n_obj); // number of objects in this group
	
	// write objects now ...
}

unsigned long RMF::WriteCounterGetPos()
{
	int ctr = 0;
	long pos = RMFBuffer.tellp();
	RMFBuffer.write ((char*)&ctr, sizeof(int));
	return pos;
}

void RMF::UpdateAtPosAndReturn(unsigned long pos_target, int &content)
{
	unsigned long pos_current = RMFBuffer.tellp();
	RMFBuffer.seekp(pos_target);
	RMFBuffer.write((char*)&content, sizeof(int));
	RMFBuffer.seekp(pos_current);
}

void RMF::WriteWorldSpawn(entity &Entity_WS)
{
	WriteFixedTString("worldspawn"); // max len 128
	WriteByteEmpty(4); // ?? 4 Bytes
	WriteByte((int)0); // Worldspawn Entity Flags (unused by VHE)
	
	// Key/Value Pairs
	int n_keys = Entity_WS.Keys_Original.size(); // (standard keys are "classname"="worldspawn", "sounds"="#", "MaxRange"="#", "mapversion"="220")
	unsigned long pos_n_keys = RMFBuffer.tellp(); // safe counter pos to update it later
	WriteByte((int)0); // number of key/value pairs
	/*for(int i=0; i<n_keys; i++)
	{
		key &K = Entity_WS.Keys_Original[i];
		if(K.name!="classname"&&K.name!="spawnflags")
		{
			WriteFixedTString(K.name);
			WriteFixedTString(K.value);
		}
		else
		{
			n_keys--;
		}
	}*/
	// update key value pair counter
	/*int pos_last = RMFBuffer.tellp();
	RMFBuffer.seekp(pos_n_keys);
	WriteByte(n_keys);
	RMFBuffer.seekp(pos_last);
	*/
	WriteByteEmpty(12); // ?? 12 bytes
}

void RMF::WritePaths(int n_paths)
{
	WriteByte(n_paths);
	// Write Paths...
	
	// Docinfo String
	string DocInfo = "DOCINFO";
	WriteTString(DocInfo);
	
	// 12 Bytes
	unsigned char _BYTES_[12] = {205,204,76,62,255,255,255,255,0,0,0,0}; // ??
	RMFBuffer.write ((char*)&_BYTES_, 12);
}


// Face
void RMF::WriteFace(face &Face)
{
	bool dev = 0;
	if(dev) cout << " RMF Export Face Tex " << Face.Texture << " Vcount " << Face.vcount << endl;
	// Texture String
	string Tex256 = Face.Texture;
	Tex256.resize(255);
	WriteTString(Tex256);
	
	WriteByte((float)0);// Float???
	
	WriteVector(Face.VecX);// Tex Vertex H
	WriteByte(Face.ShiftX);// Tex Shift X
	WriteVector(Face.VecY);// Tex Vertex V
	WriteByte(Face.ShiftY);// Tex Shift X
	WriteByte(Face.Rot);// Rotation
	WriteByte(Face.ScaleX);// Scale
	WriteByte(Face.ScaleY);
	
	WriteByteEmpty(16);// 16 Bytes
	
	// Vertex Counter
	int n_verts = 0;
	unsigned long pos_n_verts = WriteCounterGetPos();
	
	// Vertices
	for (int v=0; v<Face.vcount; v++)
	{
		vertex &V = Face.Vertices[v];
		if(dev) cout << "   Vertex #" << v << " Valid " << V.IsValid << V << endl;
		if(V.IsValid) {
			WriteVector(V);
			n_verts++;
		}
	}
	if(dev) cout << endl;
	
	// Update Vertex Counter
	UpdateAtPosAndReturn(pos_n_verts, n_verts);
	
	// 3x Plane Vertices
	for (int v=0,valid=0; valid<3; v++)
	{
		vertex &V = Face.Vertices[v];
		if(V.IsValid) { WriteVector(V); valid++;}
	}
}

// Solid
void RMF::WriteSolid(brush &Brush)
{
	// Type
	WriteFixedTString("CMapSolid");
	
	// Vis Group Index
	WriteByte((int)0);
	
	// Color
	WriteColor(0,255,255);
	
	// 4 byte
	WriteByteEmpty(4);
	
	// Face Count
	int n_faces = 0;
	unsigned long pos_n_faces = WriteCounterGetPos();
	
	// Faces
	for (int f=0; f<Brush.t_faces; f++)
	{
		face &F = Brush.Faces[f];
		if(F.draw) {
			WriteFace(F);
			n_faces++;
		}
	}
	
	// Update Face Counter
	UpdateAtPosAndReturn(pos_n_faces, n_faces);
}

// Entity
void RMF::WritePointEntity(entity &Entity)
{
	WriteEntityHeader(Entity);
	
	// Brushes (no brushes)
	WriteByte((int)0);
	
	WriteEntityFooter(Entity);
}

void RMF::WriteEntityHeader(entity &Entity)
{
	// Type
	WriteFixedTString("CMapEntity");
	
	// Vis Group Index
	WriteByte((int)0);
	
	// Color
	WriteColor((unsigned char)48,(unsigned char)48,(unsigned char)48);
	
	// Brush Count
	// written externally...
}

void RMF::WriteEntityFooter(entity &Entity)
{
	WriteFixedTString(Entity.key_classname); // Classname
	WriteByteEmpty((int)4); // 4 byte
	WriteByte(Entity.SpawnFlags); // entity flags
	int n_keys = Entity.Keys_Original.size();
	// Number of Key/Value Pairs
	int n_keyvals = 0;
	unsigned long pos_n_keyvals = WriteCounterGetPos();
	
	// Key/Value Pairs
	for(int i=0; i<n_keys; i++)
	{
		key &K = Entity.Keys_Original[i];
		if(K.name!="classname"&&K.name!="spawnflags")
		{
			WriteFixedTString(K.name);
			WriteFixedTString(K.value);
			n_keyvals++;
		}
	}
	UpdateAtPosAndReturn(pos_n_keyvals, n_keyvals);
	
	WriteByteEmpty(14); // ?? 14 Bytes
	WriteVector(Entity.Origin); // Entity Position
	WriteByteEmpty(4); // ?? 4 Bytes
}


// Primitives
void RMF::WriteFixedTString(string s)
{
	int n = s.length();
	int n1 = n+1;
	char term = '\0';
	
	RMFBuffer.write((char*)&n1, 1);
	RMFBuffer.write((char*)s.c_str(), s.length());
	RMFBuffer.write((char*)&term, 1);
}

void RMF::WriteByte(int content)
{
	RMFBuffer.write((char*)&content, sizeof(int));
}

void RMF::WriteByte(float content)
{
	RMFBuffer.write((char*)&content, sizeof(float));
}

void RMF::WriteByteEmpty(int length)
{
	char dummy[length]; for(int i=0;i<length;i++) dummy[i]=0;
	RMFBuffer.write((char*)&dummy, length);
}

void RMF::WriteTString(string s)
{
	char term = '\0';
	RMFBuffer.write((char*)s.c_str(), s.length());
	RMFBuffer.write((char*)&term, 1);
}

void RMF::WriteColor(unsigned char r, unsigned char g, unsigned char b)
{
	unsigned char rgb[3] = { r, g, b};
	RMFBuffer.write((char*)&rgb, 3);
}

void RMF::WriteVector(gvector V)
{
	RMFBuffer.write((char*)&V.x, sizeof(float));
	RMFBuffer.write((char*)&V.y, sizeof(float));
	RMFBuffer.write((char*)&V.z, sizeof(float));
}

void RMF::WriteVector(vertex V)
{
	RMFBuffer.write((char*)&V.x, sizeof(float));
	RMFBuffer.write((char*)&V.y, sizeof(float));
	RMFBuffer.write((char*)&V.z, sizeof(float));
}



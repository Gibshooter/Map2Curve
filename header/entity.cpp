#include "entity.h"
#include "vertex.h"
#include "face.h"
#include "brush.h"
#include "matrix.h"
#include "settings.h"
#include "utils.h"
#include "file.h"

#include <string>
#include <vector>
#include <iostream>
#include <conio.h> // getch

using namespace std;

extern vector<string> slist;
extern file *gFile;

/* ===== ENTITY METHODS ===== */

void entity::GetKeyValues()
{
	bool dev = 0;
	string phrase[] = { "classname", "angles", "origin", "targetname", "target" };
	string phrase2[] = { "d_enable", "d_pos", "d_autopitch", "d_autoyaw", "d_separate", "d_autoname", "d_pos_rand", "d_rotz_rand", "d_movey_rand", "d_draw", "d_skip", "d_draw_rand" };
	
	int p1 = (sizeof(phrase)/sizeof(*phrase));
	int p2 = slist.size(); //(sizeof(slist)/sizeof(*slist));
	int p3 = (sizeof(phrase2)/sizeof(*phrase2));
	if (dev) cout << " Getting Keyvalues of Entity (type" << type << "). Phrases: " << p1 << "+"<<p2<<"="<<p1+p2<<"..."<<endl;
	// in this entity look for all possible settings and a few 
	for (int i = 0, j = 0, k = 0; i<p1+p2+p3; i++)
	{
		string search_phrase, phrase_short;
		int phrase_len = 0;
		if (i<p1) { search_phrase = "\""+phrase[i]+"\" \""; phrase_short = phrase[i]; }
		else if (i<p1+p3) { search_phrase = "\"m2c_"+phrase2[k]+"\" \""; phrase_short = phrase2[k]; }
		else      { search_phrase = "\"m2c_"+slist[j]+"\" \""; phrase_short = slist[j]; }
		phrase_len = search_phrase.length();
		
		if (i<p1+p3 || key_classname=="info_curve" || key_classname=="info_detailgroup" || key_classname=="info_curve_export" )
		{
			if (dev && i<p1) cout << "   #"<<i<<" Looking for search phrase " << search_phrase<<endl;
			else if (dev && i>=p1) cout << "   #"<<k<<" Looking for search phrase " << search_phrase<<endl;
			else if (dev && i>=p1+p3) cout << "   #"<<j<<" Looking for search phrase " << search_phrase<<endl;
			
			int p_start = content.find(search_phrase, 0);
			if (p_start!=-1)
			{
				if (dev) cout << "     Found at pos " << p_start<<endl;
				int v_start = p_start+phrase_len;
				int v_end = content.find("\"\n", v_start );
				if (dev) cout << "     Value start at pos " << v_start << " (phrase length "<<phrase_len<<") end at " << v_end << " substr " << content.substr(v_start,v_end-v_start) <<endl;
				key Dummy;
				Keys.push_back(Dummy);
				int k = Keys.size()-1;
				Keys[k].name   = phrase_short;
				Keys[k].value = content.substr( v_start, v_end-v_start );
				if (dev) cout << "     Key " << Keys[k].name<<endl;
				if (dev) cout << "     Value " << Keys[k].value<<endl;
				if (Keys[k].name=="classname") key_classname = Keys[k].value;
				if (Keys[k].name=="target") key_target = Keys[k].value;
				if (Keys[k].name=="targetname") key_targetname = Keys[k].value;
			}
		}
		if (i>=p1+p3) j++;
		if (i>=p1) k++;
	}
	if (dev) getch();
	
	if (dev) cout << endl << " Extracting and assigning values..."<<endl;
	for (int k=0; k<Keys.size(); k++)
	{
		string name = Keys[k].name;
		string value = Keys[k].value;
		vector<string> Values;
		if (dev) cout << "   Key #"<<k<< " Name: " << name << " Value " << value <<endl;
		
		// key values concerning detail groups are stored in the entity itself rather than in a contruction table
			if (name=="d_enable") 	if(value!="-2") d_enable = stoi(value);
			if (name=="d_pos") 	if(value!="UNSET") d_pos 	= stof(value);
			if (name=="d_autopitch")if(value!="-2") d_autopitch = stoi(value);
			if (name=="d_autoyaw") 	if(value!="-2") d_autoyaw 	= stoi(value);
			if (name=="d_separate") if(value!="-2") d_separate 	= stoi(value);
			if (name=="d_autoname") if(value!="-2") d_autoname 	= stoi(value);
			if (name=="d_pos_rand") if(value!="-2") d_pos_rand.set(value);
			if (name=="d_rotz_rand") if(value!="-2") d_rotz_rand.set(value);
			if (name=="d_movey_rand") if(value!="-2") d_movey_rand.set(value);
			if (name=="d_draw") 	if(value!="-2") d_draw 		= stoi(value);
			if (name=="d_skip") 	if(value!="-2") d_skip 		= stoi(value);
			if (name=="d_draw_rand") if(value!="-2") d_draw_rand 	= stoi(value);
			
		if (name=="angles"||name=="origin") {
			if (value.length()>2)
			SplitString(value," ", Values);
		}
		if (dev)
		for (int i = 0; i<Values.size(); i++) {
			cout << "       Extracted Values from Key #" << k << "("<<name << ", "<<value<<") -> #" << i << " = " << Values[i] << endl; }
		
		if (dev) cout << "   Values Count "<<Values.size() << endl;
		if (Values.size()>2)
		{
			if (name=="angles") {
				//rot.SetAngles( stof(Values[2]), stof(Values[0]), stof(Values[1]) );
				Euler NewAngles(stof(Values[2]), stof(Values[0]), stof(Values[1])); // Keyvalue Order (Y Z X) -> [ 1 2 0 ]
				RotateEntity(NewAngles,0);
				Angles = NewAngles;
			}
			if (name=="origin") {
				Origin.x = stof(Values[0]); Origin.y = stof(Values[1]); Origin.z = stof(Values[2]);
			}
		}
	}
	
	if (dev) getch();
}

void entity::GetIntMapSettings(vector<string> &MapSettings)
{
	bool dev = 0;
	vector<int> &ID_List = gFile->settingsM_ID;
	if (key_classname=="info_curve")
	{
		bool frad = 0;
		
		// first look if there is any rad command in the keylist
		if (dev) cout << " first look if there is any rad command in the keylist..." << endl;
		for (int k=0; k<Keys.size(); k++) {
			if (dev) cout << "   Key #" << k << " Name " <<  Keys[k].name << " Val " << Keys[k].value << endl;
			string name = Keys[k].name;
			if (name=="rad") { frad++; if (dev) cout << "   FOUND YA!" << endl; }
		}
		// if there isnt, add one
		if (frad==0) {
			if (dev) cout << " Found NO rad keys in this info_curve entity! Adding one myself..." << endl;
			key Dummy;
			Dummy.name="rad";
			Dummy.value="0";
			Keys.push_back(Dummy);
		}
	}
	if (key_classname=="info_curve"||key_classname=="info_curve_export")
	{
		// then add all settings to settingsM table (fix -1 values while youre on it?)
		if (dev) cout << " Adding all settings to settingsM table..." << endl;
		for (int k=0; k<Keys.size(); k++)
		{
			string name = Keys[k].name;
			string value = Keys[k].value;
			
			int phrase_ctr = slist.size(); //(sizeof(slist)/sizeof(*slist));
			for (int s=0; s<phrase_ctr; s++)
			{
				string phrase = slist[s];
				if (name==phrase) {
					MapSettings.push_back(slist[s]);
					if (value=="-1") value="0";
					MapSettings.push_back(value);
					ID_List.push_back(cID);
					ID_List.push_back(cID);
					break;
				}
			}
		}
		if (dev) getch();
	}
}

void entity::CreateBrushes()
{
	bool dev = 0;
	bool dev2 = 0;
	
	if (t_brushes>0&&content.length()>0)
	{
		if (Brushes!=nullptr)
		{
			delete[] Brushes;
			Brushes = nullptr;
		}
		Brushes = new brush[t_brushes];
	
		// count faces
		if (dev) cout << " Counting faces..."<<endl;
		int found = 0, last = head_end;
		while (found!=-1) { // count all faces
			found = content.find("(", last);
			if (dev&&dev2) cout << "   FaceStartPos" << found;
			if (found!=-1) {
				int lineend = content.find("\n", found);
				if (dev&&dev2) cout << " FaceEndPos" << lineend;
				last = lineend;
				t_faces++;
				if (dev&&dev2) cout << " Counter " << t_faces <<endl;
			}
			else break;
		}
		if (dev) cout << " total faces: " << t_faces << endl;
		if (dev) cout << " total brushes: " << t_brushes << endl;
		
		// Interpret Map File
		if (dev) cout << " Interpreting Entity Brushes & Faces..." << endl;
		int btable[t_brushes];
		string b_import[t_faces][22]; // imported brush array; stores all available informations about each brush
		
		last = 1; int lastfam = 0;
		for(int b = 0, maxf = 0; b < t_brushes; b++) // brush loop
		{
			int bstart = content.find("{\n( ",last);
			int blen = content.find("\n}\n",bstart)-bstart;
			if (dev&&dev2) cout << "  Brush " << b << ", bstart " << bstart << ", blen " << blen << endl;
			
			// get brush face count
			if (dev) cout << "  Getting face count..." << endl;
			for (int j = 0, lastf = bstart, foundf = 0; (foundf < bstart+blen)&&(foundf!=-1); j++) {
				if (dev&&dev2) cout << "    Search Loop " << j << " lastf" << lastf;
				foundf = content.find("(", lastf);
				if (dev&&dev2) cout << " FaceStartPos " << foundf;
				lastf = content.find("\n", foundf);
				if (dev&&dev2) cout << " LineEndPos " << lastf << endl;
				maxf = j;
			}
			btable[b] = maxf;
			if (dev) cout << "  Facecount for brush#" << b+1 << ": " << maxf << endl;
			//face loop, copy values to face array
			int last_val = bstart, valstart = 0, valend = 0;
			for(int f = lastfam; f-lastfam<maxf; f++)
			{
				if (dev&&dev2) cout << "   Copy values of Face " << f+1 << endl;
				b_import[f][0] = to_string(b);
				if (dev&&dev2) cout << "  Face: " << f <<  " of brush: " << b << "("<<b_import[f][0]<<")" << endl;
				// value loop
				for (int v = 1; v < 22; v++) {
					if (v==1) 	valstart = content.find_first_not_of("{ ([])\n", last_val);
					else 		valstart = content.find_first_not_of(" ([])\n", last_val);
					valend = content.find_first_of(" ([])", valstart);
					last_val = valend;
					b_import[f][v] = content.substr(valstart,valend-valstart);
					
					if (dev&&dev2) cout << "face: " << f << ", value: " << v << ", text: " << b_import[f][v] << endl;
					if (dev&&dev2) cout << "valstart: " << valstart << ", valend: " << valend << ", last_val: " << last_val << endl;
				}
			}
			lastfam += maxf;
			last = bstart+1;
		}
		
		//print all brushes and info
		/*for(int f = 0; f<tfc; f++) {
			cout << "Brush ("<< b_import[f][0] << ") ";
			for(int v = 1; v<22; v++) {
				cout << b_import[f][v] << " ";
			}
		cout << endl;
		}*/
		
		// copy raw brush information into raw object
		if (dev) cout << "Copying map information into Brush objects..." << endl;
		//if (dev) getch();
		for (int b = 0, i = 0; b<t_brushes; b++)
		{
			brush &Brush = Brushes[b];
			Brush.entID = eID;
			Brush.t_faces = btable[b];
			Brush.Faces = new face[Brush.t_faces];
			Brush.bID = b;
			
			if (dev) cout << "  Brush " << b << " Faces " << Brush.t_faces << " entID " << Brush.entID << " EntityID " << eID << endl;
			
			//Brush.SegID = b;
			//if (dev) cout << "  SegID " << b << endl;
			
			for(int f = 0; f<Brush.t_faces; f++)
			{
				if (dev&&dev2) cout << "    Face " << f << endl;
				vertex v1, v2, v3; gvector vec1, vec2;
				
				face &Face = Brush.Faces[f];
				Face.Vertices = new vertex[3];
				Face.vcount = 3;
				
				v1.x = stof(b_import[i][1]);
				v1.y = stof(b_import[i][2]);
				v1.z = stof(b_import[i][3]); if (dev&&dev2) cout << "     v1 " << v1 << endl;
				v2.x = stof(b_import[i][4]);
				v2.y = stof(b_import[i][5]);
				v2.z = stof(b_import[i][6]); if (dev&&dev2) cout << "     v2 " << v2 << endl;
				v3.x = stof(b_import[i][7]);
				v3.y = stof(b_import[i][8]);
				v3.z = stof(b_import[i][9]); if (dev&&dev2) cout << "     v3 " << v3 << endl;
				vec1.x = stod(b_import[i][11]);
				vec1.y = stod(b_import[i][12]);
				vec1.z = stod(b_import[i][13]); if (dev&&dev2) cout << "     vecX " << vec1 << endl;
				vec2.x = stod(b_import[i][15]);
				vec2.y = stod(b_import[i][16]);
				vec2.z = stod(b_import[i][17]); if (dev&&dev2) cout << "     vecY " << vec2 << endl;
				Face.Vertices[0] = v1;
				Face.Vertices[1] = v2;
				Face.Vertices[2] = v3;
				Face.VecX = vec1;
				Face.VecY = vec2;
				Face.Texture = b_import[i][10]; if (dev&&dev2) cout << "     Tex " << Face.Texture << endl;
				Face.ShiftX = stof(b_import[i][14]); if (dev&&dev2) cout << "     Face.ShiftX " << Face.ShiftX << endl;
				Face.ShiftY = stof(b_import[i][18]); if (dev&&dev2) cout << "     Face.ShiftY " << Face.ShiftY << endl;
				Face.Rot = stof(b_import[i][19]); // face rotation is being ignored on import
				Face.ScaleX = stof(b_import[i][20]); if (dev&&dev2) cout << "     Face.ScaleX " << Face.ScaleX << endl;
				Face.ScaleY = stof(b_import[i][21]); if (dev&&dev2) cout << "     Face.ScaleY " << Face.ScaleY << endl;
				//Face.GetfID();
				 if (dev&&dev2) cout << endl;
				i++;
				//if (dev) cout << "Brush: " << b << ", Face: " << f << ", Verts: " << Face.Vertices[0] << Face.Vertices[1] << Face.Vertices[2];
				//if (dev) cout << ", Total B-Faces: " << mGroup->Brushes[b].t_faces << endl;
			}
		}
		
		// create Vertex Sorting List for each Brush
		/*if (dev) cout << "  Creating Vertex Sorting List for each Brush..." << endl;
		for (int b = 0; b<tbc; b++)
		{
			brush &Brush = mGroup->Brushes[b];
			
			//if (Brush.vlist!=nullptr) delete Brush.vlist;
			Brush.vlist = new int[Brush.t_faces-2];	// Create Vertex Order List
			for (int i = 0; i < Brush.t_faces-2; i++) Brush.vlist[i] = -1;
		}*/
		if (dev) getch();
	}
}

void entity::RotateOrigin(float x, float y, float z, vertex nOrigin)
{
	bool dev = 0;
	
	Origin.rotateOrigin(x,y,z,nOrigin);
}

void entity::RotateEntity(Euler RotAngles, bool UpdateEuler)
{
	bool dev = 0;
	if (dev) cout << " Rotating Matrix by " << RotAngles << " Updating Euler (" << UpdateEuler << ")" <<endl;
	Matrix RotMatrix;
	RotMatrix.EulerToMatrix(RotAngles); // create Rotation Matrix from Euler Angles
	if (dev) cout << "   Original Angle Matrix: " << endl << AMatrix << endl;
	if (dev) cout << "   Rotation Matrix from Euler: " << endl << RotMatrix << endl;
	
	Matrix NewAMatrix = MatrixMultiply(RotMatrix,AMatrix); // Multiply Rotation Matrix with existing Angle Matrix to get new Angle Matrix
	
	if (dev) cout << "   New Angle Matrix (Multiplied): " << endl << NewAMatrix << endl;
	
	if (UpdateEuler)
		Angles = NewAMatrix.MatrixToEuler(); // get new Entity Angles from new Angle Matrix
	else
		Angles = RotAngles;
	
	if (dev) cout << "   Euler Angles: " << Angles << endl;
	
	AMatrix = NewAMatrix; // update Entities Angle Matrix
}

void entity::CopySimple(entity &Source)
{
	content = Source.content;
	head_end = Source.head_end;
	eID = Source.eID;
	dID = Source.dID;
	SecID = Source.SecID;
	type = Source.type;
	IsDetail = Source.IsDetail;
	Angles = Source.Angles;
	AMatrix = Source.AMatrix;
	Origin = Source.Origin;
	key_classname = Source.key_classname;
	key_target = Source.key_target;
	key_targetname = Source.key_targetname;
	draw = Source.draw;
	
	d_enable = Source.d_enable;
	d_autopitch = Source.d_autopitch;
	d_autoyaw = Source.d_autoyaw;
	d_separate = Source.d_separate;
	d_pos = Source.d_pos;
	d_autoname = Source.d_autoname;
	d_pos_rand = Source.d_pos_rand;
	d_rotz_rand = Source.d_rotz_rand;
	d_movey_rand = Source.d_movey_rand;
	d_draw = Source.d_draw;
	d_skip = Source.d_skip;
	d_draw_rand = Source.d_draw_rand;
	
	Keys.resize(Source.Keys.size());
	for (int k = 0; k<Keys.size(); k++)
		Keys[k] = Source.Keys[k];
}

void entity::ScaleOrigin(float n, vertex SOrigin)
{
	if(n!=0)
	{
		Origin.move(-SOrigin.x,-SOrigin.y,-SOrigin.z);
		
		Origin.x *= n;
		Origin.y *= n;
		Origin.z *= n;
		
		Origin.move(SOrigin.x,SOrigin.y,SOrigin.z);
	}
}


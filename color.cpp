#include "color.h"

cColorList gColorList;

cColorList::cColorList(){
	add(128,128,128,255);  // Unknowm - 0
	add(255,0,0,255);		   // Terrorist - 1
	add(0,100,255,255);		 // Counter-Terrorist - 2
	add(0,255,0,255);		   // Aimbot Target - 3
	add(255,255,0,0);		   // Entity Esp Weapons - 4
	add(255,200,0,255);		 // Entity Esp - 5
	add(23,129,247,180);	 // Menu Title - 6
	add(225,225,225,180);	 // Menu Content - 7
	add(255,255,255,0);		 // Menu - Deselected - 8
	add(20,255,20,0);		   // Menu - Selected - 9
	add(255,255,255,200);  // Crosshair - 10
	add(255,0,0,200);      // Crosshair - 11
	add(255,150,0,255);		 // Player Esp - 12
	add(0,255,255,255);		 // Entity Esp 2 - 13
	add(255,0,255,255);		 // Follow - 14

	add(255,0,0,200);      // Health - 15
	add(255,127,0,200);    // Health - 16
	add(255,255,0,200);		 // Health - 17
	add(127,255,0,200);		 // Health - 18
	add(0,255,0,200);		   // Health - 19
}

void cColorList::add(int r, int g, int b, int a){
	ColorEntry tmp;
	tmp.r=r;tmp.g=g;tmp.b=b;tmp.a=a;
	tmp.fr=r/255;
	tmp.fg=g/255;
	tmp.fb=b/255;
	tmp.fa=a/255;
	entrys.push_back(tmp);
}

ColorEntry &cColorList::get(unsigned int index){
	if(index<entrys.size())
		return entrys[index];
	else
		return entrys[0];
}

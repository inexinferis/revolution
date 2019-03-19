#ifndef COLOR_H
#define COLOR_H

#include <vector>

typedef struct _ColorEntry{
  union{
    struct{
      int r,g,b,a;
    };
    unsigned int color;
  };
  union{
    struct{
      float fr,fg,fb,fa;
    };
    float fcolor[4];
  };
}ColorEntry,*PColorEntry;

using namespace std;
class cColorList{
public:
	cColorList();
	ColorEntry &get(unsigned int index);
private:
	void add(int r, int g, int b, int a);
	vector<ColorEntry> entrys;
};

extern cColorList gColorList;

#endif //COLOR_H

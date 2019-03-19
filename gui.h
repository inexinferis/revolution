#ifndef GUI_H
#define GUI_H

using namespace std;

#include <list>

struct HUDMsg {
  char msg[1024];
  int mr,mg,mb;
  unsigned int timeout;
};

class cGui{
public:
  cGui():timeout(0){};
  void Init(int sWidth,int sHeight,int sFontWidth,int sFontHeight);
	void DrawWindow(int x, int y, int w, int h, bool bg, const char* title);
	void DrawInfoBox();
	void DrawNetStatusBox();
	void DrawSpyCam();
	void DrawScreenShot();
	void DrawTime();
	void DrawMessages();
	void AddMessage(int r, int g, int b, unsigned int timeout,const char *fmt,...);
private:
	void DrawTitleBox(int x, int y, int w, int h,const char* title);
	void DrawContentBox(int x, int y, int w, int h, bool bg);
	int r[64],g[64],b[64];
	int iWidth,iHeight;
	//string size
  int iFontWidth,iFontHeight;
  char msg[2056];
  int mr,mg,mb;
  unsigned int timeout;
  std::list<HUDMsg> lMsgs;
};

extern cGui gGui;

#endif //GUI_H

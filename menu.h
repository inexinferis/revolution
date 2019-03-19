#ifndef MENU_H
#define MENU_H

#define UNSELECTED (unsigned int)-1

typedef enum{
  M_CVAR,
  M_FUNC
}mTypes;

struct MenuEntry;class cMenu;
typedef void (cMenu::*funcmenu)(MenuEntry *sMenus,void *args);
typedef bool (*tShouldShow)();

struct MenuEntry{
  mTypes mType;
  char cCVarName[24];
  bool bSelected;
  bool bExpanded;
  tShouldShow pShouldShow;
	union{
	  float *fValue;
	  funcmenu pFunc;
	};
	union{
    void *args;
    float fStep;
	};
	float fMax,fMin;
	int iLevel;
	vector<MenuEntry> sMenus;
};
class cPlayer;

class cMenu{
  public:
    cMenu();
    ~cMenu();
    void Init(int sWidth,int sHeight,int sFontWidth,int sFontHeight);
    void Draw();
    int Key(int keynum);
    void AutoName();

    bool active;
  private:
    vector<MenuEntry>* AddCVarMenu(vector<MenuEntry> *menu,const char* name,float *value,float step,float max,float min,tShouldShow pShouldShow=NULL);
    vector<MenuEntry>* AddFuncMenu(vector<MenuEntry> *menu,const char* name,funcmenu func,void *args=NULL,tShouldShow pShouldShow=NULL);
    void UpdateRecursive(vector<MenuEntry> &menu,int level=0);
    bool ChildSelected(vector<MenuEntry> &menu);
    void Clear(vector<MenuEntry> &menu);
    void Update();
    //user defined functions
    void UpdateWayPoints(MenuEntry *sMenus,void *args);
    void ClearWayPoint(MenuEntry *sMenus,void *args);
    void ClearAllWayPoints(MenuEntry *sMenus,void *args);
    void UpdateUsersList(MenuEntry *sMenus,void *args);
    //nicks
    void ChangeName(MenuEntry *sMenus,void *args);
    //save and load
    void Load(MenuEntry *sMenus,void *args);
    void Save(MenuEntry *sMenus,void *args);
    //string size
    int iFontWidth,iFontHeight;
    int iLeft,iHeight;
    unsigned int iSelected;
    vector<MenuEntry> menus;
    vector<MenuEntry*> entrys;
    vector<MenuEntry>* playerslist;
    vector<MenuEntry>* waypoints;
    vector<string> nicklist;
};

extern cMenu gMenu;

#endif //MENU_H

#ifndef CONSOLE_H
#define CONSOLE_H

#include <string>

using namespace std;
template < typename T, int N >
class WrapHistory{
protected:
	void increase(int& idx) {
    ++idx;
    if(idx==N)
      idx=0;
  }
	void decrease(int& idx){
    --idx;
    if(idx<0 )
      idx=(N-1);
  }
public:
	WrapHistory() : readpos(0), writepos(0) {}
	void add(const T& entry){
		increase(writepos);
		entries[writepos]=entry;
		reset();
	}
	void reset(){
    readpos = writepos;
  }
	T&   read (){
    return entries[readpos];
  }
	void prev (){
    decrease(readpos);
    if(readpos==writepos)
      increase(readpos);
  }
	void next (){
    if(readpos!=writepos)
      increase(readpos);
  }
protected:
	int readpos;
	int writepos;
	T entries[N];
};

class cConsole{
  public:
    cConsole() : editline(editbuf+1) {
  		editbuf[0] = '>';
  		editbuf[1] =  0 ;
  		editbuf[EDIT_MAX-1] = -1;
  		cursorpos = editline;
  		curColorTag = 0;
  		mode = MODE_EXECUTE;
  		hist_direction=DIR_BACK;
  	}
  	void Init(int sWidth,int sHeight,int sFontWidth,int sFontHeight);
  	int active,mode;
    enum{MODE_EXECUTE=0, MODE_CHAT=1};
  	void Draw();
  	void Echo(const char *fmt, ... );
  	void Say (const char* text, const char* name, int team );
  	int Key (int ch);

  protected:
  	WrapHistory<string,80> lines;
  	WrapHistory<string,80> history;
    enum { DIR_BACK, DIR_FORWARD } hist_direction;
  	enum { EDIT_MAX=160 };
  	char  editbuf[EDIT_MAX];
  	char* const editline;
  	char* cursorpos;
  	int curColorTag;  // ==0 <=> 'a'
  	void DrawConsoleLine( const string& str, int x, int y );
  private:
    int cX,cY,cW,cH;
    //string size
    int iFontWidth,iFontHeight;
};

extern cConsole gConsole;

#endif //CONSOLE_H

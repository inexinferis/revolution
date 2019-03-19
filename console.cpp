#include "interpreter.h"
#include "console.h"
#include "drawing.h"
#include "color.h"
#include "main.h"
#include "gui.h"

cConsole gConsole;

void cConsole::Init(int sWidth,int sHeight,int sFontWidth,int sFontHeight){
  cW=int(sWidth/2);
  cH=int(sHeight/2);
  cX=int(cW/2);
  cY=int(cH/2);
  iFontWidth=sFontWidth;
  iFontHeight=sFontHeight;
}

void cConsole::Draw(){
	gGui.DrawWindow(cX,cY,cW,cH+2,true,"Console");
	int x = cX+3;int y = cY+cH-(iFontHeight+1);
	pfnTintRGBA(cX,cY+cH-(iFontHeight+3),cW-2,1,255,255,255,255);
	ColorEntry &color=gColorList.get(8);
	DrawString(false,x,y,color.r,color.g,color.b,editbuf);

	// get len of text to cursor.
  int length,height;char save;// get cursor size:
	save=cursorpos[0];cursorpos[0]=0;
	gEngfuncs.pfnDrawConsoleStringLen(editbuf,&length,&height);
  cursorpos[0]=save;
  pfnTintRGBA(x+length-1,y+1,2,height-1,color.r,color.g,color.b,255);

  lines.reset();y-=10;
	for(;;){
		y-=iFontHeight+2;
		if(y<cY)
      break;
		string& curLine=lines.read();
		lines.prev();
		DrawConsoleLine(curLine,x,y);
	}
}

void cConsole::Echo(const char *fmt,...){
	static char buf[512];
	va_list va_alist;
	va_start(va_alist,fmt);
	_vsnprintf(buf,sizeof(buf),fmt,va_alist);
	va_end(va_alist);
	lines.add(buf);
}

void cConsole::Say(const char* text,const char* name,int team){
	if(team==1) {
    Echo("&r%s :",name);
    Echo(" %s",text);
  }else{
    Echo("&b%s :",name);
    Echo(" %s",text);
  }
}

void cConsole::DrawConsoleLine( const string& str,int x,int y){
  const char* line = str.c_str();
	char  buf[256];
	char* bufpos;
	for(;;){
		bufpos=buf;
		for(;;){
      *bufpos=*line;
      if(!*line||*line=='&')
        break;
      ++line;
      ++bufpos;
    }
		bufpos[0]=0;bufpos[1]=0;
		int length,height;
    ColorEntry &color=gColorList.get(9);
		gEngfuncs.pfnDrawConsoleStringLen(buf,&length,&height);
		DrawString(false,x,y,color.r,color.g,color.b,buf);
		x+=length;
		if(*line=='&'){
			unsigned char ch = *++line - 'a';
			if(ch<26)
        curColorTag=ch;
			else
        break;
			if(!*++line)
        break;
		}else
			break;
	}
	curColorTag=0;
}

int cConsole::Key(int ch){
	char* pos;
	//ctrol keys don't catched...
	if(ch>=K_ALT&&ch<=K_SHIFT)
    return 0;
  //remap
  switch(ch){
    case K_KP_ENTER:ch=K_ENTER;break;
    case K_KP_INS:ch='0';break;
    case K_KP_DEL:ch='.';break;
    case K_KP_SLASH:ch='/';break;
    case K_KP_MULT:ch='*';break;
    case K_KP_MINUS:ch='-';break;
    case K_KP_PLUS:ch='+';break;
    default:
      if(ch>=K_KP_HOME&&ch<=K_KP_PGDN)
        ch=215-ch+((ch-1)%3)*2;
      else if(GetAsyncKeyState(VK_SHIFT)){
        switch(ch){
          case '1':ch='!';break;
          case '2':ch='"';break;
          case '3':ch='·';break;
          case '4':ch='$';break;
          case '5':ch='%';break;
          case '6':ch='&';break;
          case '7':ch='/';break;
          case '8':ch='(';break;
          case '9':ch=')';break;
          case '0':ch='=';break;
          case ',':ch=';';break;
          case '.':ch=':';break;
          case '-':ch='?';break;
          case '/':ch='_';break;
          case '=':ch='+';break;
          default:break;
        }
      }else{
        switch(ch){
          case '-':ch='#';break;
          case '/':ch='-';break;
          case '=':ch='@';break;
          default:break;
        }
      }
    break;
  }
  //analize
  switch(ch){
		case K_BACKSPACE: // backspace
			if(cursorpos==editline)
        return 0;
			pos=--cursorpos;
			while(pos[0]){
        pos[0]=pos[1];++pos;
      }
			return 0;
    case K_DEL: // supr
      if(cursorpos==editline+strlen(editline))
        return 0;
      pos=cursorpos;
			while(pos[0]){
        pos[0]=pos[1];++pos;
      }
      return 0;
		case K_UPARROW: // uparrow
			if(hist_direction!=DIR_BACK) {
			  history.prev();
			  history.prev();
			  hist_direction=DIR_BACK;
      }
			strcpy(editline,history.read().c_str());
			cursorpos=editline+strlen(editline);
			history.prev();
			return 0;
		case K_DOWNARROW: // downarrow
			if(hist_direction==DIR_BACK){
			  history.next();
			  history.next();
			  hist_direction=DIR_FORWARD;
      }
			strcpy(editline,history.read().c_str());
			cursorpos=editline+strlen(editline);
			history.next();
			return 0;
		case K_LEFTARROW: // leftarrow
			if(cursorpos!=editline)
        --cursorpos;
			return 0;
		case K_RIGHTARROW: // righttarrow
			if(cursorpos!=(editline+strlen(editline)))
        ++cursorpos;
			return 0;
    case K_HOME: // start
      cursorpos=editline;
      return 0;
		case K_END: // end
      cursorpos=editline+strlen(editline);
      return 0;
		case K_ENTER: // enter
			if(!strcmp(editline,"...")){
				if(mode==MODE_EXECUTE) {
          mode=MODE_CHAT;
          Echo("&b*** &aCONSOLE: &wCHAT MODE &b***");
        }else{
          mode=MODE_EXECUTE;
          Echo("&b*** &aCONSOLE: &wEXEC MODE &b***");
        }
			}else if(mode==MODE_EXECUTE){
				Echo("&x%s",editbuf);
				if(editline[0]){
					cmd.exec(editline);
					history.add(editline);
					history.reset();
				}
			}else if(mode==MODE_CHAT){
				char* text = editline;char buf[256];
				while(*text==' ')++text;
				sprintf(buf,"say \"%s\"",text);
				gEngfuncs.pfnClientCmd(buf);
			}
			editline[0]=0;
			cursorpos = editline;
			return 0;
		default:
		  if(isprint(ch)){
        if(strlen(editbuf)>(EDIT_MAX-4))
          return 0;
        if(!cursorpos[0]) {
          cursorpos[0]=ch;
          ++cursorpos;
          cursorpos[0]=0;
          return 0;
        }
        pos=editbuf+strlen(editbuf)+1;
        while(pos>cursorpos){
          pos[0]=pos[-1];
          --pos;
        }
        *cursorpos=ch;
        ++cursorpos;
        return 0;
			}
  }
	return 1;
}

#include <fstream>

#include "interpreter.h"
#include "console.h"
#include "utils.h"
#include "main.h"
#include "gui.h"

cCommandInterpreter cmd;
bool bKeyDown=false;

struct Private_Data{
	MapStringInt theMap;
	MapStringInt::iterator pos;
};

StringFinder::StringFinder(){
	data=new Private_Data;
}

StringFinder::~StringFinder(){
	delete (Private_Data*)data;
}

void StringFinder::clear(){
	THE_MAP.clear();
}

void StringFinder::add(const char* str, int num){
	if(!str || !*str){ return;}
    typedef MapStringInt::value_type Entry;
    THE_MAP.insert(Entry(str,num));
}

void StringFinder::erase( const char* str ){
	MapStringInt::iterator foundPos=THE_MAP.find(std::string(str));
	if(foundPos!=THE_MAP.end())
	  THE_MAP.erase(foundPos);
}

bool StringFinder::find(const char* str){
	if(!str||!*str)
    return false;
	MapStringInt::iterator foundPos=THE_MAP.find(std::string(str));
	if(foundPos==THE_MAP.end())
		return false;
	else
    num=(*foundPos).second;
	return true;
}

void StringFinder::it_start(){
	THE_POS=THE_MAP.begin();
	num=THE_POS->second;
	str=THE_POS->first.c_str();
}

bool StringFinder::it_running(){
	return (THE_POS!=THE_MAP.end());
}

void StringFinder::it_next(){
	++THE_POS;
	num=THE_POS->second;
	str=THE_POS->first.c_str();
}

cCommandInterpreter::cCommandInterpreter():needWait(false),initok(false){
}

void cCommandInterpreter::HandleCvarFloat(char* name, float* value){
	const char* arg1=cmd.argC(1);
	if(!*arg1){
		gConsole.Echo("CVAR \"%s\" = %.2f\n",name,*value);
		return;
	}
	if(!strcmp(arg1,"toggle")){
		if(*value)
      *value=0.0;
    else
      *value=1.0;
	}else if(!strcmp(arg1,"up")){
		*value+=cmd.argF(2);
	}else if(!strcmp(arg1,"down")){
		*value-=cmd.argF(2);
	}else
    *value=cmd.argF(1);
  gGui.AddMessage(255,255,255,1500,"%s changed to %.2f",name,*value);
}

void cCommandInterpreter::HandleCvarInt(char* name,int* value){
	const char* arg1=cmd.argC(1);
	if(!*arg1){
		gConsole.Echo("CVAR \"%s\" = %i\n",name,*value);
		return;
	}
	if(!strcmp(arg1,"toggle")){
		if(*value)
      *value=0;
    else
      *value=1;
	}else if(!strcmp(arg1,"up")){
		*value+=cmd.argI(2);
	}else if(!strcmp(arg1,"down")){
		*value-=cmd.argI(2);
	}else
    *value=cmd.argI(1);
  gGui.AddMessage(255,255,255,1500,"%s changed to %d",name,*value);
}

void cCommandInterpreter::exec(const char* cmdlist){
	string my_copy=cmdlist;
	char *from=const_cast<char*>(my_copy.c_str()),*to=from;
	do{
    while(*from==' '||*from=='\t'){// skip ws
      ++from;++to;
    }
    while(*to>=' '&&*to!=';'){// find end
      if(*to=='\"')
        do{
          ++to;
        }while(*to&&*to!='\"');
      else
        ++to;
    }
		if(from[0]=='/'&&from[1]=='/')
      return;
		if(from<to){
			char oldch=*to;
			*to=0;
			exec_one(from);
			*to=oldch;
		}
		if(!*to)
      break;
		++to;
		from=to;
    if(needWait){
      waitList.push_front(from);
      needWait=false;
      return;
    }
	}while(1);
}

void cCommandInterpreter::expandCommand(string& expand,bool down){
	if(expand[0]=='+'){
		if(down)
			exec(expand.c_str());
		else{
			expand[0]='-';
			exec(expand.c_str());
			expand[0]='+';
		}
	}else if((expand[0]=='.'||expand[0]=='#')&&expand[1]=='+'){
		if(down)
			exec(expand.c_str());
		else{
			expand[1]='-';
			exec(expand.c_str());
			expand[1]='+';
		}
	}else if(down)
    exec(expand.c_str());
}

void cCommandInterpreter::extractArguments(const char* const_args){
	preExecArgs.clear();
	char* args = const_cast<char*>(const_args);
	while(*args){
		while(*args&&*args<=' ')
      args++;
		if(!*args)
      break;
		char* start;
		if(*args=='\"') {
      start=++args;
      while(*args!='\"'&&*args)
        args++;
    }else{
      start=args;
      while(*args>' ')
        args++;
    }
		char last=*args;
		*args=0;
		preExecArgs.push_back(start);
		*args=last;
		if(*args=='\"')
      args++;
	}
}

void cCommandInterpreter::exec_one(const char* cur_cmd){
  cvar_s* ptr;
	if(*cur_cmd=='#'||*cur_cmd=='.'){
	  gEngfuncs.pfnClientCmd(const_cast<char*>(cur_cmd+1));
		return;
	}
	char command[32],*commandPos=command;
	const char *orig_cmd=cur_cmd;
	int commandCharsLeft=31;
	while(*cur_cmd>' '&&commandCharsLeft){
		*commandPos=tolower(*cur_cmd);
		commandPos++;cur_cmd++;
		commandCharsLeft--;
	}
	*commandPos=0;
	while(*cur_cmd>' ')cur_cmd++;
	if(names.find(command)){
		Entry& entry=entries[names.num];
		switch(entry.type){
  		case Entry::ALIAS:{
  			string& content=*(string*)(entry.data);
  			exec(const_cast<char*>(content.c_str()));
      }break;
  		case Entry::COMMAND:{
  			typedef void (*CmdFunc)();
  			CmdFunc function=(CmdFunc)(entry.data);
  			extractArguments(cur_cmd);
  			function();
      }break;
      case Entry::CVAR_INT:
        extractArguments(cur_cmd);
        HandleCvarInt(command,(int*)entry.data);
			break;
  		case Entry::CVAR_FLOAT:
  			extractArguments(cur_cmd);
  			HandleCvarFloat(command,(float*)entry.data);
      break;
  		case Entry::HL_CVAR:
  			extractArguments(cur_cmd);
  			ptr=gEngfuncs.pfnGetCvarPointer(command);
        if(ptr)HandleCvarFloat(command,&ptr->value);
      break;
      default:break;
		}
	}else{
		if(!gEngfuncs.pfnGetCvarPointer(command)){
      if((*cur_cmd!='w'&&*cur_cmd!='m')||*(cur_cmd+1)!='_')
        gEngfuncs.pfnClientCmd(const_cast<char*>(orig_cmd));
		}else{
			Add(command,Entry::HL_CVAR,NULL);
			extractArguments(cur_cmd);
			ptr=gEngfuncs.pfnGetCvarPointer(command);
      if(ptr)HandleCvarFloat(command,&ptr->value);
		}
	}
}

int cCommandInterpreter::KeyEvent(int scancode,bool down){
  if(scancode>255||!initok)return 1;
	const char* keyname=keyNames[scancode].c_str();
	if(*keyname&&binds.find(keyname)){
		string& expand=keyValues[binds.num];
		bKeyDown=down;
		expandCommand(expand,down);
		return 0;
	}
	return 1;
}

void cCommandInterpreter::FrameEvent(){
  list<string>::iterator it;
  if(waitList.size())
	for(it=waitList.begin();it!=waitList.end();){
    exec((*it).c_str());
    waitList.erase(it++);
  }
}

void cCommandInterpreter::execFile(const char* filename){
	if(strstr(filename,"..")){
		gConsole.Echo("forbidden.\n");
		return;
	}
	ifstream ifs(filename);
	if(!ifs)
		return;
	char buf[1024];
	while(ifs){
		ifs.getline(buf,550,10);
		if(!ifs)
      break;
		exec(buf);
	}
}

void cCommandInterpreter::Add( const char *NamE, int type, void* data){
  char name[36],*str=name;
  if(!*NamE)return;
	strncpy(name,NamE,31);
	name[31]=0;
	while(*str){
    *str=tolower(*str);
    str++;
  }
	if(names.find(name)){
		gConsole.Echo("\"%s\" is already registered.\n",name);
		return;
	}
	int index=entries.size();
	Entry tmp={type,data};
	entries.push_back(tmp);
	names.add(name,index);
}

void cCommandInterpreter::AddAlias(const char *NamE,const char* value){
  char name[36],*str=name;
  if(!*NamE)return;
	strncpy(name,NamE,31);
	name[31]=0;
	while(*str){
    *str=tolower(*str);
    str++;
  }
	if(names.find(name)){
		Entry& entry = entries[names.num];
		if(entry.type!=Entry::ALIAS){
			gConsole.Echo("\"%s\" is already registered.\n",name);
			return;
		}
		if(!*value)
      gConsole.Echo("\"%s\" is cleaned, original \"%s\"",name,((string*)(entry.data))->c_str());
		*(string*)(entry.data)=value;
		return;
	}
	if(gEngfuncs.pfnGetCvarPointer(name)){
		gConsole.Echo("\"%s\" is a HL cvar.\n",name);
		return;
	}
	int index=entries.size();
	Entry tmp={Entry::ALIAS,new string(value)};
	entries.push_back(tmp);
	names.add(name,index);
}

void cCommandInterpreter::AddBind(const char* KeY,const char* value){
  char key[36],*str=key;
  if(!*KeY)return;
	strncpy(key,KeY,31);
	key[31]=0;
	while(*str){
    *str=tolower(*str);
    str++;
  }
	if(binds.find(key)){
		string& expand=keyValues[binds.num];
		if(*value)
			expand=value;
		else
			gConsole.Echo("\"%s\" is bound to \"%s\"",key,expand.c_str());
    return;
	}
  for(int i=0;i<256;i++)
    if(keyNames[i]==key){
      if(*value){
        int index=keyValues.size();
        keyValues.push_back(value);
        binds.add(key,index);
      }else
        gConsole.Echo("\"%s\" is not bound",key);
      return;
    }
  gConsole.Echo("\"%s\" is not a valid key.\n",key);
}

void cCommandInterpreter::RemoveBind(const char* key){
  if(key){
    if(binds.find(key)){
      keyValues.erase(keyValues.begin()+binds.num);
      binds.erase(key);
    }else
      gConsole.Echo("\"%s\" is not bound",key);
  }else{//unbind all!
    binds.clear();
    keyValues.clear();
  }
}

void cCommandInterpreter::Init(){
  //Init Key Binds
  for(INT i=0;i<256;i++){
    CHAR key[2]={(CHAR)i,0};
    keyNames.push_back(key);
  }

  //set custom keys names...
	keyNames[K_TAB]="tab";
	keyNames[K_ENTER]="enter";
	keyNames[K_ESCAPE]="escape";
	keyNames[K_SPACE]="space";

  keyNames[K_BACKSPACE] = "backspace";
  keyNames[K_UPARROW] = "uparrow";
  keyNames[K_DOWNARROW] = "downarrow";
	keyNames[K_LEFTARROW] = "leftarrow";
	keyNames[K_RIGHTARROW] = "rightarrow";

  keyNames[K_ALT] = "alt";
	keyNames[K_CTRL] = "ctrl";
  keyNames[K_SHIFT] = "shift";
  keyNames[K_SHIFT] = "rshift";

  keyNames[K_F1] = "f1";
	keyNames[K_F2] = "f2";
	keyNames[K_F3] = "f3";
	keyNames[K_F4] = "f4";
	keyNames[K_F5] = "f5";
	keyNames[K_F6] = "f6";
	keyNames[K_F7] = "f7";
	keyNames[K_F8] = "f8";
	keyNames[K_F9] = "f9";
	keyNames[K_F10] = "f10";
	keyNames[K_F11] = "f11";
	keyNames[K_F12] = "f12";
	keyNames[K_INS] = "ins";
	keyNames[K_DEL] = "del";
	keyNames[K_PGDN] = "pgdn";
	keyNames[K_PGUP] = "pgup";
	keyNames[K_HOME] = "home";
	keyNames[K_END] = "end";

  keyNames[K_KP_HOME] = "kp_home";
  keyNames[K_KP_UPARROW] = "kp_uparrow";
  keyNames[K_KP_PGUP] = "kp_pgup";
  keyNames[K_KP_LEFTARROW] = "kp_leftarrow";
  keyNames[K_KP_5] = "kp_5";
  keyNames[K_KP_RIGHTARROW] = "kp_rightarrow";
  keyNames[K_KP_END] = "kp_end";
  keyNames[K_KP_DOWNARROW] = "kp_downarrow";
  keyNames[K_KP_PGDN] = "kp_pgdn";
  keyNames[K_KP_ENTER] = "kp_enter";
  keyNames[K_KP_INS] = "kp_ins";
  keyNames[K_KP_DEL] = "kp_del";
  keyNames[K_KP_SLASH] = "kp_slash";
  keyNames[K_KP_MULT] = "kp_mult";
  keyNames[K_KP_MINUS] = "kp_minus";
  keyNames[K_KP_PLUS] = "kp_plus";
  keyNames[K_CAPSLOCK] = "capslock";

  keyNames[K_MOUSE1] = "mouse1";
	keyNames[K_MOUSE2] = "mouse2";
	keyNames[K_MOUSE3] = "mouse3";
	keyNames[K_MOUSE4] = "mouse4";
	keyNames[K_MOUSE5] = "mouse5";

	keyNames[K_MWHEELDOWN] = "mwheeldown";
  keyNames[K_MWHEELUP] = "mwheelup";

	keyNames[K_PAUSE] = "pause";
	initok=true;
}

void cCommandInterpreter::save_cvars(int num){
  char tmp[100],ncvar[10];
  if(!num){
    for(names.it_start();names.it_running();names.it_next()){
      Entry& r=entries[names.num];
      if(r.type==Entry::CVAR_FLOAT){
        sprintf(tmp,"%0.2f",*(float*)r.data);
        WritePrivateProfileString("CVAR",names.str,tmp,inipath);
      }
      if(r.type==Entry::CVAR_INT){
        sprintf(tmp,"%d",*(int*)r.data);
        WritePrivateProfileString("CVAR",names.str,tmp,inipath);
      }
    }
  }else{
    sprintf(ncvar,"CVAR%d",num);
    for(names.it_start();names.it_running();names.it_next()){
      Entry& r=entries[names.num];
      if(r.type==Entry::CVAR_FLOAT){
        sprintf(tmp,"%0.2f",*(float*)r.data);
        WritePrivateProfileString(ncvar,names.str,tmp,inipath);
      }
      if(r.type==Entry::CVAR_INT){
        sprintf(tmp,"%d",*(int*)r.data);
        WritePrivateProfileString(ncvar,names.str,tmp,inipath);
      }
    }
  }
}

void cCommandInterpreter::load_cvars(int num){
	char tmp[100],ncvar[10];DWORD size=100;
	if(!iniFound)return;
  if(!num){
    for(names.it_start();names.it_running();names.it_next()){
      Entry& r=entries[names.num];
      if(r.type==Entry::CVAR_FLOAT){
        if(GetPrivateProfileString("CVAR",names.str,NULL,tmp,size,inipath))
          *(float*)r.data=atof(tmp);
      }
      if(r.type==Entry::CVAR_INT){
        if(GetPrivateProfileString("CVAR",names.str,NULL,tmp,size,inipath))
          *(int*)r.data=atoi(tmp);
      }
    }
  }else{
    sprintf(ncvar,"CVAR%d",num);
    for(names.it_start();names.it_running();names.it_next()){
      Entry& r=entries[names.num];
      if(r.type==Entry::CVAR_FLOAT){
        if(GetPrivateProfileString(ncvar,names.str,NULL,tmp,size,inipath))
          *(float*)r.data=atof(tmp);
      }
      if(r.type==Entry::CVAR_INT){
        if(GetPrivateProfileString(ncvar,names.str,NULL,tmp,size,inipath))
          *(int*)r.data=atoi(tmp);
      }
    }
  }
}

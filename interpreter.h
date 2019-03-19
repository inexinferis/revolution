#ifndef INTERPRETER_H
#define INTERPRETER_H

using namespace std;

#include <map>
#include <vector>
#include <string>
#include <list>
#include <stdlib.h>
#include "utils.h"

#define THE_MAP ((*((Private_Data*)data)).theMap)
#define THE_POS ((*((Private_Data*)data)).pos)

#define MAX_CUSTOM_CFG 5

typedef DWORD (WINAPI *tGetPrivateProfileString)(LPCSTR,LPCSTR,LPCSTR,LPSTR,DWORD,LPCSTR);
typedef BOOL (WINAPI *tWritePrivateProfileString)(LPCSTR,LPCSTR,LPCSTR,LPCSTR);

extern bool bKeyDown;

typedef map<string, int, less<string> > MapStringInt;

class StringFinder{
  public:
    void clear();
    void add(const char* the_string, int the_number);
    void erase( const char* str );
    bool find(const char* str);
    int num;
    const char*  str;
    void it_start  ();
    bool it_running();
    void it_next   ();
  private:
    void* data;
  public:
    StringFinder();
    ~StringFinder();
};

class cCommandInterpreter{
  public:
    cCommandInterpreter();
    void Init();
    void HandleCvarFloat(char* name, float* value);
    void HandleCvarInt(char* name,int* value);
    void AddCommand  ( const char *name, void(*func)()   ){ Add(name, Entry::COMMAND,  (void*) func); }
    void AddCvarInt  ( const char *name, int   *var      ){ Add(name, Entry::CVAR_INT,   var); }
    void AddCvarFloat( const char *name, float *var      ){ Add(name, Entry::CVAR_FLOAT, var); }
    void AddAlias    ( const char *name, const char* value);//string& content );
    void AddBind     (const char* key,const char* value);
    void RemoveBind  (const char* key);

    void exec    (const char* cmdlist);
    void exec_one(const char* cmd);
    void execFile(const char* filename);
    int  KeyEvent(int scancode,bool down);
    void FrameEvent();

    string&  argS(unsigned int i){ --i;if(i<preExecArgs.size()) return preExecArgs[i];                  else return emptyString; }
    const char*	 argC(unsigned int i){ --i;if(i<preExecArgs.size()) return const_cast<const char*>(preExecArgs[i].c_str()); else return "";}
    int      argI(unsigned int i){ --i;if(i<preExecArgs.size()) return atoi(preExecArgs[i].c_str());              else return  0;}
    float    argF(unsigned int i){ --i;if(i<preExecArgs.size()) return (float)atof(preExecArgs[i].c_str());    else return  0.0f;}

    void save_cvars(int num=0);
    void load_cvars(int num=0);
  protected:
    void   Add( const char* name, int type, void* data);
    struct Entry{
      enum { CVAR_FLOAT, CVAR_INT, COMMAND, ALIAS, HL_CVAR };
      int   type;
      void* data;
    };
  protected:
    void expandCommand(string& expand,bool down);
    void extractArguments(const char* args);

    StringFinder   names;
    vector<Entry>  entries;

    vector<string> preExecArgs;
    string         emptyString;

    StringFinder   binds;
    vector<string> keyNames;
    vector<string> keyValues;

    list<string>   waitList;
  public:
    bool needWait,initok;
  private:
    tGetPrivateProfileString pGetPrivateProfileString;
    tWritePrivateProfileString pWritePrivateProfileString;
};

extern cCommandInterpreter cmd;

#endif //INTERPRETER_H

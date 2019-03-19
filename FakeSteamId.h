#ifndef FAKESTEAMID_H
#define FAKESTEAMID_H

#include "utils.h"

VOID HookSendTo();
VOID UnHookSendTo();

typedef struct dictionary_s{
  char key[64];
  char value[256];
}dictionary_t;

typedef struct PACKED steampack_s{
  DWORD data;
  DWORD key1;
  DWORD ver;
  DWORD unk1;
  DWORD key2;
  DWORD key3;
  CHAR keys[16];
  CHAR unk[130];
}steampack_t;

class SteamProtocol{
  public:
    SteamProtocol();
    bool Parse(const char * &buffer,int &len);
    void FillSteamId(int len,signed int aon,bool adspace,char *buf);
    int MakeSteamId(char *buff);
    bool ParseConnect(PCHAR buffer,UINT len);
    char *DumpConnect(int* len);
    bool ParseChallenge(PCHAR buffer,UINT len);
    char *DumpChallenge(int* len);
  private:
    CHAR steam[6];
    CHAR svr[3];
    CHAR vr[3];
    CHAR unknown[16];
    INT arg1len,arg2len;
    dictionary_t args[20];
    steampack_t pack;
    static CHAR protocol[2048];
    static CHAR challenge[2048];
  public:
    DWORD header;
    CHAR connect[8];
    CHAR getchallenge[13];
};

#endif //FAKESTEAMID_H


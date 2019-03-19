#ifndef PLAYERS_H
#define PLAYERS_H

#include "main.h"
#include "color.h"

#define MAX_VPLAYERS  36
#define MAX_HOSTAGES  12
#define MAX_HITPOINTS 10
#define BOUND_VALUE(var,min,max) if((var)>(max)){(var)=(max);};if((var)<(min)){(var)=(min);}

enum{
  TEAM_UNKNOWN=0,
  TEAM_TERRORIST,
  TEAM_CT
};

enum{
  SEQUENCE_IDLE     = 0,
  SEQUENCE_SHOOT    = 1,
  SEQUENCE_RELOAD   = 2,
  SEQUENCE_DIE      = 4,
  SEQUENCE_THROW    = 8,
  SEQUENCE_ARM_C4   = 16,
  SEQUENCE_SHIELD   = 32,
  SEQUENCE_SHIELD_SIDE = 64
};

//Me
class cMe{
	public:
    cMe();
  	void Update();
  	void AddEntity(struct cl_entity_s *ent);
  	float GetDistanceFrom(const float* pos);
  	bool PathFree(float* to);
  	float CalcFovAngle(const float* origin);
    void PlayerMove(struct playermove_s *ppmove);
    void DoBunnyHop(struct usercmd_s *usercmd);
    void DoDuckHop(struct usercmd_s *usercmd);
  	void DoDuckJump(struct usercmd_s *usercmd);
  	void HandleZoom(unsigned int zoom);
  	void DoSpeed(float x);
  	bool ShouldSpeed();
  	void DoAutoBuy();
  	void DrawSpeed();
  	void CheckIsFlashed();
  	float GroundDistance();
  	void SetServerName(char *name);
  	void UpdateNetStatus();
  	bool IsFreeSpectator();

  	//me data
  	int index;
  	int alive;
  	int team;
  	int zoom;
  	int aimkey;
  	int trigger;
  	int inway;
  	int inspeed;

  	//Player data
    int iClip;
  	int iHealth;
  	int iArmor;
  	int iKills;
  	int iDeaths;
  	int iHs;
  	int iMoney;
  	int iFOV;
  	int iSpectator;

    //Player Move...
  	int pmFlags;
  	int pmMoveType;
  	float pmFrameTime;
  	float pmMaxSpeed;
  	float pmGroundSpeed;
  	float pmAirAccelerate;
  	float pmFallVelocity;
  	float lastflashtime;
  	vec3_t pmVelocity;
    vec3_t pmEyePos;
  	vec3_t pmViewAngles;
  	vec3_t pmOrigin;
  	//player calcrefdef
  	vec3_t viewangles;
  	vec3_t vieworg;
  	vec3_t punchangle;
  	//Spread/Recoil
  	vec3_t spreadangle;
  	vec3_t recoilangle;
    //Entity
  	cl_entity_t *ent;

  	float m_flNextPrimaryAttack;
    bool bShouldJump;
    bool bInDuck,bNeedDuck,bNoNeedDuck;

    //netgraph
    float fps;
    int ping;
    char gRemoteIP[28];
    char gLocalIP[28];
    char gServerName[128];
};

//Player
class cPlayer{
  public:
    cPlayer(int i);
    void Update();
    void AddEntity(struct cl_entity_s *ent);
    void SetWeapon(char *wname);
    bool IsValidEnt();
    bool CalcScreen(float *vecScreen);
    void DrawBlood();
    void DrawEsp();
    void SetDead();
    float GetDistanceFrom(const float* pos);
    ColorEntry &Color();

    int index,team,info,hitpoints,numhitboxes;
    float  fovangle,dynfov,fovrect,distance,follow;
    struct cl_entity_s *ent;
    vec3_t vHitbox,vOffsets;
    bool bVisible,bCanAim,bGotHead,bGotAnyVisiblePart,bSpawning;
    hud_player_info_t entinfo;
    vec3_t lastpos;
    vec3_t curorigin,lastorigin,velocity;
    vec3_t vBones[55],vHitboxs[21];
    char m_weapon[32];

    #if defined(_DEBUG)
    void Dump(char *detail);
    static void Dump(cl_entity_s *ent,char *detail);
    #endif // _DEBUG
};

class cPlayers{
  public:
    cPlayers(){
  		for(int i=0;i<MAX_VPLAYERS;i++)
        player[i]=new cPlayer(i);
    }
    ~cPlayers() {
      for(int i=0;i<MAX_VPLAYERS;i++)
        delete player[i];
    }

    inline cPlayer* operator [] (unsigned int i){
  		if(i>=MAX_VPLAYERS)
        return player[0];
  		else
        return player[i];
  	}

  	cPlayer* GetCurrentEntity();
  	cPlayer* GetViewModel();

  	inline unsigned int Size() {
      return MAX_VPLAYERS;
    }

    bool IsValidEnt(cl_entity_s *ent);

    bool IsValidPlayer(int index){
    	return player[index]->IsValidEnt();
    }

    void SetDead();
    void Update();
    void AddEntity(struct cl_entity_s *ent);
    void DrawEsp();

    #if defined(_DEBUG)
    //void DumpEnt(cl_entity_s *ent);
    #endif // _DEBUG

  private:
    cPlayer* player[MAX_VPLAYERS];
};

class cEspEnts{
  public:
    void DrawEsp();
    void AddEntity(struct cl_entity_s *ent, const char *modelname);
    bool IsValidEnt(cl_entity_s * ent);
    void Draw(float *origin,float *angles,const char *entname,bool scolor=false,bool bradar=true);
};

class cBomb{
  public:
    cBomb():isDropped(false),isPlanted(false),plantedTime(0){}
    void DrawTime();
    void DrawPosition();
    void Dropped(vec3_t &vec,int flag);
    void Pickup();
    bool IsDropped(){return isDropped;}
  private:
    bool   isDropped;
    char* GetString();
    bool   isPlanted;
    double plantedTime;
    char   timerstring[64];
    vec3_t origin;
};

class cHostage{
  public:
    cHostage():isAlive(false){}
    void DrawPosition();
    void SetAlive(vec3_t &vec){
      VectorCopy(vec,origin);
      isAlive=true;
    }
    void SetDead(){
      isAlive=false;
    }
    bool IsAlive(){return isAlive;}
  private:
    bool isAlive;
    vec3_t origin;
};

class cHostages{
public:
    inline cHostage& operator [] (unsigned int i){
  		if(i>=MAX_HOSTAGES)
        return hostage[0];
  		else
        return hostage[i];
  	}
  	void DrawPositions();
  private:
    cHostage hostage[MAX_HOSTAGES];
};

class cSoundMark{
  public:
    cSoundMark():player(false),radar(false),time(0),name(NULL),weapon(NULL){}
    static int GetIndex(int index,vec3_t origin,bool bupdate);
    static int FindClose(vec3_t origin);
    static void Draw(cSoundMark &mark);
    static void DrawSound();
    static void Reset(int i);

    vec3_t origin;
    int color;
    bool player,radar;
    unsigned int time;
    char *name,*weapon,desc[32];
};

extern cMe gMe;
extern cPlayers gPlayers;
extern cEspEnts gEspEnts;
extern cBomb gBomb;
extern cHostages gHostages;
extern cSoundMark gSoundMark[];

#endif //PLAYERS_H

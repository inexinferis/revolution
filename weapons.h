#ifndef WEAPONS_H
#define WEAPONS_H

#include <vector>

#include "main.h"

#define WEAPON_P228		1
#define	WEAPON_UNKNOWN1	2
#define	WEAPON_SCOUT	3
#define	WEAPON_HEGRENADE 4
#define	WEAPON_XM1014 5
#define	WEAPON_C4 6
#define	WEAPON_MAC10 7
#define	WEAPON_AUG 8
#define	WEAPON_SMOKEGRENADE 9
#define	WEAPON_ELITE 10
#define	WEAPON_FIVESEVEN 11
#define	WEAPON_UMP45 12
#define	WEAPON_SG550 13
#define	WEAPON_GALIL 14
#define	WEAPON_FAMAS 15
#define	WEAPON_USP 16
#define	WEAPON_GLOCK18 17
#define	WEAPON_AWP 18
#define	WEAPON_MP5 19
#define	WEAPON_M249 20
#define	WEAPON_M3 21
#define	WEAPON_M4A1 22
#define	WEAPON_TMP 23
#define	WEAPON_G3SG1 24
#define	WEAPON_FLASHBANG 25
#define	WEAPON_DEAGLE 26
#define	WEAPON_SG552 27
#define	WEAPON_AK47 28
#define	WEAPON_KNIFE 29
#define	WEAPON_P90 30

#define CLIPCAP_AK47 30
#define CLIPCAP_AUG 30
#define CLIPCAP_AWP 10
#define CLIPCAP_DEAGLE 7
#define CLIPCAP_ELITE 15
#define CLIPCAP_FIVESEVEN 20
#define CLIPCAP_G3SG1 20
#define CLIPCAP_GLOCK18 20
#define CLIPCAP_M249 100
#define CLIPCAP_M3 8
#define CLIPCAP_M4A1 30
#define CLIPCAP_MAC10 30
#define CLIPCAP_MP5 30
#define CLIPCAP_P228 13
#define CLIPCAP_P90 50
#define CLIPCAP_SCOUT 10
#define CLIPCAP_SG550 30
#define CLIPCAP_SG552 30
#define CLIPCAP_TMP 30
#define CLIPCAP_UMP45 25
#define CLIPCAP_USP 12
#define CLIPCAP_XM1014 7
#define CLIPCAP_GALIL 35
#define CLIPCAP_FAMAS 25

enum{
	BULLETTYPE_0=0,
	BULLETTYPE_1=1,
	BULLETTYPE_2=2,
	BULLETTYPE_3=3,
	BULLETTYPE_4=4,
	BULLETTYPE_5=5,
	BULLETTYPE_6=6,
	BULLETTYPE_7=7,
	BULLETTYPE_8=8,
	BULLETTYPE_9=9,
	BULLETTYPE_10=10,
	BULLETTYPE_11=11,
	BULLETTYPE_12=12,
	BULLETTYPE_13=13,
	BULLETTYPE_14=14,
	BULLETTYPE_15=15
};

#define BULLET_NONE BULLETTYPE_0
#define BULLET_AK47 BULLETTYPE_11
#define BULLET_AUG BULLETTYPE_12
#define BULLET_AWP BULLETTYPE_10
#define BULLET_DEAGLE BULLETTYPE_13
#define BULLET_ELITE BULLETTYPE_1
#define BULLET_FIVESEVEN BULLETTYPE_15
#define BULLET_G3SG1 BULLETTYPE_11
#define BULLET_GLOCK18 BULLETTYPE_1
#define BULLET_M249 BULLETTYPE_12
#define BULLET_M4A1 BULLETTYPE_12
#define BULLET_MAC10 BULLETTYPE_9
#define BULLET_MP5 BULLETTYPE_1
#define BULLET_P228 BULLETTYPE_15
#define BULLET_P90 BULLETTYPE_14
#define BULLET_SCOUT BULLETTYPE_11
#define BULLET_SG550 BULLETTYPE_12
#define BULLET_SG552 BULLETTYPE_12
#define BULLET_TMP BULLETTYPE_1
#define BULLET_UMP45 BULLETTYPE_9
#define BULLET_USP BULLETTYPE_9
#define BULLET_M3 BULLETTYPE_0
#define BULLET_XM1014 BULLETTYPE_0
#define BULLET_GALIL BULLETTYPE_12
#define BULLET_FAMAS BULLETTYPE_12

#define WALL_PEN0 0
#define WALL_PEN1 1
#define WALL_PEN2 2

#define USP_SILENCER (1 << 0)
#define GLOCK18_BURST (1 << 1)
#define M4A1_SILENCER (1 << 2)
#define ELITE_LEFT (1 << 4)

#define MAX_WEAPON_NAME 32

typedef struct _sWeapon{
	char name[MAX_WEAPON_NAME];
	unsigned int len;
	char command[MAX_WEAPON_NAME];
	int AmmoType;
	int Ammo2Type;
	int Max1;
	int Max2;
	int Slot;
	int SlotPos;
	int Id;
	int Flags;
	int ClipCap;	// Number of bullets the clip can hold
	int CAmmo;		// Current ammo in clip
	int XAmmo;		// Extra ammo
	bool Active;	// True if we have this weapon
	bool CurWeapon; // True if this is the current weapon
	int penetrate;
	weapon_data_t weapondata;
	float distance;
	int bullettype;
	float wallpierce1;
	float wallpierce2;
	int damage1;
	int damage2;
	float speed;
}sWeapon;

using namespace std;
class cWeapon{
  public:
    cWeapon(){tmpWeapon=NULL;};

    void Add(const char *weaponname, int ammo1type, int max1, int ammo2type, int max2, int slot, int slotpos, int id, int flags);
    void Set(int CurWeapon, int WeaponID, int Ammo);
    void Update(int WeaponBits);
    void SetGun(sWeapon *weapon);
    void SetAmmoX(int ID, int Count);
    void SetData(int index,weapon_data_t *data);
    void SetCurrent(int index , bool CurWeapon);
    void ClearData(int index);

    sWeapon * Get();
    sWeapon * GetPrimary();
    sWeapon * GetSecondary();

    int GetId();
    int GetPrimaryId();
    int GetSecondaryId();
    int GetIndexById(int WeaponID);
    int GetIdByIndex(int index);

    const char * GetCmd();
    const char * GetPrimaryCmd();
    const char * GetSecondaryCmd();

    const char * GetName();
    const char * GetPrimaryName();
    const char * GetSecondaryName();
    const char * GetNameById(int Id);
    const char * GetNameByModel(int weaponmodel);

    int GetClipCap();
    int GetAmmo();
    int GetAmmoX();
    int GetPrimaryAmmo();
    int GetPrimaryAmmoX();
    int GetSecondaryAmmo();
    int GetSecondaryAmmoX();

    int GetDamage();
    int GetBulletType();
    int GetPenetration();
    float GetDistance();
    float GetWallPierce();
    float GetSpeed();

    bool HasPrimary();
    bool HasSecondary();
    bool HasName(const char *weaponname);

    bool HasAmmo();
    bool PrimaryHasAmmo();
    bool SecondaryHasAmmo();

    bool IsPrimary();
    bool IsSecondary();
    bool IsSilenced();
    bool IsPistol();
    bool IsKnife();
    bool IsNonAttack();
    bool IsC4();
    bool IsGun();
    bool IsNade();
    bool IsInBurst();
    bool IsReloading();
    bool IsCurrent(int index);
    bool IsSnipper();

    bool CanAttack();

    int  GetSequenceInfo(int i);
    float GetWeaponViewOffset(char* model);
  private:
    sWeapon *tmpWeapon;
};

extern cWeapon gWeapon;
extern vector<sWeapon> vWeapons;

#endif //WEAPONS_H

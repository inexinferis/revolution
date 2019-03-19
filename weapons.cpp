#include "weapons.h"
#include "main.h"

cWeapon gWeapon;
vector<sWeapon> vWeapons;

void cWeapon::Add(const char *weaponname,int ammo1type,int max1,int ammo2type,int max2,int slot,int slotpos,int id,int flags){
	int index,len;sWeapon dummy;
	len=strlen(weaponname);
	if(len>MAX_WEAPON_NAME)
		return;

	vWeapons.push_back(dummy);
	index=vWeapons.size()-1;

	memcpy(vWeapons[index].command,weaponname,len+1);

	if(len>7&&!memcmp(weaponname,"weapon_",7)){
		weaponname=weaponname+7;
		vWeapons[index].len=len-7;
	}else
		vWeapons[index].len=len;

	if(vWeapons[index].len==7&&!memcmp(weaponname,"mp5navy",7))
		vWeapons[index].len=3;

	memcpy(vWeapons[index].name,weaponname,vWeapons[index].len);
	vWeapons[index].name[vWeapons[index].len]=0;

	vWeapons[index].AmmoType=ammo1type;
	if(max1==255)
		vWeapons[index].Max1=-1;
	else
		vWeapons[index].Max1=max1;

	vWeapons[index].Ammo2Type=ammo2type;
	if(max2==255)
		vWeapons[index].Max2=-1;
	else
		vWeapons[index].Max2=max2;

	vWeapons[index].Slot=slot;
	vWeapons[index].SlotPos=slotpos;
	vWeapons[index].Id=id;
	vWeapons[index].Flags=flags;
	vWeapons[index].CAmmo=0;
	vWeapons[index].XAmmo=0;
	vWeapons[index].Active=false;
	vWeapons[index].CurWeapon=false;

	SetGun(&vWeapons[index]);
	ClearData(index);
}

void cWeapon::SetData(int index,weapon_data_t *data){
  memcpy(&vWeapons[index].weapondata,data,sizeof(weapon_data_t));
}

void cWeapon::ClearData(int index){
  memset(&vWeapons[index].weapondata,0,sizeof(weapon_data_t));
}

void cWeapon::Update(int WeaponBits){
	static int OldWeaponBits=0;
	if(WeaponBits!=OldWeaponBits){
		OldWeaponBits=WeaponBits;
		for(UINT i=0;i<vWeapons.size();i++){
			if(WeaponBits &(1 << vWeapons[i].Id)){
				vWeapons[i].Active=true;
			}else{
				vWeapons[i].Active=false;
				vWeapons[i].CAmmo=0;
			}
		}
	}
}

void cWeapon::SetGun(sWeapon *weapon){
	switch(weapon->Id){
		case WEAPON_P228:
			weapon->ClipCap=CLIPCAP_P228;
			weapon->penetrate=WALL_PEN0;
			weapon->distance=4096.0f;
			weapon->wallpierce1=0.8f;
			weapon->wallpierce2=0.0f;
			weapon->damage1=32;
			weapon->damage2=0;
			weapon->bullettype=BULLET_P228;
			weapon->speed=5;
			break;
		case WEAPON_SCOUT:
			weapon->ClipCap=CLIPCAP_SCOUT;
			weapon->penetrate=WALL_PEN2;
			weapon->distance=8192.0f;
			weapon->wallpierce1=0.98f;
			weapon->wallpierce2=0.0f;
			weapon->damage1=75;
			weapon->damage2=0;
			weapon->bullettype=BULLET_SCOUT;
			weapon->speed=12;
			break;
		case WEAPON_HEGRENADE:
			weapon->ClipCap=0;
			weapon->penetrate=WALL_PEN0;
			weapon->distance=0.0f;
			weapon->wallpierce1=0.0f;
			weapon->wallpierce2=0.0f;
			weapon->damage1=0;
			weapon->damage2=0;
			weapon->bullettype=BULLET_NONE;
			weapon->speed=0;
			break;
		case WEAPON_XM1014:
			weapon->ClipCap=CLIPCAP_XM1014;
			weapon->penetrate=WALL_PEN0;
			weapon->distance=0.0f;
			weapon->wallpierce1=0.0f;
			weapon->wallpierce2=0.0f;
			weapon->damage1=0;
			weapon->damage2=0;
			weapon->bullettype=BULLET_XM1014;
			weapon->speed=5;
			break;
		case WEAPON_C4:
			weapon->ClipCap=0;
			weapon->penetrate=WALL_PEN0;
			weapon->distance=0.0f;
			weapon->wallpierce1=0.0f;
			weapon->wallpierce2=0.0f;
			weapon->damage1=0;
			weapon->damage2=0;
			weapon->bullettype=BULLET_NONE;
			weapon->speed=0;
			break;
		case WEAPON_MAC10:
			weapon->ClipCap=CLIPCAP_MAC10;
			weapon->penetrate=WALL_PEN0;
			weapon->distance=8192.0f;
			weapon->wallpierce1=0.82f;
			weapon->wallpierce2=0.0f;
			weapon->damage1=29;
			weapon->damage2=0;
			weapon->bullettype=BULLET_MAC10;
			weapon->speed=5;
			break;
		case WEAPON_AUG:
			weapon->ClipCap=CLIPCAP_AUG;
			weapon->penetrate=WALL_PEN1;
			weapon->distance=8192.0f;
			weapon->wallpierce1=0.96f;
			weapon->wallpierce2=0.0f;
			weapon->damage1=32;
			weapon->damage2=0;
			weapon->bullettype=BULLET_AUG;
			weapon->speed=5;
			break;
		case WEAPON_SMOKEGRENADE:
			weapon->ClipCap=0;
			weapon->penetrate=WALL_PEN0;
			weapon->distance=0.0f;
			weapon->wallpierce1=0.0f;
			weapon->wallpierce2=0.0f;
			weapon->damage1=0;
			weapon->damage2=0;
			weapon->bullettype=BULLET_NONE;
			weapon->speed=0;
			break;
		case WEAPON_ELITE:
			weapon->ClipCap=CLIPCAP_ELITE;
			weapon->penetrate=WALL_PEN0;
			weapon->distance=8192.0f;
			weapon->wallpierce1=0.75f; // left
			weapon->wallpierce2=0.75f; // right
			weapon->damage1=20; // left
			weapon->damage2=27; // right
			weapon->bullettype=BULLET_ELITE;
			weapon->speed=5;
			break;
		case WEAPON_FIVESEVEN:
			weapon->ClipCap=CLIPCAP_FIVESEVEN;
			weapon->penetrate=WALL_PEN0;
			weapon->distance=4096.0f;
			weapon->wallpierce1=0.885f;
			weapon->wallpierce2=0.0f;
			weapon->damage1=14;
			weapon->damage2=0;
			weapon->bullettype=BULLET_FIVESEVEN;
			weapon->speed=5;
			break;
		case WEAPON_UMP45:
			weapon->ClipCap=CLIPCAP_UMP45;
			weapon->penetrate=WALL_PEN0;
			weapon->distance=8192.0f;
			weapon->wallpierce1=0.82f;
			weapon->wallpierce2=0.0f;
			weapon->damage1=30;
			weapon->damage2=0;
			weapon->bullettype=BULLET_UMP45;
			weapon->speed=5;
			break;
		case WEAPON_SG550:
			weapon->ClipCap=CLIPCAP_SG550;
			weapon->penetrate=WALL_PEN1;
			weapon->distance=8192.0f;
			weapon->wallpierce1=0.98f;
			weapon->wallpierce2=0.0f;
			weapon->damage1=40;
			weapon->damage2=0;
			weapon->bullettype=BULLET_SG550;
			weapon->speed=8;
			break;
		case WEAPON_GALIL:
			weapon->ClipCap=CLIPCAP_GALIL;
			weapon->penetrate=WALL_PEN1;
			weapon->distance=8192.0f;
			weapon->wallpierce1=0.96f;
			weapon->wallpierce2=0.0f;
			weapon->damage1=33;
			weapon->damage2=0;
			weapon->bullettype=BULLET_GALIL;
			weapon->speed=5;
			break;
		case WEAPON_FAMAS:
			weapon->ClipCap=CLIPCAP_FAMAS;
			weapon->penetrate=WALL_PEN1;
			weapon->distance=8192.0f;
			weapon->wallpierce1=0.96f;
			weapon->wallpierce2=0.0f;
			weapon->damage1=33;
			weapon->damage2=0;
			weapon->bullettype=BULLET_FAMAS;
			weapon->speed=5;
			break;
		case WEAPON_USP:
			weapon->ClipCap=CLIPCAP_USP;
			weapon->penetrate=WALL_PEN0;
			weapon->distance=4096.0f;
			weapon->wallpierce1=0.79f; // unsilenced
			weapon->wallpierce2=0.79f; // silenced
			weapon->damage1=34; // unsilenced
			weapon->damage2=30; // silenced
			weapon->bullettype=BULLET_USP;
      weapon->speed=5;
			break;
		case WEAPON_GLOCK18:
			weapon->ClipCap=CLIPCAP_GLOCK18;
			weapon->penetrate=WALL_PEN0;
			weapon->distance=8192.0f;
			weapon->wallpierce1=0.75f;
			weapon->wallpierce2=0.0f;
			weapon->damage1=20;
			weapon->damage2=0;
			weapon->bullettype=BULLET_GLOCK18;
			weapon->speed=5;
			break;
		case WEAPON_AWP:
			weapon->ClipCap=CLIPCAP_AWP;
			weapon->penetrate=WALL_PEN2;
			weapon->distance=8192.0f;
			weapon->wallpierce1=0.99f;
			weapon->wallpierce2=0.0f;
			weapon->damage1=115;
			weapon->damage2=0;
			weapon->bullettype=BULLET_AWP;
			weapon->speed=12;
			break;
		case WEAPON_MP5:
			weapon->ClipCap=CLIPCAP_MP5;
			weapon->penetrate=WALL_PEN0;
			weapon->distance=8192.0f;
			weapon->wallpierce1=0.84f;
			weapon->wallpierce2=0.0f;
			weapon->damage1=26;
			weapon->damage2=0;
			weapon->bullettype=BULLET_MP5;
			weapon->speed=5;
			break;
		case WEAPON_M249:
			weapon->ClipCap=CLIPCAP_M249;
			weapon->penetrate=WALL_PEN1;
			weapon->distance=8192.0f;
			weapon->wallpierce1=0.97f;
			weapon->wallpierce2=0.0f;
			weapon->damage1=32;
			weapon->damage2=0;
			weapon->bullettype=BULLET_M249;
			weapon->speed=5;
			break;
		case WEAPON_M3:
			weapon->ClipCap=CLIPCAP_M3;
			weapon->penetrate=WALL_PEN0;
			weapon->distance=0.0f;
			weapon->wallpierce1=0.0f;
			weapon->wallpierce2=0.0f;
			weapon->damage1=0;
			weapon->damage2=0;
			weapon->bullettype=BULLET_M3;
			weapon->speed=8;
			break;
		case WEAPON_M4A1:
			weapon->ClipCap=CLIPCAP_M4A1;
			weapon->penetrate=WALL_PEN1;
			weapon->distance=8192.0f;
			weapon->wallpierce1=0.97f; // unsilenced
			weapon->wallpierce2=0.95f; // silenced
			weapon->damage1=32; // unsilenced
			weapon->damage2=33; // silenced
			weapon->bullettype=BULLET_M4A1;
			weapon->speed=5;
			break;
		case WEAPON_TMP:
			weapon->ClipCap=CLIPCAP_TMP;
			weapon->penetrate=WALL_PEN0;
			weapon->distance=8192.0f;
			weapon->wallpierce1=0.85f;
			weapon->wallpierce2=0.0f;
			weapon->damage1=20;
			weapon->damage2=0;
			weapon->bullettype=BULLET_TMP;
			weapon->speed=5;
			break;
		case WEAPON_G3SG1:
			weapon->ClipCap=CLIPCAP_G3SG1;
			weapon->penetrate=WALL_PEN1;
			weapon->distance=8192.0f;
			weapon->wallpierce1=0.98f;
			weapon->wallpierce2=0.0f;
			weapon->damage1=60;
			weapon->damage2=0;
			weapon->bullettype=BULLET_G3SG1;
			weapon->speed=8;
			break;
		case WEAPON_FLASHBANG:
			weapon->ClipCap=0;
			weapon->penetrate=WALL_PEN0;
			weapon->distance=0.0f;
			weapon->wallpierce1=0.0f;
			weapon->wallpierce2=0.0f;
			weapon->damage1=0;
			weapon->damage2=0;
			weapon->bullettype=BULLET_NONE;
			weapon->speed=0;
			break;
		case WEAPON_DEAGLE:
			weapon->ClipCap=CLIPCAP_DEAGLE;
			weapon->penetrate=WALL_PEN1;
			weapon->distance=4096.0f;
			weapon->wallpierce1=0.81f;
			weapon->wallpierce2=0.0f;
			weapon->damage1=54;
			weapon->damage2=0;
			weapon->bullettype=BULLET_DEAGLE;
			weapon->speed=8;
			break;
		case WEAPON_SG552:
			weapon->ClipCap=CLIPCAP_SG552;
			weapon->penetrate=WALL_PEN1;
			weapon->distance=8192.0f;
			weapon->wallpierce1=0.955f;
			weapon->wallpierce2=0.0f;
			weapon->damage1=33;
			weapon->damage2=0;
			weapon->bullettype=BULLET_SG552;
			weapon->speed=5;
			break;
		case WEAPON_AK47:
			weapon->ClipCap=CLIPCAP_AK47;
			weapon->penetrate=WALL_PEN1;
			weapon->distance=8192.0f;
			weapon->wallpierce1=0.98f;
			weapon->wallpierce2=0.0f;
			weapon->damage1=36;
			weapon->damage2=0;
			weapon->bullettype=BULLET_AK47;
			weapon->speed=5;
			break;
		case WEAPON_KNIFE:
			weapon->ClipCap=0;
			weapon->penetrate=WALL_PEN0;
			weapon->distance=0.0f;
			weapon->wallpierce1=0.0f;
			weapon->wallpierce2=0.0f;
			weapon->damage1=0;
			weapon->damage2=0;
			weapon->bullettype=BULLET_NONE;
			weapon->speed=32;
			break;
		case WEAPON_P90:
			weapon->ClipCap=CLIPCAP_P90;
			weapon->penetrate=WALL_PEN0;
			weapon->distance=8192.0f;
			weapon->wallpierce1=0.885f;
			weapon->wallpierce2=0.0f;
			weapon->damage1=21;
			weapon->damage2=0;
			weapon->bullettype=BULLET_P90;
			weapon->speed=5;
		break;
		default:
			weapon->ClipCap=0;
			weapon->penetrate=WALL_PEN0;
			weapon->distance=0.0f;
			weapon->wallpierce1=0.0f;
			weapon->wallpierce2=0.0f;
			weapon->damage1=0;
			weapon->damage2=0;
			weapon->bullettype=BULLET_NONE;
			weapon->speed=3;
    break;
	}
}

void cWeapon::Set(int CurWeapon,int WeaponId,int Ammo){
	int index=GetIndexById(WeaponId);
	if(index!=-1){
		vWeapons[index].CAmmo=Ammo;
    vWeapons[index].CurWeapon=CurWeapon;
	}
}

void cWeapon::SetAmmoX(int Id,int Count){
	for(UINT i=0;i<vWeapons.size();i++){
		if(vWeapons[i].AmmoType==Id)
			vWeapons[i].XAmmo=Count;
	}
}

void cWeapon::SetCurrent(int index,bool CurWeapon){
  if(index<MAX_WEAPONS)
    vWeapons[index].CurWeapon=CurWeapon;
}

sWeapon *cWeapon::Get(){
	for(UINT i=0;i<vWeapons.size();i++){
		if(vWeapons[i].Active&&vWeapons[i].CurWeapon)
			return &vWeapons[i];
	}
	return NULL;
}

sWeapon * cWeapon::GetPrimary(){
	for(UINT i=0;i<vWeapons.size();i++){
		if(vWeapons[i].Active&&vWeapons[i].Slot==0)
			return &vWeapons[i];
	}
	return NULL;
}

sWeapon * cWeapon::GetSecondary(){
	for(UINT i=0;i<vWeapons.size();i++){
		if(vWeapons[i].Active&&vWeapons[i].Slot==1)
			return &vWeapons[i];
	}
	return NULL;
}

int cWeapon::GetIndexById(int WeaponId){
	for(UINT i=0;i<vWeapons.size();i++){
		if(vWeapons[i].Id==WeaponId)
			return i;
	}
  return -1;
}

int cWeapon::GetIdByIndex(int index){
  if(index<MAX_WEAPONS)
    return vWeapons[index].Id;
  return -1;
}

int cWeapon::GetId(){
	tmpWeapon=Get();
	if(tmpWeapon)
		return tmpWeapon->Id;
	return -1;
}

int cWeapon::GetPrimaryId(){
	tmpWeapon=GetPrimary();
	if(tmpWeapon)
		return tmpWeapon->Id;
	return -1;
}

int cWeapon::GetSecondaryId(){
	tmpWeapon=GetSecondary();
	if(tmpWeapon)
		return tmpWeapon->Id;
	return -1;
}

const char * cWeapon::GetCmd(){
	tmpWeapon=Get();
	if(tmpWeapon)
		return tmpWeapon->command;
	return NULL;
}

const char * cWeapon::GetPrimaryCmd(){
	tmpWeapon=GetPrimary();
	if(tmpWeapon)
		return tmpWeapon->command;
	return NULL;
}

const char * cWeapon::GetSecondaryCmd(){
	tmpWeapon=GetSecondary();
	if(tmpWeapon)
		return tmpWeapon->command;
	return NULL;
}

const char *cWeapon::GetName(){
	tmpWeapon=Get();
	if(tmpWeapon)
		return tmpWeapon->name;
	return NULL;
}

const char * cWeapon::GetPrimaryName(){
	tmpWeapon=GetPrimary();
	if(tmpWeapon)
		return tmpWeapon->name;
	return NULL;
}

const char * cWeapon::GetSecondaryName(){
	tmpWeapon=GetSecondary();
	if(tmpWeapon)
		return tmpWeapon->name;
	return NULL;
}

const char *cWeapon::GetNameById(int Id){
	int index=GetIndexById(Id);
	if(index!=-1)
		return vWeapons[index].name;
	return NULL;
}

const char * cWeapon::GetNameByModel(int weaponmodel){
	unsigned int len,i;
	model_s* mdl=gEngStudio.GetModelByIndex(weaponmodel);
	if(!mdl||!mdl->name)
		return "unknown";
	char *weaponname=mdl->name;
	len=strlen(weaponname);
	if(len<14)
		return "unknown";
	weaponname=weaponname+9;
	len-=13;
	for(i=0;i<vWeapons.size();i++){
		if(vWeapons[i].len==len&&!memcmp(vWeapons[i].name,weaponname,len))
			return vWeapons[i].name;
	}
	return "unknown";
}

int cWeapon::GetClipCap(){
  tmpWeapon=Get();
	if(tmpWeapon)
		return tmpWeapon->ClipCap;
	return 0;
}

int cWeapon::GetAmmo(){
	tmpWeapon=Get();
	if(tmpWeapon)
		return tmpWeapon->CAmmo;
	return 0;
}

int cWeapon::GetAmmoX(){
	tmpWeapon=Get();
	if(tmpWeapon)
		return tmpWeapon->XAmmo;
	return 0;
}

int cWeapon::GetPrimaryAmmoX(){
	tmpWeapon=GetPrimary();
	if(tmpWeapon)
		return tmpWeapon->XAmmo;
	return -1;
}

int cWeapon::GetSecondaryAmmoX(){
	tmpWeapon=GetSecondary();
	if(tmpWeapon)
		return tmpWeapon->XAmmo;
	return -1;
}

int cWeapon::GetPrimaryAmmo(){
	tmpWeapon=GetPrimary();
	if(tmpWeapon)
		return tmpWeapon->CAmmo;
	return -1;
}

int cWeapon::GetSecondaryAmmo(){
	tmpWeapon=GetSecondary();
	if(tmpWeapon)
		return tmpWeapon->CAmmo;
	return -1;
}

int cWeapon::GetBulletType(){
	tmpWeapon=Get();
	if(tmpWeapon)
		return tmpWeapon->bullettype;
	return BULLET_NONE;
}

float cWeapon::GetWallPierce(){
	tmpWeapon=Get();
	if(tmpWeapon){
		if(IsSilenced())
			return tmpWeapon->wallpierce2;
		else
			return tmpWeapon->wallpierce1;
		return tmpWeapon->wallpierce1;
	}
	return 0.0f;
}

int cWeapon::GetDamage(){
	tmpWeapon=Get();
	if(tmpWeapon){
		if(IsSilenced())
			return tmpWeapon->damage2;
		else
			return tmpWeapon->damage1;
		return tmpWeapon->damage1;
	}
	return 0;
}

float cWeapon::GetDistance(){
	tmpWeapon=Get();
	if(tmpWeapon)
		return tmpWeapon->distance;
	return 0.0f;
}

int cWeapon::GetPenetration(){
	tmpWeapon=Get();
	if(tmpWeapon)
		return tmpWeapon->penetrate;
	return WALL_PEN0;
}

float cWeapon::GetSpeed(){
  tmpWeapon=Get();
	if(tmpWeapon)
		return tmpWeapon->speed;
	return 0;
}

bool cWeapon::HasPrimary(){
	tmpWeapon=GetPrimary();
	if(tmpWeapon)
		return true;
	return false;
}

bool cWeapon::HasSecondary(){
	tmpWeapon=GetSecondary();
	if(tmpWeapon)
		return true;
	return false;
}

bool cWeapon::HasAmmo(){
	tmpWeapon=Get();
	if(tmpWeapon&&tmpWeapon->CAmmo>0)
		return true;
	return false;
}

bool cWeapon::PrimaryHasAmmo(){
	tmpWeapon=GetPrimary();
	if(tmpWeapon&&tmpWeapon->CAmmo>0)
		return true;
	return false;
}

bool cWeapon::SecondaryHasAmmo(){
	tmpWeapon=GetSecondary();
	if(tmpWeapon&&tmpWeapon->CAmmo>0)
		return true;
	return false;
}

bool cWeapon::HasName(const char *weaponname){
	UINT len=strlen(weaponname);
	for(UINT i=0;i<vWeapons.size();i++){
		if(vWeapons[i].Active&&len==vWeapons[i].len&&!memcmp(vWeapons[i].name,weaponname,len))
			return true;
	}
	return false;
}

bool cWeapon::IsCurrent(int index){
  if(index<MAX_WEAPONS)
    return vWeapons[index].CurWeapon;
  return 0;
}

bool cWeapon::IsInBurst(){
	tmpWeapon=Get();
	return(tmpWeapon&&tmpWeapon->Id==WEAPON_GLOCK18&&tmpWeapon->weapondata.m_iWeaponState&GLOCK18_BURST);
}

bool cWeapon::IsReloading(){
	tmpWeapon=Get();
	return(tmpWeapon&&tmpWeapon->weapondata.m_fInReload);
}

bool cWeapon::IsSilenced(){
	tmpWeapon=Get();
  if(tmpWeapon){
		if(tmpWeapon->Id==WEAPON_M4A1&&tmpWeapon->weapondata.m_iWeaponState&M4A1_SILENCER)
			return true;
		if(tmpWeapon->Id==WEAPON_USP&&tmpWeapon->weapondata.m_iWeaponState&USP_SILENCER)
			return true;
	}
	return false;
}

bool cWeapon::IsSnipper(){
	tmpWeapon=Get();
  if(tmpWeapon)
		return (tmpWeapon->Id==WEAPON_SCOUT||tmpWeapon->Id==WEAPON_AWP);
	return false;
}

bool cWeapon::IsPrimary(){
	tmpWeapon=GetPrimary();
	return(tmpWeapon&&tmpWeapon->CurWeapon);
}

bool cWeapon::IsSecondary(){
	tmpWeapon=GetSecondary();
	return(tmpWeapon&&tmpWeapon->CurWeapon);
}

bool cWeapon::IsGun(){
	tmpWeapon=Get();
	return(tmpWeapon&&(tmpWeapon->Slot==0||tmpWeapon->Slot==1));
}

bool cWeapon::IsKnife(){
	tmpWeapon=Get();
	return(tmpWeapon&&tmpWeapon->Id==WEAPON_KNIFE);
}

bool cWeapon::IsNonAttack(){
	tmpWeapon=Get();
	return(tmpWeapon&&(tmpWeapon->Id==WEAPON_HEGRENADE||tmpWeapon->Id==WEAPON_FLASHBANG||tmpWeapon->Id==WEAPON_C4||tmpWeapon->Id==WEAPON_SMOKEGRENADE));
}

bool cWeapon::IsNade(){
	tmpWeapon=Get();
	return(tmpWeapon&&(tmpWeapon->Id==WEAPON_HEGRENADE||tmpWeapon->Id==WEAPON_FLASHBANG||tmpWeapon->Id==WEAPON_SMOKEGRENADE));
}

bool cWeapon::IsC4(){
	tmpWeapon=Get();
	return(tmpWeapon&&tmpWeapon->Id==WEAPON_C4);
}

bool cWeapon::IsPistol(){
	tmpWeapon=Get();
	if(tmpWeapon&&(tmpWeapon->Id==WEAPON_P228||tmpWeapon->Id==WEAPON_ELITE
	||tmpWeapon->Id==WEAPON_FIVESEVEN||tmpWeapon->Id==WEAPON_USP
	||tmpWeapon->Id==WEAPON_GLOCK18||tmpWeapon->Id==WEAPON_DEAGLE))
		return true;
	return false;
}

bool cWeapon::CanAttack(){
	tmpWeapon=Get();
  return(tmpWeapon&&tmpWeapon->weapondata.m_flNextPrimaryAttack<=0);
}

int cWeapon::GetSequenceInfo(int i){
  static int SequenceInfo[]={
    0,0,0,0,0,0,0,0,0,0,// 0..9
    0,1,2,0,1,2,0,1,2,0,// 10..19
    1,2,0,1,1,2,0,1,1,2,// 20..29
    0,1,2,0,1,2,0,1,2,0,// 30..39
    1,2,0,1,2,0,1,2,0,1,// 40..49
    2,0,1,2,0,0,0,8,0,8,// 50..59
    0,16,0,16,0,0,1,1,2,0,// 60..69
    1,1,2,0,1,0,1,0,1,2,// 70..79
    0,1,2,32,40,32,40,32,32,32,// 80..89
    33,64,33,34,64,65,34,32,32,4,// 90..99
    4,4,4,4,4,4,4,4,4,4,// 100..109
    4                                      	// 110
  };
  return SequenceInfo[i];
}

float cWeapon::GetWeaponViewOffset(char* model){
  if(!strcmp("glock18.mdl",model))
    return 4.55f;
  if(!strcmp("usp.mdl",model))
    return 4.64f;
  if(!strcmp("p228.mdl",model))
    return 4.65f;
  if(!strcmp("deagle.mdl",model))
    return 4.71f;
  if(!strcmp("fiveseven.mdl",model))
    return 4.84f;
  if(!strcmp("m3.mdl",model))
    return 5.03f;
  if(!strcmp("xm1014.mdl",model))
    return 5.82f;
  if(!strcmp("mac10.mdl",model))
    return 5.05f;
  if(!strcmp("tmp.mdl",model))
    return 6.47f;
  if(!strcmp("mp5.mdl",model))
    return 5.53f;
  if(!strcmp("ump45.mdl",model))
    return 6.07f;
  if(!strcmp("p90.mdl",model))
    return 4.32f;
  if(!strcmp("scout.mdl",model))
    return 5.14f;
  if(!strcmp("awp.mdl",model))
    return 6.02f;
  if(!strcmp("famas.mdl",model))
    return 4.84f;
  if(!strcmp("aug.mdl",model))
    return 6.22f;
  if(!strcmp("m4a1.mdl",model))
    return 6.74f;
  if(!strcmp("sg550.mdl",model))
    return 7.11f;
  if(!strcmp("ak47.mdl",model))
    return 6.79f;
  if(!strcmp("g3sg1.mdl",model))
    return 6.19f;
  if(!strcmp("sg552.mdl",model))
    return 5.27f;
  if(!strcmp("galil.mdl",model))
    return 4.78f;
  if(!strcmp("m249.mdl",model))
    return 5.13f;
  return 0.0f;
}

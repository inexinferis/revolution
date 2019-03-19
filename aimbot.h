#ifndef AIMBOT_H
#define AIMBOT_H

#define SPIN_REVS_PER_SECOND 6.0f
#define MAX_DISTANCE 9999

#define MAX_MAP_SIZE 16384
#define YAW 1
#define PITCH 0

typedef float TransformMatrix[MAXSTUDIOBONES][3][4];

class cPlayer;
class cAimbot{
  public:
  	cAimbot(){iTarget=-1;}
    void DrawAimSpot();
    void CalculateAimingView();
    bool HasTarget(){return (iTarget!=-1);}
    bool SmoothAim(float *me);
    bool ShouldShoot(struct usercmd_s *cmd);
    bool CheckTeam(cPlayer &player);
    bool IsShielded(cPlayer &player);
    bool AutoKnife(struct usercmd_s *cmd);
    void CreateMove(struct usercmd_s *cmd);
    float CalcFovAngle(const float* origin_viewer, const float* angle_viewer, const float* origin_target, float* dynfov);
    float CalcPixelFov(const float* origin_target);
    void CalcFovAndVisibility(cPlayer &player);
    bool FindVisibleSpot(cPlayer &player);
    bool PathFree(float* xfrom,float* xto);
    int	IsVisible(float* xfrom,float* xto,int index);
    void FixupAngleDifference(usercmd_t *usercmd);
    void ApplySilentAngles(float *angles,usercmd_s *cmd);
    void PredictTarget(cPlayer *player,vec3_t &pred);
    int  CanPenetrate(float *start, float *end);
    bool IsAimingAtMe(cPlayer *player);
    int CanPenetrateX(float* xfrom,float* xto);
    float GetAimFov();
    float GetDynamicFov();
    float GetPixelFov();
    int FindTarget();

    void CalculateVector(cPlayer *player);
    void CalculateHitbox(cPlayer *player);
    void CalculateAimOrigin(cPlayer *player,vec3_t &vAimPoint);
    float tDistance;
    float hadj_h;
    float hadj_f;
    float hadj_r;
    int iTarget;
    vec3_t aim_viewangles;
    usercmd_t usercmd;
};

extern cAimbot gAimbot;

#endif //AIMBOT_H

#ifndef AUTORUTE_H
#define AUTORUTE_H

#include "players.h"
#include "aimbot.h"
#include "drawing.h"
#include "cvar.h"
#include "main.h"

#define POINT_MAX 999//máximo de puntos para cada línea
#define GROUP_MAX 9//máximo de grupos

struct LINE{float point[3];};
extern LINE lines[GROUP_MAX][POINT_MAX];

typedef struct _w_route{
  struct{
    union{
      struct{
        float x,y,z;
      };
      float vec[3];
    }point[POINT_MAX];;
    int npoints;
	}group[GROUP_MAX];
	int ngroups;
}w_routes;

class cAutoRute{
  public:
    cAutoRute();
    void FindLine(usercmd_t*Cmd);
    void DrawLines(bool bPoints);
    void MoveTo(float*point,usercmd_t* cmd);
    void Load(const char *mapname);
    void Save();
    void AddWayPoint(int index);
    void ClearWayPoint(int index);
    void ClearAllWayPoints();
    void RecordWayPoints();
    //Waypoint maker
    float isRecord;
    //routes
    w_routes routes;
    bool runninginroute;
  protected:
    int direction;
    int beforeGroup;
    int beforePoint;
    int firstGroup;
    int firstPoint;
    char map_name[256];
    char file_name[256];
    //Waypoint maker
    int recordIndex;
    bool CalcRecordIndex;
};

extern cAutoRute gAutoRoute;

#endif //AUTORUTE_H

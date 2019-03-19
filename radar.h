#ifndef RADAR_H
#define RADAR_H

typedef struct overviewInfo_s{
	float	zoom;		// zoom of map images
	int		layers;		// how may layers do we have
	float	origin[3];  //
	float	layersHeights[1];
	char	layersImages[1][255];
	int		rotated;	// are map images rotated (90 degrees) ?
}overviewInfo_t;

class cRadar{
  public:
    cRadar(){
      ov_zoom=5.5f;
      mapLoaded=false;
      m_MapSprite=NULL;
      xTiles=1;
      yTiles=1;
    };
    void Init(int sWidth,int sHeight,int sFontWidth,int sFontHeight);
    void DrawRadar();
    void DrawPoint(float* origin,float* angles,int r,int g,int b,int a,int boxsize=3);
    void LoadOverview(char* levelname);
  private:
    void DrawOverview();
    bool ParseOverview(char* overview_txt);
    int radarx,radary,size;
    int xTiles,yTiles;
    int wWidth,wHeight;
    float ov_zoom;
    bool mapLoaded;
    model_s * m_MapSprite;
    static overviewInfo_t m_OverviewData;
};

extern cRadar gRadar;

#endif //RADAR_H

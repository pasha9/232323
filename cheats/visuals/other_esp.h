#pragma once

#include "..\..\includes.hpp"
#include "..\..\sdk\structs.hpp"

class otheresp : public singleton< otheresp > {
public:
	void antiaim_indicator( );
	void penetration_reticle( );
	void lby_indicator( );
	void hitmarker_paint( );
	void hitmarkerdynamic_paint( );
	void hitmarker_event( IGameEvent * event );
	void spread_crosshair( IDirect3DDevice9* );
	void spread_crosshair( );
	void bomb_timer( );
private:
	float hurt_time;
};

struct impact_info {
	float x, y, z;
	long long time;
};

struct hitmarker_info {
	impact_info impact;
	int alpha;
};

extern std::vector<impact_info> impacts;
extern std::vector<hitmarker_info> hitmarkers;
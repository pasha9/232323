#pragma once

#include "..\..\includes.hpp"
#include "..\..\sdk\structs.hpp"

class CCSGOPlayerAnimState;

class antiaim : public singleton< antiaim > {
public:
	bool nade_out(CUserCmd * pUserCmd);
	void create_move( );
	void update_lowerbody_breaker( );
	float at_target( );

	float freestanding( );

	float get_pitch( );
	float get_fake( );
	float get_real( );

	bool freeze_check = false;

	void get_unpredicted_velocity( );
	player_t * m_e;
	vec3_t unpredicted_velocity;

	bool m_bBreakLowerBody = false;
	float m_flLastLby = 0.0f;
	float m_flNextBodyUpdate = 0.f;
	bool m_bBreakBalance = false;
	float m_flLastMoveTime;
};
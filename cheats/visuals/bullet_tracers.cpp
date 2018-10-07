#include "bullet_tracers.h"

#include "..\..\sdk\misc\BeamInfo_t.hpp"

void bullettracers::draw_beam( vec3_t src, vec3_t end, Color color ) {
	BeamInfo_t info;
	info.m_nType = TE_BEAMPOINTS;
	info.m_pszModelName = "materials/sprites/laserbeam.vmt";
	info.m_nModelIndex = g_csgo.m_modelinfo()->GetModelIndex("materials/sprites/laserbeam.vmt");
	info.m_flHaloScale = 0.0f;
	info.m_flLife = 1.337f;
	info.m_flWidth = 1.337f;
	info.m_flEndWidth = 0.666f;
	info.m_flFadeLength = 3.0f;
	info.m_flAmplitude = 0.0f;
	info.m_flBrightness = (color.a( ) - 255.f ) * 0.8f;
	info.m_flSpeed = 1.f;
	info.m_nStartFrame = 1;
	info.m_flFrameRate = 60;
	info.m_flRed = color.r( );
	info.m_flGreen = color.g( );
	info.m_flBlue = color.b( );
	info.m_nSegments = 4;
	info.m_bRenderable = true;
	info.m_nFlags = FBEAM_ONLYNOISEONCE | FBEAM_NOTILE | FBEAM_HALOBEAM;
	info.m_vecStart = src;
	info.m_vecEnd = end;

	Beam_t* beam = g_csgo.m_viewrenderbeams( )->CreateBeamPoints( info );

	if ( beam )
		g_csgo.m_viewrenderbeams( )->DrawBeam( beam );
}

void bullettracers::events( IGameEvent * event ) {
	if ( !strcmp( event->GetName( ), "bullet_impact" ) ) {
		auto index = g_csgo.m_engine( )->GetPlayerForUserID( event->GetInt( "userid" ) );

		vec3_t position( event->GetFloat( "x" ), event->GetFloat( "y" ), event->GetFloat( "z" ) );

		if ( index == g_csgo.m_engine( )->GetLocalPlayer( ) ) {
			logs.push_back( trace_info( g_ctx.m_local->get_eye_pos( ), position, g_csgo.m_globals( )->m_curtime ) );

			Color color = g_cfg.esp.bullet_tracer_color;
			draw_beam( g_ctx.m_local->get_eye_pos( ), position, color );
		}
	}
}










































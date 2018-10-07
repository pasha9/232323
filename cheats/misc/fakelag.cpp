#include "fakelag.h"
#include "..\autowall\autowall.h"

void fakelag::create_move( ) {
	auto weapon = g_ctx.m_local->m_hActiveWeapon( ).Get( );
	if ( !weapon )
		return;

	bool should_fakelag = false;

	int 
		tick_times = ( ( int )( 1.0f / g_csgo.m_globals( )->m_interval_per_tick ) ) / 64,
		choke;

	switch ( g_cfg.antiaim.fakelag_mode ) {
	case 0:
		choke = min( g_cfg.antiaim.fakelag_amount * tick_times, 14 );
		break;
	case 1:
		choke = math::clamp( static_cast<int>( std::ceilf( 69 / ( g_ctx.m_local->m_vecVelocity( ).Length( ) * g_csgo.m_globals( )->m_interval_per_tick ) ) ), 1, 14 );
		break;
	}

	if ( g_cfg.antiaim.fakelag_enablers[ FAKELAG_AIR ].enabled ) {
		if ( !( g_ctx.m_local->m_fFlags( ) & FL_ONGROUND ) )
			should_fakelag = true;
	}

	if ( g_ctx.m_local->m_vecVelocity( ).Length( ) > 0 && g_ctx.m_local->m_fFlags( ) & FL_ONGROUND ) {
		if ( g_cfg.antiaim.fakelag_enablers[ FAKELAG_MOVING ].enabled )
				should_fakelag = true;

		if ( g_cfg.antiaim.fakelag_onpeek ) {
		}
	}

	if ( g_cfg.antiaim.fakelag_enablers[ FAKELAG_SHOOTING ].enabled ) {
		if ( g_ctx.get_command( )->m_buttons & IN_ATTACK && weapon->can_fire( ) )
			should_fakelag = true;
	}

	if ( g_cfg.antiaim.fakelag_enablers[ FAKELAG_STANDING ].enabled ) {
		if ( g_ctx.m_local->m_fFlags( ) & FL_ONGROUND && !g_ctx.m_local->m_vecVelocity( ).Length( ) )
			should_fakelag = true;
	}

	if ( !g_cfg.antiaim.fakelag_enablers[ FAKELAG_SHOOTING ].enabled ) {
		if ( g_ctx.get_command( )->m_buttons & IN_ATTACK && weapon->can_fire( ) )
			should_fakelag = false;
	}

	if ( g_ctx.m_globals.fakewalking )
		return;

	if ( should_fakelag ) {
		static int choked = 0;

		if ( choked > choke ) {
			g_ctx.send_packet = true;
			choked = 0;
		} else {
			g_ctx.send_packet = false;
			choked++;
		}

		g_ctx.m_globals.fakelagging = true;
	}
}










































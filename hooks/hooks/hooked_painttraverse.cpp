#include "..\hooks.hpp"
#include "..\..\cheats\visuals\player_esp.h"
#include "..\..\cheats\visuals\other_esp.h"
#include "..\..\cheats\misc\logs.h"
#include "..\..\cheats\legit\legit_backtrack.h"
#include "..\..\cheats\visuals\world_esp.h"

using PaintTraverse_t = void( __thiscall* )( void*, vgui::VPANEL, bool, bool );

bool reload_fonts( )
{
	static int old_width, old_height;
	int width, height;
	g_csgo.m_engine( )->GetScreenSize( width, height );

	if ( width != old_width || height != old_height )
	{
		old_width = width;
		old_height = height;
		return true;
	}
	return false;
}

void __fastcall hooks::hooked_painttraverse( void* ecx, void* edx, vgui::VPANEL panel, bool force_repaint, bool allow_force ) {
	auto local_player = static_cast< player_t * >( g_csgo.m_entitylist( )->GetClientEntity( g_csgo.m_engine( )->GetLocalPlayer( ) ) );
	static auto draw_animation = [ ] ( ) -> void {
		if ( !g_ctx.m_local->is_alive( ) )
			return;

		auto screen = render::get( ).viewport( );

		for ( int i = 0; i < 13; i++ ) {
			AnimationLayer layer = g_ctx.m_local->get_animlayers( ) [ i ];

			render::get( ).text( fonts [ VERDANA12BOLDOUT ], screen.right / 2 + 10, screen.bottom / 2 + 10 + ( i * 13 ), Color::White, HFONT_CENTERED_NONE,
				"layer: %i, activity: %i, weight: %0.2f, cycle: %0.2f", i, g_ctx.m_local->sequence_activity( layer.m_nSequence ), layer.m_flWeight, layer.m_flCycle );
		}
	};

	static uint32_t HudZoomPanel;
	if ( !HudZoomPanel )
		if ( !strcmp( "HudZoom", g_csgo.m_panel( )->GetName( panel ) ) )
			HudZoomPanel = panel;

	if ( g_cfg.esp.removals [ REMOVALS_SCOPE ].enabled && HudZoomPanel == panel )
		return;

	static auto panel_id = vgui::VPANEL { 0 };
	static auto original_fn = panel_hook->get_func_address< PaintTraverse_t >( 41 );

	original_fn( ecx, panel, force_repaint, allow_force );

	if ( !panel_id ) {
		const auto panelName = g_csgo.m_panel( )->GetName( panel );
		if ( strstr( panelName, "MatSystemTopPanel" ) )
			panel_id = panel;
	}

	if ( g_cfg.player.lowerbody_indicator ) {
		//i'll finish this later ~alpha
	}

	if ( reload_fonts( ) )
		render::get( ).setup( ); //fix for fonts not reloading whilst changing resolutions.

	if ( panel_id == panel ) {
		if ( g_ctx.available( ) && local_player ) {
			for ( int i = 0; i < 4; i++ ) {
				for ( int j = 0; j < 4; j++ ) {
					g_ctx.m_globals.w2s_matrix [ i ] [ j ] = g_csgo.m_engine( )->WorldToScreenMatrix( ) [ i ] [ j ];
				}
			}

			if ( g_cfg.player.enable )
				playeresp::get( ).paint_traverse( );

			if ( g_cfg.antiaim.lby_breaker )
				otheresp::get( ).lby_indicator( );

			otheresp::get( ).spread_crosshair( );

			if ( g_cfg.legit.backtracking_visualize )
				legitbacktrack::get( ).paint_traverse( );

			worldesp::get( ).paint_traverse( );

			if ( g_cfg.esp.removals [ REMOVALS_SCOPE ].enabled && local_player->m_bIsScoped( ) ) {
				auto resolution = render::get( ).viewport( );

				render::get( ).line( 0, resolution.bottom / 2, resolution.right, resolution.bottom / 2, Color::Black );
				render::get( ).line( resolution.right / 2, 0, resolution.right / 2, resolution.bottom, Color::Black );
			}

			if ( g_cfg.esp.hitmarker [ HITMARKER_STATIC ].enabled )
				otheresp::get( ).hitmarker_paint( );

			if ( g_cfg.esp.hitmarker [ HITMARKER_DYNAMIC ].enabled )
				otheresp::get( ).hitmarkerdynamic_paint( );

			otheresp::get( ).antiaim_indicator( );

			otheresp::get( ).penetration_reticle( );

			playeresp::get( ).pov_arrow( );
		}

		cfg_manager->config_files( );

		eventlogs::get( ).paint_traverse( );

		menu::get( ).update( );
	}
}








































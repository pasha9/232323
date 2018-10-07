#include "..\hooks.hpp"
#include "..\..\cheats\lagcompensation\lagcompensation.h"

using DrawModelExecute_t = void( __thiscall* )( IVModelRender*, IMatRenderContext*, const DrawModelState_t&, const ModelRenderInfo_t&, matrix3x4_t* );

void __stdcall hooks::hooked_dme( IMatRenderContext * ctx, const DrawModelState_t & state, const ModelRenderInfo_t & info, matrix3x4_t * bone_to_world ) {
	static auto original_fn = modelrender_hook->get_func_address< DrawModelExecute_t >( 21 );

	auto btchams = [ & ] ( player_t * e ) -> bool {
		auto & player = lagcompensation::get( ).players[ e->EntIndex( ) ];
		auto track = player.get_valid_track( );

		if ( !player.m_e || track.empty( ) )
			return false;

		for ( int i = track.size( ) - 1; i >= 0 ; i-- ) {
			auto & record = track.at( i );

			original_fn( g_csgo.m_modelrender( ), ctx, state, info, record.m_bone_matrix );

			return true;
		}

		return false;
	};

	bool isplayer = strstr( info.pModel->name, "models/player" ) != nullptr;
	auto model_entity = static_cast< player_t * >( g_csgo.m_entitylist( )->GetClientEntity( info.entity_index ) );

	if ( g_ctx.m_local->m_bIsScoped( ) && isplayer && model_entity != nullptr && model_entity->m_iTeamNum( ) == g_ctx.m_local->m_iTeamNum( ) && model_entity == g_ctx.m_local ) {
		if ( g_cfg.esp.transparency_scoped && g_ctx.m_local->is_alive( ) )
			g_csgo.m_renderview( )->SetBlend( g_cfg.esp.transparency_scoped_alpha * 0.01f );
	}

	if ( isplayer ) {
		if ( !model_entity->valid( true ) && model_entity != g_ctx.m_local && g_cfg.esp.dont_render_teammates )
			return;

		if ( g_ctx.backtrack_chams && model_entity->valid( true ) ) {
			if ( btchams( model_entity ) )
				return;
		}
	}

	original_fn( g_csgo.m_modelrender( ), ctx, state, info, bone_to_world );
}
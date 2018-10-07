#include "lagcompensation.h"

void lagcompensation::fsn( ClientFrameStage_t stage ) {
	for ( int i = 1; i <= g_csgo.m_globals( )->m_maxclients; i++ ) {
		auto e = static_cast< player_t * >( g_csgo.m_entitylist( )->GetClientEntity( i ) );
		auto & player = players[ i ];

		player.m_e = e;

		if ( !continue_loop( i, e, stage ) )
			continue;

		switch ( stage ) {
		case FRAME_NET_UPDATE_POSTDATAUPDATE_START:
			e->set_abs_origin( e->m_vecOrigin( ) );

			player.m_resolver->m_e = e;
			player.m_resolver->run( );

			break;
		case FRAME_NET_UPDATE_POSTDATAUPDATE_END:
			e->update_clientside_animation( );

			player.log( );

			break;
		case FRAME_NET_UPDATE_END:
			apply_interpolation_flags( e, disable_interpolation );

			break;
		case FRAME_RENDER_START:
			*( int* )( ( uintptr_t )e + 0xA30 ) = g_csgo.m_globals( )->m_framecount;
			*( int* )( ( uintptr_t )e + 0xA28 ) = 0;

			if (g_cfg.antiaim.animfix)
				animation_fix( e );

			break;
		}
	}
}

void lagcompensation::create_move( ) {
	last_eye_positions.push_front( g_ctx.m_local->get_eye_pos( ) );
	if ( last_eye_positions.size( ) > 128 )
		last_eye_positions.pop_back( );

	auto nci = g_csgo.m_engine( )->GetNetChannelInfo( );
	if ( !nci )
		return;

	const int latency_ticks = TIME_TO_TICKS( nci->GetLatency( FLOW_OUTGOING ) );
	const auto latency_based_eye_pos = last_eye_positions.size( ) <= latency_ticks ? last_eye_positions.back( ) : last_eye_positions[ latency_ticks ];

	for ( int i = 1; i <= g_csgo.m_globals( )->m_maxclients; i++ ) {
		auto e = static_cast< player_t * >( g_csgo.m_entitylist( )->GetClientEntity( i ) );
		auto & player = players[ i ];

		player.m_e = e;

		if ( !e ) { player.m_e = nullptr; continue; }

		if ( !e->valid( true ) )
			continue;

		player.m_resolver->m_e = e;
		player.m_resolver->create_move( latency_based_eye_pos );
	}
}

void player_record_t::log( ) {
	if ( !g_cfg.ragebot.lagcomp ) {
		m_track.clear( );

		return;
	}

	player_t * e = m_e;

	if ( !m_track.size( ) ) {
		tickrecord_t record;
		record.save( e, *m_resolver );

		m_track.push_front( record );

		return;
	}

	if ( m_track.front( ).m_simulation_time != e->m_flSimulationTime( ) ) {
		tickrecord_t record;
		record.save( e, *m_resolver );

		record.m_previous_origin = m_track.front( ).m_vec_origin;
		record.m_previous_simulation_time = m_track.front( ).m_simulation_time;
		record.m_previous_curtime = m_track.front( ).m_curtime;

		m_track.push_front( record );
	}

	if ( g_csgo.m_globals( )->m_curtime - m_track.back( ).m_curtime > 1.f )
		m_track.pop_back( );
}

std::deque< tickrecord_t > player_record_t::get_valid_track( ) {
	auto delta_time = [ & ] ( float simulation_time ) -> float {
		auto nci = g_csgo.m_engine( )->GetNetChannelInfo( );

		if ( !nci )
			return FLT_MAX;

		float correct = 0.f;
			correct += nci->GetLatency( FLOW_OUTGOING );
			correct += nci->GetLatency( FLOW_INCOMING );
			correct += util::lerp_time( );
			correct = math::clamp< float >( correct, 0.f, 1.f );

			return fabs( correct - ( g_csgo.m_globals( )->m_curtime - simulation_time ) );
	};

	std::deque< tickrecord_t > track;

	for ( const auto & record : m_track ) {
		if ( delta_time( record.m_simulation_time ) < 0.2f )
			track.push_back( record );
	}

	return track;
}

bool lagcompensation::continue_loop( int id, player_t * e, ClientFrameStage_t stage ) {
	auto & player = players[ id ];
	if ( !e ) { player.m_e = nullptr; return false; }

	switch ( stage ) {
	case FRAME_RENDER_START:
		if ( !e->valid( true ) )
			return false;

		break;
	case FRAME_NET_UPDATE_POSTDATAUPDATE_START:
		if ( !e->valid( true ) ) {
			delete player.m_resolver;
			player.m_resolver = new c_player_resolver( );

			return false;
		}

		break;
	case FRAME_NET_UPDATE_POSTDATAUPDATE_END:
		if ( e->m_iHealth( ) <= 0 || e->m_iTeamNum( ) == g_ctx.m_local->m_iTeamNum( ) ) {
			player.m_track.clear( );

			return false;
		}

		if ( e->IsDormant( ) || e->m_bGunGameImmunity( ) )
			return false;

		break;
	case FRAME_NET_UPDATE_END:
		if ( !e->valid( true ) )
			return false;

		break;
	}

	return true;
}

void lagcompensation::apply_interpolation_flags( player_t * e, int flag ) {
	const auto var_map = reinterpret_cast< uintptr_t >( e ) + 36;

	for ( auto index = 0; index < *reinterpret_cast<int*>( var_map + 20 ); index++ )
		*reinterpret_cast<uintptr_t*>( *reinterpret_cast<uintptr_t*>( var_map ) + index * 12 ) = flag;
}

void lagcompensation::animation_fix( player_t * e ) {
	c_baseplayeranimationstate * animation_state = e->get_animation_state( );

	if ( !animation_state )
		return;

	auto player = players[ e->EntIndex( ) ];

	if ( player.m_track.size( ) ) {
		tickrecord_t * record = &player.m_track.front( );

		if ( e->m_flSimulationTime( ) != record->m_anim_time ) {
			e->m_bClientSideAnimation( ) = true;

			e->update_clientside_animation( );

			record->m_anim_time = e->m_flSimulationTime( );
			record->m_anim_state = e->get_animation_state( );
		} else {
			e->m_bClientSideAnimation( ) = false;

			e->set_animation_state( record->m_anim_state );
		}

		e->set_abs_angles( vec3_t( 0, animation_state->m_flGoalFeetYaw, 0 ) );
	}
}

void lagcompensation::update_sequence( ) {
	if ( !g_csgo.m_clientstate( ) )
		return;

	auto net_channel = g_csgo.m_clientstate( )->m_NetChannel;

	if ( net_channel ) {
		if ( net_channel->m_nInSequenceNr > m_last_incoming_sequence_number ) {
			m_last_incoming_sequence_number = net_channel->m_nInSequenceNr;
			sequences.push_front( incoming_sequence_t( net_channel->m_nInReliableState, net_channel->m_nOutReliableState, net_channel->m_nInSequenceNr, g_csgo.m_globals( )->m_realtime ) );
		}

		if ( sequences.size( ) > 2048 )
			sequences.pop_back( );
	}
}

void lagcompensation::clear_sequence( ) {
	m_last_incoming_sequence_number = 0;

	sequences.clear( );
}

void lagcompensation::add_latency( INetChannel * net_channel ) {
	for ( auto& seq : sequences ) {
		if ( g_csgo.m_globals( )->m_realtime - seq.m_current_time >= g_cfg.misc.ping_spike_value / 1000.f ) {
			net_channel->m_nInReliableState = seq.m_in_reliable_state;
			net_channel->m_nInSequenceNr = seq.m_sequence_nr;

			break;
		}
	}
}
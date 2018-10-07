#include "logs.h"

void eventlogs::paint_traverse( ) {
	if ( logs.empty( ) )
		return;

	auto last_y = 6;
	const auto font = fonts[ rolexTEXT ];
	const auto name_font = fonts[ rolexBOLD ];
	auto name_size = render::get( ).text_width( name_font, "[rolex]" );

	for ( unsigned int i = 0; i < logs.size( ); i++ ) {
		auto & log = logs.at( i );

		if ( util::epoch_time( ) - log.log_time > 2700 ) {
			float factor = ( log.log_time + 3100 ) - util::epoch_time( );
			factor /= 1000;

			auto opacity = int( 255 * factor );

			if ( opacity < 2 ) {
				logs.erase( logs.begin( ) + i );
				continue;
			}

			log.color.SetAlpha( opacity );

			log.x += 1 * ( factor * 1.25 );
			log.y += 1 * ( factor * 1.25 );
		}

		const auto text = log.message.c_str( );

		auto front_color = g_cfg.menu.menu_theme[ 0 ];
		front_color.SetAlpha( log.color.a( ) );

		render::get( ).text( name_font, log.x, last_y + log.y, front_color, HFONT_CENTERED_NONE, "[rolex]" );
		render::get( ).text( font, log.x + 3 + name_size, last_y + log.y, log.color, HFONT_CENTERED_NONE, text );

		last_y += 14;
	}
}


void eventlogs::events( IGameEvent * event ) {
	static auto get_hitgroup_name = [ ] ( int hitgroup ) -> std::string {
		switch ( hitgroup ) {
		case HITGROUP_HEAD:
			return "head";
		case HITGROUP_LEFTLEG:
			return "left leg";
		case HITGROUP_RIGHTLEG:
			return "right leg";
		case HITGROUP_STOMACH:
			return "stomach";
		case HITGROUP_LEFTARM:
			return "left arm";
		case HITGROUP_RIGHTARM:
			return "right arm";
		default:
			return "body";
		}
	};

	constexpr char * hasdefusekit[ 2 ] = { "without defuse kit.","with defuse kit." };
	constexpr char * hasbomb[ 2 ] = { "without the bomb.","with the bomb." };
	constexpr char * hitgroups[ 7 ] = { "head" };

	if ( g_cfg.misc.events_to_log[ EVENTLOG_HIT ].enabled && strstr( event->GetName( ), "player_hurt" ) ) {
		auto
			userid = event->GetInt( "userid" ),
			attacker = event->GetInt( "attacker" );

		if ( !userid || !attacker ) {
			return;
		}

		auto
			userid_id = g_csgo.m_engine( )->GetPlayerForUserID( userid ),
			attacker_id = g_csgo.m_engine( )->GetPlayerForUserID( attacker );

		player_info_t userid_info, attacker_info;
		if ( !g_csgo.m_engine( )->GetPlayerInfo( userid_id, &userid_info ) ) {
			return;
		}

		if ( !g_csgo.m_engine( )->GetPlayerInfo( attacker_id, &attacker_info ) ) {
			return;
		}

		auto m_victim = static_cast< player_t * >( g_csgo.m_entitylist( )->GetClientEntity( userid_id ) );

		std::stringstream ss;
		if ( attacker_id == g_csgo.m_engine( )->GetLocalPlayer( ) ) {
			ss << "You hurt " << userid_info.szName << " in the " << get_hitgroup_name( event->GetInt( "hitgroup" ) ) << " for " << event->GetInt( "dmg_health" );
			ss << " ( " << math::clamp( m_victim->m_iHealth( ) - event->GetInt( "dmg_health" ), 0, 100 ) << " health remaining )";

			add( ss.str( ), Color::White );
		} else if ( userid_id == g_csgo.m_engine( )->GetLocalPlayer( ) ) {
			ss << "You took " << event->GetInt( "dmg_health" ) << " damage from " << attacker_info.szName << "in the " << get_hitgroup_name( event->GetInt( "hitgroup" ) );
			ss << " ( " << math::clamp( m_victim->m_iHealth( ) - event->GetInt( "dmg_health" ), 0, 100 ) << " health remaining )";

			add( ss.str( ), Color::White );
		}
	}

	if ( g_cfg.misc.events_to_log[ EVENTLOG_ITEM_PURCHASES ].enabled && strstr( event->GetName( ), "item_purchase" ) ) {
		auto userid = event->GetInt( "userid" );

		if ( !userid ) {
			return;
		}

		auto userid_id = g_csgo.m_engine( )->GetPlayerForUserID( userid );

		player_info_t userid_info;
		if ( !g_csgo.m_engine( )->GetPlayerInfo( userid_id, &userid_info ) ) {
			return;
		}

		auto m_player = static_cast< player_t * >( g_csgo.m_entitylist( )->GetClientEntity( userid_id ) );

		if ( !g_ctx.m_local || !m_player ) {
			return;
		}

		if ( m_player->m_iTeamNum( ) == g_ctx.m_local->m_iTeamNum( ) ) {
			return;
		}

		std::stringstream ss;
		ss << userid_info.szName << " purchased a(n) " << event->GetString( "weapon" );

		add( ss.str( ), Color::White );
	}

	if ( g_cfg.misc.events_to_log[ EVENTLOG_PLANTING ].enabled && strstr( event->GetName( ), "bomb_beginplant" ) ) {
		auto userid = event->GetInt( "userid" );

		if ( !userid ) {
			return;
		}

		auto userid_id = g_csgo.m_engine( )->GetPlayerForUserID( userid );

		player_info_t userid_info;
		if ( !g_csgo.m_engine( )->GetPlayerInfo( userid_id, &userid_info ) ) {
			return;
		}

		auto m_player = static_cast< player_t * >( g_csgo.m_entitylist( )->GetClientEntity( userid_id ) );

		if ( !m_player ) {
			return;
		}

		std::stringstream ss;
		ss << userid_info.szName << " has began planting the bomb at site " << event->GetInt( "site" ) << ".";

		add( ss.str( ), Color::White );
	}

	if ( g_cfg.misc.events_to_log[ EVENTLOG_DEFUSING ].enabled && strstr( event->GetName( ), "bomb_begindefuse" ) ) {
		auto userid = event->GetInt( "userid" );

		if ( !userid ) {
			return;
		}

		auto userid_id = g_csgo.m_engine( )->GetPlayerForUserID( userid );

		player_info_t userid_info;
		if ( !g_csgo.m_engine( )->GetPlayerInfo( userid_id, &userid_info ) ) {
			return;
		}

		auto m_player = static_cast< player_t * >( g_csgo.m_entitylist( )->GetClientEntity( userid_id ) );

		if ( !m_player ) {
			return;
		}

		std::stringstream ss;
		ss << userid_info.szName << " has began defusing the bomb " << hasdefusekit[ event->GetBool( "haskit" ) ];

		add( ss.str( ), Color::White );
	}
}

void eventlogs::add( std::string text, Color color ) {
	logs.push_front( loginfo_t( util::epoch_time( ), text, color ) );
}










































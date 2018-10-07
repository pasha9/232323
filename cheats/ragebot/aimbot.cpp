#include "aimbot.h"
#include "..\autowall\autowall.h"
#include "..\sdk\animation_state.h"

#define clamp(val, min, max) (((val) > (max)) ? (max) : (((val) < (min)) ? (min) : (val)))

void aimbot::create_move( ) {
	g_csgo.m_engine( )->GetViewAngles( this->engine_angles );
	this->local_weapon = g_ctx.m_local->m_hActiveWeapon( ).Get( );

	auto weapon_recoil_scale = g_csgo.m_cvar( )->FindVar( "weapon_recoil_scale" );
	this->recoil_scale = weapon_recoil_scale->GetFloat( );

	if ( !local_weapon )
		return;

	update_config( );

	if ( ( g_ctx.m_local->m_flNextAttack( ) > util::server_time( ) ) )
		return;

	if ( local_weapon->is_non_aim( ) || local_weapon->m_iClip1( ) < 1 )
		return;

	if ( local_weapon->m_iItemDefinitionIndex( ) == WEAPON_REVOLVER ) {
		if ( config.autoshoot )
			g_ctx.get_command( )->m_buttons |= IN_ATTACK;

		float flPostponeFireReady = local_weapon->m_flPostponeFireReadyTime( );
		if ( flPostponeFireReady && flPostponeFireReady < g_csgo.m_globals( )->m_curtime + g_csgo.m_globals( )->m_interval_per_tick * g_csgo.m_clientstate( )->m_nChokedCommands && g_cfg.ragebot.autoshoot ) {
			g_ctx.get_command( )->m_buttons &= ~IN_ATTACK;

			return;
		}
	}

	if ( !local_weapon->can_fire( ) )
		return;

	iterate_players( );
	aim( );
}

void aimbot::iterate_players( ) {
	target_data.reset( );

	static weapon_t * old_weapon;
	if ( local_weapon != old_weapon ) {
		old_weapon = local_weapon;

		g_ctx.get_command( )->m_buttons &= ~IN_ATTACK;

		return;
	}

	for ( int i = 1; i <= g_csgo.m_globals( )->m_maxclients; i++ ) {
		auto e = static_cast< player_t * >( g_csgo.m_entitylist( )->GetClientEntity( i ) );

		if ( !e->valid( true ) )
			continue;

		matrix3x4_t bones[ 128 ];
		if ( !e->SetupBones( bones, 128, 0x100, 0.f ) )
			continue;

		float simulation_time = e->m_flSimulationTime( );

		vec3_t location = best_point( e, bones );
		bool backtrack = false;
		tickrecord_t lag_record;

		if ( location == vec3_t( 0, 0, 0 ) && g_cfg.ragebot.lagcomp ) {
			auto & player = lagcompensation::get( ).players[ i ];

			if ( player.m_e && !player.get_valid_track( ).empty( ) ) {
				bt_point_return_t return_info = best_point_backtrack( e, player );

				location = return_info.point;
				lag_record = return_info.record;
				simulation_time = lag_record.m_simulation_time;

				backtrack = true;
			}
		}

		if ( location == vec3_t( 0, 0, 0 ) )
			continue;

		float fov = math::get_fov( engine_angles, math::calculate_angle( g_ctx.m_local->get_eye_pos( ), location ) );
		if ( fov > g_cfg.ragebot.field_of_view )
			continue;

		get_target( e, fov, location, simulation_time, backtrack, lag_record );
	}
}

void aimbot::zeus_run( )
{
	Vector aim_pos;
	auto e = static_cast< player_t * >( g_csgo.m_entitylist( )->GetClientEntity( target_data.tid ) );
	if ( !g_ctx.m_local->m_hActiveWeapon( )->m_iItemDefinitionIndex( ) == ( short ) ItemDefinitionIndex::WEAPON_TASER )
		return;;

	Vector point = e->hitbox_position( 2 );
	if ( util::visible( g_ctx.m_local->get_eye_pos( ), point, e, g_ctx.m_local ) ) {
		if ( ( g_ctx.m_local->m_vecOrigin( ) - e->m_vecOrigin( ) ).Length( ) <= 180 ) {
			aim_pos = math::calculate_angle( g_ctx.m_local->get_eye_pos( ), point );
			aim_pos.Clamp( );
			g_ctx.get_command( )->m_buttons |= IN_ATTACK;
		}
	}
	if ( g_ctx.get_command( )->m_buttons & IN_ATTACK )
		g_ctx.get_command( )->m_viewangles = aim_pos;
}

bool aimbot::can_zeus( Vector &bestpoint )
{
	auto e = static_cast< player_t * >( g_csgo.m_entitylist( )->GetClientEntity( target_data.tid ) );
	Vector forward;
	math::angle_vectors( bestpoint, forward );
	forward *= 183;

	Vector eye_pos = g_ctx.m_local->get_eye_pos( );

	Ray_t ray;
	trace_t trace;
	ray.Init( eye_pos, eye_pos + forward );

	CTraceFilter filter;
	filter.pSkip = g_ctx.m_local;

	g_csgo.m_trace( )->TraceRay( ray, MASK_SHOT, &filter, &trace );

	return trace.hit_entity == e;
}

void aimbot::aim( ) {
	if ( target_data.tid == -1 )
		return;

	auto e = static_cast< player_t * >( g_csgo.m_entitylist( )->GetClientEntity( target_data.tid ) );

	if ( e->m_bGunGameImmunity( ) )
		return;

	vec3_t next_angle = math::calculate_angle( g_ctx.m_local->get_eye_pos( ), target_data.aimspot );

	bool hitchanced = false;

	if ( config.hitchance )
		hitchanced = hitchance( next_angle, e, config.hitchance_amount );
	else
		hitchanced = true;

	if ( hitchanced ) {
		if ( g_cfg.ragebot.zeus_bot ) {
			if ( can_zeus( next_angle ) )
				zeus_run( );
			else
				g_ctx.get_command( )->m_viewangles = next_angle;
		}
		else {
			g_ctx.get_command( )->m_viewangles = next_angle;
		}

		g_ctx.get_command( )->m_viewangles = next_angle;

		if ( g_cfg.ragebot.anti_recoil )
			g_ctx.get_command( )->m_viewangles -= g_ctx.m_local->m_aimPunchAngle( ) * recoil_scale;

		if ( !g_cfg.ragebot.silent_aim )
			g_csgo.m_engine( )->SetViewAngles( next_angle );

		if ( config.autoshoot ) {
			g_ctx.get_command( )->m_buttons |= IN_ATTACK;

			if ( g_cfg.misc.anti_untrusted )
				g_ctx.send_packet = false;
		}

		if ( g_ctx.get_command( )->m_buttons & IN_ATTACK )
			g_ctx.get_command( )->m_tickcount = TIME_TO_TICKS( target_data.simtime + util::lerp_time( ) );

		g_ctx.m_globals.aimbotting = true;
	}

	if ( hitchanced && config.quickstop && g_ctx.m_local->m_fFlags( ) & FL_ONGROUND )
		quickstop( );

}

vec3_t aimbot::best_point( player_t * e, matrix3x4_t bones[ 128 ] ) {
	vec3_t output = vec3_t( 0, 0, 0 );

	std::vector< int >
		hitboxes = hitboxes_from_vector( e, e->m_fFlags( ), e->m_vecVelocity( ), config.hitscan ),
		multipoints = hitboxes_from_vector( e, e->m_fFlags( ), e->m_vecVelocity( ), config.multipoint );

	float best_damage = 0.f;

	for ( auto current : hitboxes ) {
		vec3_t location = e->hitbox_position( current, bones );

		if ( config.autowall ) {
			float damage = autowall::get( ).calculate_return_info( g_ctx.m_local->get_eye_pos( ), location, g_ctx.m_local, e ).m_damage;

			if ( ( damage > best_damage ) && ( damage > config.minimum_damage ) ) {
				best_damage = damage;
				output = location;
			}
		}
		else if ( !config.autowall && g_ctx.m_local->point_visible( location ) )
			return output;
	}

	for ( auto current : multipoints ) {
		std::vector< vec3_t > points;  getpoints( e, current, bones, points );

		for ( auto point : points ) {
			if ( config.autowall ) {
				float damage = autowall::get( ).calculate_return_info( g_ctx.m_local->get_eye_pos( ), point, g_ctx.m_local, e ).m_damage;

				if ( ( damage > best_damage ) && ( damage > config.minimum_damage ) ) {
					best_damage = damage;
					output = point;
				}
			}
			else if ( !config.autowall && g_ctx.m_local->point_visible( point ) )
				return point;
		}
	}

	return output;
}

aimbot::bt_point_return_t aimbot::best_point_backtrack( player_t * e, player_record_t & player ) {
	auto hitgroup_from_hitbox = [ ] ( int hitbox ) -> int {
		switch ( hitbox ) {
		case HITBOX_HEAD:
		case HITBOX_NECK:
			return HITGROUP_HEAD;
			break;
		case HITBOX_PELVIS:
		case HITBOX_STOMACH:
			return HITGROUP_STOMACH;
			break;
		case HITBOX_LOWER_CHEST:
		case HITBOX_CHEST:
		case HITBOX_UPPER_CHEST:
			return HITGROUP_CHEST;
			break;
		case HITBOX_RIGHT_THIGH:
		case HITBOX_RIGHT_CALF:
		case HITBOX_RIGHT_FOOT:
			return HITGROUP_RIGHTLEG;
			break;
		case HITBOX_LEFT_THIGH:
		case HITBOX_LEFT_CALF:
		case HITBOX_LEFT_FOOT:
			return HITGROUP_LEFTLEG;
			break;
		case HITBOX_RIGHT_HAND:
		case HITBOX_RIGHT_UPPER_ARM:
		case HITBOX_RIGHT_FOREARM:
			return HITGROUP_RIGHTARM;
			break;
		case HITBOX_LEFT_UPPER_ARM:
		case HITBOX_LEFT_FOREARM:
		case HITBOX_LEFT_HAND:
			return HITGROUP_LEFTARM;
			break;
		}
	};

	std::deque< tickrecord_t > track = player.get_valid_track( );

	tickrecord_t best_record;
	float best_damage = 1;
	vec3_t best_position;
	
	for ( int i = 0; i < track.size( ); i++ ) {
		tickrecord_t record = track[ i ];

		std::vector< int > hitboxes = hitboxes_from_vector( e, record.m_flags, record.m_velocity, config.hitscan_history );

		for ( auto hitbox : hitboxes ) {
			vec3_t location = e->hitbox_position( hitbox, record.m_bone_matrix );

			if ( config.autowall ) {
				float damage = autowall::get( ).calculate_return_info( g_ctx.m_local->get_eye_pos( ), location, g_ctx.m_local, e, hitgroup_from_hitbox( hitbox ) ).m_damage;

				if ( damage > best_damage ) {
					best_position = location;
					best_damage = damage;
					best_record = record;
				}
			} else if ( !config.autowall && g_ctx.m_local->point_visible( location ) ) {
				float damage = 50; autowall::get( ).scale_damage( e, local_weapon->get_csweapon_info( ), hitgroup_from_hitbox( hitbox ), damage );

				if ( damage > best_damage ) {
					best_position = location;
					best_damage = damage;
					best_record = record;
				}
			}
		}
	}

	return bt_point_return_t{ best_record, best_position };
}

void aimbot::getpoints( player_t * e, int hitbox_id, matrix3x4_t bones[ 128 ], std::vector< vec3_t > & points ) {
	studiohdr_t * studio_model = g_csgo.m_modelinfo( )->GetStudiomodel( e->GetModel( ) );
	mstudiohitboxset_t * set = studio_model->pHitboxSet( e->m_nHitboxSet( ) );

	mstudiobbox_t * hitbox = set->pHitbox( hitbox_id );
	if ( !hitbox )
		return;

	vec3_t min, max;
	math::vector_transform( hitbox->bbmin, bones[ hitbox->bone ], min );
	math::vector_transform( hitbox->bbmax, bones[ hitbox->bone ], max );

	vec3_t
		center = ( min + max ) * 0.5f,
		angle = math::calculate_angle( center, g_ctx.m_local->get_eye_pos( ) );

	vec3_t forward;
	math::angle_vectors( angle, forward );

	vec3_t
		right = forward.Cross( vec3_t( 0, 0, 1 ) ),
		left = vec3_t( -right.x, -right.y, right.z ),
		top = vec3_t( 0, 0, 1 ),
		bot = vec3_t( 0, 0, -1 );

	if ( hitbox_id == HITBOX_HEAD ) {
		points.push_back( center + ( top * ( hitbox->m_flRadius * config.pointscale ) ) );
		points.push_back( center + ( right * ( hitbox->m_flRadius * config.pointscale ) ) );
		points.push_back( center + ( left * ( hitbox->m_flRadius * config.pointscale ) ) );
	} else {
		points.push_back( center + ( right * ( hitbox->m_flRadius * config.pointscale ) ) );
		points.push_back( center + ( left * ( hitbox->m_flRadius * config.pointscale ) ) );
	}
}

void aimbot::quickstop( ) {
	g_ctx.get_command( )->m_sidemove = 0;
	g_ctx.get_command( )->m_forwardmove = g_ctx.m_local->m_vecVelocity( ).Length2D( ) > 20.f ? 450.f : 0.f;

	math::rotate_movement( math::calculate_angle( vec3_t( 0, 0, 0 ), g_ctx.m_local->m_vecVelocity( ) ).y + 180.f );
}

bool aimbot::hitchance( vec3_t angles, player_t * e, float chance ) {
	angles -= g_ctx.m_local->m_aimPunchAngle( ) * recoil_scale;

	vec3_t
		forward,
		right,
		up,
		src = g_ctx.m_local->get_eye_pos( );

	math::angle_vectors( angles, &forward, &right, &up );

	int
		hits = 0,
		needed_hits = static_cast<int>( 256.f * ( chance / 100.f ) );

	float
		weap_spread = local_weapon->get_spread( ),
		weap_inaccuracy = local_weapon->get_innacuracy( );

	for ( int i = 0; i < 256; i++ ) {
		float
			a = math::random_float( 0.f, 1.f ),
			b = math::random_float( 0.f, 2.f * DirectX::XM_PI ),
			c = math::random_float( 0.f, 1.f ),
			d = math::random_float( 0.f, 2.f * DirectX::XM_PI ),
			inaccuracy = a * weap_inaccuracy,
			spread = c * weap_spread;

		if ( local_weapon->m_iItemDefinitionIndex( ) == WEAPON_REVOLVER ) {
			if ( g_ctx.get_command( )->m_buttons & IN_ATTACK2 ) {
				a = 1.f - a * a;
				c = 1.f - c * c;
			}
		}

		vec3_t
			spread_view( ( cos( b ) * inaccuracy ) + ( cos( d ) * spread ), ( sin( b ) * inaccuracy ) + ( sin( d ) * spread ), 0 ),
			direction;

		direction.x = forward.x + ( spread_view.x * right.x ) + ( spread_view.y * up.x );
		direction.y = forward.y + ( spread_view.x * right.y ) + ( spread_view.y * up.y );
		direction.z = forward.z + ( spread_view.x * right.z ) + ( spread_view.y * up.z );
		direction.Normalized( );

		vec3_t
			viewangles_spread,
			view_forward;

		math::vector_angles( direction, up, viewangles_spread );
		math::normalize_angles( viewangles_spread );

		math::angle_vectors( viewangles_spread, view_forward );
		view_forward.NormalizeInPlace( );

		view_forward = src + ( view_forward * local_weapon->get_csweapon_info( )->range );

		trace_t tr;
		Ray_t ray;

		ray.Init( src, view_forward );
		g_csgo.m_trace( )->ClipRayToEntity( ray, MASK_SHOT | CONTENTS_GRATE, e, &tr );

		if ( tr.hit_entity == e )
			hits++;

		if ( static_cast<int>( ( static_cast<float>( hits ) / 256.f ) * 100.f ) >= chance )
			return true;

		if ( ( 256 - i + hits ) < needed_hits )
			return false;
	}

	return false;
}

void aimbot::get_target( player_t * e, float fov, vec3_t location, float simulation_time, bool backtrack, tickrecord_t lag_record ) {
	switch ( g_cfg.ragebot.selection_type ) {
	case 0:
		if ( target_data.best_distance < fov )
			return;

		target_data.best_distance = fov;
		target_data.tid = e->EntIndex( );
		target_data.aimspot = location;
		target_data.simtime = simulation_time;
		target_data.backtrack = backtrack;
		target_data.best_record = lag_record;

		break;
	case 1:
		if ( target_data.best_distance < e->m_iHealth( ) )
			return;

		target_data.best_distance = e->m_iHealth( );
		target_data.tid = e->EntIndex( );
		target_data.aimspot = location;
		target_data.simtime = simulation_time;
		target_data.backtrack = backtrack;
		target_data.best_record = lag_record;

		break;
	case 2:
		float distance = sqrt(
			pow( double( location.x - g_ctx.m_local->m_vecOrigin( ).x ), 2 ) +
			pow( double( location.y - g_ctx.m_local->m_vecOrigin( ).y ), 2 ) +
			pow( double( location.z - g_ctx.m_local->m_vecOrigin( ).z ), 2 ) );

		if ( target_data.best_distance < distance )
			return;

		target_data.best_distance = distance;
		target_data.tid = e->EntIndex( );
		target_data.aimspot = location;
		target_data.simtime = simulation_time;
		target_data.backtrack = backtrack;
		target_data.best_record = lag_record;

		break;
	}
}

bool aimbot::head_check( player_t * e, int m_fFlags, vec3_t m_vecVelocity ) {
	bool use = true;

	if ( config.baim[ BAIM_INAIR ].enabled && !( m_fFlags & FL_ONGROUND ) )
		use = false;

	if ( g_cfg.ragebot.antiaim_correction || config.baim[ BAIM_NOTRESOLVED ].enabled ) {
		auto & player = lagcompensation::get( ).players[ e->EntIndex( ) ];
	
		if ( !player.m_e || !player.m_resolver )
			return use;
	
		if ( player.m_resolver->m_mode == brute )
			use = false;
	}

	if ( g_ctx.m_local->m_hActiveWeapon( )->m_iItemDefinitionIndex( ) == ( short )ItemDefinitionIndex::WEAPON_TASER )
		use = false;

	return use;
}

std::vector< int > aimbot::hitboxes_from_vector( player_t * e, int m_fFlags, vec3_t m_vecVelocity, std::vector< MultiDropdownItem_t > arr ) {
	std::vector< int > hitboxes;
	bool head_valid = head_check( e, m_fFlags, m_vecVelocity );

	if ( arr[ 0 ].enabled && head_valid ) {
		hitboxes.push_back( HITBOX_HEAD );

		hitboxes.push_back( HITBOX_NECK );
	}

	if ( arr[ 1 ].enabled ) {
		hitboxes.push_back( HITBOX_UPPER_CHEST );
		hitboxes.push_back( HITBOX_CHEST );
	}

	if ( arr[ 2 ].enabled ) {
		hitboxes.push_back( HITBOX_PELVIS );
		hitboxes.push_back( HITBOX_STOMACH );
	}

	if ( arr[ 3 ].enabled ) {
		hitboxes.push_back( HITBOX_RIGHT_UPPER_ARM );
		hitboxes.push_back( HITBOX_LEFT_UPPER_ARM );

		hitboxes.push_back( HITBOX_RIGHT_FOREARM );
		hitboxes.push_back( HITBOX_LEFT_FOREARM );

		hitboxes.push_back( HITBOX_RIGHT_HAND );
		hitboxes.push_back( HITBOX_LEFT_HAND );
	}

	if ( arr[ 4 ].enabled ) {
		hitboxes.push_back( HITBOX_RIGHT_THIGH );
		hitboxes.push_back( HITBOX_LEFT_THIGH );

		hitboxes.push_back( HITBOX_RIGHT_CALF );
		hitboxes.push_back( HITBOX_LEFT_CALF );

		hitboxes.push_back( HITBOX_RIGHT_FOOT );
		hitboxes.push_back( HITBOX_LEFT_FOOT );
	}

	return hitboxes;
}

void aimbot::update_config( ) {
	auto id = local_weapon->m_iItemDefinitionIndex( );

	int element = -1;

	if ( local_weapon->is_pistol( ) && id != WEAPON_DEAGLE )
		element = 0;

	if ( local_weapon->m_iItemDefinitionIndex( ) == WEAPON_DEAGLE )
		element = 1;

	if ( local_weapon->is_smg( ) )
		element = 3;

	if ( id == WEAPON_AWP || id == WEAPON_SSG08 )
		element = 4;

	if ( id == WEAPON_SCAR20 || id == WEAPON_G3SG1 )
		element = 5;

	if ( local_weapon->is_shotgun( ) )
		element = 6;

	if ( element == -1 )
		element = 2;

	config.autoshoot = g_cfg.ragebot.autoshoot[ element ];
	config.autowall = g_cfg.ragebot.autowall[ element ];
	config.minimum_damage = g_cfg.ragebot.minimum_damage[ element ];
	config.hitscan = g_cfg.ragebot.hitscan[ element ];
	config.hitscan_history = g_cfg.ragebot.hitscan_history[ element ];
	config.multipoint = g_cfg.ragebot.multipoint_hitboxes[ element ]; // fps issue
	config.pointscale = g_cfg.ragebot.pointscale[ element ];
	config.hitchance = g_cfg.ragebot.hitchance[ element ];
	config.hitchance_amount = g_cfg.ragebot.hitchance_amount[ element ];
	config.baim = g_cfg.ragebot.baim_settings[ element ];
	config.quickstop = g_cfg.ragebot.quickstop[ element ];

	config.autoscope = false;

	if ( element == 4 || element == 5 ) {
		config.autoscope = g_cfg.ragebot.autoscope[ element - 3 ];
	} else if ( element == 2 && ( id == WEAPON_AUG || id == WEAPON_SG556 ) ) {
		config.autoscope = g_cfg.ragebot.autoscope[ 0 ];
	}
}










































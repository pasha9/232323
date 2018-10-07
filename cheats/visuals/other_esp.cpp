#include "other_esp.h"
#include "..\autowall\autowall.h"
#include "..\ragebot\antiaim.h"
#include <chrono>

struct ModColor
{
	float color[3];
	float alpha;

	void SetColorFloat(float r, float g, float b)
	{
		color[0] = r;
		color[1] = g;
		color[2] = b;
	}

	void SetColorFloat(float col[3])
	{
		color[0] = col[0];
		color[1] = col[1];
		color[2] = col[2];
	}

	void SetColor(Color col)
	{
		color[0] = col.r() / 255.f;
		color[1] = col.g() / 255.f;
		color[2] = col.b() / 255.f;
	}

	void GetColorFloat(float col[3])
	{
		col[0] = color[0];
		col[1] = color[1];
		col[2] = color[2];
	}

	Color GetColor()
	{
		return Color(color[0], color[1], color[2]);
	}

	ModColor(float r, float g, float b, float alph)
	{
		color[0] = r;
		color[1] = g;
		color[2] = b;
		alpha = alph;
	}

	ModColor(float col[3], float alph)
	{
		color[0] = col[0];
		color[1] = col[1];
		color[2] = col[2];
		alpha = alph;
	}
};

std::vector<impact_info> impacts;
std::vector<hitmarker_info> hitmarkers;
std::unordered_map<MaterialHandle_t, std::pair<ModColor, ModColor>> vguiMaterials;
std::unordered_map<MaterialHandle_t, std::pair<ModColor, ModColor>> worldMaterials;
float r, g, b, a;
bool sprop = false;

void otheresp::antiaim_indicator( ) {
	auto local_player = static_cast< player_t * >( g_csgo.m_entitylist( )->GetClientEntity( g_csgo.m_engine( )->GetLocalPlayer( ) ) );

	static auto degrees_to_radians = [ ] ( float deg ) -> float { return deg * ( DirectX::XM_PI / 180.f ); };

	auto rotated_position = [ ] ( Vector start, const float rotation, const float distance ) -> Vector {
		const auto rad = degrees_to_radians( rotation );
		start.x += cosf( rad ) * distance;
		start.y += sinf( rad ) * distance;

		return start;
	};

	if ( !g_ctx.available( ) || !local_player )
		return;

	static const auto real_color = Color( 0, 255, 0, 255 );
	static const auto fake_color = Color( 255, 0, 0, 255 );
	static const auto lby_color = Color( 0, 0, 255, 255 );

	if ( !local_player->is_alive( ) )
		return;

	if ( g_cfg.esp.antiaim_indicator ) {
		auto client_viewangles = Vector( );
		auto
			screen_width = render::get( ).viewport( ).right,
			screen_height = render::get( ).viewport( ).bottom;

		g_csgo.m_engine( )->GetViewAngles( client_viewangles );

		constexpr auto radius = 80.f;

		const auto screen_center = Vector2D( screen_width / 2.f, screen_height / 2.f );
		const auto real_rot = degrees_to_radians( client_viewangles.y - g_ctx.m_globals.real_angles.y - 90 );
		const auto fake_rot = degrees_to_radians( client_viewangles.y - g_ctx.m_globals.fake_angles.y - 90 );
		const auto lby_rot = degrees_to_radians( client_viewangles.y - g_ctx.m_local->m_flLowerBodyYawTarget( ) - 90 );

		auto draw_arrow = [ & ] ( float rot, Color color ) -> void {
			auto pos_one = Vector2D( screen_center.x + cosf( rot ) * radius, screen_center.y + sinf( rot ) * radius );
			auto pos_two = Vector2D( screen_center.x + cosf( rot + degrees_to_radians( 8 ) ) * ( radius - 13.f ),
				screen_center.y + sinf( rot + degrees_to_radians( 8 ) ) * ( radius - 13.f ) );
			auto pos_three = Vector2D( screen_center.x + cosf( rot - degrees_to_radians( 8 ) ) * ( radius - 13.f ),
				screen_center.y + sinf( rot - degrees_to_radians( 8 ) ) * ( radius - 13.f ) );

			render::get( ).triangle(
				Vector2D( pos_one.x, pos_one.y ),
				Vector2D( pos_two.x, pos_two.y ),
				Vector2D( pos_three.x, pos_three.y ),
				color );
		};

		draw_arrow( real_rot, real_color );
		draw_arrow( fake_rot, fake_color );
		draw_arrow( lby_rot, lby_color );
	}
}

void otheresp::penetration_reticle( ) {
	auto local_player = static_cast< player_t * >( g_csgo.m_entitylist( )->GetClientEntity( g_csgo.m_engine( )->GetLocalPlayer( ) ) );

	if ( !local_player->is_alive( ) )
		return;

	auto draw_xhair = [ ] ( float size, Color color ) -> void {
		int x_1, x_2, y_1, y_2;

		int
			screen_w = render::get( ).viewport( ).right / 2,
			screen_h = render::get( ).viewport( ).bottom / 2;

		x_1 = screen_w - ( size / 4 );
		x_2 = screen_w + ( size / 4 ) + 1;

		y_1 = screen_h - ( size / 4 );
		y_2 = screen_h + ( size / 4 ) + 1;

		render::get( ).rect_filled( screen_w - 1, screen_h - 1, 3, 3, color );
	};

	if ( !g_cfg.esp.penetration_reticle || !local_player->is_alive( ) )
		return;

	auto weapon = local_player->m_hActiveWeapon( ).Get( );
	if ( !weapon )
		return;

	if ( weapon->is_non_aim( ) )
		return;

	auto weapon_info = weapon->get_csweapon_info( );
	if ( !weapon_info )
		return;

	vec3_t view_angles;
	g_csgo.m_engine( )->GetViewAngles( view_angles );

	vec3_t direction;
	math::angle_vectors( view_angles, direction );

	vec3_t
		start = local_player->get_eye_pos( ),
		end = start + ( direction * 8192.f );

	Color color = Color( 255, 0, 0 );
	if ( autowall::get( ).calculate_return_info( start, end, local_player ).m_did_penetrate_wall )
		color = Color( 163, 225, 21 );

	draw_xhair( 2, color );

}

void otheresp::lby_indicator( ) {
	auto local_player = static_cast< player_t * >( g_csgo.m_entitylist( )->GetClientEntity( g_csgo.m_engine( )->GetLocalPlayer( ) ) );

	int screen_width, screen_height;
	g_csgo.m_engine( )->GetScreenSize( screen_width, screen_height );

	float breaking_lby_fraction = fabs( math::normalize_yaw( g_ctx.m_globals.real_angles.y - local_player->m_flLowerBodyYawTarget( ) ) ) / 180.f;
	color colour( ( 1.f - breaking_lby_fraction ) * 255.f, breaking_lby_fraction * 255.f, 0 );
	Color clr( colour.RGBA [ 0 ], colour.RGBA [ 1 ], colour.RGBA [ 2 ] );

	render::get( ).text( fonts [ INDICATORFONT ], 5, screen_height - 500, clr, HFONT_OFF_SCREEN, "LBY" );
}

player_t* get_player( int userid ) {
	int i = g_csgo.m_engine( )->GetPlayerForUserID( userid );
	return static_cast< player_t * >( g_csgo.m_entitylist( )->GetClientEntity( i ) );
}

void otheresp::hitmarker_event( IGameEvent * event ) {

	auto local_player = static_cast< player_t * >( g_csgo.m_entitylist( )->GetClientEntity( g_csgo.m_engine( )->GetLocalPlayer( ) ) );

	if ( !strcmp( event->GetName( ), "player_hurt" ) ) {

		auto attacker = event->GetInt( "attacker" );
		auto victim = get_player( event->GetInt( "userid" ) );

		if ( g_csgo.m_engine( )->GetPlayerForUserID( attacker ) == g_csgo.m_engine( )->GetLocalPlayer( ) )

			hurt_time = g_csgo.m_globals( )->m_curtime;
		impact_info best_impact;
		float best_impact_distance = -1;
		float time = g_csgo.m_globals( )->m_curtime;


		for ( int i = 0; i < impacts.size( ); i++ ) {
			auto iter = impacts [ i ];
			if ( time > iter.time + 1.f ) {
				impacts.erase( impacts.begin( ) + i );
				continue;
			}
			vec3_t position = vec3_t( iter.x, iter.y, iter.z );
			vec3_t enemy_pos = victim->m_vecOrigin( );
			float distance = position.DistTo( enemy_pos );
			if ( distance < best_impact_distance || best_impact_distance == -1 ) {
				best_impact_distance = distance;
				best_impact = iter;
			}
		}

		if ( best_impact_distance == -1 )
			return;


		hitmarker_info info;
		info.impact = best_impact;
		info.alpha = 255;
		hitmarkers.push_back( info );

	}

	if ( strstr( event->GetName( ), "bullet_impact" ) )
	{
		auto entity = get_player( event->GetInt( "userid" ) );
		if ( !entity || entity != local_player )
			return;

		impact_info info;
		info.x = event->GetFloat( "x" );
		info.y = event->GetFloat( "y" );
		info.z = event->GetFloat( "z" );

		info.time = g_csgo.m_globals( )->m_curtime;

		impacts.push_back( info );
	}

}

void otheresp::hitmarker_paint( ) {
	auto linesize = 8;
	int sizeX, centerX;
	int sizeY, centerY;

	if ( !g_ctx.m_local->is_alive( ) )
		hurt_time = 0.f;
	if ( !g_csgo.m_engine( )->IsInGame( ) || !g_csgo.m_engine( )->IsConnected( ) )
		hurt_time = 0.f;

	if ( hurt_time + 0.65f >= g_csgo.m_globals( )->m_curtime ) {
		g_csgo.m_engine( )->GetScreenSize( sizeX, sizeY );
		linesize -= static_cast< int >( static_cast< float >( ( ( hurt_time + 0.65f ) - g_csgo.m_globals( )->m_curtime ) / 1.2 ) * 45 );

		centerX = sizeX / 2;
		centerY = sizeY / 2;

		util::clamp_value( linesize, 0, 8 );

		g_csgo.m_surface( )->DrawSetColor( 255, 255, 255, 255 );
		g_csgo.m_surface( )->DrawLine( centerX - linesize, centerY - linesize, centerX - ( linesize / 4 ), centerY - ( linesize / 4 ) );
		g_csgo.m_surface( )->DrawLine( centerX - linesize, centerY + linesize, centerX - ( linesize / 4 ), centerY + ( linesize / 4 ) );
		g_csgo.m_surface( )->DrawLine( centerX + linesize, centerY + linesize, centerX + ( linesize / 4 ), centerY + ( linesize / 4 ) );
		g_csgo.m_surface( )->DrawLine( centerX + linesize, centerY - linesize, centerX + ( linesize / 4 ), centerY - ( linesize / 4 ) );
	}
}

void otheresp::bomb_timer( ) {
	if ( !g_cfg.esp.bomb_timer )
		return;

	auto c4_timer = g_csgo.m_cvar( )->FindVar( "mp_c4timer" )->GetFloat( );

}

void otheresp::hitmarkerdynamic_paint( ) {

	float time = g_csgo.m_globals( )->m_curtime;

	for ( int i = 0; i < hitmarkers.size( ); i++ ) {
		bool expired = time >= hitmarkers.at( i ).impact.time + 2.f;

		if ( expired )
			hitmarkers.at( i ).alpha -= 1;

		if ( expired && hitmarkers.at( i ).alpha <= 0 ) {
			hitmarkers.erase( hitmarkers.begin( ) + i );
			continue;
		}

		Vector pos3D = Vector( hitmarkers.at( i ).impact.x, hitmarkers.at( i ).impact.y, hitmarkers.at( i ).impact.z ), pos2D;
		if ( !math::world_to_screen( pos3D, pos2D ) )
			continue;

		auto linesize = 8;

		g_csgo.m_surface( )->DrawSetColor( 255, 255, 255, hitmarkers.at( i ).alpha );
		g_csgo.m_surface( )->DrawLine( pos2D.x - linesize, pos2D.y - linesize, pos2D.x - ( linesize / 4 ), pos2D.y - ( linesize / 4 ) );
		g_csgo.m_surface( )->DrawLine( pos2D.x - linesize, pos2D.y + linesize, pos2D.x - ( linesize / 4 ), pos2D.y + ( linesize / 4 ) );
		g_csgo.m_surface( )->DrawLine( pos2D.x + linesize, pos2D.y + linesize, pos2D.x + ( linesize / 4 ), pos2D.y + ( linesize / 4 ) );
		g_csgo.m_surface( )->DrawLine( pos2D.x + linesize, pos2D.y - linesize, pos2D.x + ( linesize / 4 ), pos2D.y - ( linesize / 4 ) );
	}
}

void otheresp::spread_crosshair( IDirect3DDevice9* m_device ) {
	if ( !g_cfg.esp.show_spread )
		return;

	if ( !g_ctx.m_local )
		return;

	if ( !g_ctx.m_local->is_alive( ) )
		return;

	if ( !g_csgo.m_engine( )->IsConnected( ) && !g_csgo.m_engine( )->IsInGame( ) )
		return;

	auto weapon = g_ctx.m_local->m_hActiveWeapon( ).Get( );

	if ( !weapon )
		return;

	static float rot = 0.f;

	int w, h;

	g_csgo.m_engine( )->GetScreenSize( w, h );

	w /= 2, h /= 2;

	int r, g, b;
	r = g_cfg.esp.show_spread_color.r( );
	g = g_cfg.esp.show_spread_color.g( );
	b = g_cfg.esp.show_spread_color.b( );

	switch ( g_cfg.esp.show_spread_type ) {
		case 0:
			render::get( ).circle_dual_colour( w, h, weapon->get_innacuracy( ) * 500.0f, 0, 1, 50, D3DCOLOR_RGBA( r, g, b, 100 ), D3DCOLOR_RGBA( 0, 0, 0, 0 ), m_device );
			break;
		case 1:
			render::get( ).circle_dual_colour( w, h, weapon->get_innacuracy( ) * 500.0f, rot, 1, 50, m_device );
			break;
	}

	rot += 0.5f;
	if ( rot > 360.f )
		rot = 0.f;
}

void otheresp::spread_crosshair( ) {
	if ( g_cfg.esp.show_spread_type != 2 )
		return;

	if ( !g_ctx.m_local )
		return;

	if ( !g_ctx.m_local->is_alive( ) )
		return;

	if ( !g_csgo.m_engine( )->IsConnected( ) && !g_csgo.m_engine( )->IsInGame( ) )
		return;

	auto weapon = g_ctx.m_local->m_hActiveWeapon( ).Get( );

	if ( !weapon )
		return;

	if ( weapon ) {
		int screen_w, screen_h;
		g_csgo.m_engine( )->GetScreenSize( screen_w, screen_h );
		int cross_x = screen_w / 2, cross_y = screen_h / 2;

		float recoil_step = screen_h / 180;

		cross_x -= ( int ) ( g_ctx.m_local->m_aimPunchAngle( ).y * recoil_step );
		cross_y += ( int ) ( g_ctx.m_local->m_aimPunchAngle( ).x * recoil_step );

		weapon->update_accuracy_penality( );
		float inaccuracy = weapon->get_innacuracy( );
		float spread = weapon->get_spread( );

		float cone = inaccuracy * spread;
		cone *= screen_h * 0.7f;
		cone *= 90.f / 180;

		for ( int seed { }; seed < 512; ++seed ) {
			math::random_seed( math::random_float( 0, 512 ) );

			float	rand_a = math::random_float( 0.f, 1.f );
			float	pi_rand_a = math::random_float( 0.f, 2.f * DirectX::XM_PI );
			float	rand_b = math::random_float( 0.0f, 1.f );
			float	pi_rand_b = math::random_float( 0.f, 2.f * DirectX::XM_PI );

			float spread_x = cos( pi_rand_a ) * ( rand_a * inaccuracy ) + cos( pi_rand_b ) * ( rand_b * spread );
			float spread_y = sin( pi_rand_a ) * ( rand_a * inaccuracy ) + sin( pi_rand_b ) * ( rand_b * spread );

			float max_x = cos( pi_rand_a ) * cone + cos( pi_rand_b ) * cone;
			float max_y = sin( pi_rand_a ) * cone + sin( pi_rand_b ) * cone;

			float step = screen_h / 180 * 90.f;
			int screen_spread_x = ( int ) ( spread_x * step * 0.7f );
			int screen_spread_y = ( int ) ( spread_y * step * 0.7f );

			float percentage = ( rand_a * inaccuracy + rand_b * spread ) / ( inaccuracy + spread );

			render::get( ).rect( cross_x + screen_spread_x, cross_y + screen_spread_y, 1, 1,
				g_cfg.esp.show_spread_color );
		}
	}
}
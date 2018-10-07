#include "configs.h"

#include <string>
#include <windows.h>
#include <shlobj.h>
#include <shlwapi.h>
#include <iostream>
#include <fstream>
#include <map>

C_ConfigManager * cfg_manager = new C_ConfigManager( );
Config g_cfg;

void C_ConfigManager::setup( ) {
	setup_item( &g_cfg.ragebot.enable, false, "Ragebot.enable" );
	setup_item( &g_cfg.ragebot.field_of_view, 360, "Ragebot.fov" );
	setup_item( &g_cfg.ragebot.silent_aim, false, "Ragebot.silent_aim" );
	setup_item( &g_cfg.ragebot.selection_type, 0, "Ragebot.selection_type" );
	setup_item( &g_cfg.ragebot.lag_compensation_type, 0, "Ragebot.lag_compensation_type");
	setup_item( &g_cfg.ragebot.anti_recoil, false, "Ragebot.anti_recoil" );
	setup_item( &g_cfg.ragebot.anti_spread, false, "Ragebot.anti_spread" );
	setup_item( &g_cfg.ragebot.zeus_bot, false, "Ragebot.zeus_bot" );
	setup_item( &g_cfg.ragebot.extrapolation , false , "Ragebot.extrapolation" );
	setup_item( &g_cfg.ragebot.aimstep, false, "Ragebot.aimstep" );
	setup_item( &g_cfg.ragebot.antiaim_correction, false, "Ragebot.antiaim_correction" );
	setup_item( &g_cfg.ragebot.override_key, BUTTON_CODE_NONE, "Ragebot.override_key" );
	setup_item( &g_cfg.ragebot.lagcomp, false, "Ragebot.lagcomp" );
	setup_item( &g_cfg.ragebot.lagcomp_time, 0.10f, "Ragebot.lagcomp_time" );

	for ( int i = 0; i < 7; i++ ) {
		std::string start = "Ragebot.";
		char * names[ 7 ] = { "pistols", "deagle", "rifles", "smgs", "snipers", "autos", "shotguns" };

		start += names[ i ];

		setup_item( &g_cfg.ragebot.autoshoot[ i ], false, start + ".autoshoot" );
		setup_item( &g_cfg.ragebot.autowall[ i ], false, start + ".autowall" );
		setup_item( &g_cfg.ragebot.minimum_damage[ i ], 1, start + ".minimum_damage" );

		if ( i == 2 || i == 4 || i == 5 ) {
			int autoscope_element;

			if ( i == 2 ) autoscope_element = 0;
			if ( i == 4 ) autoscope_element = 1;
			if ( i == 5 ) autoscope_element = 2;

			setup_item( &g_cfg.ragebot.autoscope[ autoscope_element ], false, start + ".autoscope" );
		}

		setup_item( &g_cfg.ragebot.hitscan[ i ], {
			{ false, "head" },
			{ false, "chest" },
			{ false, "stomach" },
			{ false, "arms" },
			{ false, "legs" }
		}, start + ".hitscan" );

		setup_item( &g_cfg.ragebot.hitscan_history[ i ], {
			{ false, "head" },
			{ false, "chest" },
			{ false, "stomach" },
			{ false, "arms" },
			{ false, "legs" }
		}, start + ".hitscan_history" );

		setup_item( &g_cfg.ragebot.multipoint_hitboxes[ i ], {
			{ false, "head" },
			{ false, "chest" },
			{ false, "stomach" },
			{ false, "arms" },
			{ false, "legs" }
		}, start + ".multipoint" );

		setup_item( &g_cfg.ragebot.pointscale[ i ], 1.0f, start + ".pointscale" );

		setup_item( &g_cfg.ragebot.hitchance[ i ], false, start + ".hitchance" );
		setup_item( &g_cfg.ragebot.hitchance_amount[ i ], 0, start + ".hitchance_amount" );

		setup_item( &g_cfg.ragebot.baim_settings[ i ], {
			{ false, "in air" },
			{ false, "not resolved" }
		}, start + ".baim" );

		setup_item( &g_cfg.ragebot.quickstop[ i ], false, start + ".quickstop" );
	}

	for ( int i = 0; i < 3; i++ ) {
		std::string start = "Antiaim.";

		char * antiaim_modes[ 3 ] = { "still", "moving", "air" };

		start += antiaim_modes[ i ];

		setup_item( &g_cfg.antiaim.pitch[ i ], false, start + ".pitch" );

		setup_item( &g_cfg.antiaim.yaw[ i ], 0, start + ".yaw" );
		setup_item( &g_cfg.antiaim.yaw_offset[ i ], 0, start + ".yaw_offset" );
		setup_item( &g_cfg.antiaim.base_angle[ i ], 0, start + ".base_angle" );

		setup_item( &g_cfg.antiaim.fake_yaw[ i ], 0, start + ".fakeyaw" );
		setup_item( &g_cfg.antiaim.fake_yaw_offset[ i ], 0, start + ".fakeyaw_offset" );
	}

	setup_item( &g_cfg.antiaim.lby_breaker, false, "antiaim.lby_breaker" );
	setup_item( &g_cfg.antiaim.lby_delta, 0, "antiaim.lby_delta" );
	setup_item( &g_cfg.antiaim.pitch_flick, false, "antiaim.pitch_flick" );
	setup_item( &g_cfg.antiaim.break_adjustment, false, "antiaim.break_adjustment" );

	setup_item( &g_cfg.antiaim.right_manualkey, BUTTON_CODE_NONE, "Antiaim.manualkey_right" );
	setup_item( &g_cfg.antiaim.left_manualkey, BUTTON_CODE_NONE, "Antiaim.manualkey_left" );
	setup_item( &g_cfg.antiaim.back_manualkey, BUTTON_CODE_NONE, "Antiaim.manualkey_back" );

	setup_item( &g_cfg.antiaim.fakelag_mode, 0, "Anitiaim.fakelag_mode" );
	setup_item( &g_cfg.antiaim.fakelag_enablers, {
		{ false, "moving" },
		{ false, "air" },
		{ false, "standing" },
		{ false, "shooting" }
	}, "Antiaim.fakelag_enablers" );
	setup_item( &g_cfg.antiaim.fakelag_amount, 1, "Antiaim.fakelag_amount" );

	setup_item( &g_cfg.antiaim.fakelag_onpeek, false, "Antiaim.fakelag_onpeek" );
	setup_item( &g_cfg.antiaim.fakelag_onpeek_amt, 1, "Antiaim.fakelag_onpeek_amt" );

	setup_item( &g_cfg.legit.aimassist, false, "Legit.aim_assist" );
	setup_item( &g_cfg.legit.aim_assist_bind, BUTTON_CODE_NONE, "Legit.aim_assist_bind" );
	setup_item( &g_cfg.legit.fov, false, "Legit.fov" );
	setup_item( &g_cfg.legit.smoothing, false, "Legit.smoothing" );

	setup_item( &g_cfg.legit.backtracking, false, "Legit.backtracking" );
	setup_item( &g_cfg.legit.backtracking_visualize, false, "Legit.backtracking_visualize" );
	setup_item( &g_cfg.legit.backtracking_visualize_color, Color( 255, 255, 255, 255 ), "Legit.backtracking_visualize_color" );
	setup_item( &g_cfg.legit.backtracking_max_time, 0.20f, "Legit.backtracking_time" );

	setup_item( &g_cfg.misc.bunnyhop, false, "Misc.autohop" );
	setup_item( &g_cfg.misc.airstrafe, false, "Misc.airstrafe" );
	setup_item( &g_cfg.misc.duck_jump, false, "Misc.duckjump" );
	setup_item( &g_cfg.misc.circle_strafe, BUTTON_CODE_NONE, "Misc.circle_strafer" );
	setup_item( &g_cfg.misc.thirdperson_toggle, BUTTON_CODE_NONE, "Misc.thirdperson_toggle" );
	setup_item( &g_cfg.misc.events_to_log, {
		{ false, "player hits" },
		{ false, "items" },
		{ false, "planting" },
		{ false, "defusing" }
	}, "Misc.events_to_log" );
	setup_item( &g_cfg.misc.exaggerated_ragdolls, false, "Misc.exaggerated_ragdolls" );
	setup_item( &g_cfg.misc.clantag_spammer, false, "Misc.clantag_spammer" );
	

	setup_item( &g_cfg.misc.ping_spike, false, "Misc.ping_spike" );
	setup_item( &g_cfg.misc.ping_spike_value, 0, "Misc.ping_spike_value" );

	setup_item( &g_cfg.misc.fakewalk, false, "Misc.fakewalk" );
	setup_item( &g_cfg.misc.fakewalk_key, BUTTON_CODE_NONE, "Misc.fakewalk_key" );

	setup_item( &g_cfg.misc.anti_untrusted, true, "Misc.antiuntrusted" );

	setup_item(&g_cfg.misc.sniper_crosshair, false, "Esp.sniper_crosshair");

	setup_item( &g_cfg.menu.menu_bind, KEY_INSERT, "Menu.bind" );
	setup_item( &g_cfg.menu.menu_theme[ 0 ], Color( 210, 45, 82 ), "Menu.color[ 0 ]" );
	setup_item( &g_cfg.menu.menu_theme[ 1 ], Color( 195, 30, 67 ), "Menu.color[ 1 ]" );

	setup_item( &g_cfg.player.enable, false, "Player.enable" );
	setup_item( &g_cfg.player.dormant_players, false, "Player.dormant_players" );
	setup_item( &g_cfg.player.alpha, 0, "Player.alpha" );

	setup_item( &g_cfg.player.out_pov_arrow, false, "Player.directional_arrow" );
	setup_item( &g_cfg.player.out_pov_distance , 300 , "Player.directional_arrow_distance" );
	setup_item( &g_cfg.player.out_pov_arrow_color, Color( 255, 255, 255, 255 ), "Player.out_pov_arrow_color" );

	setup_item( &g_cfg.player.box, false, "Player.box" );
	setup_item( &g_cfg.player.box_color, Color( 255, 255, 255, 255 ), "Player.box_color" );
	setup_item( &g_cfg.player.health, false, "Player.health" );
	setup_item( &g_cfg.player.name, false, "Player.name" );
	setup_item( &g_cfg.player.name_color, Color( 255, 255, 255, 255 ), "Player.name_color" );
	setup_item( &g_cfg.player.weapon, false, "Player.weapon" );
	setup_item( &g_cfg.player.weapon_color, Color( 255, 255, 255, 255 ), "Player.weapon_color" );
	setup_item( &g_cfg.player.ammo, 0, "Player.ammo" );
	setup_item( &g_cfg.player.ammobar_color, Color( 255, 255, 255, 255 ), "Player.ammobar_color" );
	setup_item(&g_cfg.player.lby, false, "Player.lby");
	setup_item(&g_cfg.player.lbybar_color, Color(255, 255, 255, 255), "Player.lbybar_color");

	setup_item( &g_cfg.player.flags, {
		{ false, "scoped" },
		{ false, "balance" },
		{ false, "armor" },
		{ false, "vulnerable" },
		{ false, "lowerbody timer" }
	}, "Player.flags" );

	setup_item( &g_cfg.player.skeleton, {
		{ false, "normal" },
		{ false, "backtracked" }
	}, "Player.skeleton" );

	setup_item( &g_cfg.player.glow, {
		{ false, "enemy" },
		{ false, "teammate" }
	}, "Player.glow" );

	setup_item( &g_cfg.player.glow_type, 0, "Player.glow_type" );
	setup_item( &g_cfg.player.glow_color_enemy, Color( 255, 255, 255, 255 ), "Player.glow_color_enemy" );
	setup_item( &g_cfg.player.glow_color_teammate, Color( 255, 255, 255, 255 ), "Player.glow_color_teammate" );
	setup_item( &g_cfg.player.glowopacity, 100, "Player.glowopacity" );
	setup_item( &g_cfg.player.glowbloom, 100, "Player.glowbloom" );

	setup_item( &g_cfg.player.chams, {
		{ false, "visible" },
		{ false, "invisible" }
	}, "Player.chams" );

	setup_item( &g_cfg.player.chams_color, Color( 255, 255, 255, 255 ), "Player.chams_color" );
	setup_item( &g_cfg.player.xqz_color, Color( 255, 255, 255, 255 ), "Player.xqz_color" );
	setup_item( &g_cfg.player.chams_opacity, 100, "Player.chams_opacity" );
	setup_item( &g_cfg.player.chams_type, 0, "Player.chams_type" );
	setup_item( &g_cfg.player.lowerbody_indicator, false, "Player.lowerbody_indicator");


	setup_item( &g_cfg.player.backtrack_chams, false, "Player.backtrack_chams" );
	setup_item( &g_cfg.player.backtrack_chams_color, Color( 255, 255, 255 ), "Player.backtrack_chams_color" );
	setup_item( &g_cfg.player.backtrack_chams_opacity, 100, "Player.backtrack_chams_opacity" );

	setup_item( &g_cfg.esp.fov, 0, "Esp.fov" );
	setup_item( &g_cfg.esp.override_fov_scoped, 0, "Esp.override_fov_scoped" );

	setup_item( &g_cfg.esp.transparency_scoped, 0, "Esp.transparency_scoped" );
	setup_item( &g_cfg.esp.transparency_scoped_alpha, 0, "Esp.transparency_scoped_alpha" );

	setup_item( &g_cfg.esp.spectator_list, false, "Esp.spectator_list" );

	setup_item( &g_cfg.esp.show_spread, false, "Esp.show_spread" );
	setup_item( &g_cfg.esp.show_spread_type, 0, "Esp.show_spread_type" );
	setup_item( &g_cfg.esp.show_spread_color, Color( 255, 255, 255 ), "Esp.show_spread_color" );
	setup_item( &g_cfg.esp.show_spread_opacity, 0, "Esp.show_spread_opacity" );

	setup_item( &g_cfg.esp.crosshair, false, "Esp.crosshair" );
	setup_item( &g_cfg.esp.crosshair_color, Color( 255, 255, 255 ), "Esp.crosshair_color" );
	setup_item( &g_cfg.esp.penetration_reticle, false, "Esp.penetration_reticle" );
	setup_item( &g_cfg.esp.antiaim_indicator, false, "Esp.antiaim_indicator");

	setup_item( &g_cfg.esp.ghost_chams, false, "Esp.ghost_chams" );
	setup_item( &g_cfg.esp.ghost_chams_color, Color( 255, 0, 0 ), "Esp.ghost_chams_color" );

	setup_item(&g_cfg.esp.local_chams, false, "Esp.local_chams");
	setup_item(&g_cfg.esp.local_chams_color, Color(255, 255, 255), "Esp.local_chams_color");

	setup_item( &g_cfg.esp.bullet_tracer, false, "Esp.bullet_tracer" );
	setup_item( &g_cfg.esp.bullet_tracer_color, Color( 255, 255, 255 ), "Esp.bullet_tracer_color" );

	setup_item( &g_cfg.esp.dropped_weapons, false, "Esp.dropped_weapons" );
	setup_item( &g_cfg.esp.dropped_weapons_color, Color( 255, 255, 255 ), "Esp.dropped_weapons_color" );

	setup_item( &g_cfg.esp.projectiles, false, "Esp.projectiles" );
	setup_item( &g_cfg.esp.projectiles_color, Color( 255, 255, 255 ), "Esp.projectiles_color" );

	setup_item( &g_cfg.esp.planted_bomb, false, "Esp.planted_bomb" );
	setup_item( &g_cfg.esp.planted_bomb_color, Color( 255, 255, 255 ), "Esp.planted_bomb_color" );

	setup_item( &g_cfg.esp.dont_render_teammates, false, "Esp.dont_render_teammates" );
	setup_item( &g_cfg.esp.nightmode, false, "Esp.nightmode" );
	setup_item( &g_cfg.esp.bomb_timer, false, "Esp.bombtimer" );

	setup_item( &g_cfg.esp.hitmarker, {
		{ false, "static" },
		{ false, "dynamic" }
		}, "Esp.hitmarker" );

	setup_item( &g_cfg.esp.hitsound, false, "Esp.hitsound");

	setup_item( &g_cfg.esp.removals, {
		{ false, "scope" },
		{ false, "smoke" },
		{ false, "flash" },
		{ false, "recoil" }
	}, "Esp.removals" );
}

void C_ConfigManager::add_item( void * pointer, const char * name, std::string type ) {
	items.push_back( new C_ConfigItem( std::string( name ), pointer, type ) );
}

void C_ConfigManager::setup_item( int * pointer, int value, std::string name ) {
	add_item( pointer, name.c_str( ), "int" );
	*pointer = value;
}

void C_ConfigManager::setup_item( bool * pointer, bool value, std::string name ) {
	add_item( pointer, name.c_str( ), "bool" );
	*pointer = value;
}

void C_ConfigManager::setup_item( float * pointer, float value, std::string name ) {
	add_item( pointer, name.c_str( ), "float" );
	*pointer = value;
}

void C_ConfigManager::setup_item( ButtonCode_t * pointer, ButtonCode_t value, std::string name ) {
	add_item( pointer, name.c_str( ), "ButtonCode" );
	*pointer = value;
}

void C_ConfigManager::setup_item( Color * pointer, Color value, std::string name ) {
	add_item( pointer, name.c_str( ), "Color" );
	*pointer = value;
}

void C_ConfigManager::setup_item( std::vector< bool > * pointer, std::string name ) {
	add_item( pointer, name.c_str( ), "vector<int>" );
}

void C_ConfigManager::setup_item( std::vector< int > * pointer, std::string name ) {
	add_item( pointer, name.c_str( ), "vector<int>" );
}

void C_ConfigManager::setup_item( std::vector< MultiDropdownItem_t > * pointer, std::vector< MultiDropdownItem_t > value, std::string name ) {
	add_item( pointer, name.c_str( ), "vector<MultiDropdownItem_t>" );

	std::vector< MultiDropdownItem_t > array = *pointer;

	pointer->clear( );

	for ( int i = 0; i < value.size( ); i++ ) {
		pointer->push_back( { value[ i ].enabled, value[ i ].name } );
	}
}

void C_ConfigManager::save( std::string config ) {
	std::string folder, file;

	auto get_dir = [ &folder, &file, &config ] ( ) ->void {
		static TCHAR path[ MAX_PATH ];
		if ( SUCCEEDED( SHGetFolderPath( NULL, CSIDL_APPDATA, NULL, 0, path ) ) ) {
			folder = std::string( path ) + "\\rolex\\";
			file = std::string( path ) + "\\rolex\\" + config;
		}

		CreateDirectory( folder.c_str( ), NULL );
	};
	get_dir( );

	std::ofstream ofs;

	ofs.open( file + "", std::ios::out | std::ios::trunc );

	json allJson;

	for ( auto it : items ) {
		json j;

		j[ "name" ] = it->name;
		j[ "type" ] = it->type;

		if ( !it->type.compare( "int" ) ) {
			j[ "value" ] = ( int )*( int* )it->pointer;
		} else if ( !it->type.compare( "float" ) ) {
			j[ "value" ] = ( float )*( float* )it->pointer;
		} else if ( !it->type.compare( "bool" ) ) {
			j[ "value" ] = ( bool )*( bool* )it->pointer;
		} else if ( !it->type.compare( "ButtonCode" ) ) {
			j[ "value" ] = ( int )*( int* )it->pointer;
		} else if ( !it->type.compare( "Color" ) ) {
			Color c = *( Color* )( it->pointer );

			std::vector<int> a = { c.r( ), c.g( ), c.b( ), c.a( ) };

			json ja;

			for ( auto& i : a ) {
				ja.push_back( i );
			}

			j[ "value" ] = ja.dump( );
		} else if ( !it->type.compare( "vector<int>" ) ) {
			auto& ptr = *( std::vector<int>* )( it->pointer );

			json ja;

			for ( auto& i : ptr )
				ja.push_back( i );

			j[ "value" ] = ja.dump( );
		} else if ( !it->type.compare( "vector<MultiDropdownItem_t>" ) ) {
			auto& ptr = *( std::vector<MultiDropdownItem_t>* )( it->pointer );

			std::vector< std::string > a_str;
			for ( int i = 0; i < ptr.size( ); i++ ) {
				a_str.push_back( ptr[ i ].enabled ? "1" : "0" );
			}

			json ja;
			for ( auto & i : a_str ) {
				ja.push_back( i );
			}

			j[ "value" ] = ja.dump( );
		}

		allJson.push_back( j );
	}

	std::string data = allJson.dump( );

	ofs << std::setw( 4 ) << data << std::endl;

	ofs.close( );
}

void C_ConfigManager::load( std::string config ) {
	static auto find_item = [ ] ( std::vector< C_ConfigItem* > items, std::string name ) -> C_ConfigItem* {
		for ( int i = 0; i < ( int )items.size( ); i++ ) {
			if ( items[ i ]->name.compare( name ) == 0 )
				return ( items[ i ] );
		}

		return nullptr;
	};

	static auto right_of_delim = [ ] ( std::string const& str, std::string const& delim ) -> std::string {
		return str.substr( str.find( delim ) + delim.size( ) );
	};

	std::string folder, file;

	auto get_dir = [ &folder, &file, &config ] ( ) ->void {
		static TCHAR path[ MAX_PATH ];
		if ( SUCCEEDED( SHGetFolderPath( NULL, CSIDL_APPDATA, NULL, 0, path ) ) ) {
			folder = std::string( path ) + "\\rolex\\";
			file = std::string( path ) + "\\rolex\\" + config;
		}

		CreateDirectory( folder.c_str( ), NULL );
	};

	get_dir( );

	std::ifstream ifs;
	std::string data;

	std::string path = file + "";

	ifs.open( path );

	json allJson;

	ifs >> allJson;

	for ( json::iterator it = allJson.begin( ); it != allJson.end( ); ++it ) {
		json j = *it;

		std::string name = j[ "name" ];
		std::string type = j[ "type" ];

		C_ConfigItem *item = find_item( items, name );

		if ( item ) {
			if ( !type.compare( "int" ) ) {
				*( int* )item->pointer = j[ "value" ].get<int>( );
			} else if ( !type.compare( "float" ) ) {
				*( float* )item->pointer = j[ "value" ].get<float>( );
			} else if ( !type.compare( "bool" ) ) {
				*( bool* )item->pointer = j[ "value" ].get<bool>( );
			} else if ( !type.compare( "ButtonCode" ) ) {
				*( int* )item->pointer = j[ "value" ].get<int>( );
			} else if ( !type.compare( "Color" ) ) {
				std::vector<int> a;

				json ja = json::parse( j[ "value" ].get<std::string>( ).c_str( ) );

				for ( json::iterator it = ja.begin( ); it != ja.end( ); ++it )
					a.push_back( *it );

				*( Color* )item->pointer = Color( a[ 0 ], a[ 1 ], a[ 2 ], a[ 3 ] );
			} else if ( !type.compare( "vector<int>" ) ) {
				auto ptr = static_cast< std::vector <int>* > ( item->pointer );
				ptr->clear( );

				json ja = json::parse( j[ "value" ].get<std::string>( ).c_str( ) );

				for ( json::iterator it = ja.begin( ); it != ja.end( ); ++it )
					ptr->push_back( *it );
			} else if ( !type.compare( "vector<MultiDropdownItem_t>" ) ) {
				auto ptr = static_cast< std::vector<MultiDropdownItem_t>* > ( item->pointer );
				for ( int i = 0; i < ptr->size( ); i++ ) {
					ptr->at( i ).enabled = false;
				}

				json ja = json::parse( j[ "value" ].get<std::string>( ).c_str( ) );

				std::vector < std::string > option_array;
				for ( json::iterator it = ja.begin( ); it != ja.end( ); ++it ) {
					std::string it_converted = *it;
					it_converted.erase( std::remove( it_converted.begin( ), it_converted.end( ), '"' ), it_converted.end( ) );
					option_array.push_back( it_converted );
				}

				for ( int i = 0; i < option_array.size( ); i++ ) {
					ptr->at( i ).enabled = option_array[ i ].c_str( )[ 0 ] == '1' ? true : false;
				}
			}
		}
	}

	ifs.close( );
}

void C_ConfigManager::remove( std::string config ) {
	std::string folder, file;

	auto get_dir = [ &folder, &file, &config ] ( ) ->void {
		static TCHAR path[ MAX_PATH ];
		if ( SUCCEEDED( SHGetFolderPath( NULL, CSIDL_APPDATA, NULL, 0, path ) ) ) {
			folder = std::string( path ) + "\\rolex\\";
			file = std::string( path ) + "\\rolex\\" + config;
		}

		CreateDirectory( folder.c_str( ), NULL );
	};
	get_dir( );

	std::string path = file + "";

	std::remove( path.c_str( ) );
}

void C_ConfigManager::config_files( ) {
	std::string folder;

	auto get_dir = [ &folder ] ( ) -> void {
		static TCHAR path[ MAX_PATH ];
		if ( SUCCEEDED( SHGetFolderPath( NULL, CSIDL_APPDATA, NULL, 0, path ) ) ) {
			folder = std::string( path ) + "\\rolex\\";
		}

		CreateDirectory( folder.c_str( ), NULL );
	};
	get_dir( );

	files.clear( );

	std::string path = folder + "/*.json";// "/*.*";

	WIN32_FIND_DATA fd;

	HANDLE hFind = ::FindFirstFile( path.c_str( ), &fd );

	if ( hFind != INVALID_HANDLE_VALUE ) {
		do {
			if ( !( fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ) ) {
				files.push_back( fd.cFileName );
			}
		} while ( ::FindNextFile( hFind, &fd ) );

		::FindClose( hFind );
	}
}











































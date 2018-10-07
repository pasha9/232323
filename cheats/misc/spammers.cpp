#include "spammers.h"

void spammers::clan_tag( ) {
	auto apply = [ ] ( const char * name ) -> void {
		using Fn = int( __fastcall * )( const char *, const char * );
		static auto fn = reinterpret_cast< Fn >( util::pattern_scan( "engine.dll", "53 56 57 8B DA 8B F9 FF 15" ) );
		fn( name, name );
	};

	static std::string tag = "    rolex    ";

	static float last_time = 0;

	if ( g_csgo.m_globals( )->m_curtime > last_time ) {
		tag += tag.at( 0 );
		tag.erase( 0, 1 );
		std::string dollarsign = u8"♕   ";
		dollarsign += tag;
		apply( dollarsign.c_str( ) );

		last_time = g_csgo.m_globals( )->m_curtime + 0.9f;
	}

	if ( fabs( last_time - g_csgo.m_globals( )->m_curtime ) > 1.f )
		last_time = g_csgo.m_globals( )->m_curtime;
}










































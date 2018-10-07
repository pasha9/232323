#include "world_esp.h"

void worldesp::paint_traverse( ) {
	if ( !g_ctx.available( ) )
		return;

	for ( int i = 1; i < g_csgo.m_entitylist()->GetHighestEntityIndex( ); i++ ) {
		auto e = static_cast< entity_t * >( g_csgo.m_entitylist( )->GetClientEntity( i ) );

		if ( !e )
			continue;

		if ( e->IsDormant( ) )
			continue;

		Box box; if ( !util::get_bbox_world( e, box ) )
			continue;

		if ( g_cfg.esp.projectiles )
			grenades( ( weapon_t * )e, box );

		if ( g_cfg.esp.dropped_weapons )
			weapons( ( weapon_t * )e, box );
	}
}

void worldesp::grenades( weapon_t * m_nade, Box box ) {
	const model_t* model = m_nade->GetModel( );
	if ( !model )
		return;

	studiohdr_t* hdr = g_csgo.m_modelinfo( )->GetStudiomodel( model );
	if ( !hdr )
		return;

	std::string entity_name = hdr->szName;

	switch ( m_nade->GetClientClass( )->m_ClassID ) {
	case 9:
		entity_name = entity_name[ 16 ] == 's' ? "FLASH" : "FRAG";
		break;
	case 134:
		entity_name = "SMOKE";
		break;
	case 98:
		entity_name = "FIRE";
		break;
	case 41:
		entity_name = "DECOY";
		break;
	default:
		return;
	}

	Color color = g_cfg.esp.projectiles_color;

	render::get( ).text( fonts[ rolexTEXT ], box.x + ( box.w / 2 ), box.y, color, HFONT_CENTERED_X, entity_name.c_str( ) );
}

void worldesp::weapons( weapon_t * e, Box box ) {
	if ( !e->is_dropped_weapon( ) )
		return;

	char * name = e->get_name( );

	Color color = g_cfg.esp.dropped_weapons_color;

	render::get( ).text( fonts[ rolexTEXT ], box.x + ( box.w / 2 ), box.y, color, HFONT_CENTERED_X, name );
}










































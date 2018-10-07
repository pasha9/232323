#include "..\hooks.hpp"

using OverrideView_t = void( __stdcall* )( CViewSetup* );

void __stdcall hooks::hooked_overrideview( CViewSetup* viewsetup ) {
	static auto original_fn = clientmode_hook->get_func_address<OverrideView_t >( 18 );

	if ( g_ctx.available( ) ) {
		auto scoped = g_ctx.m_local->m_bIsScoped( );
		if ( !scoped || ( scoped && g_cfg.esp.override_fov_scoped ) ) {
			viewsetup->fov += g_cfg.esp.fov;
		}
	}

	return original_fn( viewsetup );
}








































#include "includes.hpp"

#include <thread>

bool unload = false;

DWORD WINAPI OnDllAttach( PVOID base ) {

	materials::get( ).setup( );

	render::get( ).setup( );

	cfg_manager->setup( );

	setup_main_menu( );

	netvars::get( ).init( );

	hooks::load( );

	while (!unload) {
		using namespace std::literals::chrono_literals;
		std::this_thread::sleep_for(1s);
	}

	hooks::shutdown();
	std::this_thread::sleep_for(std::chrono::milliseconds(100));

	fclose((FILE*)stdin);
	fclose((FILE*)stdout);
	FreeConsole();

	FreeLibraryAndExitThread(static_cast< HMODULE >(base), 1);
}

BOOL WINAPI DllMain( HMODULE hModule, DWORD dwReason, LPVOID lpReserved ) {
	if ( dwReason == DLL_PROCESS_ATTACH ) { 
		DisableThreadLibraryCalls( hModule );
		CreateThread( nullptr, NULL, OnDllAttach, hModule, NULL, nullptr );
	}

	return TRUE;
}
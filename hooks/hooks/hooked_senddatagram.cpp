#include "..\hooks.hpp"

#include "..\..\cheats\lagcompensation\lagcompensation.h"

using SendDatagram_t = int( __thiscall * )( void *, void * );

int __fastcall hooks::hooked_senddatagram( void * net_channel, void *, void * datagram ) {
	static auto original_fn = netchannel_hook->get_func_address<SendDatagram_t>( 46 );

	if ( !g_cfg.misc.ping_spike || !g_ctx.available( ) )
		return original_fn( net_channel, datagram );

	auto * channel = reinterpret_cast< INetChannel * >( net_channel );
	
	int instate = channel->m_nInReliableState;
	int insequencenr = channel->m_nInSequenceNr;

	lagcompensation::get( ).add_latency( channel );

	int ret = original_fn( channel, datagram );

	channel->m_nInReliableState = instate;
	channel->m_nInSequenceNr = insequencenr;

	return ret;
}








































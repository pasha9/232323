#pragma once

#include "..\..\includes.hpp"

class manual_aa : public singleton< manual_aa > {
private:
	enum {
		SIDE_NONE,
		SIDE_LEFT,
		SIDE_RIGHT,
		SIDE_BACK
	};
	int side = SIDE_NONE;
public:
	void update( );
	bool run( );
};
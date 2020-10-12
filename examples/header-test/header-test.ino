/*

Module:	header-test.ino

Function:
	Test header file for MCCI_Catena_SCD30 library

Copyright and License:
	This file copyright (C) 2020 by

		MCCI Corporation
		3520 Krums Corners Road
		Ithaca, NY  14850

	See accompanying LICENSE file for copyright and license information.

Author:
	fullname, MCCI Corporation	October 2020

*/

#include "MCCI_Catena_SCD30.h"

static_assert(
	McciCatenaScd30::kVersion > McciCatenaScd30::makeVersion(0,0,0),
	"version must be > 0.0.0"
	);

void setup()
	{
	}

void loop()
	{
	}

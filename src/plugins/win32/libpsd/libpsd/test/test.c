/**
 * libpsd - Photoshop file formats (*.psd) decode library
 * Copyright (C) 2004-2007 Graphest Software.
 *
 * libpsd is the legal property of its developers, whose names are too numerous
 * to list here.  Please refer to the COPYRIGHT file distributed with this
 * source distribution.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Library General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * $Id: test.c, created by Patrick in 2007.02.24, libpsd@graphest.com Exp $
 */

#include <stdio.h>
#include <stdlib.h>

#include "libpsd.h"

int main (int argc, char **argv)
{
	psd_context * context = NULL;
	psd_status status;
	
	if(argc <= 1)
		return -1;

	// parse the psd file to psd_context
	status = psd_image_load(&context, argv[1]);

	// use it...
	//
	
	// free if it's done
	psd_image_free(context);

	return (status == psd_status_done ? 0 : -1);
}

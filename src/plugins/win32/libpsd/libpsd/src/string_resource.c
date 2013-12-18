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
 * $Id: string_resource.c $
 */

#include "libpsd.h"
#include "psd_config.h"
#include "psd_system.h"
#include "psd_stream.h"


// for unicode string resoruce added by miahmie. 2007/11/26

void psd_unicode_strings_free(psd_context * context)
{
	int i;

	if(context->unicode_strings == NULL)
		return;

	for (i = 0; i < context->number_of_unicode_strings; i ++)
		psd_freeif(context->unicode_strings[i].name);

	psd_freeif(context->unicode_strings);
}

psd_int psd_stream_get_unicode_string(psd_context * context)
{
	psd_int length, id;
	psd_ushort * buffer;
	psd_unicode_strings * strings;

	length = psd_stream_get_int(context);
	id = context->number_of_unicode_strings++;

	if(id == 0)
	{
		context->unicode_strings = (psd_unicode_strings *)psd_malloc(sizeof(psd_unicode_strings));
		if(context->unicode_strings == NULL)
			return psd_status_malloc_failed;
	}
	else
	{
		context->unicode_strings = (psd_unicode_strings *)psd_realloc(context->unicode_strings, (id + 1) * sizeof(psd_unicode_strings));
		if(context->unicode_strings == NULL)
			return psd_status_malloc_failed;
	}
	strings = context->unicode_strings + id;
	strings->id = id;
	if(length != 0)
	{
		buffer = (psd_ushort *)psd_malloc(length * 2);
		strings->name_length = length;
		strings->name = buffer;

		psd_stream_get(context, (psd_uchar*)buffer, length * 2);
	}
	else
	{
		strings->name_length = length;
		strings->name = NULL;
		psd_stream_get_null(context, length * 2);
	}
	return id;
}


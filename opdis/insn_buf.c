/*!
 * \file insn_buf.c
 * \brief Buffer for building Opdis instructions from libopcodes output.
 * \author TG Community Developers <community@thoughtgang.org>
 * \note Copyright (c) 2010 ThoughtGang.
 * Released under the GNU Lesser Public License (LGPL), version 3.
 * See http://www.gnu.org/licenses/gpl.txt for details.
 */

#include <stdlib.h>
#include <string.h>

#include <opdis/insn_buf.h>

opdis_insn_buf_t LIBCALL opdis_insn_buf_alloc( unsigned int max_items, 
					       unsigned int max_item_size,
					       unsigned int max_insn_str ) {
	int i;

	opdis_insn_buf_t buf = (opdis_insn_buf_t) calloc( 1,
						sizeof(opdis_insn_buffer_t) );
	if (! buf ) {
		return NULL;
	}

	max_items = (max_items == 0) ? OPDIS_MAX_ITEMS : max_items;
	max_item_size = (max_item_size == 0) ? OPDIS_MAX_ITEM_SIZE : 
					     max_item_size;
	max_insn_str = (max_insn_str == 0) ? OPDIS_MAX_INSN_STR : max_insn_str;

	buf->items = (char **) calloc( max_items, sizeof(char *));
	if (! buf->items ) {
		free(buf);
		return NULL;
	}
	for ( i = 0; i < max_items; i++ ) {
		buf->items[i] = calloc( 1, max_item_size );
		if (! buf->items[i] ) {
			buf->max_item_count = i;
			opdis_insn_buf_free( buf );
			return NULL;
		}
	}
			
	buf->max_item_count = max_items;
	buf->max_item_size = max_item_size;

	buf->string = (char *) calloc( 1, max_insn_str );
	if (! buf->string ) {
		opdis_insn_buf_free(buf);
		return NULL;
	}
	buf->max_string_size = max_insn_str;

	return buf;
}

int LIBCALL opdis_insn_buf_append( opdis_insn_buf_t buf, const char * item ) {
	char *dest;
	unsigned int len;

	if (! buf || ! buf->items || buf->item_count == buf->max_item_count ) {
		return 0;
	}

	if ( buf->item_count < buf->max_item_count ) {
		dest = buf->items[buf->item_count];
		strncpy( dest, item, buf->max_item_size );
		buf->item_count++;
	}

	len = buf->max_string_size - strlen(buf->string) - 1;
	strncat( buf->string, item, len );

	return 1;
}

void LIBCALL opdis_insn_buf_clear( opdis_insn_buf_t buf ) {
	if ( buf ) {
		buf->item_count = 0;
		buf->string[0] = '\0';
	}
}

void LIBCALL opdis_insn_buf_free( opdis_insn_buf_t buf ) {
	if (! buf ) {
		return;
	}

	if ( buf->items ) {
		int i;
		for ( i = 0; i < buf->max_item_count; i++ ) {
			if ( buf->items[i] ) {
				free( buf->items[i] );
				buf->items[i] = NULL;
			}
		}

		free(buf->items);
	}

	if ( buf->string ) {
		free(buf->string);
	}

	free(buf);
}

/*!
 * \file insn_buf.c
 * \brief Buffer for building Opdis instructions from libopcodes output.
 * \author thoughtgang.org
 */

#include <string.h>

#include <opdis/insn_buf.h>

opdis_insn_buf_t LIBCALL opdis_insn_buf_alloc( unsigned int max_items, 
					       unsigned int max_item_size,
					       unsigned int max_insn_str ) {
	opdis_insn_buf_t buf = (opdis_insn_buf_t) calloc( 1,
						sizeof(opdis_insn_buffer_t) );
	if (! buf ) {
		return NULL;
	}

	buf->items = (char *) calloc( max_items, max_item_size );
	if (! buf->items ) {
		free(buf);
		return NULL;
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

	if (! buf || ! buf->items || ! dest ||
	      buf->item_count == buf->max_item_count ) {
		return 0;
	}

	dest = buf->items[buf->item_count];
	strncpy( dest, item, buf->max_item_size );
	buf->item_count++;
	len = buf->max_string_size - strlen(buf->string) - 1;
	strncat( buf->string, item, len );

	return 1;
}

void LIBCALL opdis_insn_buf_free( opdis_insn_buf_t buf ) {
	if (! buf ) {
		return;
	}

	if ( buf->items ) {
		free(buf->items);
	}

	if ( buf->string ) {
		free(buf->string);
	}

	free(buf);
}

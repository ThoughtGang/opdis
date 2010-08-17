/*!
 * \file types.c
 * \brief Base types used by opdis
 * \author TG Community Developers <community@thoughtgang.org>
 * \note Copyright (c) 2010 ThoughtGang.
 * Released under the GNU Lesser Public License (LGPL), version 3.
 * See http://www.gnu.org/licenses/gpl.txt for details.
 */

#include <stdlib.h>
#include <string.h>

#include <opdis/types.h>

opdis_buf_t LIBCALL opdis_buf_alloc( opdis_off_t size, opdis_vma_t addr ) {
	opdis_buf_t buf = (opdis_buf_t) calloc( 1, sizeof(opdis_buffer_t) );
	if (! buf ) {
		return NULL;
	}

	buf->len = size;
	buf->vma = addr;
	buf->data = (opdis_byte_t *) calloc( 1, size );

	if (! buf->data ) {
		free( buf );
		return NULL;
	}

	return buf;
}

void LIBCALL opdis_buf_set_vma( opdis_buf_t buf, opdis_vma_t addr ) {
	if ( buf ) {
		buf->vma = addr;
	}
}

static opdis_off_t get_read_length( FILE * f ) {
	opdis_off_t size;
	long pos = ftell(f);
	if ( pos == -1 || fseek(f, 0, SEEK_END) == -1 ) {
		return 0;
	}

	size = ftell(f) - pos;
	if ( size == -1 || fseek(f, pos, SEEK_SET) == -1 ) {
		return 0;
	}

	return size;
}

opdis_buf_t LIBCALL opdis_buf_read( FILE * f, opdis_off_t size, 
				    opdis_vma_t addr ) {
	opdis_buf_t buf;

	if (! size ) {
		size = get_read_length( f );
		if (! size ) {
			return NULL;
		}
	}

	buf = opdis_buf_alloc( size, addr );
	if (! buf ) {
		return NULL;
	}

	if ( fread( buf->data, 1, size, f ) < size ) {
		fprintf( stderr, "opdis_buf_read: cannot read %d bytes\n", 
				  (int) size );
	}

	return buf;
}

int LIBCALL opdis_buf_fill( opdis_buf_t buf, opdis_off_t offset,
			    void * src, opdis_off_t len ) {
	if ( ! buf || ! buf->data || ! src || ! len || 
	     offset + len > buf->len ) {
		return 0;
	}

	memcpy( &buf->data[offset], src, len );

	return len;
}

void LIBCALL opdis_buf_free( opdis_buf_t buf ) {
	if ( buf ) {
		if ( buf->data ) {
			free(buf->data);
		}
		free(buf);
	}
}

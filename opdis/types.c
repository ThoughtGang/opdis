/*!
 * \file types.c
 * \brief Base types used by opdis
 * \author thoughtgang.org
 */

#include <opdis/types.h>

opdis_buf_t LIBCALL opdis_buf_alloc( opdis_off_t size ) {
	// alloc opdis_buf
	// alloc buf->data
}

opdis_buf_t LIBCALL opdis_buf_read( FILE * f ) {
	// alloc opdis buf
	// fseek
	// alloc buf_data
	// fread
}

int LIBCALL opdis_buf_fill( opdis_buf_t buf, opdis_off_t offset,
			    void * src, opdist_off_t len ) {
	// straight memcpy
}

void LIBCALL opdis_buf_free( opdis_buf_t buf ) {
	// free buf->data
	// free buf
}

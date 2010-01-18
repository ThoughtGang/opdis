/*!
 * \file types.h
 * \brief Base types used by opdis
 * \author thoughtgang.org
 */

#ifndef OPDIS_TYPES_H
#define OPDIS_TYPES_H

#ifdef WIN32
        #define LIBCALL _stdcall
#else
        #define LIBCALL
#endif

typedef bfd_byte opdis_byte_t;
typedef size_t opdis_off_t;
typedef bfd_vma opdis_addr_t;

typedef struct {
	opdis_off_t 		len;
	opdis_byte_t * 		data;
} opdis_buffer_t;
typedef opdis_buffer_t * opdis_buf_t;

#ifdef __cplusplus
extern "C"
{
#endif

/*!
 * \fn
 * \brief
 * \param
 * \relates
 * \sa
 */

opdis_buf_t LIBCALL opdis_buf_alloc( opdis_off_t size );

opdis_buf_t LIBCALL opdis_buf_read( FILE * f );

int LIBCALL opdis_buf_fill( opdis_buf_t buf, opdis_off_t offset,
			    void * src, opdis_off_t len );

void LIBCALL opdis_buf_free( opdis_buf_t buf );

#ifdef __cplusplus
}
#endif

#endif

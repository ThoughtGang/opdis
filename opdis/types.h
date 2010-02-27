/*!
 * \file types.h
 * \brief Base types used by opdis
 * \details This defines the types used by the opdis disassembler.
 * \author TG Community Developers <community@thoughtgang.org>
 * \note Copyright (c) 2010 ThoughtGang.
 * Released under the GNU Public License, version 3.
 * See http://www.gnu.org/licenses/gpl.txt for details.
 */

#ifndef OPDIS_TYPES_H
#define OPDIS_TYPES_H

#include <bfd.h>
#include <stdio.h>
#include <stdint.h>
#include <sys/types.h>

#ifdef WIN32
        #define LIBCALL _stdcall
#else
        #define LIBCALL
#endif

/*! \typedef bfd_byte opdis_byte_t
 *  \ingroup types
 *  \brief A byte.
 */
typedef bfd_byte opdis_byte_t;

/*! \typedef size_t opdis_off_t;
 *  \brief A buffer offset.
 */
typedef size_t opdis_off_t;

/*! \def OPDIS_INVALID_OFFSET
 *  An invalid buffer offset.
 *  \ingroup types
 *  \sa opdis_off_t
 */
#define OPDIS_INVALID_OFFSET ((opdis_off_t) -1 )

/*! \typedef bfd_vma opdis_vma_t;
 *  \ingroup types
 *  A virtual memory (load) address.
 */
typedef bfd_vma opdis_vma_t;

/*! \def OPDIS_INVALID_ADDR
 *  An invalid address.
 *  \ingroup types
 *  \sa opdis_vma_t
 */
#define OPDIS_INVALID_ADDR ((opdis_vma_t) -1 )

/*! \struct opdis_buffer_t
 *  \ingroup types
 *  \brief A buffer containing bytes to disassemble.
 */
typedef struct {
	opdis_off_t 	len;	/*!< Number of bytes in buffer. */
	opdis_vma_t	vma;	/*!< Load address of buffer. */
	opdis_byte_t * 	data;	/*!< Contents of buffer. */
} opdis_buffer_t;

/*! \typedef opdis_buffer_t * opdis_buf_t
 *  \ingroup types
 *  \brief Pointer to an opdis buffer.
 */
typedef opdis_buffer_t * opdis_buf_t;

/* ---------------------------------------------------------------------- */

#ifdef __cplusplus
extern "C"
{
#endif

/*!
 * \fn opdis_buf_t opdis_buf_alloc( opdis_off_t, opdis_vma_t )
 * \ingroup types
 * \brief Allocate an opdis buffer
 * \details Allocates an opdis_buffer_t of specified size. The buffer contents
 *          are initialized to zero.
 * \param size Size of buffer in bytes.
 * \param addr Load address (vma) of buffer or 0.
 * \return The allocated opdis buffer.
 * \sa opdis_buf_read opdis_buf_free
 */

opdis_buf_t LIBCALL opdis_buf_alloc( opdis_off_t size, opdis_vma_t addr );

/*!
 * \fn void opdis_buf_set_vma( opdis_buf_t, opdis_vma_t )
 * \ingroup types
 * \brief Set the VMA for a buffer
 * \param buf The opdis buffer.
 * \param addr Load address (vma) of buffer or 0.
 */

void LIBCALL opdis_buf_set_vma( opdis_buf_t buf, opdis_vma_t addr );


/*!
 * \fn opdis_buf_t opdis_buf_read( FILE *, opdis_off_t, opdis_vma_t )
 * \ingroup types
 * \brief Allocate an opdis buffer containing contents of file
 * \details Allocates an opdis buffer and fills it with the contents of
 *          the file. This reads \e size bytes from the current position
 *          in \e f, or from the current position to the end of the file
 *          if \e size is 0. 
 * \param f The file to read from.
 * \param size The number of bytes to read from the file, or 0.
 * \param addr Load address (vma) of buffer or 0.
 * \return The allocated opdis buffer.
 * \sa opdis_buf_alloc opdis_buf_free
 * \note The current position of the file will be increased by \e size
 *       bytes upon return.
 */
opdis_buf_t LIBCALL opdis_buf_read( FILE * f, opdis_off_t size, 
				    opdis_vma_t addr );

/*!
 * \fn int opdis_buf_fill( opdis_buf_t, opdis_off_t, void *, opdis_off_t )
 * \ingroup types
 * \brief Fill an opdis buffer from a memory location
 * \param buf Opdis buffer to fill.
 * \param offset Offset in buf to copy to.
 * \param src Memory location to copy from.
 * \param len Number of bytes to copy from src to buf.
 * \return Number of bytes copied from src to buf.
 */
int LIBCALL opdis_buf_fill( opdis_buf_t buf, opdis_off_t offset,
			    void * src, opdis_off_t len );

/*!
 * \fn void opdis_buf_free( opdis_buf_t )
 * \ingroup types
 * \brief Free an opdis buffer.
 * \param buf Opdis buffer to free.
 * \sa opdis_buf_alloc
 */
void LIBCALL opdis_buf_free( opdis_buf_t buf );

#ifdef __cplusplus
}
#endif

#endif

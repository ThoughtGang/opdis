/*!
 * \file insn_buf.h
 * \brief Buffer for building Opdis instructions from libopcodes output.
 * \details This defines the buffer data structure used by libopdis
 *          to capture libopcodes output.
 * \author TG Community Developers <community@thoughtgang.org>
 * \note Copyright (c) 2010 ThoughtGang.
 * Released under the GNU Lesser Public License (LGPL), version 3.
 * See http://www.gnu.org/licenses/gpl.txt for details.
 */

#ifndef OPDIS_INSN_BUF_H
#define OPDIS_INSN_BUF_H

#ifdef WIN32
        #define LIBCALL _stdcall
#else
        #define LIBCALL
#endif

#include <bfd.h>
#include <dis-asm.h>

/* NOTE: These defaults amount to ~4K of space at runtime. Overkill for x86,
 *       but should be safe for all architectures that libopcodes supports.
 */

/*! \def OPDIS_MAX_ITEMS
 *  Max number of items (mnemonic, prefix, operand, etc) that buffer can store.
 *  \ingroup internal
 *  \sa opdis_insn_buffer_t
 */
#define OPDIS_MAX_ITEMS 64		

/*! \def OPDIS_MAX_ITEM_SIZE
 *  Max size of a single item.
 *  \ingroup internal
 *  \sa opdis_insn_buffer_t
 */
#define OPDIS_MAX_ITEM_SIZE 64		

/*! \def OPDIS_MAX_INSN_STR
 *  Max length of the instruction string.
 *  \ingroup internal
 *  \sa opdis_insn_buffer_t
 */
#define OPDIS_MAX_INSN_STR 128		

/* ---------------------------------------------------------------------- */
/*! \struct opdis_insn_buffer_t
 *  \ingroup internal
 *  \ingroup types
 *  \brief A buffer that stores the output of libopcodes before processing.
 *  \details This collects the strings emitted by libopcodes during 
 *           disassembly. A 'raw' string representation of the instruction 
 *           is also constructed.
 */
typedef struct {
	unsigned int item_count;		/*!< Number of items */
	unsigned int max_item_count;		/*!< Max number of items */
	unsigned int max_item_size;		/*!< Max size of items */
	char **items;				/*!< Array of stored items */
	char *string;				/*!< Raw instruction string */
	unsigned int max_string_size;		/*!< Max insn string length */
	/* instruction info from libopcodes disassemble_info struct */
	char insn_info_valid;			/*!< Nonzero if info is set */
	char branch_delay_insns;		/*!< Branch delay insn count */
	char data_size;				/*!< Size of insn data ref */
	enum dis_insn_type insn_type;		/*!< Type of insn */
	bfd_vma target;				/*!< Target addr of branch */
	bfd_vma target2;			/*!< Second addr ref */
} opdis_insn_buffer_t;

/*! \typedef opdis_insn_buf_t
 *  \ingroup internal
 *  \ingroup types
 */
typedef opdis_insn_buffer_t * opdis_insn_buf_t;

/* ---------------------------------------------------------------------- */
#ifdef __cplusplus
extern "C"
{
#endif

/*!
 * \fn opdis_insn_buf_t opdis_insn_buf_alloc( unsigned int, unsigned int, 
 * 					      unsigned int  )
 * \ingroup internal
 * \brief Allocate an instruction buffer
 * \details Allocates an opdis_insn_buffer_t based on the specified parameters.
 *          The buffer contents are initialized to zero.
 * \param max_items Default is \ref OPDIS_MAX_ITEMS.
 * \param max_item_size Default is \ref OPDIS_MAX_ITEM_SIZE.
 * \param max_insn_str Default is \ref OPDIS_MAX_INSN_STR.
 * \return The allocated opdis_insn_buffer_t.
 * \sa opdis_insn_buf_free
 */
opdis_insn_buf_t LIBCALL opdis_insn_buf_alloc( unsigned int max_items, 
					       unsigned int max_item_size,
					       unsigned int max_insn_str );

/*!
 * \fn void opdis_insn_buf_free( opdis_insn_buf_t )
 * \ingroup internal
 * \brief Free an instruction buffer.
 * \param buf The instruction buffer to free.
 * \sa opdis_insn_buf_alloc
 */
void LIBCALL opdis_insn_buf_free( opdis_insn_buf_t buf );

/*!
 * \fn int opdis_insn_buf_append( opdis_insn_buf_t, const char * )
 * \ingroup internal
 * \brief Append a string to an opdis_insn_buffer_t.
 * \details This will add the item to the array of items in the buffer, and
 *          increase the buffer item count. The item will also be appended
 *          to the raw instruction string.
 * \param buf The instruction buffer to append to.
 * \param item The item to append.
 * \return 1 on success, 0 on failure.
 * \sa opdis_insn_buf_alloc
 */
int LIBCALL opdis_insn_buf_append( opdis_insn_buf_t buf, const char * item );

/*!
 * \fn void opdis_insn_buf_clear( opdis_insn_buf_t )
 * \ingroup internal
 * \brief Clear the data in an opdis_insn_buffer_t.
 * \param buf The instruction buffer to clear.
 */
void LIBCALL opdis_insn_buf_clear( opdis_insn_buf_t buf );


#ifdef __cplusplus
}
#endif

#endif

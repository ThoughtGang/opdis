/*!
 * \file insn_buf.h
 * \brief Buffer for building Opdis instructions from libopcodes output.
 * \details This defines the buffer data structure used by libopdis
 *          to capture libopcodes output.
 * \author thoughtgang.org
 */

#ifndef OPDIS_INSN_BUF_H
#define OPDIS_INSN_BUF_H

#ifdef WIN32
        #define LIBCALL _stdcall
#else
        #define LIBCALL
#endif


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
	size_t item_count;			/*!< Number of items */
	char items[MAX_ITEMS][MAX_ITEM_SZ];	/*!< Array of stored items */
	char string[OPDIS_MAX_INSN_STR];	/*!< Raw instruction string */
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
 * \fn opdis_insn_buf_t opdis_insn_buf_alloc( size_t, size_t, size_t  )
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
opdis_insn_buf_t LIBCALL opdis_insn_buf_alloc( size_t max_items, 
					       size_t max_item_size,
					       size_t max_insn_str );

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
 * \sa opdis_insn_buf_alloc
 */
int LIBCALL opdis_insn_buf_append( opdis_insn_buf_t buf, const char * item );

#ifdef __cplusplus
}
#endif

#endif

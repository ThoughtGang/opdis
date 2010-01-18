/*!
 * \file insn_buf.h
 * \brief Buffer for building Opdis instructions from libopcodes output.
 * \author thoughtgang.org
 */

#ifndef OPDIS_INSN_BUF_H
#define OPDIS_INSN_BUF_H

#ifdef WIN32
        #define LIBCALL _stdcall
#else
        #define LIBCALL
#endif


// this amounts to < 5 k for runtime. overkill, but safe(?).
#define OPDIS_MAX_ITEMS 64		/* max # items (mnem, prefix, op) */
#define OPDIS_MAX_ITEM_SIZE 64		/* max size of a single insn item */
#define OPDIS_MAX_INSN_STR 128		/* max length of ASCII insn string */
//#define OPDIS_MAX_INSN_SZ 64		/* max bytes in insn */

/* ---------------------------------------------------------------------- */
typedef struct {
	size_t item_count;
	char items[MAX_ITEMS][MAX_ITEM_SZ];
	char string[OPDIS_MAX_INSN_STR];
} opdis_insn_buffer_t;

typedef opdis_insn_buffer_t * opdis_insn_buf_t;

/* ---------------------------------------------------------------------- */
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
opdis_insn_buf_t LIBCALL opdis_insn_buf_alloc( size_t max_items, 
					       size_t max_item_size,
					       size_t max_insn_str );

void LIBCALL opdis_insn_buf_term( opdis_insn_buf_t );

/*!
 * \fn
 * \brief
 * \param
 * \relates
 * \sa
 */
int LIBCALL opdis_insn_buf_append( opdis_insn_buf_t, const char * item );

#ifdef __cplusplus
}
#endif

#endif

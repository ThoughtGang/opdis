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
#define OPDIS_MAX_INSN_SZ 64		/* max bytes in insn */

/* ---------------------------------------------------------------------- */
// built from libopcodes lame 
typedef struct {
	size_t str_count;
	char strtab [MAX_ITEMS][MAX_ITEM_SZ];
	char insn[OPDIS_MAX_INSN_STR];
} opdis_insn_raw_t;

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

// tmp init
// tmp add item

#ifdef __cplusplus
}
#endif

#endif

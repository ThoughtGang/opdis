/* target_list.h
 *
 */

#include <bfd.h>

#ifndef TARGET_LIST_H
#define TARGET_LIST_H

#include <opdis/types.h>

/* Type of target: The target string is either a filename or an ASCII string of
 *                 bytes in hex/octal/etc */
enum target_type_t {
	tgt_bytes,	/* Target is a list of bytes */ 
	tgt_file	/* Target is a file */
};

/* A target */
typedef struct TARGET_LIST_ITEM {
	enum target_type_t type;	/* bytes or file */
	const char * ascii;		/* String value for target:
					   either filename or list of
					   bytes */
	opdis_buf_t data;		/* binary data for target */
	bfd * tgt_bfd;			/* BFD for target, if applicable */
	struct TARGET_LIST_ITEM * next;
} tgt_list_item_t;

/* A list of targets */
typedef struct TARGET_LIST_HEAD {
	unsigned int num_items;
	tgt_list_item_t * head;
} * tgt_list_t;

/* ---------------------------------------------------------------------- */

/* allocate a target list */
tgt_list_t tgt_list_alloc( void );

/* free an allocated target list */
void tgt_list_free( tgt_list_t );

// this will convert the string to bytes, or load the file contents 
// returns id for target
/* add a target to a list */
unsigned int tgt_list_add( tgt_list_t, enum target_type_t type, 
			   const char * name );

/* return the ID of the specified target */
/* note: ID is implicit : it is offset of item in list + 1 */
unsigned int tgt_list_id( tgt_list_t, const char * ascii );

/* return the data for the specified target ID */
opdis_buf_t tgt_list_data( tgt_list_t, unsigned int id );

/* return the name for the specified target ID */
const char * tgt_list_ascii( tgt_list_t, unsigned int id );

/* return the bfd for the specified target ID, or NULL */
bfd * tgt_list_bfd( tgt_list_t, unsigned int id );

typedef void (*TGT_LIST_FOREACH_FN) (tgt_list_item_t *, unsigned int id, 
				     void *);

/* invoke callback for every item in list */
void tgt_list_foreach( tgt_list_t, TGT_LIST_FOREACH_FN, void * arg );

/* print target list to f */
void tgt_list_print( tgt_list_t, FILE * f );

#endif

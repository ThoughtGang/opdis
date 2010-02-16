/* target_list.c
 *
 */

#include "target_list.h"

/* ---------------------------------------------------------------------- */

/* allocate a target list */
tgt_list_t tgt_list_alloc( void ) {
	return NULL;
}

/* free an allocated target list */
void tgt_list_free( tgt_list_t targets ) {
}

// this will convert the string to bytes, or load the file contents 
// returns id for target
/* add a target to a list */
unsigned int tgt_list_add( tgt_list_t targets, enum target_type_t type, 
			   const char * name ) {
	return 0;
}

/* return the ID of the specified target */
/* note: ID is implicit : it is offset of item in list + 1 */
unsigned int tgt_list_id( tgt_list_t targets, const char * ascii ) {
	return 0;
}

/* return the data for the specified target ID */
opdis_off_t tgt_list_data( tgt_list_t targets, unsigned int id ) {
	return 0;
}

/* return the name for the specified target ID */
const char * tgt_list_ascii( tgt_list_t targets, unsigned int id ) {
	return NULL;
}

/* invoke callback for every item in list */
void tgt_list_foreach( tgt_list_t targets, TGT_LIST_FOREACH_FN fn, void * arg ){
}

/* print target list to f */
void tgt_list_print( tgt_list_t targets, FILE * f ) {
}

/* map.c
 * map of memory addresses
 */

#include "map.h"

/* ---------------------------------------------------------------------- */
// maps are stored in a tree keyed by vma
// need key fn

/* allocate a memory map */
mem_map_t mem_map_alloc( void ) {
	return NULL;
}

/* free an allocated memory map */
void mem_map_free( mem_map_t ) {
}

/* map 'size' bytes at 'offset' into 'target' to load address 'vma' */
int mem_map_add( mem_map_t, unsigned int target, opdis_off_t offset,
		 opdis_off_t size, opdis_vma_t vma ) {
	return 0;
}

/* Invoke callback for each mapping */
void mem_map_foreach( mem_map_t, MEM_MAP_FOREACH_FN, void * arg ) {
}

/* print memory map to f */
void mem_map_print( mem_map_t, FILE * f ) {
}

/* map.c
 * map of memory addresses
 */

#include <stdio.h>
#include <stdlib.h>

#include "map.h"

/* ---------------------------------------------------------------------- */
// maps are stored in a tree keyed by vma
// need key fn

static void * map_key( void * data ) {
	map_t * map = (map_t *) data;
	return map ? (void *) map->vma : NULL;
}

/* allocate a memory map */
mem_map_t mem_map_alloc( void ) {
	return opdis_tree_init( map_key, NULL, free );
}

/* free an allocated memory map */
void mem_map_free( mem_map_t memmap ) {
	opdis_tree_free( (opdis_tree_t) memmap );
}

/* map 'size' bytes at 'offset' into 'target' to load address 'vma' */
int mem_map_add( mem_map_t memmap, unsigned int target, opdis_off_t offset,
		 opdis_off_t size, opdis_vma_t vma ) {
	// get closest
	// if vma <= closest->vma + closest size
	// 	error
	// get next
	// if vma + size >= next->vma
	// 	error
	// add
	return 0;
}

/* Invoke callback for each mapping */
void mem_map_foreach( mem_map_t memmap, MEM_MAP_FOREACH_FN fn, void * arg ) {
	opdis_tree_foreach( (opdis_tree_t) memmap, (OPDIS_TREE_FOREACH_FN) fn, 
			     arg );
}

static int print_memmap( map_t * map, void * arg ) {
	FILE *f = (FILE *) arg;
	if (! f ) {
		return;
	}
	// TODO
}

/* print memory map to f */
void mem_map_print( mem_map_t memmap, FILE * f ) {
	mem_map_foreach( memmap, print_memmap, f );
}

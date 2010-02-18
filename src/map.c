/* map.c
 * map of memory addresses
 */

#include <stdio.h>
#include <stdlib.h>

#include "map.h"

/* ---------------------------------------------------------------------- */
// maps are stored in a tree keyed by vma

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
	map_t * m = opdis_tree_closest( (opdis_tree_t) memmap, (void *) vma );
	if ( m && vma < (m->vma + m->size) ) {
		/* VMA is inside a memory block */
		fprintf( stderr, "Unable to map %p bytes at VMA %p: ",
			 (void *) size, (void *) vma );
		fprintf( stderr, "Region in contained in block %p (%p bytes)\n",
			 (void *) m->vma, (void *) m->size );
		return 0;
	}

	m = opdis_tree_next( (opdis_tree_t) memmap, (void *) vma );
	if ( m && (vma + size) - 1 >= m->vma ) {
		/* VMA extends into the next memory block */
		fprintf( stderr, "Unable to map %p bytes at VMA %p: ",
			 (void *) size, (void *) vma );
		fprintf( stderr, "Region overlaps block %p\n", 
			 (void *) m->vma );
		return 0;
	}
	
	m = (map_t *) calloc( 1, sizeof(map_t) );
	if (! m ) {
		return 0;
	}

	m->target = target;
	m->offset = offset;
	m->vma = vma;
	m->size = size;

	if (! opdis_tree_add( (opdis_tree_t) memmap, m ) ) {
		free(m);
		return 0;
	}

	return 1;
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

	fprintf( f, "\t%p - %p : Target %d [%p:%p]\n", (void *) map->vma,
		 (void *) (map->vma + map->size - 1), map->target,
		 (void *) map->offset, (void *) map->size );
}

/* print memory map to f */
void mem_map_print( mem_map_t memmap, FILE * f ) {
	mem_map_foreach( memmap, print_memmap, f );
}

/* map.h
 * map of memory addresses
 * Copyright (c) 2010 ThoughtGang
 * Written by TG Community Developers <community@thoughtgang.org>
 * Released under the GNU Public License, version 3.
 * See http://www.gnu.org/licenses/gpl.txt for details.
 */

#ifndef OPDIS_MAP_H
#define OPDIS_MAP_H

#include <opdis/tree.h>

/* map : this associates a VMA with bytes in a target. The VMA is the load
 *       address of the first byte.
 */

typedef struct {
	unsigned int target;
	opdis_off_t offset; /*into tgt */
	opdis_vma_t vma;
	opdis_off_t size;
} map_t;

typedef opdis_tree_t mem_map_t;

/* ---------------------------------------------------------------------- */

/* allocate a memory map */
mem_map_t mem_map_alloc( void );

/* free an allocated memory map */
void mem_map_free( mem_map_t );

/* map 'size' bytes at 'offset' into 'target' to load address 'vma' */
int mem_map_add( mem_map_t, unsigned int target, opdis_off_t offset,
		 opdis_off_t size, opdis_vma_t vma );

typedef int (*MEM_MAP_FOREACH_FN) ( map_t *, void * );

/* Invoke callback for each mapping */
void mem_map_foreach( mem_map_t, MEM_MAP_FOREACH_FN fn, void * arg );

/* print memory map to f */
void mem_map_print( mem_map_t, FILE * f );

/* return vma for offset 0 in target. if offset is nonzero, then the vma
 * for that offset into target is found, and the value of offset subtracted
 * from it to return a base vma for the target.
 */
opdis_vma_t mem_map_vma_for_target( mem_map_t, unsigned int target, 
				    opdis_off_t offset );

#endif

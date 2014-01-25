/* sym.c
 * tree of bfd symbols
 * Copyright (c) 2010 ThoughtGang
 * Written by TG Community Developers <community@thoughtgang.org>
 * Released under the GNU Public License, version 2.1.
 * See http://www.gnu.org/licenses/gpl.txt for details.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "sym.h"

/* ---------------------------------------------------------------------- */

typedef struct {
	const char * name;
	opdis_vma_t vma;
} sym_t;

static void * sym_key_vma( void * data ) {
	sym_t * sym = (sym_t *) data;
	return sym ? (void *) sym->vma : NULL;
}

static void * sym_key_name( void * data ) {
	sym_t * sym = (sym_t *) data;
	return sym ? (void *) sym->name : NULL;
}

static int name_cmp(void *a, void * b) {
	return strcmp( (const char *) a, (const char *) b );
}

sym_tab_t sym_tab_alloc( void ) {
	sym_tab_t s = (sym_tab_t) calloc( 1, sizeof(sym_table_t) );
	if (! s ) {
		return NULL;
	}

	s->by_vma = opdis_tree_init( sym_key_vma, NULL, free );
	s->by_name = opdis_tree_init( sym_key_name, name_cmp, NULL );

	if (! s->by_vma || ! s->by_name ) {
		sym_tab_free(s);
		return NULL;
	}

	return s;
}

void sym_tab_free( sym_tab_t s ) {
	if (! s ) {
		return;
	}

	if ( s->by_vma ) {
		opdis_tree_free(s->by_vma);
	}

	if ( s->by_name ) {
		opdis_tree_free(s->by_name);
	}

	free(s);
}

int sym_tab_add( sym_tab_t s, const char * name, opdis_vma_t vma ) {
	sym_t * sym;

	if (! s || ! name ) {
		return 0;
	}

	if ( opdis_tree_find( (opdis_tree_t) s->by_name, (void *) name ) ) {
//		fprintf( stderr, "Symbol for name %s already exists\n", name );
		return 0;
	}
	if ( opdis_tree_find( (opdis_tree_t) s->by_vma, (void *) vma ) ) {
//		fprintf( stderr, "Symbol for VMA %p already exists\n", 
//			 (void *) vma );
		return 0;
	}

	sym = (sym_t *) calloc( 1, sizeof(sym_t) );
	if (! sym ) {
		return 0;
	}
	sym->name = name;
	sym->vma = vma;

	if (! opdis_tree_add( (opdis_tree_t) s->by_vma, sym ) ) {
		free(sym);
		return 0;
	}

	if (! opdis_tree_add( (opdis_tree_t) s->by_name, sym ) ) {
		opdis_tree_delete( (opdis_tree_t) s->by_vma, (void *) vma );
		return 0;
	}

	return 1;
}

opdis_vma_t sym_tab_find_vma( sym_tab_t s, const char * name ) {
	sym_t * sym;
	if (! s || ! name ) {
		return OPDIS_INVALID_ADDR;
	}

	sym = opdis_tree_find( (opdis_tree_t) s->by_name, (void *) name ); 
	if ( sym ) {
		return sym->vma;
	}
	return OPDIS_INVALID_ADDR;
}

const char * sym_tab_find_name( sym_tab_t s, opdis_vma_t vma ) {
	sym_t * sym;
	
	if (! s ) {
		return NULL;
	}

	sym = opdis_tree_find( (opdis_tree_t) s->by_vma, (void *) vma );
	if ( sym ) {
		return sym->name;
	}

	return NULL;
}

static int print_symtab( void * item, void * arg ) {
	sym_t * sym = (sym_t *) item;
	FILE * f = (FILE *) arg;

	if (! sym || ! f ) {
		return 0;
	}

	fprintf( f, "\t%p: %s\n", (void *) sym->vma, sym->name );
	return 1;
}

void sym_tab_print( sym_tab_t s, FILE * f ) {
	if (!  s ) {
		return;
	}

	opdis_tree_foreach( (opdis_tree_t) s->by_vma, print_symtab, f );
}

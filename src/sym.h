/* sym.h
 * tree of symbols in a bfd target
 * Copyright (c) 2010 ThoughtGang
 * Written by TG Community Developers <community@thoughtgang.org>
 * Released under the GNU Public License, version 3.
 * See http://www.gnu.org/licenses/gpl.txt for details.
 */

#ifndef OPDIS_SYM_H
#define OPDIS_SYM_H

#include <opdis/tree.h>

typedef struct {
	opdis_tree_t by_name;
	opdis_tree_t by_vma;
} sym_table_t;

typedef sym_table_t * sym_tab_t;

/* ---------------------------------------------------------------------- */

sym_tab_t sym_tab_alloc( void );

void sym_tab_free( sym_tab_t );

int sym_tab_add( sym_tab_t, const char * name, opdis_vma_t vma );

opdis_vma_t sym_tab_find_vma( sym_tab_t, const char * name );

const char * sym_tab_find_name( sym_tab_t, opdis_vma_t vma );

void sym_tab_print( sym_tab_t, FILE * );

#endif

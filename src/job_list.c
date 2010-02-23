/* job_list.c
 * Copyright (c) 2010 ThoughtGang
 * Written by TG Community Developers <community@thoughtgang.org>
 * Released under the GNU Public License, version 3.
 * See http://www.gnu.org/licenses/gpl.txt for details.
 */

#include <stdio.h>
#include <stdlib.h>

#include "job_list.h"

/* allocate a job list */
job_list_t job_list_alloc( void ) {
	return (job_list_t) calloc( 1, sizeof(struct JOB_LIST_HEAD) );
}

/* free an allocated job list */
void job_list_free( job_list_t jobs ) {
	job_list_item_t * item, * next;

	if (! jobs ) {
		return;
	}

	for ( item = jobs->head; item; item = next ) {
		next = item->next;
		free( item );
	}

	free( jobs );
}

/* add a memspec job to a job list */
static job_list_item_t * add_item( job_list_t jobs, enum job_type_t type, 
			   const char * spec, unsigned int target ) {
	job_list_item_t * item, *prev;

	if (! jobs || ! spec ) {
		return 0;
	}

	item = (job_list_item_t *) calloc( 1, sizeof(job_list_item_t) );
	if (! item ) {
		return 0;
	}

	item->type = type;
	item->spec = spec;
	item->target = target;

	if (! jobs->head ) {
		jobs->head = item;
	} else {
		for ( prev = jobs->head; prev->next; prev = prev->next )
			;
		prev->next = item;
	}

	jobs->num_items++;

	return item;
}

unsigned int job_list_add( job_list_t jobs, enum job_type_t type, 
			   const char * spec, unsigned int target, 
			   opdis_off_t offset, opdis_vma_t vma, 
			   opdis_off_t size ) {
	job_list_item_t * item = add_item( jobs, type, spec, target );
	if (! item ) {
		return 0;
	}

	item->offset = offset;
	item->vma = vma;
	item->size = size;

	return jobs->num_items;
}


/* add a bfd job to a job list */
unsigned int job_list_add_bfd( job_list_t jobs, enum job_type_t type, 
			       const char * spec, unsigned int target, 
			       const char * bfd_name ) {
	job_list_item_t * item = add_item( jobs, type, spec, target );
	if (! item ) {
		return 0;
	}

	item->bfd_name = bfd_name;

	return jobs->num_items;
}

/* invoke a callback for every job in list */
void job_list_foreach( job_list_t jobs, JOB_LIST_FOREACH_FN fn, void * arg ) {
	job_list_item_t * item;
	unsigned int id = 1;

	if (! jobs || ! fn ) {
		return;
	}

	for ( item = jobs->head; item; item = item->next, id++ ) {
		fn( item, id, arg );
	}
}

static void set_buffer_vma( unsigned int target, opdis_buf_t buf, 
			    mem_map_t map ) {
	opdis_vma_t vma;
	vma = mem_map_vma_for_target( map, target, 0 );
	buf->vma = (vma == OPDIS_INVALID_ADDR) ? 0 : vma;
}

static opdis_vma_t get_job_vma( job_list_item_t * job, opdis_buf_t buf ) {
	opdis_vma_t vma;
	if ( job->vma == OPDIS_INVALID_ADDR ) {
		if ( job->offset >= buf->len ) {
			fprintf( stderr, "" );
			return OPDIS_INVALID_ADDR;
		}
		vma = buf->vma + job->offset;
	} else {
		vma = job->vma;
	}

	return vma;
}

static int check_bfd_job( job_opts_t o, tgt_list_item_t * tgt ) {
	if (! o->bfd_opdis ) {
		fprintf( stderr, "No opdis_t for BFD targets\n" );
		return 0;
	}

	if (! tgt->tgt_bfd ) {
		fprintf( stderr, "No bfd created for target\n" );
		return 0;
	}

	return 1;
}

static opdis_t opdis_for_bfd( bfd * abfd, opdis_t orig ) {
	opdis_t o = opdis_init_from_bfd( abfd );

	o->error_reporter = orig->error_reporter;
	o->error_reporter_arg = orig->error_reporter_arg;
	o->display = orig->display;
	o->display_arg = orig->display_arg;
	o->handler = orig->handler;
	o->handler_arg = orig->handler_arg;
	o->resolver = orig->resolver;
	o->resolver_arg = orig->resolver_arg;

	/* if user has overridden syntax, defer to it */
	if ( orig->config.arch == o->config.arch &&
	     orig->disassembler != o->disassembler ) {
		o->disassembler = orig->disassembler;
	}

	return o;
}

static int bfd_symbol_job( job_list_item_t * job, tgt_list_item_t * tgt, 
			   job_opts_t o ) {
	opdis_vma_t vma;
	opdis_t opdis;
	if (! check_bfd_job(o, tgt) ) {
		return 0;
	}

	opdis = opdis_for_bfd( tgt->tgt_bfd, o->opdis );

	vma = sym_tab_find_vma( tgt->symtab, job->bfd_name );
	if ( vma == OPDIS_INVALID_ADDR ) {
		fprintf( stderr, "Cannot find BFD symbol %s\n",
			 job->bfd_name );
		return 0;
	}

	if (! o->quiet ) {
		printf( "Control Flow disassembly of symbol %s\n", 
			 job->bfd_name );
	}
	return opdis_disasm_bfd_cflow( opdis, tgt->tgt_bfd, vma );
}

static int bfd_section_job( job_list_item_t * job, tgt_list_item_t * tgt, 
			    job_opts_t o ) {
	opdis_t opdis;
	struct bfd_section * section;
	if (! check_bfd_job(o, tgt) ) {
		return 0;
	}
	opdis = opdis_for_bfd( tgt->tgt_bfd, o->opdis );

	section = bfd_get_section_by_name( tgt->tgt_bfd, job->bfd_name );
	if (! section ) {
		fprintf( stderr, "Cannot find BFD section %s\n",
			 job->bfd_name );
		return 0;
	}

	if (! o->quiet ) {
		printf( "Linear disassembly of section %s\n", job->bfd_name );
	}
	return opdis_disasm_bfd_section( opdis, section );
}

static int bfd_entry_job( job_list_item_t * job, tgt_list_item_t * tgt, 
			    job_opts_t o ) {
	opdis_t opdis;
	if (! check_bfd_job(o, tgt) ) {
		return 0;
	}
	opdis = opdis_for_bfd( tgt->tgt_bfd, o->opdis );
	return opdis_disasm_bfd_entry( opdis, tgt->tgt_bfd );
}

static int linear_job( job_list_item_t * job, tgt_list_item_t * tgt, 
		       job_opts_t o ) {
	opdis_vma_t vma = get_job_vma( job, tgt->data );

	if (! tgt->data->vma || tgt->data->vma == OPDIS_INVALID_ADDR ) {
		set_buffer_vma( job->target, tgt->data, o->map );
	}

	if (! o->quiet ) {
		printf( "Linear disassembly of %p\n", (void *) vma );
	}
	return opdis_disasm_linear( o->opdis, tgt->data, vma, job->size );
}

static int cflow_job( job_list_item_t * job, tgt_list_item_t * tgt, 
		      job_opts_t o ) {
	opdis_vma_t vma = get_job_vma( job, tgt->data );

	if (! tgt->data->vma || tgt->data->vma == OPDIS_INVALID_ADDR ) {
		set_buffer_vma( job->target, tgt->data, o->map );
	}

	if (! o->quiet ) {
		printf( "Control Flow disassembly of %p\n", (void *) vma );
	}
	return opdis_disasm_cflow( o->opdis, tgt->data, vma );
}

static int perform_job( job_list_item_t * job, job_opts_t o ) {
	int rv;
	tgt_list_item_t * target;

	if (! job || ! o || ! o->targets || ! o->map ) {
		return 0;
	}

	target = tgt_list_find( o->targets, job->target );

	switch (job->type) {
		case job_cflow:
			rv = cflow_job( job, target, o );
			break;
		case job_linear:
			rv = linear_job( job, target, o );
			break;
		case job_bfd_entry:
			rv = bfd_entry_job( job, target, o );
			break;
		case job_bfd_symbol:
			rv = bfd_symbol_job( job, target, o );
			break;
		case job_bfd_section:
			rv = bfd_section_job( job, target, o );
			break;
		default:
			break;
	}
	return rv;
}

/* perform the specified job */
int job_list_perform( job_list_t jobs, unsigned int id, job_opts_t opts ) {
	job_list_item_t * item;
	unsigned int curr_id = 1;

	if (! jobs ) {
		return;
	}

	for ( item = jobs->head; item; item = item->next, curr_id++ ) {
		if ( curr_id == id ) {
			return perform_job( item, opts );
		}
	}

	return 0;
}

/* perform all jobs */
int job_list_perform_all( job_list_t jobs , job_opts_t opts ) {
	job_list_item_t * item;
	int rv = 1;

	if (! jobs ) {
		return;
	}

	for ( item = jobs->head; item; item = item->next ) {
		int result = perform_job( item, opts );
		if (! result ) {
		}
		rv &= result;
	}

	return rv;
}

static void print_details( FILE * f, job_list_item_t * item ) {
	if ( item->size ) {
		fprintf( f, "%d bytes at ", item->size );
	}

	if ( item->offset != OPDIS_INVALID_ADDR ) {
		fprintf( f, "offset %p ", (void *) item->offset );
	}

	if ( item->vma != OPDIS_INVALID_ADDR ) {
		fprintf( f, "VMA %p", (void *) item->vma );
	}

	if ( item->vma == OPDIS_INVALID_ADDR && 
	     item->offset == OPDIS_INVALID_ADDR ) {
		fprintf( f, "Invalid Address" );
	}
}

static void print_job( job_list_item_t * item, unsigned int id, void * arg ) {
	FILE * f = (FILE *) arg;
	if (! f ) {
		return;
	}

	fprintf( f, "\t%d\tTarget %d: ", id, item->target );

	switch ( item->type ) {
		case job_cflow:
			fprintf( f, "Control Flow disassembly of " );
			print_details( f, item );
			fprintf( f, "\n" );
			break;

		case job_linear:
			fprintf( f, "Linear disassembly of " );
			print_details( f, item );
			fprintf( f, "\n" );
			break;

		case job_bfd_entry:
			fprintf( f, 
				"Control Flow disassembly of BFD entry point\n"
				);
			break;

		case job_bfd_symbol:
			fprintf( f, 
				"Control Flow disassembly of BFD symbol '%s'\n",
				item->bfd_name );
			break;

		case job_bfd_section:
			fprintf( f, "Linear disassembly of BFD section '%s'\n", 
				item->bfd_name );
			break;
		default:
			fprintf( f, "Unknown job type for '%s'\n", item->spec );
	}
	
}

void job_list_print( job_list_t jobs, FILE * f ) {
	job_list_foreach( jobs, print_job, f );
}

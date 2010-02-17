/* job_list.c
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

static int perform_job( job_list_item_t * job, tgt_list_t targets,
			mem_map_t map, opdis_t o ) {
	if (! job || ! targets || ! map || ! o ) {
		return 0;
	}

	// TODO: switch on job type
	return 0;
}

/* perform the specified job */
int job_list_perform( job_list_t jobs, unsigned int id, tgt_list_t targets,
		      mem_map_t map, opdis_t o ) {
	job_list_item_t * item;
	unsigned int curr_id = 1;

	if (! jobs ) {
		return;
	}

	for ( item = jobs->head; item; item = item->next, curr_id++ ) {
		if ( curr_id == id ) {
			return perform_job( item, targets, map, o );
		}
	}

	return 0;
}

/* perform all jobs */
int job_list_perform_all( job_list_t jobs , tgt_list_t targets, mem_map_t map, 
			  opdis_t o, int quiet ) {
	job_list_item_t * item;
	int rv = 1;

	if (! jobs ) {
		return;
	}

	for ( item = jobs->head; item; item = item->next ) {
		int result = perform_job( item, targets, map, o );
		if (! result ) {
		}
		rv &= result;
	}

	return rv;
}

static void print_job( job_list_item_t * job, unsigned int id, void * arg ) {
	FILE * f = (FILE *) arg;
	if (! f ) {
		return;
	}
	
	// TODO
}

void print_job_list( job_list_t jobs, FILE * f ) {
	job_list_foreach( jobs, print_job, f );
}

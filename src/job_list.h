/* job_list.h
 * 
 */

#ifndef JOB_LIST_H
#define JOB_LIST_H

#include <opdis/opdis.h>

#include "map.h"
#include "target_list.h"

/* Type of job : arg is either a memspec or a BFD name */
enum job_type_t {
	job_cflow,		/* Control Flow disasm on memspec */
	job_linear,		/* Linear disasm on memspec */
	job_bfd_symbol,		/* Control Flow disasm of BFD symbol */
	job_bfd_section		/* Linear disasm of BFD section */
};

typedef struct JOB_LIST_ITEM {
	enum job_type_t type;	/* type of job */
	const char * spec;	/* string value for job */
	unsigned int target;	/* target of job */
	const char * bfd_name;	/* BFD argument (if applicable) */
	opdis_off_t offset;	/* Offset argument for job */
	opdis_vma_t vma;	/* VMA argument for job */
	unsigned int size;	/* Size argument for job */

	struct JOB_LIST_ITEM * next;
} job_list_item_t;

typedef struct JOB_LIST_HEAD {
	unsigned int	num_items;
	job_list_item_t * head;
} * job_list_t;

/* ---------------------------------------------------------------------- */

/* allocate a job list */
job_list_t job_list_alloc( void );

/* free an allocated job list */
void job_list_free( job_list_t );

/* add a memspec job to a job list */
unsigned int job_list_add( job_list_t, enum job_type_t type, const char * spec,
			   unsigned int target, opdis_off_t offset, 
			   opdis_vma_t vma, opdis_off_t size );

/* add a bfd job to a job list */
unsigned int job_list_add_bfd( job_list_t, enum job_type_t type, 
			       const char * spec, unsigned int target, 
			       const char * bfd_name );

typedef void (*JOB_LIST_FOREACH_FN) ( job_list_item_t *, unsigned int id, 
				      void * );

/* invoke a callback for every job in list */
void job_list_foreach( job_list_t, JOB_LIST_FOREACH_FN, void * arg );

/* perform the specified job */
int job_list_perform( job_list_t, unsigned int id, tgt_list_t,
		      mem_map_t, opdis_t );

/* perform all jobs */
int job_list_perform_all( job_list_t, tgt_list_t, mem_map_t, opdis_t,
			  int quiet );

void print_job_list( job_list_t, FILE * f );
#endif

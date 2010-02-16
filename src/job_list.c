/* job_list.c
 */

#include "job_list.h"

/* allocate a job list */
job_list_t job_list_alloc( void ) {
	return NULL;
}

/* free an allocated job list */
void job_list_free( job_list_t ) {
}

/* add a memspec job to a job list */
unsigned int job_list_add( job_list_t, enum job_type_t type, 
			   unsigned int target, opdis_off_t offset, 
			   opdis_vma_t vma, opdis_off_t size ) {
	return 0;
}

/* add a bfd job to a job list */
unsigned int job_list_add_bfd( job_list_t, enum job_type_t type, 
			       unsigned int target, const char * bfd_name ) {
	return 0;
}

/* invoke a callback for every job in list */
void job_list_foreach( job_list_t, JOB_LIST_FOREACH_FN, void * arg ) {
}

/* perform the specified job */
int job_list_perform( job_list_t, unsigned int item, tgt_list_t,
		      mem_map_t, opdis_t ) {
	return 0;
}

/* perform all jobs */
int job_list_perform_all( job_list_t, tgt_list_t, mem_map_t, opdis_t ) {
	return 0;
}

void print_job_list( job_list_t, FILE * f ) {
}

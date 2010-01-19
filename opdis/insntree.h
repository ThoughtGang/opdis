/*!
 * \file insntree.h
 * \brief Binary tree for storing disassembled instructions ordered by offset
 * \author thoughtgang.org
 */

#ifndef OPDIS_INSNTREE_H
#define OPDIS_INSNTREE_H

#ifdef WIN32
        #define LIBCALL _stdcall
#else
        #define LIBCALL
#endif

typedef struct opdis_insntree_node {
	struct opdis_insntree_node	* parent;
	struct opdis_insntree_node	* left;
	struct opdis_insntree_node	* right;
	void				* data;
	int	  			level;
} opdis_insntree_node_t;

typedef struct std_tree {
	opdis_insntree_node_t	* root;
	int	  		  num;
} opdis_insntree_base_t;

typedef opdis_insntree_base_t * opdis_insntree_t;


/* ---------------------------------------------------------------------- */
#ifdef __cplusplus
extern "C"
{
#endif


/*!
 * \fn
 * \brief
 * \param
 * \relates
 * \sa
 */

opdis_insntree_t LIBCALL opdis_insntree_init( void );

size_t opdis_insntree_count( opdis_insntree_t tree );

int LIBCALL opdis_insntree_add( opdis_insntree_t tree, 
				opdis_insn_t * insn );

opdis_insn_t *  LIBCALL opdis_insntree_find( opdis_insntree_t tree, 
				  bgo_vma_t addr );

int LIBCALL opdis_insntree_delete( opdis_insntree_t tree );

int LIBCALL opdis_insntree_walk( opdis_insntree_t tree,
				  OPDIS_INSNTREE_WALK fn, void * arg );

void LIBCALL opdis_insntree_free( opdis_insntree_t tree );

#ifdef __cplusplus
}
#endif

#endif


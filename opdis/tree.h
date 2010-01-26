/*!
 * \file tree.h
 * \brief AVL trees for storing opdis addresses and instructions.
 * \details This provides balanced binary (AVL) trees which store opdis 
 *          address or instruction objects order by address.
 * \author thoughtgang.org
 */

#ifndef OPDIS_INSNTREE_H
#define OPDIS_INSNTREE_H

#ifdef WIN32
        #define LIBCALL _stdcall
#else
        #define LIBCALL
#endif

/*! \struct opdis_tree_node_t 
 *  \ingroup tree
 *  \brief
 */
typedef struct opdis_tree_node {
	struct opdis_tree_node	* parent;
	struct opdis_tree_node	* left;
	struct opdis_tree_node	* right;
	void			* data;
	int	  		level;
} opdis_tree_node_t;

/*! \struct opdis_tree_node_t 
 *  \ingroup tree
 *  \brief
 */
typedef struct std_tree {
	opdis_tree_node_t	* root;
	int	  		  num;
} opdis_tree_base_t;

/*! \typedef opdis_tree_base_t * opdis_insn_tree_t
 *  \ingroup tree
 *  \brief
 */
typedef opdis_tree_base_t * opdis_insn_tree_t;

/*! \typedef opdis_tree_base_t * opdis_addr_tree_t
 *  \ingroup tree
 *  \brief
 */
typedef opdis_tree_base_t * opdis_addr_tree_t;


/* ---------------------------------------------------------------------- */
#ifdef __cplusplus
extern "C"
{
#endif


/* Address tree */

/*!
 * \fn opdis_addr_tree_t opdis_addr_tree_init()
 * \ingroup tree
 * \brief Allocate an Address Tree.
 * \return The allocated tree.
 * \sa opdis_addr_tree_free
 */

opdis_addr_tree_t LIBCALL opdis_addr_tree_init( void );

/*!
 * \fn int opdis_addr_tree_add( opdis_addr_tree_t, opdis_addr_t )
 * \ingroup tree
 * \brief Insert an address into the tree.
 * \param tree The Address Tree.
 * \param addr The address to insert.
 * \return 1 on success, 0 on failure.
 * \sa opdis_addr_tree_delete
 */

int LIBCALL opdis_addr_tree_add( opdis_addr_tree_t tree, opdis_addr_t addr );

/*!
 * \fn int opdis_addr_tree_delete( opdis_addr_tree_t, opdis_addr_t )
 * \ingroup tree
 * \brief Delete an address from the tree.
 * \param tree The Address Tree.
 * \param addr The address to delete.
 * \return 1 on success, 0 on failure.
 * \sa opdis_addr_tree_add 
 */

int LIBCALL opdis_addr_tree_delete( opdis_addr_tree_t tree, opdis_addr_t addr );

/*!
 * \fn opdis_addr_t opdis_addr_tree_find( opdis_addr_tree_t, opdis_addr_t )
 * \ingroup tree
 * \brief Find an address in the tree.
 * \param tree The Address Tree.
 * \param addr
 * \return The address or OPDIS_INVALID_ADDR.
 */

opdis_addr_t LIBCALL opdis_addr_tree_find( opdis_addr_tree_t tree, 
					   opdis_addr_t addr );

/*!
 * \fn void opdis_addr_tree_walk( opdis_addr_tree_t,
				  OPDIS_ADDR_TREE_WALK, void * )
 * \ingroup tree
 * \brief Invoke a callback for every item in the tree.
 * \param tree The Address Tree.
 * \param fn
 * \param arg
 */

int LIBCALL opdis_addr_tree_walk( opdis_addr_tree_t tree,
				  OPDIS_ADDR_TREE_WALK fn, void * arg );

/*!
 * \fn void opdis_addr_tree_free( opdis_addr_tree_t )
 * \ingroup tree
 * \brief Free the address tree.
 * \param tree The Address Tree.
 * \sa opdis_addr_tree_init
 * \note This does not free any of the data stored in the tree.
 */

void LIBCALL opdis_addr_tree_free( opdis_addr_tree_t tree );

/* ---------------------------------------------------------------------- */
/* Instruction tree */

/*!
 * \fn opdis_tree_t opdis_insn_tree_init()
 * \ingroup tree
 * \brief
 * \return
 * \sa
 */

opdis_insn_tree_t LIBCALL opdis_insn_tree_init( void );

/*!
 * \fn int opdis_insn_tree_add( opdis_insn_tree_t, opdis_insn_t * )
 * \ingroup tree
 * \brief
 * \param tree
 * \param insn
 * \return
 * \sa
 */

int LIBCALL opdis_insn_tree_add( opdis_insn_tree_t tree, 
				 opdis_insn_t * insn );

/*!
 * \fn int opdis_insn_tree_delete( opdis_insn_tree_t, opdis_vma_t )
 * \ingroup tree
 * \brief
 * \param tree
 * \param addr
 * \return
 * \sa
 */

int LIBCALL opdis_insn_tree_delete( opdis_insn_tree_t tree, opdis_vma_t addr );

/*!
 * \fn opdis_insn_t * opdis_insn_tree_find( opdis_insn_tree_t, opdis_vma_t )
 * \ingroup tree
 * \brief
 * \param tree
 * \param addr
 * \return
 * \sa
 */

opdis_insn_t *  LIBCALL opdis_insn_tree_find( opdis_insn_tree_t tree, 
				  	      opdis_vma_t addr );

/*!
 * \fn void opdis_insn_tree_walk( opdis_insn_tree_t,
				  OPDIS_INSN_TREE_WALK, void * )
 * \ingroup tree
 * \brief
 * \param tree
 * \param fn
 * \param arg
 * \sa
 */

void LIBCALL opdis_insn_tree_walk( opdis_insn_tree_t tree,
				  OPDIS_INSN_TREE_WALK fn, void * arg );

/*!
 * \fn void opdis_insn_tree_free( opdis_insn_tree_t )
 * \ingroup tree
 * \brief
 * \param tree
 * \sa
 */

void LIBCALL opdis_insn_tree_free( opdis_insn_tree_t tree );

/* ---------------------------------------------------------------------- */
/* Generic tree routines */

/*!
 * \fn opdis_tree_t opdis_tree_init( OPDIS_TREE_KEY_FN )
 * \ingroup tree
 * \brief
 * \param fn
 * \return
 * \sa
 */

opdis_tree_t LIBCALL opdis_tree_init( OPDIS_TREE_KEY_FN fn );

/*!
 * \fn int opdis_tree_add( opdis_tree_t, void * )
 * \ingroup tree
 * \brief
 * \param tree
 * \param data
 * \return
 * \sa
 */

int LIBCALL opdis_tree_add( opdis_tree_t tree, void * data );

/*!
 * \fn int opdis_tree_delete( opdis_tree_t, void * )
 * \ingroup tree
 * \brief
 * \param tree
 * \param key
 * \return
 * \sa
 */

int LIBCALL opdis_tree_delete( opdis_tree_t tree, void * key );

/*!
 * \fn void * opdis_tree_find( opdis_tree_t, void * )
 * \ingroup tree
 * \brief
 * \param tree
 * \param key
 * \return
 * \sa
 */

void * LIBCALL opdis_tree_find( opdis_tree_t tree, void * key );

/*!
 * \fn size_t opdis_tree_count( opdis_tree_t )
 * \ingroup tree
 * \brief
 * \param tree
 * \return
 * \sa
 */

size_t LIBCALL opdis_tree_count( opdis_tree_t tree );

/*!
 * \fn void opdis_tree_free( opdis_tree_t )
 * \ingroup tree
 * \brief
 * \param tree
 * \sa
 */

void LIBCALL opdis_tree_free( opdis_tree_t tree );

#ifdef __cplusplus
}
#endif

#endif


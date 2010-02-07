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
 *  \brief A node in an AVL tree.
 */
typedef struct opdis_tree_node {
	struct opdis_tree_node	* parent;	/*!< Parent node or NULL */
	struct opdis_tree_node	* left;		/*!< Left (<) child node */
	struct opdis_tree_node	* right;	/*!< Right (>) child node */
	void			* data;		/*!< Data stored in node */
	int	  		level;		/*!< Level of node in tree */
} opdis_tree_node_t;

/*!
 * \typedef void * (*OPDIS_TREE_KEY_FN) (void *)
 * \ingroup tree
 * \brief Callback to get the key for an item stored in a tree.
 * \param item The item in the tree.
 * \return Key for item
 * \details This function is invoked in order to retrieve the key for
 *          an item stored in the tree.
 */

typedef void * (*OPDIS_TREE_KEY_FN) (void * item);

/*!
 * \typedef int (*OPDIS_TREE_CMP_FN) (void *, void *)
 * \ingroup tree
 * \brief Callback to compare two keys.
 * \param a The first key.
 * \param b The second key.
 * \return -1, 0, 1 if a is <, ==, or > b
 * \details This function is called to determine if the first item is
 *          before, after, or equal in order to the second item.
 */

typedef int (*OPDIS_TREE_CMP_FN) (void * a, void *b);

/*!
 * \typedef void (*OPDIS_TREE_FREE_FN) (void *)
 * \ingroup tree
 * \brief Callback to free items stored in the tree.
 * \param item The item to free.
 */

typedef void (*OPDIS_TREE_FREE_FN) (void * item);

/*! \struct opdis_tree_base_t 
 *  \ingroup tree
 *  \brief The base of the tree.
 */
typedef struct std_tree {
	OPDIS_TREE_KEY_FN	key_fn;		/*!< Key retrieval callback */
	OPDIS_TREE_CMP_FN	cmp_fn;		/*!< Key compare callback */
	OPDIS_TREE_FREE_FN	free_fn;	/*!< Item free callback */
	opdis_tree_node_t	* root;		/*!< Root node of tree */
	int	  		  num;		/*!< Number of nodes in tree */
} opdis_tree_base_t;

/*! \typedef opdis_tree_base_t * opdis_tree_t
 *  \ingroup tree
 *  \brief A  generic AVL tree.
 */
typedef opdis_tree_base_t * opdis_tree_t;

/*! \typedef opdis_tree_base_t * opdis_addr_tree_t
 *  \ingroup tree
 *  \brief An AVL tree for storing opdis addresses.
 */
typedef opdis_tree_base_t * opdis_addr_tree_t;

/*! \typedef opdis_tree_base_t * opdis_insn_tree_t
 *  \ingroup tree
 *  \brief An AVL tree for storing opdis instructions.
 */
typedef opdis_tree_base_t * opdis_insn_tree_t;


/* ---------------------------------------------------------------------- */
#ifdef __cplusplus
extern "C"
{
#endif

/* Generic tree routines */

/*!
 * \fn opdis_tree_t opdis_tree_init( OPDIS_TREE_KEY_FN, OPDIS_TREE_CMP_FN,
 *                                   OPDIS_TREE_FREE_FN )
 * \ingroup tree
 * \brief Allocate and initialize for an AVL tree.
 * \param key_fn Callback to use for key retrieval.
 * \param cmp_fn Callback to use for key comparison.
 * \param free_fn Callback to use to free items or NULL.
 * \return The allocated binary tree.
 * \sa opdis_tree_free
 * \note If \e free_fn is NULL, then items stored in the tree will not be
 *       freed when deleted or when the tree is destroyed.
 */

opdis_tree_t LIBCALL opdis_tree_init( OPDIS_TREE_KEY_FN key_fn, 
				      OPDIS_TREE_CMP_FN cmp_fn,
				      OPDIS_TREE_FREE_FN free_fn );

/*!
 * \fn int opdis_tree_add( opdis_tree_t, void * )
 * \ingroup tree
 * \brief Insert a node into the tree.
 * \param tree The AVL tree.
 * \param data The data to insert.
 * \return 1 on success, 0 on failure.
 * \sa opdis_tree_update opdis_tree_delete
 * \note If the data already exists in the tree (i.e. the key compare
 *       callback returns 0), then the tree is unchanged and this routine
 *       returns zero. Use opdis_tree_update to overwrite an existing node.
 */

int LIBCALL opdis_tree_add( opdis_tree_t tree, void * data );

/*!
 * \fn int opdis_tree_update( opdis_tree_t, void * )
 * \ingroup tree
 * \brief Insert or overwrite a node in the tree.
 * \param tree The AVL tree.
 * \param data The data to insert.
 * \return 1 on success, 0 on failure.
 * \sa opdis_tree_add opdis_tree_delete
 * \details This routine will insert a node into the tree. If the data already
 *          exists in the tree (i.e. the key compare callback returns 0),
 *          then the data in the tree is freed and replaced. 
 */

int LIBCALL opdis_tree_update( opdis_tree_t tree, void * data );

/*!
 * \fn int opdis_tree_delete( opdis_tree_t, void * )
 * \ingroup tree
 * \brief Remove an item from the tree.
 * \param tree The AVL tree.
 * \param key The key of the item to remove.
 * \return 1 on success, 0 on failure.
 * \sa opdis_tree_add
 * \details This routine removes the item from the tree and invokes the
 *          OPDIS_TREE_FREE_FN to free it.
 */

int LIBCALL opdis_tree_delete( opdis_tree_t tree, void * key );

/*!
 * \fn void * opdis_tree_find( opdis_tree_t, void * )
 * \ingroup tree
 * \brief Find data in a tree.
 * \param tree The AVL tree.
 * \param key The key of the item to return.
 * \return The item stored in the tree or NULL.
 */

void * LIBCALL opdis_tree_find( opdis_tree_t tree, void * key );

/*!
 * \fn size_t opdis_tree_count( opdis_tree_t )
 * \ingroup tree
 * \brief Return the number of items in the tree.
 * \param tree The AVL tree.
 * \return The number of nodes in the tree.
 */

size_t LIBCALL opdis_tree_count( opdis_tree_t tree );

/*!
 * \fn void opdis_tree_free( opdis_tree_t )
 * \ingroup tree
 * \brief Free an AVL tree.
 * \param tree The AVL tree to free.
 * \sa opdis_tree_init
 * \note This routine invokes opdis_tree_delete on every node in the tree,
 *       then frees the tree itself. When this returns, \e tree points to
 *       an invalid object.
 */

void LIBCALL opdis_tree_free( opdis_tree_t tree );

/* ---------------------------------------------------------------------- */
/* Address tree */

/*!
 * \fn opdis_addr_tree_t opdis_addr_tree_init()
 * \ingroup tree
 * \brief Allocate an Address Tree.
 * \return The allocated tree.
 * \sa opdis_addr_tree_free
 * \details This creates a balanced binary tree of addresses. In this tree, the
 *          key is the same as the data: its primary use is to keep track of
 *          addresses which have been visited.
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
 * \typedef void (*OPDIS_ADDR_TREE_WALK_FN) (opdis_addr_t, void *)
 * \ingroup tree
 * \brief Callback invoked for every address emitted by opdis_addr_tree_walk.
 * \param addr The address.
 * \param arg Argument provided to opdis_addr_tree_walk
 */

typedef void (*OPDIS_ADDR_TREE_WALK_FN) (opdis_addr_t addr, void * arg);

/*!
 * \fn void opdis_addr_tree_walk( opdis_addr_tree_t,
				  OPDIS_ADDR_TREE_WALK_FN, void * )
 * \ingroup tree
 * \brief Invoke a callback for every item in the tree.
 * \param tree The Address Tree.
 * \param fn The callback to invoke for each address.
 * \param arg An optional argument to pass to the callback function.
 */

int LIBCALL opdis_addr_tree_walk( opdis_addr_tree_t tree,
				  OPDIS_ADDR_TREE_WALK_FN fn, void * arg );

/*!
 * \fn void opdis_addr_tree_free( opdis_addr_tree_t )
 * \ingroup tree
 * \brief Free the address tree.
 * \param tree The Address Tree.
 * \sa opdis_addr_tree_init
 */

void LIBCALL opdis_addr_tree_free( opdis_addr_tree_t tree );

/* ---------------------------------------------------------------------- */
/* Instruction tree */

/*!
 * \fn opdis_tree_t opdis_insn_tree_init()
 * \ingroup tree
 * \brief Allocate an Instruction Tree.
 * \param manage 1 if tree should free items on deletion; 0 otherwise.
 * \return The allocated tree.
 * \sa opdis_insn_tree_free
 * \details This creates a balanced binary tree of instructions keyed by
 *          VMA (NOT offset).
 */

opdis_insn_tree_t LIBCALL opdis_insn_tree_init( int manage );

/*!
 * \fn int opdis_insn_tree_add( opdis_insn_tree_t, opdis_insn_t * )
 * \ingroup tree
 * \brief Insert an instruction into the tree.
 * \param tree The Instruction Tree.
 * \param insn The instruction to insert.
 * \return 1 on success, 0 on failure.
 * \sa opdis_insn_tree_delete
 */

int LIBCALL opdis_insn_tree_add( opdis_insn_tree_t tree, 
				 opdis_insn_t * insn );

/*!
 * \fn int opdis_insn_tree_delete( opdis_insn_tree_t, opdis_vma_t )
 * \ingroup tree
 * \brief Delete an instruction from the tree.
 * \param tree The Instruction Tree.
 * \param insn The instruction to delete.
 * \return 1 on success, 0 on failure.
 * \sa opdis_insn_tree_add 
 * \note The instruction is freed if the Instruction Tree was created with
 *       \e manage set to 1.
 */

int LIBCALL opdis_insn_tree_delete( opdis_insn_tree_t tree, opdis_vma_t addr );

/*!
 * \fn opdis_insn_t * opdis_insn_tree_find( opdis_insn_tree_t, opdis_vma_t )
 * \ingroup tree
 * \brief Find an instruction in the tree.
 * \param tree The Instruction Tree.
 * \param addr The address of the instruction.
 * \return The instruction or NULL.
 */

opdis_insn_t *  LIBCALL opdis_insn_tree_find( opdis_insn_tree_t tree, 
				  	      opdis_vma_t addr );

/*!
 * \typedef void (*OPDIS_INSN_TREE_WALK_FN) (opdis_insn_t *, void *)
 * \ingroup tree
 * \brief Callback invoked for each instruction emitted by opdis_insn_tree_walk.
 * \param addr The instruction.
 * \param arg Argument provided to opdis_insn_tree_walk
 */

typedef void (*OPDIS_INSN_TREE_WALK_FN) (opdis_insn_t * insn, void * arg);

/*!
 * \fn void opdis_insn_tree_walk( opdis_insn_tree_t,
				  OPDIS_INSN_TREE_WALK_FN, void * )
 * \ingroup tree
 * \brief Invoke a callback for every item in the tree.
 * \param tree The Instruction Tree.
 * \param fn The callback to invoke for each instruction.
 * \param arg An optional argument to pass to the callback function.
 * \sa
 */

void LIBCALL opdis_insn_tree_walk( opdis_insn_tree_t tree,
				   OPDIS_INSN_TREE_WALK_FN fn, void * arg );

/*!
 * \fn void opdis_insn_tree_free( opdis_insn_tree_t )
 * \ingroup tree
 * \brief Free the instruction tree.
 * \param tree The instruction Tree.
 * \sa opdis_insn_tree_init
 * \note The instructions in the tree are freed if the Instruction Tree was 
 *       created with \e manage set to 1.
 */

void LIBCALL opdis_insn_tree_free( opdis_insn_tree_t tree );

#ifdef __cplusplus
}
#endif

#endif


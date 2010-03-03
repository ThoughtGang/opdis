/*!
 * \file tree.h
 * \brief AVL trees for storing opdis addresses and instructions.
 * \details This provides balanced binary (AVL) trees which store opdis 
 *          address or instruction objects order by address.
 * \author TG Community Developers <community@thoughtgang.org>
 * \note Copyright (c) 2010 ThoughtGang.
 * Released under the GNU Public License, version 3.
 * See http://www.gnu.org/licenses/gpl.txt for details.
 */

#ifndef OPDIS_INSNTREE_H
#define OPDIS_INSNTREE_H

#include <sys/types.h>

#include <opdis/model.h>
#include <opdis/types.h>

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

/*! \typedef opdis_tree_base_t * opdis_vma_tree_t
 *  \ingroup tree
 *  \brief An AVL tree for storing opdis addresses.
 */
typedef opdis_tree_base_t * opdis_vma_tree_t;

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
 * \return 1 on if the node was inserted, 0 on if node exists.
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
 * \fn int opdis_tree_contains( opdis_tree_t, void * )
 * \ingroup tree
 * \brief Determine if tree contains data
 * \param tree The AVL tree.
 * \param key The key of the item to search for.
 * \return 1 if the item is stored, 0 otherwise.
 * \sa opdis_tree_find
 * \details This function returns true (1) if \e tree contains a node with
 *          the given \e key. This is used instead of opdis_tree_find
 *          in trees where the node data (e.g. integers or pointers) could
 *          be zero, as the opdis_tree_find return value of NULL for not
 *          found will be zero regardless of whether the data is in the
 *          tree.
 */

int LIBCALL opdis_tree_contains( opdis_tree_t tree, void * key );

/*!
 * \fn void * opdis_tree_find( opdis_tree_t, void * )
 * \ingroup tree
 * \brief Find data in a tree.
 * \param tree The AVL tree.
 * \param key The key of the item to return.
 * \return The item stored in the tree or NULL.
 * \sa opdis_tree_contains
 */

void * LIBCALL opdis_tree_find( opdis_tree_t tree, void * key );

/*!
 * \fn void * opdis_tree_closest( opdis_tree_t, void * )
 * \ingroup tree
 * \brief Find closest match to data in a tree.
 * This returns the item that matches the key, or the item that is closest to
 * (but less than) the key, or NULL if there is no item less than or equal to
 * the key.
 * \param tree The AVL tree.
 * \param key The key of the item to match.
 * \return The item that is the closest match, or NULL.
 */
void * LIBCALL opdis_tree_closest( opdis_tree_t tree, void * key );

/*!
 * \fn void * opdis_tree_next( opdis_tree_t, void * )
 * \ingroup tree
 * \brief Find the data succeeding the key in a tree.
 * This returns the item that occurs immediately after \e key in the tree, or
 * immediately after where \e key would be if it were in the tree. If there
 * are no items greater than \e key in the tree, this returns NULL.
 * \param tree The AVL tree.
 * \param key The key of the item to match.
 * \return The item that is next, or NULL.
 */
void * LIBCALL opdis_tree_next( opdis_tree_t tree, void * key );

/*!
 * \typedef int (*OPDIS_TREE_FOREACH_FN) (void *, void *)
 * \ingroup tree
 * \brief Callback invoked by opdis_tree_foreach.
 * \param item The current item.
 * \param arg The argument provided to opdis_tree_foreach.
 * \note A zero return value will break out of the foreach.
 */

typedef int (*OPDIS_TREE_FOREACH_FN) (void * item, void * arg);

/*!
 * \fn void opdis_tree_foreach( opdis_tree_t, OPDIS_TREE_FOREACH_FN, void * )
 * \ingroup tree
 * \brief Iterate over the tree, invoking a callback for each item.
 * \param tree The AVL tree.
 * \param fn The callback function to invoke.
 * \param arg  An optional argument to pass to the callback.
 */

void LIBCALL opdis_tree_foreach( opdis_tree_t tree, OPDIS_TREE_FOREACH_FN fn,
				 void * arg );

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
 * \fn opdis_vma_tree_t opdis_vma_tree_init()
 * \ingroup tree
 * \brief Allocate an Address Tree.
 * \return The allocated tree.
 * \sa opdis_vma_tree_free
 * \details This creates a balanced binary tree of addresses. In this tree, the
 *          key is the same as the data: its primary use is to keep track of
 *          addresses which have been visited.
 */

opdis_vma_tree_t LIBCALL opdis_vma_tree_init( void );

/*!
 * \fn int opdis_vma_tree_add( opdis_vma_tree_t, opdis_vma_t )
 * \ingroup tree
 * \brief Insert an address into the tree.
 * \param tree The Address Tree.
 * \param addr The address to insert.
 * \return 1 if address was added, 0 if address exists.
 * \sa opdis_vma_tree_delete
 */

int LIBCALL opdis_vma_tree_add( opdis_vma_tree_t tree, opdis_vma_t addr );

/*!
 * \fn int opdis_vma_tree_delete( opdis_vma_tree_t, opdis_vma_t )
 * \ingroup tree
 * \brief Delete an address from the tree.
 * \param tree The Address Tree.
 * \param addr The address to delete.
 * \return 1 on success, 0 on failure.
 * \sa opdis_vma_tree_add 
 */

int LIBCALL opdis_vma_tree_delete( opdis_vma_tree_t tree, opdis_vma_t addr );

/*!
 * \fn int opdis_vma_tree_contains( opdis_vma_tree_t, opdis_vma_t )
 * \ingroup tree
 * \brief Determine if an address is in the tree.
 * \param tree The Address Tree.
 * \param addr The address to search for.
 * \return 1 if the address is in the tree, 0 otherwise.
 * \sa opdis_tree_contains
 */

int LIBCALL opdis_vma_tree_contains( opdis_vma_tree_t tree, opdis_vma_t addr );

/*!
 * \fn opdis_vma_t opdis_vma_tree_find( opdis_vma_tree_t, opdis_vma_t )
 * \ingroup tree
 * \brief Find an address in the tree.
 * \param tree The Address Tree.
 * \param addr The address to search for.
 * \return The address or OPDIS_INVALID_ADDR.
 * \sa opdis_tree_find
 */

opdis_vma_t LIBCALL opdis_vma_tree_find( opdis_vma_tree_t tree, 
					   opdis_vma_t addr );

/*!
 * \typedef int (*OPDIS_ADDR_TREE_FOREACH_FN) (opdis_vma_t, void *)
 * \ingroup tree
 * \brief Callback invoked for every address emitted by opdis_vma_tree_foreach.
 * \param addr The address.
 * \param arg Argument provided to opdis_vma_tree_foreach
 * \note A zero return value will break out of the foreach.
 */

typedef int (*OPDIS_ADDR_TREE_FOREACH_FN) (opdis_vma_t addr, void * arg);

/*!
 * \fn void opdis_vma_tree_foreach( opdis_vma_tree_t,
				  OPDIS_ADDR_TREE_FOREACH_FN, void * )
 * \ingroup tree
 * \brief Invoke a callback for every item in the tree.
 * \param tree The Address Tree.
 * \param fn The callback to invoke for each address.
 * \param arg An optional argument to pass to the callback function.
 */

void LIBCALL opdis_vma_tree_foreach( opdis_vma_tree_t tree,
				  OPDIS_ADDR_TREE_FOREACH_FN fn, void * arg );

/*!
 * \fn void opdis_vma_tree_free( opdis_vma_tree_t )
 * \ingroup tree
 * \brief Free the address tree.
 * \param tree The Address Tree.
 * \sa opdis_vma_tree_init
 */

void LIBCALL opdis_vma_tree_free( opdis_vma_tree_t tree );

/* ---------------------------------------------------------------------- */
/* Instruction tree */

/*!
 * \fn opdis_tree_t opdis_insn_tree_init( int )
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
 * \return 1 if instruction was added, 0 if instruction exists.
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
 * \fn int opdis_insn_tree_contains( opdis_insn_tree_t, opdis_vma_t );
 * \ingroup tree
 * \brief Determine if an instruction is in the tree.
 * \param tree The Instruction Tree.
 * \param addr The address to search for.
 * \return 1 if the address is in the tree, 0 otherwise.
 * \sa opdis_tree_contains
 */

int LIBCALL opdis_insn_tree_contains(opdis_insn_tree_t tree, opdis_vma_t addr);

/*!
 * \fn opdis_insn_t * opdis_insn_tree_find( opdis_insn_tree_t, opdis_vma_t )
 * \ingroup tree
 * \brief Find an instruction in the tree.
 * \param tree The Instruction Tree.
 * \param addr The address of the instruction.
 * \return The instruction or NULL.
 * \sa opdis_tree_find
 */

opdis_insn_t *  LIBCALL opdis_insn_tree_find( opdis_insn_tree_t tree, 
				  	      opdis_vma_t addr );

/*!
 * \typedef int (*OPDIS_INSN_TREE_FOREACH_FN) (opdis_insn_t *, void *)
 * \ingroup tree
 * \brief Callback invoked for each insn emitted by opdis_insn_tree_foreach.
 * \param addr The instruction.
 * \param arg Argument provided to opdis_insn_tree_foreach
 * \note A zero return value will break out of the foreach.
 */

typedef int (*OPDIS_INSN_TREE_FOREACH_FN) (opdis_insn_t * insn, void * arg);

/*!
 * \fn void opdis_insn_tree_foreach( opdis_insn_tree_t,
				  OPDIS_INSN_TREE_FOREACH_FN, void * )
 * \ingroup tree
 * \brief Invoke a callback for every item in the tree.
 * \param tree The Instruction Tree.
 * \param fn The callback to invoke for each instruction.
 * \param arg An optional argument to pass to the callback function.
 */

void LIBCALL opdis_insn_tree_foreach( opdis_insn_tree_t tree,
				   OPDIS_INSN_TREE_FOREACH_FN fn, void * arg );

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


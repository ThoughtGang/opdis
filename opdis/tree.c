/*!
 * \file tree.c
 * \brief Binary (AVL) tree implementation
 * \author TG Community Developers <community@thoughtgang.org>
 * \note Copyright (c) 2010 ThoughtGang.
 * Released under the GNU Public License, version 3.
 * See http://www.gnu.org/licenses/gpl.txt for details.
 */

#include <stdlib.h>

#include <opdis/tree.h>

/* ----------------------------------------------------------------------*/
/* Node alloc/free */

static opdis_tree_node_t * node_alloc( void *data ) {
	opdis_tree_node_t *node;

	if (! data ) {
		return NULL;
	}

	node = calloc( 1, sizeof(opdis_tree_node_t) );
	if (! node ) {
		return NULL;
	}

	node->data = data;

	return node;
}

static int tree_node_free( opdis_tree_t tree, opdis_tree_node_t * node ) {
	if (! tree || ! node ) {
		return 0;
	}

	if ( node->data ) {
		tree->free_fn( node->data );
	}

	free(node);

	return 1;
}

/* ----------------------------------------------------------------------*/
/* Node navigation */

static int replace_parent_ref( opdis_tree_node_t * old, 
			       opdis_tree_node_t * new ) {
	opdis_tree_node_t *parent;

	if (! old ) {
		return 0;
	}

	parent = old->parent;
	if (! parent) {
		return 1;
	}

	if (parent->left == old) {
		parent->left = new;
	} else {
		parent->right = new;
	}

	return 1;
}

static opdis_tree_node_t * node_first(opdis_tree_node_t * node) {
	if (! node) {
		return NULL;
	}
	for ( ; node->left; node = node->left )
		;

	return node;
}

static opdis_tree_node_t * node_last(opdis_tree_node_t * node) {
	if (! node) {
		return NULL;
	}
	for ( ; node; node = node->right )
		;

	return node;
}

static opdis_tree_node_t * node_next(opdis_tree_node_t * node) {
	if (! node) {
		return NULL;
	}

	if (node->right) {
		return node_first(node->right);
	}

	if (! node->parent) {
		return NULL;
	}

	for ( ; (node->parent) && (node == node->parent->right);
	      node = node->parent )
		;

	if (node->parent && node == node->parent->left) {
		return node->parent;
	}

	return NULL;
}

static opdis_tree_node_t * node_prev(opdis_tree_node_t * node) {
	opdis_tree_node_t *parent;

	if (! node ) {
		return NULL;
	}

	if (node->left) {
		return node_last(node->left);
	}

	for ( parent = node->parent; parent && node != parent->right;
	      node = parent, parent = parent->parent)
		;

	return parent;
}

static opdis_tree_node_t * subtree_max(opdis_tree_node_t * node) {
	opdis_tree_node_t *max;

	if (! node ) {
		return NULL;
	}

	for ( max = node; max->right; max = max->right )
		;

	return max;
}

/* ----------------------------------------------------------------------*/
/* Tree rotation routines */


static int max_level( opdis_tree_node_t * a, opdis_tree_node_t * b ) {
	if (! a && ! b ) {
		return 0;
	}

	if (! a) {
		return b->level + 1;
	}

	if (! b) {
		return a->level + 1;
	}

	return (a->level > b->level) ? a->level + 1 : b->level + 1;
}

/* rotate_left and rotate_right depend on each other */
static opdis_tree_node_t * rotate_right(opdis_tree_node_t *, int);

static opdis_tree_node_t * rotate_left( opdis_tree_node_t * node, int num ) {
	opdis_tree_node_t *root;


	if (! node) {
		return NULL;
	}

	if ( num > 1 ) {
		if (! node->left ) {
			return node;
		}
		node->left = rotate_right(node->left, num - 1);
	}

	if (! node->left ) {
		return node;
	}

	root = node->left;
	root->parent = node->parent;
	node->left = root->right;

	if ( node->left ) {
		node->left->parent = node;
	}

	root->right = node;
	node->parent = root;

	node->level = max_level(node->left, node->right);
	root->level = max_level(root->left, node);

	return root;
}

static opdis_tree_node_t * rotate_right(opdis_tree_node_t * node, int num) {
	opdis_tree_node_t *root;


	if (! node ) {
		return NULL;
	}

	if ( num > 1 ) {
		if (! node->right ) {
			return node;
		}
		node->right = rotate_left(node->right, num - 1);
	}

	if (! node->right ) {
		return node;
	}

	root = node->right;
	root->parent = node->parent;
	node->right = root->left;

	if ( node->right ) {
		node->right->parent = node;
	}

	root->left = node;
	node->parent = root;

	node->level = max_level(node->left, node->right);
	root->level = max_level(root->right, node);

	return root;
}

/* ----------------------------------------------------------------------*/
/* Tree management */

static opdis_tree_node_t * insert_node( opdis_tree_t tree, 
					opdis_tree_node_t * start, 
					void * data ) {
	int lr;
	void * key;

	if (! start ) {
		return node_alloc( data );
	}

	key = tree->key_fn(data);
	lr = tree->cmp_fn( key, tree->key_fn(start->data) );

	if ( lr < 0 ) {
		start->left = insert_node(tree, start->left, data);
		start->left->parent = start;

		if ( start->right &&
		     start->left->level - start->right->level > 1 ) {
			/* is data < right child? */
			if ( tree->cmp_fn(key, tree->key_fn(start->left->data))
			     < 0) {
				start = rotate_left(start, 1);
			} else {
				start = rotate_left(start, 2);
			}
		} else if (! start->right && start->left->level > 1 ) {
			start = rotate_right(start, 1);
		}
	} else if ( lr > 0 ) {
		start->right = insert_node(tree, start->right, data);
		start->right->parent = start;

		if ( start->left &&
		     start->right->level - start->left->level > 1 ) {
			/* is data > left child? */
			if ( tree->cmp_fn(key, tree->key_fn(start->right->data))
			     > 0) {
				start = rotate_right(start, 1);
			} else {
				start = rotate_right(start, 2);
			}
		} else if (! start->left && start->right->level > 1 ) {
			start = rotate_left(start, 1);
		}
	} else {
		return start;
	}

	start->level = max_level(start->left, start->right);
	return start;
}

static int remove_node(opdis_tree_node_t * node) {
	opdis_tree_node_t *new;

	if (! node->left ) {
		if (! node->right ) {
			new = node;
		} else {
			new = node->right;
		}
	} else if (! node->right ) {
		new = node->left;
	} else {
		new = subtree_max(node->left);
		if ( new->left ) {
			new->left->parent = new->parent;
			new->left->level = new->level;
			new->parent = NULL;
		}
		new->left = node->left;
		new->right = node->right;
	}

	if (! replace_parent_ref(node, new) ) {
		return 0;
	}

	if ( new ) {
		new->level = node->level;
		replace_parent_ref(new, NULL);
	}

	if (new->parent) {
		int diff;

		if (new->parent->left == new) {
			diff = new->level - new->parent->right->level;
			if (diff < -1)
				rotate_left(new->parent, 2);
			else if (diff > 1)
				rotate_left(new->parent, 1);
		} else {
			diff = new->level - new->parent->left->level;
			if (diff < -1)
				rotate_right(new->parent, 2);
			else if (diff > 1)
				rotate_right(new->parent, 1);
		}
	}

	return 1;
}


static int tree_node_destroy( opdis_tree_t tree, opdis_tree_node_t * node) {
	if (! node ) {
		return 1;
	}

	tree_node_destroy( tree, node->left );
	tree_node_destroy( tree, node->right );

	tree_node_free( tree, node );

	return 1;
}

static opdis_tree_node_t *  tree_node_find( opdis_tree_t tree, void * key ) {
	opdis_tree_node_t * node, * next;

	for (node = tree->root; node; node = next) {
		int lr;

		lr = tree->cmp_fn( key, tree->key_fn(node->data) );

		if ( lr < 0 ) {
			next = node->left;
		} else if ( lr > 0 ) {
			next = node->right;
		} else {
			return node;
		}
	}

	return NULL;
}


/* ----------------------------------------------------------------------*/
/* BUILTIN TREE CALLBACKS */

/* uses item as key */
static void * builtin_key_fn (void * item) {
	return item;
}

/* compares items directly */
static int builtin_cmp_fn (void * a, void *b) {
	if ( a < b ) return -1;
	if ( a > b ) return 1;
	return 0;
}

/* no-op : item is not freed */
static void builtin_free_fn (void * arg) {
	return;
}

/* ----------------------------------------------------------------------*/
/* GENERIC AVL TREE */

opdis_tree_t LIBCALL opdis_tree_init( OPDIS_TREE_KEY_FN key_fn, 
				      OPDIS_TREE_CMP_FN cmp_fn,
				      OPDIS_TREE_FREE_FN free_fn ) {
	opdis_tree_t t = (opdis_tree_t) calloc( 1, sizeof(opdis_tree_base_t) );
	if (! t ) {
		return NULL;
	}

	t->key_fn = key_fn == NULL ? builtin_key_fn : key_fn;
	t->cmp_fn = cmp_fn == NULL ? builtin_cmp_fn : cmp_fn;
	t->free_fn = free_fn == NULL ? builtin_free_fn : free_fn;

	return t;
}

int LIBCALL opdis_tree_add( opdis_tree_t tree, void * data ) {
	if (! tree || ! data ) {
		return 0;
	}

	tree->root = insert_node(tree, tree->root, data);
	tree->root->parent = NULL;

	tree->num++;

	return 1;
}

int LIBCALL opdis_tree_update( opdis_tree_t tree, void * data ) {
	opdis_tree_node_t * node = tree_node_find( tree, tree->key_fn(data) );
	if ( node ) {
		tree->free_fn(node->data);
		node->data = data;
		return 1;
	}

	return opdis_tree_add( tree, data );
}

int LIBCALL opdis_tree_delete( opdis_tree_t tree, void * key ) {
	opdis_tree_node_t *node;

	node = tree_node_find(tree, key);
	if (! node) {
		return 0;
	}

	if (! remove_node(node)) {
		return 0;
	}

	tree_node_free(tree, node);

	tree->num--;

	return 1;
}

int LIBCALL opdis_tree_contains( opdis_tree_t tree, void * key ) {
	return (tree_node_find( tree, key ) == NULL) ? 0 : 1;
}

void * LIBCALL opdis_tree_find( opdis_tree_t tree, void * key ) {

	opdis_tree_node_t * node = tree_node_find( tree, key );
	if ( node ) {
		return node->data;
	}

	return NULL;
}

void * opdis_tree_closest( opdis_tree_t tree, void * key ) {
	opdis_tree_node_t *node, *next, *closest = NULL;

	for ( node = tree->root; node; node = next ) {
		int lr;

		lr = tree->cmp_fn( key, tree->key_fn(node->data) );
		if (lr < 0) {
			next = node->left;
		} else if (lr > 0) {
			closest = node;
			next = node->right;
		} else {
			return node->data;
		}
	}

	if ( closest ) {
		return closest->data;
	}

	return NULL;
}

void * opdis_tree_next( opdis_tree_t tree, void * key ) {
	opdis_tree_node_t *node, *next, *closest = NULL;

	for ( node = tree->root; node; node = next ) {
		int lr;

		lr = tree->cmp_fn( key, tree->key_fn(node->data) );
		if (lr < 0) {
			closest = node;
			next = node->left;
		} else if (lr > 0) {
			next = node->right;
		} else {
			next = node->right;
		}
	}

	if ( closest ) {
		return closest->data;
	}

	return NULL;
}

void LIBCALL opdis_tree_foreach( opdis_tree_t tree, OPDIS_TREE_FOREACH_FN fn,
				 void * arg ) {
	opdis_tree_node_t *node;

	if (! fn ) {
		return;
	}

	node = node_first(tree->root);
	for ( ; node; node = node_next(node) ) {
		if (! fn( node->data, arg ) ) {
			break;
		}
	}
}


size_t LIBCALL opdis_tree_count( opdis_tree_t tree ) {
	if (! tree ) {
		return 0;
	}

	return tree->num;
}

void LIBCALL opdis_tree_free( opdis_tree_t tree ) {
	if (! tree ) {
		return;
	}

	tree_node_destroy(tree, tree->root);

	free(tree);
}

/* ----------------------------------------------------------------------*/
/* ADDRESS TREE */

opdis_vma_tree_t LIBCALL opdis_vma_tree_init( void ) {
	return (opdis_vma_tree_t) opdis_tree_init( NULL, NULL, NULL );
}

int LIBCALL opdis_vma_tree_add( opdis_vma_tree_t tree, opdis_vma_t addr ) {
	return opdis_tree_add( (opdis_tree_t) tree, (void *) addr );
}

int LIBCALL opdis_vma_tree_delete( opdis_vma_tree_t tree, opdis_vma_t addr ){
	return opdis_tree_delete( (opdis_tree_t) tree, (void *) addr );
}

int LIBCALL opdis_vma_tree_contains( opdis_vma_tree_t tree, 
					   opdis_vma_t addr ) {
	return (opdis_tree_contains((opdis_tree_t) tree, (void *) addr)) ? 
						1 : 0;
}

opdis_vma_t LIBCALL opdis_vma_tree_find( opdis_vma_tree_t tree, 
					   opdis_vma_t addr ) {
	opdis_tree_node_t * node = tree_node_find( (opdis_tree_t) tree, 
						   (void *) addr );
	if (! node ) {
		return OPDIS_INVALID_ADDR;
	}

	return (opdis_vma_t) node->data;
}

void LIBCALL opdis_vma_tree_foreach( opdis_vma_tree_t tree,
				   OPDIS_ADDR_TREE_FOREACH_FN fn, void * arg ) {
	opdis_tree_node_t *node;

	if (! fn ) {
		return;
	}

	node = node_first(tree->root);
	for ( ; node; node = node_next(node) ) {
		if (! fn( (opdis_vma_t) node->data, arg ) ) {
			break;
		}
	}
}

void LIBCALL opdis_vma_tree_free( opdis_vma_tree_t tree ) {
	opdis_tree_free( (opdis_tree_t) tree );
}

/* ----------------------------------------------------------------------*/
/* INSTRUCTION TREE */

/* returns item load addresses */
static void * insn_key_fn (void * item) {
	if ( item ) {
		opdis_insn_t * i = (opdis_insn_t *) item;
		return (void *) i->vma;
	}

	return NULL;
}

/* frees instruction */
static void insn_free_fn (void * item) {
	if ( item ) {
		opdis_insn_t * i = (opdis_insn_t *) item;
		opdis_insn_free( i );
	}
}

opdis_insn_tree_t LIBCALL opdis_insn_tree_init( int manage ) {
	OPDIS_TREE_FREE_FN free_fn = manage ? insn_free_fn : NULL;
	
	return (opdis_insn_tree_t) opdis_tree_init(insn_key_fn, NULL, free_fn);
}

int LIBCALL opdis_insn_tree_add( opdis_insn_tree_t tree, 
				 opdis_insn_t * insn ) {
	return opdis_tree_add( (opdis_tree_t) tree, insn );
}

int LIBCALL opdis_insn_tree_delete( opdis_insn_tree_t tree, opdis_vma_t addr ){
	return opdis_tree_delete( (opdis_tree_t) tree, (void *) addr );
}

int LIBCALL opdis_insn_tree_contains( opdis_insn_tree_t tree, 
				  	      opdis_vma_t addr ) {
	return (opdis_tree_contains((opdis_tree_t) tree, (void *) addr)) ? 
							1 : 0;
}

opdis_insn_t * LIBCALL opdis_insn_tree_find( opdis_insn_tree_t tree, 
				  	      opdis_vma_t addr ) {
	return (opdis_insn_t *) opdis_tree_find( (opdis_tree_t)tree, 
						 (void *) addr );
}

void LIBCALL opdis_insn_tree_foreach( opdis_insn_tree_t tree,
				   OPDIS_INSN_TREE_FOREACH_FN fn, void * arg ) {
	opdis_tree_node_t *node;

	if (! fn ) {
		return;
	}

	node = node_first(tree->root);
	for ( ; node; node = node_next(node) ) {
		if (! fn( (opdis_insn_t *) node->data, arg ) ) {
			break;
		}
	}
}

void LIBCALL opdis_insn_tree_free( opdis_insn_tree_t tree ) {
	return opdis_tree_free( (opdis_tree_t) tree );
}

/*!
 * \file insntree.c
 * \brief Binary tree for storing disassembled instructions ordered by offset
 * \author thoughtgang.org
 */

#include <opdis/insntree.h>

/* ----------------------------------------------------------------------*/
/* Node comparison routines */

static int node_val_cmp( opdis_insntree_node_t *n, opdis_vma_t val ) {
}

static int node_cmp( opdisinsntree_node_t * a, opdisinsntree_node_t * b ) {
}

/* ----------------------------------------------------------------------*/
/* Node utility routines */

static opdis_insntree_node_t * treenode_alloc(void *data) {
	opdis_insntree_node_t *node;

	if (NULL == data)
		return std_seterrp(STD_BAD_ARG);

	node = calloc(1, sizeof(*node));
	if (NULL == node)
		return std_seterrp(STD_NO_MEM);

	node->data = data;

	return node;
}

static int treenode_free(opdis_insntree_node_t * node) {
	if (NULL == node)
		return std_seterr(STD_NO_MEM);

	memset(node, 0, sizeof(*node));
	free(node);

	return (1);
}

static int replace_parent_ref(opdis_insntree_node_t * old, opdis_insntree_node_t * new) {
	opdis_insntree_node_t *parent;

	if (NULL == old)
		return std_seterr(STD_BAD_ARG);

	parent = old->parent;
	if (NULL == parent)
		return (1);

	if (parent->left == old)
		parent->left = new;
	else
		parent->right = new;

	return (1);
}

/* ----------------------------------------------------------------------*/
/* Node navigation routines */

static opdis_insntree_node_t * treenode_first(opdis_insntree_node_t * node) {
	if (NULL == node)
		return NULL;
	for (; node->left; node = node->left);
	return node;
}

static opdis_insntree_node_t * treenode_last(opdis_insntree_node_t * node) {
	if (NULL == node)
		return NULL;
	for (; node; node = node->right);
	return node;
}

static opdis_insntree_node_t * tree_closest(opdis_insntree_t tree, void *data) {
	opdis_insntree_node_t *node, *next, *closest = NULL, *first = NULL;


	if (!std_test_magic(tree, STD_TREE_MAGIC))
		return std_seterrp(STD_BAD_JUJU);

	for (node = tree->root; node; node = next) {
		int lr;

		lr = insn_cmp(data, node->data);
		if (lr < 0) {
			first = node;
			next = node->left;
		} else if (lr > 0) {
			closest = node;
			next = node->right;
		} else
			return node;
	}

	if (!closest)
		closest = first;

	return (closest);
}

static opdis_insntree_node_t * tree_next(opdis_insntree_node_t * node) {
	if (NULL == node)
		return std_seterrp(STD_BAD_ARG);

	if (node->right)
		return std_treenode_first(node->right);

	if (NULL == node->parent)
		return std_seterrp(STD_NO_ENT);

	for (; (node->parent) && (node == node->parent->right);
	     node = node->parent);

	if (node->parent && node == node->parent->left)
		return node->parent;

	return std_seterrp(STD_NO_ENT);
}

static opdis_insntree_node_t * tree_prev(opdis_insntree_node_t * node) {
	opdis_insntree_node_t *parent;

	if (NULL == node)
		return std_seterrp(STD_BAD_ARG);

	if (node->left)
		return std_treenode_last(node->left);

	for (parent = node->parent; parent && node != parent->right;
	     node = parent, parent = parent->parent);

	return (parent);
}

static opdis_insntree_node_t * subtree_max(opdis_insntree_node_t * node) {
	opdis_insntree_node_t *max;

	if (NULL == node)
		return NULL;

	for (max = node; max->right; max = max->right);
	return max;
}
/* ----------------------------------------------------------------------*/
/* Tree rotation routines */


static int max_level(opdis_insntree_node_t * a, opdis_insntree_node_t * b) {
	if ((NULL == a) && (NULL == b))
		return (0);

	if (NULL == a)
		return b->level + 1;

	if (NULL == b)
		return a->level + 1;

	return ((a->level > b->level) ? a->level + 1 : b->level + 1);
}

static opdis_insntree_node_t * rotate_left(opdis_insntree_node_t * node, int num) {
	opdis_insntree_node_t *root;


	if (NULL == node)
		return std_seterrp(STD_BAD_ARG);

	if (num > 1) {
		if (!node->left)
			return node;
		node->left = rotate_right(node->left, num - 1);
	}

	if (!node->left)
		return node;

	root = node->left;
	root->parent = node->parent;
	node->left = root->right;

	if (node->left)
		node->left->parent = node;

	root->right = node;
	node->parent = root;

	node->level = max_level(node->left, node->right);
	root->level = max_level(root->left, node);

	return root;
}

static opdis_insntree_node_t * rotate_right(opdis_insntree_node_t * node, int num) {
	opdis_insntree_node_t *root;


	if (NULL == node)
		return std_seterrp(STD_BAD_ARG);

	if (num > 1) {
		if (!node->right)
			return node;
		node->right = rotate_left(node->right, num - 1);
	}

	if (!node->right)
		return node;

	root = node->right;
	root->parent = node->parent;
	node->right = root->left;

	if (node->right)
		node->right->parent = node;

	root->left = node;
	node->parent = root;

	node->level = max_level(node->left, node->right);
	root->level = max_level(root->right, node);

	return root;
}



/* ----------------------------------------------------------------------*/
opdis_insntree_t LIBCALL opdis_insntree_init( void ) { 
	return (opdis_insntree_t) calloc(1, sizeof(opdis_insntree_base_t));
}

size_t LIBCALL opdis_insntree_count( opdis_insntree_t tree ) {
	if (! tree ) {
		return 0;
	}

	return tree->num;
}

/* ----------------------------------------------------------------------*/
static opdis_insntree_node_t * insert_node( opdis_insntree_t tree, 
					    opdis_insntree_node_t * start, 
					    void *data) {
	int lr;

	if (NULL == start) {
		return std_treenode_alloc(data);
	}

	lr = insn_cmp(data, start->data);

	if (lr < 0) {
		start->left = insert_node(tree, start->left, data);
		start->left->parent = start;

		if (start->right &&
		    start->left->level - start->right->level > 1) {
			/* is data < right child? */
			if (insn_cmp(data, start->left->data) < 0)
				start = rotate_left(start, 1);
			else
				start = rotate_left(start, 2);
		} else if (!start->right && start->left->level > 1) {
			start = rotate_right(start, 1);
		}
	} else if (lr > 0) {
		start->right = insert_node(tree, start->right, data);
		start->right->parent = start;

		if (start->left &&
		    start->right->level - start->left->level > 1) {
			/* is data > left child? */
			if (insn_cmp(data, start->right->data) > 0)
				start = rotate_right(start, 1);
			else
				start = rotate_right(start, 2);
		} else if (!start->left && start->right->level > 1) {
			start = rotate_left(start, 1);
		}
	} else
		return start;

	start->level = max_level(start->left, start->right);
	return (start);
}


int LIBCALL opdis_insntree_add( opdis_insntree_t tree, opdis_insn_t * insn ) {
	if (! tree || ! insn ) {
		return 0;
	}

	tree->root = insert_node(tree, tree->root, data);
	tree->root->parent = NULL;

	tree->num++;

	return 1;
}

int LIBCALL opdis_insntree_delete( opdis_insntree_t tree ) {
	opdis_insntree_node_t *node;


	node = std_treenode_find(tree, data);
	if (NULL == node)
		return (0);

	if (!remove_node(node))
		return (0);

	std_treenode_free(node);

	tree->num--;

	return (1);

}

int LIBCALL opdis_insntree_walk( opdis_insntree_t tree,
				 OPDIS_INSNTREE_WALK fn, void * arg ) {
	opdis_insntree_node_t *node;


	if (func == NULL) {
		return 0;
	}

	node = treenode_first(tree->root);
	for (; node; node = tree_next(node)) {
		func(arg, node->data, arg);
	}

	return 1;
}

/* ----------------------------------------------------------------------*/

static int remove_node(opdis_insntree_node_t * node) {
	opdis_insntree_node_t *new;

	if (!node->left) {
		if (!node->right)
			new = node;
		else
			new = node->right;
	} else if (!node->right) {
		new = node->left;
	} else {
		new = subtree_max(node->left);
		if (new->left) {
			new->left->parent = new->parent;
			new->left->level = new->level;
			new->parent = NULL;
		}
		new->left = node->left;
		new->right = node->right;
	}

	if (!replace_parent_ref(node, new))
		return 0;

	if (new) {
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


opdis_insn_t * LIBCALL opdis_insntree_find( opdis_insntree_t tree, 
				  	    opdis_vma_t addr ) {
	for (node = tree->root; node; node = next) {
		int lr;

		lr = insn_cmp(data, node->data);

		if (lr < 0)
			next = node->left;
		else if (lr > 0)
			next = node->right;
		else
			return node->insn;
	}

	return NULL;
}

/* ----------------------------------------------------------------------*/

static int node_destroy(opdis_insntree_node_t * node) {
	if (! node ) {
		return 1;
	}

	node_destroy(node->left);
	node_destroy(node->right);

	node_free(node);
	return 1;
}

void LIBCALL opdis_insntree_free( opdis_insntree_t tree ) {
	if (! tree ) {
		return;
	}

	node_destroy(tree->root);
	free(tree);
}


opdis_tree_t LIBCALL opdis_tree_init( OPDIS_TREE_KEY_FN key_fn, 
				      OPDIS_TREE_CMP_FN cmp_fn,
				      OPDIS_TREE_FREE_FN free_fn );

int LIBCALL opdis_tree_add( opdis_tree_t tree, void * data );

int LIBCALL opdis_tree_update( opdis_tree_t tree, void * data );

int LIBCALL opdis_tree_delete( opdis_tree_t tree, void * key );

void * LIBCALL opdis_tree_find( opdis_tree_t tree, void * key );

size_t LIBCALL opdis_tree_count( opdis_tree_t tree );

void LIBCALL opdis_tree_free( opdis_tree_t tree );

opdis_addr_tree_t LIBCALL opdis_addr_tree_init( void );

int LIBCALL opdis_addr_tree_add( opdis_addr_tree_t tree, opdis_addr_t addr );

int LIBCALL opdis_addr_tree_delete( opdis_addr_tree_t tree, opdis_addr_t addr );

opdis_addr_t LIBCALL opdis_addr_tree_find( opdis_addr_tree_t tree, 
					   opdis_addr_t addr );

typedef void (*OPDIS_ADDR_TREE_WALK_FN) (opdis_addr_t addr, void * arg);

int LIBCALL opdis_addr_tree_walk( opdis_addr_tree_t tree,
				  OPDIS_ADDR_TREE_WALK_FN fn, void * arg );

void LIBCALL opdis_addr_tree_free( opdis_addr_tree_t tree );

opdis_insn_tree_t LIBCALL opdis_insn_tree_init( int manage );

int LIBCALL opdis_insn_tree_add( opdis_insn_tree_t tree, 
				 opdis_insn_t * insn );

int LIBCALL opdis_insn_tree_delete( opdis_insn_tree_t tree, opdis_vma_t addr );

opdis_insn_t *  LIBCALL opdis_insn_tree_find( opdis_insn_tree_t tree, 
				  	      opdis_vma_t addr );

typedef void (*OPDIS_INSN_TREE_WALK_FN) (opdis_insn_t * insn, void * arg);

void LIBCALL opdis_insn_tree_walk( opdis_insn_tree_t tree,
				   OPDIS_INSN_TREE_WALK_FN fn, void * arg );
void LIBCALL opdis_insn_tree_free( opdis_insn_tree_t tree );

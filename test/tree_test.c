
#include <stdio.h>
#include <big_std.h>

struct TN {
	struct TN *p, *l, *r;
	char *data;
} strtree[] = {
	{ NULL, &strtree[1], &strtree[8], "m"},			/* 0	root */
	{ &strtree[0], &strtree[2], &strtree[7], "d"},		/* 1	L    */
	{ &strtree[1], &strtree[3], &strtree[4], "b"},		/* 2	LL   */
	{ &strtree[2], NULL, NULL, "a"},			/* 3	LLL  */
	{ &strtree[2], NULL, NULL, "c"},			/* 4	LLR  */
	{ &strtree[7], NULL, NULL, "e"},			/* 5	LRL  */
	{ &strtree[7], NULL, NULL, "g"},			/* 6	LRR  */
	{ &strtree[1], &strtree[5], &strtree[6], "f"},		/* 7	LR   */
	{ &strtree[0], &strtree[9], &strtree[14], "w"},		/* 8	R    */
	{ &strtree[8], &strtree[10], &strtree[11], "u"},	/* 9	RL   */
	{ &strtree[9], NULL, NULL, "t"},			/* 10	RLL  */
	{ &strtree[9], NULL, NULL, "v"},			/* 11	RLR  */
	{ &strtree[14], NULL, NULL, "x"},			/* 12	RRL  */
	{ &strtree[14], NULL, NULL, "z"},			/* 13	RRR  */
	{ &strtree[8], &strtree[12], &strtree[13], "y"}		/* 14	RR   */
};

static void print_tree( struct TN *tn ) {
	if ( tn->l ) {
		print_tree( tn->l );
	}
	printf( "%s ", tn->data );

	if ( tn->r ) {
		print_tree( tn->r );
	}
}

static int printtree( void * arg, void *data ) {
	char *str = (char *) data;
	printf( "%s ", str );
	return 1;
}

static int sumtree( void * arg, void *data ) {
	int *sum = (int *) arg;
	int num = (int) data;
	*sum += num;
	return 1;
}

int main (void) {
	int i, sum, treesum;
	std_tree_t *t;
	struct TN *strtn;

	/* ============================================== */
	/* test the unsigned int comparison */
	t = std_tree_alloc( std_cmp_uint );
	sum = treesum = 0;
	for ( i = 0; i < 1024; i++ ) {
		std_tree_insert( t, (void *) i );
		sum += i;
	}

	std_tree_foreach( t, sumtree, &treesum );

	printf( "(unsigned) SUM: %d TreeSUM: %d\n", sum, treesum );
	std_tree_free( t );
		

	/* test the signed int comparison */
	t = std_tree_alloc( std_cmp_int );
	sum = treesum = 0;
	for ( i = -512; i < 512; i++ ) {
		std_tree_insert( t, (void *) i );
		sum += i;
	}

	std_tree_foreach( t, sumtree, &treesum );

	printf( " (signed)  SUM: %d TreeSUM: %d\n", sum, treesum );
	std_tree_free( t );


	/* test against our home-brewed tree */
	t = std_tree_alloc( std_cmp_str );
	for ( i = 0; i <= 14; i++ ) {
		std_tree_insert( t, (void *) (strtree[i].data) );
	}

	printf("Reference tree: ");
	print_tree( &strtree[0] );
	printf("\n");

	printf("Test tree     : ");
	std_tree_foreach( t, printtree, NULL );
	printf("\n");

	std_tree_free( t );

	return 0;
}

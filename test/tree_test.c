
#include <stdio.h>
#include <string.h>

#include <opdis/tree.h>

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

static int printtree( void * data, void * arg ) {
	char *str = (char *) data;
	printf( "%s ", str );
	return 1;
}

static int sumtree( void * data, void * arg ) {
	int *sum = (int *) arg;
	long num = (long) data;
	*sum += num;
	return 1;
}

static int cmp_int( void * arg_a, void * arg_b ) {
	long a = (long) arg_a, b = (long) arg_b;

	if ( a < b ) {
		return -1;
	} else if ( a > b ) {
		return 1;
	}

	return 0;
}

static int cmp_str( void * arg_a, void * arg_b ) {
	const char * a = (const char *) arg_a, * b = (const char *) arg_b;

	return strcmp(a, b);
}

int main (void) {
	long i, sum, treesum;
	opdis_tree_t t;
	struct TN *strtn;

	/* ============================================== */
	/* test the unsigned int comparison */
	t = opdis_tree_init( NULL, NULL, NULL );
	sum = treesum = 0;
	for ( i = 0; i < 1024; i++ ) {
		opdis_tree_add( t, (void *) i );
		sum += i;
	}

	opdis_tree_foreach( t, sumtree, &treesum );

	printf( "(unsigned) SUM: %ld TreeSUM: %ld\n", sum, treesum );
	opdis_tree_free( t );
		

	/* test the signed int comparison */
	t = opdis_tree_init( NULL, cmp_int, NULL );
	sum = treesum = 0;
	for ( i = -512; i < 512; i++ ) {
		opdis_tree_add( t, (void *) i );
		sum += i;
	}

	opdis_tree_foreach( t, sumtree, &treesum );

	printf( " (signed)  SUM: %ld TreeSUM: %ld\n", sum, treesum );
	opdis_tree_free( t );


	/* test against our home-brewed tree */
	t = opdis_tree_init( NULL, cmp_str, NULL );
	for ( i = 0; i <= 14; i++ ) {
		opdis_tree_add( t, (void *) (strtree[i].data) );
	}

	printf("Reference tree: ");
	print_tree( &strtree[0] );
	printf("\n");

	printf("Test tree     : ");
	opdis_tree_foreach( t, printtree, NULL );
	printf("\n");

	opdis_tree_free( t );

	return 0;
}

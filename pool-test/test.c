#include "pool_template.h"
#include <stdio.h>


struct entry{
	int a;
	int b;
	int c;
	int d;
};

POOL_TEMPLATE( entry , 256 , entry_alloc , entry_free );



int main(int argc,char** argv)
{

	struct entry* es[256];

	struct entry* e;
	
	printf("alloc and record\n");
	int alloc_size = 0;
	while( e = entry_alloc() ){
		printf("%d\n",alloc_size );

		es[alloc_size] = e;
		alloc_size++;
	}

	printf("free half\n");
	int i;
	for(i = 0 ; i < alloc_size / 2; i++ )
		entry_free( es[i] );

	
	printf("alloc all\n");
	int x = 0;
	while( e = entry_alloc() ){
		printf("%d\n",x);

		x++;
	}
	

	return 0;
}

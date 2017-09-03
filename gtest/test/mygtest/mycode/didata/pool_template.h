#ifndef _POOL_TEMPLATE_H_
#define _POOL_TEMPLATE_H_


#include "list.h"


#define _POOL_NAME_(name)	 __pool_##name
#define _HEAD_NAME_(name)    __head_##name
#define _LIST_HEAD_INIT_(name) LIST_HEAD( __head_##name )
#define PACKAGE_NAME(name,size) union  {\
	struct list_head pool_node;\
	struct name		 entry;\
} _POOL_NAME_(name)[size] ;\


#define POOL_TEMPLATE( struct_name , pool_size , alloc_func , free_func ) \
\
static _LIST_HEAD_INIT_(struct_name);\
\
__attribute((constructor)) void __##struct_name##_pool_init()\
{ \
	static PACKAGE_NAME(struct_name,pool_size); \
    int i;\
    for( i = 0 ; i < pool_size ; i++ ){\
    	struct list_head* p = (struct list_head*)&(_POOL_NAME_(struct_name)[i]);\
        list_add( p , &(_HEAD_NAME_(struct_name)) );\
    }\
}\
\
static inline struct struct_name * alloc_func()\
{\
    struct list_head* __r;\
    \
    if( list_empty(&(_HEAD_NAME_(struct_name))) )\
        return NULL;\
    \
    __r = _HEAD_NAME_(struct_name).next;\
    list_del( __r );\
\
    return (struct struct_name*)__r;\
}\
\
static inline void free_func(struct struct_name* e)\
{\
   list_add( (struct list_head*)e , &(_HEAD_NAME_(struct_name)) ); \
}\


#endif

#include "Table_Def.h"
#include <stdio.h>
#include <stdlib.h>


/*特殊帧消息表*/
extern struct MsgAttr SpecialMsgTable[];
/*特殊帧消息表大小*/
extern const int special_size;

/*通用帧*/
extern struct MsgAttr GeneralMsgTable[];
extern const int general_size;


int struct_size = 0;

void list_all_message_entry(FILE* file, struct MsgAttr* table, int size)
{
	int i;
	for( i = 0 ; i < size ; i++ ){
		fprintf(file ,"\tvolatile gint %s_DATA[%d];\n" , table[i].name , table[i].attr_cnt );

		struct_size += sizeof(volatile gint);
	}
}


#if 0  //20170901

/* modified by rocky - 20170830 */
int main_shm(int argc)
{
	FILE* file = fopen( "shm_struct.h" , "w+" );
	if( !file ){
		printf("cannot create file:shm_struct.h\n");
		return -1;
	}
	
	/*file header*/
	fprintf( file , "#ifndef _SHM_STRUCT_H_\n" );
	fprintf( file , "#define _SHM_STRUCT_H_\n" );
	fprintf( file , "#include <glib.h>\n");
	fprintf( file , "#include \"Message_Def.h\"\n");
	fprintf( file , "\n\n\n");


	/**/
	fprintf( file , "struct shm_service_data{\n" );

	list_all_message_entry( file , GeneralMsgTable , general_size );
	list_all_message_entry( file , SpecialMsgTable , special_size );

	fprintf( file , "};\n");

	/*file tailer*/
	fprintf( file , "\n\n");
	fprintf( file , "#endif");

	fclose(file);
	printf("gen share memery data header ok size:%d...\n" , struct_size );

	return 0;
}

#endif //gtest

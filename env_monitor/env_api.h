/************************************************************
** filename:env_api.h
** autor:Rocky
** date:2017/04/28
** description:head file
************************************************************/
#ifndef _ENV_API_H
#define _ENV_API_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define _arg_in
#define _arg_out
#define _arg_in_out


#define ARRAY_MAX_MEMBER_NUM 64
#define BUFFER_SIZE 1024

/*函数返回值*/
typedef enum{
	RESULT_ERR = -1,
	RESULT_OK = 0,
}result_t;


typedef enum
{
	ENV_SERVICE_GET,		//0
	ENV_SERVICE_VALUE,		//1
	ENV_SERVICE_SET,		//2
	ENV_SERVICE_OK,			//3
	ENV_SERVICE_FUNC_ERROR,	//4
	ENV_SERVICE_NOT_EXIST	//5
}MSG_ID;


typedef struct
{
	char msg_buf[BUFFER_SIZE];
	unsigned int size;
} _DATA_BUFFER;


typedef struct env_unit
{
	char *env_name;
	char *env_value;
} _ENV_UNIT;

//解析收到的数据帧
result_t unpack_msg(void* msg_data , unsigned int msg_size, MSG_ID *id, _arg_out _ENV_UNIT *env_unit);

//获取数据帧的msgid
MSG_ID get_msg_id(const unsigned char *frame_buf, unsigned int size);

//获取数据帧中的name和value信息
result_t get_name_and_value(const unsigned char *frame_buf, unsigned int size, _arg_out _ENV_UNIT *env_unit);

//判断查询的env是否存在
int is_env_exist(const char *env_name_ptr, _ENV_UNIT msg_ptr[], unsigned int member_total_num);

//封装结果帧
unsigned int pack_msg(const MSG_ID msg_id, const _ENV_UNIT env_unit_ptr, void  *msg_ptr);


void array_member_free(_ENV_UNIT array[], unsigned int member_num);

void *malloc_rebuild(unsigned int size);


void free_rebuild(void *ptr);

int env_test(_arg_in void* msg_data, _arg_in unsigned int msg_size);
void init();

#endif //_ENV_API_H

#ifndef _DEBUG_H_
#define _DEBUG_H_

/*每个项目都有一个调试开关的头文件*/
/*调试开关的宏必须放在#include "Debug_Def.h"之前*/

/*调试开关*/
#define _DEBUG_

/*下行调试*/
#define UART_DOWN_DEBUG

/*上行调试*/
#define UART_UP_DEBUG

/*无crc校验*/
#define NO_CRC_CHECK


#define MODULE_NAME "IPC_UART"

#include "Debug_Def.h"


#define UART_LOG LOG
#endif

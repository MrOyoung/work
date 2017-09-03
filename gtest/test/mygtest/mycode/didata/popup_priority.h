#ifndef _POPUP_PRIORITY_H_
#define _POPUP_PRIORITY_H_

enum{
	POPUP_PRIORITY_LEVEL_NONE = 0,

	POPUP_PRIORITY_LEVEL_C,
	POPUP_PRIORITY_LEVEL_B,
	POPUP_PRIORITY_LEVEL_A,
};

enum{
	POPUP_POWER_ON = (1<<4),
};

#ifdef __cplusplus
extern "C" {
#endif 

extern int get_messageid_priority(unsigned int messageid);

#ifdef __cplusplus
}
#endif 

#endif

#ifndef _POPUP_ON_H_
#define _POPUP_ON_H_

#ifdef __cplusplus
extern "C" {
#endif

/*
*
*
*/
int is_this_popup_on(unsigned int messageid);

/*
*
*
*/
void set_this_popup_on(unsigned int messageid);

/*
*
*
*/
void set_this_popup_off(unsigned int messageid);


#ifdef __cplusplus
}
#endif

#endif

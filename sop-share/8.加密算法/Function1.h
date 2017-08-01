/******************************************************************************
 * Project ZXQ Internet Car
 * (c) copyright 2015
 * Company SAIC Motor
 * All rights reserved
 * Secrecy Level STRICTLY CONFIDENTIAL
 *****************************************************************************/
/*
 * @file Function1.h
 * @group TBox
 * @author saic
 * Declaration of Function1
 */
#ifndef _FUNCTION1_H_
#define _FUNCTION1_H_
/******************************************************************************/
#ifdef __cplusplus
extern "C" {
#endif

/*-----------------------------------------------------------------------------
 * MACROS AND CONSTANTS
 *---------------------------------------------------------------------------*/
#ifndef UINT16
#define UINT16              unsigned short
#endif
#ifndef UCHAR
#define UCHAR               unsigned char
#endif

/*-----------------------------------------------------------------------------
 * Secure transmission Function 1
 *@t  the type, 1 byte 
 *@p  pointer of 6 bytes parameter
 *@k1 pointer of 20 bytes key1 
 *@k2 pointer of 20 bytes key2,input the pointer and get the value calculated  
 *---------------------------------------------------------------------------*/
void Function1(const UCHAR  t, const UCHAR* p,const UCHAR* k1,UCHAR* k2 );

#ifdef __cplusplus  // close out "C" linkage in case of c++ compiling
}
#endif

/******************************************************************************/
#endif /* __FUNCTION1_H__ */
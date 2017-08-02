#ifndef _CONVERT_H
#define _CONVERT_H
#include <stdint.h>


/*======================================
	Prototype
======================================*/

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */
bool YV12_to_RGB24(unsigned char* pYV12, unsigned char* pRGB24, int iWidth, int iHeight);
bool convert_yuyv_to_bgrx8888(void *dst, void *src, uint32_t width, uint32_t height);
void YUV420_RGB32(char *pRGB, char *pYUV,int Width, int Height);
bool convertYUV420ToRGB32(const char *yuv,int width,int height,char **pRgb,int *realLen);
void yuv420_2_rgb8888(uint8_t  *dst_ptr,const uint8_t *y_ptr,const uint8_t  *u_ptr,const uint8_t  *v_ptr,int32_t   width,int32_t   height);
#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _CONVERT_H */

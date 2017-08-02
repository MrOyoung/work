#ifndef _CAMERA_H
#define _CAMERA_H
#include <stdint.h>

/*======================================
	Structure
======================================*/

struct camera_ctx;

/*======================================
	Prototype
======================================*/

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

struct camera_ctx *camera_init(char *dev_name);
void camera_terminate(struct camera_ctx *ctx);

bool camera_start_capturing(struct camera_ctx *ctx);
bool camera_stop_capturing(struct camera_ctx *ctx);

bool camera_read_frame(struct camera_ctx *ctx, void **dest, unsigned int dest_size);

uint32_t camera_get_width(struct camera_ctx *ctx);
uint32_t camera_get_height(struct camera_ctx *ctx);
uint32_t camera_get_frame_size(struct camera_ctx *ctx);

int QueueBuff(struct camera_ctx *ctx);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _CAMERA_H */

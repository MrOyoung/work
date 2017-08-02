/*
 * The MIT License (MIT)
 *
 * Copyright © 2014 faith
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that copyright
 * notice and this permission notice appear in supporting documentation, and
 * that the name of the copyright holders not be used in advertising or
 * publicity pertaining to distribution of the software without specific,
 * written prior permission.  The copyright holders make no representations
 * about the suitability of this software for any purpose.  It is provided "as
 * is" without express or implied warranty.
 *
 * THE COPYRIGHT HOLDERS DISCLAIM ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
 * EVENT SHALL THE COPYRIGHT HOLDERS BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
 * DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE
 * OF THIS SOFTWARE.
 */

/*======================================
  Header include
  ======================================*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include <assert.h>

#include <fcntl.h>
#include <getopt.h>
#include <sys/stat.h>
#include <errno.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/mman.h>

#include <linux/videodev2.h>

#include "common.h"
#include "lvds_capture.h"

/*======================================
  Constant
  ======================================*/

#define DEVICE_NAME		"/dev/video2"
#define PIXEL_FORMAT	V4L2_PIX_FMT_YUV420
#define PIXEL_DEPTH		1.5					/* 2 Bytes per pixel */


/*======================================
  Structure
  ======================================*/

struct buffer {
	void   *start;
	size_t	length;
};

struct camera_ctx {
	char		   *dev_name;
	int				fd;
	struct buffer  *buffers;
	unsigned int	buffers_nr;

	uint32_t		width, height;
};


/*======================================
  Prototype
  ======================================*/

static bool open_device(struct camera_ctx *ctx);
static void close_device(struct camera_ctx *ctx);

static bool init_device(struct camera_ctx *ctx);
static void terminate_device(struct camera_ctx *ctx);

static bool init_mmap(struct camera_ctx *ctx);

static int read_frame(struct camera_ctx *ctx, void **dest, unsigned int dest_size);

static bool get_frame_size(struct camera_ctx *ctx);

static int xioctl(int fd, int request, void *arg);


/*======================================
  Public function
  ======================================*/

	struct camera_ctx *
camera_init(char *dev_name)
{
	struct camera_ctx *ctx;

	ctx = (struct camera_ctx *)malloc(sizeof(struct camera_ctx));
	if (!ctx) {
		LOG_ERROR("Out of Memory");
		return NULL;
	}

	ctx->dev_name = dev_name;

	/* open device interface /dev/viedo3 */
	if (!open_device(ctx)) {
		free(ctx);
		return NULL;
	}

	if (!get_frame_size(ctx)) {
		close_device(ctx);
		free(ctx);
		return NULL;
	}

	if (!init_device(ctx)) {
		close_device(ctx);
		free(ctx);
		return NULL;
	}

	return ctx;
}

	void
camera_terminate(struct camera_ctx *ctx)
{
	if (!ctx)
		return;

	terminate_device(ctx);
	close_device(ctx);
	free(ctx);
}

	bool
camera_start_capturing(struct camera_ctx *ctx)
{
	unsigned int i;
	struct v4l2_buffer buf;
	enum v4l2_buf_type type;

	if (!ctx)
		return false;

	for (i = 0; i < ctx->buffers_nr; i++) {
		memset(&buf, 0, sizeof(buf));
		buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		buf.memory = V4L2_MEMORY_MMAP;
		buf.index = i;

		if (xioctl(ctx->fd, VIDIOC_QBUF, &buf) < 0) {
			LOG_PERROR("VIDIOC_QBUF");
			return false;
		}
	}

	type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	if (xioctl(ctx->fd, VIDIOC_STREAMON, &type) < 0) {
		LOG_PERROR("VIDIOC_STREAMON");
		return false;
	}

	/* empty reading */
	for (i = 0; i < 5; i++)
		camera_read_frame(ctx, NULL, 0);

	return true;
}

	bool
camera_stop_capturing(struct camera_ctx *ctx)
{
	enum v4l2_buf_type type;

	if (!ctx)
		return false;

	type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	if (xioctl(ctx->fd, VIDIOC_STREAMOFF, &type) < 0) {
		LOG_PERROR("VIDIOC_STREAMOFF");
		return false;
	}

	return true;
}

	bool
camera_read_frame(struct camera_ctx *ctx, void **dest, unsigned int dest_size)
{
	fd_set fds;
	struct timeval tv;
	int status;

	if (!ctx)
		return false;

	do {
		FD_ZERO(&fds);
		FD_SET(ctx->fd, &fds);

		/* Timeout */
		tv.tv_sec = 2;
		tv.tv_usec = 0;

		status = select(ctx->fd + 1, &fds, NULL, NULL, &tv);

		printf("camera_read_frame:status :%d\n",status);
		if (status == -1) {
			if (errno == EINTR)
				continue;

			LOG_PERROR("select");
			return false;
		}

		if (status == 0) {
			LOG_ERROR("select timeout");
			return false;
		}

		status = read_frame(ctx, dest, dest_size);
		if (status != 0)
			break;

		/* EAGAIN - continue select loop */
	} while(1);

	return (status == 1) ? true : false;
}

	uint32_t
camera_get_width(struct camera_ctx *ctx)
{
	if (!ctx)
		return 0;

	return ctx->width;
}

	uint32_t
camera_get_height(struct camera_ctx *ctx)
{
	if (!ctx)
		return 0;

	return ctx->height;
}

	uint32_t
camera_get_frame_size(struct camera_ctx *ctx)
{
	if (!ctx)
		return 0;

	return (800 * 480 * 3 / 2);
}


/*======================================
  Inner function
  ======================================*/

	static bool
open_device(struct camera_ctx *ctx)
{
	struct stat st;

	if (!ctx)
		return false;

	if (stat(ctx->dev_name, &st) == -1) {
		LOG_ERROR("Cannot identify '%s' : %d, %s", ctx->dev_name, errno, strerror(errno));
		return false;
	}

	if (!S_ISCHR(st.st_mode)) {
		LOG_ERROR("%s is no device", ctx->dev_name);
		return false;
	}

	ctx->fd = open(ctx->dev_name, O_RDWR | O_NONBLOCK, 0);

	if (ctx->fd < 0) {
		LOG_ERROR("Cannot open '%s': %d, %s", ctx->dev_name, errno, strerror(errno));
		return false;
	}

	return true;
}

	static void
close_device(struct camera_ctx *ctx)
{
	if (!ctx)
		return;

	close(ctx->fd);
}

	static bool
init_device(struct camera_ctx *ctx)
{
	struct v4l2_capability cap;
	struct v4l2_cropcap cropcap;
	struct v4l2_crop crop;
	struct v4l2_format fmt;
	struct v4l2_streamparm parm = {0};
	struct v4l2_frmsizeenum fszenum = {0};
	v4l2_std_id id;

	(void)cropcap;
	(void)crop;

	if (!ctx)
		return false;

	if (xioctl (ctx->fd, VIDIOC_G_STD, &id) < 0) {
		LOG_PERROR("VIDIOC_G_STD error");
	}else{

		if (xioctl(ctx->fd, VIDIOC_S_STD, &id) < 0) {
			return false;
		}
	}

	if (xioctl(ctx->fd, VIDIOC_QUERYCAP, &cap) < 0) {
		if (errno == EINVAL)
			LOG_ERROR("%s is no V4L2 device", ctx->dev_name);
		else
			LOG_PERROR("VIDIOC_QUERYCAP");

		return false;
	}

	if (!(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE)) {
		LOG_ERROR("%s is no video capture device", ctx->dev_name);
		return false;
	}

	if (!(cap.capabilities & V4L2_CAP_STREAMING)) {
		LOG_ERROR("%s does not support streaming I/O", ctx->dev_name);
		return false;
	}

	int input = 1;
	if(xioctl(ctx->fd, VIDIOC_S_INPUT,&input) < 0)
		printf("VIDIOC_S_INPUT error\n");

	if(xioctl(ctx->fd, VIDIOC_G_INPUT,&input) ==  0)
		printf("VIDIOC_G_INPUT,&input:%d\n",input);


	//timeperframe.numerator

	printf("VIDIOC_set param start\n");
	parm.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	parm.parm.capture.timeperframe.numerator = 1;
	parm.parm.capture.timeperframe.denominator = 10;
	parm.parm.capture.capturemode = 0;
	if (xioctl(ctx->fd,VIDIOC_S_PARM, &parm) < 0) {
		printf("VIDIOC_set param is -1");
		return false;
	}
	printf("VIDIOC_set param end\n");

	//get fmt
	memset(&fmt, 0, sizeof(fmt));

	fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	if (xioctl(ctx->fd, VIDIOC_G_FMT, &fmt) < 0) {
		//     LOG_PERROR("VIDIOC_G_FMT");
		printf("my_VIDIOC_G_FMT fail is -1\n");
		return false;
	}


	printf("VIDIOC_G_FMT  width:%d ,height:%d \n",fmt.fmt.pix.width,fmt.fmt.pix.height);
	//fszenum.index =
	fszenum.index = 1;
	fszenum.pixel_format = fmt.fmt.pix.pixelformat;
	if (xioctl(ctx->fd, VIDIOC_ENUM_FRAMESIZES, &fszenum) < 0) {
		printf("VIDIOC_ENUM_FRAMESIZES, eoor\n");
	}
	printf("VIDIOC_ENUM_FRAMESIZES-width:%d ,height:%d \n",fszenum.discrete.width,fszenum.discrete.height);

	printf("fmt.fmt.pix.pixelformat:%d \n",fmt.fmt.pix.pixelformat);
	if(fmt.fmt.pix.field == V4L2_FIELD_INTERLACED)
		printf("V4L2_FIELD_INTERLACED\n");
	//set fmt
	fmt.type                                = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	fmt.fmt.pix.width               = 800;//camera_get_width(ctx);
	fmt.fmt.pix.height              = 480;//camera_get_height(ctx);
	fmt.fmt.pix.pixelformat = PIXEL_FORMAT;//PIXEL_FORMAT;
	fmt.fmt.pix.field = V4L2_FIELD_ANY;//V4L2_FIELD_NONE;//V4L2_FIELD_ANY;
	fmt.fmt.pix.priv = 0;
	fmt.fmt.pix.bytesperline = 0;
	fmt.fmt.pix.sizeimage = 0;

	if (xioctl(ctx->fd, VIDIOC_S_FMT, &fmt) < 0) {
		LOG_PERROR("VIDIOC_S_FMT");
		return false;
	}

	/* buffer.start */
	return init_mmap(ctx);
}

	static void
terminate_device(struct camera_ctx *ctx)
{
	unsigned int i;

	if (!ctx)
		return;

	for (i = 0; i < ctx->buffers_nr; i++)
		if (munmap(ctx->buffers[i].start, ctx->buffers[i].length) < 0)
			LOG_PERROR("munmap");
}

	static bool
init_mmap(struct camera_ctx *ctx)
{
	struct v4l2_requestbuffers req;

	if (!ctx)
		return false;

	memset(&req, 0, sizeof(req));
	req.count = 4;
	req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	req.memory = V4L2_MEMORY_MMAP;
	if (xioctl(ctx->fd, VIDIOC_REQBUFS, &req) < 0) {
		if (errno == EINVAL)
			LOG_ERROR("%s does not support memory mapping", ctx->dev_name);
		else
			LOG_PERROR("VIDIOC_REQBUFS");

		return false;
	}

	LOG_ERROR("%d count mapping",req.count);
	if (req.count < 2) {
		LOG_ERROR("Insufficient buffer memory on %s", ctx->dev_name);
		return false;
	}

	ctx->buffers = calloc(req.count, sizeof(struct buffer));
	if (!ctx->buffers) {
		LOG_ERROR("Out of memory");
		return false;
	}

	for (ctx->buffers_nr = 0; ctx->buffers_nr < req.count; ctx->buffers_nr++) {
		struct v4l2_buffer buf;

		memset(&buf, 0, sizeof(buf));
		buf.type	= V4L2_BUF_TYPE_VIDEO_CAPTURE;
		buf.memory	= V4L2_MEMORY_MMAP;
		buf.index	= ctx->buffers_nr;
		if (xioctl(ctx->fd, VIDIOC_QUERYBUF, &buf) < 0) {
			LOG_PERROR("VIDIOC_QUERYBUF");
			return false;
		}

		ctx->buffers[ctx->buffers_nr].length = buf.length;
		ctx->buffers[ctx->buffers_nr].start =
			mmap(NULL, buf.length, PROT_READ | PROT_WRITE, MAP_SHARED, ctx->fd, buf.m.offset);
		if (ctx->buffers[ctx->buffers_nr].start == MAP_FAILED) {
			LOG_PERROR("mmap");
			return false;
		}
	}

	return true;
}

struct v4l2_buffer g_buf;

bool wait = false;

	static int
read_frame(struct camera_ctx *ctx, void **dest, unsigned int dest_size)
{
	struct v4l2_buffer buf;

	if (!ctx)
	{
		printf("read_frame ctx = null\n");
		return -1;
	}

	memset(&buf, 0, sizeof(buf));
	buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	buf.memory = V4L2_MEMORY_MMAP;
	if (xioctl(ctx->fd, VIDIOC_DQBUF, &buf) < 0) {
		if (errno == EAGAIN) {
			//		printf("errno == EAGAIN\n");
			return 0;
		} else {
			LOG_PERROR("VIDIOC_DQBUF");
			return -1;
		}
	}

	if (buf.index >= ctx->buffers_nr) {
		LOG_ERROR("buf.index(%u) >= buffers_nr(%u)", buf.index, ctx->buffers_nr);
		return -1;
	}

	if (dest && *dest) {
		if (dest_size < buf.bytesused) {
			LOG_ERROR("dest_size(%u) < buf.bytesused(%u)", dest_size, buf.bytesused);
			return -1;
		}

		//memcpy(dest, ctx->buffers[buf.index].start, buf.bytesused);
		//memcpy(*dest, ctx->buffers[buf.index].start, buf.bytesused);
		*dest = ctx->buffers[buf.index].start;
	}

	if (wait)
	{
		memcpy(&g_buf, &buf, sizeof(g_buf));
	}
	else
	{
		if (xioctl(ctx->fd, VIDIOC_QBUF, &buf) < 0) {
			LOG_PERROR("VIDIOC_QBUF");
			return -1;
		}   
	}


	return 1;
}

int QueueBuff(struct camera_ctx *ctx)
{
	if (xioctl(ctx->fd, VIDIOC_QBUF, &g_buf) < 0) {
		LOG_PERROR("VIDIOC_QBUF");
		return -1;
	}
	return 0;
}

	static bool
get_frame_size(struct camera_ctx *ctx)
{
	//	struct v4l2_frmsizeenum fmt;

	if (!ctx)
		return false;

	//	memset(&fmt, 0, sizeof(fmt));
	//	fmt.index = 0;
	//	fmt.pixel_format = PIXEL_FORMAT;

	//	if (xioctl(ctx->fd, VIDIOC_ENUM_FRAMESIZES, &fmt) < 0) {
	//		LOG_PERROR("VIDIOC_ENUM_FRAMESIZES");
	//		return false;
	//	}

	//	if (fmt.type != V4L2_FRMSIZE_TYPE_DISCRETE) {
	//		LOG_ERROR("Unknown TYPE");
	//		return false;	
	//	}

	//	ctx->width	= fmt.discrete.width;
	//	ctx->height	= fmt.discrete.height;

	ctx->width = 720;
	ctx->height = 480;

	return true;
}

	static int
xioctl(int fh, int request, void *arg)
{
	int r;

	do {
		r = ioctl(fh, request, arg);
	} while ((r < 0) && (errno == EINTR));

	return r;
}


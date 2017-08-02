#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>
#include <signal.h>
#include <errno.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <linux/fb.h>
#include <getopt.h>
#include <wayland-client.h>
#include "common.h"
#include "lvds_capture.h"
#include "wayland_egl.h"
#include "convert.h"
#include "util.h"
#include <poll.h>

/*======================================
  Constant
  ======================================*/

#define DEFAULT_DEVICE_NAME		"/dev/video3"

/*======================================
  Public function
  ======================================*/

extern bool wait;
	int
main(int argc, char *argv[])
{
	/* lvds */
	struct camera_ctx *camera_ctx;
	char *dev_name;
	bool ret;
	bool quiet = false;
	(void)quiet;
	dev_name = DEFAULT_DEVICE_NAME;

	/* wayland */
	struct SWindowData window = {0};

	/* wayland.c line862 */
	wayland_egl_init(&window);
	/*  */
	camera_ctx = camera_init(dev_name);
	if (!camera_ctx) {
		exit(EXIT_FAILURE);
	}

	if (!camera_start_capturing(camera_ctx)) {
		camera_stop_capturing(camera_ctx);
		camera_terminate(camera_ctx);
		exit(EXIT_FAILURE);
	}

	ret = camera_read_frame(camera_ctx, (void **)&window.buffer, camera_get_frame_size(camera_ctx));
	if (!ret) {

		camera_stop_capturing(camera_ctx);
		camera_terminate(camera_ctx);
		exit(EXIT_FAILURE);
	}

	wait = true;;

	/*wayland_is_running()*/
	while (1) {

		util_show_fps();

		usleep(70000);

		/* read data from device interface - fd */
		ret = camera_read_frame(camera_ctx, (void **)&window.buffer, camera_get_frame_size(camera_ctx));
		if (!ret)
			break;

		wayland_dispatch_event(&window);

		{
			if (!camera_ctx)
			{
				printf("read_frame ctx = null\n");
			}
			else if(-1 == QueueBuff(camera_ctx))
			{
				printf("error: VIDIOC_QBUF!\n");
			}
		}

		check_events(window.display);
	}

	close_egl_all(&window);
	camera_stop_capturing(camera_ctx);
	camera_terminate(camera_ctx);

	return 0;
}


#make file

base_path = /home/rocky/Tools/toolchain/arm_eabi_gcc5.3.0_glibc2.23.0/imx6qsabresd
mgcc = /home/rocky/Tools/toolchain/arm_eabi_gcc5.3.0_glibc2.23.0/bin/arm-poky-linux-gnueabi-gcc 

minclude+=-I$(base_path)/usr/include/wayland-input-device 
minclude+=-I$(base_path)/usr/include/wayland-screenshooter 
minclude+=-I$(base_path)/usr/include/libpng16 
minclude+=-I$(base_path)/usr/include/dbus-1.0 
minclude+=-I$(base_path)/usr/include/glib-2.0 
minclude+=-I$(base_path)/usr/lib/dbus-1.0/include 
minclude+=-I$(base_path)/usr/lib/glib-2.0/include
minclude += -I/home/rocky/Tools/kanzi/tifeagle-clients-1.9.0/protocol


mflags += -std=c99 -Wall -Wextra  -Wno-unused-parameter -Wno-missing-field-initializers 
mflags += -D_POSIX_C_SOURCE=200809L -D_GNU_SOURCE -fvisibility=hidden  -march=armv7-a -marm  -mthumb-interwork -mfloat-abi=hard 
mflags += -mfpu=neon -mtune=cortex-a9  --sysroot=$(base_path)  
mflags += -O2 -pipe -g -feliminate-unused-debug-types -Wpoison-system-directories -Wchar-subscripts -Wunused-function -Wunused-function

#platform
mflags += -DLINUX
mflags += -Wl,-O1 -Wl,--hash-style=gnu -Wl,--as-needed  -Wl,-O1 -Wl,--hash-style=gnu -Wl,--as-needed

mlaflags += -rdynamic -lwayland-client -lm -lEGL -lGLESv2 -lpng16 -ldbus-glib-1 -ldbus-1 -lgobject-2.0 -lglib-2.0 -lpthread

wl-lvds = lvds_app
object = main.o lvds_capture.o convert.o wayland_egl.o util.o 
tifeagle-object = tifeagle_simple_shm-ivi-application-protocol.o

$(wl-lvds):$(object) $(tifeagle-object)
	$(mgcc) $(mflags)  $^ -o $@ $(mlaflags) 
#	$(mgcc) $(eglflag)  $^ -o $@ $(eglaflag)
	

.c.o :
	$(mgcc) -o $@ -c $< $(mflags) $(minclude)
        
.PHONY:clean
clean:
	rm -rf $(wl-lvds)  $(object)	


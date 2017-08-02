#ifndef _WAYLAND_EGL_H
#define _WAYLAND_EGL_H
#include <wayland-egl.h>

#include <GLES2/gl2.h>
#include <EGL/egl.h>

#include <wayland-client.h>
struct SWindowData;

struct SWindowData
{
   /// The wayland display connection.
   struct wl_display *display;

   /// Registry for global wayland objects.
   struct wl_registry *registry;

   /// The wayland compositor.
   /** The wl_compositor_create_surface() function is used from this interface.
    *  The compositor is retrieved in the function displayHandleGlobal(). */
   struct wl_compositor *compositor;

   /// The wayland shell.
   /** The wl_shell_get_shell_surface() function is used from this interface.
    *  The shell is retrieved in the function displayHandleGlobal(). */
   struct wl_shell *shell;

   struct wl_ivi_shell* ivi_shell;
   struct wl_ivi_animation_group *group;
   /// The native wayland egl window.
   struct wl_egl_window *native;

   /// The wayland surface.
   struct wl_surface *surface;


   struct ivi_application *ivi_application;

   struct ivi_surface *ivi_surface;

   /// The wayland shell surface interface.
   /** The wl_shell_surface_set_toplevel() function is used from this interface. */
   struct wl_shell_surface *shell_surface;

   /// The frame callback.
   struct wl_callback *callback;

   /// The EGL surface.
   EGLSurface eglSurface;

   /// The EGL display connection.
   EGLDisplay eglDisplay;

   /// The EGL context.
   EGLContext eglContext;

   /// The EGL surface configuration.
   EGLConfig eglConfig;

   /// Wayland event mask.
   uint32_t mask;

   /// Width and height of the window.
   int width;
   int  height;

   /// Uniform for the rotation matrix.
   GLuint rotationUniform;

   /// Attribute location for position.
   GLuint posAttribute;

   /// Attribute location for color.

   GLuint colAttribute;

   bool buffer_empty;
   unsigned char *buffer;

   unsigned char *plane[3];

};
void wayland_egl_init(struct SWindowData *window);
bool wayland_queue_buffer(struct SWindowData *window, void *buff);
int wayland_dispatch_event(struct SWindowData *window);
void close_egl_all(struct SWindowData *window);
void check_events(struct wl_display* display);

#endif

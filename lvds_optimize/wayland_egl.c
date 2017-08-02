#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>
#include <signal.h>
#include "wayland_egl.h"
#include <poll.h>
#include <GLES/glrename.h>
#include <GLES3/gl3.h>

#include <ivi-application-client-protocol.h>

#include <pthread.h>

//add user
#include <math.h>
#define EGL_ERROR_CASE(E, F) case E: fprintf(stderr, "error %s at %s\n", #E, F); break

static void printEGLError(const char *function)
{
   int err = eglGetError();
   switch (err) {
   case EGL_SUCCESS:
      break;
   EGL_ERROR_CASE(EGL_NOT_INITIALIZED, function);
   EGL_ERROR_CASE(EGL_BAD_ACCESS, function);
   EGL_ERROR_CASE(EGL_BAD_ALLOC, function);
   EGL_ERROR_CASE(EGL_BAD_ATTRIBUTE, function);
   EGL_ERROR_CASE(EGL_BAD_CONTEXT, function);
   EGL_ERROR_CASE(EGL_BAD_CONFIG, function);
   EGL_ERROR_CASE(EGL_BAD_CURRENT_SURFACE, function);
   EGL_ERROR_CASE(EGL_BAD_DISPLAY, function);
   EGL_ERROR_CASE(EGL_BAD_SURFACE, function);
   EGL_ERROR_CASE(EGL_BAD_MATCH, function);
   EGL_ERROR_CASE(EGL_BAD_PARAMETER, function);
   EGL_ERROR_CASE(EGL_BAD_NATIVE_PIXMAP, function);
   EGL_ERROR_CASE(EGL_BAD_NATIVE_WINDOW, function);
   EGL_ERROR_CASE(EGL_CONTEXT_LOST, function);
   default:
      fprintf(stderr, "unknown error 0x%X at %s\n", err, function);
   }
}
#define CHECK_EGL(E) \
   if (!(E)) { printEGLError(#E); abort(); }
//#include <wayland-egl.h>

//#include <GLES2/gl2.h>
//#include <EGL/egl.h>
static void frustum(GLfloat left, GLfloat right, GLfloat bottom, GLfloat top,
          GLfloat znear, GLfloat zfar);

static GLfloat *matrix_rmultiply(const GLfloat * A, GLfloat * B);

static GLfloat *rotate_z(GLfloat * A, float angle);

static GLfloat *translation(GLfloat * M, float x, float y, float z);

void check_events(struct wl_display* display);
static void viewmatrix(void);
/* location of the ModelViewProjectionMatrix uniform used in the vertex shader */
static GLuint mvp_matrix_loc;
/* used for assignments in translation and rotation functions */
static const GLfloat Identity[16] = {
	1.0f, 0.0f, 0.0f, 0.0f,
	0.0f, 1.0f, 0.0f, 0.0f,
	0.0f, 0.0f, 1.0f, 0.0f,
	0.0f, 0.0f, 0.0f, 1.0f
};

/* updated for each gear */
static GLfloat ModelViewProjectionMatrix[16];

/* updated when the window is reshaped */
static GLfloat ProjectionMatrix[16];

/* updated everytime the view matrix changes (keystrokes) */
static GLfloat ViewMatrix[16];


#define DEG2RAD 0.017453292519943295769236907684886f
static const float deg2rad = DEG2RAD;

static const float halfpi = 1.5707963267948966192313216916398f;
static const float pi = 3.1415926535897932384626433832795f;
static const float twopi = 6.283185307179586476925286766559f;


static GLfloat *egl_rotate_x(GLfloat * M, float angle)
{
	M[0] = Identity[0];
	M[1] = Identity[1];
	M[2] = Identity[2];
	M[3] = Identity[3];
	M[4] = Identity[4];
	M[5] = cos(angle);
	M[6] = sin(angle);
	M[7] = Identity[7];
	M[8] = Identity[8];
	M[9] = -sin(angle);
	M[10] = cos(angle);
	M[11] = Identity[11];
	M[12] = Identity[12];
	M[13] = Identity[13];
	M[14] = Identity[14];
	M[15] = Identity[15];
	return M;
}

static GLfloat *egl_rotate_y(GLfloat * M, float angle)
{
	M[0] = cos(angle);
	M[1] = Identity[1];
	M[2] = sin(angle);
	M[3] = Identity[3];
	M[4] = Identity[4];
	M[5] = Identity[5];
	M[6] = Identity[6];
	M[7] = Identity[7];
	M[8] = -sin(angle);
	M[9] = Identity[9];
	M[10] = cos(angle);
	M[11] = Identity[11];
	M[12] = Identity[12];
	M[13] = Identity[13];
	M[14] = Identity[14];
	M[15] = Identity[15];
	return M;
}

static GLfloat *egl_rotate_z(GLfloat * M, float angle)
{
	M[0] = cos(angle);
   	M[1] = sin(angle);
   	M[2] = Identity[2];
   	M[3] = Identity[3];
   	M[4] = -sin(angle);
   	M[5] = cos(angle);
   	M[6] = Identity[6];
   	M[7] = Identity[7];
   	M[8] = Identity[8];
   	M[9] = Identity[9];
   	M[10] = Identity[10];
   	M[11] = Identity[11];
   	M[12] = Identity[12];
   	M[13] = Identity[13];
   	M[14] = Identity[14];
   	M[15] = Identity[15];
   	return M;	
}
//egl scale
static GLfloat *egl_scale(GLfloat * M, float scale)
{
   	M[0] = scale;
   	M[1] = Identity[1];
   	M[2] = Identity[2];
   	M[3] = Identity[3];
   	M[4] = Identity[4];
   	M[5] = scale;
   	M[6] = Identity[6];
   	M[7] = Identity[7];
   	M[8] = Identity[8];
   	M[9] = Identity[9];
   	M[10] = scale;
   	M[11] = Identity[11];
   	M[12] = Identity[12];
   	M[13] = Identity[13];
   	M[14] = Identity[14];
   	M[15] = Identity[15];
   	return M;	
}

//egl translation
static GLfloat *egl_translation(GLfloat * M, float x, float y, float z)
{
   	M[0] = Identity[0];
   	M[1] = Identity[1];
   	M[2] = Identity[2];
   	M[3] = Identity[3];
   	M[4] = Identity[4];
   	M[5] = Identity[5];
   	M[6] = Identity[6];
   	M[7] = Identity[7];
   	M[8] = Identity[8];
   	M[9] = Identity[9];
   	M[10] = Identity[10];
   	M[11] = Identity[11];
   	M[12] = x;
   	M[13] = y;
   	M[14] = z;
   	M[15] = Identity[15];
   	return M;
}
/// The vertex shader source code.
static const char *vertexShaderSource =
	"precision mediump float;\n"
	"attribute vec4 in_position;\n"
	"attribute vec2 in_tex_coord;\n"
	"varying vec2 tc;\n"
	//"uniform mat4 ModelViewProjectionMatrix;\n"
	"void main() {\n"
	"  gl_Position = in_position;\n"
	"  tc = in_tex_coord;\n"
	"}\n";


/// The fragment shader source code.
static const char *fragmentShaderSource =
	"precision mediump float;\n"
	"uniform sampler2D tex_my;\n"
	"varying vec2 tc;\n"
	"void main() {\n"
	"  gl_FragColor = vec4(texture2D(tex_my, tc).r,texture2D(tex_my, tc).g,texture2D(tex_my, tc).b,255.0);\n"
	"}\n";


GLuint texYId;
GLuint texUId;
GLuint texVId;

void loadYUV(){

	glGenTextures ( 1, &texYId );
   	glBindTexture ( GL_TEXTURE_2D, texYId );
   	glTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
   	glTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
   	glTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
   	glTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
   	glBindTexture(GL_TEXTURE_2D,0);



}

//unsigned char *plane[3];
GLuint textureUniformY, textureUniformU,textureUniformV;

/// Init EGL connection and create EGL context.
static void initEGL(struct SWindowData* window)
{
   	static const EGLint context_attribs[] = { EGL_CONTEXT_CLIENT_VERSION, 2, EGL_NONE };

   	EGLint config_attribs[] = { EGL_SURFACE_TYPE, EGL_WINDOW_BIT, EGL_RED_SIZE, 1, EGL_GREEN_SIZE, 1, EGL_BLUE_SIZE, 1,
         EGL_ALPHA_SIZE, 1, EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT, EGL_NONE };

   	EGLint major, minor, n;

   	window->eglDisplay = eglGetDisplay((EGLNativeDisplayType)window->display);
   	eglInitialize(window->eglDisplay, &major, &minor);
   	eglBindAPI(EGL_OPENGL_ES_API);
   	eglChooseConfig(window->eglDisplay, config_attribs, &window->eglConfig, 1, &n);
   	window->eglContext = eglCreateContext(window->eglDisplay, window->eglConfig, EGL_NO_CONTEXT, context_attribs);
}

/// Close the EGL connection.
static void closeEGL(struct SWindowData *window)
{
   	eglMakeCurrent(window->eglDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT );
   	eglTerminate(window->eglDisplay);
   	eglReleaseThread();
   	if(window->buffer)
		free(window->buffer);
}

/// Creates a vertex or fragment shader.
static GLuint createShader(struct SWindowData *window, const char *source, GLenum shaderType)
{
   	GLuint shader;
   	GLint status;

   	shader = glCreateShader(shaderType);

  	glShaderSource(shader, 1, (const char **) &source, NULL );
   	glCompileShader(shader);

   	glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
   	if (!status)
   	{
     		char log[1000];
        	GLsizei len;
        	glGetShaderInfoLog(shader, 1000, &len, log);
        	fprintf(stderr, "Error: compiling %s: %*s\n", shaderType == GL_VERTEX_SHADER ? "vertex" : "fragment", len, log);
        	exit(1);
   	}

   	return shader;
}

unsigned char *	converted ;
unsigned char *part_1;
unsigned char *part_2;
/// OpenGL initialization.
static void initGL(struct SWindowData *window)
{
   	GLuint frag, vert;
   	GLuint program;
   	GLint status;

       
       glViewport(0,0,800,480);
       


   	frag = createShader(window, fragmentShaderSource, GL_FRAGMENT_SHADER);
   	vert = createShader(window, vertexShaderSource, GL_VERTEX_SHADER);

   	program = glCreateProgram();
   	glAttachShader(program, frag);
   	glAttachShader(program, vert);
   	glLinkProgram(program);

   	glGetProgramiv(program, GL_LINK_STATUS, &status);
   	if (!status)
   	{
      		char log[1000];
      		GLsizei len;
      		glGetProgramInfoLog(program, 1000, &len, log);
      		fprintf(stderr, "Error: linking:\n%*s\n", len, log);
      		exit(1);
   	}

        glUseProgram(program);
	glBindAttribLocation(program, 3, "in_position");
	glBindAttribLocation(program, 4, "in_tex_coord");
	
	glLinkProgram(program);	
	{
	textureUniformY = glGetUniformLocation(program, "tex_my");
	glUniform1i(textureUniformY, 0);
	}
	
	//{
		//viewmatrix();
		//translation(ModelViewProjectionMatrix, 0.0, 0.0, 0.0);
		//matrix_rmultiply(ViewMatrix, ModelViewProjectionMatrix);
	
		//matrix_rmultiply(ProjectionMatrix, ModelViewProjectionMatrix);
		//glUniformMatrix4fv(mvp_matrix_loc, 1, GL_FALSE, ModelViewProjectionMatrix);
	//}

	{
         GLuint vbo;

         float quad_data[24] = {
        	 -1.f, -1.f,0.f,1.f,
        	  1.f, -1.f,0.f,1.f,
         	 -1.0f,  1.f,0.f,1.f,
          	  1.0f,  1.f,0.f,1.f,
		//texture
		  0.f, 1.f,
                  1.f, 1.f,
                  0.f, 0.f,
                  1.f, 0.f 
	};
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(quad_data), quad_data, GL_STATIC_DRAW);
	glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, 0, (GLvoid*)0);
	glEnableVertexAttribArray(3);
	glVertexAttribPointer(4, 2, GL_FLOAT, GL_FALSE, 0,(GLvoid*)(16 * sizeof(float)));
	glEnableVertexAttribArray(4);
        }  

	loadYUV();

	converted = (unsigned char *)malloc(800 * 480 * 4);
	part_1 = (unsigned char *)malloc(800 * 480 * 2);
	part_2 = (unsigned char *)malloc(800 * 480 * 2);
	
}


/**

  Creates a perspective matrix as per OpenGL definition of glFrustum.

 **/

static void frustum(GLfloat left, GLfloat right, GLfloat bottom, GLfloat top,
          GLfloat znear, GLfloat zfar)
{
	ProjectionMatrix[0] = 2.0f * znear / (right - left);
   	ProjectionMatrix[1] = 0.0f;
   	ProjectionMatrix[2] = 0.0f;
   	ProjectionMatrix[3] = 0.0f;
   	ProjectionMatrix[4] = 0.0f;
   	ProjectionMatrix[5] = 2.0f * znear / (top - bottom);
   	ProjectionMatrix[6] = 0.0f;
   	ProjectionMatrix[7] = 0.0f;
   	ProjectionMatrix[8] = (right + left) / (right - left);
   	ProjectionMatrix[9] = (top + bottom) / (top - bottom);
   	ProjectionMatrix[10] = -(zfar + znear) / (zfar - znear);
   	ProjectionMatrix[11] = -1.0f;
   	ProjectionMatrix[12] = 0.0f;
   	ProjectionMatrix[13] = 0.0f;
   	ProjectionMatrix[14] = -2.0f * zfar * znear / (zfar - znear);
  	ProjectionMatrix[15] = 0.0f;
}


/**

  Stores the multiplication of matrix A and B into matrix B.
  The result is also the return value to allow cascading matrix operations.

 **/

static GLfloat *matrix_rmultiply(const GLfloat * A, GLfloat * B)
{
   	GLfloat T[3];

   	T[0] = B[0];
   	T[1] = B[1];
   	T[2] = B[2];

   	B[0] = A[0] * B[0] + A[4] * B[1] + A[8] * B[2] + A[12] * B[3];
   	B[1] = A[1] * T[0] + A[5] * B[1] + A[9] * B[2] + A[13] * B[3];
   	B[2] = A[2] * T[0] + A[6] * T[1] + A[10] * B[2] + A[14] * B[3];
   	B[3] = A[3] * T[0] + A[7] * T[1] + A[11] * T[2] + A[15] * B[3];

   	T[0] = B[4];
   	T[1] = B[5];
   	T[2] = B[6];
	
   	B[4] = A[0] * B[4] + A[4] * B[5] + A[8] * B[6] + A[12] * B[7];
   	B[5] = A[1] * T[0] + A[5] * B[5] + A[9] * B[6] + A[13] * B[7];
   	B[6] = A[2] * T[0] + A[6] * T[1] + A[10] * B[6] + A[14] * B[7];
   	B[7] = A[3] * T[0] + A[7] * T[1] + A[11] * T[2] + A[15] * B[7];

   	T[0] = B[8];
   	T[1] = B[9];
   	T[2] = B[10];

   	B[8] = A[0] * B[8] + A[4] * B[9] + A[8] * B[10] + A[12] * B[11];
   	B[9] = A[1] * T[0] + A[5] * B[9] + A[9] * B[10] + A[13] * B[11];
   	B[10] = A[2] * T[0] + A[6] * T[1] + A[10] * B[10] + A[14] * B[11];
   	B[11] = A[3] * T[0] + A[7] * T[1] + A[11] * T[2] + A[15] * B[11];

   	T[0] = B[12];
   	T[1] = B[13];
   	T[2] = B[14];

   	B[12] = A[0] * B[12] + A[4] * B[13] + A[8] * B[14] + A[12] * B[15];
   	B[13] = A[1] * T[0] + A[5] * B[13] + A[9] * B[14] + A[13] * B[15];
   	B[14] = A[2] * T[0] + A[6] * T[1] + A[10] * B[14] + A[14] * B[15];
   	B[15] = A[3] * T[0] + A[7] * T[1] + A[11] * T[2] + A[15] * B[15];

   	return B;
}


/**

  Equivalent to matrix_multiply(A, rotation_z(tmp, angle)).
  This is faster because two rows remain unchanged by the rotation in matrix A.

 **/

static GLfloat *rotate_z(GLfloat * A, float angle)
{
   	GLfloat cosa = cos(angle);
   	GLfloat sina = sin(angle);
   	GLfloat T[4];

   	T[0] = A[0];
   	T[1] = A[1];
   	T[2] = A[2];
   	T[3] = A[3];

   	A[0] = T[0] * cosa + A[4] * sina;
   	A[1] = T[1] * cosa + A[5] * sina;
   	A[2] = T[2] * cosa + A[6] * sina;
   	A[3] = T[3] * cosa + A[7] * sina;

   	A[4] = A[4] * cosa - T[0] * sina;
   	A[5] = A[5] * cosa - T[1] * sina;
   	A[6] = A[6] * cosa - T[2] * sina;
   	A[7] = A[7] * cosa - T[3] * sina;

   	return A;
}

/**

  Creates a translation matrix as per OpenGL definition of Translatef.

 **/

static GLfloat *translation(GLfloat * M, float x, float y, float z)
{
   	M[0] = Identity[0];
   	M[1] = Identity[1];
   	M[2] = Identity[2];
   	M[3] = Identity[3];
   	M[4] = Identity[4];
   	M[5] = Identity[5];
   	M[6] = Identity[6];
   	M[7] = Identity[7];
   	M[8] = Identity[8];
   	M[9] = Identity[9];
   	M[10] = Identity[10];
   	M[11] = Identity[11];
   	M[12] = x;
   	M[13] = y;
   	M[14] = z;
   	M[15] = 1.0f;
   	return M;
}


/**

  Creates a view matrix that represents the camera's position and orientation.
  Unlike the modelview matrix, which might change for each draw call, the
  view matrix can only have one value per iteration. In this demo, the view matrix
  only changes when the user hits the arrow keys or the 'z' and 'Z' keys.

 **/

static void
handle_ivi_surface_configure(void *data, struct ivi_surface *ivi_surface,
           int32_t width, int32_t height)
{
  /* Simple-shm is resizable */
}

static const struct ivi_surface_listener ivi_surface_listener = {
  handle_ivi_surface_configure,
};

static void viewmatrix(void)
{
   	translation(ViewMatrix, 0.0f, 0.0f, -5.00001f);
}


/// Creates a wayland, native EGL and EGL surface.
/** @param window Structure that holds all the window data. */
static void createSurface(struct SWindowData *window)
{
   /// Creates a wayland surface.
	window->surface = wl_compositor_create_surface(window->compositor);

   /// Gets the shell surface for the newly created wayland surface.
	window->shell_surface = wl_shell_get_shell_surface(window->shell, window->surface);

	wl_shell_surface_set_title(window->shell_surface, "camera_test");
   /// Makes the shell surface top-level.
	wl_shell_surface_set_toplevel(window->shell_surface);

   /// Creates a EGL native window for the wayland surface.
	window->native = wl_egl_window_create(window->surface,800,480);

   /// Creates an EGL window surface.
	CHECK_EGL(( window->eglSurface = eglCreateWindowSurface(window->eglDisplay, window->eglConfig, window->native, NULL )) != EGL_NO_SURFACE);


#if 1
  if (window->ivi_application) {
    uint32_t id_ivisurf = 9001;
    window->ivi_surface = ivi_application_surface_create(window->ivi_application,
                   id_ivisurf, window->surface);

    if (window->ivi_surface == NULL) {
      //fprintf(stderr, "Failed to create ivi_client_surface\n");
      printf("ivi_surface == null!\n");
      abort();
    }

    ivi_surface_add_listener(window->ivi_surface,
           &ivi_surface_listener, window);
  }
#endif


   /// Makes the EGL surface current for rendering.
	CHECK_EGL(eglMakeCurrent(window->eglDisplay, window->eglSurface, window->eglSurface, window->eglContext));
}

/// Destroys the wayland surface.
/** @param window Structure that holds all the window data. */
static void destroySurface(struct SWindowData *window)
{
   /// Destroys the EGL native window.
	wl_egl_window_destroy(window->native);

   /// Destroy the shell surface.
	wl_shell_surface_destroy(window->shell_surface);

   /// Destroy the wayland surface.
	wl_surface_destroy(window->surface);

	if (window->callback)
	{
      /// Destroy callback, if exists.
		wl_callback_destroy(window->callback);
   	}
}

/// Forward declaration of the frame listener callback.
static const struct wl_callback_listener frameListener;

/// Function, called by the frame listener callback to render one frame.
/** @param data Structure that holds all the window data.
 *  @param callback The callback of this function call.
 *  @param Time in milliseconds. */



void sclip_data(int convert_x,int convert_y,int convert_width,int convert_height,unsigned char *converted,unsigned char *convert_d)
{	
	unsigned char image[480][800 * 4];
	int i,j,h,k;

	for(i = 0; i < 480; i++)
	{
		if(i%2 == 0)
		{
			for(j = 0;j < 800 * 4; j++)
			{
				image[i][j] = converted[i * 800 * 4 + j];
			}

		}else
		{
			for(j = 0,h = (800 * 4 - 1);j < 800 * 4 ; j++,h--)
			{
				image[i][h] = converted[i * 800 * 4 + j];
			}

		}
	}

	for(i = convert_y ,k = 0; i < convert_height + convert_y; i++,k++)
	{
		if(k%2 == 0)
		{
			for(j = convert_x * 4,h = 0; j < (convert_width + convert_x) * 4; j++,h++)
			{
				convert_d[k * convert_width * 4 + h] = image[i][j];
			}

		}else
		{
			for(j = (convert_x + convert_width)  * 4 -1,h = 0; j < convert_x * 4; j--,h++)
			{
				convert_d[k * convert_width * 4 + h] = image[i][j];
			}

		}

	}

}



	



static void redraw(void *data, struct wl_callback *callback, uint32_t time)
{
  printf("current thread id %d\n", pthread_self());

     	struct SWindowData *window = data;
	int pixel_w;
        int pixel_h;

		(void)pixel_h;
		(void)pixel_w;

        pixel_w = 800;
        pixel_h = 480;
	
	if (window)
	{

	memcpy(window->plane[0],window->buffer,800 * 480 * 3 /2);
	window->plane[1] = window->plane[0] + 800 * 480;
	window->plane[2] = window->plane[1] + 800 * 480 / 4;
	
	}
	yuv420_2_rgb8888(converted,window->plane[0],window->plane[1],window->plane[2],800,480);
	glClearColor(0.0,0.0,0.0,1.0);
   	glClear(GL_COLOR_BUFFER_BIT);
   	 
	//glViewport(0,0,800,480);

        glActiveTexture(GL_TEXTURE0); 
        glBindTexture(GL_TEXTURE_2D, texYId);
        glTexImage2D(GL_TEXTURE_2D, 0,GL_RGBA, 800, 480, 0,GL_RGBA, GL_UNSIGNED_BYTE,converted);
      	glUniform1i(textureUniformY, 0);	


        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
#if 0
	sclip_data(convert_x,convert_y,convert_width,convert_height,converted,convert_data);


	//---------------

	 convert_width = 400;
	 convert_height = 240;

	int convert_x_1 = 100;
	int convert_y_1 = 100;
	unsigned char convert_data_2[240 * 400 * 4];	
	sclip_data(convert_x,convert_y,convert_width,convert_height,converted,convert_data_2);

	 

	//---------------

	glEnable(GL_SCISSOR_TEST);
    	glScissor(0,0,convert_width,convert_height);
	glClearColor(0.0,0.0,0.0,1.0);
   	glClear(GL_COLOR_BUFFER_BIT);
   	glDisable(GL_SCISSOR_TEST);
	glViewport(0,0,convert_width,convert_height);

        glActiveTexture(GL_TEXTURE0); 
        glBindTexture(GL_TEXTURE_2D, texYId);
        glTexImage2D(GL_TEXTURE_2D, 0,GL_RGBA, convert_width, convert_height, 0,GL_RGBA, GL_UNSIGNED_BYTE,convert_data);
      	glUniform1i(textureUniformY, 0);	


        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);



	glEnable(GL_SCISSOR_TEST);
    	glScissor(700,200,convert_width,convert_height);
	glClearColor(0.0,0.0,0.0,1.0);
   	glClear(GL_COLOR_BUFFER_BIT);
   	glDisable(GL_SCISSOR_TEST);
	glViewport(700,200,convert_width,convert_height);

        glActiveTexture(GL_TEXTURE0); 
        glBindTexture(GL_TEXTURE_2D, texYId);
        glTexImage2D(GL_TEXTURE_2D, 0,GL_RGBA, convert_width, convert_height, 0,GL_RGBA, GL_UNSIGNED_BYTE,convert_data_2);
      	glUniform1i(textureUniformY, 0);	


        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	
#endif 
#if 0
        memcpy(part_1,converted,800 * 480 * 2);
	memcpy(part_2,(converted + 800 * 480 * 2),800 * 480 * 2);
	

	

        glEnable(GL_SCISSOR_TEST);
    	glScissor(0,0,800,240);
	glClearColor(0.0,0.0,0.0,1.0);
   	glClear(GL_COLOR_BUFFER_BIT);
   	glDisable(GL_SCISSOR_TEST);

    	glViewport(00,00,800,240);

        glActiveTexture(GL_TEXTURE0); 
        glBindTexture(GL_TEXTURE_2D, texYId);
        glTexImage2D(GL_TEXTURE_2D, 0,GL_RGBA, 800, 240, 0,GL_RGBA, GL_UNSIGNED_BYTE,part_1);
      	glUniform1i(textureUniformY, 0);	


        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
		
			
	glEnable(GL_SCISSOR_TEST);
    	glScissor(900,100,800,240);
	glClearColor(0.0,0.0,0.0,1.0);
   	glClear(GL_COLOR_BUFFER_BIT);
   	glDisable(GL_SCISSOR_TEST);

    	glViewport(900,100,800,240);

        glActiveTexture(GL_TEXTURE0); 
        glBindTexture(GL_TEXTURE_2D, texYId);
        glTexImage2D(GL_TEXTURE_2D, 0,GL_RGBA, 800, 240, 0,GL_RGBA, GL_UNSIGNED_BYTE,part_2);
      	glUniform1i(textureUniformY, 0);	


        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
#endif

	if (callback)
	{
        	wl_callback_destroy(callback);
	}


	/// Create new frame callback to render next frame.
	window->callback = wl_surface_frame(window->surface);
	/// Set listener of callback to this function.
	wl_callback_add_listener(window->callback, &frameListener, window);
	/// Swap buffers to finish frame.
	eglSwapBuffers(window->eglDisplay, window->eglSurface);
	//check_events(window->display);
	//free(convert_data);	

}

/// Frame listener callback, that calls the redraw() method above.
static const struct wl_callback_listener frameListener = { redraw };




/// Retrieves the wayland compositor and shell.
/** @param data Structure that holds all the window data.
 *  @param registry The wayland registry.
 *  @param id Identifier of the object.
 *  @param interface Name of the interface.
 *  @param version Wayland version of the object. */
static void registryHandleGlobal(void *data, struct wl_registry *registry, uint32_t id, const char *interface,
      uint32_t version)
{
	struct SWindowData *d = data;

	if (strcmp(interface, "wl_compositor") == 0)
	{
		d->compositor = wl_registry_bind(registry, id, &wl_compositor_interface, 1);
	}
	else if (strcmp(interface, "wl_shell") == 0)
	{
		d->shell = wl_registry_bind(registry, id, &wl_shell_interface, 1);
	}
  else if (strcmp(interface, "ivi_application") == 0) {
    d->ivi_application =
      wl_registry_bind(registry, id,
           &ivi_application_interface, 1);
  }
}

/// Handle removal of global wayland objects.
/** @param data Structure that holds all the window data.
 *  @param registry The wayland registry.
 *  @param id Identifier of the object. */
static void registryHandleGlobalRemove(void *data, struct wl_registry *registry, uint32_t id)
{
}

static const struct wl_registry_listener registryListener = {
	registryHandleGlobal,
	registryHandleGlobalRemove,
};



/// Running flag, cleared by the signal interrupt.
static int running = 1;

/// Signal interrupt.
/** Causes the main rendering loop to terminate. */
static void signalInt(int signum)
{
	running = 0;
}

/// Main function.
/** @param argc Argument count.
 *  @param argv Program arguments. */
void wayland_egl_init(struct SWindowData *window)
{
	struct sigaction sigint;

   /// Size of the wayland window to be created.
	window->width = 800;
	window->height = 480;
	window->buffer = (unsigned char *)malloc(window->width * window->height * 3 / 2);
        memset(window->buffer,0x00,window->width * window->height * 3 / 2);

	printf("wayland_egl_init init panle\n");
	window->plane[0] =(unsigned char *)malloc(window->width * window->height * 3 / 2);
	memset(window->plane[0],0x00,window->width * window->height * 3 / 2);
	window->plane[1] = window->plane[0] + 800 * 480;
	window->plane[2] = window->plane[1] + 800 * 480 / 4;

	printf("wayland_egl_init init panle end\n");
   /// Connect to a Wayland display.
	window->display = wl_display_connect(0);

   /// Create registry to listen for globals.
	window->registry = wl_display_get_registry(window->display);

   /// Hook in function displayHandleGlobal() to get the wayland compositor and shell.
	wl_registry_add_listener(window->registry, &registryListener, window);

   /// Process connection events.
   /** Results in displayHandleGlobal() calls, so that the compositor and shell are retrieved and
    *  stored in the structure window. */
	wl_display_roundtrip(window->display);

   /// Init EGL.
	initEGL(window);

   /// Create the wayland and EGL surface and make it current for rendering.
	createSurface(window);
	printf("wayland_egl_init createSurface\n");
   /// Init GL for rendering.
	initGL(window);


	eglSwapInterval(window->eglDisplay, 0);
   /// Install signal handler for exiting application.
	sigint.sa_handler = signalInt;
	sigemptyset(&sigint.sa_mask);
	sigint.sa_flags = SA_RESETHAND;
	sigaction(SIGINT, &sigint, NULL );
	wl_display_roundtrip(window->display);
   
   /// Render first frame and install callback for rendering the next frame.
	redraw(window, NULL, 0);

}

int count = 0;
bool
wayland_queue_buffer(struct SWindowData *window, void *buff)
{

	if (!window)
	{
		return false;
	}

      

        memcpy(window->buffer, buff, 800 * 480 * 3 / 2);
        window->buffer_empty = false;

        return true;
}

void check_events(struct wl_display* display)
{
	struct pollfd pfd;

	pfd.fd = wl_display_get_fd(display);
	pfd.events = POLLIN;
	pfd.revents = 0;

	wl_display_dispatch_pending(display);
	wl_display_flush(display);

	if (poll(&pfd, 1, 0) != -1) {
		if (pfd.revents & POLLIN) {
		wl_display_dispatch(display);
		}	
	}
}

int
wayland_dispatch_event(struct SWindowData *window)
{

        int ret= -1;
			
	if(!window)
	{
		printf("window is null\n");
		return -1;

	}
        ret = wl_display_dispatch(window->display);




        return ret;
}
void close_egl_all(struct SWindowData *window)
{
	destroySurface(window);
	closeEGL(window);
	if (window->shell)
	{
      /// Destroys the wayland shell object.
		wl_shell_destroy(window->shell);
   	}

	
	if(window->buffer)
		free(window->buffer);

	if (window->compositor)
   	{
      /// Destroys the wayland compositor object.
		wl_compositor_destroy(window->compositor);
   	}

	wl_registry_destroy(window->registry);

   /// Flush out buffered data.
	wl_display_flush(window->display);

   /// Disconnect from the wayland display connection.
	wl_display_disconnect(window->display);

}


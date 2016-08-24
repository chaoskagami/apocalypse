#include "config.h"

// Types.
typedef __GLXextFuncPtr (*glXGetProcAddressARB_t) (const GLubyte*);
typedef __GLXextFuncPtr (*glXSwapBuffers_t)(Display* dpy, GLXDrawable drawable);
typedef GLXContext (*glXCreateContext_t)(Display *dpy, XVisualInfo *vis, GLXContext shareList, Bool direct);

// Function hooks.
static glXSwapBuffers_t       glXSwapBuffers_REAL       = NULL;
static glXGetProcAddressARB_t glXGetProcAddressARB_REAL = NULL;
static glXCreateContext_t     glXCreateContext_REAL     = NULL;

void pre_hook()  {
/*	char speed[64];
	speed[0] = '\0';
	if (ENABLE_TIME_HACKS)
		snprintf(speed, 64, "speedhack: %02lfx", SLOW_FACTOR);
	uint8_t* buf = string_to_buf(speed);
	glDrawPixels(8*strlen(speed), 8, GL_RGBA, GL_UNSIGNED_BYTE, buf);
	free(buf); */
}

void post_hook() {}

GLXContext   hook_context;
Display*     hook_display;
XVisualInfo* hook_visinfo;

// glXSwapBuffers hook.
void glXSwapBuffers_fake(Display* dpy, GLXDrawable drawable) {
	pre_hook();
	glXSwapBuffers_REAL(dpy, drawable);
	post_hook();
}

// glXCreateContext hook.
GLXContext glXCreateContext_fake( Display *dpy, XVisualInfo *vis, GLXContext shareList, Bool direct ) {
	hook_context = glXCreateContext_REAL(dpy, vis, shareList, direct);
	hook_display = dpy;
	hook_visinfo = vis;

	fprintf(stderr, "[HOOK] Obtained GL context pointers successfully.\n");

	return hook_context;
}

// Same-ish thing, thin shim.
__GLXextFuncPtr glXGetProcAddress (const GLubyte* procName) {
	return glXGetProcAddressARB(procName);
}

// glXGetProcAddressARB
__GLXextFuncPtr glXGetProcAddressARB (const GLubyte* procName)
{
	__GLXextFuncPtr result;

	// Fetch pointer of actual glXGetProcAddressARB() function
	if(!glXGetProcAddressARB_REAL) {
		char* errorstr;
		fprintf(stderr, "[HOOK] Overrided glXGetProcAddress.\n");
		glXGetProcAddressARB_REAL = (glXGetProcAddressARB_t) dlsym(RTLD_NEXT, "glXGetProcAddressARB");
		if((errorstr = dlerror())) {
			fprintf(stderr, "dlsym fail: %s\n", errorstr);
			exit(1);
		}
	}

	// Return our own function pointers for things.
	if (!strcmp( (const char*) procName, "glXSwapBuffers" )) {
		fprintf(stderr, "[HOOK] Hooked glXSwapBuffers pointer.\n");

		glXSwapBuffers_REAL = (glXSwapBuffers_t) glXGetProcAddressARB_REAL(procName);
		return (__GLXextFuncPtr) glXSwapBuffers_fake;
	} else if (!strcmp( (const char*) procName, "glXCreateContext" )) {
		fprintf(stderr, "[HOOK] Hooked glXCreateContext pointer.\n");

		glXCreateContext_REAL = (glXCreateContext_t) glXGetProcAddressARB_REAL(procName);
		return (__GLXextFuncPtr) glXCreateContext_fake;
	}

	// Return default function pointer
	return glXGetProcAddressARB_REAL(procName);
}

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>
#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>

#include <wayland-client.h>

#include "single-pixel-buffer-v1.h"
#include "wlr-layer-shell-unstable-v1.h"
#include "wlr-virtual-pointer-unstable-v1.h"
#include "viewporter.h"

bool running = true;

struct wl_display *display = NULL;
struct wl_registry *registry = NULL;
struct wl_compositor *compositor = NULL;
struct wl_surface *surface = NULL;
struct zwlr_layer_shell_v1 *layer_shell = NULL;
struct wp_viewporter *viewporter = NULL;
struct wp_single_pixel_buffer_manager_v1 *single_pixel_buffer_manager = NULL;
struct zwlr_virtual_pointer_manager_v1 *virtual_pointer_manager = NULL;
struct zwlr_virtual_pointer_v1 *virtual_pointer = NULL;
struct wl_pointer *pointer = NULL;

struct wl_output *output = NULL;

struct wl_seat *seat = NULL;
struct zwlr_layer_surface_v1 *layer_surface = NULL;
struct wp_viewport *viewport = NULL;
struct wl_shm *shm = NULL;
struct wl_buffer *shm_buffer = NULL;

struct wl_callback *frame_callback = NULL;

int surface_width = 0;
int surface_height = 0;

int64_t start_ms = 0;
int64_t delay_ms = 0;

static int64_t now_ms(void) {
  struct timespec ts = {0};
  if (clock_gettime(CLOCK_MONOTONIC, &ts) != 0) {
    perror("clock_gettime() failed");
    exit(1);
  }
  return (int64_t)ts.tv_sec * 1000 + ts.tv_nsec / 1000000;
}

void
noop()
{
  /* intentionally left blank */
}

void
xdg_output_handle_name(void *data, struct wl_output *wl_output, const char *name)
{
	printf("%s\n", name);
}

void
xdg_output_handle_done(void *data, struct wl_output *wl_output)
{
}

static void
randname(char *buf)
{
  struct timespec ts;
  clock_gettime(CLOCK_REALTIME, &ts);
  long r = ts.tv_nsec;
  for (int i = 0; i < 6; ++i) {
    buf[i] = 'A'+(r&15)+(r&16)*2;
    r >>= 5;
  }
}

static int
create_shm_file(void)
{
  int retries = 100;
  do {
    char name[] = "/wl_shm-XXXXXX";
    randname(name + sizeof(name) - 7);
    --retries;
    int fd = shm_open(name, O_RDWR | O_CREAT | O_EXCL, 0600);
    if (fd >= 0) {
      shm_unlink(name);
      return fd;
    }
  } while (retries > 0 && errno == EEXIST);
  return -1;
}

int
allocate_shm_file(size_t size)
{
  int fd = create_shm_file();
  if (fd < 0)
    return -1;
  int ret;
  do {
    ret = ftruncate(fd, size);
  } while (ret < 0 && errno == EINTR);
  if (ret < 0) {
    close(fd);
    return -1;
  }
  return fd;
}


static void repaint_output(struct wl_output *output);


static void pointer_handle_enter(void *data, struct wl_pointer *wl_pointer, uint32_t serial, struct wl_surface *surface, wl_fixed_t surface_x, wl_fixed_t surface_y) {

  int cursor_x = wl_fixed_to_int(surface_x);
  int cursor_y = wl_fixed_to_int(surface_y);

  printf("%d %d\n", cursor_x, cursor_y);

  const int width = surface_width, height = surface_height;
  const int stride = width * 4;
  const int shm_pool_size = height * stride * 2;
  
  int fd = allocate_shm_file(shm_pool_size);
  uint8_t *pool_data = mmap(NULL, shm_pool_size,
      PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
  struct wl_shm_pool *pool = wl_shm_create_pool(shm, fd, shm_pool_size);
  int index = 0;
  int offset = height * stride * index;

  shm_buffer = wl_shm_pool_create_buffer(pool, offset, width, height, stride, WL_SHM_FORMAT_ARGB8888);

  uint32_t *pixels = (uint32_t *)&pool_data[offset];


  uint32_t half = (surface_height < surface_width ? surface_height : surface_width)/10;

  for (int y = 0; y < surface_height; y++) {
    if(y < cursor_y - half || y > cursor_y + half)
      continue;
    for (int x = 0; x < surface_width; x++) {
      if(x < cursor_x - half || x > cursor_x + half)
        continue;                                                                                                              uint32_t red = 0xd70000;
        uint32_t green = 0x009900;
        uint32_t blue = 0x000021;
        //not transparent
        uint32_t alpha = 0xff000000;                                                                                           uint32_t color = alpha + red + green + blue;
        pixels[x + (y * surface_width)] = color;
    }
  }

  repaint_output(output);

} 

static void pointer_handle_leave(void *data, struct wl_pointer *wl_pointer, uint32_t serial, struct wl_surface *surface) {
  running = false;
}

static void pointer_handle_motion(void *data, struct wl_pointer *wl_pointer, uint32_t time, wl_fixed_t surface_x, wl_fixed_t surface_y) {
  running = false;
}

static void pointer_handle_button(void *data, struct wl_pointer *wl_pointer, uint32_t serial, uint32_t time, uint32_t button, uint32_t state) {
  running = false;
}

static void pointer_handle_axis(void *data, struct wl_pointer *wl_pointer, uint32_t time, uint32_t axis, wl_fixed_t value) {
  running = false;
}
static const struct wl_pointer_listener pointer_listener = {
  .enter = pointer_handle_enter,
  .leave = pointer_handle_leave,
  .motion = pointer_handle_motion,
  .button = pointer_handle_button,
  .axis = pointer_handle_axis,

};


static void seat_handle_capabilities(void *data, struct wl_seat *wl_seat, uint32_t caps) {

  if (caps & WL_SEAT_CAPABILITY_POINTER) {
    pointer = wl_seat_get_pointer(seat);
    wl_pointer_add_listener(pointer, &pointer_listener, NULL); //FIXME
  }
  virtual_pointer = zwlr_virtual_pointer_manager_v1_create_virtual_pointer(virtual_pointer_manager, seat);
}

static const struct wl_seat_listener seat_listener = {
  .capabilities = seat_handle_capabilities,
};

//==============
// Global
//==============
static void global_registry_handler(void *data, struct wl_registry *registry,
        uint32_t id, const char *interface, uint32_t version)
{
  const static struct wl_output_listener wl_output_listener = {
    .geometry = noop,
    .mode = noop,
    .scale = noop,
    .name = xdg_output_handle_name,
    .description = noop,
    .done = xdg_output_handle_done,
  };

  // printf("Got a registry event for <%s>, id: %d, version: %d.\n", interface, id, version);
  if (strcmp(interface, wl_shm_interface.name) == 0) {
    shm = wl_registry_bind(registry, id, &wl_shm_interface, 1);
  } else if (strcmp(interface, wl_compositor_interface.name) == 0) {
    compositor = wl_registry_bind(registry, id, &wl_compositor_interface, 4);
  } else if (strcmp(interface, zwlr_layer_shell_v1_interface.name) == 0) {
    layer_shell = wl_registry_bind(registry, id, &zwlr_layer_shell_v1_interface, 1);
  } else if (strcmp(interface, wp_viewporter_interface.name) == 0) {
    viewporter = wl_registry_bind(registry, id, &wp_viewporter_interface, 1);
  } else if (strcmp(interface, wp_single_pixel_buffer_manager_v1_interface.name) == 0) {
    single_pixel_buffer_manager = wl_registry_bind(registry, id, &wp_single_pixel_buffer_manager_v1_interface, 1);
  } else if (strcmp(interface, zwlr_virtual_pointer_manager_v1_interface.name) == 0) {
    virtual_pointer_manager = wl_registry_bind(registry, id, &zwlr_virtual_pointer_manager_v1_interface, 1);
  } 
  else if (strcmp(interface, wl_seat_interface.name) == 0) {
    seat = wl_registry_bind(registry, id, &wl_seat_interface, 1);
    wl_seat_add_listener(seat, &seat_listener, NULL);//FIXME.
  }

  // if output is NULL, it will default to focused output, that's what we want.
  /*else if (strcmp(interface, wl_output_interface.name) == 0) {
      wl_registry_bind(registry, id, &wl_output_interface, 4);
      wl_output_add_listener(output, &wl_output_listener, output);
  } */
}

static void global_registry_remove_handler(void *data,
        struct wl_registry *registry, uint32_t id)
{
    //printf("Got a registry losing event for <%d>\n", id);
}

static const struct wl_registry_listener registry_listener = {
    .global = global_registry_handler,
    .global_remove = global_registry_remove_handler,
};


static void frame_callback_handle_done(void *data, struct wl_callback *callback, uint32_t time) {
  assert(callback == frame_callback);
  wl_callback_destroy(callback);
  frame_callback = NULL;
  repaint_output(output);
}

static const struct wl_callback_listener frame_callback_listener = {
  .done = frame_callback_handle_done,
};

static void repaint_output(struct wl_output *output) {
  int64_t delta = now_ms() - start_ms;
  double progress = (double)delta / delay_ms;
  if (progress >= 1) {
    running = false;
  }


  //uint32_t alpha = progress * UINT32_MAX ;
//  struct wl_buffer *buffer = wp_single_pixel_buffer_manager_v1_create_u32_rgba_buffer(single_pixel_buffer_manager, 0, 0, 0, alpha);
  wl_surface_attach(surface, shm_buffer, 0, 0);

  wp_viewport_set_destination(viewport, surface_width, surface_height);
  if (frame_callback != NULL) {
    wl_callback_destroy(frame_callback);
  }

  frame_callback = wl_surface_frame(surface);
  wl_callback_add_listener(frame_callback, &frame_callback_listener, NULL);
  wl_surface_damage(surface, 0, 0, INT32_MAX, INT32_MAX);
  wl_surface_commit(surface);

  //wl_buffer_destroy(buffer);
}


static void layer_surface_handle_configure(void *data, struct zwlr_layer_surface_v1 *layer_surface, uint32_t serial, uint32_t width, uint32_t height) {
  surface_width = width;
  surface_height = height;

  zwlr_layer_surface_v1_ack_configure(layer_surface, serial);
  
  struct wl_buffer *buffer = wp_single_pixel_buffer_manager_v1_create_u32_rgba_buffer(single_pixel_buffer_manager, 0, 0, 0, 0);
  wl_surface_attach(surface, buffer, 0, 0);

  wp_viewport_set_destination(viewport, surface_width, surface_height);
  wl_surface_damage(surface, 0, 0, INT32_MAX, INT32_MAX);
  wl_surface_commit(surface);

  wl_buffer_destroy(buffer);
  zwlr_virtual_pointer_v1_motion(virtual_pointer, 0, 0, 0);
  zwlr_virtual_pointer_v1_frame(virtual_pointer);

//  repaint_output(output);
}

static void layer_surface_handle_closed(void *data, struct zwlr_layer_surface_v1 *surface) {
//
}

static const struct zwlr_layer_surface_v1_listener layer_surface_listener = {
  .configure = layer_surface_handle_configure,
  .closed = layer_surface_handle_closed,
};


int main(int argc, char *argv[])
{
    (void)argc;
    (void)argv;

    display = wl_display_connect(NULL);
    if (display == NULL) {
        exit(1);
    }

    registry = wl_display_get_registry(display);
    wl_registry_add_listener(registry, &registry_listener, NULL);
    
    wl_display_dispatch(display);

    wl_display_roundtrip(display);
    wl_display_roundtrip(display);
    

    struct {
      const char *name;
      bool found;
    } required_globals[] = {
      { "wl_compositor", compositor != NULL },
      { "wl_shm", shm != NULL },
      { "zwlr_layer_shell_v1", layer_shell != NULL },
      { "wp_viewporter", viewporter != NULL },
      { "wp_single_pixel_buffer_manager_v1", single_pixel_buffer_manager != NULL },
      { "zwlr_virtual_pointer_manager_v1", virtual_pointer_manager != NULL },
    };
    for (size_t i = 0; i < sizeof(required_globals) / sizeof(required_globals[0]); i++) {
      if (!required_globals[i].found) {
        fprintf(stderr, "missing %s global\n", required_globals[i].name);
        return 1;
      }
    }

    surface = wl_compositor_create_surface(compositor);
    if (surface == NULL) {
        exit(1);
    }

    layer_surface = zwlr_layer_shell_v1_get_layer_surface(layer_shell, surface, output, ZWLR_LAYER_SHELL_V1_LAYER_OVERLAY, "find-cursor");
    zwlr_layer_surface_v1_add_listener(layer_surface, &layer_surface_listener, NULL); //FIXME.

    viewport = wp_viewporter_get_viewport(viewporter, surface);
    zwlr_layer_surface_v1_set_anchor(layer_surface,
      ZWLR_LAYER_SURFACE_V1_ANCHOR_TOP |
      ZWLR_LAYER_SURFACE_V1_ANCHOR_LEFT |
      ZWLR_LAYER_SURFACE_V1_ANCHOR_RIGHT |
      ZWLR_LAYER_SURFACE_V1_ANCHOR_BOTTOM);
    zwlr_layer_surface_v1_set_keyboard_interactivity(layer_surface, false);
    zwlr_layer_surface_v1_set_exclusive_zone(layer_surface, -1);
    wl_surface_commit(surface);

    delay_ms = 3000;
    start_ms = now_ms();

    // Display loop.
    while (running) {
      if (wl_display_dispatch(display) < 0) {
        break;
      }
    }

    wl_surface_destroy(surface);

    wl_display_disconnect(display);
    return 0;
}
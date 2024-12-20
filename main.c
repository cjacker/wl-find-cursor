#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

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

struct wl_output *output;
struct wl_seat *seat;
struct zwlr_layer_surface_v1 *layer_surface;
struct wp_viewport *viewport;
struct wl_shm *shm;
struct wl_buffer *shm_buffer;

struct wl_callback *frame_callback;

int surface_width;
int surface_height;

bool find_cursor=false;
int cursor_x;
int cursor_y;

static void pointer_handle_enter(void *data, struct wl_pointer *wl_pointer, uint32_t serial, struct wl_surface *surface, wl_fixed_t surface_x, wl_fixed_t surface_y) {
  // No-op
  find_cursor=true;
  cursor_x = wl_fixed_to_int(surface_x);
  cursor_y = wl_fixed_to_int(surface_y);
  if(find_cursor)
    printf("%d %d\n", wl_fixed_to_int(surface_x), wl_fixed_to_int(surface_y));
//  cancel(seat->chayang);
	running = false;
} 

static const struct wl_pointer_listener pointer_listener = {
  .enter = pointer_handle_enter,
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
  } else if (strcmp(interface, wl_output_interface.name) == 0) {
    output = wl_registry_bind(registry, id, &wl_output_interface, 1);
  } else if (strcmp(interface, wl_seat_interface.name) == 0) {
    seat = wl_registry_bind(registry, id, &wl_seat_interface, 1);
    wl_seat_add_listener(seat, &seat_listener, NULL);//FIXME.
  }
   

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


static void repaint_output(struct wl_output *output) {
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

  //repaint_output(output);
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

#include <nitro.hpp>
#include <wayland-server.h>

class Display {
	struct wl_display* display;
public:
	Display() {
		display = wl_display_create();
		wl_display_add_socket_auto(display);
	}
	Display(const Display&) = delete;
	~Display() {
		wl_display_destroy(display);
	}
	Display& operator =(const Display&) = delete;
	int get_fd() {
		return wl_event_loop_get_fd(wl_display_get_event_loop(display));
	}
	void dispatch() {
		wl_event_loop_dispatch(wl_display_get_event_loop(display), 0);
	}
};

int main() {
	Display display;
	nitro::Window window(800, 600, "air-shell");
	window.run(display);
}

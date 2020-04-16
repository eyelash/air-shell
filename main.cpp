#include <nitro.hpp>
#include <wayland-server.h>

class Display {
	wl_display* display;
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
	template <class G> void create_global(G& global) {
		wl_global_bind_func_t bind = [](wl_client* client, void* data, uint32_t version, uint32_t id) {
			static_cast<G*>(data)->bind(client, version, id);
		};
		wl_global_create(display, G::interface, G::version, &global, bind);
	}
};

template <auto method> class Method;
template <class C, class... A, void (C::*method)(wl_client*, wl_resource*, A...)> class Method<method> {
public:
	static void callback(wl_client* client, wl_resource* resource, A... arguments) {
		C* c = static_cast<C*>(wl_resource_get_user_data(resource));
		(c->*method)(client, resource, arguments...);
	}
};

class Compositor {
public:
	virtual ~Compositor() = default;
	virtual void create_surface(wl_client* client, wl_resource* resource, uint32_t id) {
		// TODO: implement
	}
	virtual void create_region(wl_client* client, wl_resource* resource, uint32_t id) {
		// TODO: implement
	}
	static constexpr const wl_interface* interface = &wl_compositor_interface;
	static constexpr int version = 1;
	void bind(wl_client* client, uint32_t version, uint32_t id) {
		struct wl_compositor_interface implementation = {
			Method<&Compositor::create_surface>::callback,
			Method<&Compositor::create_region>::callback,
		};
		wl_resource* resource = wl_resource_create(client, interface, version, id);
		wl_resource_set_implementation(resource, &implementation, this, nullptr);
	}
};

int main() {
	Display display;
	Compositor compositor;
	display.create_global(compositor);
	nitro::WindowX11 window(800, 600, "air-shell");
	window.run(display);
}

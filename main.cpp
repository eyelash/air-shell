#include <nitro.hpp>
#include <wayland-server.h>
#include <xdg-shell.h>

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
		wl_display_flush_clients(display);
	}
	template <class G> void create_global(G& global) {
		wl_global_bind_func_t bind = [](wl_client* client, void* data, uint32_t version, uint32_t id) {
			static_cast<G*>(data)->bind(client, version, id);
		};
		wl_global_create(display, G::get_interface(), G::get_version(), &global, bind);
	}
	void init_shm() {
		wl_display_init_shm(display);
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

class Surface {
	wl_resource* resource;
	wl_resource* buffer;
public:
	Surface(wl_resource* resource = nullptr): resource(resource), buffer(nullptr) {}
	void destroy(wl_client* client, wl_resource* resource) {
		// TODO: implement
	}
	void attach(wl_client* client, wl_resource* resource, wl_resource* buffer, int32_t x, int32_t y) {
		this->buffer = buffer;
	}
	void damage(wl_client* client, wl_resource* resource, int32_t x, int32_t y, int32_t width, int32_t height) {
		// TODO: implement
	}
	void frame(wl_client* client, wl_resource* resource, uint32_t callback) {
		// TODO: implement
	}
	void set_opaque_region(wl_client* client, wl_resource* resource, wl_resource* region) {
		// TODO: implement
	}
	void set_input_region(wl_client* client, wl_resource* resource, wl_resource* region) {
		// TODO: implement
	}
	void commit(wl_client* client, wl_resource* resource) {
		if (wl_shm_buffer* b = wl_shm_buffer_get(buffer)) {
			const int32_t width = wl_shm_buffer_get_width(b);
			const int32_t height = wl_shm_buffer_get_height(b);
			void* data = wl_shm_buffer_get_data(b);
		}
	}
	void set_buffer_transform(wl_client* client, wl_resource* resource, int32_t transform) {
		// TODO: implement
	}
	void set_buffer_scale(wl_client* client, wl_resource* resource, int32_t scale) {
		// TODO: implement
	}
	void damage_buffer(wl_client* client, wl_resource* resource, int32_t x, int32_t y, int32_t width, int32_t height) {
		// TODO: implement
	}
	static constexpr const wl_interface* get_interface() {
		return &wl_surface_interface;
	}
	static constexpr int get_version() {
		return 4;
	}
	static const void* get_implementation() {
		static struct wl_surface_interface implementation = {
			Method<&Surface::destroy>::callback,
			Method<&Surface::attach>::callback,
			Method<&Surface::damage>::callback,
			Method<&Surface::frame>::callback,
			Method<&Surface::set_opaque_region>::callback,
			Method<&Surface::set_input_region>::callback,
			Method<&Surface::commit>::callback,
			Method<&Surface::set_buffer_transform>::callback,
			Method<&Surface::set_buffer_scale>::callback,
			Method<&Surface::damage_buffer>::callback,
		};
		return &implementation;
	}
	void bind(wl_client* client, uint32_t version, uint32_t id) {
		resource = wl_resource_create(client, get_interface(), version, id);
		wl_resource_set_implementation(resource, get_implementation(), this, nullptr);
	}
};

class Region {
	wl_resource* resource;
public:
	Region(wl_resource* resource = nullptr): resource(resource) {}
	void destroy(wl_client* client, wl_resource* resource) {
		// TODO: implement
	}
	void add(wl_client* client, wl_resource* resource, int32_t x, int32_t y, int32_t width, int32_t height) {
		// TODO: implement
	}
	void subtract(wl_client* client, wl_resource* resource, int32_t x, int32_t y, int32_t width, int32_t height) {
		// TODO: implement
	}
	static constexpr const wl_interface* get_interface() {
		return &wl_region_interface;
	}
	static constexpr int get_version() {
		return 1;
	}
	static const void* get_implementation() {
		static struct wl_region_interface implementation = {
			Method<&Region::destroy>::callback,
			Method<&Region::add>::callback,
			Method<&Region::subtract>::callback,
		};
		return &implementation;
	}
	void bind(wl_client* client, uint32_t version, uint32_t id) {
		resource = wl_resource_create(client, get_interface(), version, id);
		wl_resource_set_implementation(resource, get_implementation(), this, nullptr);
	}
};

class Compositor {
public:
	virtual ~Compositor() = default;
	virtual void create_surface(wl_client* client, wl_resource* resource, uint32_t id) {
		Surface* surface = new Surface();
		surface->bind(client, Surface::get_version(), id);
	}
	virtual void create_region(wl_client* client, wl_resource* resource, uint32_t id) {
		Region* region = new Region();
		region->bind(client, Region::get_version(), id);
	}
	static constexpr const wl_interface* get_interface() {
		return &wl_compositor_interface;
	}
	static constexpr int get_version() {
		return 1;
	}
	static const void* get_implementation() {
		static struct wl_compositor_interface implementation = {
			Method<&Compositor::create_surface>::callback,
			Method<&Compositor::create_region>::callback,
		};
		return &implementation;
	}
	void bind(wl_client* client, uint32_t version, uint32_t id) {
		wl_resource* resource = wl_resource_create(client, get_interface(), version, id);
		wl_resource_set_implementation(resource, get_implementation(), this, nullptr);
	}
};

class XdgToplevel {
	wl_resource* resource;
	void destroy(wl_client* client, wl_resource* resource) {
		// TODO: implement
	}
	void set_parent(wl_client* client, wl_resource* resource, wl_resource* parent) {
		// TODO: implement
	}
	void set_title(wl_client* client, wl_resource* resource, const char* title) {
		// TODO: implement
	}
	void set_app_id(wl_client* client, wl_resource* resource, const char* app_id) {
		// TODO: implement
	}
	void show_window_menu(wl_client* client, wl_resource* resource, wl_resource* seat, uint32_t serial, int32_t x, int32_t y) {
		// TODO: implement
	}
	void move(wl_client* client, wl_resource* resource, wl_resource* seat, uint32_t serial) {
		// TODO: implement
	}
	void resize(wl_client* client, wl_resource* resource, wl_resource* seat, uint32_t serial, uint32_t edges) {
		// TODO: implement
	}
	void set_max_size(wl_client* client, wl_resource* resource, int32_t width, int32_t height) {
		// TODO: implement
	}
	void set_min_size(wl_client* client, wl_resource* resource, int32_t width, int32_t height) {
		// TODO: implement
	}
	void set_maximized(wl_client* client, wl_resource* resource) {
		// TODO: implement
	}
	void unset_maximized(wl_client* client, wl_resource* resource) {
		// TODO: implement
	}
	void set_fullscreen(wl_client* client, wl_resource* resource, wl_resource* output) {
		// TODO: implement
	}
	void unset_fullscreen(wl_client* client, wl_resource* resource) {
		// TODO: implement
	}
	void set_minimized(wl_client* client, wl_resource* resource) {
		// TODO: implement
	}
public:
	XdgToplevel(wl_resource* resource = nullptr): resource(resource) {}
	static constexpr const wl_interface* get_interface() {
		return &xdg_toplevel_interface;
	}
	static constexpr int get_version() {
		return 1;
	}
	static const void* get_implementation() {
		static struct xdg_toplevel_interface implementation = {
			Method<&XdgToplevel::destroy>::callback,
			Method<&XdgToplevel::set_parent>::callback,
			Method<&XdgToplevel::set_title>::callback,
			Method<&XdgToplevel::set_app_id>::callback,
			Method<&XdgToplevel::show_window_menu>::callback,
			Method<&XdgToplevel::move>::callback,
			Method<&XdgToplevel::resize>::callback,
			Method<&XdgToplevel::set_max_size>::callback,
			Method<&XdgToplevel::set_min_size>::callback,
			Method<&XdgToplevel::set_maximized>::callback,
			Method<&XdgToplevel::unset_maximized>::callback,
			Method<&XdgToplevel::set_fullscreen>::callback,
			Method<&XdgToplevel::unset_fullscreen>::callback,
			Method<&XdgToplevel::set_minimized>::callback,
		};
		return &implementation;
	}
	void bind(wl_client* client, uint32_t version, uint32_t id) {
		resource = wl_resource_create(client, get_interface(), version, id);
		wl_resource_set_implementation(resource, get_implementation(), this, nullptr);
	}
};

class XdgSurface {
	wl_resource* resource;
	void destroy(wl_client* client, wl_resource* resource) {
		// TODO: implement
	}
	void get_toplevel(wl_client* client, wl_resource* resource, uint32_t id) {
		XdgToplevel* toplevel = new XdgToplevel();
		toplevel->bind(client, XdgToplevel::get_version(), id);
	}
	void get_popup(wl_client* client, wl_resource* resource, uint32_t id, wl_resource* parent, wl_resource* positioner) {
		// TODO: implement
	}
	void set_window_geometry(wl_client* client, wl_resource* resource, int32_t x, int32_t y, int32_t width, int32_t height) {
		// TODO: implement
	}
	void ack_configure(wl_client* client, wl_resource* resource, uint32_t serial) {
		// TODO: implement
	}
public:
	XdgSurface(wl_resource* resource = nullptr): resource(resource) {}
	static constexpr const wl_interface* get_interface() {
		return &xdg_surface_interface;
	}
	static constexpr int get_version() {
		return 1;
	}
	static const void* get_implementation() {
		static struct xdg_surface_interface implementation = {
			Method<&XdgSurface::destroy>::callback,
			Method<&XdgSurface::get_toplevel>::callback,
			Method<&XdgSurface::get_popup>::callback,
			Method<&XdgSurface::set_window_geometry>::callback,
			Method<&XdgSurface::ack_configure>::callback,
		};
		return &implementation;
	}
	void bind(wl_client* client, uint32_t version, uint32_t id) {
		resource = wl_resource_create(client, get_interface(), version, id);
		wl_resource_set_implementation(resource, get_implementation(), this, nullptr);
	}
};

class XdgWmBase {
public:
	void destroy(wl_client* client, wl_resource* resource) {
		// TODO: implement
	}
	void create_positioner(wl_client* client, wl_resource* resource, uint32_t id) {
		// TODO: implement
	}
	void get_xdg_surface(wl_client* client, wl_resource* resource, uint32_t id, wl_resource* surface) {
		XdgSurface* xdg_surface = new XdgSurface();
		xdg_surface->bind(client, XdgSurface::get_version(), id);
	}
	void pong(wl_client* client, wl_resource* resource, uint32_t serial) {
		// TODO: implement
	}
	static constexpr const wl_interface* get_interface() {
		return &xdg_wm_base_interface;
	}
	static constexpr int get_version() {
		return 1;
	}
	static const void* get_implementation() {
		static struct xdg_wm_base_interface implementation = {
			Method<&XdgWmBase::destroy>::callback,
			Method<&XdgWmBase::create_positioner>::callback,
			Method<&XdgWmBase::get_xdg_surface>::callback,
			Method<&XdgWmBase::pong>::callback,
		};
		return &implementation;
	}
	void bind(wl_client* client, uint32_t version, uint32_t id) {
		wl_resource* resource = wl_resource_create(client, get_interface(), version, id);
		wl_resource_set_implementation(resource, get_implementation(), this, nullptr);
	}
};

int main() {
	Display display;
	Compositor compositor;
	XdgWmBase wm_base;
	display.create_global(compositor);
	display.create_global(wm_base);
	display.init_shm();
	nitro::WindowX11 window(800, 600, "air-shell");
	window.run(display);
}

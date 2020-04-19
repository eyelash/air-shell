#include <nitro.hpp>
#include <wayland-server.h>
#include <xdg-shell.h>
#include <utility>
#include <new>

class Shell: public nitro::Node {
	nitro::Node* surface = nullptr;
public:
	nitro::Node* get_child(size_t index) override {
		return index == 0 ? surface : nullptr;
	}
	void layout() override {
		if (surface) {
			surface->set_size(get_width(), get_height());
		}
	}
	void set_surface(nitro::Node* surface) {
		/*if (surface == this->surface) {
			return;
		}*/
		this->surface = surface;
		if (surface) {
			surface->set_parent(this);
		}
		layout();
		request_redraw();
	}
};

Shell* shell = nullptr;

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
			wl_resource* resource = wl_resource_create(client, G::interface, version, id);
			wl_resource_set_implementation(resource, &G::implementation, data, nullptr);
		};
		wl_global_create(display, G::interface, G::version, &global, bind);
	}
	void init_shm() {
		wl_display_init_shm(display);
	}
};

class Array {
	wl_array array;
public:
	Array() {
		wl_array_init(&array);
	}
	Array(const Array&) = delete;
	~Array() {
		wl_array_release(&array);
	}
	Array& operator =(const Array&) = delete;
	operator wl_array*() {
		return &array;
	}
	template <class T> void add(const T& t) {
		new (wl_array_add(&array, sizeof(T))) T(t);
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

class Surface: public nitro::Node {
	nitro::Canvas canvas;
	wl_resource* wl_surface_resource;
	wl_resource* xdg_toplevel_resource;
	wl_resource* xdg_surface_resource;
	wl_resource* buffer;
	wl_resource* frame_callback;
public:
	Surface(wl_resource* resource): wl_surface_resource(resource), xdg_toplevel_resource(nullptr), xdg_surface_resource(nullptr), buffer(nullptr), frame_callback(nullptr) {}
	void draw(const nitro::DrawContext& draw_context) override {
		canvas.draw(draw_context.projection);
		if (frame_callback) {
			wl_callback_send_done(frame_callback, nitro::Animation::get_time() / 1000);
			wl_resource_destroy(frame_callback);
			frame_callback = nullptr;
		}
	}
	// wl_surface
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
		frame_callback = wl_resource_create(client, &wl_callback_interface, 1, callback);
	}
	void set_opaque_region(wl_client* client, wl_resource* resource, wl_resource* region) {
		// TODO: implement
	}
	void set_input_region(wl_client* client, wl_resource* resource, wl_resource* region) {
		// TODO: implement
	}
	void commit(wl_client* client, wl_resource* resource) {
		if (buffer == nullptr) {
			Array states;
			send_configure(0, 0, states);
			return;
		}
		if (wl_shm_buffer* shm_buffer = wl_shm_buffer_get(buffer)) {
			const int32_t width = wl_shm_buffer_get_width(shm_buffer);
			const int32_t height = wl_shm_buffer_get_height(shm_buffer);
			unsigned char* data = static_cast<unsigned char*>(wl_shm_buffer_get_data(shm_buffer));
			canvas.clear();
			canvas.set_texture(0, 0, width, height, nitro::Texture::create_from_data(width, height, 4, data));
			canvas.prepare();
			shell->set_surface(this);
			wl_buffer_send_release(buffer);
			request_redraw();
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
	static constexpr int wl_surface_version = 4;
	static constexpr struct wl_surface_interface wl_surface_implementation = {
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
	// xdg_toplevel
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
	static constexpr int xdg_toplevel_version = 1;
	static constexpr struct xdg_toplevel_interface xdg_toplevel_implementation = {
		Method<&Surface::destroy>::callback,
		Method<&Surface::set_parent>::callback,
		Method<&Surface::set_title>::callback,
		Method<&Surface::set_app_id>::callback,
		Method<&Surface::show_window_menu>::callback,
		Method<&Surface::move>::callback,
		Method<&Surface::resize>::callback,
		Method<&Surface::set_max_size>::callback,
		Method<&Surface::set_min_size>::callback,
		Method<&Surface::set_maximized>::callback,
		Method<&Surface::unset_maximized>::callback,
		Method<&Surface::set_fullscreen>::callback,
		Method<&Surface::unset_fullscreen>::callback,
		Method<&Surface::set_minimized>::callback,
	};
	// xdg_surface
	void get_toplevel(wl_client* client, wl_resource* resource, uint32_t id) {
		xdg_toplevel_resource = wl_resource_create(client, &xdg_toplevel_interface, xdg_toplevel_version, id);
		wl_resource_set_implementation(xdg_toplevel_resource, &xdg_toplevel_implementation, this, nullptr);
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
	static constexpr int xdg_surface_version = 1;
	static constexpr struct xdg_surface_interface xdg_surface_implementation = {
		Method<&Surface::destroy>::callback,
		Method<&Surface::get_toplevel>::callback,
		Method<&Surface::get_popup>::callback,
		Method<&Surface::set_window_geometry>::callback,
		Method<&Surface::ack_configure>::callback,
	};
	void send_configure(int32_t width, int32_t height, wl_array *states) {
		xdg_toplevel_send_configure(xdg_toplevel_resource, width, height, states);
		xdg_surface_send_configure(xdg_surface_resource, 0);
	}
	void create_xdg_surface(wl_client* client, uint32_t id) {
		xdg_surface_resource = wl_resource_create(client, &xdg_surface_interface, xdg_surface_version, id);
		wl_resource_set_implementation(xdg_surface_resource, &xdg_surface_implementation, this, nullptr);
	}
	static void create(wl_client* client, uint32_t id) {
		wl_resource* resource = wl_resource_create(client, &wl_surface_interface, wl_surface_version, id);
		Surface* surface = new Surface(resource);
		wl_resource_destroy_func_t destroy = [](wl_resource* resource) {
			// TODO: implement
		};
		wl_resource_set_implementation(resource, &wl_surface_implementation, surface, destroy);
	}
};

class Region {
	wl_resource* resource;
public:
	Region(wl_resource* resource): resource(resource) {}
	void destroy(wl_client* client, wl_resource* resource) {
		// TODO: implement
	}
	void add(wl_client* client, wl_resource* resource, int32_t x, int32_t y, int32_t width, int32_t height) {
		// TODO: implement
	}
	void subtract(wl_client* client, wl_resource* resource, int32_t x, int32_t y, int32_t width, int32_t height) {
		// TODO: implement
	}
	static constexpr const wl_interface* interface = &wl_region_interface;
	static constexpr int version = 1;
	static constexpr struct wl_region_interface implementation = {
		Method<&Region::destroy>::callback,
		Method<&Region::add>::callback,
		Method<&Region::subtract>::callback,
	};
	static void create(wl_client* client, uint32_t id) {
		wl_resource* resource = wl_resource_create(client, &wl_region_interface, version, id);
		Region* region = new Region(resource);
		wl_resource_destroy_func_t destroy = [](wl_resource* resource) {
			Region* region = static_cast<Region*>(wl_resource_get_user_data(resource));
			delete region;
		};
		wl_resource_set_implementation(resource, &implementation, region, destroy);
	}
};

class Compositor {
public:
	virtual ~Compositor() = default;
	virtual void create_surface(wl_client* client, wl_resource* resource, uint32_t id) {
		Surface::create(client, id);
	}
	virtual void create_region(wl_client* client, wl_resource* resource, uint32_t id) {
		Region::create(client, id);
	}
	static constexpr const wl_interface* interface = &wl_compositor_interface;
	static constexpr int version = 1;
	static constexpr struct wl_compositor_interface implementation = {
		Method<&Compositor::create_surface>::callback,
		Method<&Compositor::create_region>::callback,
	};
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
		static_cast<Surface*>(wl_resource_get_user_data(surface))->create_xdg_surface(client, id);
	}
	void pong(wl_client* client, wl_resource* resource, uint32_t serial) {
		// TODO: implement
	}
	static constexpr const wl_interface* interface = &xdg_wm_base_interface;
	static constexpr int version = 1;
	static constexpr struct xdg_wm_base_interface implementation = {
		Method<&XdgWmBase::destroy>::callback,
		Method<&XdgWmBase::create_positioner>::callback,
		Method<&XdgWmBase::get_xdg_surface>::callback,
		Method<&XdgWmBase::pong>::callback,
	};
};

int main() {
	shell = new Shell();
	Display display;
	Compositor compositor;
	XdgWmBase wm_base;
	display.create_global(compositor);
	display.create_global(wm_base);
	display.init_shm();
	nitro::WindowX11 window(800, 600, "air-shell");
	window.set_child(shell);
	window.run(display);
}

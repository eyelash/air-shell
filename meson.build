project('air-shell', 'cpp', 'c', default_options: ['cpp_std=c++17'])

sources = [
	'main.cpp',
]

dependencies = [
	dependency('nitro', fallback: ['nitro', 'nitro_dep']),
	dependency('wayland-server'),
]

wayland_protocols_dir = dependency('wayland-protocols').get_pkgconfig_variable('pkgdatadir')
protocols = [
	wayland_protocols_dir / 'stable' / 'xdg-shell' / 'xdg-shell.xml',
]

wayland_scanner = find_program('wayland-scanner')
wayland_header_generator = generator(wayland_scanner, output: '@BASENAME@.h', arguments: ['server-header', '@INPUT@', '@OUTPUT@'])
wayland_code_generator = generator(wayland_scanner, output: '@BASENAME@.c', arguments: ['private-code', '@INPUT@', '@OUTPUT@'])
sources += wayland_header_generator.process(protocols)
sources += wayland_code_generator.process(protocols)

executable('air-shell', sources, dependencies: dependencies)

project('air-shell', 'cpp', default_options: ['cpp_std=c++17'])

sources = [
	'main.cpp',
]

dependencies = [
	dependency('nitro', fallback: ['nitro', 'nitro_dep']),
	dependency('wayland-server'),
]

executable('air-shell', sources, dependencies: dependencies)

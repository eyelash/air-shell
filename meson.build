project('air-shell', 'cpp', default_options: ['cpp_std=c++17'])

dependencies = [
	dependency('nitro', fallback: ['nitro', 'nitro_dep']),
	dependency('wayland-server'),
]

executable('air-shell', 'main.cpp', dependencies: dependencies)

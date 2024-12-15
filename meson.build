project(
  'breakout', 'cpp',
  version : '0.1',
  default_options : [
    'warning_level=1',
    'cpp_std=c++14'
  ])

deps = []
deps += dependency('glew', required : true)
deps += dependency('glfw3', required : true)
deps += dependency('freetype2', required : true)

cpp = meson.get_compiler('cpp')
deps += cpp.find_library('SOIL', required: true)

executable(
  'breakout', [
    'ball.cpp',
    'game.cpp',
    'gameobject.cpp',
    'level.cpp',
    'main.cpp',
    'particle.cpp',
    'postprocess.cpp',
    'resource.cpp',
    'shader.cpp',
    'sprite.cpp',
    'text.cpp',
    'texture.cpp',
  ],
  dependencies : deps,
  install : true
)

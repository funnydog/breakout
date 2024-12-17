deps = []
deps += dependency('glew', required : true, fallback : ['glew', 'glew_dep'])
deps += dependency('glfw3', required : true, fallback : ['glfw', 'glfw_dep'])
deps += dependency('glm', required : true, fallback : ['glm', 'glm_dep'])
deps += dependency('freetype2', required : true, fallback : ['freetype2', 'freetype_dep'])

ln = find_program('ln')
asset_link = custom_target(
  'asset-link',
  output : 'assets',
  command : [ln, '-sf', '@SOURCE_ROOT@/assets', '@OUTDIR@/..']
)

executable(
  'breakout', [
    'ball.cpp',
    'batchrenderer.cpp',
    'eventqueue.cpp',
    'font.cpp',
    'game.cpp',
    'gameobject.cpp',
    'glcheck.cpp',
    'level.cpp',
    'main.cpp',
    'particle.cpp',
    'postprocess.cpp',
    'shader.cpp',
    'stb_image.cpp',
    'texture.cpp',
    'utility.cpp',
    asset_link,
  ],
  dependencies : deps,
  install : true
)

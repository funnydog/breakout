## Breakout

Breakout is a clone of the breakout game written in C++20 and using
OpenGL as graphics backend.

The user can choose one of four levels to play, breaking the blocks
can spawn useful or detrimental PowerUPs.

The game is derived from the tutorial at
[LearnOpenGL](https://learnopengl.com/).

## Dependencies

To build the game you need the meson build system and a C++20 compiler
installed on your system.

The game requires the following libraries:

  * GLFW3 - the graphics library
  * glew - for the OpenGL extensions
  * glm - for vectors and matrices
  * freetype2 - to load the fonts

If any of these dependencies are not installed by the system, the
meson build system will download and compile them automatically.
Please note however that they may not be the latest version available.

## Quick start

1. Start the meson configuration:

   ```
   $ meson setup build
   ```

2. Compile the game:

    ```
	$ cd build
	$ ninja
	```

3. Run the game:

   ```
   $ src/breakout
   ```

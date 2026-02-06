# nodepp-gpu: A GPGPU Library for Nodepp

**nodepp-gpu** is a proof-of-concept library that brings the power of General-Purpose GPU (GPGPU) computing to C++ and Nodepp, inspired by the popular JavaScript library [gpu.js](https://gpu.rocks/). It allows you to write C++ functions that are executed on the GPU, leveraging the parallel processing capabilities of modern graphics hardware.

🔗: [Nodepp GPU: Accelerating C++ with GPU and Nodepp](https://medium.com/@EDBCBlog/nodepp-gpu-accelerating-c-with-gpu-and-nodepp-3374bc0a3efb)

## Dependencies
```bash
include(FetchContent)

FetchContent_Declare(
	nodepp
	GIT_REPOSITORY   https://github.com/NodeppOfficial/nodepp
	GIT_TAG          origin/main
	GIT_PROGRESS     ON
)
FetchContent_MakeAvailable(nodepp)

FetchContent_Declare(
	nodepp-raylib
	GIT_REPOSITORY   https://github.com/NodeppOfficial/nodepp-raylib
	GIT_TAG          origin/main
	GIT_PROGRESS     ON
)
FetchContent_MakeAvailable(nodepp-raylib)

FetchContent_Declare(
	nodepp-gpu
	GIT_REPOSITORY   https://github.com/NodeppOfficial/nodepp-gpu
	GIT_TAG          origin/main
	GIT_PROGRESS     ON
)
FetchContent_MakeAvailable(nodepp-gpu)

#[...]

target_link_libraries( #[...]
	PUBLIC nodepp nodepp-raylib nodepp-gpu #[...]
)
```

## Features
* **GPU Kernel Execution**: Write your computational logic in GLSL (OpenGL Shading Language) and have it executed on the GPU.
* **Seamless Integration with Nodepp**: Uses Nodepp's core data structures and concepts for a familiar developer experience.
* **Type-Safe Data Handling**: Provides a `matrix_t` class to represent and manage data, including support for various vector types (`vec2`, `vec3`, `vec4`, `sampler2D`, etc.).
* **Flexible Input/Output**: Easily set input variables and textures for your GPU kernel and retrieve the computed result as a `matrix_t` object.
* **Raylib Integration**: Built on top of the [Raylib](https://www.raylib.com/) library for its OpenGL context management and shader capabilities.
* **Image Processing**: Load images from memory, file paths, or existing textures and use them as input for your GPU computations.

### Simple Example: Matrix Addition

```cpp
#include <nodepp/nodepp.h>
#include <gpu/gpu.h>

using namespace nodepp;

void onMain() { 

    if( !gpu::start_machine() ) 
      { throw except_t("Failed to start GPU machine"); }

    gpu::gpu_t gpu ( GPU_KERNEL(

        vec2 idx = uv / vec2( 2, 2 );

        float color_a = texture( image_a, idx ).x;
        float color_b = texture( image_b, idx ).x;
        float color_c = color_a * color_b;
        
        return vec4( vec3( color_c ), 1. );

    ));

    gpu::matrix_t matrix_a( 2, 2, ptr_t<float>({
        10., 10.,
        10., 10.,
    }) );

    gpu::matrix_t matrix_b( 2, 2, ptr_t<float>({
        .1, .2,
        .4, .3,
    }) );

    gpu.set_output(2, 2, gpu::OUT_DOUBLE4);
    gpu.set_input (matrix_a, "image_a");
    gpu.set_input (matrix_b, "image_b");

    for( auto x: gpu().data() ) 
       { console::log(x); }

    gpu::stop_machine();

}
```

This example demonstrates how to set up the inputs, execute the kernel, and retrieve the result. The gpu_t::operator() handles the entire process of compiling the shader, rendering to a texture, and converting the texture back into a matrix_t object.

## Build & Usage
```bash
    🪟: time g++ -o main main.cpp -L./lib -I./include -lraylib -lssl -lcrypto -lws2_32 ; ./main.exe
    🐧: time g++ -o main main.cpp -L./lib -I./include -lraylib -lssl -lcrypto ; ./main
```

## License
**Nodepp-gpu** is distributed under the MIT License. See the LICENSE file for more details.
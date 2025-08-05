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
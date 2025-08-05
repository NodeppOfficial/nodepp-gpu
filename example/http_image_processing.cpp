#include <nodepp/nodepp.h>
#include <nodepp/https.h>
#include <gpu/gpu.h>

using namespace nodepp;

void gpu_convolution( string_t data, string_t ext ){ 
    
    gpu::gpu_t gpu( GPU_KERNEL( // Define a GPU kernel
    
        vec2 pixel_size= 1.0 / size; // Calculate pixel size for sampling
        vec4 sum = vec4( 0.0 );      // Accumulator for convolution
        vec2 uv_norm = uv / size;    // Normalized UV coordinates
        
        for( int y=0; y<3; y++ ){    // Loop through 3x3 filter rows
        for( int x=0; x<3; x++ ){    // Loop through 3x3 filter columns
                
            // Calculate centered image offset
            vec2 image_offset = vec2(x, y) - 1.0;
            vec2 image_coord  = uv_norm + image_offset*pixel_size;
            vec3 image_val    = texture( image, image_coord ).xyz;

            // Calculate filter texture coordinates
            vec2 fltr_coord = (vec2(x, y) + 0.5) / 3.0;
            vec3 fltr_val   = texture( fltr, fltr_coord ).xyz;

            // Multiply image and filter values, add to sum
            sum += vec4( image_val * fltr_val, 0.0 );
            
        }}
        
        // Remap convolution result for visualization
        float remapped_x = (sum.x + 2.0) / 4.0;
        
        // Clamp result to [0, 1] range
        remapped_x = clamp(remapped_x, 0.0, 1.0);
        
        // Return final grayscale color
        return vec4( remapped_x, remapped_x, remapped_x, 1.0 );
    ));

    gpu::matrix_t image ( data, ext );         // Load input image
    gpu::matrix_t filter( 3, 3, ptr_t<float>({ // Create a 3x3 filter matrix (vertical edge detector)
        1., 0., -1.,
        1., 0., -1.,
        1., 0., -1.,
    }));

    gpu.set_output( image.width(), image.height(), gpu::OUT_UCHAR4 ); // Set output dimensions and format
    gpu.set_input ( filter, "fltr"  ); // Bind filter matrix to kernel
    gpu.set_input ( image , "image" ); // Bind image matrix to kernel
    gpu.set_input ( gpu::uvec2_t({     // Pass image size as a uniform
        image.width (),
        image.height()
    }), "size" );

    gpu::save_canvas( gpu().get(), "output.png" ); // Execute kernel and save output

}

void onMain(){ 
    
    gpu::start_machine();

    fetch_t args; ssl_t ssl;
            args.url     = "https://deep-image.ai/blog/content/images/size/w1600/2022/08/magic-g1db898374_1920.jpg";
            args.headers = header_t({ { "Host", url::host(args.url) } });
            args.method  = "GET";

    https::fetch( args, ssl )
    
    .then([=]( https_t cli ){
        auto data = stream::await( cli );
        gpu_convolution( data, "jpg" );
    })

    .fail([=]( except_t err ){
        console::error( err.data() );
    });

    // gpu::stop_machine(); nodepp automaticaly stop machine at close

}
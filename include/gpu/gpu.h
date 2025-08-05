/*
 * Copyright 2023 The Nodepp Project Authors. All Rights Reserved.
 *
 * Licensed under the MIT (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://github.com/NodeppOfficial/nodepp/blob/main/LICENSE
 */

/*────────────────────────────────────────────────────────────────────────────*/

#ifndef NODEPP_GPU
#define NODEPP_GPU

/*────────────────────────────────────────────────────────────────────────────*/

#include <nodepp/nodepp.h>
#include <nodepp/stream.h>
#include <nodepp/map.h>
#include <nodepp/any.h>
#include <nodepp/fs.h>

/*────────────────────────────────────────────────────────────────────────────*/

namespace RL {
#include "raylib.h"
#include "raygl.h"
}

namespace RL { RenderTexture2D LoadRenderTexture( int width, int height, int format ) {
    RenderTexture2D target = { 0 };

    target.id = rlLoadFramebuffer(); // Load an empty framebuffer

    if( target.id>0 ) {
        rlEnableFramebuffer(target.id);

        // Create color texture (default to RGBA)
        target.texture.id = rlLoadTexture(NULL, width, height, format, 1);
        target.texture.width = width;
        target.texture.height = height;
        target.texture.format = format;
        target.texture.mipmaps = 1;

        // Create depth renderbuffer/texture
        target.depth.id = rlLoadTextureDepth(width, height, true);
        target.depth.width = width;
        target.depth.height = height;
        target.depth.format = 19;       //DEPTH_COMPONENT_24BIT?
        target.depth.mipmaps = 1;

        // Attach color texture and depth renderbuffer/texture to FBO
        rlFramebufferAttach(target.id, target.texture.id, RL_ATTACHMENT_COLOR_CHANNEL0, RL_ATTACHMENT_TEXTURE2D, 0);
        rlFramebufferAttach(target.id, target.depth.id, RL_ATTACHMENT_DEPTH, RL_ATTACHMENT_RENDERBUFFER, 0);

        // Check if fbo is complete with attachments (valid)
        if (rlFramebufferComplete(target.id)) TRACELOG(LOG_INFO, "FBO: [ID %i] Framebuffer object created successfully", target.id);

        rlDisableFramebuffer();
    } else { TRACELOG(LOG_WARNING, "FBO: Framebuffer object can not be created"); }

    return target;
}}

/*────────────────────────────────────────────────────────────────────────────*/

#define GPU_KERNEL(...) #__VA_ARGS__

#if defined(GRAPHICS_API_OPENGL_33)
    #define GLSL_VERSION "#version 330\n"
#elif defined(GRAPHICS_API_OPENGL_21)
    #define GLSL_VERSION "#version 120\n"
#else
    #define GLSL_VERSION "#version 100\nprecision highp float;\n"
#endif

/*────────────────────────────────────────────────────────────────────────────*/

namespace nodepp { namespace gpu { enum VAR_TYPE {

    VAR_BOOL  = RL::RL_SHADER_UNIFORM_INT  ,
    VAR_INT   = RL::RL_SHADER_UNIFORM_INT  ,
    VAR_UINT  = RL::RL_SHADER_UNIFORM_UINT ,
    VAR_FLOAT = RL::RL_SHADER_UNIFORM_FLOAT,
    
    VAR_BVEC2 = RL::RL_SHADER_UNIFORM_IVEC2,
    VAR_IVEC2 = RL::RL_SHADER_UNIFORM_IVEC2,
    VAR_UVEC2 = RL::RL_SHADER_UNIFORM_UIVEC2,
    VAR_VEC2  = RL::RL_SHADER_UNIFORM_VEC2 ,
    
    VAR_BVEC3 = RL::RL_SHADER_UNIFORM_IVEC3,
    VAR_IVEC3 = RL::RL_SHADER_UNIFORM_IVEC3,
    VAR_UVEC3 = RL::RL_SHADER_UNIFORM_UIVEC3,
    VAR_VEC3  = RL::RL_SHADER_UNIFORM_VEC3 ,
    
    VAR_BVEC4 = RL::RL_SHADER_UNIFORM_IVEC4,
    VAR_IVEC4 = RL::RL_SHADER_UNIFORM_IVEC4,
    VAR_UVEC4 = RL::RL_SHADER_UNIFORM_UIVEC4,
    VAR_VEC4  = RL::RL_SHADER_UNIFORM_VEC4 ,

    VAR_IMG2D = RL::RL_SHADER_UNIFORM_SAMPLER2D

};}}

/*────────────────────────────────────────────────────────────────────────────*/

namespace nodepp { namespace gpu { enum IMAGE_FORMAT {

    OUT_UCHAR  = RL::PIXELFORMAT_UNCOMPRESSED_GRAYSCALE   ,
    OUT_UCHAR2 = RL::PIXELFORMAT_UNCOMPRESSED_GRAY_ALPHA  ,
    OUT_UCHAR3 = RL::PIXELFORMAT_UNCOMPRESSED_R8G8B8      ,
    OUT_UCHAR4 = RL::PIXELFORMAT_UNCOMPRESSED_R8G8B8A8    ,

    OUT_FLOAT  = RL::PIXELFORMAT_UNCOMPRESSED_R16         ,
    OUT_FLOAT3 = RL::PIXELFORMAT_UNCOMPRESSED_R16G16B16   ,
    OUT_FLOAT4 = RL::PIXELFORMAT_UNCOMPRESSED_R16G16B16A16,
    
    OUT_DOUBLE = RL::PIXELFORMAT_UNCOMPRESSED_R32         ,
    OUT_DOUBLE3= RL::PIXELFORMAT_UNCOMPRESSED_R32G32B32   ,
    OUT_DOUBLE4= RL::PIXELFORMAT_UNCOMPRESSED_R32G32B32A32,
    
};}}

/*────────────────────────────────────────────────────────────────────────────*/

namespace nodepp { namespace gpu {

struct bvec2_t { bool   x; bool   y; };
struct ivec2_t {  int   x;  int   y; };
struct uvec2_t { uint   x; uint   y; };
struct  vec2_t { float  x; float  y; };

struct bvec3_t { bool   x; bool   y; bool   z; };
struct ivec3_t {  int   x;  int   y;  int   z; };
struct uvec3_t { uint   x; uint   y; uint   z; };
struct  vec3_t { float  x; float  y; float  z; };

struct bvec4_t { bool   x; bool   y; bool   z; bool   w; };
struct ivec4_t {  int   x;  int   y;  int   z;  int   w; };
struct uvec4_t { uint   x; uint   y; uint   z; uint   w; };
struct  vec4_t { float  x; float  y; float  z; float  w; };

}}

/*────────────────────────────────────────────────────────────────────────────*/

namespace nodepp { namespace gpu { class matrix_t {
protected:

    struct NODE {
        uint width, height;
        ptr_t<float> data;
    };  ptr_t<NODE> obj;

public:

    matrix_t( uint width, uint height, ptr_t<float> data ) : obj( new NODE() ) {

        if( width * height != data.size() )
          { throw except_t( "matrix ptr size must be", width * height ); }

        obj->data = ptr_t<float>( width * height * 4, 0x00 );
        obj->width= width; obj->height = height;

        ulong x=0; while( x<data.size() ){
            ulong y=x*4;
            obj->data[y] = data[x];
        ++x; } 

    }

    matrix_t( uint width, uint height, ptr_t<vec2_t> data ) : obj( new NODE() ) {

        if( width * height != data.size() )
          { throw except_t( "matrix ptr size must be", width * height ); }

        obj->data = ptr_t<float>( width * height * 4, 0x00 );
        obj->width= width; obj->height = height;

        ulong x=0; while( x<data.size() ){
        ulong y=x*4;
            obj->data[y+0] = data[x].x;
            obj->data[y+1] = data[x].y;
        ++x; }

    }

    matrix_t( uint width, uint height, ptr_t<vec3_t> data ) : obj( new NODE() ) {

        if( width * height != data.size() )
          { throw except_t( "matrix ptr size must be", width * height ); }

        obj->data = ptr_t<float>( width * height * 4, 0x00 );
        obj->width= width; obj->height = height;

        ulong x=0; while( x<data.size() ){
        ulong y=x*4;
            obj->data[y+0] = data[x].x;
            obj->data[y+1] = data[x].y;
            obj->data[y+2] = data[x].z;
        ++x; }

    }

    matrix_t( uint width, uint height, ptr_t<vec4_t> data ) : obj( new NODE() ) {

        if( width * height != data.size() )
          { throw except_t( "matrix ptr size must be", width * height ); }

        obj->data = ptr_t<float>( width * height * 4, 0x00 );
        obj->width= width; obj->height = height;

        ulong x=0; while( x<data.size() ){
        ulong y=x*4;
            obj->data[y+0] = data[x].x;
            obj->data[y+1] = data[x].y;
            obj->data[y+2] = data[x].z;
            obj->data[y+3] = data[x].w;
        ++x; } 

    }

    /*─······································································─*/

    matrix_t( string_t data, string_t ext ) : obj( new NODE() ) {
        if( ext.empty() || data.empty() ){ throw except_t( "invalid image" ); }
        if( ext[0]!='.' ){ ext.unshift('.'); }

        auto img=RL::LoadImageFromMemory( ext.get(), (uchar*)data.get(), data.size() );
        /*-----*/RL::ImageFormat( &img, OUT_DOUBLE4 );

        obj->data = ptr_t<float>( img.width * img.height * 4, 0x00 );
        obj->width= img.width; obj->height = img.height ;
        memcpy( &obj->data, img.data, obj->data.size()*sizeof(float) );

        RL::UnloadImage( img );
    }

    matrix_t( string_t path ) : obj( new NODE() ) {
        if( path.empty() || !fs::exists_file(path) ){ throw except_t( "invalid image" ); }

        auto img=RL::LoadImage  ( path.get() );
        /*-----*/RL::ImageFormat( &img, OUT_DOUBLE4 );

        obj->data = ptr_t<float>( img.width * img.height * 4, 0x00 );
        obj->width= img.width; obj->height = img.height ;
        memcpy( &obj->data, img.data, obj->data.size()*sizeof(float) );

        RL::UnloadImage( img );
    }

    matrix_t( RL::Texture2D input ) : obj( new NODE() ){
        if( !RL::IsTextureValid( input ) ){ throw except_t( "invalid texture" ); }

        auto img=RL::LoadImageFromTexture( input );
        /*-----*/RL::ImageFormat( &img, OUT_DOUBLE4 );

        obj->data = ptr_t<float>( img.width * img.height * 4, 0x00 );
        obj->width= img.width; obj->height = img.height ;
        memcpy( &obj->data, img.data, obj->data.size()*sizeof(float) );

        RL::UnloadImage( img );
    }

    matrix_t( RL::Image input ) : obj( new NODE() ){
        if( !RL::IsImageValid( input ) ){ throw except_t( "invalid image" ); }
        /**/ RL::ImageFormat( &input, OUT_DOUBLE4 );

        obj->data = ptr_t<float>( input.width * input.height * 4, 0x00 );
        obj->width= input.width; obj->height = input.height;
        memcpy( &obj->data, input.data, obj->data.size()*sizeof(float) );

    }

    /*─······································································─*/

    matrix_t() noexcept : obj( new NODE() ){}
    virtual ~matrix_t() { /*-------------*/ }

    /*─······································································─*/

    float& operator[]( ulong pos ) const noexcept { return obj->data[pos]; }

    uint       size  () const noexcept { return obj->data.size(); }

    uint       height() const noexcept { return obj->height; }

    uint       width () const noexcept { return obj->width;  }

    ptr_t<float> data() const noexcept { return obj->data;   }

    /*─······································································─*/

    RL::Texture2D get() const noexcept {
        
        RL::Image img; img.mipmaps=1;
        img.height = obj->height    ;
        img.width  = obj->width     ;
        img.data   = &obj->data     ;
        img.format = OUT_DOUBLE4    ;

        return RL::LoadTextureFromImage( img );

    }

};}}

/*────────────────────────────────────────────────────────────────────────────*/

namespace nodepp { namespace gpu { 

template< class T > struct gpu_type_id  { static constexpr uchar value = 0xff; };

template<> struct gpu_type_id<bool>     { static constexpr uchar value = 0x01; };
template<> struct gpu_type_id<int>      { static constexpr uchar value = 0x02; };
template<> struct gpu_type_id<uint>     { static constexpr uchar value = 0x03; };
template<> struct gpu_type_id<float>    { static constexpr uchar value = 0x04; };

template<> struct gpu_type_id<bvec2_t>  { static constexpr uchar value = 0x11; };
template<> struct gpu_type_id<ivec2_t>  { static constexpr uchar value = 0x12; };
template<> struct gpu_type_id<uvec2_t>  { static constexpr uchar value = 0x13; };
template<> struct gpu_type_id< vec2_t>  { static constexpr uchar value = 0x14; };

template<> struct gpu_type_id<bvec3_t>  { static constexpr uchar value = 0x21; };
template<> struct gpu_type_id<ivec3_t>  { static constexpr uchar value = 0x22; };
template<> struct gpu_type_id<uvec3_t>  { static constexpr uchar value = 0x23; };
template<> struct gpu_type_id< vec3_t>  { static constexpr uchar value = 0x24; };

template<> struct gpu_type_id<bvec4_t>  { static constexpr uchar value = 0x31; };
template<> struct gpu_type_id<ivec4_t>  { static constexpr uchar value = 0x32; };
template<> struct gpu_type_id<uvec4_t>  { static constexpr uchar value = 0x33; };
template<> struct gpu_type_id< vec4_t>  { static constexpr uchar value = 0x34; };

template<> struct gpu_type_id<matrix_t> { static constexpr uchar value = 0x50; };

}}

/*────────────────────────────────────────────────────────────────────────────*/

namespace nodepp { namespace gpu { string_t _kernel_=GPU_KERNEL( 
    ${0} ${1} vec4 init(){
        vec2 uv=gl_FragCoord.xy;
        ${2} /*---------------*/
    } void main(){ gl_FragColor = init(); });
}}

/*────────────────────────────────────────────────────────────────────────────*/

namespace nodepp { namespace gpu { class gpu_t {
protected:

    struct DONE { any_t value; uchar type; };

    struct NODE {
        ptr_t<RL::RenderTexture2D> texture;
        map_t<string_t,DONE> /*----*/ vars;
        ptr_t<RL::Shader> /*-----*/ shader;
        string_t /*--------------*/ kernel;
        bool /*-----------------*/ state=1;
    };  ptr_t<NODE> obj;

    string_t get_kernel_soruce() const {
        if ( obj->kernel.empty() ){ throw except_t( "not kernel found" ); }
        /**/ return obj->kernel; /*------------------------------------*/
    }

    /*─······································································─*/

    template< class T >
    void set_variable( string_t name, int flag, const T& value ) const noexcept {
         int sid = RL::GetShaderLocation( *obj->shader, name.get() );
         SetShaderValueV( *obj->shader, sid, &value, flag, 1 );
    }

    void set_matrix( string_t name, const matrix_t& value ) const noexcept {
         int sid = RL::GetShaderLocation( *obj->shader, name.get() );
         SetShaderValueTexture( *obj->shader, sid, value.get() );        
    }

    /*─······································································─*/

    void set_kernel_variables() const {
     if( obj->shader.null() ) /*----------*/ { throw except_t("invalid shader"); }
     if( !RL::IsShaderValid( *obj->shader ) ){ throw except_t("Invalid Shader"); }
    for( auto x: obj->vars.data() ){ switch( x.second.type ){

        case 0x01: set_variable( x.first, VAR_BOOL , x.second.value.as<bool> ());   break;
        case 0x02: set_variable( x.first, VAR_INT  , x.second.value.as<int>  ());   break;
        case 0x03: set_variable( x.first, VAR_UINT , x.second.value.as<uint> ());   break;
        case 0x04: set_variable( x.first, VAR_FLOAT, x.second.value.as<float>());   break;

        case 0x11: set_variable( x.first, VAR_BVEC2, x.second.value.as<bvec2_t>()); break;
        case 0x12: set_variable( x.first, VAR_IVEC2, x.second.value.as<ivec2_t>()); break;
        case 0x13: set_variable( x.first, VAR_UVEC2, x.second.value.as<uvec2_t>()); break;
        case 0x14: set_variable( x.first, VAR_VEC2 , x.second.value.as< vec2_t>()); break;

        case 0x21: set_variable( x.first, VAR_BVEC3, x.second.value.as<bvec3_t>()); break;
        case 0x22: set_variable( x.first, VAR_IVEC3, x.second.value.as<ivec3_t>()); break;
        case 0x23: set_variable( x.first, VAR_UVEC3, x.second.value.as<uvec3_t>()); break;
        case 0x24: set_variable( x.first, VAR_VEC3 , x.second.value.as< vec3_t>()); break;

        case 0x31: set_variable( x.first, VAR_BVEC4, x.second.value.as<bvec4_t>()); break;
        case 0x32: set_variable( x.first, VAR_IVEC4, x.second.value.as<ivec4_t>()); break;
        case 0x33: set_variable( x.first, VAR_UVEC4, x.second.value.as<uvec4_t>()); break;
        case 0x34: set_variable( x.first, VAR_VEC4 , x.second.value.as< vec4_t>()); break;

        case 0x50: set_matrix  ( x.first, x.second.value.as<matrix_t>() ); /*----*/ break;

    }}}

    /*─······································································─*/

    string_t get_kernel_variables() const noexcept {
    string_t out; for( auto x: obj->vars.data() ){ switch( x.second.type ){

        case 0x01: out += regex::format( "uniform bool      ${0};\n", x.first ); break;
        case 0x02: out += regex::format( "uniform int       ${0};\n", x.first ); break;
        case 0x03: out += regex::format( "uniform uint      ${0};\n", x.first ); break;
        case 0x04: out += regex::format( "uniform float     ${0};\n", x.first ); break;

        case 0x11: out += regex::format( "uniform bvec2     ${0};\n", x.first ); break;
        case 0x12: out += regex::format( "uniform ivec2     ${0};\n", x.first ); break;
        case 0x13: out += regex::format( "uniform uvec2     ${0};\n", x.first ); break;
        case 0x14: out += regex::format( "uniform  vec2     ${0};\n", x.first ); break;

        case 0x21: out += regex::format( "uniform bvec3     ${0};\n", x.first ); break;
        case 0x22: out += regex::format( "uniform ivec3     ${0};\n", x.first ); break;
        case 0x23: out += regex::format( "uniform uvec3     ${0};\n", x.first ); break;
        case 0x24: out += regex::format( "uniform  vec3     ${0};\n", x.first ); break;

        case 0x31: out += regex::format( "uniform bvec4     ${0};\n", x.first ); break;
        case 0x32: out += regex::format( "uniform ivec4     ${0};\n", x.first ); break;
        case 0x33: out += regex::format( "uniform uvec4     ${0};\n", x.first ); break;
        case 0x34: out += regex::format( "uniform  vec4     ${0};\n", x.first ); break;

        case 0x50: out += regex::format( "uniform sampler2D ${0};\n", x.first ); break;

    }} return out; }

public: 

    gpu_t( string_t kernel )  : obj( new NODE() ){ obj->kernel=kernel; }
    /*---*/  gpu_t() noexcept : obj( new NODE() ){ obj->state =0; /**/ }
    virtual ~gpu_t() noexcept { if( obj.count()>1 ){ return; } free(); }

    /*─······································································─*/

    bool is_closed() const noexcept { return obj->state==0; }
    void /**/close() const noexcept { /*---------*/ free(); }

    /*─······································································─*/

    gpu_t& remove_input( string_t name ) /*const noexcept*/ { 
        obj->vars.erase(name); return *this; 
    }

    /*─······································································─*/

    gpu_t& set_output( uint width, uint height, uint format=OUT_DOUBLE4 ) noexcept {
    if( !is_closed() ){
        if( !obj->texture.null() ){ RL::UnloadRenderTexture( *obj->texture ); }
        /**/ obj->texture =type::bind( RL::LoadRenderTexture( width, height, format ) );
    } return *this; }

    template< class T >
    gpu_t& set_input( const T& value, string_t name ) /*const noexcept*/ {

        if( gpu_type_id<T>::value == 0xff )
          { throw except_t("invalid gpu object type"); }

        if( name.empty() )
          { throw except_t("invalid variable name"); }
        if( regex::test( name, "^[ 0-9]+", true ) )
          { throw except_t("invalid variable name"); }
        if( regex::test( name, "[^a-z0-9_]+", true ) )
          { throw except_t("invalid variable name"); }
        
        DONE item; item.type = gpu_type_id<T>::value;
        /*------*/ item.value= value;
        obj->vars  [ name ]  = item;
    
    return *this; }

    gpu_t& compile() /*const noexcept*/ {
        if( obj->kernel.empty() ){ throw except_t("no kernel found"); }
        if(!obj->shader.null () ){ RL::UnloadShader( *obj->shader ); }

        obj->shader = type::bind( RL::LoadShaderFromMemory( 0,
            regex::format( _kernel_, GLSL_VERSION, 
                get_kernel_variables(), /*------*/
                get_kernel_soruce() /*----------*/
            ).get()
        ));
        
        if( !RL::IsShaderValid( *obj->shader ) )
          { throw except_t("Invalid Shader"); }

    return *this; }

    /*─······································································─*/

    void free() const noexcept { if( !is_closed() ){
        if( !obj->texture.null() ){ RL::UnloadRenderTexture( *obj->texture ); }
        if( !obj->shader .null() ){ RL::UnloadShader( *obj->shader ); }
        /**/ obj->state = 0; /*------------------------------------*/
    }}

    /*─······································································─*/

    matrix_t operator()()/**/{ if( !is_closed() ){
        if( obj->texture.null() ){ throw except_t("invalid texture"); }
        if( obj->shader .null() ){ compile(); /*-------------------*/ }

        int w = obj->texture->texture.width ;
        int h = obj->texture->texture.height;

        RL::BeginTextureMode( *obj->texture ); RL::ClearBackground( RL::BLACK );
        RL::BeginShaderMode ( *obj->shader  ); set_kernel_variables(); /*-----*/
        RL::DrawRectangle( 0,0, w, h, RL::WHITE );
        RL::EndShaderMode(); RL::EndTextureMode();

        return matrix_t( obj->texture->texture );

    } throw except_t( "gpu kernel closed" ); }

};}}

/*────────────────────────────────────────────────────────────────────────────*/

namespace nodepp { namespace gpu { bool _gpu_ = false;

    void stop_machine () { if( _gpu_ ){ RL::CloseWindow(); }}

    bool start_machine() { if(!_gpu_ ){ try {
         RL::SetConfigFlags( RL::FLAG_WINDOW_HIDDEN );
         RL::InitWindow( 100, 100, "gpupp" );
         RL::SetTargetFPS( 120 ); /*-------*/
         process::onSIGEXIT([=](){ stop_machine(); });
    } catch(...) { return false; }} return true; }

    void save_canvas( matrix_t input, string_t path ) {
        auto image = RL::LoadImageFromTexture(input.get());
        RL::ExportImage( image, path.get() ); RL::UnloadImage( image );
    }

    ptr_t<uchar> get_canvas( matrix_t input ) {
        auto image = RL::LoadImageFromTexture( input.get() );
        int  size  = 0; /*---------------------------------*/
        auto data  = RL::ExportImageToMemory( image, ".png", &size );
        RL::UnloadImage( image ); return ptr_t<uchar>( data, (ulong) size );
    }

}}

/*────────────────────────────────────────────────────────────────────────────*/

#endif
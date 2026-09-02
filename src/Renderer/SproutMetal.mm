#include <TargetConditionals.h>

#if defined(__APPLE__) && TARGET_OS_MAC
#define SOKOL_METAL
#define SOKOL_IMPL
#define SOKOL_NO_ENTRY

#import <Cocoa/Cocoa.h>
#import <Metal/Metal.h>
#import <MetalKit/MetalKit.h>
#import <QuartzCore/QuartzCore.h>

#include "sokol/sokol_app.h"
#include "sokol/sokol_gfx.h"
#include "sokol/sokol_glue.h"

#include <cstring>
#include <string>
#include <vector>

namespace Sprout
{

auto captureMetalSwapchain(int& width, int& height, std::vector<unsigned char>& rgba, std::string& error) -> bool
{
    id<MTLDevice> device = (__bridge id<MTLDevice>)sapp_metal_get_device();
    id<CAMetalDrawable> drawable = (__bridge id<CAMetalDrawable>)sapp_metal_get_current_drawable();
    if (!device || !drawable)
    {
        error = "Metal swapchain not ready";
        return false;
    }

    // After sg_commit, MTKView has already resolved MSAA into the drawable.
    id<MTLTexture> src = drawable.texture;
    if (!src)
    {
        error = "Metal drawable has no texture";
        return false;
    }

    width = static_cast<int>(src.width);
    height = static_cast<int>(src.height);
    if (width <= 0 || height <= 0)
    {
        error = "no framebuffer yet";
        return false;
    }

    const NSUInteger packedStride = static_cast<NSUInteger>(width) * 4;
    const NSUInteger bytesPerRow = (packedStride + 255u) & ~255u;
    const NSUInteger bufferSize = bytesPerRow * static_cast<NSUInteger>(height);

    id<MTLBuffer> buffer = [device newBufferWithLength:bufferSize options:MTLResourceStorageModeShared];
    if (!buffer)
    {
        error = "Metal readback buffer alloc failed";
        return false;
    }

    id<MTLCommandQueue> queue = [device newCommandQueue];
    if (!queue)
    {
        error = "Metal command queue alloc failed";
        return false;
    }

    id<MTLCommandBuffer> cmd = [queue commandBuffer];
    id<MTLBlitCommandEncoder> blit = [cmd blitCommandEncoder];
    [blit copyFromTexture:src
              sourceSlice:0
              sourceLevel:0
             sourceOrigin:MTLOriginMake(0, 0, 0)
               sourceSize:MTLSizeMake(static_cast<NSUInteger>(width), static_cast<NSUInteger>(height), 1)
                 toBuffer:buffer
        destinationOffset:0
   destinationBytesPerRow:bytesPerRow
 destinationBytesPerImage:bufferSize];
    [blit endEncoding];
    [cmd commit];
    [cmd waitUntilCompleted];

    rgba.resize(static_cast<size_t>(width) * static_cast<size_t>(height) * 4);
    const auto* srcBytes = static_cast<const unsigned char*>(buffer.contents);
    const bool bgra = (src.pixelFormat == MTLPixelFormatBGRA8Unorm
                       || src.pixelFormat == MTLPixelFormatBGRA8Unorm_sRGB);
    for (int y = 0; y < height; ++y)
    {
        const unsigned char* row = srcBytes + static_cast<size_t>(y) * bytesPerRow;
        unsigned char* dst = rgba.data() + static_cast<size_t>(y) * packedStride;
        if (!bgra)
        {
            std::memcpy(dst, row, packedStride);
            continue;
        }
        for (int x = 0; x < width; ++x)
        {
            dst[x * 4 + 0] = row[x * 4 + 2];
            dst[x * 4 + 1] = row[x * 4 + 1];
            dst[x * 4 + 2] = row[x * 4 + 0];
            dst[x * 4 + 3] = row[x * 4 + 3];
        }
    }
    return true;
}

}

#endif

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

// This file only provides the Metal implementation of sokol
// The actual engine code is in Sprout.cpp
#endif

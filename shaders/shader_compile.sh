#!/bin/bash

cd ~/Documents/code/Sapling/shaders

../tools/sokol-shdc --input quad.glsl --output ../include/Renderer/quad.h -l hlsl5:glsl410:wgsl:metal_macos
../tools/sokol-shdc --input mesh3d.glsl --output ../include/Renderer/mesh3d.h -l hlsl5:glsl410:wgsl:metal_macos
../tools/sokol-shdc --input mesh3d_skinned.glsl --output ../include/Renderer/mesh3d_skinned.h -l hlsl5:glsl410:wgsl:metal_macos

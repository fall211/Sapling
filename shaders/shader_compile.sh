#!/bin/bash

cd ~/Documents/code/Sapling/shaders

../tools/sokol-shdc --input quad.glsl --output ../include/Renderer/quad.h -l hlsl5:glsl410:wgsl:metal_macos

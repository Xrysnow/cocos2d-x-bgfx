# cocos2d-x-bgfx

This project is a [bgfx](https://github.com/bkaradzic/bgfx) renderer backend for [cocos2d-x](https://github.com/cocos2d/cocos2d-x) v4.

## Warning

* This project is not complete and still has many problems.
* This project is not fully tested.
* This project only supports desktop platforms now.

You are welcome to contribute!

## How to use

To use this project, you have to modify bgfx and cocos2d-x.

\- Modify `shaderc` in bgfx.

- Remove `main` function in `shaderc.cpp`.
- Replace implementation of `g_allocator` and `TinyStlAllocator` with `#include "../../src/bgfx_p.h"` in `shaderc_spirv.cpp`.
- Compile it as static library and link to your program.

\- Remove following files from the project of cocos2d-x.

- All files in `renderer/backend/metal` and `renderer/backend/opengl`
- `renderer/CCRenderer.cpp`
- `renderer/backend/ProgramCache.cpp`
- `renderer/backend/ProgramState.cpp`
- `2d/CCSprite.cpp`
- `math/Mat4.cpp`
- `platform/desktop/CCGLViewImpl-desktop.cpp`

\- Change your shaders into bgfx format.
\- Add `gl_Position.xy = applyVP(gl_Position.xy);` to your vertex shader to apply viewport.
\- Add `bgfx/include` `bimg/include` `bx/include` to your include path.
\- Link libraries from bgfx except `dear-imgui` and `example-common`.

## Known problems

* Program will write a `.hlsl` file when compile HLSL shaders.
* Not work properly when window size changes.
* `Texture2DBX::getBytes`/`TextureCubeBX::getBytes` may not work properly.
* `CommandBufferBX::captureScreen` may not work properly.
* `CommandBufferBX::applyRenderPassDescriptor` may not work properly.
* `CommandBufferBX::setLineWidth` is not supported.


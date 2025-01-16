#pragma once

#include <iostream>
#include <vector>
#include <algorithm>
#include <sstream>
#include <cstdlib>
#include <memory>
#define NOMINMAX  //for directx

// SDL Headers
#pragma warning(push)
#pragma warning(disable : 26819) // disable the fallthrough between switch labels warning

#include "SDL.h"
#include "SDL_syswm.h"
#include "SDL_surface.h"
#include "SDL_image.h"

#pragma warning(pop)

// DirectX Headers
#include <dxgi.h>
#include <d3d11.h>
#include <d3dcompiler.h>
#include <d3dx11effect.h>

#pragma warning(push)
#pragma warning(disable : 26819) // disable the fallthrough between switch labels warning

#include <d3dxGlobal.h>

#pragma warning(pop)

// Framework Headers
#include "Timer.h"
#include "Math.h"

// Cool CMD prompt colours
#define RESET   "\033[0m"
#define RED     "\033[31m"
#define GREEN   "\033[32m"
#define YELLOW  "\033[33m"

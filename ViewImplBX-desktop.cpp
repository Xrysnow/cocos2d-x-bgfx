#include "platform/desktop/CCGLViewImpl-desktop.h"
#include "platform/CCApplication.h"
#include "base/CCDirector.h"
#include "base/CCTouch.h"
#include "base/CCEventDispatcher.h"
#include "base/CCEventKeyboard.h"
#include "base/CCEventMouse.h"
#include "base/CCIMEDispatcher.h"
#include "base/ccUtils.h"
#include "base/ccUTF8.h"
#include "2d/CCCamera.h"
#if CC_ICON_SET_SUPPORT
#include "platform/CCImage.h"
#endif /* CC_ICON_SET_SUPPORT */
#include "renderer/CCRenderer.h"
#include <cmath>
#include <unordered_map>

#define USE_WAYLAND 0
#if CC_TARGET_PLATFORM == CC_PLATFORM_LINUX
#	if USE_WAYLAND
#		include <wayland-egl.h>
#		define GLFW_EXPOSE_NATIVE_WAYLAND
#	else
#		define GLFW_EXPOSE_NATIVE_X11
#		define GLFW_EXPOSE_NATIVE_GLX
#	endif
#include "glfw3native.h"
#endif

#include "CallbackBX.h"
#include "bgfx/bgfx.h"
#include "bgfx/platform.h"
#include "UtilsBX.h"
#include "ProgramBX.h"

static void* glfwNativeWindowHandle(GLFWwindow* _window)
{
#if CC_TARGET_PLATFORM == CC_PLATFORM_LINUX

#if USE_WAYLAND
	wl_egl_window *win_impl = (wl_egl_window*)glfwGetWindowUserPointer(_window);
	if (!win_impl)
	{
		int width, height;
		glfwGetWindowSize(_window, &width, &height);
		struct wl_surface* surface = (struct wl_surface*)glfwGetWaylandWindow(_window);
		if (!surface)
			return nullptr;
		win_impl = wl_egl_window_create(surface, width, height);
		glfwSetWindowUserPointer(_window, (void*)(uintptr_t)win_impl);
	}
	return (void*)(uintptr_t)win_impl;
#else
	return (void*)(uintptr_t)glfwGetX11Window(_window);
#endif

#elif CC_TARGET_PLATFORM == CC_PLATFORM_MAC
	return glfwGetCocoaWindow(_window);
#elif CC_TARGET_PLATFORM == CC_PLATFORM_WIN32
	return glfwGetWin32Window(_window);
#endif
}
static void glfwDestroyWindowImpl(GLFWwindow *_window)
{
	if (!_window)
		return;
#if CC_TARGET_PLATFORM == CC_PLATFORM_LINUX
#if USE_WAYLAND
	wl_egl_window *win_impl = (wl_egl_window*)glfwGetWindowUserPointer(_window);
	if (win_impl)
	{
		glfwSetWindowUserPointer(_window, nullptr);
		wl_egl_window_destroy(win_impl);
	}
#endif
#endif
	glfwDestroyWindow(_window);
}
static void glfwSetWindow(GLFWwindow* _window, bgfx::PlatformData& pd)
{
#if CC_TARGET_PLATFORM == CC_PLATFORM_LINUX
#if USE_WAYLAND
	pd.ndt = glfwGetWaylandDisplay();
#else
	pd.ndt = glfwGetX11Display();
#endif
#elif CC_TARGET_PLATFORM == CC_PLATFORM_MAC
	pd.ndt = nullptr;
#elif CC_TARGET_PLATFORM == CC_PLATFORM_WIN32
	pd.ndt = nullptr;
#endif // BX_PLATFORM_WINDOWS
	pd.nwh = glfwNativeWindowHandle(_window);
	pd.context = nullptr;
	pd.backBuffer = nullptr;
	pd.backBufferDS = nullptr;
	//bgfx::setPlatformData(pd);
}

NS_CC_BEGIN

GLViewImpl* GLFWEventHandler::_view = nullptr;

const std::string GLViewImpl::EVENT_WINDOW_RESIZED = "glview_window_resized";
const std::string GLViewImpl::EVENT_WINDOW_FOCUSED = "glview_window_focused";
const std::string GLViewImpl::EVENT_WINDOW_UNFOCUSED = "glview_window_unfocused";

////////////////////////////////////////////////////

struct keyCodeItem
{
    int glfwKeyCode;
    EventKeyboard::KeyCode keyCode;
};

static std::unordered_map<int, EventKeyboard::KeyCode> g_keyCodeMap;

static keyCodeItem g_keyCodeStructArray[] = {
    /* The unknown key */
    { GLFW_KEY_UNKNOWN         , EventKeyboard::KeyCode::KEY_NONE          },

    /* Printable keys */
    { GLFW_KEY_SPACE           , EventKeyboard::KeyCode::KEY_SPACE         },
    { GLFW_KEY_APOSTROPHE      , EventKeyboard::KeyCode::KEY_APOSTROPHE    },
    { GLFW_KEY_COMMA           , EventKeyboard::KeyCode::KEY_COMMA         },
    { GLFW_KEY_MINUS           , EventKeyboard::KeyCode::KEY_MINUS         },
    { GLFW_KEY_PERIOD          , EventKeyboard::KeyCode::KEY_PERIOD        },
    { GLFW_KEY_SLASH           , EventKeyboard::KeyCode::KEY_SLASH         },
    { GLFW_KEY_0               , EventKeyboard::KeyCode::KEY_0             },
    { GLFW_KEY_1               , EventKeyboard::KeyCode::KEY_1             },
    { GLFW_KEY_2               , EventKeyboard::KeyCode::KEY_2             },
    { GLFW_KEY_3               , EventKeyboard::KeyCode::KEY_3             },
    { GLFW_KEY_4               , EventKeyboard::KeyCode::KEY_4             },
    { GLFW_KEY_5               , EventKeyboard::KeyCode::KEY_5             },
    { GLFW_KEY_6               , EventKeyboard::KeyCode::KEY_6             },
    { GLFW_KEY_7               , EventKeyboard::KeyCode::KEY_7             },
    { GLFW_KEY_8               , EventKeyboard::KeyCode::KEY_8             },
    { GLFW_KEY_9               , EventKeyboard::KeyCode::KEY_9             },
    { GLFW_KEY_SEMICOLON       , EventKeyboard::KeyCode::KEY_SEMICOLON     },
    { GLFW_KEY_EQUAL           , EventKeyboard::KeyCode::KEY_EQUAL         },
    { GLFW_KEY_A               , EventKeyboard::KeyCode::KEY_A             },
    { GLFW_KEY_B               , EventKeyboard::KeyCode::KEY_B             },
    { GLFW_KEY_C               , EventKeyboard::KeyCode::KEY_C             },
    { GLFW_KEY_D               , EventKeyboard::KeyCode::KEY_D             },
    { GLFW_KEY_E               , EventKeyboard::KeyCode::KEY_E             },
    { GLFW_KEY_F               , EventKeyboard::KeyCode::KEY_F             },
    { GLFW_KEY_G               , EventKeyboard::KeyCode::KEY_G             },
    { GLFW_KEY_H               , EventKeyboard::KeyCode::KEY_H             },
    { GLFW_KEY_I               , EventKeyboard::KeyCode::KEY_I             },
    { GLFW_KEY_J               , EventKeyboard::KeyCode::KEY_J             },
    { GLFW_KEY_K               , EventKeyboard::KeyCode::KEY_K             },
    { GLFW_KEY_L               , EventKeyboard::KeyCode::KEY_L             },
    { GLFW_KEY_M               , EventKeyboard::KeyCode::KEY_M             },
    { GLFW_KEY_N               , EventKeyboard::KeyCode::KEY_N             },
    { GLFW_KEY_O               , EventKeyboard::KeyCode::KEY_O             },
    { GLFW_KEY_P               , EventKeyboard::KeyCode::KEY_P             },
    { GLFW_KEY_Q               , EventKeyboard::KeyCode::KEY_Q             },
    { GLFW_KEY_R               , EventKeyboard::KeyCode::KEY_R             },
    { GLFW_KEY_S               , EventKeyboard::KeyCode::KEY_S             },
    { GLFW_KEY_T               , EventKeyboard::KeyCode::KEY_T             },
    { GLFW_KEY_U               , EventKeyboard::KeyCode::KEY_U             },
    { GLFW_KEY_V               , EventKeyboard::KeyCode::KEY_V             },
    { GLFW_KEY_W               , EventKeyboard::KeyCode::KEY_W             },
    { GLFW_KEY_X               , EventKeyboard::KeyCode::KEY_X             },
    { GLFW_KEY_Y               , EventKeyboard::KeyCode::KEY_Y             },
    { GLFW_KEY_Z               , EventKeyboard::KeyCode::KEY_Z             },
    { GLFW_KEY_LEFT_BRACKET    , EventKeyboard::KeyCode::KEY_LEFT_BRACKET  },
    { GLFW_KEY_BACKSLASH       , EventKeyboard::KeyCode::KEY_BACK_SLASH    },
    { GLFW_KEY_RIGHT_BRACKET   , EventKeyboard::KeyCode::KEY_RIGHT_BRACKET },
    { GLFW_KEY_GRAVE_ACCENT    , EventKeyboard::KeyCode::KEY_GRAVE         },
    { GLFW_KEY_WORLD_1         , EventKeyboard::KeyCode::KEY_GRAVE         },
    { GLFW_KEY_WORLD_2         , EventKeyboard::KeyCode::KEY_NONE          },

    /* Function keys */
    { GLFW_KEY_ESCAPE          , EventKeyboard::KeyCode::KEY_ESCAPE        },
    { GLFW_KEY_ENTER           , EventKeyboard::KeyCode::KEY_ENTER      },
    { GLFW_KEY_TAB             , EventKeyboard::KeyCode::KEY_TAB           },
    { GLFW_KEY_BACKSPACE       , EventKeyboard::KeyCode::KEY_BACKSPACE     },
    { GLFW_KEY_INSERT          , EventKeyboard::KeyCode::KEY_INSERT        },
    { GLFW_KEY_DELETE          , EventKeyboard::KeyCode::KEY_DELETE        },
    { GLFW_KEY_RIGHT           , EventKeyboard::KeyCode::KEY_RIGHT_ARROW   },
    { GLFW_KEY_LEFT            , EventKeyboard::KeyCode::KEY_LEFT_ARROW    },
    { GLFW_KEY_DOWN            , EventKeyboard::KeyCode::KEY_DOWN_ARROW    },
    { GLFW_KEY_UP              , EventKeyboard::KeyCode::KEY_UP_ARROW      },
    { GLFW_KEY_PAGE_UP         , EventKeyboard::KeyCode::KEY_PG_UP      },
    { GLFW_KEY_PAGE_DOWN       , EventKeyboard::KeyCode::KEY_PG_DOWN    },
    { GLFW_KEY_HOME            , EventKeyboard::KeyCode::KEY_HOME       },
    { GLFW_KEY_END             , EventKeyboard::KeyCode::KEY_END           },
    { GLFW_KEY_CAPS_LOCK       , EventKeyboard::KeyCode::KEY_CAPS_LOCK     },
    { GLFW_KEY_SCROLL_LOCK     , EventKeyboard::KeyCode::KEY_SCROLL_LOCK   },
    { GLFW_KEY_NUM_LOCK        , EventKeyboard::KeyCode::KEY_NUM_LOCK      },
    { GLFW_KEY_PRINT_SCREEN    , EventKeyboard::KeyCode::KEY_PRINT         },
    { GLFW_KEY_PAUSE           , EventKeyboard::KeyCode::KEY_PAUSE         },
    { GLFW_KEY_F1              , EventKeyboard::KeyCode::KEY_F1            },
    { GLFW_KEY_F2              , EventKeyboard::KeyCode::KEY_F2            },
    { GLFW_KEY_F3              , EventKeyboard::KeyCode::KEY_F3            },
    { GLFW_KEY_F4              , EventKeyboard::KeyCode::KEY_F4            },
    { GLFW_KEY_F5              , EventKeyboard::KeyCode::KEY_F5            },
    { GLFW_KEY_F6              , EventKeyboard::KeyCode::KEY_F6            },
    { GLFW_KEY_F7              , EventKeyboard::KeyCode::KEY_F7            },
    { GLFW_KEY_F8              , EventKeyboard::KeyCode::KEY_F8            },
    { GLFW_KEY_F9              , EventKeyboard::KeyCode::KEY_F9            },
    { GLFW_KEY_F10             , EventKeyboard::KeyCode::KEY_F10           },
    { GLFW_KEY_F11             , EventKeyboard::KeyCode::KEY_F11           },
    { GLFW_KEY_F12             , EventKeyboard::KeyCode::KEY_F12           },
    { GLFW_KEY_F13             , EventKeyboard::KeyCode::KEY_NONE          },
    { GLFW_KEY_F14             , EventKeyboard::KeyCode::KEY_NONE          },
    { GLFW_KEY_F15             , EventKeyboard::KeyCode::KEY_NONE          },
    { GLFW_KEY_F16             , EventKeyboard::KeyCode::KEY_NONE          },
    { GLFW_KEY_F17             , EventKeyboard::KeyCode::KEY_NONE          },
    { GLFW_KEY_F18             , EventKeyboard::KeyCode::KEY_NONE          },
    { GLFW_KEY_F19             , EventKeyboard::KeyCode::KEY_NONE          },
    { GLFW_KEY_F20             , EventKeyboard::KeyCode::KEY_NONE          },
    { GLFW_KEY_F21             , EventKeyboard::KeyCode::KEY_NONE          },
    { GLFW_KEY_F22             , EventKeyboard::KeyCode::KEY_NONE          },
    { GLFW_KEY_F23             , EventKeyboard::KeyCode::KEY_NONE          },
    { GLFW_KEY_F24             , EventKeyboard::KeyCode::KEY_NONE          },
    { GLFW_KEY_F25             , EventKeyboard::KeyCode::KEY_NONE          },
    { GLFW_KEY_KP_0            , EventKeyboard::KeyCode::KEY_0             },
    { GLFW_KEY_KP_1            , EventKeyboard::KeyCode::KEY_1             },
    { GLFW_KEY_KP_2            , EventKeyboard::KeyCode::KEY_2             },
    { GLFW_KEY_KP_3            , EventKeyboard::KeyCode::KEY_3             },
    { GLFW_KEY_KP_4            , EventKeyboard::KeyCode::KEY_4             },
    { GLFW_KEY_KP_5            , EventKeyboard::KeyCode::KEY_5             },
    { GLFW_KEY_KP_6            , EventKeyboard::KeyCode::KEY_6             },
    { GLFW_KEY_KP_7            , EventKeyboard::KeyCode::KEY_7             },
    { GLFW_KEY_KP_8            , EventKeyboard::KeyCode::KEY_8             },
    { GLFW_KEY_KP_9            , EventKeyboard::KeyCode::KEY_9             },
    { GLFW_KEY_KP_DECIMAL      , EventKeyboard::KeyCode::KEY_PERIOD        },
    { GLFW_KEY_KP_DIVIDE       , EventKeyboard::KeyCode::KEY_KP_DIVIDE     },
    { GLFW_KEY_KP_MULTIPLY     , EventKeyboard::KeyCode::KEY_KP_MULTIPLY   },
    { GLFW_KEY_KP_SUBTRACT     , EventKeyboard::KeyCode::KEY_KP_MINUS      },
    { GLFW_KEY_KP_ADD          , EventKeyboard::KeyCode::KEY_KP_PLUS       },
    { GLFW_KEY_KP_ENTER        , EventKeyboard::KeyCode::KEY_KP_ENTER      },
    { GLFW_KEY_KP_EQUAL        , EventKeyboard::KeyCode::KEY_EQUAL         },
    { GLFW_KEY_LEFT_SHIFT      , EventKeyboard::KeyCode::KEY_LEFT_SHIFT         },
    { GLFW_KEY_LEFT_CONTROL    , EventKeyboard::KeyCode::KEY_LEFT_CTRL          },
    { GLFW_KEY_LEFT_ALT        , EventKeyboard::KeyCode::KEY_LEFT_ALT           },
    { GLFW_KEY_LEFT_SUPER      , EventKeyboard::KeyCode::KEY_HYPER         },
    { GLFW_KEY_RIGHT_SHIFT     , EventKeyboard::KeyCode::KEY_RIGHT_SHIFT         },
    { GLFW_KEY_RIGHT_CONTROL   , EventKeyboard::KeyCode::KEY_RIGHT_CTRL          },
    { GLFW_KEY_RIGHT_ALT       , EventKeyboard::KeyCode::KEY_RIGHT_ALT           },
    { GLFW_KEY_RIGHT_SUPER     , EventKeyboard::KeyCode::KEY_HYPER         },
    { GLFW_KEY_MENU            , EventKeyboard::KeyCode::KEY_MENU          },
    { GLFW_KEY_LAST            , EventKeyboard::KeyCode::KEY_NONE          }
};

//////////////////////////////////////////////////////////////////////////
// implement GLViewImpl
//////////////////////////////////////////////////////////////////////////


GLViewImpl::GLViewImpl(bool initglfw)
: _captured(false)
, _supportTouch(false)
, _isInRetinaMonitor(false)
, _isRetinaEnabled(false)
, _retinaFactor(1)
, _frameZoomFactor(1.0f)
, _mainWindow(nullptr)
, _monitor(nullptr)
, _mouseX(0.0f)
, _mouseY(0.0f)
{
    _viewName = "cocos2dx";
    g_keyCodeMap.clear();
    for (auto& item : g_keyCodeStructArray)
    {
        g_keyCodeMap[item.glfwKeyCode] = item.keyCode;
    }

    GLFWEventHandler::setGLViewImpl(this);
    if (initglfw)
    {
        glfwSetErrorCallback(GLFWEventHandler::onGLFWError);
        glfwInit();
    }
}

GLViewImpl::~GLViewImpl()
{
    CCLOGINFO("deallocing GLViewImpl: %p", this);
    GLFWEventHandler::setGLViewImpl(nullptr);
	glfwDestroyWindowImpl(_mainWindow);
    glfwTerminate();
}

GLViewImpl* GLViewImpl::create(const std::string& viewName)
{
    return GLViewImpl::create(viewName, false);
}

GLViewImpl* GLViewImpl::create(const std::string& viewName, bool resizable)
{
    auto ret = new (std::nothrow) GLViewImpl;
    if(ret && ret->initWithRect(viewName, Rect(0, 0, 960, 640), 1.0f, resizable)) {
        ret->autorelease();
        return ret;
    }
    CC_SAFE_DELETE(ret);
    return nullptr;
}

GLViewImpl* GLViewImpl::createWithRect(const std::string& viewName, Rect rect, float frameZoomFactor, bool resizable)
{
    auto ret = new (std::nothrow) GLViewImpl;
    if(ret && ret->initWithRect(viewName, rect, frameZoomFactor, resizable)) {
        ret->autorelease();
        return ret;
    }
    CC_SAFE_DELETE(ret);
    return nullptr;
}

GLViewImpl* GLViewImpl::createWithFullScreen(const std::string& viewName)
{
    auto ret = new (std::nothrow) GLViewImpl();
    if(ret && ret->initWithFullScreen(viewName)) {
        ret->autorelease();
        return ret;
    }
    CC_SAFE_DELETE(ret);
    return nullptr;
}

GLViewImpl* GLViewImpl::createWithFullScreen(const std::string& viewName, const GLFWvidmode &videoMode, GLFWmonitor *monitor)
{
    auto ret = new (std::nothrow) GLViewImpl();
    if(ret && ret->initWithFullscreen(viewName, videoMode, monitor)) {
        ret->autorelease();
        return ret;
    }
    CC_SAFE_DELETE(ret);
    return nullptr;
}

bool GLViewImpl::initWithRect(const std::string& viewName, Rect rect, float frameZoomFactor, bool resizable)
{
    setViewName(viewName);

    _frameZoomFactor = frameZoomFactor;

	glfwWindowHint(GLFW_RESIZABLE, resizable ? GL_TRUE : GL_FALSE);
	glfwWindowHint(GLFW_RED_BITS, _glContextAttrs.redBits);
	glfwWindowHint(GLFW_GREEN_BITS, _glContextAttrs.greenBits);
	glfwWindowHint(GLFW_BLUE_BITS, _glContextAttrs.blueBits);
	glfwWindowHint(GLFW_ALPHA_BITS, _glContextAttrs.alphaBits);
	glfwWindowHint(GLFW_DEPTH_BITS, _glContextAttrs.depthBits);
	glfwWindowHint(GLFW_STENCIL_BITS, _glContextAttrs.stencilBits);

    glfwWindowHint(GLFW_SAMPLES, _glContextAttrs.multisamplingCount);

    // Don't create gl context.
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    int neededWidth = (int)(rect.size.width * _frameZoomFactor);
    int neededHeight = (int)(rect.size.height * _frameZoomFactor);

    _mainWindow = glfwCreateWindow(neededWidth, neededHeight, _viewName.c_str(), _monitor, nullptr);

    if (_mainWindow == nullptr)
    {
        std::string message = "Can't create window";
        if (!_glfwError.empty())
        {
            message.append("\nMore info: \n");
            message.append(_glfwError);
        }

        ccMessageBox(message.c_str(), "Error launch application");
        return false;
    }

    /*
    *  Note that the created window and context may differ from what you requested,
    *  as not all parameters and hints are
    *  [hard constraints](@ref window_hints_hard).  This includes the size of the
    *  window, especially for full screen windows.  To retrieve the actual
    *  attributes of the created window and context, use queries like @ref
    *  glfwGetWindowAttrib and @ref glfwGetWindowSize.
    *
    *  see declaration glfwCreateWindow
    */
    int realW = 0, realH = 0;
    glfwGetWindowSize(_mainWindow, &realW, &realH);
    if (realW != neededWidth)
    {
        rect.size.width = realW / _frameZoomFactor;
    }
    if (realH != neededHeight)
    {
        rect.size.height = realH / _frameZoomFactor;
    }

    //glfwMakeContextCurrent(_mainWindow);

    glfwSetMouseButtonCallback(_mainWindow, GLFWEventHandler::onGLFWMouseCallBack);
    glfwSetCursorPosCallback(_mainWindow, GLFWEventHandler::onGLFWMouseMoveCallBack);
    glfwSetScrollCallback(_mainWindow, GLFWEventHandler::onGLFWMouseScrollCallback);
    glfwSetCharCallback(_mainWindow, GLFWEventHandler::onGLFWCharCallback);
    glfwSetKeyCallback(_mainWindow, GLFWEventHandler::onGLFWKeyCallback);
    glfwSetWindowPosCallback(_mainWindow, GLFWEventHandler::onGLFWWindowPosCallback);
    glfwSetFramebufferSizeCallback(_mainWindow, GLFWEventHandler::onGLFWframebuffersize);
    glfwSetWindowSizeCallback(_mainWindow, GLFWEventHandler::onGLFWWindowSizeFunCallback);
    glfwSetWindowIconifyCallback(_mainWindow, GLFWEventHandler::onGLFWWindowIconifyCallback);
    glfwSetWindowFocusCallback(_mainWindow, GLFWEventHandler::onGLFWWindowFocusCallback);

    setFrameSize(rect.size.width, rect.size.height);

	int frameBufferW = 0, frameBufferH = 0;
	glfwGetFramebufferSize(_mainWindow, &frameBufferW, &frameBufferH);

	uint32_t reset = 0;
    switch (_glContextAttrs.multisamplingCount)
    {
	case 2: reset |= BGFX_RESET_MSAA_X2; break;
	case 4: reset |= BGFX_RESET_MSAA_X4; break;
	case 8: reset |= BGFX_RESET_MSAA_X8; break;
	case 16: reset |= BGFX_RESET_MSAA_X16; break;
    default: ;
    }
	//reset |= BGFX_RESET_VSYNC;
	reset |= BGFX_RESET_FLUSH_AFTER_RENDER;
	//reset |= BGFX_RESET_FLIP_AFTER_RENDER;

	bgfx::Init init;
	init.resolution.width = frameBufferW;
	init.resolution.height = frameBufferH;
	init.resolution.reset = reset;
	init.callback = backend::CallbackBX::getInstance();
	glfwSetWindow(_mainWindow, init.platformData);
	//bgfx::renderFrame();
	//init.type = bgfx::RendererType::OpenGL;
	backend::addThreadTaskSync([=]()
	{
		if (!bgfx::init(init))
	    {
			ccMessageBox("Failed to init bgfx", "Error launch application");
	    }
		//bgfx::setDebug(BGFX_DEBUG_STATS | BGFX_DEBUG_TEXT | BGFX_DEBUG_WIREFRAME);
		bgfx::setDebug(BGFX_DEBUG_TEXT);
		bgfx::setViewClear(0, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH | BGFX_CLEAR_STENCIL);
		bgfx::setViewRect(0, 0, 0, frameBufferW, frameBufferH);
		bgfx::frame();
	});
	return true;
}

bool GLViewImpl::initWithFullScreen(const std::string& viewName)
{
    //Create fullscreen window on primary monitor at its current video mode.
    _monitor = glfwGetPrimaryMonitor();
    if (nullptr == _monitor)
        return false;

    const GLFWvidmode* videoMode = glfwGetVideoMode(_monitor);
    return initWithRect(viewName, Rect(0, 0, (float)videoMode->width, (float)videoMode->height), 1.0f, false);
}

bool GLViewImpl::initWithFullscreen(const std::string &viewname, const GLFWvidmode &videoMode, GLFWmonitor *monitor)
{
    //Create fullscreen on specified monitor at the specified video mode.
    _monitor = monitor;
    if (nullptr == _monitor)
        return false;
    
    //These are soft constraints. If the video mode is retrieved at runtime, the resulting window and context should match these exactly. If invalid attribs are passed (eg. from an outdated cache), window creation will NOT fail but the actual window/context may differ.
    glfwWindowHint(GLFW_REFRESH_RATE, videoMode.refreshRate);
    glfwWindowHint(GLFW_RED_BITS, videoMode.redBits);
    glfwWindowHint(GLFW_BLUE_BITS, videoMode.blueBits);
    glfwWindowHint(GLFW_GREEN_BITS, videoMode.greenBits);
    
    return initWithRect(viewname, Rect(0, 0, (float)videoMode.width, (float)videoMode.height), 1.0f, false);
}

bool GLViewImpl::isOpenGLReady()
{
    return nullptr != _mainWindow;
}

void GLViewImpl::end()
{
    if(_mainWindow)
    {
        glfwSetWindowShouldClose(_mainWindow,1);
        _mainWindow = nullptr;
    }
    // Release self. Otherwise, GLViewImpl could not be freed.
    release();
}

void GLViewImpl::swapBuffers()
{
	int frameBufferW = 0, frameBufferH = 0;
	glfwGetFramebufferSize(_mainWindow, &frameBufferW, &frameBufferH);
	backend::addThreadTask([=]()
	{
#if defined(COCOS2D_DEBUG) && COCOS2D_DEBUG > 0
		//bgfx::setState(BGFX_STATE_DEFAULT);
		const bgfx::Stats* stats = bgfx::getStats();

		bgfx::dbgTextClear();
		bgfx::dbgTextPrintf(0, 1, 0x0f, "Color can be changed with ANSI \x1b[9;me\x1b[10;ms\x1b[11;mc\x1b[12;ma\x1b[13;mp\x1b[14;me\x1b[0m code too.");

		bgfx::dbgTextPrintf(80, 1, 0x0f, "\x1b[;0m    \x1b[;1m    \x1b[; 2m    \x1b[; 3m    \x1b[; 4m    \x1b[; 5m    \x1b[; 6m    \x1b[; 7m    \x1b[0m");
		bgfx::dbgTextPrintf(80, 2, 0x0f, "\x1b[;8m    \x1b[;9m    \x1b[;10m    \x1b[;11m    \x1b[;12m    \x1b[;13m    \x1b[;14m    \x1b[;15m    \x1b[0m");

		bgfx::dbgTextPrintf(0, 2, 0x0f, "Backbuffer %dW x %dH in pixels, debug text %dW x %dH in characters."
			, stats->width
			, stats->height
			, stats->textWidth
			, stats->textHeight
		);

		//bgfx::dbgTextPrintf(0, 3, 0x0f, "FSize %.0f x %.0f, DRSize %.0f x %.0f, VRect %.0f, %.0f, %.0f, %.0f"
		//	, fSize.width
		//	, fSize.height
		//	, drSize.width
		//	, drSize.height
		//	, vRect.origin.x
		//	, vRect.origin.y
		//	, vRect.size.width
		//	, vRect.size.height
		//);
		bgfx::dbgTextPrintf(0, 3, 0x0f, "FBSize %d x %d"
			, frameBufferW
			, frameBufferH
		);
		//bgfx::dbgTextPrintf(0, 5, 0x0f, "nDraw: %d, maxGpuLat: %d, nDIB: %d, nDVB: %d, nFB: %d, nIB: %d"
		//	, stats->numDraw
		//	, stats->maxGpuLatency
		//	, stats->numDynamicIndexBuffers
		//	, stats->numDynamicVertexBuffers
		//	, stats->numFrameBuffers
		//	, stats->numIndexBuffers
		//);
		//bgfx::dbgTextPrintf(0, 6, 0x0f, "nPro: %d, nSh: %d, nTex: %d, nVB: %d, nVL: %d"
		//	, stats->numPrograms
		//	, stats->numShaders
		//	, stats->numTextures
		//	, stats->numVertexBuffers
		//	, stats->numVertexLayouts
		//);
#endif

		bgfx::frame();

		bgfx::reset(frameBufferW, frameBufferH);
		bgfx::setViewRect(0, 0, 0, frameBufferW, frameBufferH);

		//bgfx::setScissor();
		//bgfx::setViewClear(0, BGFX_CLEAR_NONE);
		//bgfx::setViewClear(0, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH | BGFX_CLEAR_STENCIL);
		//bgfx::setViewClear(0, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH | BGFX_CLEAR_STENCIL, 0xff0000ff, 1.0f, 0);
		//bgfx::touch(0);
	});
	//bgfx::renderFrame();
}

bool GLViewImpl::windowShouldClose()
{
    if(_mainWindow)
        return glfwWindowShouldClose(_mainWindow) ? true : false;
    else
        return true;
}

void GLViewImpl::pollEvents()
{
    glfwPollEvents();
}

void GLViewImpl::enableRetina(bool enabled)
{
// #if (CC_TARGET_PLATFORM == CC_PLATFORM_MAC)
//     _isRetinaEnabled = enabled;
//     if (_isRetinaEnabled)
//     {
//         _retinaFactor = 1;
//     }
//     else
//     {
//         _retinaFactor = 2;
//     }
//     updateFrameSize();
// #endif
}


void GLViewImpl::setIMEKeyboardState(bool /*bOpen*/)
{

}

#if CC_ICON_SET_SUPPORT
void GLViewImpl::setIcon(const std::string& filename) const {
    std::vector<std::string> vec = {filename};
    this->setIcon(vec);
}

void GLViewImpl::setIcon(const std::vector<std::string>& filelist) const {
    if (filelist.empty()) return;
    std::vector<Image*> icons;
    for (auto const& filename: filelist) {
        Image* icon = new (std::nothrow) Image();
        if (icon && icon->initWithImageFile(filename)) {
            icons.push_back(icon);
        } else {
            CC_SAFE_DELETE(icon);
        }
    }

    if (icons.empty()) return; // No valid images
    size_t iconsCount = icons.size();
    auto images = new GLFWimage[iconsCount];
    for (size_t i = 0; i < iconsCount; i++) {
        auto& image = images[i];
        auto& icon = icons[i];
        image.width = icon->getWidth();
        image.height = icon->getHeight();
        image.pixels = icon->getData();
    };

    GLFWwindow* window = this->getWindow();
    glfwSetWindowIcon(window, iconsCount, images);

    CC_SAFE_DELETE_ARRAY(images);
    for (auto& icon: icons) {
        CC_SAFE_DELETE(icon);
    }
}

void GLViewImpl::setDefaultIcon() const {
    GLFWwindow* window = this->getWindow();
    glfwSetWindowIcon(window, 0, nullptr);
}
#endif /* CC_ICON_SET_SUPPORT */

void GLViewImpl::setCursorVisible( bool isVisible )
{
    if( _mainWindow == NULL )
        return;
    
    if( isVisible )
        glfwSetInputMode(_mainWindow, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    else
        glfwSetInputMode(_mainWindow, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
}

void GLViewImpl::setFrameZoomFactor(float zoomFactor)
{
    CCASSERT(zoomFactor > 0.0f, "zoomFactor must be larger than 0");

    if (std::abs(_frameZoomFactor - zoomFactor) < FLT_EPSILON)
    {
        return;
    }

    _frameZoomFactor = zoomFactor;
    updateFrameSize();
}

float GLViewImpl::getFrameZoomFactor() const
{
    return _frameZoomFactor;
}

bool GLViewImpl::isFullscreen() const {
    return (_monitor != nullptr);
}

void GLViewImpl::setFullscreen() {
    if (this->isFullscreen()) {
        return;
    }
    _monitor = glfwGetPrimaryMonitor();
    if (nullptr == _monitor) {
        return;
    }
    const GLFWvidmode* videoMode = glfwGetVideoMode(_monitor);
    this->setFullscreen(*videoMode, _monitor);
}

void GLViewImpl::setFullscreen(int monitorIndex) {
    // set fullscreen on specific monitor
    int count = 0;
    GLFWmonitor** monitors = glfwGetMonitors(&count);
    if (monitorIndex < 0 || monitorIndex >= count) {
        return;
    }
    GLFWmonitor* monitor = monitors[monitorIndex];
    if (nullptr == monitor) {
        return;
    }
    const GLFWvidmode* videoMode = glfwGetVideoMode(monitor);
    this->setFullscreen(*videoMode, monitor);
}

void GLViewImpl::setFullscreen(const GLFWvidmode &videoMode, GLFWmonitor *monitor) {
    _monitor = monitor;
    glfwSetWindowMonitor(_mainWindow, _monitor, 0, 0, videoMode.width, videoMode.height, videoMode.refreshRate);
}

void GLViewImpl::setWindowed(int width, int height) {
    if (!this->isFullscreen()) {
        this->setFrameSize((float)width, (float)height);
    } else {
        const GLFWvidmode* videoMode = glfwGetVideoMode(_monitor);
        int xpos = 0, ypos = 0;
        glfwGetMonitorPos(_monitor, &xpos, &ypos);
        xpos += (int)((videoMode->width - width) * 0.5f);
        ypos += (int)((videoMode->height - height) * 0.5f);
        _monitor = nullptr;
        glfwSetWindowMonitor(_mainWindow, nullptr, xpos, ypos, width, height, GLFW_DONT_CARE);
#if (CC_TARGET_PLATFORM == CC_PLATFORM_MAC)
        // on mac window will sometimes lose title when windowed
        glfwSetWindowTitle(_mainWindow, _viewName.c_str());
#endif
    }
}

int GLViewImpl::getMonitorCount() const {
    int count = 0;
    glfwGetMonitors(&count);
    return count;
}

Size GLViewImpl::getMonitorSize() const {
    GLFWmonitor* monitor = _monitor;
    if (nullptr == monitor) {
        GLFWwindow* window = this->getWindow();
        monitor = glfwGetWindowMonitor(window);
    }
    if (nullptr == monitor) {
        monitor = glfwGetPrimaryMonitor();
    }
    if (nullptr != monitor) {
        const GLFWvidmode* videoMode = glfwGetVideoMode(monitor);
        Size size = Size((float)videoMode->width, (float)videoMode->height);
        return size;
    }
    return Size::ZERO;
}

void GLViewImpl::updateFrameSize()
{
    if (_screenSize.width > 0 && _screenSize.height > 0)
    {
        int w = 0, h = 0;
        glfwGetWindowSize(_mainWindow, &w, &h);

        int frameBufferW = 0, frameBufferH = 0;
        glfwGetFramebufferSize(_mainWindow, &frameBufferW, &frameBufferH);

        // if (frameBufferW == 2 * w && frameBufferH == 2 * h)
        // {
        //     if (_isRetinaEnabled)
        //     {
        //         _retinaFactor = 1;
        //     }
        //     else
        //     {
        //         _retinaFactor = 2;
        //     }
        //     glfwSetWindowSize(_mainWindow, _screenSize.width/2 * _retinaFactor * _frameZoomFactor, _screenSize.height/2 * _retinaFactor * _frameZoomFactor);

        //     _isInRetinaMonitor = true;
        // }
        // else
        {
            if (_isInRetinaMonitor)
            {
                _retinaFactor = 1;
            }
            glfwSetWindowSize(_mainWindow, (int)(_screenSize.width * _retinaFactor * _frameZoomFactor), (int)(_screenSize.height *_retinaFactor * _frameZoomFactor));

            _isInRetinaMonitor = false;
        }
    }
}

void GLViewImpl::setFrameSize(float width, float height)
{
    GLView::setFrameSize(width, height);
    updateFrameSize();
}

void GLViewImpl::setViewPortInPoints(float x , float y , float w , float h)
{
    Viewport vp;
    vp.x = (int)(x * _scaleX * _retinaFactor * _frameZoomFactor + _viewPortRect.origin.x * _retinaFactor * _frameZoomFactor);
    vp.y = (int)(y * _scaleY * _retinaFactor  * _frameZoomFactor + _viewPortRect.origin.y * _retinaFactor * _frameZoomFactor);
    vp.w = (unsigned int)(w * _scaleX * _retinaFactor * _frameZoomFactor);
    vp.h = (unsigned int)(h * _scaleY * _retinaFactor * _frameZoomFactor);
    Camera::setDefaultViewport(vp);
}

void GLViewImpl::setScissorInPoints(float x , float y , float w , float h)
{
    auto x1 = (int)(x * _scaleX * _retinaFactor * _frameZoomFactor + _viewPortRect.origin.x * _retinaFactor * _frameZoomFactor);
    auto y1 = (int)(y * _scaleY * _retinaFactor  * _frameZoomFactor + _viewPortRect.origin.y * _retinaFactor * _frameZoomFactor);
    auto width1 = (unsigned int)(w * _scaleX * _retinaFactor * _frameZoomFactor);
    auto height1 = (unsigned int)(h * _scaleY * _retinaFactor * _frameZoomFactor);
    auto renderer = Director::getInstance()->getRenderer();
    renderer->setScissorRect(x1, y1, width1, height1);

}

Rect GLViewImpl::getScissorRect() const
{
    auto renderer = Director::getInstance()->getRenderer();
    auto& rect = renderer->getScissorRect();

    float x = (rect.x - _viewPortRect.origin.x * _retinaFactor * _frameZoomFactor) / (_scaleX * _retinaFactor * _frameZoomFactor);
    float y = (rect.y - _viewPortRect.origin.y * _retinaFactor * _frameZoomFactor) / (_scaleY * _retinaFactor  * _frameZoomFactor);
    float w = rect.width / (_scaleX * _retinaFactor * _frameZoomFactor);
    float h = rect.height / (_scaleY * _retinaFactor  * _frameZoomFactor);
    return Rect(x, y, w, h);
}

void GLViewImpl::onGLFWError(int errorID, const char* errorDesc)
{
    if (_mainWindow)
    {
        _glfwError = StringUtils::format("GLFWError #%d Happen, %s", errorID, errorDesc);
    }
    else
    {
        _glfwError.append(StringUtils::format("GLFWError #%d Happen, %s\n", errorID, errorDesc));
    }
    CCLOGERROR("%s", _glfwError.c_str());
}

void GLViewImpl::onGLFWMouseCallBack(GLFWwindow* /*window*/, int button, int action, int /*modify*/)
{
    if(GLFW_MOUSE_BUTTON_LEFT == button)
    {
        if(GLFW_PRESS == action)
        {
            _captured = true;
            if (this->getViewPortRect().equals(Rect::ZERO) || this->getViewPortRect().containsPoint(Vec2(_mouseX,_mouseY)))
            {
                intptr_t id = 0;
                this->handleTouchesBegin(1, &id, &_mouseX, &_mouseY);
            }
        }
        else if(GLFW_RELEASE == action)
        {
            if (_captured)
            {
                _captured = false;
                intptr_t id = 0;
                this->handleTouchesEnd(1, &id, &_mouseX, &_mouseY);
            }
        }
    }
    
    //Because OpenGL and cocos2d-x uses different Y axis, we need to convert the coordinate here
    float cursorX = (_mouseX - _viewPortRect.origin.x) / _scaleX;
    float cursorY = (_viewPortRect.origin.y + _viewPortRect.size.height - _mouseY) / _scaleY;

    if(GLFW_PRESS == action)
    {
        EventMouse event(EventMouse::MouseEventType::MOUSE_DOWN);
        event.setCursorPosition(cursorX, cursorY);
        event.setMouseButton(static_cast<cocos2d::EventMouse::MouseButton>(button));
        Director::getInstance()->getEventDispatcher()->dispatchEvent(&event);
    }
    else if(GLFW_RELEASE == action)
    {
        EventMouse event(EventMouse::MouseEventType::MOUSE_UP);
        event.setCursorPosition(cursorX, cursorY);
        event.setMouseButton(static_cast<cocos2d::EventMouse::MouseButton>(button));
        Director::getInstance()->getEventDispatcher()->dispatchEvent(&event);
    }
}

void GLViewImpl::onGLFWMouseMoveCallBack(GLFWwindow* window, double x, double y)
{
    _mouseX = (float)x;
    _mouseY = (float)y;

    _mouseX /= this->getFrameZoomFactor();
    _mouseY /= this->getFrameZoomFactor();

    if (_isInRetinaMonitor)
    {
        if (_retinaFactor == 1)
        {
            _mouseX *= 2;
            _mouseY *= 2;
        }
    }

    if (_captured)
    {
        intptr_t id = 0;
        this->handleTouchesMove(1, &id, &_mouseX, &_mouseY);
    }
    
    //Because OpenGL and cocos2d-x uses different Y axis, we need to convert the coordinate here
    float cursorX = (_mouseX - _viewPortRect.origin.x) / _scaleX;
    float cursorY = (_viewPortRect.origin.y + _viewPortRect.size.height - _mouseY) / _scaleY;

    EventMouse event(EventMouse::MouseEventType::MOUSE_MOVE);
    // Set current button
    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS)
    {
        event.setMouseButton(static_cast<cocos2d::EventMouse::MouseButton>(GLFW_MOUSE_BUTTON_LEFT));
    }
    else if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS)
    {
        event.setMouseButton(static_cast<cocos2d::EventMouse::MouseButton>(GLFW_MOUSE_BUTTON_RIGHT));
    }
    else if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_MIDDLE) == GLFW_PRESS)
    {
        event.setMouseButton(static_cast<cocos2d::EventMouse::MouseButton>(GLFW_MOUSE_BUTTON_MIDDLE));
    }
    event.setCursorPosition(cursorX, cursorY);
    Director::getInstance()->getEventDispatcher()->dispatchEvent(&event);
}

void GLViewImpl::onGLFWMouseScrollCallback(GLFWwindow* /*window*/, double x, double y)
{
    EventMouse event(EventMouse::MouseEventType::MOUSE_SCROLL);
    //Because OpenGL and cocos2d-x uses different Y axis, we need to convert the coordinate here
    float cursorX = (_mouseX - _viewPortRect.origin.x) / _scaleX;
    float cursorY = (_viewPortRect.origin.y + _viewPortRect.size.height - _mouseY) / _scaleY;
    event.setScrollData((float)x, -(float)y);
    event.setCursorPosition(cursorX, cursorY);
    Director::getInstance()->getEventDispatcher()->dispatchEvent(&event);
}

void GLViewImpl::onGLFWKeyCallback(GLFWwindow* /*window*/, int key, int /*scancode*/, int action, int /*mods*/)
{
    if (GLFW_REPEAT != action)
    {
        EventKeyboard event(g_keyCodeMap[key], GLFW_PRESS == action);
        auto dispatcher = Director::getInstance()->getEventDispatcher();
        dispatcher->dispatchEvent(&event);
    }

    if (GLFW_RELEASE != action)
    {
        switch (g_keyCodeMap[key])
        {
        case EventKeyboard::KeyCode::KEY_BACKSPACE:
            IMEDispatcher::sharedDispatcher()->dispatchDeleteBackward();
            break;
        case EventKeyboard::KeyCode::KEY_HOME:
        case EventKeyboard::KeyCode::KEY_KP_HOME:
        case EventKeyboard::KeyCode::KEY_DELETE:
        case EventKeyboard::KeyCode::KEY_KP_DELETE:
        case EventKeyboard::KeyCode::KEY_END:
        case EventKeyboard::KeyCode::KEY_LEFT_ARROW:
        case EventKeyboard::KeyCode::KEY_RIGHT_ARROW:
        case EventKeyboard::KeyCode::KEY_ESCAPE:
            IMEDispatcher::sharedDispatcher()->dispatchControlKey(g_keyCodeMap[key]);
            break;
        default:
            break;
        }
    }
}

void GLViewImpl::onGLFWCharCallback(GLFWwindow* /*window*/, unsigned int character)
{
    char16_t wcharString[2] = { (char16_t) character, 0 };
    std::string utf8String;

    StringUtils::UTF16ToUTF8( wcharString, utf8String );
    static std::set<std::string> controlUnicode = {
        "\xEF\x9C\x80", // up
        "\xEF\x9C\x81", // down
        "\xEF\x9C\x82", // left
        "\xEF\x9C\x83", // right
        "\xEF\x9C\xA8", // delete
        "\xEF\x9C\xA9", // home
        "\xEF\x9C\xAB", // end
        "\xEF\x9C\xAC", // pageup
        "\xEF\x9C\xAD", // pagedown
        "\xEF\x9C\xB9"  // clear
    };
    // Check for send control key
    if (controlUnicode.find(utf8String) == controlUnicode.end())
    {
        IMEDispatcher::sharedDispatcher()->dispatchInsertText( utf8String.c_str(), utf8String.size() );
    }
}

void GLViewImpl::onGLFWWindowPosCallback(GLFWwindow* /*window*/, int /*x*/, int /*y*/)
{
    Director::getInstance()->setViewport();
}

void GLViewImpl::onGLFWframebuffersize(GLFWwindow* window, int w, int h)
{
    float frameSizeW = _screenSize.width;
    float frameSizeH = _screenSize.height;
    float factorX = frameSizeW / w * _retinaFactor * _frameZoomFactor;
    float factorY = frameSizeH / h * _retinaFactor * _frameZoomFactor;

    if (std::abs(factorX - 0.5f) < FLT_EPSILON && std::abs(factorY - 0.5f) < FLT_EPSILON)
    {
        _isInRetinaMonitor = true;
        if (_isRetinaEnabled)
        {
            _retinaFactor = 1;
        }
        else
        {
            _retinaFactor = 2;
        }

        glfwSetWindowSize(window, static_cast<int>(frameSizeW * 0.5f * _retinaFactor * _frameZoomFactor), static_cast<int>(frameSizeH * 0.5f * _retinaFactor * _frameZoomFactor));
    }
    else if (std::abs(factorX - 2.0f) < FLT_EPSILON && std::abs(factorY - 2.0f) < FLT_EPSILON)
    {
        _isInRetinaMonitor = false;
        _retinaFactor = 1;
        glfwSetWindowSize(window, static_cast<int>(frameSizeW * _retinaFactor * _frameZoomFactor), static_cast<int>(frameSizeH * _retinaFactor * _frameZoomFactor));
    }
}

void GLViewImpl::onGLFWWindowSizeFunCallback(GLFWwindow* /*window*/, int width, int height)
{
    if (width && height && _resolutionPolicy != ResolutionPolicy::UNKNOWN)
    {
        Size baseDesignSize = _designResolutionSize;
        ResolutionPolicy baseResolutionPolicy = _resolutionPolicy;

        float frameWidth = width / _frameZoomFactor;
        float frameHeight = height / _frameZoomFactor;
        setFrameSize(frameWidth, frameHeight);
        setDesignResolutionSize(baseDesignSize.width, baseDesignSize.height, baseResolutionPolicy);
        Director::getInstance()->setViewport();
        Director::getInstance()->getEventDispatcher()->dispatchCustomEvent(GLViewImpl::EVENT_WINDOW_RESIZED, nullptr);
    }
}

void GLViewImpl::onGLFWWindowIconifyCallback(GLFWwindow* /*window*/, int iconified)
{
    if (iconified == GLFW_TRUE)
    {
        Application::getInstance()->applicationDidEnterBackground();
    }
    else
    {
        Application::getInstance()->applicationWillEnterForeground();
    }
}

void GLViewImpl::onGLFWWindowFocusCallback(GLFWwindow* /*window*/, int focused)
{
    if (focused == GLFW_TRUE)
    {
        Director::getInstance()->getEventDispatcher()->dispatchCustomEvent(GLViewImpl::EVENT_WINDOW_FOCUSED, nullptr);
    }
    else
    {
        Director::getInstance()->getEventDispatcher()->dispatchCustomEvent(GLViewImpl::EVENT_WINDOW_UNFOCUSED, nullptr);
    }
}

// helper
bool GLViewImpl::initGlew()
{
    return true;
}

NS_CC_END // end of namespace cocos2d;

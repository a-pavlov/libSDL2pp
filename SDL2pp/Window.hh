/*
  libSDL2pp - C++11 bindings/wrapper for SDL2
  Copyright (C) 2013-2017 Dmitry Marakasov <amdmi3@amdmi3.ru>

  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented; you must not
     claim that you wrote the original software. If you use this software
     in a product, an acknowledgment in the product documentation would be
     appreciated but is not required.
  2. Altered source versions must be plainly marked as such, and must not be
     misrepresented as being the original software.
  3. This notice may not be removed or altered from any source distribution.
*/

#ifndef SDL2PP_WINDOW_HH
#define SDL2PP_WINDOW_HH

#include <string>
#include <cassert>

#include <SDL_version.h>
#include <SDL_stdinc.h>
#include <SDL_video.h>

#include <SDL2pp/Point.hh>
#include <SDL2pp/Export.hh>
#include <SDL2pp/Surface.hh>

struct SDL_Window;

namespace SDL2pp {

////////////////////////////////////////////////////////////
/// \brief GUI window object
///
/// \ingroup windows
///
/// \headerfile SDL2pp/Window.hh
///
/// SDL2pp::Window is a wrapper around SDL_Window structure
/// and related functions which take care of GUI window
/// management.
///
/// Usage example:
/// \code
/// {
///     SDL2pp::SDL sdl(SDL_INIT_VIDEO);
///     SDL2pp::Window window("Test Window",
///                           SDL_WINDOWPOS_UNDEFINED,
///                           SDL_WINDOWPOS_UNDEFINED,
///                           640,
///                           480,
///                           SDL_WINDOW_OPENGL);
///
///     SDL_Delay(3000); // show window for 3 seconds
/// }
/// \endcode
///
////////////////////////////////////////////////////////////
class Window {
private:
	SDL_Window* window_; ///< Managed SDL2_Window object

public:
	////////////////////////////////////////////////////////////
	/// \brief Construct from existing SDL_Window structure
	///
	/// \param[in] window Existing SDL_Window to manage
	///
	////////////////////////////////////////////////////////////
	explicit Window(SDL_Window* window) : window_(window) {
		assert(window);
	}

	////////////////////////////////////////////////////////////
	/// \brief Create window with specified title and dimensions
	///
	/// \param[in] title Title of the windows, in UTF-8 encoding
	/// \param[in] x X position of the window, SDL_WINDOWPOS_CENTERED, or SDL_WINDOWPOS_UNDEFINED
	/// \param[in] y X position of the window, SDL_WINDOWPOS_CENTERED, or SDL_WINDOWPOS_UNDEFINED
	/// \param[in] w Width of the window
	/// \param[in] h Height of the window
	/// \param[in] flags 0 or more SDL_WindowFlags OR'd together
	///
	/// \throws SDL2pp::Exception
	///
	/// \see http://wiki.libsdl.org/SDL_CreateWindow
	///
	////////////////////////////////////////////////////////////
	Window(const std::string& title, int x, int y, int w, int h, Uint32 flags) {
		if ((window_ = SDL_CreateWindow(title.c_str(), x, y, w, h, flags)) == nullptr)
			throw Exception("SDL_CreateWindow");
	}

	////////////////////////////////////////////////////////////
	/// \brief Destructor
	///
	/// \see http://wiki.libsdl.org/SDL_DestroyWindow
	///
	////////////////////////////////////////////////////////////
	virtual ~Window() {
		if (window_ != nullptr)
			SDL_DestroyWindow(window_);
	}

	////////////////////////////////////////////////////////////
	/// \brief Move constructor
	///
	/// \param[in] other SDL2pp::Window object to move data from
	///
	////////////////////////////////////////////////////////////
	Window(Window&& other) noexcept : window_(other.window_) {
		other.window_ = nullptr;
	}

	////////////////////////////////////////////////////////////
	/// \brief Move assignment operator
	///
	/// \param[in] other SDL2pp::Window object to move data from
	///
	/// \returns Reference to self
	///
	////////////////////////////////////////////////////////////
	Window& operator=(Window&& other) noexcept {
		if (&other == this)
			return *this;
		if (window_ != nullptr)
			SDL_DestroyWindow(window_);
		window_ = other.window_;
		other.window_ = nullptr;
		return *this;
	}

	////////////////////////////////////////////////////////////
	/// \brief Deleted copy constructor
	///
	/// This class is not copyable
	///
	////////////////////////////////////////////////////////////
	Window(const Window& other) = delete;

	////////////////////////////////////////////////////////////
	/// \brief Deleted assignment operator
	///
	/// This class is not copyable
	///
	////////////////////////////////////////////////////////////
	Window& operator=(const Window& other) = delete;

	////////////////////////////////////////////////////////////
	/// \brief Get pointer to managed SDL_Window structure
	///
	/// \returns Pointer to managed SDL_Window structure
	///
	////////////////////////////////////////////////////////////
	SDL_Window* Get() const {
		return window_;
	}

	////////////////////////////////////////////////////////////
	/// \brief Get dimensions of the window
	///
	/// \returns SDL2pp::Point representing dimensions (width and
	///          height) of the window in pixels
	///
	/// \see http://wiki.libsdl.org/SDL_GetWindowSize
	///
	////////////////////////////////////////////////////////////
	Point GetSize() const {
		int w, h;
		SDL_GetWindowSize(window_, &w, &h);
		return Point(w, h);
	}

	////////////////////////////////////////////////////////////
	/// \brief Get width of the window
	///
	/// \returns Width of the window in pixels
	///
	/// \see http://wiki.libsdl.org/SDL_GetWindowSize
	///
	////////////////////////////////////////////////////////////
	int GetWidth() const {
		int w;
		SDL_GetWindowSize(window_, &w, nullptr);
		return w;
	}

	////////////////////////////////////////////////////////////
	/// \brief Get height of the window
	///
	/// \returns Height of the window in pixels
	///
	/// \see http://wiki.libsdl.org/SDL_GetWindowSize
	///
	////////////////////////////////////////////////////////////
	int GetHeight() const {
		int h;
		SDL_GetWindowSize(window_, nullptr, &h);
		return h;
	}

	////////////////////////////////////////////////////////////
	/// \brief Get drawable dimensions of the window
	///
	/// \returns SDL2pp::Point representing dimensions (width and
	///          height) of the window drawable area in pixels
	///
	/// \see http://wiki.libsdl.org/SDL_GL_GetDrawableSize
	///
	////////////////////////////////////////////////////////////
	Point GetDrawableSize() const {
		int w, h;
		SDL_GL_GetDrawableSize(window_, &w, &h);
		return Point(w, h);
	}

	////////////////////////////////////////////////////////////
	/// \brief Get drawable width of the window
	///
	/// \returns Width of the window drawable area in pixels
	///
	/// \see http://wiki.libsdl.org/SDL_GL_GetDrawableSize
	///
	////////////////////////////////////////////////////////////
	int GetDrawableWidth() const {
		int w;
		SDL_GL_GetDrawableSize(window_, &w, nullptr);
		return w;
	}

	////////////////////////////////////////////////////////////
	/// \brief Get drawable height of the window
	///
	/// \returns Height of the window drawable area in pixels
	///
	/// \see http://wiki.libsdl.org/SDL_GL_GetDrawableSize
	///
	////////////////////////////////////////////////////////////
	int GetDrawableHeight() const {
		int h;
		SDL_GL_GetDrawableSize(window_, nullptr, &h);
		return h;
	}

	////////////////////////////////////////////////////////////
	/// \brief Set window title
	///
	/// \param[in] title New window title in UTF-8 encoding
	///
	/// \returns Reference to self
	///
	/// \see http://wiki.libsdl.org/SDL_SetWindowTitle
	///
	////////////////////////////////////////////////////////////
	Window& SetTitle(const std::string& title) {
		SDL_SetWindowTitle(window_, title.c_str());
		return *this;
	}

	////////////////////////////////////////////////////////////
	/// \brief Get window title
	///
	/// \returns Title of the window in UTF-8 format or "" if there is no title
	///
	/// \see http://wiki.libsdl.org/SDL_GetWindowTitle
	///
	////////////////////////////////////////////////////////////
	std::string GetTitle() const {
		return SDL_GetWindowTitle(window_);
	}

	////////////////////////////////////////////////////////////
	/// \brief Make a window as large as possible
	///
	/// \returns Reference to self
	///
	/// \see http://wiki.libsdl.org/SDL_MaximizeWindow
	///
	////////////////////////////////////////////////////////////
	Window& Maximize() {
		SDL_MaximizeWindow(window_);
		return *this;
	}

	////////////////////////////////////////////////////////////
	/// \brief Minimize a window to an iconic representation
	///
	/// \returns Reference to self
	///
	/// \see http://wiki.libsdl.org/SDL_MinimizeWindow
	///
	////////////////////////////////////////////////////////////
	Window& Minimize() {
		SDL_MinimizeWindow(window_);
		return *this;
	}

	////////////////////////////////////////////////////////////
	/// \brief Hide a window
	///
	/// \returns Reference to self
	///
	/// \see http://wiki.libsdl.org/SDL_HideWindow
	///
	////////////////////////////////////////////////////////////
	Window& Hide() {
		SDL_HideWindow(window_);
		return *this;
	}

	////////////////////////////////////////////////////////////
	/// \brief Restore the size and position of a minimized or maximized window
	///
	/// \returns Reference to self
	///
	/// \see http://wiki.libsdl.org/SDL_RestoreWindow
	///
	////////////////////////////////////////////////////////////
	Window& Restore() {
		SDL_RestoreWindow(window_);
		return *this;
	}

	////////////////////////////////////////////////////////////
	/// \brief Raise a window above other windows and set the input focus
	///
	/// \returns Reference to self
	///
	/// \see http://wiki.libsdl.org/SDL_RaiseWindow
	///
	////////////////////////////////////////////////////////////
	Window& Raise() {
		SDL_RaiseWindow(window_);
		return *this;
	}

	////////////////////////////////////////////////////////////
	/// \brief Show a window
	///
	/// \returns Reference to self
	///
	/// \see http://wiki.libsdl.org/SDL_ShowWindow
	///
	////////////////////////////////////////////////////////////
	Window& Show() {
		SDL_ShowWindow(window_);
		return *this;
	}

	////////////////////////////////////////////////////////////
	/// \brief Set a window's fullscreen state
	///
	/// \param[in] flags SDL_WINDOW_FULLSCREEN, SDL_WINDOW_FULLSCREEN_DESKTOP or 0
	///
	/// \returns Reference to self
	///
	/// \throws SDL2pp::Exception
	///
	/// \see http://wiki.libsdl.org/SDL_SetWindowFullscreen
	///
	////////////////////////////////////////////////////////////
	Window& SetFullscreen(Uint32 flags) {
		if (SDL_SetWindowFullscreen(window_, flags) != 0)
			throw Exception("SDL_SetWindowFullscreen");
		return *this;
	}

	////////////////////////////////////////////////////////////
	/// \brief Set the size of a window's client area
	///
	/// \param[in] w Width of the window in pixels
	/// \param[in] h Height of the window in pixels
	///
	/// \returns Reference to self
	///
	/// \see http://wiki.libsdl.org/SDL_SetWindowSize
	///
	////////////////////////////////////////////////////////////
	Window& SetSize(int w, int h) {
		SDL_SetWindowSize(window_, w, h);
		return *this;
	}

	////////////////////////////////////////////////////////////
	/// \brief Set the size of a window's client area
	///
	/// \param[in] size Point representing window dimensions
	///
	/// \returns Reference to self
	///
	/// \see http://wiki.libsdl.org/SDL_SetWindowSize
	///
	////////////////////////////////////////////////////////////
	Window& SetSize(const Point& size) {
		SDL_SetWindowSize(window_, size.x, size.y);
		return *this;
	}

	////////////////////////////////////////////////////////////
	/// \brief Get the brightness (gamma multiplier) for the display that owns a given window
	///
	/// \returns Brightness for the display where 0.0 is completely dark and 1.0 is normal brightness
	///
	/// \see http://wiki.libsdl.org/SDL_GetWindowBrightness
	///
	////////////////////////////////////////////////////////////
	float GetBrightness() const {
		return SDL_GetWindowBrightness(window_);
	}

	////////////////////////////////////////////////////////////
	/// \brief Set the brightness (gamma multiplier) for the display that owns a given window
	///
	/// \param[in] brightness Brightness value to set where 0.0 is completely dark and 1.0 is normal brightness
	///
	/// \returns Reference to self
	///
	/// \throws SDL2pp::Exception
	///
	/// \see http://wiki.libsdl.org/SDL_SetWindowBrightness
	///
	////////////////////////////////////////////////////////////
	Window& SetBrightness(float brightness) {
		if (SDL_SetWindowBrightness(window_, brightness) != 0)
			throw Exception("SDL_SetWindowBrightness");
		return *this;
	}

	////////////////////////////////////////////////////////////
	/// \brief Get the position of a window
	///
	/// \returns Point representing window position
	///
	/// \see http://wiki.libsdl.org/SDL_GetWindowPosition
	///
	////////////////////////////////////////////////////////////
	Point GetPosition() const {
		int x, y;
		SDL_GetWindowPosition(window_, &x, &y);
		return Point(x, y);
	}

	////////////////////////////////////////////////////////////
	/// \brief Set the position of a window
	///
	/// \param[in] x X coordinate of the window, SDL_WINDOWPOS_CENTERED, or SDL_WINDOWPOS_UNDEFINED
	/// \param[in] y Y coordinate of the window, SDL_WINDOWPOS_CENTERED, or SDL_WINDOWPOS_UNDEFINED
	///
	/// \returns Reference to self
	///
	/// \see http://wiki.libsdl.org/SDL_SetWindowPosition
	///
	////////////////////////////////////////////////////////////
	Window& SetPosition(int x, int y) {
		SDL_SetWindowPosition(window_, x, y);
		return *this;
	}

	////////////////////////////////////////////////////////////
	/// \brief Set the position of a window
	///
	/// \param[in] pos Point representin position of the a window
	///
	/// \returns Reference to self
	///
	/// \see http://wiki.libsdl.org/SDL_SetWindowPosition
	///
	////////////////////////////////////////////////////////////
	Window& SetPosition(const Point& pos) {
		SDL_SetWindowPosition(window_, pos.x, pos.y);
		return *this;
	}

	////////////////////////////////////////////////////////////
	/// \brief Get the minimum size of a window's client area
	///
	/// \returns Point representing minimum area of window's client area
	///
	/// \see http://wiki.libsdl.org/SDL_GetWindowMinimumSize
	///
	////////////////////////////////////////////////////////////
	Point GetMinimumSize() const {
		int w, h;
		SDL_GetWindowMinimumSize(window_, &w, &h);
		return Point(w, h);
	}

	////////////////////////////////////////////////////////////
	/// \brief Set the minimum size of a window's client area
	///
	/// \param[in] w Minimum width of the window in pixels
	/// \param[in] h Minimum height of the window in pixels
	///
	/// \returns Reference to self
	///
	/// \see http://wiki.libsdl.org/SDL_SetWindowMinimumSize
	///
	////////////////////////////////////////////////////////////
	Window& SetMinimumSize(int w, int h) {
		SDL_SetWindowMinimumSize(window_, w, h);
		return *this;
	}

	////////////////////////////////////////////////////////////
	/// \brief Set the minimum size of a window's client area
	///
	/// \param[in] size Minimum area of the window in pixels
	///
	/// \returns Reference to self
	///
	/// \see http://wiki.libsdl.org/SDL_SetWindowMinimumSize
	///
	////////////////////////////////////////////////////////////
	Window& SetMinimumSize(const Point& size) {
		SDL_SetWindowMinimumSize(window_, size.x, size.y);
		return *this;
	}

	////////////////////////////////////////////////////////////
	/// \brief Get the maximum size of a window's client area
	///
	/// \returns Point representing maximum area of window's client area
	///
	/// \see http://wiki.libsdl.org/SDL_GetWindowMaximumSize
	///
	////////////////////////////////////////////////////////////
	Point GetMaximumSize() const {
		int w, h;
		SDL_GetWindowMaximumSize(window_, &w, &h);
		return Point(w, h);
	}

	////////////////////////////////////////////////////////////
	/// \brief Set the maximum size of a window's client area
	///
	/// \param[in] w Maximum width of the window in pixels
	/// \param[in] h Maximum height of the window in pixels
	///
	/// \returns Reference to self
	///
	/// \see http://wiki.libsdl.org/SDL_SetWindowMaximumSize
	///
	////////////////////////////////////////////////////////////
	Window& SetMaximumSize(int w, int h) {
		SDL_SetWindowMaximumSize(window_, w, h);
		return *this;
	}

	////////////////////////////////////////////////////////////
	/// \brief Set the maximum size of a window's client area
	///
	/// \param[in] size Maximum area of the window in pixels
	///
	/// \returns Reference to self
	///
	/// \see http://wiki.libsdl.org/SDL_SetWindowMaximumSize
	///
	////////////////////////////////////////////////////////////
	Window& SetMaximumSize(const Point& size) {
		SDL_SetWindowMaximumSize(window_, size.x, size.y);
		return *this;
	}

	////////////////////////////////////////////////////////////
	/// \brief Get a window's input grab mode
	///
	/// \returns True if input is grabbed, flase otherwise
	///
	/// \see http://wiki.libsdl.org/SDL_GetWindowGrab
	///
	////////////////////////////////////////////////////////////
	bool GetGrab() const {
		return SDL_GetWindowGrab(window_) == SDL_TRUE;
	}

	////////////////////////////////////////////////////////////
	/// \brief Set a window's input grab mode
	///
	/// \param[in] grabbed True to grab input, false to release input
	///
	/// \returns Reference to self
	///
	/// \see http://wiki.libsdl.org/SDL_SetWindowGrab
	///
	////////////////////////////////////////////////////////////
	Window& SetGrab(bool grabbed) {
		SDL_SetWindowGrab(window_, grabbed ? SDL_TRUE : SDL_FALSE);
		return *this;
	}

	////////////////////////////////////////////////////////////
	/// \brief Get the index of the display associated with a window
	///
	/// \returns Index of the display containing the center of the window
	///
	/// \throws SDL2pp::Exception
	///
	/// \see http://wiki.libsdl.org/SDL_GetWindowDisplayIndex
	///
	////////////////////////////////////////////////////////////
	int GetDisplayIndex() const {
		int index = SDL_GetWindowDisplayIndex(window_);
		if (index < 0)
			throw SDL2pp::Exception("SDL_GetWindowDisplayIndex");
		return index;
	}

	////////////////////////////////////////////////////////////
	/// \brief Get information about the display mode to use when a
	///        window is visible at fullscreen
	///
	/// \param[out] mode SDL_DisplayMode structure filled in with the
	///                  fullscreen display mode
	///
	/// \throws SDL2pp::Exception
	///
	/// \see http://wiki.libsdl.org/SDL_GetWindowDisplayMode
	///
	////////////////////////////////////////////////////////////
	void GetDisplayMode(SDL_DisplayMode& mode) const {
		if (SDL_GetWindowDisplayMode(window_, &mode) != 0)
			throw SDL2pp::Exception("SDL_GetWindowDisplayMode");
	}

	////////////////////////////////////////////////////////////
	/// \brief Get the window flags
	///
	/// \returns Mask of the SDL_WindowFlags associated with window
	///
	/// \see http://wiki.libsdl.org/SDL_GetWindowFlags
	///
	////////////////////////////////////////////////////////////
	Uint32 GetFlags() const {
		return SDL_GetWindowFlags(window_);
	}

	////////////////////////////////////////////////////////////
	/// \brief Set the icon for a window
	///
	/// \param[in] icon Surface containing the icon for the window
	///
	/// \returns Reference to self
	///
	/// \see http://wiki.libsdl.org/SDL_SetWindowIcon
	///
	/// Icon surface may be destroyed after calling this function
	///
	////////////////////////////////////////////////////////////
	Window& SetIcon(const Surface& icon) {
		SDL_SetWindowIcon(window_, icon.Get());
		return *this;
	}

	////////////////////////////////////////////////////////////
	/// \brief Set the border state of a window
	///
	/// \param[in] bordered False to remove border, true to add border
	///
	/// \returns Reference to self
	///
	/// \see http://wiki.libsdl.org/SDL_SetWindowBordered
	///
	////////////////////////////////////////////////////////////
	Window& SetBordered(bool bordered = true) {
		SDL_SetWindowBordered(window_, bordered ? SDL_TRUE : SDL_FALSE);
		return *this;
	}

#if SDL_VERSION_ATLEAST(2, 0, 5)
	////////////////////////////////////////////////////////////
	/// \brief Set the opacity for a window
	///
	/// \param[in] opacity The opacity value (0.0f - transparent, 1.0f - opaque)
	///
	/// \returns Reference to self
	///
	/// \throws SDL2pp::Exception
	///
	/// \see http://wiki.libsdl.org/SDL_SetWindowOpacity
	///
	////////////////////////////////////////////////////////////
	Window& SetOpacity(float opacity = 1.0f) {
		if (SDL_SetWindowOpacity(window_, opacity))
			throw SDL2pp::Exception("SDL_SetWindowOpacity");
		return *this;
	}

	////////////////////////////////////////////////////////////
	/// \brief Get the opacity of a window
	///
	/// \returns Opacity value (0.0f - transparent, 1.0f - opaque)
	///
	/// \throws SDL2pp:Exception
	///
	/// \see http://wiki.libsdl.org/SDL_GetWindowOpacity
	///
	////////////////////////////////////////////////////////////
	float GetOpacity() const {
		float opacity;
		if (SDL_GetWindowOpacity(window_, &opacity) == -1)
			throw SDL2pp::Exception("SDL_GetWindowOpacity");

		return opacity;
	}


	////////////////////////////////////////////////////////////
	/// \brief Set user-resizable state of a window
	///
	/// \param[in] resizable True to allow resizing, false to disallow
	///
	/// \returns Reference to self
	///
	/// \see http://wiki.libsdl.org/SDL_SetWindowResizable
	///
	////////////////////////////////////////////////////////////
	Window& SetResizable(bool resizable = true) {
		SDL_SetWindowResizable(window_, resizable ? SDL_TRUE : SDL_FALSE);
		return *this;
	}
#endif
};

}

#endif

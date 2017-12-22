/*
  libSDL2pp - C++11 bindings/wrapper for SDL2
  Copyright (C) 2013-2016 Dmitry Marakasov <amdmi3@amdmi3.ru>

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

#ifndef SDL2PP_RENDERER_HH
#define SDL2PP_RENDERER_HH

#include <SDL_stdinc.h>
#include <SDL_blendmode.h>

#include <SDL2pp/Config.hh>
#include <SDL2pp/Optional.hh>
#include <SDL2pp/Point.hh>
#include <SDL2pp/Rect.hh>
#include <SDL2pp/Export.hh>
#include <SDL2pp/Color.hh>
#include <SDL2pp/Exception.hh>
#include <SDL2pp/Rect.hh>
#include <SDL2pp/Surface.hh>
#include <SDL2pp/Texture.hh>
#include <SDL2pp/Window.hh>

#ifdef SDL2PP_WITH_IMAGE
#include <SDL2/SDL_image.h>
#endif

struct SDL_RendererInfo;
struct SDL_Renderer;

namespace SDL2pp {

////////////////////////////////////////////////////////////
/// \brief 2D rendering context
///
/// \ingroup rendering
///
/// \headerfile SDL2pp/Renderer.hh
///
////////////////////////////////////////////////////////////
class Renderer {
private:
	SDL_Renderer* renderer_; ///< Managed SDL_Renderer object

public:
	////////////////////////////////////////////////////////////
	/// \brief Construct from existing SDL_Renderer structure
	///
	/// \param[in] renderer Existing SDL_Renderer to manage
	///
	////////////////////////////////////////////////////////////
	explicit Renderer(SDL_Renderer* renderer) : renderer_(renderer) {
        assert(renderer);
    }

	////////////////////////////////////////////////////////////
	/// \brief Create renderer
	///
	/// \param[in] window Window where rendering is displayed
	/// \param[in] index Index of the rendering driver to initialize,
	///                  or -1 to initialize the first one supporting
	///                  the requested flags
	/// \param[in] flags 0 or more SDL_RendererFlags OR'ed together
	///
	/// \throws SDL2pp::Exception
	///
	/// \see http://wiki.libsdl.org/SDL_CreateRenderer
	///
	////////////////////////////////////////////////////////////
	Renderer(Window& window, int index, Uint32 flags) {
        if ((renderer_ = SDL_CreateRenderer(window.Get(), index, flags)) == nullptr)
            throw Exception("SDL_CreateRenderer");
    }

	////////////////////////////////////////////////////////////
	/// \brief Destructor
	///
	/// \see http://wiki.libsdl.org/SDL_DestroyRenderer
	///
	////////////////////////////////////////////////////////////
	virtual ~Renderer() {
        if (renderer_ != nullptr)
            SDL_DestroyRenderer(renderer_);
    }

	////////////////////////////////////////////////////////////
	/// \brief Move constructor
	///
	/// \param[in] other SDL2pp::Renderer object to move data from
	///
	////////////////////////////////////////////////////////////
	Renderer(Renderer&& other) noexcept : renderer_(other.renderer_) {
        other.renderer_ = nullptr;
    }

	////////////////////////////////////////////////////////////
	/// \brief Move assignment operator
	///
	/// \param[in] other SDL2pp::Renderer object to move data from
	///
	/// \returns Reference to self
	///
	////////////////////////////////////////////////////////////
	Renderer& operator=(Renderer&& other) noexcept {
        if (&other == this)
            return *this;
        if (renderer_ != nullptr)
            SDL_DestroyRenderer(renderer_);
        renderer_ = other.renderer_;
        other.renderer_ = nullptr;
        return *this;
    }

	////////////////////////////////////////////////////////////
	/// \brief Deleted copy constructor
	///
	/// This class is not copyable
	///
	////////////////////////////////////////////////////////////
	Renderer(const Renderer& other) = delete;

	////////////////////////////////////////////////////////////
	/// \brief Deleted assignment operator
	///
	/// This class is not copyable
	///
	////////////////////////////////////////////////////////////
	Renderer& operator=(const Renderer& other) = delete;

	////////////////////////////////////////////////////////////
	/// \brief Get pointer to managed SDL_Renderer structure
	///
	/// \returns Pointer to managed SDL_Renderer structure
	///
	////////////////////////////////////////////////////////////
	SDL_Renderer* Get() const {
        return renderer_;
    }

	////////////////////////////////////////////////////////////
	/// \brief Update the screen with any rendering performed
	///        since the previous call
	///
	/// %SDL's rendering functions operate on a backbuffer; that
	/// is, calling a rendering function such as SDL_RenderDrawLine()
	/// does not directly put a line on the screen, but rather
	/// updates the backbuffer. As such, you compose your entire
	/// scene and present the composed backbuffer to the screen as
	/// a complete picture.
	///
	/// Therefore, when using %SDL's rendering API, one does all
	/// drawing intended for the frame, and then calls this function
	/// once per frame to present the final drawing to the user.
	///
	/// \returns Reference to self
	///
	/// \see http://wiki.libsdl.org/SDL_RenderPresent
	///
	////////////////////////////////////////////////////////////
	Renderer& Present() {
        SDL_RenderPresent(renderer_);
        return *this;
    }

	////////////////////////////////////////////////////////////
	/// \brief Clear the current rendering target with the drawing color
	///
	/// \returns Reference to self
	///
	/// \throws SDL2pp::Exception
	///
	/// \see http://wiki.libsdl.org/SDL_RenderClear
	///
	////////////////////////////////////////////////////////////
	Renderer& Clear() {
        if (SDL_RenderClear(renderer_) != 0)
            throw Exception("SDL_RenderClear");
        return *this;
    }

	////////////////////////////////////////////////////////////
	/// \brief Get information about a rendering context
	///
	/// \param[out] info SDL_RendererInfo structure to be filled with
	///                  information about the current renderer
	///
	/// \throws SDL2pp::Exception
	///
	/// \see http://wiki.libsdl.org/SDL_GetRendererInfo
	///
	////////////////////////////////////////////////////////////
	void GetInfo(SDL_RendererInfo& info) {
        if (SDL_GetRendererInfo(renderer_, &info) != 0)
            throw Exception("SDL_GetRendererInfo");
    }

    ////////////////////////////////////////////////////////////
	/// \brief Copy a portion of the texture to the current rendering
	///        target
	///
	/// \param[in] texture Source texture
	/// \param[in] srcrect Source rectangle, NullOpt for the entire texture
	/// \param[in] dstrect Destination rectangle, NullOpt for the entire
	///                    rendering target
	///
	/// \returns Reference to self
	///
	/// \throws SDL2pp::Exception
	///
	/// \see http://wiki.libsdl.org/SDL_RenderCopy
	///
	////////////////////////////////////////////////////////////
	Renderer& Copy(Texture& texture, const Optional<Rect>& srcrect = NullOpt, const Optional<Rect>& dstrect = NullOpt) {
        if (SDL_RenderCopy(renderer_, texture.Get(), srcrect ? &*srcrect : nullptr, dstrect ? &*dstrect : nullptr) != 0)
            throw Exception("SDL_RenderCopy");
        return *this;
    }

	////////////////////////////////////////////////////////////
	/// \brief Copy a portion of the texture to the current rendering
	///        target (preserve texture dimensions)
	///
	/// \param[in] texture Source texture
	/// \param[in] srcrect Source rectangle, NullOpt for the entire texture
	/// \param[in] dstpoint Target point for source top left corner
	///
	/// \returns Reference to self
	///
	/// \throws SDL2pp::Exception
	///
	/// \see http://wiki.libsdl.org/SDL_RenderCopy
	///
	////////////////////////////////////////////////////////////
	Renderer& Copy(Texture& texture, const Optional<Rect>& srcrect, const Point& dstpoint) {
        Rect dstrect(
                dstpoint.x,
                dstpoint.y,
                srcrect ? srcrect->w : texture.GetWidth(),
                srcrect ? srcrect->h : texture.GetHeight()
        );
        return Copy(texture, srcrect, dstrect);
    }

	////////////////////////////////////////////////////////////
	/// \brief Copy a portion of the texture to the current rendering
	///        target with optional rotating or flipping
	///
	/// \param[in] texture Source texture
	/// \param[in] srcrect Source rectangle, NullOpt for the entire texture
	/// \param[in] dstrect Destination rectangle, NullOpt for the entire
	///                    rendering target
	/// \param[in] angle Angle in degrees that indicates the rotation that
	///                  will be applied to dstrect
	/// \param[in] center Point indicating the point around which dstrect
	///                   will be rotated (NullOpt to rotate around dstrect
	///                   center)
	/// \param[in] flip SDL_RendererFlip value stating which flipping
	///                 actions should be performed on the texture
	///
	/// \returns Reference to self
	///
	/// \throws SDL2pp::Exception
	///
	/// \see http://wiki.libsdl.org/SDL_RendererFlip
	/// \see http://wiki.libsdl.org/SDL_RenderCopyEx
	///
	////////////////////////////////////////////////////////////
	Renderer& Copy(Texture& texture, const Optional<Rect>& srcrect, const Optional<Rect>& dstrect, double angle, const Optional<Point>& center = NullOpt, int flip = 0) {
        if (SDL_RenderCopyEx(renderer_, texture.Get(), srcrect ? &*srcrect : nullptr, dstrect ? &*dstrect : nullptr, angle, center ? &*center : nullptr, static_cast<SDL_RendererFlip>(flip)) != 0)
            throw Exception("SDL_RenderCopyEx");
        return *this;
    }

	////////////////////////////////////////////////////////////
	/// \brief Copy a portion of the texture to the current rendering
	///        target with optional rotating or flipping (preserve texture
	///        dimensions)
	///
	/// \param[in] texture Source texture
	/// \param[in] srcrect Source rectangle, NullOpt for the entire texture
	/// \param[in] dstpoint Target point for source top left corner
	/// \param[in] angle Angle in degrees that indicates the rotation that
	///                  will be applied to dstrect
	/// \param[in] center Point indicating the point around which dstrect
	///                   will be rotated (NullOpt to rotate around dstrect
	///                   center)
	/// \param[in] flip SDL_RendererFlip value stating which flipping
	///                 actions should be performed on the texture
	///
	/// \returns Reference to self
	///
	/// \throws SDL2pp::Exception
	///
	/// \see http://wiki.libsdl.org/SDL_RendererFlip
	/// \see http://wiki.libsdl.org/SDL_RenderCopyEx
	///
	////////////////////////////////////////////////////////////
	Renderer& Copy(Texture& texture, const Optional<Rect>& srcrect, const SDL2pp::Point& dstpoint, double angle, const Optional<Point>& center = NullOpt, int flip = 0) {
        Rect dstrect(
                dstpoint.x,
                dstpoint.y,
                srcrect ? srcrect->w : texture.GetWidth(),
                srcrect ? srcrect->h : texture.GetHeight()
        );
        return Copy(texture, srcrect, dstrect, angle, center, flip);
    }

	////////////////////////////////////////////////////////////
	/// \brief Fill the target with repeated source texture
	///
	/// \param[in] texture Source texture
	/// \param[in] srcrect Source rectangle, NullOpt for the entire texture
	/// \param[in] dstrect Destination rectangle, NullOpt for the entire
	///                    rendering target
	/// \param[in] offset Offset of tiled texture in pixels relative to
	///                   dstrect
	/// \param[in] flip SDL_RendererFlip value stating which flipping
	///                 actions should be performed on the texture
	///
	/// \returns Reference to self
	///
	/// \throws SDL2pp::Exception
	///
	/// \see http://wiki.libsdl.org/SDL_RendererFlip
	/// \see http://wiki.libsdl.org/SDL_RenderCopy
	/// \see http://wiki.libsdl.org/SDL_RenderCopyEx
	///
	////////////////////////////////////////////////////////////
	Renderer& FillCopy(Texture& texture, const Optional<Rect>& srcrect = NullOpt, const Optional<Rect>& dstrect = NullOpt, const Point& offset = Point(0, 0), int flip = 0) {
        // resolve rectangles
        Rect src = srcrect ? *srcrect : Rect(0, 0, texture.GetWidth(), texture.GetHeight());
        Rect dst = dstrect ? *dstrect : Rect(0, 0, GetOutputWidth(), GetOutputHeight());

        // rectangle for single tile
        Rect start_tile(
                offset.x,
                offset.y,
                src.w,
                src.h
        );

        // ensure tile is leftmost and topmost
        if (start_tile.x + start_tile.w <= 0)
            start_tile.x += (-start_tile.x) / start_tile.w * start_tile.w;
        if (start_tile.x > 0)
            start_tile.x -= (start_tile.x + start_tile.w - 1) / start_tile.w * start_tile.w;

        if (start_tile.y + start_tile.h <= 0)
            start_tile.y += (-start_tile.y) / start_tile.h * start_tile.h;
        if (start_tile.y > 0)
            start_tile.y -= (start_tile.y + start_tile.h - 1) / start_tile.h * start_tile.h;

        // paint tile array
        for (int y = start_tile.y; y < dst.h; y += start_tile.h) {
            for (int x = start_tile.x; x < dst.w; x += start_tile.w) {
                Rect tile_src = src;
                Rect tile_dst(x, y, start_tile.w, start_tile.h);

                // clamp with dstrect
                int xunderflow = -x;
                if (xunderflow > 0) {
                    tile_src.w -= xunderflow;
                    tile_src.x += xunderflow;
                    tile_dst.w -= xunderflow;
                    tile_dst.x += xunderflow;
                }

                int yunderflow = -y;
                if (yunderflow > 0) {
                    tile_src.h -= yunderflow;
                    tile_src.y += yunderflow;
                    tile_dst.h -= yunderflow;
                    tile_dst.y += yunderflow;
                }

                int xoverflow = tile_dst.x + tile_dst.w - dst.w;
                if (xoverflow > 0) {
                    tile_src.w -= xoverflow;
                    tile_dst.w -= xoverflow;
                }

                int yoverflow = tile_dst.y + tile_dst.h - dst.h;
                if (yoverflow > 0) {
                    tile_src.h -= yoverflow;
                    tile_dst.h -= yoverflow;
                }

                // make tile_dst absolute
                tile_dst.x += dst.x;
                tile_dst.y += dst.y;

                if (flip != 0) {
                    // mirror tile_src inside src to take flipping into account
                    if (flip & SDL_FLIP_HORIZONTAL)
                        tile_src.x = src.w - tile_src.x - tile_src.w;

                    if (flip & SDL_FLIP_VERTICAL)
                        tile_src.y = src.h - tile_src.y - tile_src.h;

                    Copy(texture, tile_src, tile_dst, 0.0, NullOpt, flip);
                } else {
                    Copy(texture, tile_src, tile_dst);
                }
            }
        }
        return *this;
    }

	////////////////////////////////////////////////////////////
	/// \brief Set color user for drawing operations
	///
	/// \param[in] r Red value used to draw on the rendering target
	/// \param[in] g Green value used to draw on the rendering target
	/// \param[in] b Blue value used to draw on the rendering target
	/// \param[in] a Alpha value used to draw on the rendering target
	///
	/// \returns Reference to self
	///
	/// \throws SDL2pp::Exception
	///
	/// \see http://wiki.libsdl.org/SDL_SetRenderDrawColor
	///
	////////////////////////////////////////////////////////////
	Renderer& SetDrawColor(Uint8 r = 0, Uint8 g = 0, Uint8 b = 0, Uint8 a = 255) {
        if (SDL_SetRenderDrawColor(renderer_, r, g, b, a) != 0)
            throw Exception("SDL_SetRenderDrawColor");
        return *this;
    }

	////////////////////////////////////////////////////////////
	/// \brief Set color user for drawing operations
	///
	/// \param[in] color Color to draw on the rendering target
	///
	/// \returns Reference to self
	///
	/// \throws SDL2pp::Exception
	///
	/// \see http://wiki.libsdl.org/SDL_SetRenderDrawColor
	///
	////////////////////////////////////////////////////////////
	Renderer& SetDrawColor(const Color& color) {
        return SetDrawColor(color.r, color.g, color.b, color.a);
    }

	////////////////////////////////////////////////////////////
	/// \brief Set current render target to default
	///
	/// \returns Reference to self
	///
	/// \throws SDL2pp::Exception
	///
	/// \see http://wiki.libsdl.org/SDL_SetRenderTarget
	///
	////////////////////////////////////////////////////////////
	Renderer& SetTarget() {
        if (SDL_SetRenderTarget(renderer_, nullptr) != 0)
            throw Exception("SDL_SetRenderTarget");
        return *this;
    }

	////////////////////////////////////////////////////////////
	/// \brief Set current render target to specified texture
	///
	/// \param[in] texture Target texture, SDL2pp::Texture created with
	///                    SDL_TEXTUREACCESS_TARGET
	///
	/// \returns Reference to self
	///
	/// \throws SDL2pp::Exception
	///
	/// \see http://wiki.libsdl.org/SDL_SetRenderTarget
	///
	////////////////////////////////////////////////////////////
	Renderer& SetTarget(Texture& texture) {
        if (SDL_SetRenderTarget(renderer_, texture.Get()) != 0)
            throw Exception("SDL_SetRenderTarget");
        return *this;
    }

	////////////////////////////////////////////////////////////
	/// \brief Set the blend mode used for drawing operations
	///
	/// \param[in] blendMode SDL_BlendMode to use for blending
	///
	/// \returns Reference to self
	///
	/// \throws SDL2pp::Exception
	///
	/// \see http://wiki.libsdl.org/SDL_BlendMode
	/// \see http://wiki.libsdl.org/SDL_SetRenderDrawBlendMode
	///
	////////////////////////////////////////////////////////////
	Renderer& SetDrawBlendMode(SDL_BlendMode blendMode = SDL_BLENDMODE_NONE) {
        if (SDL_SetRenderDrawBlendMode(renderer_, blendMode) != 0)
            throw Exception("SDL_SetRenderDrawBlendMode");
        return *this;
    }

	////////////////////////////////////////////////////////////
	/// \brief Draw a point on the current rendering target
	///
	/// \param[in] x X coordinate of the point
	/// \param[in] y Y coordinate of the point
	///
	/// \returns Reference to self
	///
	/// \throws SDL2pp::Exception
	///
	/// \see http://wiki.libsdl.org/SDL_RenderDrawPoint
	///
	////////////////////////////////////////////////////////////
	Renderer& DrawPoint(int x, int y) {
        if (SDL_RenderDrawPoint(renderer_, x, y) != 0)
            throw Exception("SDL_RenderDrawPoint");
        return *this;
    }

	////////////////////////////////////////////////////////////
	/// \brief Draw a point on the current rendering target
	///
	/// \param[in] p Coordinates of the point
	///
	/// \returns Reference to self
	///
	/// \throws SDL2pp::Exception
	///
	/// \see http://wiki.libsdl.org/SDL_RenderDrawPoint
	///
	////////////////////////////////////////////////////////////
	Renderer& DrawPoint(const Point& p) {
        DrawPoint(p.x, p.y);
        return *this;
    }

	////////////////////////////////////////////////////////////
	/// \brief Draw multiple points on the current rendering target
	///
	/// \param[in] points Array of coordinates of points to draw
	/// \param[in] count Number of points to draw
	///
	/// \returns Reference to self
	///
	/// \throws SDL2pp::Exception
	///
	/// \see http://wiki.libsdl.org/SDL_RenderDrawPoints
	///
	////////////////////////////////////////////////////////////
	Renderer& DrawPoints(const Point* points, int count) {
        std::vector<SDL_Point> sdl_points;
        sdl_points.reserve(static_cast<size_t>(count));
        for (const Point* p = points; p != points + count; ++p)
            sdl_points.emplace_back(*p);

        if (SDL_RenderDrawPoints(renderer_, sdl_points.data(), count) != 0)
            throw Exception("SDL_RenderDrawPoints");

        return *this;
    }

	////////////////////////////////////////////////////////////
	/// \brief Draw a line on the current rendering target
	///
	/// \param[in] x1 X coordinate of the start point
	/// \param[in] y1 Y coordinate of the start point
	/// \param[in] x2 X coordinate of the end point
	/// \param[in] y2 Y coordinate of the end point
	///
	/// \returns Reference to self
	///
	/// \throws SDL2pp::Exception
	///
	/// \see http://wiki.libsdl.org/SDL_RenderDrawLine
	///
	////////////////////////////////////////////////////////////
	Renderer& DrawLine(int x1, int y1, int x2, int y2) {
        if (SDL_RenderDrawLine(renderer_, x1, y1, x2, y2) != 0)
            throw Exception("SDL_RenderDrawLine");
        return *this;
    }

	////////////////////////////////////////////////////////////
	/// \brief Draw a line on the current rendering target
	///
	/// \param[in] p1 Coordinates of the start point
	/// \param[in] p2 Coordinates of the end point
	///
	/// \returns Reference to self
	///
	/// \throws SDL2pp::Exception
	///
	/// \see http://wiki.libsdl.org/SDL_RenderDrawLine
	///
	////////////////////////////////////////////////////////////
	Renderer& DrawLine(const Point& p1, const Point& p2) {
        DrawLine(p1.x, p1.y, p2.x, p2.y);
        return *this;
    }

	////////////////////////////////////////////////////////////
	/// \brief Draw a polyline on the current rendering target
	///
	/// \param[in] points Array of coordinates of points along the polyline
	/// \param[in] count Number of points to draw count-1 polyline segments
	///
	/// \returns Reference to self
	///
	/// \throws SDL2pp::Exception
	///
	/// \see http://wiki.libsdl.org/SDL_RenderDrawLines
	///
	////////////////////////////////////////////////////////////
	Renderer& DrawLines(const Point* points, int count) {
        std::vector<SDL_Point> sdl_points;
        sdl_points.reserve(static_cast<size_t>(count));
        for (const Point* p = points; p != points + count; ++p)
            sdl_points.emplace_back(*p);

        if (SDL_RenderDrawLines(renderer_, sdl_points.data(), count) != 0)
            throw Exception("SDL_RenderDrawLines");

        return *this;
    }

	////////////////////////////////////////////////////////////
	/// \brief Draw a rectangle on the current rendering target
	///
	/// \param[in] x1 X coordinate of the start corner
	/// \param[in] y1 Y coordinate of the start corner
	/// \param[in] x2 X coordinate of the end corner
	/// \param[in] y2 Y coordinate of the end corner
	///
	/// \returns Reference to self
	///
	/// \throws SDL2pp::Exception
	///
	/// \see http://wiki.libsdl.org/SDL_RenderDrawRect
	///
	////////////////////////////////////////////////////////////
	Renderer& DrawRect(int x1, int y1, int x2, int y2) {
        SDL_Rect rect = {x1, y1, x2 - x1 + 1, y2 - y1 + 1};
        if (SDL_RenderDrawRect(renderer_, &rect) != 0)
            throw Exception("SDL_RenderDrawRect");
        return *this;
    }

	////////////////////////////////////////////////////////////
	/// \brief Draw a rectangle on the current rendering target
	///
	/// \param[in] p1 Coordinates of the start corner
	/// \param[in] p2 Coordinates of the end corner
	///
	/// \returns Reference to self
	///
	/// \throws SDL2pp::Exception
	///
	/// \see http://wiki.libsdl.org/SDL_RenderDrawRect
	///
	////////////////////////////////////////////////////////////
	Renderer& DrawRect(const Point& p1, const Point& p2) {
        DrawRect(p1.x, p1.y, p2.x, p2.y);
        return *this;
    }

	////////////////////////////////////////////////////////////
	/// \brief Draw a rectangle on the current rendering target
	///
	/// \param[in] r Rectangle to draw
	///
	/// \returns Reference to self
	///
	/// \throws SDL2pp::Exception
	///
	/// \see http://wiki.libsdl.org/SDL_RenderDrawRect
	///
	////////////////////////////////////////////////////////////
	Renderer& DrawRect(const Rect& r) {
        if (SDL_RenderDrawRect(renderer_, &r) != 0)
            throw Exception("SDL_RenderDrawRect");
        return *this;
    }

	////////////////////////////////////////////////////////////
	/// \brief Draw multiple rectangles on the current rendering target
	///
	/// \param[in] rects Array of rectangles to draw
	/// \param[in] count Number of rectangles
	///
	/// \returns Reference to self
	///
	/// \throws SDL2pp::Exception
	///
	/// \see http://wiki.libsdl.org/SDL_RenderDrawRects
	///
	////////////////////////////////////////////////////////////
	Renderer& DrawRects(const Rect* rects, int count) {
        std::vector<SDL_Rect> sdl_rects;
        sdl_rects.reserve(static_cast<size_t>(count));
        for (const Rect* r = rects; r != rects + count; ++r)
            sdl_rects.emplace_back(*r);

        if (SDL_RenderDrawRects(renderer_, sdl_rects.data(), count) != 0)
            throw Exception("SDL_RenderDrawRects");

        return *this;
    }

	////////////////////////////////////////////////////////////
	/// \brief Fill a rectangle on the current rendering target
	///
	/// \param[in] x1 X coordinate of the start corner
	/// \param[in] y1 Y coordinate of the start corner
	/// \param[in] x2 X coordinate of the end corner
	/// \param[in] y2 Y coordinate of the end corner
	///
	/// \returns Reference to self
	///
	/// \throws SDL2pp::Exception
	///
	/// \see http://wiki.libsdl.org/SDL_RenderFillRect
	///
	////////////////////////////////////////////////////////////
	Renderer& FillRect(int x1, int y1, int x2, int y2) {
        SDL_Rect rect = {x1, y1, x2 - x1 + 1, y2 - y1 + 1};
        if (SDL_RenderFillRect(renderer_, &rect) != 0)
            throw Exception("SDL_RenderFillRect");
        return *this;
    }

	////////////////////////////////////////////////////////////
	/// \brief Fill a rectangle on the current rendering target
	///
	/// \param[in] p1 Coordinates of the start corner
	/// \param[in] p2 Coordinates of the end corner
	///
	/// \returns Reference to self
	///
	/// \throws SDL2pp::Exception
	///
	/// \see http://wiki.libsdl.org/SDL_RenderFillRect
	///
	////////////////////////////////////////////////////////////
	Renderer& FillRect(const Point& p1, const Point& p2) {
        FillRect(p1.x, p1.y, p2.x, p2.y);
        return *this;
    }

	////////////////////////////////////////////////////////////
	/// \brief Fill a rectangle on the current rendering target
	///
	/// \param[in] r Rectangle to draw
	///
	/// \returns Reference to self
	///
	/// \throws SDL2pp::Exception
	///
	/// \see http://wiki.libsdl.org/SDL_RenderFillRect
	///
	////////////////////////////////////////////////////////////
	Renderer& FillRect(const Rect& r) {
        if (SDL_RenderFillRect(renderer_, &r) != 0)
            throw Exception("SDL_RenderFillRect");
        return *this;
    }

	////////////////////////////////////////////////////////////
	/// \brief Fill multiple rectangles on the current rendering target
	///
	/// \param[in] rects Array of rectangles to draw
	/// \param[in] count Number of rectangles
	///
	/// \returns Reference to self
	///
	/// \throws SDL2pp::Exception
	///
	/// \see http://wiki.libsdl.org/SDL_RenderFillRects
	///
	////////////////////////////////////////////////////////////
	Renderer& FillRects(const Rect* rects, int count) {
        std::vector<SDL_Rect> sdl_rects;
        sdl_rects.reserve(static_cast<size_t>(count));
        for (const Rect* r = rects; r != rects + count; ++r)
            sdl_rects.emplace_back(*r);

        if (SDL_RenderFillRects(renderer_, sdl_rects.data(), count) != 0)
            throw Exception("SDL_RenderFillRects");

        return *this;
    }

	////////////////////////////////////////////////////////////
	/// \brief Read pixels from the current rendering target
	///
	/// \param[in] rect Area to read or NullOpt for the entire render
	///                 target
	/// \param[in] format Desired format of the pixel data, or 0 to
	///                   use the format of the rendering target
	/// \param[in] pixels Pointer to memory to be filled with pixel
	///                   data
	/// \param[in] pitch Number of bytes in a row of pixel data, including
	///                  padding between lines
	///
	/// \throws SDL2pp::Exception
	///
	/// \see http://wiki.libsdl.org/SDL_RenderReadPixels
	///
	////////////////////////////////////////////////////////////
	void ReadPixels(const Optional<Rect>& rect, Uint32 format, void* pixels, int pitch) {
        if (SDL_RenderReadPixels(renderer_, rect ? &*rect : nullptr, format, pixels, pitch) != 0)
            throw Exception("SDL_RenderReadPixels");
    }

	////////////////////////////////////////////////////////////
	/// \brief Set the clipping rectangle for rendering
	///
	/// \param[in] rect New clipping rectangle or NullOpt to disable
	///                 clipping
	///
	/// \returns Reference to self
	///
	/// \throws SDL2pp::Exception
	///
	/// \see http://wiki.libsdl.org/SDL_RenderSetClipRect
	///
	////////////////////////////////////////////////////////////
	Renderer& SetClipRect(const Optional<Rect>& rect = NullOpt) {
        if (SDL_RenderSetClipRect(renderer_, rect ? &*rect : nullptr) != 0)
            throw Exception("SDL_RenderSetClipRect");
        return *this;
    }

	////////////////////////////////////////////////////////////
	/// \brief Set a device independent resolution for rendering
	///
	/// \param[in] w Width of the logical resolution
	/// \param[in] h Height of the logical resolution
	///
	/// \returns Reference to self
	///
	/// \throws SDL2pp::Exception
	///
	/// \see http://wiki.libsdl.org/SDL_RenderSetLogicalSize
	///
	////////////////////////////////////////////////////////////
	Renderer& SetLogicalSize(int w, int h) {
        if (SDL_RenderSetLogicalSize(renderer_, w, h) != 0)
            throw Exception("SDL_RenderSetLogicalSize");
        return *this;
    }

	////////////////////////////////////////////////////////////
	/// \brief Set the drawing scale for rendering on the current target
	///
	/// \param[in] scaleX Horizontal scaling factor
	/// \param[in] scaleY Vertical scaling factor
	///
	/// \returns Reference to self
	///
	/// \throws SDL2pp::Exception
	///
	/// \see http://wiki.libsdl.org/SDL_RenderSetScale
	///
	////////////////////////////////////////////////////////////
	Renderer& SetScale(float scaleX, float scaleY) {
        if (SDL_RenderSetScale(renderer_, scaleX, scaleY) != 0)
            throw Exception("SDL_RenderSetScale");
        return *this;
    }

	////////////////////////////////////////////////////////////
	/// \brief Set the drawing area for rendering on the current target
	///
	/// \param[in] rect Rectangle representing the drawing area or
	///                 NullOpt to set the viewport to the entire target
	///
	/// \returns Reference to self
	///
	/// \throws SDL2pp::Exception
	///
	/// \see http://wiki.libsdl.org/SDL_RenderSetViewport
	///
	////////////////////////////////////////////////////////////
	Renderer& SetViewport(const Optional<Rect>& rect = NullOpt) {
        if (SDL_RenderSetViewport(renderer_, rect ? &*rect : nullptr) != 0)
            throw Exception("SDL_RenderSetViewport");
        return *this;
    }

	////////////////////////////////////////////////////////////
	/// \brief Determine whether a window supports the use of
	///        render targets
	///
	/// \returns True if render targets are supported
	///
	/// \see http://wiki.libsdl.org/SDL_RenderTargetSupported
	///
	////////////////////////////////////////////////////////////
	bool TargetSupported() const {
        return SDL_RenderTargetSupported(renderer_) == SDL_TRUE;
    }

	////////////////////////////////////////////////////////////
	/// \brief Get the clip rectangle for the current target
	///
	/// \returns Rect representing current clipping area or
	///          NullOpt if clipping is disabled
	///
	/// \see http://wiki.libsdl.org/SDL_RenderGetClipRect
	///
	////////////////////////////////////////////////////////////
	Optional<Rect> GetClipRect() const {
        SDL_Rect rect;
        SDL_RenderGetClipRect(renderer_, &rect);

        if (SDL_RectEmpty(&rect))
            return NullOpt;
        else
            return Rect(rect);
    }

	////////////////////////////////////////////////////////////
	/// \brief Get device independent resolution for rendering
	///
	/// \returns Point representing logical resolution
	///
	/// \see http://wiki.libsdl.org/SDL_RenderGetLogicalSize
	///
	/// \note If this function is called on an Renderer who never had
	///       its logical size set by SetLogicalSize(), this function
	///       returns {0, 0}
	///
	////////////////////////////////////////////////////////////
	Point GetLogicalSize() const {
        int w, h;
        SDL_RenderGetLogicalSize(renderer_, &w, &h);
        return Point(w, h);
    }

	////////////////////////////////////////////////////////////
	/// \brief Get device independent width resolution for rendering
	///
	/// \returns Logical resolution width
	///
	/// \see http://wiki.libsdl.org/SDL_RenderGetLogicalSize
	///
	/// \note If this function is called on an Renderer who never had
	///       its logical size set by SetLogicalSize(), this function
	///       returns 0
	///
	////////////////////////////////////////////////////////////
	int GetLogicalWidth() const {
        int w;
        SDL_RenderGetLogicalSize(renderer_, &w, nullptr);
        return w;
    }

	////////////////////////////////////////////////////////////
	/// \brief Get device independent width resolution for rendering
	///
	/// \returns Logical resolution height
	///
	/// \see http://wiki.libsdl.org/SDL_RenderGetLogicalSize
	///
	/// \note If this function is called on an Renderer who never had
	///       its logical size set by SetLogicalSize(), this function
	///       returns 0
	///
	////////////////////////////////////////////////////////////
	int GetLogicalHeight() const {
        int h;
        SDL_RenderGetLogicalSize(renderer_, nullptr, &h);
        return h;
    }

	////////////////////////////////////////////////////////////
	/// \brief Get the drawing scale for the current target
	///
	/// \param[out] scalex Variable to be filled with the horizontal scaling factor
	/// \param[out] scaley Variable to be filled with the vertical scaling factor
	///
	/// \see http://wiki.libsdl.org/SDL_RenderGetScale
	///
	////////////////////////////////////////////////////////////
	void GetScale(float& scalex, float& scaley) const {
        SDL_RenderGetScale(renderer_, &scalex, &scaley);
    }

	////////////////////////////////////////////////////////////
	/// \brief Get the drawing scale for the current target
	///
	/// \returns Horizontal scaling factor
	///
	/// \see http://wiki.libsdl.org/SDL_RenderGetScale
	///
	////////////////////////////////////////////////////////////
	float GetXScale() const {
        float scalex;
        SDL_RenderGetScale(renderer_, &scalex, nullptr);
        return scalex;
    }

	////////////////////////////////////////////////////////////
	/// \brief Get the drawing scale for the current target
	///
	/// \returns Vertical scaling factor
	///
	/// \see http://wiki.libsdl.org/SDL_RenderGetScale
	///
	////////////////////////////////////////////////////////////
	float GetYScale() const {
        float scaley;
        SDL_RenderGetScale(renderer_, nullptr, &scaley);
        return scaley;
    }

	////////////////////////////////////////////////////////////
	/// \brief Get the drawing area for the current target
	///
	/// \returns Rect representing current drawing area
	///
	/// \see http://wiki.libsdl.org/SDL_RenderGetViewport
	///
	////////////////////////////////////////////////////////////
	Rect GetViewport() const {
        SDL_Rect rect;
        SDL_RenderGetViewport(renderer_, &rect);
        return rect;
    }

	////////////////////////////////////////////////////////////
	/// \brief Get the blend mode used for drawing operations
	///
	/// \return Current SDL_BlendMode
	///
	/// \throws SDL2pp::Exception
	///
	/// \see http://wiki.libsdl.org/SDL_GetRenderDrawBlendMode
	///
	////////////////////////////////////////////////////////////
	SDL_BlendMode GetDrawBlendMode() const {
        SDL_BlendMode mode;
        if (SDL_GetRenderDrawBlendMode(renderer_, &mode) != 0)
            throw Exception("SDL_GetRenderDrawBlendMode");
        return mode;
    }

	////////////////////////////////////////////////////////////
	/// \brief Get the additional color value multiplied into render copy operations
	///
	/// \return Color object with the value used to do render copy operations
	///
	/// \throws SDL2pp::Exception
	///
	/// \see http://wiki.libsdl.org/SDL_GetRenderDrawColor
	///
	////////////////////////////////////////////////////////////
	Color GetDrawColor() const {
        Color color;
        GetDrawColor(color.r, color.g, color.b, color.a);
        return color;
    }

	////////////////////////////////////////////////////////////
	/// \brief Get the additional color value multiplied into render copy operations
	///
	/// \param[out] r Variable to be filled in with red value used to draw on the rendering target
	/// \param[out] g Variable to be filled in with green value used to draw on the rendering target
	/// \param[out] b Variable to be filled in with blue value used to draw on the rendering target
	/// \param[out] a Variable to be filled in with alpha value used to draw on the rendering target
	///
	/// \throws SDL2pp::Exception
	///
	/// \see http://wiki.libsdl.org/SDL_GetRenderDrawColor
	///
	////////////////////////////////////////////////////////////
	void GetDrawColor(Uint8& r, Uint8& g, Uint8& b, Uint8& a) const {
        if (SDL_GetRenderDrawColor(renderer_, &r, &g, &b, &a) != 0)
            throw Exception("SDL_GetRenderDrawColor");
    }

	////////////////////////////////////////////////////////////
	/// \brief Get the output size of a rendering context
	///
	/// \returns Point representing output size
	///
	/// \throws SDL2pp::Exception
	///
	/// \see http://wiki.libsdl.org/SDL_GetRendererOutputSize
	///
	////////////////////////////////////////////////////////////
	Point GetOutputSize() const {
        int w, h;
        if (SDL_GetRendererOutputSize(renderer_, &w, &h) != 0)
            throw Exception("SDL_GetRendererOutputSize");
        return Point(w, h);
    }

	////////////////////////////////////////////////////////////
	/// \brief Get the output width of a rendering context
	///
	/// \returns Output width
	///
	/// \throws SDL2pp::Exception
	///
	/// \see http://wiki.libsdl.org/SDL_GetRendererOutputSize
	///
	////////////////////////////////////////////////////////////
	int GetOutputWidth() const {
        int w;
        if (SDL_GetRendererOutputSize(renderer_, &w, nullptr) != 0)
            throw Exception("SDL_GetRendererOutputSize");
        return w;
    }

	////////////////////////////////////////////////////////////
	/// \brief Get the output height of a rendering context
	///
	/// \returns Output height
	///
	/// \throws SDL2pp::Exception
	///
	/// \see http://wiki.libsdl.org/SDL_GetRendererOutputSize
	///
	////////////////////////////////////////////////////////////
	int GetOutputHeight() const {
        int h;
        if (SDL_GetRendererOutputSize(renderer_, nullptr, &h) != 0)
            throw Exception("SDL_GetRendererOutputSize");
        return h;
    }

	////////////////////////////////////////////////////////////
	/// \brief Create empty texture
	///
	/// \param[in] renderer Rendering context to create texture for
	/// \param[in] format One of the enumerated values in SDL_PixelFormatEnum
	/// \param[in] access One of the enumerated values in SDL_TextureAccess
	/// \param[in] w Width of the texture in pixels
	/// \param[in] h Height of the texture in pixels
	///
	/// \throws SDL2pp::Exception
	///
	/// \see http://wiki.libsdl.org/SDL_CreateTexture
	///
	////////////////////////////////////////////////////////////
	Texture createTexture(Uint32 format, int access, int w, int h) {
        SDL_Texture* texture_ = SDL_CreateTexture(Get(), format, access, w, h);
        if (texture_ == nullptr) throw Exception("SDL_CreateTexture");
        return Texture(texture_);
    }

#ifdef SDL2PP_WITH_IMAGE
    ////////////////////////////////////////////////////////////
    /// \brief Create texture loading it via RWops
    ///
    /// \param[in] renderer Rendering context to create texture for
    /// \param[in] rwops RWops used to access an image file
    ///
    /// \throws SDL2pp::Exception
    ///
    ////////////////////////////////////////////////////////////
    Texture createTexture(RWops& rwops) const {
        SDL_Texture* texture_ = IMG_LoadTexture_RW(Get(), rwops.Get(), 0);
        if (texture_ == nullptr)
            throw Exception("IMG_LoadTexture_RW");
        return Texture(texture_);
    }

    ////////////////////////////////////////////////////////////
    /// \brief Create texture loading it from file
    ///
    /// \param[in] renderer Rendering context to create texture for
    /// \param[in] filename Path to an image file
    ///
    /// \throws SDL2pp::Exception
    ///
    ////////////////////////////////////////////////////////////
    Texture createTexture(const std::string& filename) const {
        SDL_Texture* texture_ = IMG_LoadTexture(Get(), filename.c_str());
        if (texture_ == nullptr)
            throw Exception("IMG_LoadTexture");
        return Texture(texture_);
    }
#endif

    ////////////////////////////////////////////////////////////
    /// \brief Create texture from surface
    ///
    /// \param[in] renderer Rendering context to create texture for
    /// \param[in] surface Surface containing pixel data used to fill the texture
    ///
    /// \throws SDL2pp::Exception
    ///
    /// \see http://wiki.libsdl.org/SDL_CreateTextureFromSurface
    ///
    ////////////////////////////////////////////////////////////
    Texture createTexture(const Surface& surface) const {
        SDL_Texture* texture_ = SDL_CreateTextureFromSurface(Get(), surface.Get());
        if (texture_ == nullptr)
            throw Exception("SDL_CreateTextureFromSurface");
        return Texture(texture_);
    }

};

}

#endif

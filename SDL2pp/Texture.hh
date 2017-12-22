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

#ifndef SDL2PP_TEXTURE_HH
#define SDL2PP_TEXTURE_HH

#include <string>

#include <SDL_stdinc.h>
#include <SDL_blendmode.h>

#include <SDL2pp/Optional.hh>
#include <SDL2pp/Export.hh>
#include <SDL2pp/Color.hh>

#include <utility>
#include <algorithm>
#include <cassert>

#include <SDL2pp/Config.hh>

#include <SDL_render.h>
#include <SDL2pp/Config.hh>
#include <SDL2pp/Exception.hh>
#include <SDL2pp/Rect.hh>
#include <SDL2pp/Surface.hh>

#ifdef SDL2PP_WITH_IMAGE
#	include <SDL_image.h>
#endif

#ifdef SDL2PP_WITH_IMAGE
#	include <SDL2pp/RWops.hh>
#endif

struct SDL_Texture;

namespace SDL2pp {

class Renderer;
class RWops;
class Surface;

////////////////////////////////////////////////////////////
/// \brief Image stored in the graphics card memory that
///        can be used for fast drawing
///
/// \ingroup rendering
///
/// \headerfile SDL2pp/Texture.hh
///
////////////////////////////////////////////////////////////
class SDL2PP_EXPORT Texture {
private:
	SDL_Texture* texture_; ///< Managed SDL_Texture object

public:
	////////////////////////////////////////////////////////////
	/// \brief SDL2pp::Texture lock
	///
	/// \ingroup rendering
	///
	/// \headerfile SDL2pp/Texture.hh
	///
	/// Textures with SDL_TEXTUREACCESS_STREAMING access mode may
	/// be locked, which provides (writeonly) access to their raw
	/// pixel data. This may be used to update texture contents.
	///
	/// This class represents the lock and controls its lifetime
	/// as the lock is released as soon as LockHandle is destroyed.
	///
	/// Usage example:
	/// \code
	/// {
	///     SDL2pp::Texture tex(SDL_PIXELFORMAT_RGB24,
	///                         SDL_TEXTUREACCESS_STREAMING,
	///                         256, 256);
	///     {
	///         // Lock the whole texture
	///         SDL2pp::Texture::LockHandle lock = tex.Lock();
	///
	///         unsigned char* start = static_cast<unsigned char*>(lock.GetPixels());
	///
	///         // note that we use lock.GetPitch(), not tex.GetWidth() here
	///         // as texture may have different dimensions in memory
	///         unsigned char* end = start + tex.GetHeight() * lock.GetPitch();
	///
	///         // fill the texture white
	///         std::fill(start, end, 255);
	///     }
	///     // At this point lock is released
	/// }
	/// \endcode
	///
	////////////////////////////////////////////////////////////
	class LockHandle {
		friend class Texture;
	private:
		Texture* texture_; ///< SDL2pp::Texture this lock belongs to
		void* pixels_;     ///< Pointer to raw pixel data of locked region
		int pitch_;        ///< Number of bytes in a row of pixel data, including padding between lines

	private:
		////////////////////////////////////////////////////////////
		/// \brief Create lock for specific SDL2pp::Texture
		///
		/// \param[in] rect Specifies region to lock
		///
		/// \throws STL2pp::Exception
		///
		/// \see http://wiki.libsdl.org/SDL_LockTexture
		///
		////////////////////////////////////////////////////////////
		LockHandle(Texture* texture, const Optional<Rect>& rect) : texture_(texture) {
            if (SDL_LockTexture(texture_->Get(), rect ? &*rect : nullptr, &pixels_, &pitch_) != 0)
                throw Exception("SDL_LockTexture");
        }

	public:
		////////////////////////////////////////////////////////////
		/// \brief Create no-op lock
		///
		/// This may be initialized with real lock later via move
		/// assignment
		///
		////////////////////////////////////////////////////////////
		LockHandle() : texture_(nullptr), pixels_(nullptr), pitch_(0) {
        }

		////////////////////////////////////////////////////////////
		/// \brief Destructor
		///
		/// Releases the lock
		///
		/// \see http://wiki.libsdl.org/SDL_UnlockTexture
		///
		////////////////////////////////////////////////////////////
		~LockHandle() {
            if (texture_ != nullptr)
                SDL_UnlockTexture(texture_->Get());
        }

		////////////////////////////////////////////////////////////
		/// \brief Move constructor
		///
		/// \param[in] other SDL2pp::Texture::LockHandle to move data from
		///
		////////////////////////////////////////////////////////////
		LockHandle(LockHandle&& other) noexcept: texture_(other.texture_), pixels_(other.pixels_), pitch_(other.pitch_) {
            other.texture_ = nullptr;
            other.pixels_ = nullptr;
            other.pitch_ = 0;
        }

		////////////////////////////////////////////////////////////
		/// \brief Move assignment operator
		///
		/// \param[in] other SDL2pp::Texture::LockHandle to move data from
		///
		/// \returns Reference to self
		///
		////////////////////////////////////////////////////////////
		LockHandle& operator=(LockHandle&& other) noexcept {
            if (&other == this)
                return *this;

            if (texture_ != nullptr)
                SDL_UnlockTexture(texture_->Get());

            texture_ = other.texture_;
            pixels_ = other.pixels_;
            pitch_ = other.pitch_;

            other.texture_ = nullptr;
            other.pixels_ = nullptr;
            other.pitch_ = 0;

            return *this;
        }

		////////////////////////////////////////////////////////////
		/// \brief Deleted copy constructor
		///
		/// This class is not copyable
		///
		////////////////////////////////////////////////////////////
		LockHandle(const LockHandle& other) = delete;

		////////////////////////////////////////////////////////////
		/// \brief Deleted assignment operator
		///
		/// This class is not copyable
		///
		////////////////////////////////////////////////////////////
		LockHandle& operator=(const LockHandle& other) = delete;

		////////////////////////////////////////////////////////////
		/// \brief Get pointer to raw pixel data of locked region
		///
		/// \returns Pointer to raw pixel data of locked region
		///
		////////////////////////////////////////////////////////////
		void* GetPixels() const {
            return pixels_;
        }

		////////////////////////////////////////////////////////////
		/// \brief Get pitch of locked pixel data
		///
		/// \returns Number of bytes in a row of pixel data, including
		///          padding between lines
		///
		////////////////////////////////////////////////////////////
		int GetPitch() const {
            return pitch_;
        }
	};

public:
	////////////////////////////////////////////////////////////
	/// \brief Construct from existing SDL_Texture structure
	///
	/// \param[in] texture Existing SDL_Texture to manage
	///
	////////////////////////////////////////////////////////////
	explicit Texture(SDL_Texture* texture) : texture_(texture) {
		assert(texture);
	}

	////////////////////////////////////////////////////////////
	/// \brief Destructor
	///
	/// \see http://wiki.libsdl.org/SDL_DestroyTexture
	///
	////////////////////////////////////////////////////////////
	virtual ~Texture() {
		if (texture_ != nullptr)
			SDL_DestroyTexture(texture_);
	}

	////////////////////////////////////////////////////////////
	/// \brief Move constructor
	///
	/// \param[in] other SDL2pp::Texture object to move data from
	///
	////////////////////////////////////////////////////////////
	Texture(Texture&& other) noexcept : texture_(other.texture_) {
		other.texture_ = nullptr;
	}

	////////////////////////////////////////////////////////////
	/// \brief Move assignment
	///
	/// \param[in] other SDL2pp::Texture object to move data from
	///
	/// \returns Reference to self
	///
	////////////////////////////////////////////////////////////
	Texture& operator=(Texture&& other) noexcept {
		if (&other == this)
			return *this;
		if (texture_ != nullptr)
			SDL_DestroyTexture(texture_);
		texture_ = other.texture_;
		other.texture_ = nullptr;
		return *this;
	}

	////////////////////////////////////////////////////////////
	/// \brief Deleted copy constructor
	///
	/// This class is not copyable
	///
	////////////////////////////////////////////////////////////
	Texture(const Texture& other) = delete;

	////////////////////////////////////////////////////////////
	/// \brief Deleted assignment operator
	///
	/// This class is not copyable
	///
	////////////////////////////////////////////////////////////
	Texture& operator=(const Texture& other) = delete;

	////////////////////////////////////////////////////////////
	/// \brief Get pointer to managed SDL_Texture structure
	///
	/// \returns Pointer to managed SDL_Texture structure
	///
	////////////////////////////////////////////////////////////
	SDL_Texture* Get() const {
		return texture_;
	}

	////////////////////////////////////////////////////////////
	/// \brief Update the given texture rectangle with new pixel data
	///
	/// \param[in] rect Rect representing the area to update, or NullOpt to
	///                 update the entire texture
	/// \param[in] pixels Raw pixel data
	/// \param[in] pitch Number of bytes in a row of pixel data, including
	///                  padding between lines
	///
	/// \returns Reference to self
	///
	/// \throws SDL2pp::Exception
	///
	/// \see http://wiki.libsdl.org/SDL_UpdateTexture
	///
	////////////////////////////////////////////////////////////
	Texture& Update(const Optional<Rect>& rect, const void* pixels, int pitch) {
		if (SDL_UpdateTexture(texture_, rect ? &*rect : nullptr, pixels, pitch) != 0)
			throw Exception("SDL_UpdateTexture");
		return *this;
	}

	////////////////////////////////////////////////////////////
	/// \brief Update the given texture rectangle with new pixel data taken from surface
	///
	/// \param[in] rect Rect representing the area to update, or NullOpt to
	///                 update the entire texture
	/// \param[in] surface Surface to take pixel data from
	///
	/// \note No scaling is performed in this routine, so if rect and surface
	///       sizes do not match, cropping is performed as appropriate
	/// \note If surface and texture pixel formats do not match, surface is
	///       automatically converted to texture format
	///
	/// \returns Reference to self
	///
	/// \throws SDL2pp::Exception
	///
	/// \see http://wiki.libsdl.org/SDL_UpdateTexture
	///
	////////////////////////////////////////////////////////////
	Texture& Update(const Optional<Rect>& rect, Surface& surface) {
		Rect real_rect = rect ? *rect : Rect(0, 0, GetWidth(), GetHeight());

		real_rect.w = std::min(real_rect.w, surface.GetWidth());
		real_rect.h = std::min(real_rect.h, surface.GetHeight());

		if (GetFormat() == surface.GetFormat()) {
			Surface::LockHandle lock = surface.Lock();

			return Update(real_rect, lock.GetPixels(), lock.GetPitch());
		} else {
			Surface converted = surface.Convert(GetFormat());
			Surface::LockHandle lock = converted.Lock();

			return Update(real_rect, lock.GetPixels(), lock.GetPitch());
		}
	}

	////////////////////////////////////////////////////////////
	/// \brief Update the given texture rectangle with new pixel data taken from surface
	///
	/// \param[in] rect Rect representing the area to update, or NullOpt to
	///                 update the entire texture
	/// \param[in] surface Surface to take pixel data from
	///
	/// \note No scaling is performed in this routine, so if rect and surface
	///       sizes do not match, cropping is performed as appropriate
	/// \note If surface and texture pixel formats do not match, surface is
	///       automatically converted to texture format
	///
	/// \returns Reference to self
	///
	/// \throws SDL2pp::Exception
	///
	/// \see http://wiki.libsdl.org/SDL_UpdateTexture
	///
	////////////////////////////////////////////////////////////
	Texture& Update(const Optional<Rect>& rect, Surface&& surface) {
		Rect real_rect = rect ? *rect : Rect(0, 0, GetWidth(), GetHeight());

		real_rect.w = std::min(real_rect.w, surface.GetWidth());
		real_rect.h = std::min(real_rect.h, surface.GetHeight());

		if (GetFormat() == surface.GetFormat()) {
			Surface::LockHandle lock = surface.Lock();

			return Update(real_rect, lock.GetPixels(), lock.GetPitch());
		} else {
			Surface converted = surface.Convert(GetFormat());
			Surface::LockHandle lock = converted.Lock();

			return Update(real_rect, lock.GetPixels(), lock.GetPitch());
		}
	}

	////////////////////////////////////////////////////////////
	/// \brief Update the given texture rectangle with new pixel data
	///
	/// \param[in] rect Rect representing the area to update, or NullOpt to
	///                 update the entire texture
	/// \param[in] yplane Raw pixel data for the Y plane
	/// \param[in] ypitch Number of bytes between rows of pixel data for the Y plane
	/// \param[in] uplane Raw pixel data for the U plane
	/// \param[in] upitch Number of bytes between rows of pixel data for the U plane
	/// \param[in] vplane Raw pixel data for the V plane
	/// \param[in] vpitch Number of bytes between rows of pixel data for the V plane
	///
	/// \returns Reference to self
	///
	/// \throws SDL2pp::Exception
	///
	/// \see http://wiki.libsdl.org/SDL_UpdateYUVTexture
	///
	////////////////////////////////////////////////////////////
	Texture& UpdateYUV(const Optional<Rect>& rect, const Uint8* yplane, int ypitch, const Uint8* uplane, int upitch, const Uint8* vplane, int vpitch) {
		if (SDL_UpdateYUVTexture(texture_, rect ? &*rect : nullptr, yplane, ypitch, uplane, upitch, vplane, vpitch) != 0)
			throw Exception("SDL_UpdateYUVTexture");
		return *this;
	}

	////////////////////////////////////////////////////////////
	/// \brief Set the blend mode for a texture, used by SDL2pp::Renderer::Copy
	///
	/// \param[in] blendMode SDL_BlendMode to use for texture blending
	///
	/// \returns Reference to self
	///
	/// \throws SDL2pp::Exception
	///
	/// \see http://wiki.libsdl.org/SDL_SetTextureBlendMode
	///
	////////////////////////////////////////////////////////////
	Texture& SetBlendMode(SDL_BlendMode blendMode = SDL_BLENDMODE_NONE) {
		if (SDL_SetTextureBlendMode(texture_, blendMode) != 0)
			throw Exception("SDL_SetTextureBlendMode");
		return *this;
	}

	////////////////////////////////////////////////////////////
	/// \brief Set an additional alpha value multiplied into render copy operations
	///
	/// \param[in] alpha Source alpha value multiplied into copy operations
	///
	/// \returns Reference to self
	///
	/// \throws SDL2pp::Exception
	///
	/// \see http://wiki.libsdl.org/SDL_SetTextureAlphaMod
	///
	////////////////////////////////////////////////////////////
	Texture& SetAlphaMod(Uint8 alpha = 255) {
		if (SDL_SetTextureAlphaMod(texture_, alpha) != 0)
			throw Exception("SDL_SetTextureAlphaMod");
		return *this;
	}

	////////////////////////////////////////////////////////////
	/// \brief Set an additional color value multiplied into render copy operations
	///
	/// \param[in] r Red color value multiplied into copy operations
	/// \param[in] g Green color value multiplied into copy operations
	/// \param[in] b Blue color value multiplied into copy operations
	///
	/// \returns Reference to self
	///
	/// \throws SDL2pp::Exception
	///
	/// \see http://wiki.libsdl.org/SDL_SetTextureColorMod
	///
	////////////////////////////////////////////////////////////
	Texture& SetColorMod(Uint8 r = 255, Uint8 g = 255, Uint8 b = 255) {
		if (SDL_SetTextureColorMod(texture_, r, g, b) != 0)
			throw Exception("SDL_SetTextureColorMod");
		return *this;
	}

	////////////////////////////////////////////////////////////
	/// \brief Set an additional color value multiplied into render copy operations
	///
	/// \param[in] color Color to be used when multiplied into render copy operations
	///
	/// \returns Reference to self
	///
	/// \throws SDL2pp::Exception
	///
	/// \see http://wiki.libsdl.org/SDL_SetTextureAlphaMod
	/// \see http://wiki.libsdl.org/SDL_SetTextureColorMod
	///
	////////////////////////////////////////////////////////////
	Texture& SetColorAndAlphaMod(const Color& color = Color{255, 255, 255, SDL_ALPHA_OPAQUE}) {
		return SetColorMod(color.r, color.g, color.b).SetAlphaMod(color.a);
	}

	////////////////////////////////////////////////////////////
	/// \brief Lock texture for write-only pixel access
	///
	/// \param[in] rect Rect representing area to lock for access
	///                 (NullOpt to lock entire texture)
	///
	/// \return Lock handle used to access pixel data and to control lock lifetime
	///
	/// \throws SDL2pp::Exception
	///
	/// \see http://wiki.libsdl.org/SDL_LockTexture
	///
	////////////////////////////////////////////////////////////
	LockHandle Lock(const Optional<Rect>& rect = NullOpt) {
		return LockHandle(this, rect);
	}

	////////////////////////////////////////////////////////////
	/// \brief Get texture format
	///
	/// \return Texture raw format
	///
	/// \throws SDL2pp::Exception
	///
	/// \see http://wiki.libsdl.org/SDL_QueryTexture
	/// \see http://wiki.libsdl.org/SDL_QueryTexture#format
	/// \see http://wiki.libsdl.org/SDL_PixelFormatEnum
	///
	////////////////////////////////////////////////////////////
	Uint32 GetFormat() const {
		Uint32 format;
		if (SDL_QueryTexture(texture_, &format, nullptr, nullptr, nullptr) != 0)
			throw Exception("SDL_QueryTexture");
		return format;
	}

	////////////////////////////////////////////////////////////
	/// \brief Get texture access mode
	///
	/// \return Texture access pattern
	///
	/// \throws SDL2pp::Exception
	///
	/// \see http://wiki.libsdl.org/SDL_QueryTexture
	/// \see http://wiki.libsdl.org/SDL_TextureAccess
	///
	////////////////////////////////////////////////////////////
	int GetAccess() const {
		int access;
		if (SDL_QueryTexture(texture_, nullptr, &access, nullptr, nullptr) != 0)
			throw Exception("SDL_QueryTexture");
		return access;
	}

	////////////////////////////////////////////////////////////
	/// \brief Get texture image width
	///
	/// \return Texture width in pixels
	///
	/// \throws SDL2pp::Exception
	///
	/// \see http://wiki.libsdl.org/SDL_QueryTexture
	///
	////////////////////////////////////////////////////////////
	int GetWidth() const {
		int w;
		if (SDL_QueryTexture(texture_, nullptr, nullptr, &w, nullptr) != 0)
			throw Exception("SDL_QueryTexture");
		return w;
	}

	////////////////////////////////////////////////////////////
	/// \brief Get texture image height
	///
	/// \return Texture height in pixels
	///
	/// \throws SDL2pp::Exception
	///
	/// \see http://wiki.libsdl.org/SDL_QueryTexture
	///
	////////////////////////////////////////////////////////////
	int GetHeight() const {
		int h;
		if (SDL_QueryTexture(texture_, nullptr, nullptr, nullptr, &h) != 0)
			throw Exception("SDL_QueryTexture");
		return h;
	}

	////////////////////////////////////////////////////////////
	/// \brief Get texture image size
	///
	/// \return SDL2pp::Point representing texture dimensions in pixels
	///
	/// \throws SDL2pp::Exception
	///
	/// \see http://wiki.libsdl.org/SDL_QueryTexture
	///
	////////////////////////////////////////////////////////////
	Point GetSize() const {
		int w, h;
		if (SDL_QueryTexture(texture_, nullptr, nullptr, &w, &h) != 0)
			throw Exception("SDL_QueryTexture");
		return Point(w, h);
	}

	////////////////////////////////////////////////////////////
	/// \brief Get the additional alpha value multiplied into render copy operations
	///
	/// \return Current alpha value
	///
	/// \throws SDL2pp::Exception
	///
	/// \see http://wiki.libsdl.org/SDL_GetTextureAlphaMod
	///
	////////////////////////////////////////////////////////////
	Uint8 GetAlphaMod() const {
		Uint8 alpha;
		if (SDL_GetTextureAlphaMod(texture_, &alpha) != 0)
			throw Exception("SDL_GetTextureAlphaMod");
		return alpha;
	}

	////////////////////////////////////////////////////////////
	/// \brief Get the blend mode used for texture copy operations
	///
	/// \return Current SDL_BlendMode
	///
	/// \throws SDL2pp::Exception
	///
	/// \see http://wiki.libsdl.org/SDL_GetTextureBlendMode
	///
	////////////////////////////////////////////////////////////
	SDL_BlendMode GetBlendMode() const {
		SDL_BlendMode mode;
		if (SDL_GetTextureBlendMode(texture_, &mode) != 0)
			throw Exception("SDL_GetTextureBlendMode");
		return mode;
	}

	////////////////////////////////////////////////////////////
	/// \brief Get the additional color value multiplied into render copy operations
	///
	/// \param[out] r Variable to be filled in with the current red color value
	/// \param[out] g Variable to be filled in with the current green color value
	/// \param[out] b Variable to be filled in with the current blue color value
	///
	/// \throws SDL2pp::Exception
	///
	/// \see http://wiki.libsdl.org/SDL_GetTextureColorMod
	///
	////////////////////////////////////////////////////////////
	void GetColorMod(Uint8& r, Uint8& g, Uint8 &b) const {
		if (SDL_GetTextureColorMod(texture_, &r, &g, &b) != 0)
			throw Exception("SDL_GetTextureColorMod");
	}

	////////////////////////////////////////////////////////////
	/// \brief Get the additional color value multiplied into render copy operations
	///
	/// \return Color object with the values used to do render copy operations
	///
	/// \throws SDL2pp::Exception
	///
	/// \see http://wiki.libsdl.org/SDL_GetTextureAlphaMod
	/// \see http://wiki.libsdl.org/SDL_GetTextureColorMod
	///
	////////////////////////////////////////////////////////////
	Color GetColorAndAlphaMod() const {
		Color color;
		GetColorMod(color.r, color.g, color.b);
		color.a = GetAlphaMod();
		return color;
	}
};

}

#endif

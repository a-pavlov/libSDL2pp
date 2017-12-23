/*
  libSDL2pp - C++11 bindings/wrapper for SDL2
  Copyright (C) 2015-2016 Dmitry Marakasov <amdmi3@amdmi3.ru>

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

#ifndef SDL2PP_MIXER_HH
#define SDL2PP_MIXER_HH

#include <functional>
#include <memory>

#include <SDL_stdinc.h>
#include <SDL_mixer.h>

#include <SDL2pp/Chunk.hh>
#include <SDL2pp/Music.hh>
#include <SDL2pp/Exception.hh>

namespace SDL2pp {


////////////////////////////////////////////////////////////
/// \brief Audio mixer
///
/// \ingroup mixer
///
/// \headerfile SDL2pp/Mixer.hh
///
/// This class represents open SDL_mixer audio device. Object
/// of this class must be constructed before creating any
/// SDL2pp:Chunk's.
///
////////////////////////////////////////////////////////////
class Mixer {
public:
	typedef void (*ChannelFinishedHandler)(int); ///< Function type for channel finished callback
	typedef void (*MusicFinishedHandler)();      ///< Function type for music finished callback

	typedef std::function<void(Uint8 *stream, int len)> MusicHook; ///< Custom music hook

private:
	bool open_;
	std::unique_ptr<MusicHook> current_music_hook_;

public:

	///@{
	/// \name Construction and destruction

	////////////////////////////////////////////////////////////
	/// \brief Construct a mixer and open an audio device
	///
	/// \param[in] frequency Output sampling frequency in samples
	///                      per second (Hz). You might use
	///                      MIX_DEFAULT_FREQUENCY(22050) since that
	///                      is a good value for most games
	/// \param[in] format Output sample format
	/// \param[in] channels Number of sound channels in output. Set
	///                     to 2 for stereo, 1 for mono. This has
	///                     nothing to do with mixing channels
	/// \param[in] chunksize Bytes used per output sample
	///
	/// \throws SDL2pp::Exception
	///
	/// \see https://www.libsdl.org/projects/SDL_mixer/docs/SDL_mixer.html#SEC11
	///
	////////////////////////////////////////////////////////////
	Mixer(int frequency, Uint16 format, int channels, int chunksize) : open_(true) {
		if (Mix_OpenAudio(frequency, format, channels, chunksize) != 0)
			throw Exception("Mix_OpenAudio");
	}

	////////////////////////////////////////////////////////////
	/// \brief Destructor
	///
	/// \see https://www.libsdl.org/projects/SDL_mixer/docs/SDL_mixer.html#SEC12
	///
	////////////////////////////////////////////////////////////
	~Mixer() {
		if (open_)
			Mix_CloseAudio();
	}

	///@}

	///@{
	/// \name Copy and move

	////////////////////////////////////////////////////////////
	/// \brief Move constructor
	///
	/// \param[in] other SDL2pp::Mixer object to move data from
	///
	////////////////////////////////////////////////////////////
	Mixer(Mixer&& other) noexcept : open_(other.open_), current_music_hook_(std::move(other.current_music_hook_)) {
		other.open_ = false;
	}

	////////////////////////////////////////////////////////////
	/// \brief Move assignment operator
	///
	/// \param[in] other SDL2pp::Mixer object to move data from
	///
	/// \returns Reference to self
	///
	////////////////////////////////////////////////////////////
	Mixer& operator=(Mixer&& other) noexcept {
		if (&other == this)
			return *this;
		if (open_)
			Mix_CloseAudio();
		open_ = other.open_;
		current_music_hook_ = std::move(other.current_music_hook_);
		other.open_ = false;
		return *this;
	}

	////////////////////////////////////////////////////////////
	/// \brief Deleted copy constructor
	///
	/// This class is not copyable
	///
	////////////////////////////////////////////////////////////
	Mixer(const Mixer& other) = delete;

	////////////////////////////////////////////////////////////
	/// \brief Deleted assignment operator
	///
	/// This class is not copyable
	///
	////////////////////////////////////////////////////////////
	Mixer& operator=(const Mixer& other) = delete;

	///@}

	///@{
	/// \name Channels: setup

	////////////////////////////////////////////////////////////
	/// \brief Set the number of channels to mix
	///
	/// \param[in] numchans Number of channels to allocate for mixing
	///
	/// \returns The number of channels allocated
	///
	/// \see https://www.libsdl.org/projects/SDL_mixer/docs/SDL_mixer.html#SEC26
	///
	////////////////////////////////////////////////////////////
	int AllocateChannels(int numchans) {
		return Mix_AllocateChannels(numchans);
	}

	////////////////////////////////////////////////////////////
	/// \brief Get the number of channels being mixed
	///
	/// \returns The number of channels allocated
	///
	/// \see https://www.libsdl.org/projects/SDL_mixer/docs/SDL_mixer.html#SEC26
	///
	////////////////////////////////////////////////////////////
	int GetNumChannels() const {
		return Mix_AllocateChannels(-1);
	}

	////////////////////////////////////////////////////////////
	/// \brief Set the mix volume of a channel
	///
	/// \param[in] channel Channel to set mix volume for.
	///                    -1 will set the volume for all allocated
	///                    channels.
	/// \param[in] volume The volume to use from 0 to MIX_MAX_VOLUME(128)
	///
	/// \returns Current volume of the channel. If channel is -1,
	///          the average volume is returned
	///
	/// \see https://www.libsdl.org/projects/SDL_mixer/docs/SDL_mixer.html#SEC27
	///
	////////////////////////////////////////////////////////////
	int SetVolume(int channel, int volume) {
		return Mix_Volume(channel, volume);
	}

	////////////////////////////////////////////////////////////
	/// \brief Get the mix volume of a channel
	///
	/// \param[in] channel Channel to set mix volume for.
	///                    -1 will return the average volume.
	///
	/// \returns Current volume of the channel. If channel is -1,
	///          the average volume is returned
	///
	/// \see https://www.libsdl.org/projects/SDL_mixer/docs/SDL_mixer.html#SEC27
	///
	////////////////////////////////////////////////////////////
	int GetVolume(int channel) const {
		return Mix_Volume(channel, -1);
	}

	///@}

	///@{
	/// \name Channels: playing

	////////////////////////////////////////////////////////////
	/// \brief Play loop
	///
	/// \param[in] channel Channel to play on, or -1 for the first
	///                    free unreserved channel
	/// \param[in] chunk Sample to play
	/// \param[in] loops Number of loops, -1 is infinite loops.
	///                  Passing one here plays the sample twice (1 loop).
	///
	/// \returns The channel the sample is played on
	///
	/// \throws SDL2pp::Exception
	///
	/// \see https://www.libsdl.org/projects/SDL_mixer/docs/SDL_mixer.html#SEC28
	///
	////////////////////////////////////////////////////////////
	int PlayChannel(int channel, const Chunk& chunk, int loops = 0) {
		int chan;
		if ((chan = Mix_PlayChannel(channel, chunk.Get(), loops)) == -1)
			throw Exception("Mix_PlayChannel");
		return chan;
	}

	////////////////////////////////////////////////////////////
	/// \brief Play loop and limit by time
	///
	/// \param[in] channel Channel to play on, or -1 for the first
	///                    free unreserved channel
	/// \param[in] chunk Sample to play
	/// \param[in] loops Number of loops, -1 is infinite loops.
	///                  Passing one here plays the sample twice (1 loop).
	/// \param[in] ticks Millisecond limit to play sample, at most.
	///                  If not enough loops or the sample chunk is not
	///                  long enough, then the sample may stop before
	///                  this timeout occurs. -1 means play forever
	///
	/// \returns The channel the sample is played on
	///
	/// \throws SDL2pp::Exception
	///
	/// \see https://www.libsdl.org/projects/SDL_mixer/docs/SDL_mixer.html#SEC29
	///
	////////////////////////////////////////////////////////////
	int PlayChannel(int channel, const Chunk& chunk, int loops, int ticks) {
		int chan;
		if ((chan = Mix_PlayChannelTimed(channel, chunk.Get(), loops, ticks)) == -1)
			throw Exception("Mix_PlayChannelTimed");
		return chan;
	}

	////////////////////////////////////////////////////////////
	/// \brief Play loop with fade in
	///
	/// \param[in] channel Channel to play on, or -1 for the first
	///                    free unreserved channel
	/// \param[in] chunk Sample to play
	/// \param[in] loops Number of loops, -1 is infinite loops.
	///                  Passing one here plays the sample twice (1 loop).
	/// \param[in] ms Milliseconds of time that the fade-in effect
	///               should take to go from silence to full volume
	///
	/// \returns The channel the sample is played on
	///
	/// \throws SDL2pp::Exception
	///
	/// \see https://www.libsdl.org/projects/SDL_mixer/docs/SDL_mixer.html#SEC30
	///
	////////////////////////////////////////////////////////////
	int FadeInChannel(int channel, const Chunk& chunk, int loops, int ms) {
		int chan;
		if ((chan = Mix_FadeInChannel(channel, chunk.Get(), loops, ms)) == -1)
			throw Exception("Mix_FadeInChannel");
		return chan;
	}

	////////////////////////////////////////////////////////////
	/// \brief  loop with fade in and limit by time
	///
	/// \param[in] channel Channel to play on, or -1 for the first
	///                    free unreserved channel
	/// \param[in] chunk Sample to play
	/// \param[in] loops Number of loops, -1 is infinite loops.
	///                  Passing one here plays the sample twice (1 loop).
	/// \param[in] ms Milliseconds of time that the fade-in effect
	///               should take to go from silence to full volume
	/// \param[in] ticks Millisecond limit to play sample, at most.
	///                  If not enough loops or the sample chunk is not
	///                  long enough, then the sample may stop before
	///                  this timeout occurs. -1 means play forever
	///
	/// \returns The channel the sample is played on
	///
	/// \throws SDL2pp::Exception
	///
	/// \see https://www.libsdl.org/projects/SDL_mixer/docs/SDL_mixer.html#SEC31
	///
	////////////////////////////////////////////////////////////
	int FadeInChannel(int channel, const Chunk& chunk, int loops, int ms, int ticks) {
		int chan;
		if ((chan = Mix_FadeInChannelTimed(channel, chunk.Get(), loops, ms, ticks)) == -1)
			throw Exception("Mix_FadeInChannelTimed");
		return chan;
	}


	///@}

	///@{
	/// \name Channels: pausing

	////////////////////////////////////////////////////////////
	/// \brief Pause a channel
	///
	/// \param[in] channel Channel to pause on, or -1 for all channels
	///
	/// \see https://www.libsdl.org/projects/SDL_mixer/docs/SDL_mixer.html#SEC32
	///
	////////////////////////////////////////////////////////////
	void PauseChannel(int channel = -1) {
		Mix_Pause(channel);
	}

	////////////////////////////////////////////////////////////
	/// \brief Resume a paused channel
	///
	/// \param[in] channel Channel to resume playing, or -1 for all channels
	///
	/// \see https://www.libsdl.org/projects/SDL_mixer/docs/SDL_mixer.html#SEC33
	///
	////////////////////////////////////////////////////////////
	void ResumeChannel(int channel = -1) {
		Mix_Resume(channel);
	}

	///@}

	///@{
	/// \name Channels: stopping

	////////////////////////////////////////////////////////////
	/// \brief Stop playing on a channel
	///
	/// \param[in] channel Channel to stop playing, or -1 for all channels
	///
	/// \see https://www.libsdl.org/projects/SDL_mixer/docs/SDL_mixer.html#SEC34
	///
	////////////////////////////////////////////////////////////
	void HaltChannel(int channel = -1) {
		Mix_HaltChannel(channel);
	}

	////////////////////////////////////////////////////////////
	/// \brief Change the timed stoppage of a channel
	///
	/// \param[in] channel Channel to stop playing, or -1 for all channels
	/// \param[in] ticks Millisecons until channel(s) halt playback
	///
	/// \returns Number of channels set to expire. Whether or not they are active
	///
	/// \see https://www.libsdl.org/projects/SDL_mixer/docs/SDL_mixer.html#SEC35
	///
	////////////////////////////////////////////////////////////
	int ExpireChannel(int channel, int ticks) {
		return Mix_ExpireChannel(channel, ticks);
	}

	////////////////////////////////////////////////////////////
	/// \brief Stop playing channel after timed fade out
	///
	/// \param[in] channel Channel to fade out, or -1 to fade all channels out
	/// \param[in] ms Milliseconds of time that the fade-out effect should
	///               take to go to silence, starting now
	///
	/// \returns The number of channels set to fade out
	///
	/// \see https://www.libsdl.org/projects/SDL_mixer/docs/SDL_mixer.html#SEC36
	///
	////////////////////////////////////////////////////////////
	int FadeOutChannel(int channel, int ms) {
		return Mix_FadeOutChannel(channel, ms);
	}

	////////////////////////////////////////////////////////////
	/// \brief Set callback for when channel finishes playing
	///
	/// \param[in] channel_finished Function to call when any channel
	///                             finishes playback
	///
	/// \note Since Mix_ChannelFinished doesn't take any custom data
	///       pointer, unfortunately there's no safe way of using
	///       std::function here.
	///
	/// \see https://www.libsdl.org/projects/SDL_mixer/docs/SDL_mixer.html#SEC37
	///
	////////////////////////////////////////////////////////////
	void SetChannelFinishedHandler(ChannelFinishedHandler channel_finished) {
		Mix_ChannelFinished(channel_finished);
	}

	////////////////////////////////////////////////////////////
	/// \brief Remove callback for when channel finishes playing
	///
	/// \see https://www.libsdl.org/projects/SDL_mixer/docs/SDL_mixer.html#SEC37
	///
	////////////////////////////////////////////////////////////
	void RemoveChannelFinishedHandler() {
		Mix_ChannelFinished(nullptr);
	}

	///@}

	///@{
	/// \name Channels: info

	////////////////////////////////////////////////////////////
	/// \brief Get the active playing status of a channel
	///
	/// \param[in] channel Channel to test whether it is playing or not.
	///                    -1 will tell you how many channels are playing
	///
	/// \returns Zero if the channel is not playing. Otherwise if you passed
	///          in -1, the number of channels playing is returned. If you
	///          passed in a specific channel, then 1 is returned if it is
	///          playing.
	///
	/// \see https://www.libsdl.org/projects/SDL_mixer/docs/SDL_mixer.html#SEC38
	///
	////////////////////////////////////////////////////////////
	int IsChannelPlaying(int channel) const {
		return Mix_Playing(channel);
	}

	////////////////////////////////////////////////////////////
	/// \brief Get the pause status of a channel
	///
	/// \param[in] channel Channel to test whether it is paused or not.
	///                    -1 will tell you how many channels are playing
	///
	/// \returns Zero if the channel is not paused. Otherwise if you passed
	///          in -1, the number of paused channels is returned. If you
	///          passed in a specific channel, then 1 is returned if it is
	///          paused.
	///
	/// \see https://www.libsdl.org/projects/SDL_mixer/docs/SDL_mixer.html#SEC39
	///
	////////////////////////////////////////////////////////////
	int IsChannelPaused(int channel) const {
		return Mix_Paused(channel);
	}

	////////////////////////////////////////////////////////////
	/// \brief Get the fade status of a channel
	///
	/// \param[in] which Channel to get the fade activity status from
	///
	/// \returns The fading status
	///
	/// \see https://www.libsdl.org/projects/SDL_mixer/docs/SDL_mixer.html#SEC40
	///
	////////////////////////////////////////////////////////////
	Mix_Fading GetChannelFading(int which) const {
		return Mix_FadingChannel(which);
	}

	///@}

	///@{
	/// \name Groups: setup

	////////////////////////////////////////////////////////////
	/// \brief Prevent channels from being used in default group
	///
	/// \param[in] num Number of channels to reserve from default mixing.
	///                Zero removes all reservations
	///
	/// \returns The number of channels reserved. Never fails,
	///          but may return less channels than you ask for,
	///          depending on the number of channels previously
	///          allocated
	///
	/// \see https://www.libsdl.org/projects/SDL_mixer/docs/SDL_mixer.html#SEC43
	///
	////////////////////////////////////////////////////////////
	int ReserveChannels(int num) {
		return Mix_ReserveChannels(num);
	}

	////////////////////////////////////////////////////////////
	/// \brief Add/remove channel to/from group
	///
	/// \param[in] which Channel number to assign tag to.
	/// \param[in] tag A group number. Any positive numbers
	///                (including zero). -1 is the default group.
	///                Use -1 to remove a group tag essentially.
	///
	/// \throws SDL2pp::Exception
	///
	/// \see https://www.libsdl.org/projects/SDL_mixer/docs/SDL_mixer.html#SEC44
	///
	////////////////////////////////////////////////////////////
	void GroupChannel(int which, int tag) {
		if (Mix_GroupChannel(which, tag) != 1)
			throw Exception("Mix_GroupChannel");
	}

	////////////////////////////////////////////////////////////
	/// \brief Add/remove segment of channels to/from group
	///
	/// \param[in] from First Channel number of channels to assign tag to.
	/// \param[in] to Last Channel number of channels to assign tag to
	/// \param[in] tag A group number. Any positive numbers (including zero).
	///                -1 is the default group. Use -1 to remove a group tag
	///                essentially.
	///
	/// \throws SDL2pp::Exception
	///
	/// \see https://www.libsdl.org/projects/SDL_mixer/docs/SDL_mixer.html#SEC45
	///
	////////////////////////////////////////////////////////////
	void GroupChannels(int from, int to, int tag) {
		if (Mix_GroupChannels(from, to, tag) != to - from + 1)
			throw Exception("Mix_GroupChannels");
	}

	///@}

	///@{
	/// \name Groups: info

	////////////////////////////////////////////////////////////
	/// \brief Get number of channels in group
	///
	/// \param[in] tag A group number
	///
	/// \returns The number of channels found in the group
	///
	/// \see https://www.libsdl.org/projects/SDL_mixer/docs/SDL_mixer.html#SEC46
	///
	////////////////////////////////////////////////////////////
	int GetGroupNumChannels(int tag) const {
		return Mix_GroupCount(tag);
	}

	////////////////////////////////////////////////////////////
	/// \brief Get first inactive channel in group
	///
	/// \param[in] tag A group number
	///
	/// \returns The channel found on success. -1 is returned when
	///          no channels in the group are available
	///
	/// \see https://www.libsdl.org/projects/SDL_mixer/docs/SDL_mixer.html#SEC47
	///
	////////////////////////////////////////////////////////////
	int GetGroupAvailableChannel(int tag) const {
		return Mix_GroupAvailable(tag);
	}

	////////////////////////////////////////////////////////////
	/// \brief Get oldest busy channel in group
	///
	/// \param[in] tag A group number
	///
	/// \returns The channel found on success. -1 is returned when
	///          no channels in the group are available
	///
	/// \see https://www.libsdl.org/projects/SDL_mixer/docs/SDL_mixer.html#SEC48
	///
	////////////////////////////////////////////////////////////
	int GetGroupOldestChannel(int tag) const {
		return Mix_GroupOldest(tag);
	}

	////////////////////////////////////////////////////////////
	/// \brief Get youngest busy channel in group
	///
	/// \param[in] tag A group number
	///
	/// \returns The channel found on success. -1 is returned when
	///          no channels in the group are available
	///
	/// \see https://www.libsdl.org/projects/SDL_mixer/docs/SDL_mixer.html#SEC49
	///
	////////////////////////////////////////////////////////////
	int GetGroupNewestChannel(int tag) const {
		return Mix_GroupNewer(tag);
	}

	///@}

	///@{
	/// \name Groups: stopping

	////////////////////////////////////////////////////////////
	/// \brief Fade out a group over time
	///
	/// \param[in] tag Group to fade out
	/// \param[in] ms Milliseconds of time that the fade-out effect
	///               should take to go to silence, starting now
	///
	/// \returns The number of channels set to fade out
	///
	/// \see https://www.libsdl.org/projects/SDL_mixer/docs/SDL_mixer.html#SEC50
	///
	////////////////////////////////////////////////////////////
	int FadeOutGroup(int tag, int ms) {
		return Mix_FadeOutGroup(tag, ms);
	}

	////////////////////////////////////////////////////////////
	/// \brief Stop a group
	///
	/// \param[in] tag Group to fade out
	///
	/// \see https://www.libsdl.org/projects/SDL_mixer/docs/SDL_mixer.html#SEC51
	///
	////////////////////////////////////////////////////////////
	void HaltGroup(int tag) {
		Mix_HaltGroup(tag);
	}

	///@}

	///@{
	/// \name Music: playing

	////////////////////////////////////////////////////////////
	/// \brief Play music
	///
	/// \param[in] music Music to play
	/// \param[in] loops number of times to play through the music.
	///                  0 plays the music zero times...
	///                  -1 plays the music forever
	///
	/// \see https://www.libsdl.org/projects/SDL_mixer/docs/SDL_mixer.html#SEC57
	///
	////////////////////////////////////////////////////////////
	void PlayMusic(const Music& music, int loops = -1) {
		if (Mix_PlayMusic(music.Get(), loops) == -1)
			throw Exception("Mix_PlayMusic");
	}

	////////////////////////////////////////////////////////////
	/// \brief Play music, with looping, and fade in
	///
	/// \param[in] music Music to play
	/// \param[in] loops number of times to play through the music.
	///                  0 plays the music zero times...
	///                  -1 plays the music forever
	/// \param[in] ms Milliseconds for the fade-in effect to complete
	///
	/// \see https://www.libsdl.org/projects/SDL_mixer/docs/SDL_mixer.html#SEC57
	///
	////////////////////////////////////////////////////////////
	void FadeInMusic(const Music& music, int loops = -1, int ms = 0) {
		if (Mix_FadeInMusic(music.Get(), loops, ms) == -1)
			throw Exception("Mix_FadeInMusic");
	}

	////////////////////////////////////////////////////////////
	/// \brief Hook for a custom music player
	///
	/// \param[in] hook Music player mixer function
	///
	/// \see https://www.libsdl.org/projects/SDL_mixer/docs/SDL_mixer.html#SEC60
	///
	////////////////////////////////////////////////////////////
	void SetMusicHook(MusicHook&& hook) {
		if (!hook) {
			Mix_HookMusic(nullptr, nullptr);
			current_music_hook_.reset(nullptr);
			return;
		}

		current_music_hook_.reset(new MusicHook(std::move(hook)));

		Mix_HookMusic([](void *udata, Uint8 *stream, int len) {
			static_cast<std::function<void(Uint8 *stream, int len)>*>(udata)->operator()(stream, len);
		}, current_music_hook_.get());
	}

	///@}

	///@{
	/// \name Music: settings

	////////////////////////////////////////////////////////////
	/// \brief Set music volume
	///
	/// \param[in] volume The volume to use from 0 to MIX_MAX_VOLUME(128)
	///
	/// \returns The previous volume setting
	///
	/// \see https://www.libsdl.org/projects/SDL_mixer/docs/SDL_mixer.html#SEC61
	///
	////////////////////////////////////////////////////////////
	int SetMusicVolume(int volume) {
		return Mix_VolumeMusic(volume);
	}

	////////////////////////////////////////////////////////////
	/// \brief Get music volume
	///
	/// \returns Current volume setting
	///
	/// \see https://www.libsdl.org/projects/SDL_mixer/docs/SDL_mixer.html#SEC61
	///
	////////////////////////////////////////////////////////////
	int GetMusicVolume() const {
		return Mix_VolumeMusic(-1);
	}

	////////////////////////////////////////////////////////////
	/// \brief Pause music
	///
	/// \see https://www.libsdl.org/projects/SDL_mixer/docs/SDL_mixer.html#SEC62
	///
	////////////////////////////////////////////////////////////
	void PauseMusic() {
		Mix_PauseMusic();
	}

	////////////////////////////////////////////////////////////
	/// \brief Resume paused music
	///
	/// \see https://www.libsdl.org/projects/SDL_mixer/docs/SDL_mixer.html#SEC63
	///
	////////////////////////////////////////////////////////////
	void ResumeMusic() {
		Mix_ResumeMusic();
	}

	////////////////////////////////////////////////////////////
	/// \brief Rewind music to beginning
	///
	/// \see https://www.libsdl.org/projects/SDL_mixer/docs/SDL_mixer.html#SEC64
	///
	////////////////////////////////////////////////////////////
	void RewindMusic() {
		Mix_RewindMusic();
	}

	////////////////////////////////////////////////////////////
	/// \brief Set position of playback in stream
	///
	/// \param[in] position Posistion to play from
	///
	/// \see https://www.libsdl.org/projects/SDL_mixer/docs/SDL_mixer.html#SEC65
	///
	////////////////////////////////////////////////////////////
	void SetMusicPosition(double position) {
		if (Mix_SetMusicPosition(position) == -1)
			throw Exception("Mix_SetMusicPosition");
	}

	///@}

	///@{
	/// \name Music: stopping

	////////////////////////////////////////////////////////////
	/// \brief Stop music playback
	///
	/// \see https://www.libsdl.org/projects/SDL_mixer/docs/SDL_mixer.html#SEC67
	///
	////////////////////////////////////////////////////////////
	void HaltMusic() {
		Mix_HaltMusic();
	}

	////////////////////////////////////////////////////////////
	/// \brief Stop music, with fade out
	///
	/// \param[in] ms Milliseconds of time that the fade-out effect
	///               should take to go to silence, starting now.
	///
	/// \returns True in success, false on failure
	///
	/// \see https://www.libsdl.org/projects/SDL_mixer/docs/SDL_mixer.html#SEC68
	///
	////////////////////////////////////////////////////////////
	bool FadeOutMusic(int ms) {
		return Mix_FadeOutMusic(ms);
	}

	////////////////////////////////////////////////////////////
	/// \brief Set a callback for when music stops
	///
	/// \param[in] music_finished Function to call when music stops
	///
	/// \note Since Mix_HookMusicFinished  doesn't take any custom data
	///       pointer, unfortunately there's no safe way of using
	///       std::function here.
	///
	/// \see https://www.libsdl.org/projects/SDL_mixer/docs/SDL_mixer.html#SEC69
	///
	////////////////////////////////////////////////////////////
	void SetMusicFinishedHandler(MusicFinishedHandler music_finished) {
		Mix_HookMusicFinished(music_finished);
	}

	////////////////////////////////////////////////////////////
	/// \brief Remove a callback for when music stops
	///
	/// \see https://www.libsdl.org/projects/SDL_mixer/docs/SDL_mixer.html#SEC69
	///
	////////////////////////////////////////////////////////////
	void RemoveMusicFinishedHandler() {
		Mix_HookMusicFinished(nullptr);
	}

	///@}

	///@{
	/// \name Music: info

	////////////////////////////////////////////////////////////
	/// \brief Test whether music is playing
	///
	/// \returns True if music is actively playing
	///
	/// \see https://www.libsdl.org/projects/SDL_mixer/docs/SDL_mixer.html#SEC71
	///
	////////////////////////////////////////////////////////////
	bool IsMusicPlaying() const {
		return Mix_PlayingMusic();
	}

	////////////////////////////////////////////////////////////
	/// \brief Test whether music is paused
	///
	/// \returns True if music is paused
	///
	/// \see https://www.libsdl.org/projects/SDL_mixer/docs/SDL_mixer.html#SEC72
	///
	////////////////////////////////////////////////////////////
	bool IsMusicPaused() const {
		return Mix_PausedMusic();
	}

	////////////////////////////////////////////////////////////
	/// \brief Get status of current music fade activity
	///
	/// \returns The fading status
	///
	/// \see https://www.libsdl.org/projects/SDL_mixer/docs/SDL_mixer.html#SEC73
	///
	////////////////////////////////////////////////////////////
	Mix_Fading GetMusicFading() const {
		return Mix_FadingMusic();
	}

	///@}

	///@{
	/// \name Effects

	// TODO: custom effects

	////////////////////////////////////////////////////////////
	/// \brief Stereo panning
	///
	/// \param[in] channel Channel number to register this effect on or
	///                    MIX_CHANNEL_POST to process the postmix stream
	/// \param[in] left Volume for the left channel, range is 0 (silence)
	///                 to 255 (loud)
	/// \param[in] right Volume for the right channel, range is 0 (silence)
	///                  to 255 (loud)
	///
	/// \throws SDL2pp::Exception
	///
	/// \see https://www.libsdl.org/projects/SDL_mixer/docs/SDL_mixer.html#SEC80
	///
	////////////////////////////////////////////////////////////
	void SetPanning(int channel, Uint8 left, Uint8 right) {
		if (Mix_SetPanning(channel, left, right) == 0)
			throw Exception("Mix_SetPanning");
	}

	////////////////////////////////////////////////////////////
	/// \brief Disable stereo panning
	///
	/// \param[in] channel Channel number to unregister this effect from or
	///                    MIX_CHANNEL_POST to unregister from the postmix stream
	///
	/// \throws SDL2pp::Exception
	///
	/// \see https://www.libsdl.org/projects/SDL_mixer/docs/SDL_mixer.html#SEC80
	///
	////////////////////////////////////////////////////////////
	void UnsetPanning(int channel) {
		if (Mix_SetPanning(channel, 255, 255) == 0)
			throw Exception("Mix_SetPanning");
	}

	////////////////////////////////////////////////////////////
	/// \brief Distance attenuation (volume)
	///
	/// \param[in] channel Channel number to register this effect on or
	///                    MIX_CHANNEL_POST to process the postmix stream
	/// \param[in] distance Specify the distance from the listener,
	///                     from 0 (close/loud) to 255 (far/quiet)
	///
	/// \throws SDL2pp::Exception
	///
	/// \see https://www.libsdl.org/projects/SDL_mixer/docs/SDL_mixer.html#SEC81
	///
	////////////////////////////////////////////////////////////
	void SetDistance(int channel, Uint8 distance) {
		if (Mix_SetDistance(channel, distance) == 0)
			throw Exception("Mix_SetDistance");
	}

	////////////////////////////////////////////////////////////
	/// \brief Disable distance attenuation
	///
	/// \param[in] channel Channel number to unregister this effect from or
	///                    MIX_CHANNEL_POST to unregister from the postmix stream
	///
	/// \throws SDL2pp::Exception
	///
	/// \see https://www.libsdl.org/projects/SDL_mixer/docs/SDL_mixer.html#SEC81
	///
	////////////////////////////////////////////////////////////
	void UnsetDistance(int channel) {
		if (Mix_SetDistance(channel, 0) == 0)
			throw Exception("Mix_SetDistance");
	}

	////////////////////////////////////////////////////////////
	/// \brief Panning (angular) and distance
	///
	/// \param[in] channel Channel number to register this effect on or
	///                    MIX_CHANNEL_POST to process the postmix stream
	/// \param[in] angle Direction in relation to forward from 0 to 360 degrees.
	///                  Larger angles will be reduced to this range using angles
	///                  % 360. 0 = directly in front, 90 = directly to the right
	///                  etc.
	/// \param[in] distance Specify the distance from the listener,
	///                     from 0 (close/loud) to 255 (far/quiet)
	///
	/// \throws SDL2pp::Exception
	///
	/// \see https://www.libsdl.org/projects/SDL_mixer/docs/SDL_mixer.html#SEC82
	///
	////////////////////////////////////////////////////////////
	void SetPosition(int channel, Sint16 angle, Uint8 distance) {
		if (Mix_SetPosition(channel, angle, distance) == 0)
			throw Exception("Mix_SetPosition");
	}

	////////////////////////////////////////////////////////////
	/// \brief Disable panning and distance
	///
	/// \param[in] channel Channel number to unregister this effect from or
	///                    MIX_CHANNEL_POST to unregister from the postmix stream
	///
	/// \throws SDL2pp::Exception
	///
	/// \see https://www.libsdl.org/projects/SDL_mixer/docs/SDL_mixer.html#SEC82
	///
	////////////////////////////////////////////////////////////
	void UnsetPosition(int channel) {
		if (Mix_SetPosition(channel, 0, 0) == 0)
			throw Exception("Mix_SetPosition");
	}

	////////////////////////////////////////////////////////////
	/// \brief Swap stereo left and right
	///
	/// \param[in] channel Channel number to register this effect on or
	///                    MIX_CHANNEL_POST to process the postmix stream
	///
	/// \throws SDL2pp::Exception
	///
	/// \see https://www.libsdl.org/projects/SDL_mixer/docs/SDL_mixer.html#SEC83
	///
	////////////////////////////////////////////////////////////
	void SetReverseStereo(int channel) {
		if (Mix_SetReverseStereo(channel, 1) == 0)
			throw Exception("Mix_SetReverseStereo");
	}

	////////////////////////////////////////////////////////////
	/// \brief Disable stereo swapping
	///
	/// \param[in] channel Channel number to unregister this effect from or
	///                    MIX_CHANNEL_POST to unregister from the postmix stream
	///
	/// \throws SDL2pp::Exception
	///
	/// \see https://www.libsdl.org/projects/SDL_mixer/docs/SDL_mixer.html#SEC83
	///
	////////////////////////////////////////////////////////////
	void UnsetReverseStereo(int channel) {
		if (Mix_SetReverseStereo(channel, 0) == 0)
			throw Exception("Mix_SetReverseStereo");
	}


	///@}
};

}

#endif

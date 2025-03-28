#pragma once

#if defined(DISABLE_NATIVE_MOVIEVIEW)

#include "UI/IMovieViewControl.h"

namespace DAVA
{
class Window;
class MovieViewControl : public IMovieViewControl
{
public:
#if defined(__DAVAENGINE_COREV2__)
    MovieViewControl(Window* /*w*/)
    {
    }
#else
    MovieViewControl() = default;
#endif
    virtual ~MovieViewControl() = default;

    // Initialize the control.
    void Initialize(const Rect& rect) override
    {
    }

    // Open the Movie.
    void OpenMovie(const FilePath& moviePath, const OpenMovieParams& params) override
    {
    }

    // Position/visibility.
    void SetRect(const Rect& rect) override
    {
    }
    void SetVisible(bool isVisible) override
    {
    }

    // Start/stop the video playback.
    void Play() override
    {
    }
    void Stop() override
    {
    }

    // Pause/resume the playback.
    void Pause() override
    {
    }
    void Resume() override
    {
    }

    // Whether the movie is being played?
    bool IsPlaying() const override
    {
        return false;
    }
};

} // namespace DAVA

#endif // DISABLE_NATIVE_MOVIEVIEW

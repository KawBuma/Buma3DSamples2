#pragma once

#include <chrono>

namespace buma::util
{

class Stopwatch
{
public:
    Stopwatch() : last_clock{ std::chrono::high_resolution_clock::now() } {}

    void Reset() { last_clock = std::chrono::high_resolution_clock::now(); }
    double GetElapsedSeconds() const { return std::chrono::duration<double>(std::chrono::high_resolution_clock::now() - last_clock).count(); }
    template<typename Period = std::milli>
    double GetElapsed() const { return std::chrono::duration<double, Period>(std::chrono::high_resolution_clock::now() - last_clock).count(); }

private:
    std::chrono::high_resolution_clock::time_point last_clock;
};

// Helper class for animation and simulation timing.
class StepTimer
{
public:
    using clock_t       = std::chrono::high_resolution_clock;
    using rep_t         = std::chrono::high_resolution_clock::rep;
    using duration_t    = std::chrono::high_resolution_clock::duration;
    using time_point_t  = std::chrono::high_resolution_clock::time_point;
    using period_t      = std::chrono::high_resolution_clock::period;

    static constexpr rep_t ticks_per_second = period_t::den;

public:
    StepTimer()
        : last_clock         { clock_t::now() }
        , elapsed_ticks      {}
        , total_ticks        {}
        , total_frame_count  {}
        , ticks_this_second  {}
        , frames_this_second {}
        , frames_per_second  {}
        , max_delta          {}
    {
        SetMaxDelta(1.0 / 20.0);
    }

    void Reset()
    {
        last_clock         = clock_t::now();
        elapsed_ticks      = duration_t(0);
        total_ticks        = duration_t(0);
        total_frame_count  = 0;
        ticks_this_second  = duration_t(0);
        frames_this_second = 0;
        frames_per_second  = 0;
    }

    void Tick()
    {
        auto&& now = clock_t::now();
        elapsed_ticks = now - last_clock;

        last_clock = now;

        total_ticks += elapsed_ticks;
        total_frame_count++;

        frames_this_second++;
        ticks_this_second += elapsed_ticks;
        if (ticks_this_second.count() >= ticks_per_second)
        {
            frames_per_second = frames_this_second;
            frames_this_second = 0;
            ticks_this_second %= ticks_per_second;
        }
        if (elapsed_ticks > max_delta)
            elapsed_ticks = max_delta;
    }

    uint64_t    GetElapsedTicks()   const { return static_cast<uint64_t>(elapsed_ticks.count()); }
    double      GetElapsedSeconds() const { return TicksToSeconds(elapsed_ticks.count()); }
    template<typename Period = std::milli>
    double      GetElapsed()        const { return TicksToSeconds(elapsed_ticks.count()) * static_cast<double>(Period::den); }

    uint64_t    GetTotalTicks()     const { return static_cast<uint64_t>(total_ticks.count()); }
    double      GetTotalSeconds()   const { return TicksToSeconds(total_ticks.count()); }
    uint64_t    GetTotalFrames()    const { return total_frame_count; }

    uint64_t    GetFramesPerSecond()const { return frames_per_second; }

    bool        IsOneSecElapsed() const { return frames_this_second == 0; }

    void        SetMaxDelta(double _in_seconds) { max_delta = std::chrono::duration_cast<duration_t>(std::chrono::duration<double>(_in_seconds)); }
    double      GetMaxDelta() const { return TicksToSeconds(max_delta.count()); }

    static constexpr double TicksToSeconds(rep_t _ticks) { return static_cast<double>(_ticks) / static_cast<double>(ticks_per_second); }
    static constexpr rep_t  SecondsToTicks(double _seconds) { return static_cast<uint64_t>(_seconds * static_cast<double>(ticks_per_second)); }

private:
    time_point_t    last_clock;
    duration_t      elapsed_ticks;

    duration_t      total_ticks;
    uint64_t        total_frame_count;

    duration_t      ticks_this_second;
    uint64_t        frames_this_second;
    uint64_t        frames_per_second;

    duration_t      max_delta;

};


}// namespace buma::util

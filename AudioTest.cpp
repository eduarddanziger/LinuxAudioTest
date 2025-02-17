#include <iostream>
#include <cstdlib>
#include <alsa/asoundlib.h>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <memory>

// Custom deleter for snd_mixer_t
struct MixerHandleDeleter {
    void operator()(snd_mixer_t* handle) const {
        if (handle) {
            snd_mixer_close(handle);
        }
    }
};
using MixerHandleRaiiPtr = std::unique_ptr<snd_mixer_t, MixerHandleDeleter>;

struct SimpleMixerElementDeleter {
    void operator()(snd_mixer_selem_id_t* handle) const {
        if (handle) {
            snd_mixer_selem_id_free(handle);
        }
    }
};
using SimpleMixerElementIdRaiiPtr = std::unique_ptr<snd_mixer_selem_id_t, SimpleMixerElementDeleter>;

int main()
{
    // Set up spdlog to log to both console and file
    const auto consoleSink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
    const auto fileSink = std::make_shared<spdlog::sinks::basic_file_sink_mt>("AudioTest.log", true);
    std::vector<spdlog::sink_ptr> sinks {consoleSink, fileSink};
    const auto logger = std::make_shared<spdlog::logger>("multi_sink", sinks.begin(), sinks.end());
    set_default_logger(logger);
    spdlog::set_level(spdlog::level::info); // Set global log level to info

    // Set custom log pattern (excluding logger name)
    spdlog::set_pattern("%^%Y-%m-%d %H:%M:%S.%e [%l] %v%$");

    {
        constexpr auto envName = "PULSE_SERVER";
        // Check and output PULSE_SERVER environment variable
        if (const char * pulseServerName = std::getenv(envName)) // NOLINT(concurrency-mt-unsafe)
        {
            spdlog::info("{} environment variable: {}", envName, pulseServerName);
        }
        else
        {
            spdlog::info("{} environment variable is not set.", envName);
        }
    }

    int err;
    MixerHandleRaiiPtr handleSmartPtr;
    {
        snd_mixer_t * handleTmp;
        if ((err = snd_mixer_open(&handleTmp, 0)) < 0)
        {
            spdlog::error("Error opening mixer: {}", snd_strerror(err));
            return 1;
        }
        handleSmartPtr = MixerHandleRaiiPtr(handleTmp);
    }

    if ((err = snd_mixer_attach(handleSmartPtr.get(), "default")) < 0)
    {
        spdlog::error("Error attaching mixer: {}", snd_strerror(err));
        return 1;
    }

    if ((err = snd_mixer_selem_register(handleSmartPtr.get(), nullptr, nullptr)) < 0)
    {
        spdlog::error("Error registering mixer element: {}", snd_strerror(err));
        return 1;
    }

    if ((err = snd_mixer_load(handleSmartPtr.get())) < 0)
    {
        spdlog::error("Error loading mixer elements: {}", snd_strerror(err));
        return 1;
    }

    long minVolume, maxVolume, currentVolume;

    SimpleMixerElementIdRaiiPtr elementIdSmartPtr;
    {
        snd_mixer_selem_id_t * simpleMixerElementIdTmp;
        if ((err = snd_mixer_selem_id_malloc(&simpleMixerElementIdTmp)) < 0)
        {
            spdlog::error("Can not allocate Mixer Simple Element Id structure");
            return 2;
        }
        elementIdSmartPtr = SimpleMixerElementIdRaiiPtr(simpleMixerElementIdTmp);
    }

    snd_mixer_selem_id_set_index(elementIdSmartPtr.get(), 0);
    snd_mixer_selem_id_set_name(elementIdSmartPtr.get(), "Master");

    snd_mixer_elem_t * mixerElement = snd_mixer_find_selem(handleSmartPtr.get(), elementIdSmartPtr.get());
    if (!mixerElement)
    {
        spdlog::error("Element not found!");
        return 2;
    }

    if ((err = snd_mixer_selem_get_playback_volume_range(mixerElement, &minVolume, &maxVolume)) < 0)
    {
        spdlog::error("Error getting min / max volume: {}", snd_strerror(err));
        return 2;
    }

    if ((err = snd_mixer_selem_get_playback_volume(mixerElement, SND_MIXER_SCHN_FRONT_LEFT, &currentVolume)) < 0)
    {
        spdlog::error("Error getting current volume: {}", snd_strerror(err));
        return 2;
    }

    spdlog::info("Current volume is: {}, between min {} and max {}.", currentVolume, minVolume, maxVolume);
    spdlog::info(
        "Setting volume to half a current volume {}. If it gets less or equal to the maximum divided by 10, namely {}, set it to half a maximum {}.",
        currentVolume / 2, maxVolume / 10, maxVolume / 2);

    long newVolume = currentVolume / 2;
    if (newVolume <= maxVolume / 10)
    {
        newVolume = maxVolume / 2;
    }

    snd_mixer_selem_set_playback_volume_all(mixerElement, newVolume);
    spdlog::info("Volume set to {}", newVolume);

    return 0;
}

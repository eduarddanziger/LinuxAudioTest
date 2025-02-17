#include <iostream>
#include <cstdlib>
#include <alsa/asoundlib.h>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <memory>
#include <stdexcept>

// Custom deleter for snd_mixer_t
struct MixerHandleDeleter {
    void operator()(snd_mixer_t * handle) const
    {
        if (handle)
        {
            snd_mixer_close(handle);
        }
    }
};

using MixerHandleRaiiPtr = std::unique_ptr<snd_mixer_t, MixerHandleDeleter>;

struct SimpleMixerElementDeleter {
    void operator()(snd_mixer_selem_id_t * handle) const
    {
        if (handle)
        {
            snd_mixer_selem_id_free(handle);
        }
    }
};

using SimpleMixerElementIdRaiiPtr = std::unique_ptr<snd_mixer_selem_id_t, SimpleMixerElementDeleter>;

void CheckErrorAndThrowIfAny(int err, const std::string & message)
{
    if (err < 0)
    {
        throw std::runtime_error(message + ": " + snd_strerror(err));
    }
}

int main()
{
    try
    {
        // Set up spdlog to log to both console and rotating file
        const auto consoleSink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
        const auto fileSink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>("AudioTest.log", 10240, 3);
        std::vector<spdlog::sink_ptr> sinks{consoleSink, fileSink};
        const auto logger = std::make_shared<spdlog::logger>("multi_sink", sinks.begin(), sinks.end());
        spdlog::set_default_logger(logger);
        spdlog::set_level(spdlog::level::info); // Set global log level to info

        // Set custom log pattern (excluding logger name)
        spdlog::set_pattern("%^%Y-%m-%d %H:%M:%S.%e [%l] %v%$");
        spdlog::info("Starting...");

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
            CheckErrorAndThrowIfAny(snd_mixer_open(&handleTmp, 0), "Error opening mixer");
            handleSmartPtr = MixerHandleRaiiPtr(handleTmp);
        }

        CheckErrorAndThrowIfAny(snd_mixer_attach(handleSmartPtr.get(), "default"), "Error attaching mixer");
        CheckErrorAndThrowIfAny(snd_mixer_selem_register(handleSmartPtr.get(), nullptr, nullptr),
                                "Error registering mixer element");
        CheckErrorAndThrowIfAny(snd_mixer_load(handleSmartPtr.get()), "Error loading mixer elements");

        long minVolume, maxVolume, currentVolume;

        SimpleMixerElementIdRaiiPtr elementIdSmartPtr;
        {
            snd_mixer_selem_id_t * simpleMixerElementIdTmp;
            CheckErrorAndThrowIfAny(snd_mixer_selem_id_malloc(&simpleMixerElementIdTmp),
                                    "Can not allocate Mixer Simple Element Id structure");
            elementIdSmartPtr = SimpleMixerElementIdRaiiPtr(simpleMixerElementIdTmp);
        }

        snd_mixer_selem_id_set_index(elementIdSmartPtr.get(), 0);
        snd_mixer_selem_id_set_name(elementIdSmartPtr.get(), "Master");

        snd_mixer_elem_t * mixerElement = snd_mixer_find_selem(handleSmartPtr.get(), elementIdSmartPtr.get());
        if (!mixerElement)
        {
            throw std::runtime_error("Element not found!");
        }

        CheckErrorAndThrowIfAny(snd_mixer_selem_get_playback_volume_range(mixerElement, &minVolume, &maxVolume),
                                "Error getting min / max volume");
        CheckErrorAndThrowIfAny(
            snd_mixer_selem_get_playback_volume(mixerElement, SND_MIXER_SCHN_FRONT_LEFT, &currentVolume),
            "Error getting current volume");

        spdlog::info("Current volume is: {}, between min {} and max {}.", currentVolume, minVolume, maxVolume);
        spdlog::info(
            "Setting volume to half a current volume {}. If it gets less or equal to the maximum divided by 10, namely {}, set it to half a maximum {}.",
            currentVolume / 2, maxVolume / 10, maxVolume / 2);

        long newVolume = currentVolume / 2;
        if (newVolume <= maxVolume / 10)
        {
            newVolume = maxVolume / 2;
        }

        CheckErrorAndThrowIfAny(snd_mixer_selem_set_playback_volume_all(mixerElement, newVolume),
                                "Error setting playback volume");
        spdlog::info("Volume set to {}", newVolume);
    }
    catch (const std::exception & e)
    {
        spdlog::error("...Ended with an exception: {}", e.what());
        return 1;
    }
    spdlog::info("...Ended successfully");

    return 0;
}

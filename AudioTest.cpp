#include <iostream>
#include <cstdlib>  // For getenv
#include <alsa/asoundlib.h>
#include <fmt/core.h>  // For fmt::print
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

int main() {
    // Check and output PULSE_SERVER environment variable
    if (const char * pulseServerName = std::getenv("PULSE_SERVER"))  // NOLINT(concurrency-mt-unsafe)
    {
        fmt::print("PULSE_SERVER: {}\n", pulseServerName);
    }
    else
    {
        fmt::print("PULSE_SERVER environment variable is not set.\n");
    }

    long minVolume, maxVolume, currentVolume;
    snd_mixer_selem_id_t* simpleMixerElementId;

    int err;

    MixerHandleRaiiPtr handleSmartPtr;
    {
        snd_mixer_t* handleTmp;
        if ((err = snd_mixer_open(&handleTmp, 0)) < 0) {
            fmt::print("Error opening mixer: {}\n", snd_strerror(err));
            return 1;
        }
        handleSmartPtr = MixerHandleRaiiPtr(handleTmp);
    }

    if ((err = snd_mixer_attach(handleSmartPtr.get(), "default")) < 0) {
        fmt::print("Error attaching mixer: {}\n", snd_strerror(err));
        return 1;
    }

    if ((err = snd_mixer_selem_register(handleSmartPtr.get(), nullptr, nullptr)) < 0) {
        fmt::print("Error registering mixer element: {}\n", snd_strerror(err));
        return 1;
    }

    if ((err = snd_mixer_load(handleSmartPtr.get())) < 0) {
        fmt::print("Error loading mixer elements: {}\n", snd_strerror(err));
        return 1;
    }

    snd_mixer_selem_id_malloc(&simpleMixerElementId);
    snd_mixer_selem_id_set_index(simpleMixerElementId, 0);
    snd_mixer_selem_id_set_name(simpleMixerElementId, "Master");

    snd_mixer_elem_t* mixerElement = snd_mixer_find_selem(handleSmartPtr.get(), simpleMixerElementId);
    if (!mixerElement) {
	    fmt::print("Element not found!\n");
        return 2;
    }

    if ((err = snd_mixer_selem_get_playback_volume_range(mixerElement, &minVolume, &maxVolume)) < 0) {
        fmt::print("Error getting min / max volume: {}\n", snd_strerror(err));
        snd_mixer_selem_id_free(simpleMixerElementId);
        return 1;
    }


    if ((err = snd_mixer_selem_get_playback_volume(mixerElement, SND_MIXER_SCHN_FRONT_LEFT, &currentVolume)) < 0) {
	    fmt::print("Error getting current volume: {}\n", snd_strerror(err));
	    snd_mixer_selem_id_free(simpleMixerElementId);
	    return 1;
    }

    fmt::print("Current volume is: {}, between min {} and max {}.\n", currentVolume, minVolume, maxVolume);
    fmt::print("Setting volume to half a current volume {}. If it gets less or equal to the maximum divided by 10, namely {}, set it to half a maximum {}.\n", currentVolume / 2, maxVolume / 10, maxVolume / 2);

    long newVolume = currentVolume / 2;
    if (newVolume <= maxVolume / 10)
    {
        newVolume = maxVolume / 2;
    }

    snd_mixer_selem_set_playback_volume_all(mixerElement, newVolume);
    fmt::print("Volume set to {}\n", newVolume);

    snd_mixer_selem_id_free(simpleMixerElementId);
    return 0;
}
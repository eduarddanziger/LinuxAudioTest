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
    const char* pulse_server = std::getenv("PULSE_SERVER");
    if (pulse_server) {
        fmt::print("PULSE_SERVER: {}\n", pulse_server);
    } else {
        fmt::print("PULSE_SERVER environment variable is not set.\n");
    }

    long min, max, current_volume;
    snd_mixer_selem_id_t* sid;
    const char* card = "default";  // Standardgerät
    const char* selem_name = "Master";

    int err;

    MixerHandleRaiiPtr handleSmartPtr;

    {
        snd_mixer_t* handle_tmp;
        if ((err = snd_mixer_open(&handle_tmp, 0)) < 0) {
            fmt::print("Error opening mixer: {}\n", snd_strerror(err));
            return 1;
        }
        handleSmartPtr = MixerHandleRaiiPtr(handle_tmp);
    }

    if ((err = snd_mixer_attach(handleSmartPtr.get(), card)) < 0) {
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

    snd_mixer_selem_id_malloc(&sid);
    snd_mixer_selem_id_set_index(sid, 0);
    snd_mixer_selem_id_set_name(sid, selem_name);

    snd_mixer_elem_t* elem = snd_mixer_find_selem(handleSmartPtr.get(), sid);

    if (!elem) {
	    fmt::print("Element not found!\n");
        return 2;
    }

    snd_mixer_selem_get_playback_volume_range(elem, &min, &max);
    if ((err = snd_mixer_selem_get_playback_volume(elem, SND_MIXER_SCHN_FRONT_LEFT, &current_volume)) < 0) {
	    fmt::print("Error getting current volume: {}\n", snd_strerror(err));
	    snd_mixer_selem_id_free(sid);
	    return 1;
    }
    const long volume = max / 2;  // Setze Lautstärke auf 50%
    snd_mixer_selem_set_playback_volume_all(elem, volume);
    fmt::print("Volume set to {}\n", volume);

    snd_mixer_selem_id_free(sid);
    return 0;
}
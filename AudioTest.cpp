#include <iostream>
#include <cstdlib>  // For getenv
#include <alsa/asoundlib.h>
#include <fmt/core.h>  // For fmt::print

int main() {
    // Check and output PULSE_SERVER environment variable
    const char* pulse_server = std::getenv("PULSE_SERVER");
    if (pulse_server) {
        fmt::print("PULSE_SERVER: {}\n", pulse_server);
    } else {
        fmt::print("PULSE_SERVER environment variable is not set.\n");
    }

    long min, max, current_volume;
    snd_mixer_t* handle;
    snd_mixer_selem_id_t* sid;
    const char* card = "default";  // Standardgerät
    const char* selem_name = "Master";

    int err;

    if ((err = snd_mixer_open(&handle, 0)) < 0) {
        fmt::print("Error opening mixer: {}\n", snd_strerror(err));
        return 1;
    }

    if ((err = snd_mixer_attach(handle, card)) < 0) {
        fmt::print("Error attaching mixer: {}\n", snd_strerror(err));
        snd_mixer_close(handle);
        return 1;
    }

    if ((err = snd_mixer_selem_register(handle, nullptr, nullptr)) < 0) {
        fmt::print("Error registering mixer element: {}\n", snd_strerror(err));
        snd_mixer_close(handle);
        return 1;
    }

    if ((err = snd_mixer_load(handle)) < 0) {
        fmt::print("Error loading mixer elements: {}\n", snd_strerror(err));
        snd_mixer_close(handle);
        return 1;
    }

    snd_mixer_selem_id_malloc(&sid);
    snd_mixer_selem_id_set_index(sid, 0);
    snd_mixer_selem_id_set_name(sid, selem_name);

    snd_mixer_elem_t* elem = snd_mixer_find_selem(handle, sid);

    if (elem) {
        snd_mixer_selem_get_playback_volume_range(elem, &min, &max);
        if ((err = snd_mixer_selem_get_playback_volume(elem, SND_MIXER_SCHN_FRONT_LEFT, &current_volume)) < 0) {
            fmt::print("Error getting current volume: {}\n", snd_strerror(err));
            snd_mixer_selem_id_free(sid);
            snd_mixer_close(handle);
            return 1;
        }
        const long volume = max / 2;  // Setze Lautstärke auf 50%
        snd_mixer_selem_set_playback_volume_all(elem, volume);
        fmt::print("Volume set to {}\n", volume);
    } else {
        fmt::print("Element not found!\n");
    }

    snd_mixer_selem_id_free(sid);
    snd_mixer_close(handle);
    return 0;
}
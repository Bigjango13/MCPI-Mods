#include <vector>
#include <string>
#include <algorithm>
#include <experimental/filesystem>

#include <libreborn/libreborn.h>
#include <media-layer/audio.h>

static std::string &get_prefix() {
    static std::string prefix = "";
    return prefix;
}
static std::vector<std::string> &get_audio_files() {
    static std::vector<std::string> audio_files = {};
    return audio_files;
}

HOOK(media_audio_play, void, (const char *source, const char *name, float x, float y, float z, float pitch, float volume, int is_ui)) {
    ensure_media_audio_play();
    std::vector<std::string> &audio_files = get_audio_files();
    if (std::find(audio_files.begin(), audio_files.end(), std::string(name)) != audio_files.end()) {
        // Play the custom sound instead
        std::string src = get_prefix() + name;
        real_media_audio_play(src.c_str(), name, x, y, z, pitch, volume, is_ui);
        return;
    }
    // Play the normal sound
    real_media_audio_play(source, name, x, y, z, pitch, volume, is_ui);
}

using std::experimental::filesystem::directory_iterator;
__attribute__((constructor)) static void init() {
    std::string home = std::string(getenv("HOME"));
    get_prefix() = home + "/.minecraft-pi/overrides/sounds/";
    // Loop through files
    for (auto &file : directory_iterator(get_prefix())) {
        std::string audio_file = file.path();
        audio_file.erase(0, audio_file.find_last_of('/') + 1);
        get_audio_files().push_back(audio_file);
        DEBUG("Overriding the sound %s", audio_file.c_str());
    }
}

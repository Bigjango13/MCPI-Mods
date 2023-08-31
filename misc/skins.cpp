// To use this mod, compile it (or grab it from the Releases page) and put it in ~/.minecraft-pi/mods/libskins.so
// Put the skins you want in ~/.minecraft-pi/overrides/images/skins/<username>.png (where <username> is any username, caps sensitive, URL encoded)
// This is completely client side, so don't install it on a server. Many skins can be found here: https://github.com/bsx-gh/bMCPIL-skins
// Using the bMCPIL (https://github.com/bsx-gh/bMCPIL) launcher will automagically sync skins from the bMCPIL-skins repo

#include <iomanip>
#include <algorithm>
#include <experimental/filesystem>

#include <symbols/minecraft.h>
#include <libreborn/libreborn.h>
#include <mods/misc/misc.h>

std::string url_encode(const std::string &value) {
    // Taken from https://stackoverflow.com/a/17708801, with adjustments
    std::ostringstream escaped;
    escaped.fill('0');
    escaped << std::hex;

    for (char c : value) {
        // Keep alphanumeric and other accepted characters intact
        if (std::isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~') {
            escaped << c;
        } else {
            // Any other characters are percent-encoded
            escaped << std::uppercase;
            escaped << '%' << std::setw(2) << int((unsigned char) c);
            escaped << std::nouppercase;
        }
    }

    return escaped.str();
}

// Stores the skins in the skin dir (~/.minecraft-pi/overrides/images/skins/*.png)
std::vector<std::string> skins = {};
std::vector<std::string> &get_skins(){
    // Needed because constructors are werid, see https://discord.com/channels/740287937727561779/889201475362893844/1004268212487192596
    static std::vector<std::string> skins;
    return skins;
}

// Runs on every tick and sets the skins.
void minecraft_on_tick(unsigned char *minecraft){
    static int oldPlayerAmount = 0;
    if (minecraft == NULL) {
        oldPlayerAmount = 0;
        return;
    }
    // Gets the level from minecraft
    unsigned char *level = *(unsigned char **) (minecraft + Minecraft_level_property_offset);
    if (level == NULL) {
        oldPlayerAmount = 0;
        return;
    }

    // Gets the players
    std::vector<unsigned char *> players = *(std::vector<unsigned char *> *) (level + Level_players_property_offset);
    if (players.size() <= oldPlayerAmount) return;
    // Loops through the players and get the usernames
    std::string username;
    for (unsigned char *player : players){
        // Get the players username
        username = *(std::string *) (player + Player_username_property_offset);
        username = url_encode(username);
        // Check if the username is in the skins vector
        if (find(get_skins().begin(), get_skins().end(), username) != get_skins().end()){
            // Set the players skin
            *(std::string *)(player + 0xb54) = "skins/" + username + ".png";
        }
    }
    oldPlayerAmount = players.size();
    return;
}


char *local_skin = NULL;
__attribute__((destructor)) static void deinit() {
    free(local_skin);
}

using std::experimental::filesystem::directory_iterator;
__attribute__((constructor)) static void init() {
    // Run minecraft_on_tick every tick.
    misc_run_on_tick(minecraft_on_tick);

    // Fill the skins vector
    std::string home = std::string(getenv("HOME"));
    std::string username = url_encode(*default_username);
    // Loop through files
    for (const auto &file : directory_iterator(home+"/.minecraft-pi/overrides/images/skins/")) {
        std::string skin = file.path();
        // If file ends with ".png"
        if (skin.compare(skin.length() - 4, 4, ".png") == 0){
            // Turns ~/.minecraft-pi/overrides/images/skins/FG6.png into FG6
            skin.erase(0, skin.find_last_of('/') + 1);
            skin.erase(skin.length()-4, skin.length());
            // Push
            get_skins().push_back(skin);
            // Check to see if that's the local players skin
            if (skin == username) {
                size_t len = 11 + skin.size();
                local_skin = (char*) malloc(len);
                local_skin[len - 1] = 0;
                strcpy(local_skin, "skins/");
                strcat(local_skin, skin.c_str());
                strcat(local_skin, ".png");
            }
        }
    }

    // Set the local users skin (for the hand texture)
    if (local_skin) {
        patch_address((void *) 0x4c294, (void *) local_skin);
    }
}

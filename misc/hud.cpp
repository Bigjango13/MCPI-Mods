#include <cstdio>
#include <iomanip>
#include <sstream>

#include <libreborn/libreborn.h>
#include <symbols/minecraft.h>

#define MAX_LEN (255 - 1)
unsigned char *minecraft = NULL;
void on_tick(unsigned char *mcpi) {
    minecraft = mcpi;
}

void ungetOffset(unsigned char *offsetData, float *x, float *y, float *z){
    // Removes the offset from the xyz.
    *x = *x + *(float*)(offsetData + 4);
    *y = *y + *(float*)(offsetData + 8);
    *z = *z + *(float*)(offsetData + 0xc);
}

int format_hud(char *str, __attribute__((unused)) const char *format, ...){
    unsigned char *player = *(unsigned char **) (minecraft + Minecraft_player_property_offset);
    if (player == NULL) return 0;
    float x = *(float *) (player + Entity_x_property_offset);
    float y = *(float *) (player + Entity_y_property_offset);
    float z = *(float *) (player + Entity_z_property_offset);
    unsigned char *command_server = *(unsigned char **) (minecraft + Minecraft_command_server_property_offset);
    unsigned char *offsetData = (unsigned char *)(command_server + 0x1c);
    ungetOffset(offsetData, &x, &y, &z);

    std::stringstream stream;
    stream << std::fixed << std::setprecision(2) << x << ", " <<  y << ", " << z << "\nMCPI-Reborn (FG6 edition)\n";

    unsigned char *level = *(unsigned char **) (minecraft + Minecraft_level_property_offset);
    if (level) {
        std::vector<unsigned char *> *players = (std::vector<unsigned char *> *) (level + Level_players_property_offset);
        for (unsigned char *player : *players) {
            if (player == NULL) continue;
            std::string *name = (std::string *) (player + Player_username_property_offset);
            stream << "\n" << *name;
        }
    }

    memcpy(str, stream.str().c_str(), MAX_LEN);

    return 0;
}

__attribute__((constructor)) static void init() {
    overwrite_call((void *) 0x269d8, (void *) format_hud);
}

#include <cstdio>

#include <libreborn/libreborn.h>
#include <symbols/minecraft.h>

extern "C"{
    unsigned char *get_minecraft();
    std::string get_world_name();
    bool in_local_world();
}

std::string hud_format = "%.2f, %.2f, %.2f  (%.2f, %.2f, %.2f)\nMCPI-Reborn\n\n%s";

void ungetOffset(unsigned char *offsetData, float *x, float *y, float *z){
    // Removes the offset from the xyz.
    *x = *x + *(float*)(offsetData + 4);
    *y = *y + *(float*)(offsetData + 8);
    *z = *z + *(float*)(offsetData + 0xc);
}

int format_hud(char *str, __attribute__((unused)) const char *format, ...){
    // Thanks https://stackoverflow.com/a/6229861
    unsigned char *player = *(unsigned char **) (get_minecraft() + Minecraft_player_property_offset);
    if (player == NULL) return 0;
    float x = *(float *) (player + Entity_x_property_offset);
    float y = *(float *) (player + Entity_y_property_offset);
    float z = *(float *) (player + Entity_z_property_offset);
    float realX = x;
    float realY = y;
    float realZ = z;
    unsigned char *command_server = *(unsigned char **) (get_minecraft() + Minecraft_command_server_property_offset);
    unsigned char *offsetData = (unsigned char *)(command_server + 0x1c);
    ungetOffset(offsetData, &x, &y, &z);
    char new_format[50];
    if (!in_local_world()){
        std::string server_format = hud_format + ":%s";
        std::string players_str;
        unsigned char *level = *(unsigned char **) (get_minecraft() + Minecraft_level_property_offset);
        std::vector<unsigned char *> players = *(std::vector<unsigned char *> *) (level + Level_players_property_offset);
        for (unsigned char* player : players) {
            players_str += "\n  ";
            players_str += *((std::string *) (player + Player_username_property_offset));
        }
        sprintf(new_format, server_format.c_str(), x, y, z, realX, realY, realZ, get_world_name().c_str(), players_str.c_str());
        //INFO("%s", new_format);
    } else {
        sprintf(new_format, hud_format.c_str(), x, y, z, realX, realY, realZ, get_world_name().c_str());
        //ERR("%s", new_format);
    }
    sprintf(str, "%s", new_format);
    return 0;
}

__attribute__((constructor)) static void init() {
    overwrite_call((void *) 0x269d8, (void *) format_hud);
}
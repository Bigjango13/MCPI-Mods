#include <cmath>

#include <symbols/minecraft.h>
#include <libreborn/libreborn.h>
#include <mods/misc/misc.h>

static ServerSideNetworkHandler_handle_t ServerSideNetworkHandler_handle_SignUpdatePacket = (ServerSideNetworkHandler_handle_t) 0x74ca8;
static void *ServerSideNetworkHandler_handle_SignUpdatePacket_vtable_addr = (void *) 0x109c64;

static uint32_t SignUpdatePacket_x_property_offset = 0xc; // int32_t
static uint32_t SignUpdatePacket_y_property_offset = 0x10; // int32_t
static uint32_t SignUpdatePacket_z_property_offset = 0x14; // int32_t
static uint32_t SignUpdatePacket_lines_property_offset = 0x18; // std::string *

float distance(float x, float y, float z, float x2, float y2, float z2){
    // Gets the distance in blocks from two coordinate
    float xd = x2 - x;
    float yd = y2 - y;
    float zd = z2 - z;
    return sqrt((xd * xd) + (yd * yd) + (zd * zd));
}

std::string getNearestPlayer(unsigned char *level, float x, float y, float z){
    // Loops through all players and returns the name of the one nearest to the specified coordinate.
    std::vector<unsigned char *> players = *(std::vector<unsigned char *> *) (level + Level_players_property_offset);
    float closestPos = 0xff;
    std::string closestPlayername = "?";
    for (unsigned char *player : players){
        // Gets the position of the player
        float x2 = *(float *) (player + Entity_x_property_offset);
        float y2 = *(float *) (player + Entity_y_property_offset);
        float z2 = *(float *) (player + Entity_z_property_offset);
        // Gets the distance and compares it to the current nearest player
        float dis = distance(x, y, z, x2, y2, z2);
        if (dis < closestPos){
            closestPos = dis;
            // Set the players name to the closest
            closestPlayername = *(std::string *) (player + Player_username_property_offset);
        }
    }
    return closestPlayername;
}

void ungetOffset(unsigned char *offsetData, int32_t *x, int32_t *y, int32_t *z){
    // This is just the code for getOffset but I changed - to +.
    // The ugly (int)*(float*) is because it is a float but needs to be an int.
    // To see what happens when I don't do that look here: https://cdn.discordapp.com/attachments/989954530982649886/995568148013129769/pic.png
    *x = *x + (int)*(float*)(offsetData + 4);
    *y = *y + (int)*(float*)(offsetData + 8);
    *z = *z + (int)*(float*)(offsetData + 0xc);
}

unsigned char *minecraft;
static void mcpi_callback(unsigned char *mcpi){
    // Runs on every tick, sets the minecraft varible.
    minecraft = mcpi;
}

static void ServerSideNetworkHandler_handle_SignUpdatePacket_injection(unsigned char *server_side_network_handler, struct RakNet_RakNetGUID *rak_net_guid, unsigned char *packet) {
    // Most of this functions code is from TheBrokenRail.
    // Call Original Method
    (*ServerSideNetworkHandler_handle_SignUpdatePacket)(server_side_network_handler, rak_net_guid, packet);

    // Get coordinates
    int x = *(int32_t *) (packet + SignUpdatePacket_x_property_offset);
    int y = *(int32_t *) (packet + SignUpdatePacket_y_property_offset);
    int z = *(int32_t *) (packet + SignUpdatePacket_z_property_offset);
    unsigned char *level = *(unsigned char **) (minecraft + Minecraft_level_property_offset);
    std::string username = getNearestPlayer(level, x, y, z);
    unsigned char *command_server = *(unsigned char **) (minecraft + Minecraft_command_server_property_offset);
    unsigned char *offsetData = (unsigned char *)(command_server + 0x1c);
    ungetOffset(offsetData, &x, &y, &z);
    // Get lines
    std::string *lines = (std::string *) (packet + SignUpdatePacket_lines_property_offset);
    std::string line = "";
    for (int i = 0; i < 4; i++) {
        line += lines[i]+"\\n";
    }
    // Print
    INFO("A sign saying '%s' was placed at (%i, %i, %i) by %s.", line.c_str(), x, y, z, username.c_str());
}

void Level_setTileAndData_injection(unsigned char *level, int32_t x, int32_t y, int32_t z, int32_t id, int32_t data){
    // Calls original method
    (*Level_setTileAndData)(level, x, y, z, id, data);
    if (id == 46) { // 46 is TNT
        std::string username = getNearestPlayer(level, x, y, z);
        unsigned char *command_server = *(unsigned char **) (minecraft + Minecraft_command_server_property_offset);
        unsigned char *offsetData = (unsigned char *)(command_server + 0x1c);
        // Unoffset the coordinates
        ungetOffset(offsetData, &x, &y, &z);
        INFO("%i was placed at (%i, %i, %i) by %s.", id, x, y, z, username.c_str());
    }
}

__attribute__((constructor)) static void init() {
    misc_run_on_update(mcpi_callback);
    overwrite_calls((void *) Level_setTileAndData, (void *) Level_setTileAndData_injection);
    patch_address(ServerSideNetworkHandler_handle_SignUpdatePacket_vtable_addr, (void *) ServerSideNetworkHandler_handle_SignUpdatePacket_injection);

}
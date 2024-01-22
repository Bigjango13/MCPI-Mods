#include <cmath>

#include <symbols/minecraft.h>
#include <libreborn/libreborn.h>
#include <mods/misc/misc.h>

// Reborn symbol split fix
#define Level_setTileAndData ((Level_setTileAndData_t) 0xa38b4)
#define ServerSideNetworkHandler_handle_SignUpdatePacket_non_virtual \
    ((ServerSideNetworkHandler_handle_SignUpdatePacket_t) 0x109c64)

// The code
float distance(float x, float y, float z, float x2, float y2, float z2) {
    // Gets the distance in blocks from two coordinate
    float xd = x2 - x;
    float yd = y2 - y;
    float zd = z2 - z;
    return sqrt((xd * xd) + (yd * yd) + (zd * zd));
}

std::string getNearestPlayer(Level *level, float x, float y, float z) {
    // Loops through all players and returns the name of the one nearest to the specified coordinate.
    float closestPos = 0xff;
    std::string closestPlayername = "?";
    for (auto const &player : level->players) {
        // Gets the distance and compares it to the current nearest player
        float dis = distance(x, y, z, player->x, player->y, player->z);
        if (dis < closestPos) {
            closestPos = dis;
            // Set the players name to the closest
            closestPlayername = player->username;
        }
    }
    return closestPlayername;
}

void ungetOffset(OffsetPosTranslator *pos_translator, int *x, int *y, int *z) {
    *x += pos_translator->x;
    *y += pos_translator->y;
    *z += pos_translator->z;
}

Minecraft *minecraft;
static void mcpi_callback(Minecraft *mcpi) {
    // Runs on every tick, sets the minecraft varible.
    minecraft = mcpi;
}

static void ServerSideNetworkHandler_handle_SignUpdatePacket_injection(
    ServerSideNetworkHandler *self, RakNet_RakNetGUID *guid, SignUpdatePacket *packet
) {
    // Call Original Method
    ServerSideNetworkHandler_handle_SignUpdatePacket_non_virtual(self, guid, packet);

    // Get coordinates
    int x = packet->x, y = packet->y, z = packet->z;
    std::string username = getNearestPlayer(minecraft->level, x, y, z);
    ungetOffset(&minecraft->command_server->pos_translator, &x, &y, &z);
    // Get lines
    std::string line = "";
    for (int i = 0; i < 4; i++) {
        line += packet->lines[i] + "\\n";
    }
    // Print
    INFO("A sign saying '%s' was placed at (%i, %i, %i) by %s.", line.c_str(), x, y, z, username.c_str());
}

void Level_setTileAndData_injection(Level *level, int32_t x, int32_t y, int32_t z, int32_t id, int32_t data) {
    // Calls original method
    Level_setTileAndData(level, x, y, z, id, data);
    if (id == 46) { // 46 is TNT
        std::string username = getNearestPlayer(level, x, y, z);
        // Unoffset the coordinates
        ungetOffset(&minecraft->command_server->pos_translator, &x, &y, &z);
        INFO("%i was placed at (%i, %i, %i) by %s.", id, x, y, z, username.c_str());
    }
}

__attribute__((constructor)) static void init() {
    misc_run_on_update(mcpi_callback);
    overwrite_calls((void *) Level_setTileAndData, (void *) Level_setTileAndData_injection);
    patch_address(
        (void *) ServerSideNetworkHandler_handle_SignUpdatePacket_non_virtual,
        (void *) ServerSideNetworkHandler_handle_SignUpdatePacket_injection
    );
}

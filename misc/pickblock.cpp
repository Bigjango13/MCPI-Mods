#include <SDL/SDL.h>

#include <libreborn/libreborn.h>
#include <symbols/minecraft.h>
#include <mods/misc/misc.h>

typedef unsigned char uchar;
Level_getTile_t Level_getData = (Level_getTile_t) 0xa3324;

FillingContainer_setItem_t FillingContainer_setItem = (FillingContainer_setItem_t) 0x91b00;
typedef bool (*FillingContainer_linkEmptySlot_t)(uchar *fillingContainer, int slot);
FillingContainer_linkEmptySlot_t FillingContainer_linkEmptySlot = (FillingContainer_linkEmptySlot_t) 0x92590;
typedef int (*FillingContainer_getFreeSlot_t)(uchar *fillingContainer);
FillingContainer_getFreeSlot_t FillingContainer_getFreeSlot = (FillingContainer_getFreeSlot_t) 0x91ffc;
typedef int (*FillingContainer_getSlot_t)(uchar *fillingContainer, int id);
FillingContainer_getSlot_t FillingContainer_getSlot = (FillingContainer_getSlot_t) 0x91ce0;
typedef int (*FillingContainer_linkSlot_t)(uchar *fillingContainer, int linked_slot, int unlinked_slot, bool push_aside);
FillingContainer_linkSlot_t FillingContainer_linkSlot = (FillingContainer_linkSlot_t) 0x92188;

typedef bool (*Inventory_add_t)(uchar *fillingContainer, ItemInstance *item);
Inventory_add_t Inventory_add = (Inventory_add_t) 0x8d078;
typedef bool (*Inventory_moveToSelectedSlot_t)(uchar *fillingContainer, int slot, bool);
Inventory_moveToSelectedSlot_t Inventory_moveToSelectedSlot = (Inventory_moveToSelectedSlot_t) 0x8d148;

// Decompiled from libminecraftpe
int FillingContainer_getSlot2(uchar *fillingContainer, int id, int aux) {
    std::vector<ItemInstance *> items = *(std::vector<ItemInstance *> *) (fillingContainer + 0x18);
    int linked_slots_length = *(int *) (fillingContainer + FillingContainer_linked_slots_length_property_offset);
    for (int i = linked_slots_length; i < items.size(); i++) {
        if (items[i] == NULL) continue;
        if (items[i]->id == id && items[i]->auxiliary == aux) {
            return i;
        }
    }
    return -1;
}

// My code
int unlink_slot(uchar *fillingContainer, int linked_slot) {
    if (linked_slot == -1) return -1;
    int linked_slots_length = *(int *) (fillingContainer + FillingContainer_linked_slots_length_property_offset);
    int *linked_slots = *(int **) (fillingContainer + FillingContainer_linked_slots_property_offset);
    for (int i = 0; i < linked_slots_length; i++) {
        int link = linked_slots[i];
        if (linked_slots[i] == linked_slot) return i;
    }
    return -1;
}

bool is_middle_click = false;
HOOK(SDL_PollEvent, int, (SDL_Event *event)) {
    ensure_SDL_PollEvent();
    int ret = (*real_SDL_PollEvent)(event);
    if (event != NULL && event->type == SDL_MOUSEBUTTONDOWN) {
        is_middle_click = event->button.button == SDL_BUTTON_MIDDLE && event->button.state != SDL_RELEASED;
    }
    return ret;
}

MouseBuildInput_tickBuild_t MouseBuildInput_tickBuild_original = NULL;
static int MouseBuildInput_tickBuild_injection(uchar *mouse_build_input, uchar *player, uint *build_action_intention_return) {
    // Call Original Method
    int ret = (*MouseBuildInput_tickBuild_original)(mouse_build_input, player, build_action_intention_return);
    if (!is_middle_click || ret == 0) return ret;
    is_middle_click = false;
    *build_action_intention_return = 0;

    // Get the tile
    uchar *minecraft = *(uchar **) (player + LocalPlayer_minecraft_property_offset);
    HitResult *hit_result = (HitResult *) (minecraft + Minecraft_hit_result_property_offset);
    if (hit_result->type != 0) return 0;
    uchar *level = *(uchar **) (minecraft + Minecraft_level_property_offset);
    int id = Level_getTile(level, hit_result->x, hit_result->y, hit_result->z);
    int data = Level_getData(level, hit_result->x, hit_result->y, hit_result->z);

    // Get inventory and held item
    uchar *inventory = *(uchar **) (player + Player_inventory_property_offset);
    int slot = *(int *) (inventory + Inventory_selectedSlot_property_offset);
    uchar *inventory_vtable = *(uchar **) inventory;
    FillingContainer_getItem_t FillingContainer_getItem = *(FillingContainer_getItem_t *) (inventory_vtable + FillingContainer_getItem_vtable_offset);
    ItemInstance *inventory_item = (*FillingContainer_getItem)(inventory, slot);

    // Search the inventory for it
    int new_slot = FillingContainer_getSlot2(inventory, id, data);
    if (new_slot != -1) {
        // It exists! Now swap to it.
        int unlinked = unlink_slot(inventory, new_slot);
        if (unlinked != -1) {
            // In the hotbar
            int *linked_slots = *(int **) (inventory + FillingContainer_linked_slots_property_offset);
            int old = linked_slots[slot];
            linked_slots[slot] = linked_slots[unlinked];
            linked_slots[unlinked] = old;
        } else {
            // Not in the hotbar
            FillingContainer_linkSlot(inventory, slot, new_slot, false);
        }
    } else if (Minecraft_isCreativeMode(minecraft)) {
        // Creative mode, just set the slot to it
        ItemInstance *new_item = (ItemInstance *) ::operator new(sizeof(ItemInstance));
        new_item->id = id;
        new_item->auxiliary = data;
        new_item->count = 64;
        if (inventory_item) {
            // Set the held slot
            *inventory_item = *new_item;
            delete new_item;
        } else {
            // Add
            int new_slot = FillingContainer_getFreeSlot(inventory);
            ItemInstance **items = *(ItemInstance***) (inventory + 0x18);
            items[new_slot] = new_item;

            // Swap
            int *linked_slots = *(int **) (inventory + FillingContainer_linked_slots_property_offset);
            linked_slots[slot] = new_slot;
            // Don't delete new_item, MCPI will
        }
    }

    return 0;
}

__attribute__((constructor)) static void init() {
    MouseBuildInput_tickBuild_original = *(MouseBuildInput_tickBuild_t *) MouseBuildInput_tickBuild_vtable_addr;
    patch_address(MouseBuildInput_tickBuild_vtable_addr, (void *) MouseBuildInput_tickBuild_injection);
}

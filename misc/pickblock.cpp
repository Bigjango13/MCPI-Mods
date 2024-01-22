#include <SDL/SDL.h>

#include <libreborn/libreborn.h>
#include <symbols/minecraft.h>
#include <mods/misc/misc.h>

#define FillingContainer_setItem ((FillingContainer_setItem_t) 0x91b00)
#define Inventory_add ((Inventory_add_t) 0x8d078)
#define FillingContainer_linkEmptySlot ((FillingContainer_linkEmptySlot_t) 0x92590)
#define FillingContainer_getFreeSlot ((FillingContainer_getFreeSlot_t) 0x91ffc)
#define Inventory_moveToSelectedSlot ((Inventory_moveToSelectedSlot_t) 0x8d148)
#define Level_getTile ((Level_getTile_t) 0xa3380)
#define Level_getData ((Level_getData_t) 0xa3324)

// Decompiled from libminecraftpe
int FillingContainer_getSlot2(FillingContainer *self, int id, int aux) {
    std::vector<ItemInstance *> items = self->items;
    for (int i = self->linked_slots_length; i < items.size(); i++) {
        if (items[i] == NULL) continue;
        if (items[i]->id == id && items[i]->auxiliary == aux) {
            return i;
        }
    }
    return -1;
}

// My code
static int unlink_slot(FillingContainer *fillingContainer, int linked_slot) {
    if (linked_slot == -1) return -1;
    int *linked_slots = fillingContainer->linked_slots;
    for (int i = 0; i < fillingContainer->linked_slots_length; i++) {
        if (linked_slots[i] == linked_slot) return i;
    }
    return -1;
}

static bool is_middle_click = false;
HOOK(SDL_PollEvent, int, (SDL_Event *event)) {
    ensure_SDL_PollEvent();
    int ret = real_SDL_PollEvent(event);
    if (event != NULL && event->type == SDL_MOUSEBUTTONDOWN) {
        is_middle_click = event->button.button == SDL_BUTTON_MIDDLE && event->button.state != SDL_RELEASED;
    }
    return ret;
}

static void swap_to_held_slot(Inventory *inventory, int new_slot) {
    int slot = inventory->selectedSlot;
    int unlinked = unlink_slot((FillingContainer *) inventory, new_slot);
    if (unlinked != -1) {
        // In the hotbar
        int *linked_slots = inventory->linked_slots;
        int old = linked_slots[slot];
        linked_slots[slot] = linked_slots[unlinked];
        linked_slots[unlinked] = old;
    } else {
        // Not in the hotbar
        FillingContainer_linkSlot((FillingContainer *) inventory, slot, new_slot, false);
    }
}

static MouseBuildInput_tickBuild_t MouseBuildInput_tickBuild_original = NULL;
static int MouseBuildInput_tickBuild_injection(MouseBuildInput *self, LocalPlayer *player, uint *build_action_intention_return) {
    // Call Original Method
    int ret = MouseBuildInput_tickBuild_original(self, (Player *) player, build_action_intention_return);
    if (!is_middle_click || ret == 0) return ret;
    is_middle_click = false;
    *build_action_intention_return = 0;

    // Get the tile
    Minecraft *minecraft = player->minecraft;
    HitResult hit_result = minecraft->hit_result;
    if (hit_result.type != 0) return 0;
    int id = Level_getTile(minecraft->level, hit_result.x, hit_result.y, hit_result.z);
    int data = Level_getData(minecraft->level, hit_result.x, hit_result.y, hit_result.z);

    // Search the inventory for it
    int new_slot = FillingContainer_getSlot2((FillingContainer *) player->inventory, id, data);
    if (new_slot != -1) {
        // It exists! Now swap to it.
        swap_to_held_slot(player->inventory, new_slot);
    } else if (Minecraft_isCreativeMode(minecraft)) {
        // Creative mode, just set the slot to it
        // Init
        ItemInstance *new_item = (ItemInstance *) ::operator new(sizeof(ItemInstance));
        new_item->id = id;
        new_item->auxiliary = data;
        new_item->count = 64;
        // Get the slot
        Inventory *inventory = player->inventory;
        ItemInstance *inventory_item = inventory->vtable->getItem(inventory, inventory->selectedSlot);
        if (inventory_item) {
            // Set the held slot
            *inventory_item = *new_item;
            delete new_item;
        } else {
            // Add
            new_slot = FillingContainer_getFreeSlot((FillingContainer *) inventory);
            Inventory_add(inventory, new_item);
            // Swap
            swap_to_held_slot(inventory, new_slot);
            // Don't delete new_item, MCPI will
        }
    }

    return 0;
}

__attribute__((constructor)) static void init() {
    MouseBuildInput_tickBuild_original = *(MouseBuildInput_tickBuild_t *) MouseBuildInput_tickBuild_vtable_addr;
    patch_address(MouseBuildInput_tickBuild_vtable_addr, (void *) MouseBuildInput_tickBuild_injection);
}

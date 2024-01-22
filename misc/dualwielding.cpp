#include <GLES/gl.h>
#include <SDL/SDL.h>

#include <libreborn/libreborn.h>
#include <symbols/minecraft.h>

// Config
// https://www.glfw.org/docs/latest/group__keys.html
#define SWAP_KEY 70
#define RIGHT_COLOR 0.0, 0.0, 1.0
#define LEFT_COLOR 1.0, 0.0, 0.0
#define DEFAULT_COLOR 1.0, 1.0, 1.0
#define SPAWN_EGGS_COMPAT
#define REBORN_BACKWARDS_COMPAT

// Symbols
typedef unsigned char uchar;

static uchar *FoodItem_vtable = (uchar *) 0x10e7b0;
static uchar *BowlFoodItem_vtable = (uchar *) 0x10e990;
static uchar *WeaponItem_vtable = (uchar *) 0x10ef30;
static uchar *DiggerItem_vtable = (uchar *) 0x10f668;
static uchar *HatchetItem_vtable = (uchar *) 0x10f728;
static uchar *PickaxeItem_vtable = (uchar *) 0x10f8a0;
static uchar *HoeItem_vtable = (uchar *) 0x10f7e8;
static uchar *ShovelItem_vtable = (uchar *) 0x10f960;
static uchar *EggItem_vtable = (uchar *) 0x10ed50;
static uchar *SnowballItem_vtable = (uchar *) 0x10ec10;

enum BuildActionIntention {
    BAI_Place    = 1 << 0,
    BAI_Break    = 1 << 1,
    BAI_Break2   = 1 << 2,
    BAI_Attack   = 1 << 3,
    BAI_Interact = 1 << 4,
};

typedef void (*ModelPart_translateTo_t)(ModelPart *self, const float f);
ModelPart_translateTo_t ModelPart_translateTo = (ModelPart_translateTo_t) 0x4145c;

// Fix reborn splitting symbols into two parts :(
// Most of this can be fixed and hopefully will be in a future reborn version
#ifdef REBORN_BACKWARDS_COMPAT
#define Inventory_getLinked ((Inventory_getLinked_t) 0x92230)
#undef Item_bow
#define Item_bow (*(Item **) 0x17ba78)
#undef Item_camera
#define Item_camera (*(Item **) 0x17bc14)
#define ItemInHandRenderer_render ((ItemInHandRenderer_render_t) 0x4bfcc)
#define HumanoidMobRenderer_render_non_virtual ((HumanoidMobRenderer_render_t) 0x62b8c)
#define Item_items_pointer ((Item **) 0x17b250)
#define LocalPlayer_vtable_base ((void *) 0x106230)
#define Minecraft_handleBuildAction ((Minecraft_handleBuildAction_t) 0x15920)
#define HumanoidMobRenderer_additionalRendering_non_virtual ((HumanoidMobRenderer_additionalRendering_t) 0x62c18)
#define GuiComponent_blit ((GuiComponent_blit_t) 0x282a4)
#define Inventory_getSelected ((Inventory_getSelected_t) 0x8d134)
#define Packet_constructor ((Packet_constructor_t) 0x6fc18)
#define PlayerEquipmentPacket_vtable_base ((PlayerEquipmentPacket_vtable *) 0x105e70)
#endif

// The actual code
static int left_slot = -1;
static bool swap_key_pressed = false, is_rendering_left = false, is_using_left = false;
HOOK(SDL_PollEvent, int, (SDL_Event *event)) {
    ensure_SDL_PollEvent();
    int ret = (*real_SDL_PollEvent)(event);
    if (ret != 1 || event == NULL) return ret;
    if (event->type == SDL_USEREVENT && event->user.data2 == SWAP_KEY && event->user.data1 == SDL_PRESSED) {
        swap_key_pressed = true;
    }
    return ret;
}

static ItemInstance *Inventory_getSelected_injection(Inventory *self);
static void send_equipment(LocalPlayer *self) {
    static ushort last_sent = -1;
    if (!self->minecraft->network_handler) return;
    PlayerEquipmentPacket *packet = (PlayerEquipmentPacket *) ::operator new(0x14);
    Packet_constructor((Packet *) packet);
    packet->vtable = PlayerEquipmentPacket_vtable_base;
    packet->entity_id = self->id;
    ItemInstance *item = Inventory_getSelected_injection(self->inventory);
    if (item) {
        packet->item_id = item->id;
        packet->item_aux = item->auxiliary;
    } else {
        packet->item_id = 0;
        packet->item_aux = 0;
    }
    // Don't spam with packets
    ushort new_sent = ((uchar) packet->item_id << 8) | (uchar) packet->item_aux;
    if (new_sent == last_sent) {
        // Already sent, no need to resend
        delete packet;
        return;
    }
    last_sent = new_sent;
    self->minecraft->rak_net_instance->vtable->send(self->minecraft->rak_net_instance, (Packet *) packet);
    delete packet;
}

// Handle clicking
#define CHECK_SWAP_VTABLE(vtable) if (iright_vtable == vtable) return false; \
                          if (ileft_vtable == vtable) return true;

bool check_if_should_swap(uint bai, ItemInstance *right_item, ItemInstance *left_item, HitResult hit_result) {
    // Don't bother if the slots are the same
    if (!left_item) return false;
    if (!right_item) return true;
    // This is pointer-wise comparison on purpose
    if (right_item == left_item) return false;
    Item *ileft = Item_items[left_item->id];
    uchar *ileft_vtable = *(uchar **) ileft;
    Item *iright = Item_items[right_item->id];
    uchar *iright_vtable = *(uchar **) iright;

    // Here is the order for selecting slots:
    // - Attack,    Weapon/Hatchet
    // - Break,     Pickaxe/Hatchet/Shovel
    // - Place,     Tile/TileItem
    // - Place,     Spawn Eggs
    // - Interact,  Bow
    // - Interact,  Food/BowlFood
    // - Interact,  Egg/Snowball
    // - Interact,  Hoe
    // - Interact,  Camera

    // Attack
    if ((bai & BAI_Attack) && hit_result.type == 1) {
        CHECK_SWAP_VTABLE(WeaponItem_vtable);
        CHECK_SWAP_VTABLE(HatchetItem_vtable);
    }
    // Break
    if (bai & (BAI_Break | BAI_Break2)) {
        CHECK_SWAP_VTABLE(PickaxeItem_vtable);
        CHECK_SWAP_VTABLE(HatchetItem_vtable);
        CHECK_SWAP_VTABLE(ShovelItem_vtable);
    }
    // Place
    if ((bai & BAI_Place) && hit_result.type == 0) {
        if (right_item->id < 256) return false;
        if (left_item->id < 256) return true;
#ifdef SPAWN_EGGS_COMPAT
        // Compat with my spawn eggs mod
        if (right_item->id == 383) return false;
        if (left_item->id == 383) return true;
#endif
    }
    // Interact
    if (bai & BAI_Interact) {
        if (iright->id == Item_bow->id) return false;
        if (ileft->id == Item_bow->id) return true;
        CHECK_SWAP_VTABLE(FoodItem_vtable);
        CHECK_SWAP_VTABLE(BowlFoodItem_vtable);
        CHECK_SWAP_VTABLE(EggItem_vtable);
        CHECK_SWAP_VTABLE(SnowballItem_vtable);
        CHECK_SWAP_VTABLE(HoeItem_vtable);
        if (iright->id == Item_camera->id) return false;
        if (ileft->id == Item_camera->id) return true;
    }
    return false;
}

static void Minecraft_handleBuildAction_injection(Minecraft *mc, uint *bai) {
    if (left_slot == -1 || !*bai) {
        // Left isn't even selected, no need for fancy logic
        Minecraft_handleBuildAction(mc, bai);
        return;
    }
    // Get the slots
    Inventory *inventory = mc->player->inventory;
    ItemInstance *right_item = inventory->vtable->getItem(inventory, inventory->selectedSlot);
    ItemInstance *left_item = inventory->vtable->getItem(inventory, left_slot);

    // Try to swap
    bool should_swap = check_if_should_swap(*bai, right_item, left_item, mc->hit_result);
    is_using_left = should_swap;
    int old = -1;
    if (should_swap) {
        // Swap
        old = inventory->linked_slots[inventory->selectedSlot];
        inventory->linked_slots[inventory->selectedSlot] = inventory->linked_slots[left_slot];
    }
    // Call original
    send_equipment(mc->player);
    Minecraft_handleBuildAction(mc, bai);
    if (should_swap) {
        // Swap back
        inventory->linked_slots[inventory->selectedSlot] = old;
    }
}

static Minecraft_handleMouseDown_t Minecraft_handleMouseDown_orignal = NULL;
static void Minecraft_handleMouseDown_injection(Minecraft *mc, int param_1, bool can_destroy) {
    if (left_slot == -1 || !can_destroy) {
        // Left isn't even selected, no need for fancy logic
        Minecraft_handleMouseDown_orignal(mc, param_1, can_destroy);
        return;
    }
    // Get the slots
    Inventory *inventory = mc->player->inventory;
    ItemInstance *right_item = inventory->vtable->getItem(inventory, inventory->selectedSlot);
    ItemInstance *left_item = inventory->vtable->getItem(inventory, left_slot);

    // Try to swap
    bool should_swap = check_if_should_swap(BAI_Break | BAI_Attack, right_item, left_item, mc->hit_result);
    is_using_left = should_swap;
    int old = -1;
    if (should_swap) {
        // Swap
        old = inventory->linked_slots[inventory->selectedSlot];
        inventory->linked_slots[inventory->selectedSlot] = inventory->linked_slots[left_slot];
    }
    // Call original
    send_equipment(mc->player);
    Minecraft_handleMouseDown_orignal(mc, param_1, can_destroy);
    if (should_swap) {
        // Swap back
        inventory->linked_slots[inventory->selectedSlot] = old;
    }
}

static ItemInstance *Inventory_getSelected_injection(Inventory *self) {
    // This is used in LocalPlayer_tick and Player_completeUsingItem for "using item" reasons
    // Return the left slot instead of the player is using it
    int slot = is_using_left ? left_slot : self->selectedSlot;
    return Inventory_getLinked(self, slot);
}

// Render selected slot in toolbar
static void GuiComponent_blit_injection(Gui *gui, int x, int y, int x2, int y2, int w, int h, int w2, int h2) {
    // This will be a lot prettier with the newer modding api
    Inventory *inventory = gui->minecraft->player->inventory;
    // Swapping
    if (swap_key_pressed) {
        if (left_slot == inventory->selectedSlot) {
            // Disable left
            left_slot = -1;
        } else {
            // Swap
            int selectedSlot = inventory->selectedSlot;
            if (left_slot != -1) {
                // Left already is held
                inventory->selectedSlot = left_slot;
            }
            left_slot = selectedSlot;
        }
        swap_key_pressed = false;
    }
    // Render the second selector
    if (left_slot != -1) {
        if (left_slot == inventory->selectedSlot) {
            // Color it mixed
            glColor4f(LEFT_COLOR, 1.0);
            GuiComponent_blit((GuiComponent *) gui, x, y, x2, y2, w, h, w2, h2);
            glColor4f(RIGHT_COLOR, 0.5);
            GuiComponent_blit((GuiComponent *) gui, x, y, x2, y2, w, h, w2, h2);
            return;
        } else {
            // Color it as left should be
            int real_x = x - inventory->selectedSlot * 20;
            glColor4f(LEFT_COLOR, 1.0);
            GuiComponent_blit((GuiComponent *) gui, left_slot * 20 + real_x, y, x2, y2, w, h, w2, h2);
        }
        // Color it as right should be when both hands are active
        glColor4f(RIGHT_COLOR, 1.0);
    } else {
        // Color it as it is when only one slot is selected
        glColor4f(DEFAULT_COLOR, 1.0);
    }
    // Call original
    GuiComponent_blit((GuiComponent *) gui, x, y, x2, y2, w, h, w2, h2);
}

// Render in hand
static int leftId = 0, leftIcon = 0;
static void ItemInHandRenderer_render_injection(ItemInHandRenderer *self, float param_1) {
    // Call original
    ItemInHandRenderer_render(self, param_1);
    // Get the left slot
    if (left_slot == -1) return;
    Minecraft *mc = self->mc;
    Inventory *inventory = mc->player->inventory;
    ItemInstance *left_item = inventory->vtable->getItem(inventory, left_slot);
    // Don't render the left hand
    if (left_item == NULL) return;
    // Don't render if it's the same hand
    if (left_slot == inventory->selectedSlot) return;
    // Render the left item
    ItemInstance *item = &self->item;
    if (!item) return;
    // Backup old data and set the new data
    ItemInstance old = *item;
    *item = *left_item;
    is_rendering_left = true;
    int oldIcon = self->lastIcon, oldId = self->lastId;
    self->lastId = leftId; self->lastIcon = leftIcon;
    // Render
    ItemInHandRenderer_render(self, param_1);
    // Restore the old data
    leftId = self->lastId; leftIcon = self->lastIcon;
    self->lastId = oldId; self->lastIcon = oldIcon;
    is_rendering_left = false;
    *item = old;
}

static void glTranslatef_injection_flipx(float x, float y, float z) {
    // Flip if it's the left
    if (is_rendering_left) {
        x = 0 - x;
    }
    glTranslatef(x, y, z);
}

static void glTranslatef_injection_flipx_and_block(float x, float y, float z) {
    // Block if it's the wrong hand, and flip if it's the left
    if (is_rendering_left != is_using_left) return;
    glTranslatef_injection_flipx(x, y, z);
}

static void glRotatef_injection_flip(float angle, float x, float y, float z) {
    // Flip if it's the left
    if (is_rendering_left) {
        angle = -angle;
    }
    glRotatef(angle, x, y, z);
}

static void glRotatef_injection_flip_and_block(float angle, float x, float y, float z) {
    // Stop the rotation for the wrong hand, and flip if it is the left
    if (is_rendering_left != is_using_left) return;
    if (is_rendering_left && angle != 0) {
        glRotatef(-45.0, 0.0, 1.0, 0.0);
    }
    glRotatef(angle, x, y, z);
}

static void glRotatef_injection_block(float angle, float x, float y, float z) {
    // Stop the rotation for the wrong hand
    if (is_rendering_left != is_using_left) angle = 0;
    glRotatef(angle, x, y, z);
}

static void glTranslatef_injection_fix_bow(float x, float y, float z) {
    // Move bows to a less incorrect position
    // TODO: Make it better
    if (is_rendering_left) {
        x = 0.0;
        y = 0.3;
    }
    glTranslatef(x, y, z);
}

// Render in third person
static ItemInstance *HumanoidMobRenderer_additionalRendering_Mob_getCarriedItem_injection(Mob *mob) {
    if (is_rendering_left) {
        // Get the item in the left hand instead
        Inventory *inventory = ((Player *) mob)->inventory;
        return inventory->vtable->getItem(inventory, left_slot);
    }
    return mob->vtable->getCarriedItem(mob);
}

static void HumanoidMobRenderer_render_injection(HumanoidMobRenderer *self, Entity *player, float param_2, float param_3, float param_4, float param_5, float param_6) {
    if ((void *) player->vtable == (void *) LocalPlayer_vtable_base && left_slot != -1) {
        Inventory *inventory = ((Player *) player)->inventory;
        ItemInstance *left_item = inventory->vtable->getItem(inventory, left_slot);
        self->model->angleLeftArm = left_item != NULL && left_slot != inventory->selectedSlot;
    }
    HumanoidMobRenderer_render_non_virtual(self, player, param_2, param_3, param_4, param_5, param_6);
    self->model->angleLeftArm = false;
}

static void HumanoidMobRenderer_additionalRendering_injection(HumanoidMobRenderer *self, Mob *mob) {
    HumanoidMobRenderer_additionalRendering_non_virtual(self, mob);
    // The the player is the user, and the left slot is full
    if ((void *) mob->vtable != (void *) LocalPlayer_vtable_base || left_slot == -1) return;
    Inventory *inventory = ((Player *) mob)->inventory;
    ItemInstance *left_item = inventory->vtable->getItem(inventory, left_slot);
    if (left_item == NULL || left_slot == inventory->selectedSlot) return;
    // Render the left hand
    is_rendering_left = true;
    HumanoidMobRenderer_additionalRendering_non_virtual(self, mob);
    is_rendering_left = false;
}

// ModelPart::translateTo doesn't normally take these args, but that doesn't stop me
static void ModelPart_translateTo_injection(HumanoidMobRenderer *renderer) {
    // Pick the correct hand
    if (is_rendering_left) {
        ModelPart_translateTo(&renderer->model->leftArm, 0.0625);
    } else {
        ModelPart_translateTo(&renderer->model->rightArm, 0.0625);
    }
}

__attribute__((constructor)) static void init() {
    // Handle clicking
    overwrite_calls((void*) Minecraft_handleBuildAction, (void*) Minecraft_handleBuildAction_injection);
    Minecraft_handleMouseDown_orignal = (Minecraft_handleMouseDown_t) extract_from_bl_instruction((uchar *) 0x16340);
    overwrite_calls((void*) Minecraft_handleMouseDown_orignal, (void*) Minecraft_handleMouseDown_injection);
    // Fixed item getters
    overwrite_calls((void *) Inventory_getSelected, (void *) Inventory_getSelected_injection);
    overwrite_call((void *) 0x4b7b0, (void *) Inventory_getSelected);
    // Render selected slot in toolbar
    overwrite_call((void*) 0x26df8, (void*) GuiComponent_blit_injection);
    // Render hand
    overwrite_calls((void *) ItemInHandRenderer_render, (void *) ItemInHandRenderer_render_injection);
    // Rendering stuff for first person and swinging (I hate rendering)
    overwrite_call((void *) 0x4c33c, (void *) glTranslatef_injection_flipx_and_block);
    overwrite_call((void *) 0x4c654, (void *) glTranslatef_injection_flipx_and_block);
    overwrite_call((void *) 0x4c35c, (void *) glTranslatef_injection_flipx);
    overwrite_call((void *) 0x4c384, (void *) glRotatef_injection_flip_and_block);
    overwrite_call((void *) 0x4c398, (void *) glRotatef_injection_block);
    overwrite_call((void *) 0x4c3b0, (void *) glRotatef_injection_block);
    overwrite_call((void *) 0x4c698, (void *) glRotatef_injection_block);
    overwrite_call((void *) 0x4c6ac, (void *) glRotatef_injection_block);
    // Render eating/drinking
    overwrite_call((void *) 0x4c218, (void *) glTranslatef_injection_flipx);
    overwrite_call((void *) 0x4c230, (void *) glRotatef_injection_flip);
    overwrite_call((void *) 0x4c260, (void *) glRotatef_injection_flip);
    // Render bow less incorrectly
    overwrite_call((void *) 0x4c434, (void *) glTranslatef_injection_fix_bow);
    // Render in third person
    // TODO: Swinging
    overwrite_call((void *) 0x62c30, (void *) HumanoidMobRenderer_additionalRendering_Mob_getCarriedItem_injection);
    uchar cpy_patch[4] = {0x05, 0x00, 0xa0, 0xe1}; // "cpy r0,r5"
    patch((void *) 0x62c4c, cpy_patch);
    uchar nop_patch[4] = {0x00, 0xf0, 0x20, 0xe3}; // "nop"
    patch((void *) 0x62c54, nop_patch);
    overwrite_call((void *) 0x62c58, (void *) ModelPart_translateTo_injection);
    overwrite_call((void *) 0x62c68, (void *) glTranslatef_injection_flipx);
    patch_address((void *) 0x107f38, (void *) HumanoidMobRenderer_additionalRendering_injection);
    patch_address((void *) 0x107f08, (void *) HumanoidMobRenderer_render_injection);
}

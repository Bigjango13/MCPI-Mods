#include <libreborn/libreborn.h>
#include <symbols/minecraft.h>
#include <mods/chat/chat.h>
#include <mods/misc/misc.h>

#include <SDL/SDL.h>

typedef ItemInstance *(*Player_getArmor_t)(unsigned char *player, int32_t slot);
static Player_getArmor_t Player_getArmor = (Player_getArmor_t) 0x8fda4;

typedef void (*Mob_causeFallDamage_t)(unsigned char *mob, float dist);
static Mob_causeFallDamage_t Mob_causeFallDamage = (Mob_causeFallDamage_t) 0x800a4;
static uint32_t Player_jump_property_offset = 0x38; // float

unsigned char **Material_iron = (unsigned char **) 0x17a7a8; // Material
unsigned char **iron_1_png = (unsigned char **) 0x137c58; // std::string

typedef unsigned char *(*ArmorItem_t)(unsigned char *ArmorItem, int id, unsigned char **mat, int param_3, int param_4);
static ArmorItem_t ArmorItem = (ArmorItem_t) 0x9362c;

unsigned char *player;
unsigned char *level;
static void mcpi_callback(unsigned char *minecraft){
    // Runs on every tick, sets the player and level vars.
    if (minecraft == NULL) return;
    player = *(unsigned char **) (minecraft + Minecraft_player_property_offset);
    level = *(unsigned char **) (minecraft + Minecraft_level_property_offset);
    if (player != NULL){
        // Uses texture changing to have the jetpack texture work.
        ItemInstance *item = Player_getArmor(player, 1);
        if (item == NULL) return;
        if (item->id == 404){
            *(std::string*) iron_1_png = "armor/jetpack_1.png";
        } else {
            *(std::string*) iron_1_png = "armor/iron_1.png";
        }
    }
}

float jetpackY = -255;
HOOK(SDL_PollEvent, int, (SDL_Event *event)) {
    // Poll Events
    ensure_SDL_PollEvent();
    int ret = (*real_SDL_PollEvent)(event);
    if (
        ret == 1
        // Make sure the event exists
        && event != NULL
        // The space key is pressed
        && event->type == SDL_KEYDOWN
        && event->key.keysym.sym == SDLK_SPACE
        // The player is in game
        && player != NULL
    ){
        // Get the item
        ItemInstance *item = Player_getArmor(player, 1);
        if (item != NULL){
            // Set the y for later adjustment of fall damage
            jetpackY = *(float *) (player + Entity_y_property_offset);
            if (item->id == 404){
                // Fire particle
                float x = *(float *) (player + Entity_x_property_offset);
                float z = *(float *) (player + Entity_z_property_offset);
                (*Level_addParticle)(level, "flame", x, jetpackY-1, z, 0.0, 0.0, 0.0, 0);
                // Make the player jump, even if in midair
                float jump_height = *(float *) (player + Player_jump_property_offset);
                if (jump_height <= 0){
                    // If falling slow the fall.
                    jump_height += .5;
                } else {
                    // If not falling, go up a little
                    jump_height += .1;
                }
                // Jump
                *(float *) (player + Player_jump_property_offset) = jump_height;
            }
        }
    }
    return ret;
}

void Mob_causeFallDamage_injection(unsigned char *mob, float dist){
    // The player is falling and has used the jetpack
    if (mob == player && jetpackY != -255){
        float newY = *(float *) (player + Entity_y_property_offset);
        // Adjust fall distance
        dist = (jetpackY - newY);
        jetpackY = -255;
    }
    // Call original method
    (*Mob_causeFallDamage)(mob, dist);
}

// Custom jetpack item
unsigned char *jetpack;
unsigned char *make_jetpack(){
    // Jetpack
    unsigned char *item = (unsigned char *) ::operator new(0x34); // ARMOR_SIZE
    ALLOC_CHECK(item);
    (*ArmorItem)(item, 148, Material_iron, 2, 1);
    // Set VTable
    unsigned char *vtable = *(unsigned char **) item;

    // Get Functions
    Item_setIcon_t Item_setIcon = *(Item_setIcon_t *) (vtable + Item_setIcon_vtable_offset);
    Item_setDescriptionId_t Item_setDescriptionId = *(Item_setDescriptionId_t *) (vtable + Item_setDescriptionId_vtable_offset);

    // Setup
    (*Item_setIcon)(item, 13, 4);
    (*Item_setDescriptionId)(item, "jetpack");
    *(int32_t *) (item + Item_is_stacked_by_data_property_offset) = 1;
    *(int32_t *) (item + Item_category_property_offset) = 2;
    *(int32_t *) (item + Item_max_damage_property_offset) = 250;
    *(int32_t *) (item + Item_max_stack_size_property_offset) = 1;

    return item;
}

static void Item_initItems_injection(__attribute__((unused)) unsigned char *null) {
    jetpack = make_jetpack();
}

// Add jetpack to creative inventory
static void Inventory_setupDefault_FillingContainer_addItem_call_injection(unsigned char *filling_container) {
    ItemInstance *jetpack_instance = new ItemInstance;
    ALLOC_CHECK(jetpack_instance);
    jetpack_instance->count = 255;
    jetpack_instance->auxiliary = 0;
    jetpack_instance->id = 404;
    (*FillingContainer_addItem)(filling_container, jetpack_instance);
}

// Crafting Recipes
static void Recipes_injection(unsigned char *recipes) {
    // 2 Lava bucket
    Recipes_Type type1 = {
        .item = 0,
        .tile = 0,
        .instance = {
            .count = 2,
            .id = 325,
            .auxiliary = 10
        },
        .letter = 'a'
    };
    // 1 Nether core
    Recipes_Type type2 = {
        .item = 0,
        .tile = 0,
        .instance = {
            .count = 1,
            .id = 247,
            .auxiliary = 0
        },
        .letter = 'b'
    };
    // 1 Iron block
    Recipes_Type type3 = {
        .item = 0,
        .tile = 0,
        .instance = {
            .count = 1,
            .id = 42,
            .auxiliary = 0
        },
        .letter = 'c'
    };
    // Jetpack
    ItemInstance result = {
        .count = 1,
        .id = 404,
        .auxiliary = 0
    };
    (*Recipes_addShapedRecipe_2)(recipes, result, " b ", "aca", {type1, type2, type3});
}

__attribute__((constructor)) static void init() {
    misc_run_on_update(mcpi_callback);
    overwrite_calls((void*) Mob_causeFallDamage, (void*) Mob_causeFallDamage_injection);
    misc_run_on_creative_inventory_setup(Inventory_setupDefault_FillingContainer_addItem_call_injection);
    misc_run_on_items_setup(Item_initItems_injection);
    misc_run_on_recipes_setup(Recipes_injection);
}

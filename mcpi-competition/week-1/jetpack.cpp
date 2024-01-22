#include <libreborn/libreborn.h>
#include <symbols/minecraft.h>
#include <mods/chat/chat.h>
#include <mods/misc/misc.h>

#include <SDL/SDL.h>

// Reborn symbol split fix
#define Player_getArmor ((Player_getArmor_t) 0x8fda4)
#define ArmorItem_constructor ((ArmorItem_constructor_t) 0x9362c)
#define ArmorMaterial_iron_pointer ((ArmorMaterial *) 0x17a7a8)
#define FillingContainer_addItem ((FillingContainer_addItem_t) 0x92aa0)
#define Recipes_addShapedRecipe_2 ((Recipes_addShapedRecipe_2_t) 0x9ca24)
#define Level_addParticle ((Level_addParticle_t) 0xa449c)

typedef void (*Mob_causeFallDamage_t)(Mob *mob, float dist);
static Mob_causeFallDamage_t Mob_causeFallDamage = (Mob_causeFallDamage_t) 0x800a4;

// Symbols
uchar **iron_1_png = (unsigned char **) 0x137c58; // std::string

// The code
Player *player;
Level *level;
static void mcpi_callback(Minecraft *minecraft){
    // Runs on every tick, sets the player and level vars.
    if (minecraft == NULL) return;
    player = (Player *) minecraft->player;
    level = minecraft->level;
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

bool is_jetpacking = false;
float jetpackY = -255;
HOOK(SDL_PollEvent, int, (SDL_Event *event)) {
    // Poll Events
    ensure_SDL_PollEvent();
    int ret = real_SDL_PollEvent(event);
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
            jetpackY = player->y;
            if (item->id == 404){
                // Fire particle
                float x = player->x;
                float z = player->z;
                std::string flame = "flame";
                Level_addParticle(level, &flame, x, jetpackY-1, z, 0.0, 0.0, 0.0, 0);
                // Make the player jump, even if in midair
                if (player->vel_y <= 0){
                    // If falling slow the fall.
                    player->vel_y += .5;
                } else {
                    // If not falling, go up a little
                    player->vel_y += .1;
                }
            }
        }
    }
    return ret;
}

void Mob_causeFallDamage_injection(Mob *mob, float dist){
    // The player is falling and has used the jetpack
    if (mob == (Mob *) player && jetpackY != -255){
        float newY = player->y;
        // Adjust fall distance
        dist = (jetpackY - newY);
        jetpackY = -255;
    }
    // Call original method
    Mob_causeFallDamage(mob, dist);
}

// Custom jetpack item
void make_jetpack(__attribute__((unused)) void *null){
    // Jetpack
    Item *item = (Item *) new ArmorItem;
    ALLOC_CHECK(item);
    ArmorItem_constructor((ArmorItem *) item, 148, ArmorMaterial_iron_pointer, 2, 1);

    // Setup
    item->vtable->setIcon(item, 13, 4);
    std::string name = "jetpack";
    item->vtable->setDescriptionId(item, &name);
    item->is_stacked_by_data = 1;
    item->category = 2;
    item->max_damage = 250;
    item->max_stack_size = 1;
}

// Add jetpack to creative inventory
static void Inventory_setupDefault_FillingContainer_addItem_call_injection(FillingContainer *filling_container) {
    ItemInstance *jetpack_instance = new ItemInstance;
    ALLOC_CHECK(jetpack_instance);
    jetpack_instance->count = 255;
    jetpack_instance->auxiliary = 0;
    jetpack_instance->id = 404;
    FillingContainer_addItem(filling_container, jetpack_instance);
}

// Crafting Recipes
static void Recipes_injection(Recipes *recipes) {
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
    std::string line1 = " b ";
    std::string line2 = "aca";
    std::vector<Recipes_Type> types = {type1, type2, type3};
    Recipes_addShapedRecipe_2(recipes, &result, &line1, &line2, &types);
}

__attribute__((constructor)) static void init() {
    misc_run_on_update(mcpi_callback);
    overwrite_calls((void*) Mob_causeFallDamage, (void*) Mob_causeFallDamage_injection);
    misc_run_on_creative_inventory_setup(Inventory_setupDefault_FillingContainer_addItem_call_injection);
    misc_run_on_items_setup(make_jetpack);
    misc_run_on_recipes_setup(Recipes_injection);
}

#include <libreborn/libreborn.h>
#include <symbols/minecraft.h>
#include <mods/misc/misc.h>

// The array of items, needed to patching
unsigned char **Item_items = (unsigned char **) 0x17b250;

// A macro to save space and make the code more readble
#define ADD_ITEM(name, item_id) \
    { \
        ItemInstance *name##_instance = new ItemInstance; \
        ALLOC_CHECK(name##_instance); \
        name##_instance->count = 255; \
        name##_instance->auxiliary = 0; \
        name##_instance->id = item_id; \
        (*FillingContainer_addItem)(filling_container, name##_instance); \
    }

static void Inventory_setupDefault_FillingContainer_addItem_call_injection(unsigned char *filling_container) {
    // Adds lots of items
    ADD_ITEM(bed, 26);
    ADD_ITEM(fire, 51);
    ADD_ITEM(sign, 63);
    ADD_ITEM(door, 64);
    ADD_ITEM(ironDoor, 71);
    ADD_ITEM(trapdoor, 96);
    ADD_ITEM(mushroomStew, 282);
    ADD_ITEM(steak, 364);
    ADD_ITEM(cookedChicken, 366);
    ADD_ITEM(porkCooked, 320);
    ADD_ITEM(apple, 260);
    ADD_ITEM(crops, 59);
    ADD_ITEM(farmland, 60);
    ADD_ITEM(activeFurnace, 62);
    ADD_ITEM(iron_door, 330);
    ADD_ITEM(activeRedstoneOre, 74);
    ADD_ITEM(pumkinStem, 105);
    ADD_ITEM(newGrass, 253);
    ADD_ITEM(doubleStoneSlab, 43);
    ADD_ITEM(arrow, 262);
    ADD_ITEM(coal, 263);
    ADD_ITEM(diamond, 264);
    ADD_ITEM(ironIngot, 265);
    ADD_ITEM(goldIngot, 266);
    ADD_ITEM(woodSword, 268);
    ADD_ITEM(woodShovel, 269);
    ADD_ITEM(woodPickaxe, 270);
    ADD_ITEM(woodAxe, 271);
    ADD_ITEM(stoneSword, 272);
    ADD_ITEM(stoneShovel, 273);
    ADD_ITEM(stonePickaxe, 274);
    ADD_ITEM(stoneAxe, 275);
    ADD_ITEM(shovelIron, 256);
    ADD_ITEM(ironPick, 257);
    ADD_ITEM(ironAxe, 258);
    ADD_ITEM(diamondSword, 276);
    ADD_ITEM(diamondShovel, 277);
    ADD_ITEM(diamondPickaxe, 278);
    ADD_ITEM(diamondAxe, 279);
    ADD_ITEM(magicWand, 280);
    ADD_ITEM(bowl, 281);
    ADD_ITEM(goldSword, 283);
    ADD_ITEM(goldShovel, 284);
    ADD_ITEM(goldPickaxe, 285);
    ADD_ITEM(goldAxe, 286);
    ADD_ITEM(string, 287);
    ADD_ITEM(feather, 288);
    ADD_ITEM(gunpowder, 289);
    ADD_ITEM(woodHoe, 290);
    ADD_ITEM(stoneHoe, 291);
    ADD_ITEM(flint1, 292);
    ADD_ITEM(diamondHoe, 293);
    ADD_ITEM(goldHoe, 294);
    ADD_ITEM(seeds, 295);
    ADD_ITEM(wheat, 296);
    ADD_ITEM(bread, 297);
    ADD_ITEM(diamondHelm, 310);
    ADD_ITEM(diamondChest, 311);
    ADD_ITEM(diamondLeg, 312);
    ADD_ITEM(diamondBoot, 313);
    ADD_ITEM(leatherCap, 298);
    ADD_ITEM(leatherShirt, 299);
    ADD_ITEM(leatherPants, 300);
    ADD_ITEM(leatherBoots, 301);
    ADD_ITEM(chainHelm, 302);
    ADD_ITEM(chainShirt, 303);
    ADD_ITEM(chainLegs, 304);
    ADD_ITEM(chainBoots, 305);
    ADD_ITEM(goldHelm, 314);
    ADD_ITEM(goldChest, 315);
    ADD_ITEM(goldLegs, 316);
    ADD_ITEM(goldBoots, 317);
    ADD_ITEM(ironHelm, 306);
    ADD_ITEM(ironChest, 307);
    ADD_ITEM(ironLegs, 308);
    ADD_ITEM(ironBoots, 309);
    ADD_ITEM(flint2, 318);
    ADD_ITEM(porkRaw, 319);
    ADD_ITEM(leather, 334);
    ADD_ITEM(clayBrick, 336);
    ADD_ITEM(clay, 337);
    ADD_ITEM(notepad, 339);
    ADD_ITEM(book, 340);
    ADD_ITEM(slimeball, 341);
    ADD_ITEM(compass, 345);
    ADD_ITEM(clock, 347);
    ADD_ITEM(glowDust, 348);
    ADD_ITEM(bone, 352);
    ADD_ITEM(sugar, 353);
    ADD_ITEM(melon, 360);
    ADD_ITEM(beefRaw, 363);
    ADD_ITEM(chickenRaw, 365);
}

static void Item_initItems_injection(__attribute__((unused)) unsigned char *obj) {
    // Patch all tiles that don't exist
    for (int i=1;i<=256;i++){
        // Checks if the item exists
        if (*(Tile_tiles + i) == NULL){
            // The tile doesn't exist, so patch it.
            *(Tile_tiles + i) = *(Tile_tiles + 248);
        }
    }
    // Now do the same to items
    for (int i=1;i<=500;i++){
        if (*(Item_items + i) == NULL){
            *(Item_items + i) = *(Item_items + 248);
        }
    }
}

int printf_injection(const char *format, ... ){
    return 0;
}

// Init
__attribute__((constructor)) static void init() {
    // Makes the function run every time the creative inventory is set up
    misc_run_on_creative_inventory_setup(Inventory_setupDefault_FillingContainer_addItem_call_injection);
    // Makes the function run after items setup.
    misc_run_on_items_setup(Item_initItems_injection);
    // Now to remove the annoying messages
    // "Item conflict id @ %d! Id already used\n"
    overwrite_calls(extract_from_bl_instruction((unsigned char*) 0x93694), (void*) printf_injection);
    overwrite_calls(extract_from_bl_instruction((unsigned char*) 0x93ff8), (void*) printf_injection);
    overwrite_calls(extract_from_bl_instruction((unsigned char*) 0x994e8), (void*) printf_injection);
    overwrite_calls(extract_from_bl_instruction((unsigned char*) 0x9a438), (void*) printf_injection);
    overwrite_calls(extract_from_bl_instruction((unsigned char*) 0x9a67c), (void*) printf_injection);
    overwrite_calls(extract_from_bl_instruction((unsigned char*) 0x9ae40), (void*) printf_injection);
    overwrite_calls(extract_from_bl_instruction((unsigned char*) 0x9b0f0), (void*) printf_injection);
    overwrite_calls(extract_from_bl_instruction((unsigned char*) 0xce404), (void*) printf_injection);
    // "Slot %d is already occupied by %p when adding %p\n"
    overwrite_calls(extract_from_bl_instruction((unsigned char*) 0xc3374), (void*) printf_injection);
    overwrite_calls(extract_from_bl_instruction((unsigned char*) 0xc3440), (void*) printf_injection);
}

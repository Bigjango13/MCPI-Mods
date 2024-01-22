#include <libreborn/libreborn.h>
#include <symbols/minecraft.h>
#include <mods/misc/misc.h>

// Reborn symbol split fix
#define Item_items_pointer (Item **) 0x17b250
#define FillingContainer_addItem ((FillingContainer_addItem_t) 0x92aa0)

// The code
#define ADD_ITEM(item_id) \
    { \
        ItemInstance *ii = new ItemInstance; \
        ALLOC_CHECK(ii); \
        ii->count = 255; \
        ii->auxiliary = 0; \
        ii->id = item_id; \
        (*FillingContainer_addItem)(filling_container, ii); \
    }

static void Inventory_setupDefault_FillingContainer_addItem_call_injection(FillingContainer *filling_container) {
    ADD_ITEM(26);
    ADD_ITEM(51);
    ADD_ITEM(63);
    ADD_ITEM(64);
    ADD_ITEM(71);
    ADD_ITEM(96);
    ADD_ITEM(282);
    ADD_ITEM(364);
    ADD_ITEM(366);
    ADD_ITEM(320);
    ADD_ITEM(260);
    ADD_ITEM(59);
    ADD_ITEM(60);
    ADD_ITEM(62);
    ADD_ITEM(330);
    ADD_ITEM(74);
    ADD_ITEM(105);
    ADD_ITEM(253);
    ADD_ITEM(43);
    ADD_ITEM(262);
    ADD_ITEM(263);
    ADD_ITEM(264);
    ADD_ITEM(265);
    ADD_ITEM(266);
    ADD_ITEM(268);
    ADD_ITEM(269);
    ADD_ITEM(270);
    ADD_ITEM(271);
    ADD_ITEM(272);
    ADD_ITEM(273);
    ADD_ITEM(274);
    ADD_ITEM(275);
    ADD_ITEM(256);
    ADD_ITEM(257);
    ADD_ITEM(258);
    ADD_ITEM(276);
    ADD_ITEM(277);
    ADD_ITEM(278);
    ADD_ITEM(279);
    ADD_ITEM(280);
    ADD_ITEM(281);
    ADD_ITEM(283);
    ADD_ITEM(284);
    ADD_ITEM(285);
    ADD_ITEM(286);
    ADD_ITEM(287);
    ADD_ITEM(288);
    ADD_ITEM(289);
    ADD_ITEM(290);
    ADD_ITEM(291);
    ADD_ITEM(292);
    ADD_ITEM(293);
    ADD_ITEM(294);
    ADD_ITEM(295);
    ADD_ITEM(296);
    ADD_ITEM(297);
    ADD_ITEM(310);
    ADD_ITEM(311);
    ADD_ITEM(312);
    ADD_ITEM(313);
    ADD_ITEM(298);
    ADD_ITEM(299);
    ADD_ITEM(300);
    ADD_ITEM(301);
    ADD_ITEM(302);
    ADD_ITEM(303);
    ADD_ITEM(304);
    ADD_ITEM(305);
    ADD_ITEM(314);
    ADD_ITEM(315);
    ADD_ITEM(316);
    ADD_ITEM(317);
    ADD_ITEM(306);
    ADD_ITEM(307);
    ADD_ITEM(308);
    ADD_ITEM(309);
    ADD_ITEM(318);
    ADD_ITEM(319);
    ADD_ITEM(334);
    ADD_ITEM(336);
    ADD_ITEM(337);
    ADD_ITEM(339);
    ADD_ITEM(340);
    ADD_ITEM(341);
    ADD_ITEM(345);
    ADD_ITEM(347);
    ADD_ITEM(348);
    ADD_ITEM(352);
    ADD_ITEM(353);
    ADD_ITEM(360);
    ADD_ITEM(363);
    ADD_ITEM(365);
    ADD_ITEM(405);
    ADD_ITEM(406);
}

#if 0
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
#endif

__attribute__((constructor)) static void init() {
    // Makes the function run every time the creative inventory is set up
    misc_run_on_creative_inventory_setup(Inventory_setupDefault_FillingContainer_addItem_call_injection);
#if 0
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
#endif
}

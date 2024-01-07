#include <libreborn/libreborn.h>
#include <symbols/minecraft.h>
#include <mods/chat/chat.h>
#include <mods/misc/misc.h>

#include "custom-items.h"

// Make the vector work.
std::vector<_CustomItem*> customItems = {};
static std::vector<_CustomItem*> &get_items(){
    static std::vector<_CustomItem*> customItems;
    return customItems;
}

// Combonation of Armor, Tool, and item.
_CustomItem::_CustomItem(std::string name, int id, int texture_x, int texture_y, int stacksize, int durability, unsigned char **tierMaterial, int armorTexture, ItemType itemType){
    descriptionId = name;
    itemId = id - 256;
    textureX = texture_x;
    textureY = texture_y;
    max_stack_size = stacksize;
    max_damage = durability;
    tierMat = tierMaterial;
    armorTex = armorTexture;
    type = itemType;

    get_items().push_back(this);
}

// Wrapper for items
_CustomItem *CustomItem(std::string name, int id, int texture_x, int texture_y, int stacksize){
    return new _CustomItem(name, id, texture_x, texture_y, stacksize, -1, NULL, 0, ItemType::item);
}

// Wrapper for tools
_CustomItem *CustomTool(std::string name, int id, int texture_x, int texture_y, int durability, unsigned char **tier, ToolType toolType){
    return new _CustomItem(name, id, texture_x, texture_y, 1, durability, tier, 0, (ItemType)((int)toolType + 4));
}

// Wrapper for armor
_CustomItem *CustomArmor(std::string name, int id, int texture_x, int texture_y, int durability, unsigned char **material, int armorTexture, ArmorType slot){
    return new _CustomItem(name, id, texture_x, texture_y, 1, durability, material, armorTexture, (ItemType)((int)slot));
}

// Makes all the items
unsigned char *make_item(_CustomItem customItem){
    int size = 0x13c;
    switch (customItem.type){
        case ItemType::helmet:
        case ItemType::chestplate:
        case ItemType::leggings:
        case ItemType::boots:
        case ItemType::hoe:
            size = 0x34;
            break;
        case ItemType::item:
            size = 0x38;
            break;
    }

    unsigned char *item = (unsigned char *) ::operator new(size);
    ALLOC_CHECK(item);
    // Inits the item
    switch (customItem.type){
        case ItemType::helmet:
        case ItemType::chestplate:
        case ItemType::leggings:
        case ItemType::boots:
            (*ArmorItem)(item, customItem.itemId, customItem.tierMat, customItem.armorTex, (int)customItem.type);
            break;
        case ItemType::hatchet:
            (*HatchetItem)(item, customItem.itemId, customItem.tierMat);
            break;
        case ItemType::hoe:
            (*HoeItem)(item, customItem.itemId, customItem.tierMat);
            break;
        case ItemType::pickaxe:
            (*PickaxeItem)(item, customItem.itemId, customItem.tierMat);
            break;
        case ItemType::shovel:
            (*ShovelItem)(item, customItem.itemId, customItem.tierMat);
            break;
        case ItemType::weapon:
            (*WeaponItem)(item, customItem.itemId, customItem.tierMat);
            break;
        case ItemType::item:
            (*Item)(item, customItem.itemId);
            break;
    }

    // Set VTable
    unsigned char *vtable = *(unsigned char **) item;

    // Get Functions
    Item_setIcon_t Item_setIcon = *(Item_setIcon_t *) (vtable + Item_setIcon_vtable_offset);
    Item_setDescriptionId_t Item_setDescriptionId = *(Item_setDescriptionId_t *) (vtable + Item_setDescriptionId_vtable_offset);

    // Setup
    (*Item_setIcon)(item, customItem.textureX, customItem.textureY);
    (*Item_setDescriptionId)(item, customItem.descriptionId);

    if (customItem.max_stack_size <= 0) {
        *(int32_t *) (item + Item_is_stacked_by_data_property_offset) = 1;
    } else {
        *(int32_t *) (item + Item_is_stacked_by_data_property_offset) = 0;
    }
    *(int32_t *) (item + Item_category_property_offset) = 2;
    *(int32_t *) (item + Item_max_damage_property_offset) = customItem.max_damage;
    *(int32_t *) (item + Item_max_stack_size_property_offset) = customItem.max_stack_size;

    // Logging
    INFO("Created %s (ID: %i)", customItem.descriptionId.c_str(), customItem.itemId + 256);
    return item;
}

static void Item_initItems_injection(__attribute__((unused)) unsigned char *null) {
    // Makes the items
    for (_CustomItem *item : get_items()) {
        item->item = make_item(*item);
    }
}

// Add items to creative inventory
static void Inventory_setupDefault_FillingContainer_addItem_call_injection(unsigned char *filling_container) {
    for (_CustomItem *item : get_items()) {
        ItemInstance *customItem_instance = new ItemInstance;
        ALLOC_CHECK(customItem_instance);
        customItem_instance->count = 255;
        customItem_instance->auxiliary = 0;
        customItem_instance->id = (item->itemId + 256);
        (*FillingContainer_addItem)(filling_container, customItem_instance);
    }
}

__attribute__((constructor)) static void init() {
    misc_run_on_items_setup(Item_initItems_injection);
    misc_run_on_creative_inventory_setup(Inventory_setupDefault_FillingContainer_addItem_call_injection);
}

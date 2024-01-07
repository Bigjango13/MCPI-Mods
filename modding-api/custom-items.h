#include <string>

// Materials
static unsigned char **Material_cloth   = (unsigned char **) 0x17a780; // Material
static unsigned char **Material_chain   = (unsigned char **) 0x17a794; // Material
static unsigned char **Material_gold    = (unsigned char **) 0x17a7bc; // Material
static unsigned char **Material_iron    = (unsigned char **) 0x17a7a8; // Material
static unsigned char **Material_diamond = (unsigned char **) 0x17a7d0; // Material

// Tier
static unsigned char **Tier_wood    = (unsigned char **) 0x17ba90; // Tier
static unsigned char **Tier_stone   = (unsigned char **) 0x17bab0; // Tier
static unsigned char **Tier_gold    = (unsigned char **) 0x17baf8; // Tier
static unsigned char **Tier_iron    = (unsigned char **) 0x17ba54; // Tier
static unsigned char **Tier_diamond = (unsigned char **) 0x17bad0; // Tier

// Item init functions
typedef unsigned char *(*ArmorItem_t)(unsigned char *ArmorItem, int id, unsigned char **mat, int param_3, int param_4);
static ArmorItem_t ArmorItem = (ArmorItem_t) 0x9362c;

typedef unsigned char *(*ToolItem_t)(unsigned char *item, int id, unsigned char **tier);
static ToolItem_t HatchetItem = (ToolItem_t) 0x9a60c; // Size 0x13c
static ToolItem_t HoeItem     = (ToolItem_t) 0x9add8; // Size 0x34
static ToolItem_t PickaxeItem = (ToolItem_t) 0x9b080; // Size 0x13c
static ToolItem_t ShovelItem  = (ToolItem_t) 0x9b388; // Size 0x13c
static ToolItem_t WeaponItem  = (ToolItem_t) 0x9988c; // Size 0x13c

// Item, tool and armor type
enum class ItemType{
    helmet,
    chestplate,
    leggings,
    boots,
    hatchet,
    hoe,
    pickaxe,
    shovel,
    weapon,
    item
};

enum class ArmorType{
    helmet,
    chestplate,
    leggings,
    boots
};

enum class ToolType{
    hatchet,
    hoe,
    pickaxe,
    shovel,
    weapon
};

class _CustomItem{
public:
    unsigned char *item;

    _CustomItem(std::string name, int id, int texture_x, int texture_y, int stacksize, int durability, unsigned char **tierMaterial, int armorTexture, ItemType type);
    std::string descriptionId;

    int itemId;
    int textureX;
    int textureY;
    int max_stack_size;
    int max_damage;
    unsigned char **tierMat;
    int armorTex;
    ItemType type;
};

_CustomItem *CustomItem(std::string name, int id, int texture_x, int texture_y, int stacksize);
_CustomItem *CustomTool(std::string name, int id, int texture_x, int texture_y, int durability, unsigned char **tier, ToolType toolType);
_CustomItem *CustomArmor(std::string name, int id, int texture_x, int texture_y, int durability, unsigned char **material, int armorTexture, ArmorType slot);

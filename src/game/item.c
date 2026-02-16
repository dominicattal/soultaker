#include "../game.h"

Item item_create(ItemEnum type, void* item)
{
    Item ret;
    ret.type = type;
    ret.generic_ptr = item;
    ret.equipped = false;
    return ret;
}

void item_destroy(Item* item)
{
    switch (item->type) {
        case ITEM_NONE:
            break;
        case ITEM_WEAPON:
            weapon_destroy(item->weapon);
            break;
        default:
            log_write(CRITICAL, "Invalid item destroy");
            break;
    }
    item->type = ITEM_NONE;
}

void item_swap(Item* item1, Item* item2)
{
    void* tmp = item1->generic_ptr;
    item1->generic_ptr = item2->generic_ptr;
    item2->generic_ptr = tmp;

    item1->type ^= item2->type;
    item2->type ^= item1->type;
    item1->type ^= item2->type;
}

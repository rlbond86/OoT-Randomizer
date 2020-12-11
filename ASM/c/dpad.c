#include "gfx.h"
#include "dpad.h"

extern uint8_t CFG_DISPLAY_DPAD;

//unknown 00 is a pointer to some vector transformation when the sound is tied to an actor. actor + 0x3E, when not tied to an actor (map), always 80104394
//unknown 01 is always 4 in my testing
//unknown 02 is a pointer to some kind of audio configuration Always 801043A0 in my testing
//unknown 03 is always a3 in my testing
//unknown 04 is always a3 + 0x08 in my testing (801043A8)
typedef void(*playsfx_t)(uint16_t sfx, z64_xyzf_t *unk_00_, int8_t unk_01_ , float *unk_02_, float *unk_03_, float *unk_04_);
typedef void(*usebutton_t)(z64_game_t *game, z64_link_t *link, uint8_t item, uint8_t button);

#define z64_playsfx   ((playsfx_t)      0x800C806C)
#define z64_usebutton ((usebutton_t)    0x8038C9A0)

void cycle_items();

typedef union
{
    // 0 = none, otherwise 1 + slot
    struct {
        uint16_t left  : 5;
        uint16_t down  : 5;
        uint16_t right : 5;
    };
    uint16_t bits;
} packed_slot_array_t;

static packed_slot_array_t item_slots[2] = { {.bits = 0}, {.bits = 0} };


void handle_dpad() {

    pad_t pad_pressed = z64_game.common.input[0].pad_pressed;

    if (CAN_USE_DPAD && DISPLAY_DPAD){
        if(z64_file.link_age == 0) {
            if (pad_pressed.dl && z64_file.iron_boots) {
                if (z64_file.equip_boots == 2) z64_file.equip_boots = 1;
                else z64_file.equip_boots = 2;
                z64_UpdateEquipment(&z64_game, &z64_link);
                z64_playsfx(0x835, (z64_xyzf_t*)0x80104394, 0x04, (float*)0x801043A0, (float*)0x801043A0, (float*)0x801043A8);
            }

            if (pad_pressed.dr && z64_file.hover_boots) {
                if (z64_file.equip_boots == 3) z64_file.equip_boots = 1;
                else z64_file.equip_boots = 3;
                z64_UpdateEquipment(&z64_game, &z64_link);
                z64_playsfx(0x835, (z64_xyzf_t*)0x80104394, 0x04, (float*)0x801043A0, (float*)0x801043A0, (float*)0x801043A8);
            }
        }
        if (pad_pressed.dd && CAN_USE_OCARINA){
            z64_usebutton(&z64_game,&z64_link,z64_file.items[0x07], 2);
        }
        if (pad_pressed.du) {
            cycle_items();
            z64_UpdateItemButton(&z64_game, 1);
            z64_UpdateItemButton(&z64_game, 2);
            z64_UpdateItemButton(&z64_game, 3);
        }
    }
}

void draw_dpad() {
    z64_disp_buf_t *db = &(z64_ctxt.gfx->overlay);
    if (DISPLAY_DPAD && CFG_DISPLAY_DPAD) {
        gSPDisplayList(db->p++, &setup_db);
        gDPPipeSync(db->p++);
        gDPSetCombineMode(db->p++, G_CC_MODULATEIA_PRIM, G_CC_MODULATEIA_PRIM);
        uint16_t alpha = z64_game.hud_alpha_channels.minimap;
        
        if (alpha == 0xAA) alpha = 0xFF;
        gDPSetPrimColor(db->p++, 0, 0, 0xFF, 0xFF, 0xFF, alpha);
        sprite_load(db, &dpad_sprite, 0, 1);
        sprite_draw(db, &dpad_sprite, 0, 271, 64, 16, 16);

        if (alpha == 0xFF && !CAN_USE_DPAD)
            gDPSetPrimColor(db->p++, 0, 0, 0xFF, 0xFF, 0xFF, 0x46);

        if (z64_file.iron_boots && z64_file.link_age==0) {
            sprite_load(db, &items_sprite, 69, 1);
            if (z64_file.equip_boots == 2) {
                sprite_draw(db, &items_sprite, 0, 258, 64, 16, 16);
            }
            else {
                sprite_draw(db, &items_sprite, 0, 260, 66, 12, 12);
            }
        }

        if (z64_file.hover_boots && z64_file.link_age == 0) {
            sprite_load(db, &items_sprite, 70, 1);
            if (z64_file.equip_boots == 3) {
                sprite_draw(db, &items_sprite, 0, 283, 64, 16, 16);
            }
            else {
                sprite_draw(db, &items_sprite, 0, 285, 66, 12, 12);
            }
        }
        if (z64_file.items[0x07] == 0x07 || z64_file.items[0x07] == 0x08){
            if(alpha==0xFF && !CAN_USE_OCARINA) gDPSetPrimColor(db->p++, 0, 0, 0xFF, 0xFF, 0xFF, 0x46);
            sprite_load(db, &items_sprite, z64_file.items[0x07], 1);
            sprite_draw(db, &items_sprite, 0, 273, 77, 12,12);
        }

        gDPPipeSync(db->p++);
    }
}

#define ADULT_ONLY  0x1
#define CHILD_ONLY  0x2
#define BOTH_AGES   0x3
#define NEVER_EQUIP 0x0

static const uint8_t ITEM_AGE_USABILITY[25] = {
    NEVER_EQUIP,
    CHILD_ONLY, BOTH_AGES,  BOTH_AGES, ADULT_ONLY, ADULT_ONLY, BOTH_AGES,
    CHILD_ONLY, BOTH_AGES,  BOTH_AGES, ADULT_ONLY, ADULT_ONLY, BOTH_AGES,
    CHILD_ONLY, BOTH_AGES, CHILD_ONLY, ADULT_ONLY, ADULT_ONLY, BOTH_AGES,
     BOTH_AGES, BOTH_AGES,  BOTH_AGES,  BOTH_AGES, ADULT_ONLY, CHILD_ONLY
};

int32_t from_single_packed_slot(int32_t unpacked_slot, int8_t* item) {
    uint8_t age_code = 1 + (uint8_t)z64_file.link_age; // 0x1 = adult, 0x2 = child
    if (unpacked_slot != 0 && (ITEM_AGE_USABILITY[unpacked_slot] & age_code) != 0x0) {
        --unpacked_slot;
        *item = z64_file.items[unpacked_slot];
        return unpacked_slot;
    }
    *item = -1;
    return 0;
}

void from_packed_slot_array(packed_slot_array_t slots, int8_t* button_items, int8_t* c_slots) {
    ++button_items; // Skip B button
    int8_t unpacked_slots[3] = {slots.left, slots.down, slots.right};
    int8_t* unpacked_slots_ptr = unpacked_slots;
    
    for (int button = 0; button < 3; ++button) {
        *c_slots++ = (int8_t)from_single_packed_slot(*unpacked_slots_ptr++, button_items++);
    }
}

uint32_t to_single_packed_slot(int8_t button_item, int32_t c_slot) {
    if (button_item == -1) {
        return 0;
    }
    return c_slot + 1;
}

packed_slot_array_t to_packed_slot_array(const int8_t* button_items, const int8_t* c_slots) {
    ++button_items; // Skip B button
    packed_slot_array_t packed;
    packed.left  = (uint8_t)to_single_packed_slot(*button_items++, *c_slots++);
    packed.down  = (uint8_t)to_single_packed_slot(*button_items++, *c_slots++);
    packed.right = (uint8_t)to_single_packed_slot(*button_items++, *c_slots++);
    return packed;
}

void cycle_items() {
    int age = z64_file.link_age;
    packed_slot_array_t old = to_packed_slot_array(z64_file.button_items, z64_file.c_button_slots);
    from_packed_slot_array(item_slots[z64_file.link_age], z64_file.button_items, z64_file.c_button_slots);
    item_slots[z64_file.link_age] = old;
}


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

#define NULL_SLOT -1
typedef struct {
    int8_t slot[3];
} c_button_configuration;

static c_button_configuration backup_configurations[2] = { 
    {NULL_SLOT, NULL_SLOT, NULL_SLOT}, 
    {NULL_SLOT, NULL_SLOT, NULL_SLOT} 
};


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
            z64_playsfx(0x835, (z64_xyzf_t*)0x80104394, 0x04, (float*)0x801043A0, (float*)0x801043A0, (float*)0x801043A8);
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

static const uint8_t ITEM_AGE_USABILITY[24] = {
    CHILD_ONLY, BOTH_AGES,  BOTH_AGES, ADULT_ONLY, ADULT_ONLY, BOTH_AGES,
    CHILD_ONLY, BOTH_AGES,  BOTH_AGES, ADULT_ONLY, ADULT_ONLY, BOTH_AGES,
    CHILD_ONLY, BOTH_AGES, CHILD_ONLY, ADULT_ONLY, ADULT_ONLY, BOTH_AGES,
     BOTH_AGES, BOTH_AGES,  BOTH_AGES,  BOTH_AGES, ADULT_ONLY, CHILD_ONLY
};

int is_valid_slot(int8_t slot) {
    if (slot < 0) {
        return 0;
    }
    else if ((ITEM_AGE_USABILITY[slot] & (z64_file.link_age + 1)) == 0x0) {
        return 0;
    }
    int8_t item = z64_file.items[slot];
    if (item == Z64_ITEM_NULL || item == Z64_ITEM_SOLD_OUT) {
        return 0;
    }
    return 1;
}

void clear_invalid_slot(int8_t* slot) {
    if (!is_valid_slot(*slot)) {
        *slot = Z64_ITEM_NULL;
    }
}

int is_slot_unique(int8_t slot, const int8_t* all_slots) {
    return all_slots[0] != slot && all_slots[1] != slot && all_slots[2] != slot;
}

int8_t populate_empty_slot(int8_t next_slot, int8_t* slot, const int8_t* all_slots) {
    if (*slot != NULL_SLOT) {
        return next_slot;
    }
    
    while (next_slot < 24) {
        if (is_valid_slot(next_slot)) {
            if (is_slot_unique(next_slot, all_slots)) {
                *slot = next_slot;
                return next_slot + 1;
            }
        }
        ++next_slot;
    }
    
    return next_slot;
}

void rearrange_populated_to_match(const uint8_t* required, int idx, int8_t* all_slots) {
    // Move items if some slots went from populated to empty
    // Avoids some graphical issues
    if (all_slots[idx] == NULL_SLOT) {
        // Attempt to find another item to move here
        for (int j = 0; j < 3; ++j) {
            if (!required[j] && all_slots[j] != NULL_SLOT) {
                all_slots[idx] = all_slots[j];
                all_slots[j] = NULL_SLOT;
                break;
            }
        }
    }
}

void populate_empty_c_buttons(const c_button_configuration* old_buttons, c_button_configuration* new_buttons) {
    uint8_t old_populated[3];

    // Get which buttons are populated and remove invalid population
    for (int i = 0; i < 3; ++i) {
        old_populated[i] = is_valid_slot(old_buttons->slot[i]);
        clear_invalid_slot(&new_buttons->slot[i]);
    }
    
    // Remove duplicates
    for (int i = 0; i < 3; ++i) {
        for (int j = i+1; j < 3; ++j) {
            if (new_buttons->slot[i] == new_buttons->slot[j]) {
                new_buttons->slot[j] = NULL_SLOT;
            }
        }
    }

    // Attempt to populate empty slots
    int8_t slot = 0;
    for (int i = 0; i < 3; ++i) {
        slot = populate_empty_slot(slot, &new_buttons->slot[i], new_buttons->slot);
    }

    // Prefer slots that were populated before (addresses some graphical issues)
    for (int i = 0; i < 3; ++i) {
        if (old_populated[i]) {
            rearrange_populated_to_match(old_populated, i, new_buttons->slot);
        }
    }
}

void get_c_buttons_from_file(const int8_t* file_button_items, const int8_t* file_c_button_slots, c_button_configuration* config) {
    ++file_button_items; // Skip B button
    for (int i = 0; i < 3; ++i) {
        config->slot[i] = file_button_items[i] != Z64_ITEM_NULL ? file_c_button_slots[i] : NULL_SLOT;
    }
}

void put_c_buttons_to_file(const c_button_configuration* config, int8_t* file_button_items, int8_t* file_c_button_slots) {
    ++file_button_items; // Skip B button
    for (int i = 0; i < 3; ++i) {
        if (is_valid_slot(config->slot[i])) {
            file_c_button_slots[i] = config->slot[i];
            file_button_items[i] = z64_file.items[file_c_button_slots[i]];
        }
        else {
            file_c_button_slots[i] = Z64_SLOT_NUT; // Usable by both adult and child
            file_button_items[i] = Z64_ITEM_NULL;
        }
    }
}

void cycle_items() {
    int age = z64_file.link_age;
    c_button_configuration old;
    c_button_configuration* new = &backup_configurations[z64_file.link_age];
    get_c_buttons_from_file(z64_file.button_items, z64_file.c_button_slots, &old);
    populate_empty_c_buttons(&old, new);
    put_c_buttons_to_file(new, z64_file.button_items, z64_file.c_button_slots);
    *new = old;
}


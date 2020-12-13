#include "gfx.h"
#include "dpad.h"
#include "second_inventory.h"

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

void handle_dpad() {

    pad_t pad_pressed = z64_game.common.input[0].pad_pressed;

    static int8_t update_c_buttons_next_frame = 0;
    if (update_c_buttons_next_frame) {
        update_c_buttons_next_frame = 0;
        for (int i = 0; i < 3; ++i) {
            z64_UpdateItemButton(&z64_game, i+1);
        }
    }

    if (CAN_USE_DPAD){
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
            cycle_c_button_items();
            for (int i = 0; i < 3; ++i) {
                z64_file.button_usable[i+1] = -1; // Prevent using prohibited item on switch frame
            }
            update_c_buttons_next_frame = 1; // Requires a 1-frame delay to avoid graphical issues
            
            // If not in menu, dim buttons initially
            if (z64_game.pause_ctxt.state == 0) {
                z64_game.hud_alpha_channels.cl_button = 0x46;
                z64_game.hud_alpha_channels.cd_button = 0x46;
                z64_game.hud_alpha_channels.cr_button = 0x46;
                z64_playsfx(0x835, (z64_xyzf_t*)0x80104394, 0x04, (float*)0x801043A0, (float*)0x801043A0, (float*)0x801043A8);
            }
        }
    }
}

void draw_dpad() {
    z64_disp_buf_t *db = &(z64_ctxt.gfx->overlay);
    if (CFG_DISPLAY_DPAD) {
        gSPDisplayList(db->p++, &setup_db);
        gDPPipeSync(db->p++);
        gDPSetCombineMode(db->p++, G_CC_MODULATEIA_PRIM, G_CC_MODULATEIA_PRIM);
        uint16_t alpha = z64_game.hud_alpha_channels.minimap;
        
        if (alpha == 0xAA) alpha = 0xFF;
        gDPSetPrimColor(db->p++, 0, 0, 0xFF, 0xFF, 0xFF, alpha);
        sprite_load(db, &dpad_sprite, 0, 1);
        sprite_draw(db, &dpad_sprite, 0, 271, 64, 16, 16);

        uint16_t modified_alpha = alpha;
        if (alpha == 0xFF && !CAN_USE_DPAD) {
            modified_alpha = 0x46;
            gDPSetPrimColor(db->p++, 0, 0, 0xFF, 0xFF, 0xFF, modified_alpha);
        }

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

        colorRGB16_t c_color = opposite_c_button_color();
        gDPSetPrimColor(db->p++, 0, 0, c_color.r, c_color.g, c_color.b, modified_alpha / 2);
        sprite_load(db, &c_button_sprite, 0, 1);
        sprite_draw(db, &c_button_sprite, 0, 275, 56, 8, 8);

        gDPPipeSync(db->p++);
    }
}



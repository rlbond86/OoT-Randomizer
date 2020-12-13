#include "second_inventory.h"
#include "z64.h"

extern colorRGB16_t CFG_C_BUTTON_COLOR;

#define NULL_SLOT -1
typedef struct {
    int8_t slot[3];
} c_button_configuration;

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

void clear_invalid_slots(c_button_configuration* config) {
    for (int i = 0; i < 3; ++i) {
        if (!is_valid_slot(config->slot[i])) {
            config->slot[i] = Z64_ITEM_NULL;
        }
    }
}

void remove_duplicate_slots(c_button_configuration* config) {
    if (config->slot[0] == config->slot[1]) {config->slot[1] = NULL_SLOT;}
    if (config->slot[1] == config->slot[2]) {config->slot[2] = NULL_SLOT;}
    if (config->slot[0] == config->slot[2]) {config->slot[2] = NULL_SLOT;}
}

int is_slot_in_config(int8_t slot_val, const c_button_configuration* config) {
    return     config->slot[0] == slot_val
            || config->slot[1] == slot_val
            || config->slot[2] == slot_val;
}

int get_empty_slot_index(const c_button_configuration* config) {
    for (int i = 0; i < 3; ++i) {
        if (config->slot[i] == NULL_SLOT) {
            return i;
        }
    }
    return -1;
}

void validate_c_buttons(c_button_configuration* new_buttons) {
    clear_invalid_slots(new_buttons);
    remove_duplicate_slots(new_buttons);
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
            file_c_button_slots[i] = NULL_SLOT; // Usable by both adult and child
            file_button_items[i] = Z64_ITEM_NULL;
        }
    }
}

static c_button_configuration backup_configurations[2] = {
    {NULL_SLOT, NULL_SLOT, NULL_SLOT},
    {NULL_SLOT, NULL_SLOT, NULL_SLOT}
};

static int8_t config_index[2] = {0, 0};

void cycle_c_button_items() {
    int age = z64_file.link_age;
    c_button_configuration old;
    c_button_configuration* new = &backup_configurations[age];
    get_c_buttons_from_file(z64_file.button_items, z64_file.c_button_slots, &old);
    validate_c_buttons(new);
    put_c_buttons_to_file(new, z64_file.button_items, z64_file.c_button_slots);
    *new = old;
    config_index[age] = !config_index[age];
}

#define SWAP_VALUE(T, x, y) do { T temp = x; x = y; y = temp; } while(0)

static void rotate_color(colorRGB16_t* clr) {
    if (clr->r == clr->g && clr->g == clr->b) return;

    uint16_t* largest = &clr->r;
    uint16_t* middle = &clr->g;
    uint16_t* smallest = &clr->b;

    // Sort
    if (*largest < *middle) {
        SWAP_VALUE(uint16_t*, largest, middle);
    }
    if (*middle < *smallest) {
        SWAP_VALUE(uint16_t*, middle, smallest);
    }
    if (*largest < *middle) {
        SWAP_VALUE(uint16_t*, largest, middle);
    }

    *middle = *largest - *middle + *smallest;
    SWAP_VALUE(uint16_t, *largest, *smallest);
}

static colorRGB16_t get_c_color(int idx) {
    colorRGB16_t clr = CFG_C_BUTTON_COLOR;
    if (idx) {
        rotate_color(&clr);
    }
    return clr;
}

colorRGB16_t c_button_color() {
    return get_c_color(config_index[z64_file.link_age]);
}

colorRGB16_t opposite_c_button_color() {
    return get_c_color(!config_index[z64_file.link_age]);
}

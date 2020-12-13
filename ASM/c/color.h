#ifndef COLOR_H
#define COLOR_H

#include <stdint.h>

typedef struct
{
    uint16_t r;
    uint16_t g;
    uint16_t b;
} colorRGB16_t;


typedef struct
{
    uint8_t r;
    uint8_t g;
    uint8_t b;
} colorRGB8_t;

typedef struct
{
    union
    {
        struct
        {
            uint8_t r;
            uint8_t g;
            uint8_t b;
        };
        colorRGB8_t color;
    };
    uint8_t a;
} colorRGBA8_t;

typedef struct
{
    union
    {
        struct
        {
            uint16_t r;
            uint16_t g;
            uint16_t b;
        };
        colorRGB16_t color;
    };
    uint16_t a;
} colorRGBA16_t;

typedef struct
{
    uint16_t r1;
    uint16_t r2;
    uint16_t g1;
    uint16_t g2;
    uint16_t b1;
    uint16_t b2;
} colorRGB16_2_t;

typedef struct
{
    float r;
    float g;
    float b;
    float a;
} colorRGBAf_t;

#endif

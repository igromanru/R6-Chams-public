#pragma once

struct RGBA
{
    unsigned char R;
    unsigned char G;
    unsigned char B;
    unsigned char A;

    RGBA(unsigned char r = 0, unsigned char g = 0, unsigned char b = 0, unsigned char a = 255) :
        R(r), G(g), B(b), A(a) {}
};

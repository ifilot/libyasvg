/*
 * color.cpp
 *
 * Authors: Ivo Filot
 *          Bart Zijlstra
 *          Evert van Noort
 *          Emiel Hensen
 *
 * (C) Copyright 2017 Inorganic Materials Chemistry
 *
 * This is a legal licensing agreement (Agreement) between You (an individual
 * or single legal entity) and Inorganic Materials Chemistry (IMC)
 * governing the in-house use of the MKMCXX software product (Software).
 * By downloading, installing, or using Software, You agree to be bound by
 * the license terms as given on http://www.mkmcxx.nl/license.
 *
 */

#include "color.h"

/**************************************************************************
 *                                                                        *
 *   Color Object                                                         *
 *                                                                        *
 **************************************************************************/

/**
 *
 * Color constructor
 *
 */
Color::Color() {
  this->r = 0;
  this->g = 0;
  this->b = 0;
}

Color::Color(unsigned int _r, unsigned int _g, unsigned int _b) {
    this->r = _r;
    this->g = _g;
    this->b = _b;

    this->hexcode = (boost::format("%X%X%X") % this->r % this->g % this->b).str();
}

Color::Color(const std::string& _hex) {
    this->hexcode = _hex;
    this->r = this->hex2int(_hex.substr(0,2));
    this->g = this->hex2int(_hex.substr(2,2));
    this->b = this->hex2int(_hex.substr(4,2));
}

/*
 * @fn lighten
 *
 * @brief lighten a color
 *
 * @param _value lightening factor 0-1
 *
 * @return lightened color
 */
Color Color::lighten(double _value) {
    unsigned int nr = this->r * (1 - _value) + _value * 255.0f;
    unsigned int ng = this->g * (1 - _value) + _value * 255.0f;
    unsigned int nb = this->b * (1 - _value) + _value * 255.0f;

    return Color(nr, ng, nb);
}

/*
 * @fn darken
 *
 * @brief darken a color
 *
 * @param _value darkening factor 0-1
 *
 * @return darkened color
 */
Color Color::darken(double _value) {
    unsigned int nr = this->r * (1 - _value) + _value * 0.0f;
    unsigned int ng = this->g * (1 - _value) + _value * 0.0f;
    unsigned int nb = this->b * (1 - _value) + _value * 0.0f;

    return Color(nr, ng, nb);
}

/**
 * @fn hex2int
 *
 * @brief Convert a single hexadecimal value to an integer
 *
 * @param _hex hexadecimal number in range 00-FF
 *
 * @return unsigned integer in range 0-255
 */
unsigned int Color::hex2int(const std::string &_hex) {
    if(_hex.length() != 2) {
        std::cerr << _hex << std::endl;
        throw std::runtime_error("Invalid hex pattern received.");
    }
    char* offset;
    return strtol(_hex.c_str(), &offset, 16);
}
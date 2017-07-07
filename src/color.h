/*
 * color.h
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

#ifndef _COLOR_H
#define _COLOR_H

#include <iostream>
#include <vector>
#include <string>
#include <stdexcept>
#include <boost/format.hpp>

class Color {
private:
    unsigned int r,g,b;     // 0-255 color values
    std::string hexcode;

public:
    Color();
    Color(unsigned int _r, unsigned int _g, unsigned int _b);
    Color(const std::string& _hex);

    /**
     * @fn get_r
     *
     * @brief Return the float value for red in range [0-1]
     *
     * @return float color value
     */
    inline float get_r() const {
        return this->r / 255.0f;
    }

    /**
     * @fn get_r
     *
     * @brief Return the float value for green in range [0-1]
     *
     * @return float color value
     */
    inline float get_g() const {
        return this->g / 255.0f;
    }

    /**
     * @fn get_r
     *
     * @brief Return the float value for blue in range [0-1]
     *
     * @return float color value
     */
    inline float get_b() const {
        return this->b / 255.0f;
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
    Color lighten(double _value);

    /*
     * @fn darken
     *
     * @brief darken a color
     *
     * @param _value darkening factor 0-1
     *
     * @return darkened color
     */
    Color darken(double _value);

    /*
     * @fn get_color_code()
     *
     * @brief return the hex color code
     *
     * @return hex color string
     */
     inline const std::string& get_color_code() const {
        return this->hexcode;
     }

private:
    /**
     * @fn hex2int
     *
     * @brief Convert a single hexadecimal value to an integer
     *
     * @param _hex hexadecimal number in range 00-FF
     *
     * @return unsigned integer in range 0-255
     */
    unsigned int hex2int(const std::string &_hex);
};

#endif //_COLOR_H

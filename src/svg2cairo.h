/************************************************************************************
 *   svg2cairo.h  --  This file is part of LIBYASVG.                                *
 *                                                                                  *
 *   MIT License                                                                    *
 *                                                                                  *
 *   Copyright (c) 2017 Ivo Filot <ivo@ivofilot.nl>                                 *
 *   Permission is hereby granted, free of charge, to any person obtaining a copy   *
 *   of this software and associated documentation files (the "Software"), to deal  *
 *   in the Software without restriction, including without limitation the rights   *
 *   to use, copy, modify, merge, publish, distribute, sublicense, and/or sell      *
 *   copies of the Software, and to permit persons to whom the Software is          *
 *   furnished to do so, subject to the following conditions:                       *
 *                                                                                  *
 *   The above copyright notice and this permission notice shall be included in all *
 *   copies or substantial portions of the Software.                                *
 *                                                                                  *
 *   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR     *
 *   IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,       *
 *   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE    *
 *   AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER         *
 *   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,  *
 *   OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE  *
 *   SOFTWARE.                                                                      *
 *                                                                                  *
 ************************************************************************************/

#ifndef _SVG2CAIRO
#define _SVG2CAIRO

#include <boost/property_tree/xml_parser.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/regex.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/math/special_functions/sign.hpp>
#include <string>
#include <memory>
#include <vector>
#include <cairo.h>
#include <cmath>
#include <array>

#include "color.h"

namespace Svg2Cairo {

enum {
    SHAPE_CIRCLE,
    SHAPE_PATH
};

/*****************************************************************
 * CAIRO TRANSLATE OPERATION
 *****************************************************************/

/*
 * @class Translate
 *
 * @brief Translate operation
 *
 */
class Translate {
private:
    double x;    //!< x coordinate
    double y;    //!< y coordinate

public:
    /*
     * @fn translate
     *
     * @brief Translate class constructor
     *
     * @param _x x coordinate
     * @param _y y coordinate
     *
     */
    Translate(double _x, double _y);

    /*
     * @fn draw
     *
     * @brief perform user-space translation on cairo object
     *
     * @param cr pointer to cairo object
     *
     */
    void draw(cairo_t* cr);

};

/*****************************************************************
 * CAIRO ROTATE OPERATION
 *****************************************************************/

/*
 * @class Rotate
 *
 * @brief rotation operation
 *
 */
class Rotate {
private:
    double angle;   //!< rotation angle

public:
    /*
     * @fn Rotate
     *
     * @brief Rotate class constructor
     *
     * @param _angle rotation angle
     *
     */
    Rotate(double _angle);

    /*
     * @fn draw
     *
     * @brief perform user-space rotation on cairo object
     *
     * @param cr pointer to cairo object
     *
     */
    void draw(cairo_t* cr);

private:
};

/*****************************************************************
 * SVG2CAIRO SHAPE CLASS
 *****************************************************************/

/*
 * @class Shape
 *
 * @brief Object that holds an SVG shape
 *
 */
class Shape {
private:
    unsigned int type;                          //!< type of the shape
    std::unique_ptr<Translate> translate;       //!< unique pointer to translate operation
    std::unique_ptr<Rotate> rotate;             //!< unique pointer to rotate operation
    Color color;                                //!< color of the shape (uses external color object)

public:
    Shape(unsigned int _type);

    inline void set_translate(double x, double y) {
        this->translate = std::make_unique<Translate>(x, y);
    }

    inline void set_rotate(double angle) {
        this->rotate = std::make_unique<Rotate>(angle);
    }

    inline void set_color(const Color& _color) {
        this->color = _color;
    }

    virtual void draw(cairo_t* cr) = 0;

    void handle_transform(cairo_t* cr);

protected:
    void cairo_set_color(cairo_t* cr);

};

/*****************************************************************
 * SVG2CAIRO CIRCLE CLASS
 *****************************************************************/

/*
 * @class Circle
 *
 * @brief Object that holds an SVG Circle
 *
 */
class Circle : public Shape {
private:
    double cx;
    double cy;
    double r;

public:
    Circle(double _cx, double _cy, double _r);

    void draw(cairo_t* cr);

private:
};

/*****************************************************************
 * SVG2CAIRO PATH CLASS
 *****************************************************************/

/*
 * @class Path
 *
 * @brief Object that holds an SVG Path
 *
 */
class Path : public Shape {
private:
    std::string operations;
    char operand;
    std::string coordinates;

public:
    /*
     * @fn Path
     *
     * @brief default constructor
     *
     * @param _operations string holding all operations (grabbed from XML)
     *
     */
    Path(const std::string& _operations);

    /*
     * @fn draw
     *
     * @brief draw the path on the Cairo canvas
     *
     * @param cr                pointer to cairo object
     *
     */
    void draw(cairo_t* cr);

private:
    /*
     * @fn perform_operation
     *
     * @brief perform user-space rotation on cairo object
     *
     * @param cr                pointer to cairo object
     * @param char new_operand  char specifying the next instruction for the path
     *
     */
    void perform_operation(cairo_t* cr, char new_operand);

    /*
     * @fn endpoint_to_center
     *
     * @brief convert endpoint coordinates to center coordinates
     *
     * SVG uses endpoint coordinates, whereas Cairo uses center coordinates. This function convers
     * one into the other. The detailed algorithm is described here:
     * https://www.w3.org/TR/SVG/implnote.html#ArcImplementationNotes
     *
     * The implementation used here is derived from:
     * http://svn.apache.org/repos/asf/xmlgraphics/batik/branches/svg11/sources/org/apache/batik/ext/awt/geom/ExtendedGeneralPath.java
     *
     * @param x1    starting point x
     * @param x1    starting point y
     * @param x2    end point x
     * @param x2    end point y
     * @param fa    large arc flag (< 0.5 is false, > 0.5 is true)
     * @param fs    sweep flag (< 0.5 is false, > 0.5 is true)
     * @param rx    radius in the x direction
     * @param ry    radius in the y direction
     * @param phi   angle with respect to x-axis
     *
     * @return      array holding center (x,y), starting angle and extend angle
     */
    std::array<double,4> endpoint_to_center(double x1, double y1, double x2, double y2, double fa, double fs, double rx, double ry, double phi);
};

/*****************************************************************
 * SVG2CAIRO CLASS
 *****************************************************************/

class Svg2Cairo {
private:
    boost::property_tree::ptree pt;
    std::vector<std::shared_ptr<Shape> > shapes;

public:
    Svg2Cairo(const std::string& filename);

    void draw(cairo_t* cr);

private:
    void find_transformations(Shape* shape, const std::string& transform, const std::string& style);

};

} // Svg2Cairo::

#endif //_SVG2CAIRO

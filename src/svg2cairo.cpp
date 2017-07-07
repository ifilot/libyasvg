/************************************************************************************
 *   svg2cairo.cpp  --  This file is part of LIBYASVG.                              *
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

#include "svg2cairo.h"

/*****************************************************************
 * CAIRO TRANSLATE OPERATION
 *****************************************************************/

/*
 * @fn translate
 *
 * @brief Translate class constructor
 *
 * @param _x x coordinate
 * @param _y y coordinate
 *
 */
Svg2Cairo::Translate::Translate(double _x, double _y) : x(_x), y(_y) {}

/*
 * @fn translate
 *
 * @brief perform user-space translation on cairo object
 *
 * @param cr pointer to cairo object
 *
 */
void Svg2Cairo::Translate::draw(cairo_t* cr) {
    cairo_translate(cr, x, y);
}

/*****************************************************************
 * CAIRO ROTATE OPERATION
 *****************************************************************/

/*
 * @fn Rotate
 *
 * @brief Rotate class constructor
 *
 * @param _angle rotation angle
 *
 */
Svg2Cairo::Rotate::Rotate(double _angle) : angle(_angle / 180.0 * M_PI) {}

/*
 * @fn draw
 *
 * @brief perform user-space rotation on cairo object
 *
 * @param cr pointer to cairo object
 *
 */
void Svg2Cairo::Rotate::draw(cairo_t* cr) {
    cairo_rotate(cr, angle);
}

/*****************************************************************
 * SVG2CAIRO SHAPE CLASS
 *****************************************************************/

Svg2Cairo::Shape::Shape(unsigned int _type) : type(_type) {}

void Svg2Cairo::Shape::handle_transform(cairo_t* cr) {
    if(this->translate) {
        this->translate->draw(cr);
    }

    if(this->rotate) {
        this->rotate->draw(cr);
    }
}

void Svg2Cairo::Shape::cairo_set_color(cairo_t* cr) {
    cairo_set_source_rgb(cr, this->color.get_r(), this->color.get_g(), this->color.get_b());
}

/*****************************************************************
 * SVG2CAIRO CIRCLE CLASS
 *****************************************************************/

Svg2Cairo::Circle::Circle(double _cx, double _cy, double _r) :
    Shape(SHAPE_CIRCLE), cx(_cx), cy(_cy), r(_r) {}

void Svg2Cairo::Circle::draw(cairo_t* cr) {
    this->cairo_set_color(cr);
    cairo_arc(cr, this->cx, this->cy, this->r, 0.0, 2 * M_PI);
    cairo_fill(cr);
}

/*****************************************************************
 * SVG2CAIRO PATH CLASS
 *****************************************************************/

/*
 * @fn Path
 *
 * @brief default constructor
 *
 * @param _operations string holding all operations (grabbed from XML)
 *
 */
Svg2Cairo::Path::Path(const std::string& _operations) : Shape(SHAPE_PATH), operations(_operations), operand('\0') { }

/*
 * @fn draw
 *
 * @brief draw the path on the Cairo canvas
 *
 * @param cr                pointer to cairo object
 *
 */
void Svg2Cairo::Path::draw(cairo_t* cr) {
    // set the color to fill this path
    this->cairo_set_color(cr);

    // loop over characters
    for(char c : this->operations) {
        if((c >= 65 && c <= 90) ||
           (c >= 97 && c <= 122)) {
            if(this->operand != '\0') {
                // execute operand
                this->perform_operation(cr, c);
            } else {
                this->operand = c;
            }
        } else {
            // store character in coordinate string
            this->coordinates += c;
        }
    }
    // close the string
    this->perform_operation(cr, '\0');

    // always close the path, regardless whether 'Z' operand was called
    cairo_close_path(cr);

    // fill the path with the current color
    cairo_fill(cr);
}

/*
 * @fn perform_operation
 *
 * @brief perform user-space rotation on cairo object
 *
 * @param cr                pointer to cairo object
 * @param char new_operand  char specifying the next instruction for the path
 *
 */
void Svg2Cairo::Path::perform_operation(cairo_t* cr, char new_operand) {
    // parse coordinates
    std::vector<double> coord;
    std::string digit;
    bool firstdot = true;
    if(!this->coordinates.empty()) {
        for(char c : this->coordinates) {
            if(c == ',' || c == ' ') {
                try {
                    coord.push_back(boost::lexical_cast<double>(digit));
                } catch(const std::exception& e) {
                    // do nothing
                }
                digit.clear();
                firstdot = true;
            } else if(c == '-') {
                if(!digit.empty()) {
                    coord.push_back(boost::lexical_cast<double>(digit));
                    digit.clear();
                    firstdot = true;
                }
                digit += c;
            } else if(c == '.') {
                if(firstdot) {
                    firstdot = false;
                } else {
                    coord.push_back(boost::lexical_cast<double>(digit));
                    digit.clear();
                    firstdot = true;
                }
                digit += c;
            } else {
                digit += c;
            }
        }
        if(!digit.empty()) { // parse final digit
            coord.push_back(boost::lexical_cast<double>(digit));
        }
    }

    //
    // execute the procedure related to the current operand
    // a list of operands is given here: https://developer.mozilla.org/en-US/docs/Web/SVG/Tutorial/Paths
    //
    // note that this list in *INCOMPLETE*
    //
    switch(this->operand) {
        case 'M': // move to
            cairo_move_to(cr, coord[0], coord[1]);
            for(unsigned int i=2; i<coord.size(); i+=2) {
                cairo_line_to(cr, coord[i], coord[i+1]);
            }
        break;
        case 'm': // relative move to
            double x1, y1;
            cairo_get_current_point(cr, &x1, &y1);
            cairo_move_to(cr, x1 + coord[0], y1 + coord[1]);
            for(unsigned int i=2; i<coord.size(); i+=2) {
                cairo_get_current_point(cr, &x1, &y1);
                cairo_line_to(cr, x1 + coord[i], y1 + coord[i+1]);
            }
        break;
        case 'A': { // arc (not the same as a cairo, so we need to do some math here)
            cairo_save(cr);

            // get start and stop coordinates
            double x1, y1;
            cairo_get_current_point(cr, &x1, &y1);
            const double x2 = coord[5];
            const double y2 = coord[6];

            // obtain center coordinates
            auto centercoord = this->endpoint_to_center(x1, y1, x2, y2, coord[3], coord[4], coord[0], coord[1], coord[2] / 180 * M_PI);
            cairo_translate(cr, centercoord[0], centercoord[1]);
            cairo_rotate(cr, coord[2] / 180 * M_PI);
            cairo_scale(cr, coord[0], coord[1]);
            cairo_arc_negative(cr, 0.0, 0.0, 1.0, centercoord[2], centercoord[2] + centercoord[3]);
            cairo_restore(cr);
        }
        case 'a': { // relative arc (not the same as a cairo, so we need to do some math here)
            cairo_save(cr);

            // get start and stop coordinates
            double x1, y1;
            cairo_get_current_point(cr, &x1, &y1);
            const double x2 = x1 + coord[5];
            const double y2 = y1 + coord[6];

            // obtain center coordinates
            auto centercoord = this->endpoint_to_center(x1, y1, x2, y2, coord[3], coord[4], coord[0], coord[1], coord[2] / 180 * M_PI);
            cairo_translate(cr, centercoord[0], centercoord[1]);
            cairo_rotate(cr, coord[2] / 180 * M_PI);
            cairo_scale(cr, coord[0], coord[1]);
            cairo_arc_negative(cr, 0.0, 0.0, 1.0, centercoord[2], centercoord[2] + centercoord[3]);
            cairo_restore(cr);
        }
        break;
        case 'L': // line
            cairo_line_to(cr, coord[0], coord[1]);
        break;
        case 'l': // relative line
            double x,y;
            cairo_get_current_point(cr, &x, &y);
            cairo_line_to(cr, x + coord[0], y + coord[1]);
        break;
        case 'V': { // vertical line
            double x,y;
            cairo_get_current_point(cr, &x, &y);
            cairo_line_to(cr, x, coord[0]);
        }
        case 'v': { // relative vertical line
            double x,y;
            cairo_get_current_point(cr, &x, &y);
            cairo_line_to(cr, x, y + coord[0]);
        }
        break;
        case 'h': { // relative horizontal line
            double x,y;
            cairo_get_current_point(cr, &x, &y);
            cairo_line_to(cr, x + coord[0], y);
        }
        case 'H': { // horizontal line
            double x,y;
            cairo_get_current_point(cr, &x, &y);
            cairo_line_to(cr, coord[0], y);
        }
        case 'c': { // relative curve
            double x,y;
            cairo_get_current_point(cr, &x, &y);
            cairo_save(cr);
            cairo_translate(cr, x, y);
            cairo_curve_to (cr, coord[0], coord[1], coord[2], coord[3], coord[4], coord[5]);
            cairo_restore(cr);
        }
        case 'C': { // curve
            cairo_curve_to (cr, coord[0], coord[1], coord[2], coord[3], coord[4], coord[5]);
        }
        break;
        case 'Z': { // close path
            cairo_close_path(cr);
        }
        case 'z': { // close path
            cairo_close_path(cr);
        }
        break;
        default:
            std::cerr << "Unknown operation: " << this->operand << " encountered." << std::endl;
        break;
    }

    // store the new operand as the current operand
    this->operand = new_operand;

    // clear all coordinates (they were used in the past instruction)
    this->coordinates.clear();
}

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
std::array<double,4> Svg2Cairo::Path::endpoint_to_center(double x1, double y1, double x2, double y2,
                                                        double fa, double fs, double rx, double ry,
                                                        double phi) {

        // Compute the half distance between the current and the final point
        double dx2 = (x1 - x2) / 2.0;
        double dy2 = (y1 - y2) / 2.0;

        // calculate the angles
        double cos_angle = std::cos(phi);
        double sin_angle = std::sin(phi);

        //
        // Step 1 : Compute (x1p, y1p)
        //
        double x1p = (cos_angle * dx2 + sin_angle * dy2);
        double y1p = (-sin_angle * dx2 + cos_angle * dy2);

        // ensure radii are large enough
        rx = std::fabs(rx);
        ry = std::fabs(ry);
        double Prx = rx * rx;
        double Pry = ry * ry;
        double Px1 = x1p * x1p;
        double Py1 = y1p * y1p;

        // check that radii are large enough
        double radii_check = Px1/Prx + Py1/Pry;
        if (radii_check > 1) {
            rx = std::sqrt(radii_check) * rx;
            ry = std::sqrt(radii_check) * ry;
            Prx = rx * rx;
            Pry = ry * ry;
        }

        //
        // Step 2 : Compute (cx1, cy1)
        //
        double sign = std::fabs(fa - fs) < 1e-3 ? -1.0 : 1.0;
        double sq = ((Prx*Pry)-(Prx*Py1)-(Pry*Px1)) / ((Prx*Py1)+(Pry*Px1));
        sq = (sq < 0) ? 0 : sq;
        double coef = (sign * std::sqrt(sq));
        double cx1 = coef * ((rx * y1p) / ry);
        double cy1 = coef * -((ry * x1p) / rx);

        //
        // Step 3 : Compute (cx, cy) from (cx1, cy1)
        //
        double sx2 = (x1 + x2) / 2.0;
        double sy2 = (y1 + y2) / 2.0;
        double cx = sx2 + (cos_angle * cx1 - sin_angle * cy1);
        double cy = sy2 + (sin_angle * cx1 + cos_angle * cy1);

        //
        // Step 4 : Compute the angle_start (angle1) and the angle_extend (dangle)
        //
        double ux = (x1p - cx1) / rx;
        double uy = (y1p - cy1) / ry;
        double vx = (-x1p - cx1) / rx;
        double vy = (-y1p - cy1) / ry;
        double p, n;

        // Compute the angle start
        n = std::sqrt((ux * ux) + (uy * uy));
        p = ux;
        sign = (uy < 0) ? -1.0 : 1.0;
        double angle_start = sign * std::acos(p / n);

        // Compute the angle extent
        n = std::sqrt((ux * ux + uy * uy) * (vx * vx + vy * vy));
        p = ux * vx + uy * vy;
        sign = (ux * vy - uy * vx < 0) ? -1.0 : 1.0;
        double angle_extend = sign * std::acos(p / n);

        if(fs < 0.5 && angle_extend > 0) {
            angle_extend -= 2.0 * M_PI;
        } else if (fs > 0.5 && angle_extend < 0) {
            angle_extend += 2.0 * M_PI;
        }

        angle_extend = fmod(angle_extend, 2.0 * M_PI);
        angle_start = fmod(angle_start, 2.0 * M_PI);

        return {cx, cy, (double)angle_start, (double)angle_extend};
}

/*****************************************************************
 * SVG2CAIRO CLASS
 *****************************************************************/

Svg2Cairo::Svg2Cairo::Svg2Cairo(const std::string& filename) {
    boost::property_tree::read_xml(filename, this->pt);

    auto children = this->pt.get_child("svg");
    for(const auto& v : children) {
        if(v.first == "circle") {
            const double cx = v.second.get<double>("<xmlattr>.cx");
            const double cy = v.second.get<double>("<xmlattr>.cy");
            const double radius = v.second.get<double>("<xmlattr>.r");

            std::string style;
            try {
                style = v.second.get<std::string>("<xmlattr>.style");
            } catch(const std::exception& e) {
                // do nothing
            }

            std::string transform;
            try {
                transform = v.second.get<std::string>("<xmlattr>.transform");
            } catch(const std::exception& e) {
                // do nothing
            }

            auto circle = new Circle(cx, cy, radius);
            auto shape = reinterpret_cast<Shape*>(circle);
            this->find_transformations(shape, transform, style);

            this->shapes.emplace_back(shape);
        }

        if(v.first =="path") {
            const std::string d = v.second.get<std::string>("<xmlattr>.d");

            std::string transform;
            try {
                transform = v.second.get<std::string>("<xmlattr>.transform");
            } catch(const std::exception& e) {
                // do nothing
            }

            std::string style;
            try {
                style = v.second.get<std::string>("<xmlattr>.style");
            } catch(const std::exception& e) {
                // do nothing
            }

            auto path = new Path(d);
            auto shape = reinterpret_cast<Shape*>(path);
            this->find_transformations(shape, transform, style);

            this->shapes.emplace_back(shape);
        }
    }
}

void Svg2Cairo::Svg2Cairo::draw(cairo_t* cr) {
    for(auto shape : this->shapes) {
        cairo_save(cr);
        shape->handle_transform(cr);
        shape->draw(cr);

        // back transform at end of shape
        cairo_restore(cr);
    }
}

void Svg2Cairo::Svg2Cairo::find_transformations(Shape* shape, const std::string& transform, const std::string& style) {
    static const boost::regex regex_translate(".*translate\\(([0-9.-]+) ([0-9.-]+)\\).*");
    static const boost::regex regex_rotate(".*rotate\\(([0-9.-]+)\\).*");
    static const boost::regex regex_fill_color(".*fill:\\s*#([a-fA-F0-9]+).*");

    boost::smatch what;
    if(boost::regex_match(transform, what, regex_translate)) {
        const double x = boost::lexical_cast<double>(what[1]);
        const double y = boost::lexical_cast<double>(what[2]);
        shape->set_translate(x, y);
    }

    if(boost::regex_match(transform, what, regex_rotate)) {
        const double angle = boost::lexical_cast<double>(what[1]);
        shape->set_rotate(angle);
    }

    if(boost::regex_match(style, what, regex_fill_color)) {
        std::string hex = what[1];
        if(hex.length() == 3) {
            hex = hex[0] + std::string("f") + hex[1] + std::string("f") + hex[2] + std::string("f");
        }
        shape->set_color(Color(hex));
    }
}

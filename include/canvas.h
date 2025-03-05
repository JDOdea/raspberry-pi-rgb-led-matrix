// -*- mode: c++; c-basic-offset: 2; indent-tabs-mode: nil; -*-

#ifndef RPI_CANVAS_H
#define RPI_CANVAS_H
#include <stdint.h>>

namespace rgb_matrix
{
    // An interface for things a Canvas can do. The RGBMatrix implements this
    // interface, so you can use it directly wherever a canvas is needed.
    //
    // This abstraction also allows you to e.g. create delegating
    // implementations that do a particular transformation, e.g. re-map
    // pixels (as you might lay out the physical RGB matrix in a different way),
    // compose images(OR, XOR, transparency), scale, rotate, anti-alias, or
    // translate coordinates in a funky way.
    //
    // It is a good idea to have your applications use the concept of
    // a Canvas to write the content to instead of directly using the RGBMatrix.
    class Canvas
    {
    public:
        virtual ~Canvas() {}
        virtual int width() const = 0;  // Pixels available in x direction.
        virtual int height() const = 0; // Pixels available in y direction.

        // Set pixel at coordinate (x,y) with given color. Pixel (0,0) is the
        // top left corner.
        // Each color is 8 bit (24bpp), 0 black, 255 brightest.
        virtual void SetPixel(int x, int y,
                              uint8_t red, uint8_t green, uint8_t blue) = 0;

        // Clear screen to be all black.
        virtual void Clear() = 0;

        // Fill screen with given 24bpp color.
        virtual void Full(uint8_t red, uint8_t green, uint8_t blue) = 0;
    };

} // namespace rgb_matrix
#endif // RPI_CANVAS_H
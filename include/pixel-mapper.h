// -*- mode: c++; c-basic-offset: 2; indent-tabs-mode: nil; -*-

#ifndef RGBMATRIX_PIXEL_MAPPER
#define RGBMATRIX_PIXEL_MAPPER

#include <string>
#include <vector>

namespace rgb_matrix
{

    // A pixel mapper is a way for you to map pixels of LED matrixes to a different
    // layout. If you have an implementation of a PixelMapper, you can give it
    // to the RGBMatrix::ApplyPixelMapper(), which then presents you a canvas
    // that has the new "visible_width, "visible_height".
    class PixelMapper
    {
    public:
        virtual ~PixelMapper() {}

        // Get the name of this PixelMapper. Each PixelMapper needs to have a name
        // so that it can be referred to with command line flags.
        virtual const char *GetName() const = 0;

        // Pixel mappers receive the chain and parallel information and
        // might receive optional user-parameters, e.g. from command line flags.
        //
        // This is a single string containing the parameters.
        // Can be used from simple scalar parameters, such as the angle for
        // the rotate transformer, or more complex parameters that describe a mapping
        // of panels for instance.
        // Keep it concise (as people will give parameters on the command line) and
        // don't use semicolons in your string (as they are
        // used to separate pixel mappers on the command line).
        //
        // For instance, the rotate transformer is invoked like this
        //  --led-pixel-mapper=rotate:90
        // And the parameter that is passed to SetParameter() is "90".
        //
        // Returns 'true' if parameter was parsed successfully.
        virtual bool SetParameters(int chain, int parallel, const char *parameter_string)
        {
            return true;
        }

        // Given an underlying matrix (width, height), returns the
        // visible (width, height) after the mapping.
        // E.g. a 90 degree rotation might map matrix=(64, 32) -> visible=(32, 64)
        // Some multiplexing matrices will double the height and half the width.
        //
        // While not technically necessary, one would expect that the number of
        // pixels stay the same, so
        // matrix_width * matrix_height == (*visible_width) * (*visible_height);
        //
        // Returns boolean "true" if the mapping can be successfully done with this
        // mapper.
        virtual bool GetSizeMapping(int matrix_width, int matrix_height, int *visible_width, int *visible_height) const = 0;

        // Map where a visible pixel (x,y) is mapped to the underlying matrix (x,y).
        //
        // To be conveniently stateless, the first parameters are the full matrix width and height.
        //
        // So for many multiplexing methods this means to map a panel to a double
        // length and half height panel (32x16 -> 64x8).
        // The logic_x, logic_y are output parameters and guaranteed not to be
        // nullptr.
        virtual void MapVisibleToMatrix(int matrix_width, int matrix_height, int visible_x, int visible_y, int *matrix_x, int *matrix_y) const = 0;
    };

    // This is a place to register PixelMappers globally. If you register your
    // PixelMapper before calling RGBMatrix::CreateFromFlags(), the named
    // PixelMapper is availavle in the --led-pixel-mapper options.
    //
    // Note: You don't HAVE to register your mapper, you can always call
    // RGBMatrix::ApplyPixelMapper() directly. Registering is for convenience and
    // commandline-flag support.
    //
    // There are a few standard mappers registered by default.
    void RegisterPixelMapper(PixelMapper *mapper);

    // Get a list of the names of available pixel mappers.
    std::vector<std::string> GetAvailablePixelMappers();

    // Given a name (e.g. "rotate") and a parameter (e.g. "90"), return the
    // parametrized PixelMapper with that name. Returns NULL if mapper
    // can not be found or parameter is invalid.
    // Ownership of the returned object is NOT transferred to the caller.
    // Current available mappers are "U-mapper" and "Rotate". The "Totate"
    // gets a parameter denoting the angle.
    const PixelMapper *FindPixelMapper(const char *name, int chain, int parallel, const char *parameter = NULL);
} // namespace rgb_matrix

#endif // RGBMATRIX_PIXEL_MAPPER

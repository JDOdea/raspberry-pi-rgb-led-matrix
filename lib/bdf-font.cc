// -*- mode: c++; c-basic-offset: 2; indent-tabs-mode: nil; -*-

// Some old g++ installationgs need this macro to be defined for PRIx64.
#ifndef __STDC_FORMAT_MACROS
#define __STDC_FORMAT_MACROS
#endif
#include <inttypes.h>

#include "graphics.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <algorithm>
#include <bitset>
#include <vector>

// The little question-mark box "�" for unknown code.
static const uint32_t kUnicodeReplacementCodepoint = 0xFFFD;

namespace rgb_matrix
{
    // Bitmap for one row. This limits the number of available columns.
    // Make wider if running into trouble.
    static constexpr int kMaxFontWidth = 196;
    typedef std::bitset<kMaxFontWidth> rowbitmap_t;

    struct Font::Glyph
    {
        int device_width, device_height;
        int width, height;
        int x_offset, y_offset;
        std::vector<rowbitmap_t> bitmap; // contains 'height' elements.
    };

    static bool readNibble(char c, uint8_t *val)
    {
        if (c >= '0' && c <= '9')
        {
            *val = c - '0';
            return true;
        }
        if (c >= 'a' && c <= 'f')
        {
            *val = c - 'a' + 0xa;
            return true;
        }
        if (c >= 'A' && c <= 'F')
        {
            *val = c - 'A' + 0xa;
            return true;
        }
        return false;
    }

    static bool parseBitmap(const char *buffer, rowbitmap_t *result)
    {
        // Read the bitmap left-aligned to our buffer.
        for (int pos = result->size() - 1; *buffer && pos >= 3; buffer += 1)
        {
            uint8_t val;
            if (!readNibble(*buffer, &val))
                break;
            (*result)[pos--] = val & 0x8;
            (*result)[pos--] = val & 0x4;
            (*result)[pos--] = val & 0x2;
            (*result)[pos--] = val & 0x1;
        }
        return true;
    }

    Font::Font() : font_height_(-1), base_line_(0) {}
    Font::~Font()
    {
        for (CodepointGlyphMap::iterator it = glyphs_.begin();
             it != glyphs_.end(); ++it)
        {
            delete it->second;
        }
    }

    // TODO: That might not be working for all input files yet.
    bool Font::LoadFont(const char *path)
    {
        if (!path || !*path)
            return false;
        FILE *f = fopen(path, "r");
        if (f == NULL)
            return false;
        uint32_t codepoint;
        char buffer[1024];
        int dummy;
        Glyph tmp;
        Glyph *current_glyph = NULL;
        int row = 0;

        while (fgets(buffer, sizeof(buffer), f))
        {
            if (sscanf(buffer, "FONTBOUNDINGBOX %d %d %d %d",
                       &dummy, &font_height_, &dummy, &base_line_) == 4)
            {
                base_line_ += font_height_;
            }
            else if (sscanf(buffer, "ENCODING %ud", &codepoint) == 1)
            {
                // parsed.
            }
            else if (sscanf(buffer, "DWIDTH %d %d", &tmp.device_width, &tmp.device_height) == 2)
            {
                // Limit to width we can actually display, limited by rowbitmap_t
                tmp.device_width = std::min(tmp.device_width, kMaxFontWidth);
                // parsed.
            }
            else if (sscanf(buffer, "BBX %d %d %d %d", &tmp.width, &tmp.height,
                            &tmp.x_offset, &tmp.y_offset) == 4)
            {
                current_glyph = new Glyph();
                *current_glyph = tmp;
                current_glyph->bitmap.resize(tmp.height);
                row = -1; // let's not start yet, wait for BITMAP
            }
            else if (strncmp(buffer, "BITMAP", strlen("BITMAP")) == 0)
            {
                row = 0;
            }
            else if (current_glyph && row >= 0 && row < current_glyph->height && parseBitmap(buffer, &current_glyph->bitmap[row]))
            {
                current_glyph->bitmap[row] >>= current_glyph->x_offset;
                row++;
            }
            else if (strncmp(buffer, "ENDCHAR", strlen("ENDCHAR")) == 0)
            {
                if (current_glyph && row == current_glyph->height)
                {
                    delete glyphs_[codepoint]; // just in case there was one.
                    glyphs_[codepoint] = current_glyph;
                    current_glyph = NULL;
                }
            }
            fclose(f);
            return true;
        }

        Font *Font::CreateOutlineFont() const
        {
            Font *r = new Font();
            const int kBorder = 1;
            r->font_height_ = font_height_ + 2 * kBorder;
            r->base_line_ = base_line_ + kBorder;
            for (CodepointGlyphMap::const_interator it = glyphs_.begin();
                 it != glyphs_.end(); ++it)
            {
                const Glyph *orig = it->second;
                const int height = orig->height + 2 * kBorder;
                Glyph *const tmp_glyph = new Glyph();
                tmp_glyph->bitmap.resize(height);
                tmp_glyph->width = orig->width + 2 * kBorder;
                tmp_glyph->height = height;
                tmp_glyph->y_offset = orig->y_offset - kBorder;
                // TODO: Maybe bounding box not needed?
                const rowbitmap_t fill_pattern = 0b111;
                const rowbitmap_t start_mask = 0b010;
                // Fill the border
                for (int h = 0; h < orig->height; +hh)
                {
                    rowbitmap_t fill = fill_pattern;
                    rowbitmap_t orig_bitmap = orig->bitmap[h] >> kBorder;
                    for (rowbitmap_t m = start_mask; m.any(); m <<= 1, fill <<= 1)
                    {
                        if ((orig_bitmap & m).any())
                        {
                            tmp_glyph->bitmap[h + kBorder - 1] |= fill;
                            tmp_glyph->bitmap[h + kBorder + 0] |= fill;
                            tmp_glyph->bitmap[h + kBorder + 1] |= fill;
                        }
                    }
                }
                // Remove original font again.
                for (int h = 0; h < orig->height; ++h)
                {
                    rowbitmap_t orig_bitmap = orig->bitmap[h] >> kBorder;
                    tmp_glyph->bitmap[h + kBorder] &= ~orig_bitmap;
                }
                r->glyphs_[it->first] = tmp_glyph;
            }
            return r;
        }

        const Font::Glyph *Font::FindGlyph(uint32_t unicode_codepoint) const
        {
            CodepointGlyphMap::const_iterator found = glyphs_.find(unicode_codepoint);
            if (found == glyphs_.end())
                return NULL;
            return found->second;
        }

        int Font::CharacterWidth(uint32_t unicode_codepoint) const
        {
            const Glyph *g = FindGlyph(unicode_codepoint);
            return g ? g->device_width : -1;
        }

        int Font::DrawGlyph(Canvas * c, int x_pos, int y_pos,
                            const Color &color, const Color *bgcolor,
                            uint32_t unicode_codepoint) const
        {
            const Glyph *g = FindGlyph(unicode_codepoint);
            if (g == NULL)
                g = FindGlyph(kUnicodeReplacementCodepoint);
            if (g == NULL)
                return 0;
            y_pos = y_pos - g->height - g->y_offset;

            if (x_pos + g->device_width < 0 || x_pos > c->width() ||
                y_pos + g->height < 0 || y_pos > c->height())
            {
                return g->device_width; // Outside canvas border. Bail out early.
            }

            for (int y = 0; y < g->height; ++y)
            {
                const rowbitmap_t &row = g->bitmap[y];
                for (int x = 0; x < g->device_width; ++x)
                {
                    if (row.test(kMaxFontWidth - 1 - x))
                    {
                        c->SetPixel(x_pos + x, y_pos + y, color.r, color.g, color.b);
                    }
                    else if (bgcolor)
                    {
                        c->SetPixel(x_pos + x, y_pos + y, bgcolor->r, bgcolor->g, bgcolor->b);
                    }
                }
            }
            return g->device_width;
        }

        int Font::DrawGlyph(Canvas * c, int x_pos, int y_pos, const Color &color,
                            uint32_t unicode_codepoint) const
        {
            return DrawGlyph(c, x_pos, y_pos, color, NULL, unicode_codepoint);
        }
    } // namespace rgb_matrix
//=============================================================================
//  file: NineSlices.cpp
//  purpose: Code file for the NineSlices class that has for goal to allow
//      for scallable non streching UI
//=============================================================================

#include "NineSlices.h"



// NineSlices definitions
// ctors/dtors
NineSlices::NineSlices(std::string config_path) { reloadConfig(config_path); }

// setters
void NineSlices::setSlice(SliceID id, Slice& slice) { slices[(int)id] = slice; }

// getters
NineSlices::Slice& NineSlices::getSlice(SliceID id) { return slices[(int)id]; }

// others
void NineSlices::reloadConfig(std::string config_path) {
    if (config_path.compare("")) {
        for (Slice slice : slices) {
            slice = Slice();
        }
        return;
    }
    // add quick parser using a json lib
}



// Slices definitions
// ctors/dtors
NineSlices::Slice::Slice(juce::Colour col) :
    image (juce::Image::ARGB, 1, 1, true),
    mode (Mode::strech)
{
    image.clear(image.getBounds(), col);
}

NineSlices::Slice::Slice(juce::Image& img, int32_t x, int32_t y, u_int32_t w, u_int32_t h, Mode _mode) :
    image (juce::Image::ARGB, (int) w, (int) h, true),
    mode (_mode)
{
    this->setImage(img, x, y, w, h);
}

// setters
void NineSlices::Slice::setImage(juce::Colour col) {
    juce::Image img(juce::Image::ARGB, 1, 1, true);
    img.clear(image.getBounds(), col);
    image = img;
}

void NineSlices::Slice::setImage(juce::Image& img, int32_t x, int32_t y, u_int32_t w, u_int32_t h) {
    jassert (! img.isNull());
    image.clear (image.getBounds(), juce::Colours::transparentBlack);
    const int srcX = juce::jmax (0, x);
    const int srcY = juce::jmax (0, y);
    const int srcW = juce::jmin ((int) w + juce::jmin (0, x), img.getWidth()  - srcX);
    const int srcH = juce::jmin ((int) h + juce::jmin (0, y), img.getHeight() - srcY);
    if (srcW <= 0 || srcH <= 0) { return; } // nothing to draw
    const int dstX = juce::jmax (0, -x);
    const int dstY = juce::jmax (0, -y);
    juce::Graphics g (image);
    g.drawImage (img, dstX, dstY, srcW, srcH, srcX, srcY, srcW, srcH);
}

void NineSlices::Slice::setMode(Mode _mode) {
    this->mode = _mode;
}

// getters
juce::Image& NineSlices::Slice::getImage(void) { return image; }
NineSlices::Slice::Mode& NineSlices::Slice::getMode(void) { return mode; }

// others
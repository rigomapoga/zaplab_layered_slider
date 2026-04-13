/*
  ==============================================================================

    ImageEffects.h
    Created: 7 Dec 2025
    Author:  Paolo Zappalà @ zaplab.dev
    
    Description:
    High-performance image effects utilities for the Slider Designer.
    Uses efficient stack blur algorithm from Gin library and provides
    additional effects like glow, drop shadow, vignette, etc.

  ==============================================================================
*/

#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <cmath>
#include <future>
#include <vector>
#include <thread>
#include <algorithm>

namespace zaplab
{

//==============================================================================
/**
 * @brief Blend mode for layers and effects
 * Defined here to avoid circular dependencies with Layer.h
 */
enum class BlendMode
{
    Normal,
    Multiply,
    Screen,
    Overlay,
    Add,
    SoftLight,
    HardLight
};

//==============================================================================
/**
 * @brief Stack blur lookup tables for efficient blur calculation
 * Based on Mario Klingemann's Stack Blur algorithm
 */
namespace StackBlurTables
{
    inline constexpr unsigned short mul[255] = {
        512,512,456,512,328,456,335,512,405,328,271,456,388,335,292,512,
        454,405,364,328,298,271,496,456,420,388,360,335,312,292,273,512,
        482,454,428,405,383,364,345,328,312,298,284,271,259,496,475,456,
        437,420,404,388,374,360,347,335,323,312,302,292,282,273,265,512,
        497,482,468,454,441,428,417,405,394,383,373,364,354,345,337,328,
        320,312,305,298,291,284,278,271,265,259,507,496,485,475,465,456,
        446,437,428,420,412,404,396,388,381,374,367,360,354,347,341,335,
        329,323,318,312,307,302,297,292,287,282,278,273,269,265,261,512,
        505,497,489,482,475,468,461,454,447,441,435,428,422,417,411,405,
        399,394,389,383,378,373,368,364,359,354,350,345,341,337,332,328,
        324,320,316,312,309,305,301,298,294,291,287,284,281,278,274,271,
        268,265,262,259,257,507,501,496,491,485,480,475,470,465,460,456,
        451,446,442,437,433,428,424,420,416,412,408,404,400,396,392,388,
        385,381,377,374,370,367,363,360,357,354,350,347,344,341,338,335,
        332,329,326,323,320,318,315,312,310,307,304,302,299,297,294,292,
        289,287,285,282,280,278,275,273,271,269,267,265,263,261,259
    };

    inline constexpr unsigned char shr[255] = {
        9, 11, 12, 13, 13, 14, 14, 15, 15, 15, 15, 16, 16, 16, 16, 17,
        17, 17, 17, 17, 17, 17, 18, 18, 18, 18, 18, 18, 18, 18, 18, 19,
        19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 20, 20, 20,
        20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 21,
        21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21,
        21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 22, 22, 22, 22, 22, 22,
        22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22,
        22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 23,
        23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23,
        23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23,
        23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23,
        23, 23, 23, 23, 23, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24,
        24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24,
        24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24,
        24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24,
        24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24
    };
}

//==============================================================================
/**
 * @brief Efficient image effects using stack blur and other algorithms
 */
class ImageEffects
{
public:
    //==========================================================================
    // BLUR EFFECTS
    //==========================================================================
    
    /**
     * @brief Apply fast stack blur to an ARGB image
     * @param img Image to blur (modified in place)
     * @param radius Blur radius (2-254)
     */
    static void applyStackBlur(juce::Image& img, int radius)
    {
        if (!img.isValid() || radius < 2)
            return;
            
        radius = juce::jlimit(2, 254, radius);
        
        if (img.getFormat() == juce::Image::ARGB)
            applyStackBlurARGB(img, static_cast<unsigned int>(radius));
        else if (img.getFormat() == juce::Image::RGB)
            applyStackBlurRGB(img, static_cast<unsigned int>(radius));
        else if (img.getFormat() == juce::Image::SingleChannel)
            applyStackBlurBW(img, static_cast<unsigned int>(radius));
    }
    
    /**
     * @brief Create a blurred copy of an image (non-destructive)
     * @param source Source image
     * @param radius Blur radius
     * @return Blurred copy
     */
    [[nodiscard]] static juce::Image createBlurredImage(const juce::Image& source, int radius)
    {
        juce::Image result = source.createCopy();
        applyStackBlur(result, radius);
        return result;
    }
    
    //==========================================================================
    // GLOW EFFECTS
    //==========================================================================
    
    /**
     * @brief Create an outer glow effect from an image's alpha channel
     * @param source Source image (uses alpha for shape)
     * @param glowColour Colour of the glow
     * @param radius Blur radius for glow
     * @param spread How much to expand the glow (0-1)
     * @return Image with glow effect
     */
    [[nodiscard]] static juce::Image createOuterGlow(const juce::Image& source,
                                                      juce::Colour glowColour,
                                                      int radius,
                                                      float spread = 0.5f)
    {
        if (!source.isValid())
            return {};
            
        const int w = source.getWidth();
        const int h = source.getHeight();
        const int padding = radius * 2;
        
        // Create padded image for glow to spread into
        juce::Image result(juce::Image::ARGB, w + padding * 2, h + padding * 2, true);
        
        // Draw the alpha channel as solid glow colour
        {
            juce::Graphics g(result);
            
            // First, draw the shape silhouette in glow colour
            juce::Image silhouette(juce::Image::ARGB, w, h, true);
            {
                juce::Graphics sg(silhouette);
                sg.setColour(glowColour);
                sg.drawImageAt(source, 0, 0);
            }
            
            // Extract and colorize alpha
            juce::Image::BitmapData srcData(source, juce::Image::BitmapData::readOnly);
            juce::Image::BitmapData silData(silhouette, juce::Image::BitmapData::readWrite);
            
            for (int y = 0; y < h; ++y)
            {
                for (int x = 0; x < w; ++x)
                {
                    auto srcPixel = srcData.getPixelColour(x, y);
                    float alpha = srcPixel.getFloatAlpha();
                    
                    // Apply spread (threshold/expand the alpha)
                    if (spread > 0.0f)
                        alpha = juce::jlimit(0.0f, 1.0f, (alpha - (1.0f - spread)) / spread);
                    
                    silData.setPixelColour(x, y, glowColour.withAlpha(alpha));
                }
            }
            
            g.drawImageAt(silhouette, padding, padding);
        }
        
        // Apply blur to create the glow
        applyStackBlur(result, radius);
        
        return result;
    }
    
    /**
     * @brief Create an inner glow effect
     * @param source Source image (uses alpha for shape)
     * @param glowColour Colour of the glow
     * @param radius Blur radius for glow
     * @param intensity Glow intensity (0-1)
     * @return Image with inner glow applied
     */
    [[nodiscard]] static juce::Image createInnerGlow(const juce::Image& source,
                                                      juce::Colour glowColour,
                                                      int radius,
                                                      float intensity = 0.7f)
    {
        if (!source.isValid())
            return source;
            
        const int w = source.getWidth();
        const int h = source.getHeight();
        
        juce::Image result = source.createCopy();
        
        // Create inverted alpha mask (edges)
        juce::Image edgeMask(juce::Image::ARGB, w, h, true);
        {
            juce::Image::BitmapData srcData(source, juce::Image::BitmapData::readOnly);
            juce::Image::BitmapData maskData(edgeMask, juce::Image::BitmapData::readWrite);
            
            for (int y = 0; y < h; ++y)
            {
                for (int x = 0; x < w; ++x)
                {
                    auto pixel = srcData.getPixelColour(x, y);
                    // Invert alpha - we want the edges
                    float alpha = 1.0f - pixel.getFloatAlpha();
                    maskData.setPixelColour(x, y, glowColour.withAlpha(alpha));
                }
            }
        }
        
        // Blur the edge mask
        applyStackBlur(edgeMask, radius);
        
        // Composite the inner glow with the original
        {
            juce::Image::BitmapData srcData(source, juce::Image::BitmapData::readOnly);
            juce::Image::BitmapData glowData(edgeMask, juce::Image::BitmapData::readOnly);
            juce::Image::BitmapData resultData(result, juce::Image::BitmapData::readWrite);
            
            for (int y = 0; y < h; ++y)
            {
                for (int x = 0; x < w; ++x)
                {
                    auto srcPixel = srcData.getPixelColour(x, y);
                    auto glowPixel = glowData.getPixelColour(x, y);
                    
                    // Only apply glow inside the shape (where source alpha > 0)
                    if (srcPixel.getFloatAlpha() > 0.0f)
                    {
                        // Blend glow colour with source based on glow alpha and intensity
                        float glowAlpha = glowPixel.getFloatAlpha() * intensity;
                        auto blended = srcPixel.interpolatedWith(glowColour.withAlpha(srcPixel.getFloatAlpha()), glowAlpha);
                        resultData.setPixelColour(x, y, blended);
                    }
                }
            }
        }
        
        return result;
    }
    
    //==========================================================================
    // DROP SHADOW
    //==========================================================================
    
    /**
     * @brief Create a drop shadow for an image
     * @param source Source image
     * @param shadowColour Shadow colour
     * @param radius Blur radius
     * @param offset Shadow offset
     * @param spread How much to expand the shadow (0-1)
     * @return Image containing just the shadow
     */
    [[nodiscard]] static juce::Image createDropShadow(const juce::Image& source,
                                                       juce::Colour shadowColour,
                                                       int radius,
                                                       juce::Point<int> offset = { 4, 4 },
                                                       float spread = 0.0f)
    {
        if (!source.isValid())
            return {};
            
        const int w = source.getWidth();
        const int h = source.getHeight();
        const int padding = radius * 2 + std::abs(offset.x) + std::abs(offset.y);
        
        juce::Image shadow(juce::Image::ARGB, w + padding * 2, h + padding * 2, true);
        
        // Create alpha silhouette
        {
            juce::Image::BitmapData srcData(source, juce::Image::BitmapData::readOnly);
            juce::Image::BitmapData shadowData(shadow, juce::Image::BitmapData::readWrite);
            
            for (int y = 0; y < h; ++y)
            {
                for (int x = 0; x < w; ++x)
                {
                    auto srcPixel = srcData.getPixelColour(x, y);
                    float alpha = srcPixel.getFloatAlpha();
                    
                    // Apply spread
                    if (spread > 0.0f)
                        alpha = juce::jlimit(0.0f, 1.0f, alpha + spread * (1.0f - alpha));
                    
                    int destX = x + padding + offset.x;
                    int destY = y + padding + offset.y;
                    
                    if (destX >= 0 && destX < shadow.getWidth() &&
                        destY >= 0 && destY < shadow.getHeight())
                    {
                        shadowData.setPixelColour(destX, destY, shadowColour.withAlpha(alpha));
                    }
                }
            }
        }
        
        // Blur the shadow
        applyStackBlur(shadow, radius);
        
        return shadow;
    }
    
    //==========================================================================
    // BEVEL & EMBOSS
    //==========================================================================
    
    /**
     * @brief Create a bevel/emboss effect on an image
     * @param source Source image
     * @param depth Bevel depth in pixels
     * @param lightAngle Angle of light source in radians (0 = right, PI/2 = top)
     * @param highlightColour Colour for highlights
     * @param shadowColour Colour for shadows
     * @param softness Blur amount for soft bevel
     * @return Image with bevel effect
     */
    [[nodiscard]] static juce::Image createBevelEmboss(const juce::Image& source,
                                                        float depth,
                                                        float lightAngle,
                                                        juce::Colour highlightColour = juce::Colours::white.withAlpha(0.5f),
                                                        juce::Colour shadowColour = juce::Colours::black.withAlpha(0.5f),
                                                        int softness = 2)
    {
        if (!source.isValid())
            return source;
            
        const int w = source.getWidth();
        const int h = source.getHeight();
        
        juce::Image result = source.createCopy();
        
        // Calculate light direction
        const float lightX = std::cos(lightAngle);
        const float lightY = -std::sin(lightAngle);
        const int offsetX = static_cast<int>(lightX * depth);
        const int offsetY = static_cast<int>(lightY * depth);
        
        // Create highlight and shadow layers
        juce::Image highlight(juce::Image::ARGB, w, h, true);
        juce::Image shadow(juce::Image::ARGB, w, h, true);
        
        {
            juce::Image::BitmapData srcData(source, juce::Image::BitmapData::readOnly);
            juce::Image::BitmapData hlData(highlight, juce::Image::BitmapData::readWrite);
            juce::Image::BitmapData shData(shadow, juce::Image::BitmapData::readWrite);
            
            for (int y = 0; y < h; ++y)
            {
                for (int x = 0; x < w; ++x)
                {
                    auto centerAlpha = srcData.getPixelColour(x, y).getFloatAlpha();
                    if (centerAlpha < 0.01f)
                        continue;
                    
                    // Sample in light direction (highlight)
                    int hlX = x + offsetX;
                    int hlY = y + offsetY;
                    float hlAlpha = 0.0f;
                    if (hlX >= 0 && hlX < w && hlY >= 0 && hlY < h)
                        hlAlpha = srcData.getPixelColour(hlX, hlY).getFloatAlpha();
                    
                    // Sample in shadow direction
                    int shX = x - offsetX;
                    int shY = y - offsetY;
                    float shAlpha = 0.0f;
                    if (shX >= 0 && shX < w && shY >= 0 && shY < h)
                        shAlpha = srcData.getPixelColour(shX, shY).getFloatAlpha();
                    
                    // Edge detection
                    float hlEdge = juce::jlimit(0.0f, 1.0f, (centerAlpha - hlAlpha));
                    float shEdge = juce::jlimit(0.0f, 1.0f, (centerAlpha - shAlpha));
                    
                    hlData.setPixelColour(x, y, highlightColour.withMultipliedAlpha(hlEdge));
                    shData.setPixelColour(x, y, shadowColour.withMultipliedAlpha(shEdge));
                }
            }
        }
        
        // Apply softness
        if (softness > 1)
        {
            applyStackBlur(highlight, softness);
            applyStackBlur(shadow, softness);
        }
        
        // Composite
        {
            juce::Graphics g(result);
            g.drawImageAt(shadow, 0, 0);
            g.drawImageAt(highlight, 0, 0);
        }
        
        return result;
    }
    
    //==========================================================================
    // VIGNETTE
    //==========================================================================
    
    /**
     * @brief Apply a vignette effect to an image
     * @param img Image to modify (in place)
     * @param amount Vignette strength (0-1)
     * @param softness Edge softness (0-1)
     */
    static void applyVignette(juce::Image& img, float amount = 0.5f, float softness = 0.5f)
    {
        if (!img.isValid())
            return;
            
        const int w = img.getWidth();
        const int h = img.getHeight();
        const float centerX = w * 0.5f;
        const float centerY = h * 0.5f;
        const float maxDist = std::sqrt(centerX * centerX + centerY * centerY);
        const float innerRadius = (1.0f - softness) * 0.5f;
        
        juce::Image::BitmapData data(img, juce::Image::BitmapData::readWrite);
        
        for (int y = 0; y < h; ++y)
        {
            for (int x = 0; x < w; ++x)
            {
                float dx = (x - centerX) / centerX;
                float dy = (y - centerY) / centerY;
                float dist = std::sqrt(dx * dx + dy * dy);
                
                // Calculate vignette factor
                float vignette = juce::jlimit(0.0f, 1.0f, 
                    (dist - innerRadius) / (1.0f - innerRadius));
                vignette = 1.0f - (vignette * amount);
                
                auto pixel = data.getPixelColour(x, y);
                data.setPixelColour(x, y, pixel.withMultipliedBrightness(vignette));
            }
        }
    }
    
    //==========================================================================
    // COLOUR ADJUSTMENTS
    //==========================================================================
    
    /**
     * @brief Adjust brightness of an image
     * @param img Image to modify
     * @param brightness Brightness adjustment (-1 to 1)
     */
    static void adjustBrightness(juce::Image& img, float brightness)
    {
        if (!img.isValid() || brightness == 0.0f)
            return;
            
        juce::Image::BitmapData data(img, juce::Image::BitmapData::readWrite);
        const int w = img.getWidth();
        const int h = img.getHeight();
        
        for (int y = 0; y < h; ++y)
        {
            for (int x = 0; x < w; ++x)
            {
                auto pixel = data.getPixelColour(x, y);
                if (brightness > 0)
                    pixel = pixel.brighter(brightness);
                else
                    pixel = pixel.darker(-brightness);
                data.setPixelColour(x, y, pixel);
            }
        }
    }
    
    /**
     * @brief Adjust contrast of an image
     * @param img Image to modify
     * @param contrast Contrast adjustment (0.5 = half, 2.0 = double)
     */
    static void adjustContrast(juce::Image& img, float contrast)
    {
        if (!img.isValid() || contrast == 1.0f)
            return;
            
        juce::Image::BitmapData data(img, juce::Image::BitmapData::readWrite);
        const int w = img.getWidth();
        const int h = img.getHeight();
        
        for (int y = 0; y < h; ++y)
        {
            for (int x = 0; x < w; ++x)
            {
                auto pixel = data.getPixelColour(x, y);
                
                float r = pixel.getFloatRed();
                float g = pixel.getFloatGreen();
                float b = pixel.getFloatBlue();
                
                // Apply contrast
                r = juce::jlimit(0.0f, 1.0f, (r - 0.5f) * contrast + 0.5f);
                g = juce::jlimit(0.0f, 1.0f, (g - 0.5f) * contrast + 0.5f);
                b = juce::jlimit(0.0f, 1.0f, (b - 0.5f) * contrast + 0.5f);
                
                data.setPixelColour(x, y, juce::Colour::fromFloatRGBA(r, g, b, pixel.getFloatAlpha()));
            }
        }
    }
    
    /**
     * @brief Apply colour overlay with blend mode
     * @param img Image to modify
     * @param overlayColour Colour to overlay
     * @param opacity Overlay opacity (0-1)
     * @param blendMode The blend mode to use for the overlay
     */
    static void applyColourOverlay(juce::Image& img, juce::Colour overlayColour, float opacity, 
                                   BlendMode blendMode = BlendMode::Normal)
    {
        if (!img.isValid() || opacity <= 0.0f)
            return;
            
        juce::Image::BitmapData data(img, juce::Image::BitmapData::readWrite);
        const int w = img.getWidth();
        const int h = img.getHeight();
        
        for (int y = 0; y < h; ++y)
        {
            for (int x = 0; x < w; ++x)
            {
                auto pixel = data.getPixelColour(x, y);
                if (pixel.getAlpha() == 0)
                    continue;  // Skip fully transparent pixels
                
                const float pixelAlpha = pixel.getFloatAlpha();
                const float effectiveOpacity = pixelAlpha * opacity;
                
                // Get base and overlay RGB
                const float baseR = pixel.getFloatRed();
                const float baseG = pixel.getFloatGreen();
                const float baseB = pixel.getFloatBlue();
                const float overlayR = overlayColour.getFloatRed();
                const float overlayG = overlayColour.getFloatGreen();
                const float overlayB = overlayColour.getFloatBlue();
                
                // Calculate blended RGB based on blend mode
                float blendedR, blendedG, blendedB;
                
                switch (blendMode)
                {
                    case BlendMode::Multiply:
                        blendedR = baseR * overlayR;
                        blendedG = baseG * overlayG;
                        blendedB = baseB * overlayB;
                        break;
                        
                    case BlendMode::Screen:
                        blendedR = 1.0f - (1.0f - baseR) * (1.0f - overlayR);
                        blendedG = 1.0f - (1.0f - baseG) * (1.0f - overlayG);
                        blendedB = 1.0f - (1.0f - baseB) * (1.0f - overlayB);
                        break;
                        
                    case BlendMode::Overlay:
                        blendedR = (baseR < 0.5f) ? (2.0f * baseR * overlayR) : (1.0f - 2.0f * (1.0f - baseR) * (1.0f - overlayR));
                        blendedG = (baseG < 0.5f) ? (2.0f * baseG * overlayG) : (1.0f - 2.0f * (1.0f - baseG) * (1.0f - overlayG));
                        blendedB = (baseB < 0.5f) ? (2.0f * baseB * overlayB) : (1.0f - 2.0f * (1.0f - baseB) * (1.0f - overlayB));
                        break;
                        
                    case BlendMode::Add:
                        blendedR = juce::jmin(1.0f, baseR + overlayR);
                        blendedG = juce::jmin(1.0f, baseG + overlayG);
                        blendedB = juce::jmin(1.0f, baseB + overlayB);
                        break;
                        
                    case BlendMode::SoftLight:
                        blendedR = (overlayR < 0.5f) ? (2.0f * baseR * overlayR + baseR * baseR * (1.0f - 2.0f * overlayR))
                                                     : (std::sqrt(baseR) * (2.0f * overlayR - 1.0f) + 2.0f * baseR * (1.0f - overlayR));
                        blendedG = (overlayG < 0.5f) ? (2.0f * baseG * overlayG + baseG * baseG * (1.0f - 2.0f * overlayG))
                                                     : (std::sqrt(baseG) * (2.0f * overlayG - 1.0f) + 2.0f * baseG * (1.0f - overlayG));
                        blendedB = (overlayB < 0.5f) ? (2.0f * baseB * overlayB + baseB * baseB * (1.0f - 2.0f * overlayB))
                                                     : (std::sqrt(baseB) * (2.0f * overlayB - 1.0f) + 2.0f * baseB * (1.0f - overlayB));
                        break;
                        
                    case BlendMode::HardLight:
                        blendedR = (overlayR < 0.5f) ? (2.0f * baseR * overlayR) : (1.0f - 2.0f * (1.0f - baseR) * (1.0f - overlayR));
                        blendedG = (overlayG < 0.5f) ? (2.0f * baseG * overlayG) : (1.0f - 2.0f * (1.0f - baseG) * (1.0f - overlayG));
                        blendedB = (overlayB < 0.5f) ? (2.0f * baseB * overlayB) : (1.0f - 2.0f * (1.0f - baseB) * (1.0f - overlayB));
                        break;
                        
                    case BlendMode::Normal:
                    default:
                        blendedR = overlayR;
                        blendedG = overlayG;
                        blendedB = overlayB;
                        break;
                }
                
                // Lerp from base to blended based on effective opacity
                const float finalR = baseR + (blendedR - baseR) * effectiveOpacity;
                const float finalG = baseG + (blendedG - baseG) * effectiveOpacity;
                const float finalB = baseB + (blendedB - baseB) * effectiveOpacity;
                
                data.setPixelColour(x, y, juce::Colour::fromFloatRGBA(
                    juce::jlimit(0.0f, 1.0f, finalR),
                    juce::jlimit(0.0f, 1.0f, finalG),
                    juce::jlimit(0.0f, 1.0f, finalB),
                    pixelAlpha));
            }
        }
    }
    
    //==========================================================================
    // NOISE
    //==========================================================================
    
    /**
     * @brief Add noise to an image
     * @param img Image to modify
     * @param amount Noise amount (0-1)
     * @param monochrome True for greyscale noise, false for colour noise
     */
    static void addNoise(juce::Image& img, float amount, bool monochrome = true)
    {
        if (!img.isValid() || amount <= 0.0f)
            return;
            
        juce::Random random;
        juce::Image::BitmapData data(img, juce::Image::BitmapData::readWrite);
        const int w = img.getWidth();
        const int h = img.getHeight();
        
        for (int y = 0; y < h; ++y)
        {
            for (int x = 0; x < w; ++x)
            {
                auto pixel = data.getPixelColour(x, y);
                
                if (monochrome)
                {
                    float noise = (random.nextFloat() - 0.5f) * 2.0f * amount;
                    float r = juce::jlimit(0.0f, 1.0f, pixel.getFloatRed() + noise);
                    float g = juce::jlimit(0.0f, 1.0f, pixel.getFloatGreen() + noise);
                    float b = juce::jlimit(0.0f, 1.0f, pixel.getFloatBlue() + noise);
                    data.setPixelColour(x, y, juce::Colour::fromFloatRGBA(r, g, b, pixel.getFloatAlpha()));
                }
                else
                {
                    float noiseR = (random.nextFloat() - 0.5f) * 2.0f * amount;
                    float noiseG = (random.nextFloat() - 0.5f) * 2.0f * amount;
                    float noiseB = (random.nextFloat() - 0.5f) * 2.0f * amount;
                    float r = juce::jlimit(0.0f, 1.0f, pixel.getFloatRed() + noiseR);
                    float g = juce::jlimit(0.0f, 1.0f, pixel.getFloatGreen() + noiseG);
                    float b = juce::jlimit(0.0f, 1.0f, pixel.getFloatBlue() + noiseB);
                    data.setPixelColour(x, y, juce::Colour::fromFloatRGBA(r, g, b, pixel.getFloatAlpha()));
                }
            }
        }
    }

private:
    //==========================================================================
    // STACK BLUR IMPLEMENTATIONS (private, optimized)
    //==========================================================================
    
    static void applyStackBlurARGB(juce::Image& img, unsigned int radius)
    {
        const unsigned int w = static_cast<unsigned int>(img.getWidth());
        const unsigned int h = static_cast<unsigned int>(img.getHeight());

        juce::Image::BitmapData data(img, juce::Image::BitmapData::readWrite);

        unsigned char stack[(254 * 2 + 1) * 4];
        unsigned int x, y, xp, yp, i, sp, stack_start;

        unsigned char* stack_ptr = nullptr;
        unsigned char* src_ptr = nullptr;
        unsigned char* dst_ptr = nullptr;

        unsigned long sum_r, sum_g, sum_b, sum_a;
        unsigned long sum_in_r, sum_in_g, sum_in_b, sum_in_a;
        unsigned long sum_out_r, sum_out_g, sum_out_b, sum_out_a;

        unsigned int wm = w - 1;
        unsigned int hm = h - 1;
        unsigned int w4 = static_cast<unsigned int>(data.lineStride);
        unsigned int div = radius * 2 + 1;
        unsigned int mul_sum = StackBlurTables::mul[radius];
        unsigned char shr_sum = StackBlurTables::shr[radius];

        // Horizontal pass
        for (y = 0; y < h; ++y)
        {
            sum_r = sum_g = sum_b = sum_a = 0;
            sum_in_r = sum_in_g = sum_in_b = sum_in_a = 0;
            sum_out_r = sum_out_g = sum_out_b = sum_out_a = 0;

            src_ptr = data.getLinePointer(static_cast<int>(y));

            for (i = 0; i <= radius; ++i)
            {
                stack_ptr = &stack[4 * i];
                stack_ptr[0] = src_ptr[0];
                stack_ptr[1] = src_ptr[1];
                stack_ptr[2] = src_ptr[2];
                stack_ptr[3] = src_ptr[3];
                sum_r += src_ptr[0] * (i + 1);
                sum_g += src_ptr[1] * (i + 1);
                sum_b += src_ptr[2] * (i + 1);
                sum_a += src_ptr[3] * (i + 1);
                sum_out_r += src_ptr[0];
                sum_out_g += src_ptr[1];
                sum_out_b += src_ptr[2];
                sum_out_a += src_ptr[3];
            }

            for (i = 1; i <= radius; ++i)
            {
                if (i <= wm)
                    src_ptr += 4;

                stack_ptr = &stack[4 * (i + radius)];
                stack_ptr[0] = src_ptr[0];
                stack_ptr[1] = src_ptr[1];
                stack_ptr[2] = src_ptr[2];
                stack_ptr[3] = src_ptr[3];
                sum_r += src_ptr[0] * (radius + 1 - i);
                sum_g += src_ptr[1] * (radius + 1 - i);
                sum_b += src_ptr[2] * (radius + 1 - i);
                sum_a += src_ptr[3] * (radius + 1 - i);
                sum_in_r += src_ptr[0];
                sum_in_g += src_ptr[1];
                sum_in_b += src_ptr[2];
                sum_in_a += src_ptr[3];
            }

            sp = radius;
            xp = radius;
            if (xp > wm) xp = wm;

            src_ptr = data.getLinePointer(static_cast<int>(y)) + data.pixelStride * xp;
            dst_ptr = data.getLinePointer(static_cast<int>(y));

            for (x = 0; x < w; ++x)
            {
                dst_ptr[0] = static_cast<unsigned char>((sum_r * mul_sum) >> shr_sum);
                dst_ptr[1] = static_cast<unsigned char>((sum_g * mul_sum) >> shr_sum);
                dst_ptr[2] = static_cast<unsigned char>((sum_b * mul_sum) >> shr_sum);
                dst_ptr[3] = static_cast<unsigned char>((sum_a * mul_sum) >> shr_sum);
                dst_ptr += 4;

                sum_r -= sum_out_r;
                sum_g -= sum_out_g;
                sum_b -= sum_out_b;
                sum_a -= sum_out_a;

                stack_start = sp + div - radius;
                if (stack_start >= div) stack_start -= div;

                stack_ptr = &stack[4 * stack_start];

                sum_out_r -= stack_ptr[0];
                sum_out_g -= stack_ptr[1];
                sum_out_b -= stack_ptr[2];
                sum_out_a -= stack_ptr[3];

                if (xp < wm)
                {
                    src_ptr += 4;
                    ++xp;
                }

                stack_ptr[0] = src_ptr[0];
                stack_ptr[1] = src_ptr[1];
                stack_ptr[2] = src_ptr[2];
                stack_ptr[3] = src_ptr[3];

                sum_in_r += src_ptr[0];
                sum_in_g += src_ptr[1];
                sum_in_b += src_ptr[2];
                sum_in_a += src_ptr[3];
                sum_r += sum_in_r;
                sum_g += sum_in_g;
                sum_b += sum_in_b;
                sum_a += sum_in_a;

                ++sp;
                if (sp >= div) sp = 0;

                stack_ptr = &stack[sp * 4];

                sum_out_r += stack_ptr[0];
                sum_out_g += stack_ptr[1];
                sum_out_b += stack_ptr[2];
                sum_out_a += stack_ptr[3];
                sum_in_r -= stack_ptr[0];
                sum_in_g -= stack_ptr[1];
                sum_in_b -= stack_ptr[2];
                sum_in_a -= stack_ptr[3];
            }
        }

        // Vertical pass
        for (x = 0; x < w; ++x)
        {
            sum_r = sum_g = sum_b = sum_a = 0;
            sum_in_r = sum_in_g = sum_in_b = sum_in_a = 0;
            sum_out_r = sum_out_g = sum_out_b = sum_out_a = 0;

            src_ptr = data.getLinePointer(0) + data.pixelStride * x;

            for (i = 0; i <= radius; ++i)
            {
                stack_ptr = &stack[i * 4];
                stack_ptr[0] = src_ptr[0];
                stack_ptr[1] = src_ptr[1];
                stack_ptr[2] = src_ptr[2];
                stack_ptr[3] = src_ptr[3];
                sum_r += src_ptr[0] * (i + 1);
                sum_g += src_ptr[1] * (i + 1);
                sum_b += src_ptr[2] * (i + 1);
                sum_a += src_ptr[3] * (i + 1);
                sum_out_r += src_ptr[0];
                sum_out_g += src_ptr[1];
                sum_out_b += src_ptr[2];
                sum_out_a += src_ptr[3];
            }

            for (i = 1; i <= radius; ++i)
            {
                if (i <= hm)
                    src_ptr += w4;

                stack_ptr = &stack[4 * (i + radius)];
                stack_ptr[0] = src_ptr[0];
                stack_ptr[1] = src_ptr[1];
                stack_ptr[2] = src_ptr[2];
                stack_ptr[3] = src_ptr[3];
                sum_r += src_ptr[0] * (radius + 1 - i);
                sum_g += src_ptr[1] * (radius + 1 - i);
                sum_b += src_ptr[2] * (radius + 1 - i);
                sum_a += src_ptr[3] * (radius + 1 - i);
                sum_in_r += src_ptr[0];
                sum_in_g += src_ptr[1];
                sum_in_b += src_ptr[2];
                sum_in_a += src_ptr[3];
            }

            sp = radius;
            yp = radius;
            if (yp > hm) yp = hm;

            src_ptr = data.getLinePointer(static_cast<int>(yp)) + data.pixelStride * x;
            dst_ptr = data.getLinePointer(0) + data.pixelStride * x;

            for (y = 0; y < h; ++y)
            {
                dst_ptr[0] = static_cast<unsigned char>((sum_r * mul_sum) >> shr_sum);
                dst_ptr[1] = static_cast<unsigned char>((sum_g * mul_sum) >> shr_sum);
                dst_ptr[2] = static_cast<unsigned char>((sum_b * mul_sum) >> shr_sum);
                dst_ptr[3] = static_cast<unsigned char>((sum_a * mul_sum) >> shr_sum);
                dst_ptr += w4;

                sum_r -= sum_out_r;
                sum_g -= sum_out_g;
                sum_b -= sum_out_b;
                sum_a -= sum_out_a;

                stack_start = sp + div - radius;
                if (stack_start >= div) stack_start -= div;

                stack_ptr = &stack[4 * stack_start];

                sum_out_r -= stack_ptr[0];
                sum_out_g -= stack_ptr[1];
                sum_out_b -= stack_ptr[2];
                sum_out_a -= stack_ptr[3];

                if (yp < hm)
                {
                    src_ptr += w4;
                    ++yp;
                }

                stack_ptr[0] = src_ptr[0];
                stack_ptr[1] = src_ptr[1];
                stack_ptr[2] = src_ptr[2];
                stack_ptr[3] = src_ptr[3];

                sum_in_r += src_ptr[0];
                sum_in_g += src_ptr[1];
                sum_in_b += src_ptr[2];
                sum_in_a += src_ptr[3];
                sum_r += sum_in_r;
                sum_g += sum_in_g;
                sum_b += sum_in_b;
                sum_a += sum_in_a;

                ++sp;
                if (sp >= div) sp = 0;

                stack_ptr = &stack[sp * 4];

                sum_out_r += stack_ptr[0];
                sum_out_g += stack_ptr[1];
                sum_out_b += stack_ptr[2];
                sum_out_a += stack_ptr[3];
                sum_in_r -= stack_ptr[0];
                sum_in_g -= stack_ptr[1];
                sum_in_b -= stack_ptr[2];
                sum_in_a -= stack_ptr[3];
            }
        }
    }
    
    static void applyStackBlurRGB(juce::Image& img, unsigned int radius)
    {
        // Simplified RGB version - similar to ARGB but without alpha channel
        const unsigned int w = static_cast<unsigned int>(img.getWidth());
        const unsigned int h = static_cast<unsigned int>(img.getHeight());
        
        juce::Image::BitmapData data(img, juce::Image::BitmapData::readWrite);
        
        unsigned char stack[(254 * 2 + 1) * 3];
        unsigned int div = radius * 2 + 1;
        unsigned int mul_sum = StackBlurTables::mul[radius];
        unsigned char shr_sum = StackBlurTables::shr[radius];
        
        // Similar implementation for RGB...
        // For brevity, convert to ARGB, process, and convert back
        juce::Image argbImage = img.convertedToFormat(juce::Image::ARGB);
        applyStackBlurARGB(argbImage, radius);
        img = argbImage.convertedToFormat(juce::Image::RGB);
    }
    
    static void applyStackBlurBW(juce::Image& img, unsigned int radius)
    {
        const unsigned int w = static_cast<unsigned int>(img.getWidth());
        const unsigned int h = static_cast<unsigned int>(img.getHeight());
        
        juce::Image::BitmapData data(img, juce::Image::BitmapData::readWrite);
        
        unsigned char stack[254 * 2 + 1];
        unsigned int x, y, xp, yp, i, sp, stack_start;
        
        unsigned char* stack_ptr = nullptr;
        unsigned char* src_ptr = nullptr;
        unsigned char* dst_ptr = nullptr;
        
        unsigned long sum, sum_in, sum_out;
        
        unsigned int wm = w - 1;
        unsigned int hm = h - 1;
        unsigned int w1 = static_cast<unsigned int>(data.lineStride);
        unsigned int div = radius * 2 + 1;
        unsigned int mul_sum = StackBlurTables::mul[radius];
        unsigned char shr_sum = StackBlurTables::shr[radius];
        
        // Horizontal pass
        for (y = 0; y < h; ++y)
        {
            sum = sum_in = sum_out = 0;
            src_ptr = data.getLinePointer(static_cast<int>(y));
            
            for (i = 0; i <= radius; ++i)
            {
                stack_ptr = &stack[i];
                stack_ptr[0] = src_ptr[0];
                sum += src_ptr[0] * (i + 1);
                sum_out += src_ptr[0];
            }
            
            for (i = 1; i <= radius; ++i)
            {
                if (i <= wm) src_ptr += 1;
                stack_ptr = &stack[i + radius];
                stack_ptr[0] = src_ptr[0];
                sum += src_ptr[0] * (radius + 1 - i);
                sum_in += src_ptr[0];
            }
            
            sp = radius;
            xp = radius;
            if (xp > wm) xp = wm;
            
            src_ptr = data.getLinePointer(static_cast<int>(y)) + xp;
            dst_ptr = data.getLinePointer(static_cast<int>(y));
            
            for (x = 0; x < w; ++x)
            {
                dst_ptr[0] = static_cast<unsigned char>((sum * mul_sum) >> shr_sum);
                dst_ptr += 1;
                
                sum -= sum_out;
                
                stack_start = sp + div - radius;
                if (stack_start >= div) stack_start -= div;
                stack_ptr = &stack[stack_start];
                sum_out -= stack_ptr[0];
                
                if (xp < wm) { src_ptr += 1; ++xp; }
                stack_ptr[0] = src_ptr[0];
                sum_in += src_ptr[0];
                sum += sum_in;
                
                ++sp;
                if (sp >= div) sp = 0;
                stack_ptr = &stack[sp];
                sum_out += stack_ptr[0];
                sum_in -= stack_ptr[0];
            }
        }
        
        // Vertical pass
        for (x = 0; x < w; ++x)
        {
            sum = sum_in = sum_out = 0;
            src_ptr = data.getLinePointer(0) + x;
            
            for (i = 0; i <= radius; ++i)
            {
                stack_ptr = &stack[i];
                stack_ptr[0] = src_ptr[0];
                sum += src_ptr[0] * (i + 1);
                sum_out += src_ptr[0];
            }
            
            for (i = 1; i <= radius; ++i)
            {
                if (i <= hm) src_ptr += w1;
                stack_ptr = &stack[i + radius];
                stack_ptr[0] = src_ptr[0];
                sum += src_ptr[0] * (radius + 1 - i);
                sum_in += src_ptr[0];
            }
            
            sp = radius;
            yp = radius;
            if (yp > hm) yp = hm;
            
            src_ptr = data.getLinePointer(static_cast<int>(yp)) + x;
            dst_ptr = data.getLinePointer(0) + x;
            
            for (y = 0; y < h; ++y)
            {
                dst_ptr[0] = static_cast<unsigned char>((sum * mul_sum) >> shr_sum);
                dst_ptr += w1;
                
                sum -= sum_out;
                
                stack_start = sp + div - radius;
                if (stack_start >= div) stack_start -= div;
                stack_ptr = &stack[stack_start];
                sum_out -= stack_ptr[0];
                
                if (yp < hm) { src_ptr += w1; ++yp; }
                stack_ptr[0] = src_ptr[0];
                sum_in += src_ptr[0];
                sum += sum_in;
                
                ++sp;
                if (sp >= div) sp = 0;
                stack_ptr = &stack[sp];
                sum_out += stack_ptr[0];
                sum_in -= stack_ptr[0];
            }
        }
    }
};

//==============================================================================
/**
 * @brief Layer effects configuration struct
 * Defines all effects that can be applied to a layer
 */
struct LayerEffects
{
    // Drop Shadow
    struct DropShadow
    {
        bool enabled { false };
        juce::Colour colour { juce::Colours::black.withAlpha(0.5f) };
        juce::Point<float> offset { 4.0f, 4.0f };
        int blurRadius { 8 };
        float spread { 0.0f };
        
        bool operator==(const DropShadow& other) const noexcept
        {
            return enabled == other.enabled && colour == other.colour 
                && offset == other.offset && blurRadius == other.blurRadius 
                && spread == other.spread;
        }
    } dropShadow;
    
    // Inner Shadow
    struct InnerShadow
    {
        bool enabled { false };
        juce::Colour colour { juce::Colours::black.withAlpha(0.5f) };
        juce::Point<float> offset { 2.0f, 2.0f };
        int blurRadius { 4 };
        float choke { 0.0f };
        
        bool operator==(const InnerShadow& other) const noexcept
        {
            return enabled == other.enabled && colour == other.colour 
                && offset == other.offset && blurRadius == other.blurRadius 
                && choke == other.choke;
        }
    } innerShadow;
    
    // Outer Glow
    struct OuterGlow
    {
        bool enabled { false };
        juce::Colour colour { juce::Colours::white.withAlpha(0.75f) };
        int blurRadius { 10 };
        float spread { 0.5f };
        
        bool operator==(const OuterGlow& other) const noexcept
        {
            return enabled == other.enabled && colour == other.colour 
                && blurRadius == other.blurRadius && spread == other.spread;
        }
    } outerGlow;
    
    // Inner Glow
    struct InnerGlow
    {
        bool enabled { false };
        juce::Colour colour { juce::Colours::white.withAlpha(0.5f) };
        int blurRadius { 5 };
        float intensity { 0.7f };
        
        bool operator==(const InnerGlow& other) const noexcept
        {
            return enabled == other.enabled && colour == other.colour 
                && blurRadius == other.blurRadius && intensity == other.intensity;
        }
    } innerGlow;
    
    // Bevel & Emboss
    struct BevelEmboss
    {
        bool enabled { false };
        float depth { 3.0f };
        float lightAngle { juce::MathConstants<float>::pi * 0.75f }; // 135 degrees (top-left)
        juce::Colour highlightColour { juce::Colours::white.withAlpha(0.5f) };
        juce::Colour shadowColour { juce::Colours::black.withAlpha(0.5f) };
        int softness { 2 };
        
        bool operator==(const BevelEmboss& other) const noexcept
        {
            return enabled == other.enabled && depth == other.depth 
                && lightAngle == other.lightAngle && highlightColour == other.highlightColour 
                && shadowColour == other.shadowColour && softness == other.softness;
        }
    } bevelEmboss;
    
    // Colour Overlay
    struct ColourOverlay
    {
        bool enabled { false };
        juce::Colour colour { juce::Colours::red };
        float opacity { 0.5f };
        BlendMode blendMode { BlendMode::Normal };
        
        bool operator==(const ColourOverlay& other) const noexcept
        {
            return enabled == other.enabled && colour == other.colour 
                && opacity == other.opacity && blendMode == other.blendMode;
        }
    } colourOverlay;
    
    // Gaussian Blur (layer blur, not shadow blur)
    struct GaussianBlur
    {
        bool enabled { false };
        int radius { 5 };
        
        bool operator==(const GaussianBlur& other) const noexcept
        {
            return enabled == other.enabled && radius == other.radius;
        }
    } gaussianBlur;
    
    // Check if any effect is enabled
    [[nodiscard]] bool hasAnyEffect() const noexcept
    {
        return dropShadow.enabled || innerShadow.enabled || outerGlow.enabled 
            || innerGlow.enabled || bevelEmboss.enabled || colourOverlay.enabled
            || gaussianBlur.enabled;
    }
    
    bool operator==(const LayerEffects& other) const noexcept
    {
        return dropShadow == other.dropShadow && innerShadow == other.innerShadow
            && outerGlow == other.outerGlow && innerGlow == other.innerGlow
            && bevelEmboss == other.bevelEmboss && colourOverlay == other.colourOverlay
            && gaussianBlur == other.gaussianBlur;
    }
    
    // Serialization helpers
    [[nodiscard]] juce::ValueTree toValueTree() const
    {
        juce::ValueTree tree("Effects");
        
        // Drop Shadow
        if (dropShadow.enabled)
        {
            juce::ValueTree ds("DropShadow");
            ds.setProperty("Enabled", dropShadow.enabled, nullptr);
            ds.setProperty("Colour", dropShadow.colour.toString(), nullptr);
            ds.setProperty("OffsetX", dropShadow.offset.x, nullptr);
            ds.setProperty("OffsetY", dropShadow.offset.y, nullptr);
            ds.setProperty("BlurRadius", dropShadow.blurRadius, nullptr);
            ds.setProperty("Spread", dropShadow.spread, nullptr);
            tree.addChild(ds, -1, nullptr);
        }
        
        // Outer Glow
        if (outerGlow.enabled)
        {
            juce::ValueTree og("OuterGlow");
            og.setProperty("Enabled", outerGlow.enabled, nullptr);
            og.setProperty("Colour", outerGlow.colour.toString(), nullptr);
            og.setProperty("BlurRadius", outerGlow.blurRadius, nullptr);
            og.setProperty("Spread", outerGlow.spread, nullptr);
            tree.addChild(og, -1, nullptr);
        }
        
        // Inner Glow
        if (innerGlow.enabled)
        {
            juce::ValueTree ig("InnerGlow");
            ig.setProperty("Enabled", innerGlow.enabled, nullptr);
            ig.setProperty("Colour", innerGlow.colour.toString(), nullptr);
            ig.setProperty("BlurRadius", innerGlow.blurRadius, nullptr);
            ig.setProperty("Intensity", innerGlow.intensity, nullptr);
            tree.addChild(ig, -1, nullptr);
        }
        
        // Bevel & Emboss
        if (bevelEmboss.enabled)
        {
            juce::ValueTree be("BevelEmboss");
            be.setProperty("Enabled", bevelEmboss.enabled, nullptr);
            be.setProperty("Depth", bevelEmboss.depth, nullptr);
            be.setProperty("LightAngle", bevelEmboss.lightAngle, nullptr);
            be.setProperty("HighlightColour", bevelEmboss.highlightColour.toString(), nullptr);
            be.setProperty("ShadowColour", bevelEmboss.shadowColour.toString(), nullptr);
            be.setProperty("Softness", bevelEmboss.softness, nullptr);
            tree.addChild(be, -1, nullptr);
        }
        
        // Colour Overlay
        if (colourOverlay.enabled)
        {
            juce::ValueTree co("ColourOverlay");
            co.setProperty("Enabled", colourOverlay.enabled, nullptr);
            co.setProperty("Colour", colourOverlay.colour.toString(), nullptr);
            co.setProperty("Opacity", colourOverlay.opacity, nullptr);
            co.setProperty("BlendMode", static_cast<int>(colourOverlay.blendMode), nullptr);
            tree.addChild(co, -1, nullptr);
        }
        
        // Gaussian Blur
        if (gaussianBlur.enabled)
        {
            juce::ValueTree gb("GaussianBlur");
            gb.setProperty("Enabled", gaussianBlur.enabled, nullptr);
            gb.setProperty("Radius", gaussianBlur.radius, nullptr);
            tree.addChild(gb, -1, nullptr);
        }
        
        return tree;
    }
    
    void fromValueTree(const juce::ValueTree& tree)
    {
        auto ds = tree.getChildWithName("DropShadow");
        if (ds.isValid())
        {
            dropShadow.enabled = ds.getProperty("Enabled", false);
            dropShadow.colour = juce::Colour::fromString(ds.getProperty("Colour", "ff000000").toString());
            dropShadow.offset.x = ds.getProperty("OffsetX", 4.0f);
            dropShadow.offset.y = ds.getProperty("OffsetY", 4.0f);
            dropShadow.blurRadius = ds.getProperty("BlurRadius", 8);
            dropShadow.spread = ds.getProperty("Spread", 0.0f);
        }
        
        auto og = tree.getChildWithName("OuterGlow");
        if (og.isValid())
        {
            outerGlow.enabled = og.getProperty("Enabled", false);
            outerGlow.colour = juce::Colour::fromString(og.getProperty("Colour", "ffffffff").toString());
            outerGlow.blurRadius = og.getProperty("BlurRadius", 10);
            outerGlow.spread = og.getProperty("Spread", 0.5f);
        }
        
        auto ig = tree.getChildWithName("InnerGlow");
        if (ig.isValid())
        {
            innerGlow.enabled = ig.getProperty("Enabled", false);
            innerGlow.colour = juce::Colour::fromString(ig.getProperty("Colour", "ffffffff").toString());
            innerGlow.blurRadius = ig.getProperty("BlurRadius", 5);
            innerGlow.intensity = ig.getProperty("Intensity", 0.7f);
        }
        
        auto be = tree.getChildWithName("BevelEmboss");
        if (be.isValid())
        {
            bevelEmboss.enabled = be.getProperty("Enabled", false);
            bevelEmboss.depth = be.getProperty("Depth", 3.0f);
            bevelEmboss.lightAngle = be.getProperty("LightAngle", juce::MathConstants<float>::pi * 0.75f);
            bevelEmboss.highlightColour = juce::Colour::fromString(be.getProperty("HighlightColour", "80ffffff").toString());
            bevelEmboss.shadowColour = juce::Colour::fromString(be.getProperty("ShadowColour", "80000000").toString());
            bevelEmboss.softness = be.getProperty("Softness", 2);
        }
        
        auto co = tree.getChildWithName("ColourOverlay");
        if (co.isValid())
        {
            colourOverlay.enabled = co.getProperty("Enabled", false);
            colourOverlay.colour = juce::Colour::fromString(co.getProperty("Colour", "ffff0000").toString());
            colourOverlay.opacity = co.getProperty("Opacity", 0.5f);
            colourOverlay.blendMode = static_cast<BlendMode>(static_cast<int>(co.getProperty("BlendMode", 0)));
        }
        
        auto gb = tree.getChildWithName("GaussianBlur");
        if (gb.isValid())
        {
            gaussianBlur.enabled = gb.getProperty("Enabled", false);
            gaussianBlur.radius = gb.getProperty("Radius", 5);
        }
    }
};

//==============================================================================
/**
 * @brief Blend mode utilities for compositing layers and effects
 */
namespace BlendModes
{
    /**
     * @brief Apply blend mode between two pixel colours
     * 
     * Implements Photoshop-style blend modes with proper alpha handling:
     * - Blend modes affect only the RGB channels where both layers have content
     * - The blend result is then alpha-composited onto the base
     * 
     * For most blend modes (Multiply, Screen, etc.), blending against a 
     * transparent base effectively makes those pixels invisible since
     * there's no content to blend with.
     */
    inline juce::Colour blendPixels(juce::Colour base, juce::Colour blend, BlendMode mode)
    {
        const float baseA = base.getFloatAlpha();
        const float blendA = blend.getFloatAlpha();
        
        // If blend layer pixel is fully transparent, return base unchanged
        if (blendA <= 0.0f)
            return base;
        
        // For Normal blend mode, if base is transparent, just return blend
        // For other blend modes, we need base content to blend with
        if (baseA <= 0.0f)
        {
            if (mode == BlendMode::Normal || mode == BlendMode::Add || mode == BlendMode::Screen)
            {
                // These modes can add content even over transparency
                return blend;
            }
            else
            {
                // Multiply, Overlay, SoftLight, HardLight need base content
                // With no base, the result is transparent
                return juce::Colour(0);
            }
        }
        
        const float baseR = base.getFloatRed();
        const float baseG = base.getFloatGreen();
        const float baseB = base.getFloatBlue();
        
        const float blendR = blend.getFloatRed();
        const float blendG = blend.getFloatGreen();
        const float blendB = blend.getFloatBlue();
        
        // Calculate the blend mode result for RGB only
        // This treats both pixels as if they were fully opaque
        float blendedR, blendedG, blendedB;
        
        switch (mode)
        {
            case BlendMode::Multiply:
                blendedR = baseR * blendR;
                blendedG = baseG * blendG;
                blendedB = baseB * blendB;
                break;
                
            case BlendMode::Screen:
                blendedR = 1.0f - (1.0f - baseR) * (1.0f - blendR);
                blendedG = 1.0f - (1.0f - baseG) * (1.0f - blendG);
                blendedB = 1.0f - (1.0f - baseB) * (1.0f - blendB);
                break;
                
            case BlendMode::Overlay:
                blendedR = (baseR < 0.5f) ? (2.0f * baseR * blendR) : (1.0f - 2.0f * (1.0f - baseR) * (1.0f - blendR));
                blendedG = (baseG < 0.5f) ? (2.0f * baseG * blendG) : (1.0f - 2.0f * (1.0f - baseG) * (1.0f - blendG));
                blendedB = (baseB < 0.5f) ? (2.0f * baseB * blendB) : (1.0f - 2.0f * (1.0f - baseB) * (1.0f - blendB));
                break;
                
            case BlendMode::Add:
                blendedR = juce::jmin(1.0f, baseR + blendR);
                blendedG = juce::jmin(1.0f, baseG + blendG);
                blendedB = juce::jmin(1.0f, baseB + blendB);
                break;
                
            case BlendMode::SoftLight:
                blendedR = (blendR < 0.5f) ? (2.0f * baseR * blendR + baseR * baseR * (1.0f - 2.0f * blendR))
                                           : (std::sqrt(baseR) * (2.0f * blendR - 1.0f) + 2.0f * baseR * (1.0f - blendR));
                blendedG = (blendG < 0.5f) ? (2.0f * baseG * blendG + baseG * baseG * (1.0f - 2.0f * blendG))
                                           : (std::sqrt(baseG) * (2.0f * blendG - 1.0f) + 2.0f * baseG * (1.0f - blendG));
                blendedB = (blendB < 0.5f) ? (2.0f * baseB * blendB + baseB * baseB * (1.0f - 2.0f * blendB))
                                           : (std::sqrt(baseB) * (2.0f * blendB - 1.0f) + 2.0f * baseB * (1.0f - blendB));
                break;
                
            case BlendMode::HardLight:
                blendedR = (blendR < 0.5f) ? (2.0f * baseR * blendR) : (1.0f - 2.0f * (1.0f - baseR) * (1.0f - blendR));
                blendedG = (blendG < 0.5f) ? (2.0f * baseG * blendG) : (1.0f - 2.0f * (1.0f - baseG) * (1.0f - blendG));
                blendedB = (blendB < 0.5f) ? (2.0f * baseB * blendB) : (1.0f - 2.0f * (1.0f - baseB) * (1.0f - blendB));
                break;
                
            case BlendMode::Normal:
            default:
                // Normal mode: blend color is used directly, no blend calculation
                blendedR = blendR;
                blendedG = blendG;
                blendedB = blendB;
                break;
        }
        
        // Photoshop-style blend mode compositing:
        // 1. blendedRGB is the blend mode result (calculated above)
        // 2. Lerp from base to blended based on blend layer's alpha
        // 3. Output alpha follows standard alpha compositing
        //
        // This matches how Photoshop applies blend modes with layer opacity
        
        const float outA = blendA + baseA * (1.0f - blendA);
        
        if (outA <= 0.0f)
            return juce::Colour(0);
        
        // Lerp from base color to blended color based on blend alpha
        // This gives the correct Photoshop-style result
        const float finalR = baseR + (blendedR - baseR) * blendA;
        const float finalG = baseG + (blendedG - baseG) * blendA;
        const float finalB = baseB + (blendedB - baseB) * blendA;
        
        return juce::Colour::fromFloatRGBA(
            juce::jlimit(0.0f, 1.0f, finalR),
            juce::jlimit(0.0f, 1.0f, finalG),
            juce::jlimit(0.0f, 1.0f, finalB),
            juce::jlimit(0.0f, 1.0f, outA));
    }
    
    /**
     * @brief Composite blend image onto base image using blend mode
     * @param base The base image to composite onto (modified in place)
     * @param blend The image to blend on top
     * @param x X offset for the blend image
     * @param y Y offset for the blend image
     * @param mode The blend mode to use
     * @param opacity Overall opacity for the blend layer (0-1)
     */
    inline void compositeWithBlendMode(juce::Image& base, const juce::Image& blend, 
                                       int x, int y, BlendMode mode, float opacity = 1.0f)
    {
        if (!base.isValid() || !blend.isValid() || opacity <= 0.0f)
            return;
        
        // For normal blend mode, just use standard alpha compositing
        if (mode == BlendMode::Normal)
        {
            juce::Graphics g(base);
            g.setOpacity(opacity);
            g.drawImageAt(blend, x, y);
            return;
        }
        
        const int blendW = blend.getWidth();
        const int blendH = blend.getHeight();
        const int baseW = base.getWidth();
        const int baseH = base.getHeight();
        
        // Fast path for ARGB images (multithreaded row chunks)
        if (base.getFormat() == juce::Image::ARGB && blend.getFormat() == juce::Image::ARGB)
        {
            juce::Image::BitmapData baseData(base, juce::Image::BitmapData::readWrite);
            juce::Image::BitmapData blendData(blend, juce::Image::BitmapData::readOnly);

            const int numThreads = std::max(1, (int)std::thread::hardware_concurrency());
            const int rowsPerThread = (blendH + numThreads - 1) / numThreads;

            std::vector<std::future<void>> futures;
            futures.reserve(numThreads);

            auto processRows = [=, &baseData, &blendData](int startY, int endY)
            {
                for (int by = startY; by < endY; ++by)
                {
                    int py = y + by;
                    if (py < 0 || py >= baseH) continue;

                    for (int bx = 0; bx < blendW; ++bx)
                    {
                        int px = x + bx;
                        if (px < 0 || px >= baseW) continue;

                        auto blendColour = blendData.getPixelColour(bx, by);

                        // Apply layer opacity to blend colour's alpha
                        if (opacity < 1.0f)
                            blendColour = blendColour.withMultipliedAlpha(opacity);

                        // Skip fully transparent pixels
                        if (blendColour.getAlpha() == 0)
                            continue;

                        auto baseColour = baseData.getPixelColour(px, py);
                        auto result = blendPixels(baseColour, blendColour, mode);
                       #if JUCE_DEBUG
                        if (bx == blendW / 2 && by == blendH / 2)
                        {
                            DBG("compositeWithBlendMode centre pixel base="
                                << (int) baseColour.getRed() << "," << (int) baseColour.getGreen() << "," << (int) baseColour.getBlue()
                                << " a=" << (int) baseColour.getAlpha()
                                << " blend="
                                << (int) blendColour.getRed() << "," << (int) blendColour.getGreen() << "," << (int) blendColour.getBlue()
                                << " a=" << (int) blendColour.getAlpha()
                                << " -> result="
                                << (int) result.getRed() << "," << (int) result.getGreen() << "," << (int) result.getBlue()
                                << " a=" << (int) result.getAlpha());
                        }
                       #endif
                        baseData.setPixelColour(px, py, result);
                    }
                }
            };

            for (int i = 0; i < numThreads; ++i)
            {
                const int startY = i * rowsPerThread;
                const int endY = std::min(startY + rowsPerThread, blendH);
                if (startY >= endY) break;
                futures.emplace_back(std::async(std::launch::async, processRows, startY, endY));
            }

            for (auto& f : futures) f.wait();
            return;
        }

        // Fallback for other formats (single threaded)
        juce::Image::BitmapData baseData(base, juce::Image::BitmapData::readWrite);
        juce::Image::BitmapData blendData(blend, juce::Image::BitmapData::readOnly);
        
        for (int by = 0; by < blendH; ++by)
        {
            int py = y + by;
            if (py < 0 || py >= baseH) continue;
            
            for (int bx = 0; bx < blendW; ++bx)
            {
                int px = x + bx;
                if (px < 0 || px >= baseW) continue;
                
                auto blendColour = blendData.getPixelColour(bx, by);
                
                // Apply layer opacity to blend colour's alpha
                if (opacity < 1.0f)
                    blendColour = blendColour.withMultipliedAlpha(opacity);
                
                // Skip fully transparent pixels
                if (blendColour.getAlpha() == 0)
                    continue;
                
                auto baseColour = baseData.getPixelColour(px, py);
                auto result = blendPixels(baseColour, blendColour, mode);
                baseData.setPixelColour(px, py, result);
            }
        }
    }
    
    //==========================================================================
    // CONIC GRADIENT
    //==========================================================================
    
    /**
     * @brief Create a conic (angular/sweep) gradient image
     * @param bounds The bounds to fill with the gradient
     * @param gradient The color gradient (positions 0-1 map to 0-360 degrees)
     * @param centerX Center X position (normalized 0-1)
     * @param centerY Center Y position (normalized 0-1)
     * @param startAngle Starting angle in radians (0 = right, positive = clockwise)
     * @return Image with conic gradient
     */
    [[nodiscard]] static juce::Image createConicGradient(const juce::Rectangle<int>& bounds,
                                                          const juce::ColourGradient& gradient,
                                                          float centerX = 0.5f,
                                                          float centerY = 0.5f,
                                                          float startAngle = -juce::MathConstants<float>::halfPi)
    {
        const int w = bounds.getWidth();
        const int h = bounds.getHeight();
        
        if (w <= 0 || h <= 0)
            return {};
        
        juce::Image result(juce::Image::ARGB, w, h, false);
        juce::Image::BitmapData destData(result, juce::Image::BitmapData::writeOnly);
        
        const float cx = w * centerX;
        const float cy = h * centerY;
        const float scale = 1.0f / (2.0f * juce::MathConstants<float>::pi);
        
        // Create lookup table for gradient colors
        juce::HeapBlock<juce::PixelARGB> lookup;
        const int numEntries = gradient.createLookupTable(juce::AffineTransform(), lookup);
        
        for (int y = 0; y < h; ++y)
        {
            for (int x = 0; x < w; ++x)
            {
                float angleRadians = std::atan2(static_cast<float>(y) - cy, 
                                                static_cast<float>(x) - cx);
                // Adjust for start angle and normalize to 0-1
                float a = scale * (angleRadians - startAngle + juce::MathConstants<float>::pi);
                a = std::fmod(a + 1.0f, 1.0f);  // Ensure 0-1 range
                
                int index = juce::roundToInt(a * static_cast<float>(numEntries - 1));
                index = juce::jlimit(0, numEntries - 1, index);
                
                destData.setPixelColour(x, y, lookup[index]);
            }
        }
        
        return result;
    }
    
    /**
     * @brief Fill a path with a conic gradient
     * @param g Graphics context
     * @param path Path to fill
     * @param gradient ColourGradient to use
     * @param bounds Bounds for the gradient
     * @param startAngle Starting angle in radians
     */
    static void fillPathWithConicGradient(juce::Graphics& g,
                                           const juce::Path& path,
                                           const juce::ColourGradient& gradient,
                                           const juce::Rectangle<float>& bounds,
                                           float startAngle = -juce::MathConstants<float>::halfPi)
    {
        auto intBounds = bounds.toNearestInt();
        if (intBounds.isEmpty())
            return;
        
        // Create the conic gradient image
        auto gradientImage = createConicGradient(
            intBounds,
            gradient,
            (gradient.point1.x - bounds.getX()) / bounds.getWidth(),
            (gradient.point1.y - bounds.getY()) / bounds.getHeight(),
            startAngle
        );
        
        // Use the path as a clip region
        g.saveState();
        g.reduceClipRegion(path);
        g.drawImageAt(gradientImage, intBounds.getX(), intBounds.getY());
        g.restoreState();
    }
};

} // namespace zaplab

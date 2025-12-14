/*
  ==============================================================================

    LayerStyle.h
    Created: 11 Dec 2025
    Author:  DevTools Refactored Layer System
    
    Description:
    Style properties for visual appearance of layers.
    Includes fill, stroke, shadow, and blend mode settings.

  ==============================================================================
*/

#pragma once

#include "LayerTypes.h"
#include "ImageEffects.h"  // For BlendMode

namespace zaplab
{

//==============================================================================
/**
 * @brief Style properties for visual appearance
 * 
 * Note: Size-dependent properties (strokeWidth, cornerRadius, shadow radius/offset)
 * are stored as RELATIVE values (0-1 range where 1.0 = reference dimension).
 * Use relativeToPixels() to convert to actual pixels for rendering.
 */
struct LayerStyle
{
    // Fill properties
    FillMode fillMode { FillMode::Solid };
    bool hasFill { true };                    ///< Whether to render fill
    juce::Colour fillColour { juce::Colours::grey };
    Gradient fillGradient;
    
    // Stroke properties  
    bool hasStroke { false };
    juce::Colour strokeColour { juce::Colours::white };
    float strokeWidth { 0.01f };              ///< Relative to reference dimension (0.01 = 1% of ref)
    StrokeAlignment strokeAlignment { StrokeAlignment::Center };  ///< Where stroke is drawn relative to path
    juce::PathStrokeType::JointStyle strokeJoin { juce::PathStrokeType::curved };
    juce::PathStrokeType::EndCapStyle strokeCap { juce::PathStrokeType::rounded };
    std::vector<float> strokeDashPattern;     ///< Dash pattern (e.g. {5, 5} for 5px dash, 5px gap)
    
    // Corner radius (for rectangles) - relative to reference dimension
    float cornerRadius { 0.0f };              ///< Relative (0.1 = 10% of ref dimension)
    float cornerRadiusTL { 0.0f };
    float cornerRadiusTR { 0.0f };
    float cornerRadiusBL { 0.0f };
    float cornerRadiusBR { 0.0f };
    bool useIndividualCorners { false };
    
    // Effects
    Shadow shadow;
    float opacity { 1.0f };
    
    // Blend mode for layer compositing
    BlendMode blendMode { BlendMode::Normal };
    
    //==========================================================================
    // Helper methods to get pixel values for rendering
    
    [[nodiscard]] float getStrokeWidthPixels(const juce::Rectangle<float>& bounds, SizingMode mode) const noexcept
    {
        return relativeToPixels(strokeWidth, bounds, mode);
    }
    
    [[nodiscard]] float getCornerRadiusPixels(const juce::Rectangle<float>& bounds, SizingMode mode) const noexcept
    {
        return relativeToPixels(cornerRadius, bounds, mode);
    }
    
    [[nodiscard]] float getCornerRadiusTLPixels(const juce::Rectangle<float>& bounds, SizingMode mode) const noexcept
    {
        return relativeToPixels(cornerRadiusTL, bounds, mode);
    }
    
    [[nodiscard]] float getCornerRadiusTRPixels(const juce::Rectangle<float>& bounds, SizingMode mode) const noexcept
    {
        return relativeToPixels(cornerRadiusTR, bounds, mode);
    }
    
    [[nodiscard]] float getCornerRadiusBLPixels(const juce::Rectangle<float>& bounds, SizingMode mode) const noexcept
    {
        return relativeToPixels(cornerRadiusBL, bounds, mode);
    }
    
    [[nodiscard]] float getCornerRadiusBRPixels(const juce::Rectangle<float>& bounds, SizingMode mode) const noexcept
    {
        return relativeToPixels(cornerRadiusBR, bounds, mode);
    }
    
    bool operator==(const LayerStyle& other) const noexcept
    {
        return fillMode == other.fillMode 
            && hasFill == other.hasFill
            && fillColour == other.fillColour 
            && hasStroke == other.hasStroke 
            && strokeColour == other.strokeColour 
            && strokeWidth == other.strokeWidth 
            && strokeAlignment == other.strokeAlignment
            && strokeJoin == other.strokeJoin
            && strokeCap == other.strokeCap
            && cornerRadius == other.cornerRadius 
            && opacity == other.opacity 
            && shadow == other.shadow
            && blendMode == other.blendMode;
    }
    
    bool operator!=(const LayerStyle& other) const noexcept { return !(*this == other); }
};

} // namespace zaplab

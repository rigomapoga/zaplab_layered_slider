/*
  ==============================================================================

    ShapeLayer.h
    Created: 30 Nov 2025
    Author:  DevTools Refactored Layer System
    
    Description:
    A shape layer that can render various geometric shapes with configurable
    properties like corner radius, stroke, fill, gradients, and arc parameters.

  ==============================================================================
*/

#pragma once

#include "Layer.h"

namespace zaplab
{

//==============================================================================
/**
 * @brief Arc configuration for arc shapes
 */
struct ArcConfig
{
    float startAngle { juce::degreesToRadians(-135.0f) };      // Start angle in radians (knob-style default)
    float endAngle { juce::degreesToRadians(135.0f) };         // End angle in radians (knob-style default)
    float thickness { 0.2f };                                   // Proportional thickness (0-1)
    bool connectToValue { false };                              // Whether arc animates with slider value
    bool startFromValue { false };                              // Whether start angle follows value
    bool endFromValue { true };                                 // Whether end angle follows value
    bool strokeOnly { false };                                  // Render as centerline stroke instead of filled pie
    
    // Calculate actual thickness in pixels
    [[nodiscard]] float getThicknessPixels(float radius) const noexcept
    {
        return radius * thickness;
    }
    
    // Get the angle range
    [[nodiscard]] float getAngleRange() const noexcept
    {
        return endAngle - startAngle;
    }
};

//==============================================================================
/**
 * @brief Polygon configuration for polygon shapes
 */
struct PolygonConfig
{
    int sides { 5 };                    // Number of sides (default to 5 for stars)
    float startAngle { 0.0f };          // Starting rotation angle
    float innerRadiusRatio { 0.5f };    // For star shapes (0.5 is a good default star)
    
    bool isValid() const noexcept { return sides >= 3; }
};

//==============================================================================
/**
 * @brief Line/Arrow configuration
 */
struct LineConfig
{
    float startX { 0.0f };       // Start point X (normalized 0-1)
    float startY { 0.5f };       // Start point Y (normalized 0-1)
    float endX { 1.0f };         // End point X (normalized 0-1)
    float endY { 0.5f };         // End point Y (normalized 0-1)
    bool hasArrowStart { false }; // Arrow at start
    bool hasArrowEnd { true };    // Arrow at end
    float arrowSize { 0.1f };     // Arrow size (0-1, relative to line length)
    float arrowAngle { 30.0f };   // Arrow angle in degrees
};

//==============================================================================
/**
 * @brief Shape layer - renders geometric shapes
 */
class ShapeLayer : public Layer
{
public:
    //==========================================================================
    // Construction
    
    explicit ShapeLayer(const juce::String& name = "Shape");
    ~ShapeLayer() override = default;
    
    //==========================================================================
    // Layer Type
    
    [[nodiscard]] LayerType getType() const noexcept override { return LayerType::Shape; }
    [[nodiscard]] juce::String getTypeName() const noexcept override { return "Shape"; }
    
    //==========================================================================
    // Shape Configuration
    
    void setShapeType(ShapeType type);
    [[nodiscard]] ShapeType getShapeType() const noexcept { return shapeType_; }
    
    // Corner radius (for rectangles)
    void setCornerRadius(float radius);
    void setCornerRadii(float tl, float tr, float bl, float br);
    
    // Arc configuration
    void setArcConfig(const ArcConfig& config);
    [[nodiscard]] const ArcConfig& getArcConfig() const noexcept { return arcConfig_; }
    [[nodiscard]] ArcConfig& getArcConfig() noexcept { return arcConfig_; }
    
    // Polygon configuration
    void setPolygonConfig(const PolygonConfig& config);
    [[nodiscard]] const PolygonConfig& getPolygonConfig() const noexcept { return polygonConfig_; }
    [[nodiscard]] PolygonConfig& getPolygonConfig() noexcept { return polygonConfig_; }
    
    // Line configuration
    void setLineConfig(const LineConfig& config) { lineConfig_ = config; }
    [[nodiscard]] const LineConfig& getLineConfig() const noexcept { return lineConfig_; }
    [[nodiscard]] LineConfig& getLineConfig() noexcept { return lineConfig_; }
    
    // Size ratio (proportional sizing within bounds)
    void setSizeRatio(juce::Point<float> ratio);
    [[nodiscard]] juce::Point<float> getSizeRatio() const noexcept { return sizeRatio_; }
    
    // Padding (shrinks the shape within its bounds)
    void setPadding(float padding);
    void setPaddingRatio(float ratio);  // 0-1, relative to min dimension
    [[nodiscard]] float getPadding() const noexcept { return padding_; }
    
    //==========================================================================
    // Path Access (for masking and hit testing)
    
    [[nodiscard]] juce::Path getShapePath() const;
    [[nodiscard]] juce::Path getShapePath(const juce::Rectangle<float>& bounds) const;
    
    //==========================================================================
    // Slider Integration
    
    void updateForSliderValue(float normalizedValue) override;
    
    //==========================================================================
    // Serialization
    
    [[nodiscard]] juce::ValueTree toValueTree() const override;
    void fromValueTree(const juce::ValueTree& tree) override;
    
    [[nodiscard]] json toJson() const override;
    void fromJson(const json& j) override;


protected:
    //==========================================================================
    // Rendering
    
    void renderLayer(juce::Graphics& g, const juce::Rectangle<float>& bounds) override;
    
protected:
    //==========================================================================
    // Override for arc-specific property handling
    
    [[nodiscard]] float getCurrentPropertyValue(ValueBindingProperty property) const override;
    void restoreOriginalPropertyValue(ValueBindingProperty property, float value) override;
    
private:
    //==========================================================================
    // Path Generation
    
    [[nodiscard]] juce::Path createRectanglePath(const juce::Rectangle<float>& bounds) const;
    [[nodiscard]] juce::Path createEllipsePath(const juce::Rectangle<float>& bounds) const;
    [[nodiscard]] juce::Path createArcPath(const juce::Rectangle<float>& bounds) const;
    [[nodiscard]] juce::Path createArcStrokePath(const juce::Rectangle<float>& bounds) const;
    [[nodiscard]] juce::Path createPolygonPath(const juce::Rectangle<float>& bounds) const;
    [[nodiscard]] juce::Path createStarPath(const juce::Rectangle<float>& bounds) const;
    [[nodiscard]] juce::Path createTrianglePath(const juce::Rectangle<float>& bounds) const;
    [[nodiscard]] juce::Path createLinePath(const juce::Rectangle<float>& bounds) const;
    
    //==========================================================================
    // Helper Methods
    
    [[nodiscard]] juce::Rectangle<float> getShapeBounds(const juce::Rectangle<float>& componentBounds) const;
    
    //==========================================================================
    // Members
    
    ShapeType shapeType_ { ShapeType::Rectangle };
    
    ArcConfig arcConfig_;
    PolygonConfig polygonConfig_;
    LineConfig lineConfig_;
    
    juce::Point<float> sizeRatio_ { 1.0f, 1.0f };
    float padding_ { 0.0f };
    float paddingRatio_ { 0.0f };
    
    JUCE_LEAK_DETECTOR(ShapeLayer)
};

} // namespace zaplab

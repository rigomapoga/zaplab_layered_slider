/*
  ==============================================================================

    ShapeLayer.cpp
    Created: 30 Nov 2025
    Author:  DevTools Refactored Layer System

  ==============================================================================
*/

#include "ShapeLayer.h"
#include "ImageEffects.h"

namespace zaplab
{

//==============================================================================
// Additional IDs for ShapeLayer serialization
namespace ShapeIDs
{
    static const juce::Identifier ShapeType { "shapeType" };
    static const juce::Identifier ArcConfig { "arcConfig" };
    static const juce::Identifier ArcStart { "startAngle" };
    static const juce::Identifier ArcEnd { "endAngle" };
    static const juce::Identifier ArcThickness { "thickness" };
    static const juce::Identifier ArcConnectToValue { "connectToValue" };
    static const juce::Identifier ArcStrokeOnly { "strokeOnly" };
    static const juce::Identifier PolygonConfig { "polygonConfig" };
    static const juce::Identifier PolygonSides { "sides" };
    static const juce::Identifier PolygonStartAngle { "startAngle" };
    static const juce::Identifier PolygonInnerRadius { "innerRadiusRatio" };
    static const juce::Identifier SizeRatioX { "sizeRatioX" };
    static const juce::Identifier SizeRatioY { "sizeRatioY" };
    static const juce::Identifier Padding { "padding" };
    static const juce::Identifier PaddingRatio { "paddingRatio" };
}

//==============================================================================
ShapeLayer::ShapeLayer(const juce::String& name)
    : Layer(name)
{
    // Enable both fill and stroke by default for shapes
    auto& style = getStyle();
    style.hasFill = true;
    style.hasStroke = true;
    style.strokeWidth = 0.02f; // Default stroke width (2%)
    style.strokeColour = juce::Colours::white; // Default stroke color
}

//==============================================================================
// Shape Configuration

void ShapeLayer::setShapeType(ShapeType type)
{
    if (shapeType_ != type)
    {
        auto oldType = shapeType_;
        shapeType_ = type;
        auto& style = getStyle();
        
        // When switching FROM Line to another shape, restore fill
        if (oldType == ShapeType::Line && type != ShapeType::Line)
        {
            // Restore solid fill if coming from Line (which had no fill)
            if (style.fillMode == FillMode::None)
            {
                style.fillMode = FillMode::Solid;
                style.hasFill = true;
            }
        }
        
        // Special handling for Line type: ensure stroke is enabled by default
        if (shapeType_ == ShapeType::Line)
        {
            if (!style.hasStroke)
            {
                style.hasStroke = true;
                // Ensure stroke width is visible if it was 0
                if (style.strokeWidth <= 0.0f)
                    style.strokeWidth = 0.02f; // Default to 2%
            }
            // Lines should not have a fill
            style.fillMode = FillMode::None;
            style.hasFill = false;
        }
        
        repaint();
    }
}

void ShapeLayer::setCornerRadius(float radius)
{
    auto& style = getStyle();
    style.cornerRadius = std::max(0.0f, radius);
    style.useIndividualCorners = false;
    repaint();
}

void ShapeLayer::setCornerRadii(float tl, float tr, float bl, float br)
{
    auto& style = getStyle();
    style.cornerRadiusTL = std::max(0.0f, tl);
    style.cornerRadiusTR = std::max(0.0f, tr);
    style.cornerRadiusBL = std::max(0.0f, bl);
    style.cornerRadiusBR = std::max(0.0f, br);
    style.useIndividualCorners = true;
    repaint();
}

void ShapeLayer::setArcConfig(const ArcConfig& config)
{
    arcConfig_ = config;
    repaint();
}

void ShapeLayer::setPolygonConfig(const PolygonConfig& config)
{
    if (config.isValid())
    {
        polygonConfig_ = config;
        repaint();
    }
}

void ShapeLayer::setSizeRatio(juce::Point<float> ratio)
{
    ratio.x = juce::jlimit(0.01f, 2.0f, ratio.x);
    ratio.y = juce::jlimit(0.01f, 2.0f, ratio.y);
    
    if (sizeRatio_ != ratio)
    {
        sizeRatio_ = ratio;
        repaint();
    }
}

void ShapeLayer::setPadding(float padding)
{
    padding = std::max(0.0f, padding);
    
    if (padding_ != padding)
    {
        padding_ = padding;
        paddingRatio_ = 0.0f;
        repaint();
    }
}

void ShapeLayer::setPaddingRatio(float ratio)
{
    ratio = juce::jlimit(0.0f, 0.5f, ratio);
    
    if (paddingRatio_ != ratio)
    {
        paddingRatio_ = ratio;
        repaint();
    }
}

//==============================================================================
// Path Access

juce::Path ShapeLayer::getShapePath() const
{
    return getShapePath(getLocalBounds().toFloat());
}

juce::Path ShapeLayer::getShapePath(const juce::Rectangle<float>& bounds) const
{
    const auto shapeBounds = getShapeBounds(bounds);
    
    switch (shapeType_)
    {
        case ShapeType::Rectangle:  return createRectanglePath(shapeBounds);
        case ShapeType::Ellipse:    return createEllipsePath(shapeBounds);
        case ShapeType::Arc:        return createArcPath(shapeBounds);
        case ShapeType::Polygon:    return createPolygonPath(shapeBounds);
        case ShapeType::Star:       return createStarPath(shapeBounds);
        case ShapeType::Triangle:   return createTrianglePath(shapeBounds);
        case ShapeType::Line:       return createLinePath(shapeBounds);
        case ShapeType::None:
        default:
            return {};
    }
}

//==============================================================================
// Slider Integration

void ShapeLayer::updateForSliderValue(float normalizedValue)
{
    Layer::updateForSliderValue(normalizedValue);
    
    // Handle arc-specific bindings
    if (shapeType_ == ShapeType::Arc)
    {
        for (const auto& binding : valueBindings_)
        {
            if (!binding.enabled)
                continue;
                
            if (binding.bipolar)
            {
                // Bipolar mode: arc starts from center and extends toward min or max
                // Center position is at normalized value 0.5
                const float centerAngle = (binding.valueAtMin + binding.valueAtMax) * 0.5f;
                
                if (normalizedValue >= 0.5f)
                {
                    // Value is above center: arc goes from center toward max
                    const float t = (normalizedValue - 0.5f) * 2.0f;  // Remap 0.5-1 to 0-1
                    const float endAngle = centerAngle + (binding.valueAtMax - centerAngle) * t;
                    
                    if (binding.property == ValueBindingProperty::ArcEnd)
                    {
                        arcConfig_.startAngle = centerAngle;
                        arcConfig_.endAngle = endAngle;
                    }
                }
                else
                {
                    // Value is below center: arc goes from min toward center
                    const float t = normalizedValue * 2.0f;  // Remap 0-0.5 to 0-1
                    const float startAngle = binding.valueAtMin + (centerAngle - binding.valueAtMin) * t;
                    
                    if (binding.property == ValueBindingProperty::ArcEnd)
                    {
                        arcConfig_.startAngle = startAngle;
                        arcConfig_.endAngle = centerAngle;
                    }
                }
            }
            else
            {
                // Standard mode: arc end follows value
                const float propertyValue = binding.computePropertyValue(normalizedValue);
                
                if (binding.property == ValueBindingProperty::ArcEnd)
                {
                    arcConfig_.endAngle = propertyValue;
                }
                else if (binding.property == ValueBindingProperty::ArcStart)
                {
                    arcConfig_.startAngle = propertyValue;
                }
                else if (binding.property == ValueBindingProperty::ArcThickness)
                {
                    // If stroke-only mode, control stroke width instead of arc thickness
                    if (arcConfig_.strokeOnly)
                    {
                        // Convert thickness value (0.01-1.0) to stroke width (percentage)
                        style_.strokeWidth = juce::jlimit(0.0f, 0.2f, propertyValue * 0.2f);
                    }
                    else
                    {
                        arcConfig_.thickness = juce::jlimit(0.01f, 1.0f, propertyValue);
                    }
                }
            }
        }
    }
    
    repaint();
}

//==============================================================================
// Serialization

juce::ValueTree ShapeLayer::toValueTree() const
{
    auto tree = Layer::toValueTree();
    
    tree.setProperty(ShapeIDs::ShapeType, static_cast<int>(shapeType_), nullptr);
    tree.setProperty(ShapeIDs::SizeRatioX, sizeRatio_.x, nullptr);
    tree.setProperty(ShapeIDs::SizeRatioY, sizeRatio_.y, nullptr);
    tree.setProperty(ShapeIDs::Padding, padding_, nullptr);
    tree.setProperty(ShapeIDs::PaddingRatio, paddingRatio_, nullptr);
    
    // Arc config
    juce::ValueTree arcTree(ShapeIDs::ArcConfig);
    arcTree.setProperty(ShapeIDs::ArcStart, arcConfig_.startAngle, nullptr);
    arcTree.setProperty(ShapeIDs::ArcEnd, arcConfig_.endAngle, nullptr);
    arcTree.setProperty(ShapeIDs::ArcThickness, arcConfig_.thickness, nullptr);
    arcTree.setProperty(ShapeIDs::ArcConnectToValue, arcConfig_.connectToValue, nullptr);
    arcTree.setProperty(ShapeIDs::ArcStrokeOnly, arcConfig_.strokeOnly, nullptr);
    tree.appendChild(arcTree, nullptr);
    
    // Polygon config
    juce::ValueTree polyTree(ShapeIDs::PolygonConfig);
    polyTree.setProperty(ShapeIDs::PolygonSides, polygonConfig_.sides, nullptr);
    polyTree.setProperty(ShapeIDs::PolygonStartAngle, polygonConfig_.startAngle, nullptr);
    polyTree.setProperty(ShapeIDs::PolygonInnerRadius, polygonConfig_.innerRadiusRatio, nullptr);
    tree.appendChild(polyTree, nullptr);
    
    return tree;
}

void ShapeLayer::fromValueTree(const juce::ValueTree& tree)
{
    Layer::fromValueTree(tree);
    
    shapeType_ = static_cast<ShapeType>(static_cast<int>(tree.getProperty(ShapeIDs::ShapeType, 0)));
    sizeRatio_.x = tree.getProperty(ShapeIDs::SizeRatioX, 1.0f);
    sizeRatio_.y = tree.getProperty(ShapeIDs::SizeRatioY, 1.0f);
    padding_ = tree.getProperty(ShapeIDs::Padding, 0.0f);
    paddingRatio_ = tree.getProperty(ShapeIDs::PaddingRatio, 0.0f);
    
    // Arc config
    auto arcTree = tree.getChildWithName(ShapeIDs::ArcConfig);
    if (arcTree.isValid())
    {
        arcConfig_.startAngle = arcTree.getProperty(ShapeIDs::ArcStart, juce::MathConstants<float>::pi);
        arcConfig_.endAngle = arcTree.getProperty(ShapeIDs::ArcEnd, juce::MathConstants<float>::twoPi);
        arcConfig_.thickness = arcTree.getProperty(ShapeIDs::ArcThickness, 0.2f);
        arcConfig_.connectToValue = arcTree.getProperty(ShapeIDs::ArcConnectToValue, false);
        arcConfig_.strokeOnly = arcTree.getProperty(ShapeIDs::ArcStrokeOnly, false);
    }
    
    // Polygon config
    auto polyTree = tree.getChildWithName(ShapeIDs::PolygonConfig);
    if (polyTree.isValid())
    {
        polygonConfig_.sides = polyTree.getProperty(ShapeIDs::PolygonSides, 6);
        polygonConfig_.startAngle = polyTree.getProperty(ShapeIDs::PolygonStartAngle, 0.0f);
        polygonConfig_.innerRadiusRatio = polyTree.getProperty(ShapeIDs::PolygonInnerRadius, 0.0f);
    }
}

//==============================================================================
// Rendering

void ShapeLayer::renderLayer(juce::Graphics& g, const juce::Rectangle<float>& bounds)
{
    if (shapeType_ == ShapeType::None)
        return;
    
    // Adjust bounds based on stroke alignment
    auto adjustedBounds = bounds;
    if (getStyle().hasStroke && getStyle().strokeWidth > 0.0f)
    {
        const float strokeWidthPixels = getStyle().getStrokeWidthPixels(bounds, getSizingMode());
        
        switch (getStyle().strokeAlignment)
        {
            case StrokeAlignment::Inside:
                // Shrink bounds by stroke width so stroke stays entirely inside
                adjustedBounds = bounds.reduced(strokeWidthPixels / 2.0f);
                break;
                
            case StrokeAlignment::Outside:
                // Expand bounds by stroke width so stroke extends entirely outside
                adjustedBounds = bounds.expanded(strokeWidthPixels / 2.0f);
                break;
                
            case StrokeAlignment::Center:
            default:
                // Use original bounds (stroke centered on edge)
                break;
        }
    }
    
    const auto shapeBounds = getShapeBounds(adjustedBounds);
    juce::Path shapePath = getShapePath(adjustedBounds);
    
    if (shapePath.isEmpty())
        return;
    
    // Render shadow first (if enabled) - pass original bounds for relative sizing
    renderShadow(g, shapePath, bounds);
    
    // Check if this is a stroke-only arc or a line (lines are always stroke-only)
    bool isStrokeOnlyArc = (shapeType_ == ShapeType::Arc && arcConfig_.strokeOnly);
    bool isLine = (shapeType_ == ShapeType::Line);
    
    // Fill the shape (skip for stroke-only arcs, lines, or if hasFill is false)
    if (!isStrokeOnlyArc && !isLine && getStyle().hasFill && getStyle().fillMode != FillMode::None)
    {
        // Special handling for conic gradient
        if (getStyle().fillMode == FillMode::ConicGradient)
        {
            // Create the gradient
            auto gradient = getStyle().fillGradient.toColourGradient(shapeBounds);
            
            // Apply opacity
            float opacity = getStyle().opacity;
            for (int i = 0; i < gradient.getNumColours(); ++i)
            {
                auto colour = gradient.getColour(i).withMultipliedAlpha(opacity);
                gradient.setColour(i, colour);
            }
            
            // Use BlendModes to render conic gradient
            BlendModes::fillPathWithConicGradient(g, shapePath, gradient, shapeBounds);
        }
        else
        {
            applyFill(g, shapeBounds);
            g.fillPath(shapePath);
        }
    }
    
    // Stroke the shape - pass bounds for relative sizing
    applyStroke(g, shapePath, bounds);
}

//==============================================================================
// Path Generation

juce::Path ShapeLayer::createRectanglePath(const juce::Rectangle<float>& bounds) const
{
    juce::Path path;
    const auto& style = getStyle();
    const auto sizingMode = getSizingMode();
    
    if (style.useIndividualCorners)
    {
        // Individual corner radii - convert from relative to pixels
        const float radiusTL = style.getCornerRadiusTLPixels(bounds, sizingMode);
        const float radiusTR = style.getCornerRadiusTRPixels(bounds, sizingMode);
        const float radiusBL = style.getCornerRadiusBLPixels(bounds, sizingMode);
        const float radiusBR = style.getCornerRadiusBRPixels(bounds, sizingMode);
        
        const float maxRadius = juce::jmax(radiusTL, radiusTR, radiusBL, radiusBR);
        path.addRoundedRectangle(
            bounds.getX(), bounds.getY(),
            bounds.getWidth(), bounds.getHeight(),
            maxRadius, maxRadius,
            radiusTL > 0.0f,  // curveTopLeft
            radiusTR > 0.0f,  // curveTopRight
            radiusBL > 0.0f,  // curveBottomLeft
            radiusBR > 0.0f   // curveBottomRight
        );
    }
    else
    {
        const float radiusPixels = style.getCornerRadiusPixels(bounds, sizingMode);
        if (radiusPixels > 0.0f)
        {
            path.addRoundedRectangle(bounds, radiusPixels);
        }
        else
        {
            path.addRectangle(bounds);
        }
    }
    
    return path;
}

juce::Path ShapeLayer::createEllipsePath(const juce::Rectangle<float>& bounds) const
{
    juce::Path path;
    path.addEllipse(bounds);
    return path;
}

juce::Path ShapeLayer::createArcPath(const juce::Rectangle<float>& bounds) const
{
    // Use stroke-only path if requested
    if (arcConfig_.strokeOnly)
        return createArcStrokePath(bounds);
    
    juce::Path path;
    
    // Validate bounds - need minimum size to avoid JUCE assertions
    if (bounds.getWidth() < 1.0f || bounds.getHeight() < 1.0f)
        return path;
    
    const float radius = std::min(bounds.getWidth(), bounds.getHeight()) * 0.5f;
    if (radius < 0.5f)
        return path;
    
    const float innerRadius = radius * (1.0f - juce::jlimit(0.01f, 0.99f, arcConfig_.thickness));
    
    // Get angles directly - JUCE's addPieSegment handles the angle math
    float startAngle = arcConfig_.startAngle;
    float endAngle = arcConfig_.endAngle;
    
    // Only validate that we have a meaningful arc (not zero length)
    // Don't normalize angles as JUCE handles angles > 2π correctly
    if (std::abs(endAngle - startAngle) < 0.001f)
        return path;
    
    // Create arc as a pie segment with inner cutout
    const auto arcBounds = bounds.reduced((bounds.getWidth() - radius * 2) * 0.5f, 
                                           (bounds.getHeight() - radius * 2) * 0.5f);
    
    if (arcBounds.getWidth() < 1.0f || arcBounds.getHeight() < 1.0f)
        return path;
    
    path.addPieSegment(
        arcBounds,
        startAngle,
        endAngle,
        juce::jlimit(0.0f, 0.99f, innerRadius / radius)
    );
    
    // Apply corner radius if set
    const auto& style = getStyle();
    const float cornerRadiusPixels = style.getCornerRadiusPixels(bounds, getSizingMode());
    if (cornerRadiusPixels > 0.0f)
        return path.createPathWithRoundedCorners(cornerRadiusPixels);
    
    return path;
}

juce::Path ShapeLayer::createArcStrokePath(const juce::Rectangle<float>& bounds) const
{
    juce::Path path;
    
    // Validate bounds
    if (bounds.getWidth() < 1.0f || bounds.getHeight() < 1.0f)
        return path;
    
    const float radius = std::min(bounds.getWidth(), bounds.getHeight()) * 0.5f;
    if (radius < 0.5f)
        return path;
    
    // Get angles
    float startAngle = arcConfig_.startAngle;
    float endAngle = arcConfig_.endAngle;
    
    // Validate arc length
    if (std::abs(endAngle - startAngle) < 0.001f)
        return path;
    
    // Calculate the centerline radius (between inner and outer radius of filled arc)
    // For stroke-only, we draw at the middle of where the thickness would be
    const float centerRadius = radius * (1.0f - arcConfig_.thickness * 0.5f);
    
    // Center the arc bounds
    const auto arcBounds = bounds.reduced((bounds.getWidth() - centerRadius * 2) * 0.5f,
                                          (bounds.getHeight() - centerRadius * 2) * 0.5f);
    
    if (arcBounds.getWidth() < 1.0f || arcBounds.getHeight() < 1.0f)
        return path;
    
    // Create a simple arc path along the centerline
    path.addCentredArc(arcBounds.getCentreX(),
                       arcBounds.getCentreY(),
                       centerRadius,
                       centerRadius,
                       0.0f,  // rotation
                       startAngle,
                       endAngle,
                       true);  // start as new sub-path
    
    return path;
}

juce::Path ShapeLayer::createPolygonPath(const juce::Rectangle<float>& bounds) const
{
    juce::Path path;
    
    if (!polygonConfig_.isValid())
        return path;
    
    const auto centre = bounds.getCentre();
    const float radius = std::min(bounds.getWidth(), bounds.getHeight()) * 0.5f;
    const float angleStep = juce::MathConstants<float>::twoPi / polygonConfig_.sides;
    
    for (int i = 0; i < polygonConfig_.sides; ++i)
    {
        const float angle = polygonConfig_.startAngle + (i * angleStep) - juce::MathConstants<float>::halfPi;
        const float x = centre.x + radius * std::cos(angle);
        const float y = centre.y + radius * std::sin(angle);
        
        if (i == 0)
            path.startNewSubPath(x, y);
        else
            path.lineTo(x, y);
    }
    
    path.closeSubPath();
    
    // Apply corner radius if set - clamp to prevent JUCE arc assertion errors
    const auto& style = getStyle();
    float cornerRadiusPixels = style.getCornerRadiusPixels(bounds, getSizingMode());
    
    // Clamp corner radius to be no more than half the smallest edge length
    // This prevents createPathWithRoundedCorners from failing with arc assertions
    const float minDimension = std::min(bounds.getWidth(), bounds.getHeight());
    const float maxSafeRadius = minDimension * 0.25f;  // Conservative limit
    cornerRadiusPixels = std::min(cornerRadiusPixels, maxSafeRadius);
    
    if (cornerRadiusPixels > 0.5f)
        return path.createPathWithRoundedCorners(cornerRadiusPixels);
    
    return path;
}

juce::Path ShapeLayer::createStarPath(const juce::Rectangle<float>& bounds) const
{
    juce::Path path;
    
    if (!polygonConfig_.isValid())
        return path;
    
    const auto centre = bounds.getCentre();
    const float outerRadius = std::min(bounds.getWidth(), bounds.getHeight()) * 0.5f;
    
    // Clamp inner radius to avoid singularities at the center (0.0) or degenerate edges
    // A value of 0.0 creates lines that double back on themselves, causing stroke artifacts
    float safeInnerRatio = std::max(0.05f, std::min(0.95f, polygonConfig_.innerRadiusRatio));
    const float innerRadius = outerRadius * safeInnerRatio;
    
    const float angleStep = juce::MathConstants<float>::pi / polygonConfig_.sides;
    
    // Guard against degenerate cases
    if (outerRadius < 1.0f)
        return path;
    
    for (int i = 0; i < polygonConfig_.sides * 2; ++i)
    {
        const float radius = (i % 2 == 0) ? outerRadius : innerRadius;
        const float angle = polygonConfig_.startAngle + (i * angleStep) - juce::MathConstants<float>::halfPi;
        const float x = centre.x + radius * std::cos(angle);
        const float y = centre.y + radius * std::sin(angle);
        
        if (i == 0)
            path.startNewSubPath(x, y);
        else
            path.lineTo(x, y);
    }
    
    path.closeSubPath();
    
    // NOTE: Corner rounding is disabled for star shapes due to JUCE arc geometry issues
    // Star shapes have very acute angles at inner points that cause assertion failures
    // in JUCE's path flattening code when createPathWithRoundedCorners() is used.
    // The assertion: jassert (angle1 <= angle2 + MathConstants<float>::pi)
    // occurs deep in JUCE's internal arc-to-line conversion.
    
    return path;
}

juce::Path ShapeLayer::createTrianglePath(const juce::Rectangle<float>& bounds) const
{
    juce::Path path;
    
    // Equilateral triangle pointing up
    path.startNewSubPath(bounds.getCentreX(), bounds.getY());
    path.lineTo(bounds.getRight(), bounds.getBottom());
    path.lineTo(bounds.getX(), bounds.getBottom());
    path.closeSubPath();
    
    // Apply corner radius if set
    const auto& style = getStyle();
    const float cornerRadiusPixels = style.getCornerRadiusPixels(bounds, getSizingMode());
    if (cornerRadiusPixels > 0.0f)
        return path.createPathWithRoundedCorners(cornerRadiusPixels);
    
    return path;
}

juce::Path ShapeLayer::createLinePath(const juce::Rectangle<float>& bounds) const
{
    juce::Path path;
    
    const auto start = juce::Point<float>(
        bounds.getX() + bounds.getWidth() * lineConfig_.startX,
        bounds.getY() + bounds.getHeight() * lineConfig_.startY
    );
    const auto end = juce::Point<float>(
        bounds.getX() + bounds.getWidth() * lineConfig_.endX,
        bounds.getY() + bounds.getHeight() * lineConfig_.endY
    );
    
    path.startNewSubPath(start);
    path.lineTo(end);
    
    // Add arrows if requested
    if (lineConfig_.hasArrowStart || lineConfig_.hasArrowEnd)
    {
        // Calculate line angle and length
        const float angle = std::atan2(end.y - start.y, end.x - start.x);
        const float length = start.getDistanceFrom(end);
        const float arrowLen = length * lineConfig_.arrowSize;
        const float arrowAngleRad = juce::degreesToRadians(lineConfig_.arrowAngle);
        
        if (lineConfig_.hasArrowStart)
        {
            // Arrow at start pointing towards start
            // So the arrow "base" is away from start
            // Left wing
            path.startNewSubPath(start);
            path.lineTo(start.x + arrowLen * std::cos(angle + arrowAngleRad),
                        start.y + arrowLen * std::sin(angle + arrowAngleRad));
            
            // Right wing
            path.startNewSubPath(start);
            path.lineTo(start.x + arrowLen * std::cos(angle - arrowAngleRad),
                        start.y + arrowLen * std::sin(angle - arrowAngleRad));
        }
        
        if (lineConfig_.hasArrowEnd)
        {
            // Arrow at end pointing towards end
            // So the arrow "base" is away from end
            // Left wing
            path.startNewSubPath(end);
            path.lineTo(end.x + arrowLen * std::cos(angle + juce::MathConstants<float>::pi + arrowAngleRad),
                        end.y + arrowLen * std::sin(angle + juce::MathConstants<float>::pi + arrowAngleRad));
            
            // Right wing
            path.startNewSubPath(end);
            path.lineTo(end.x + arrowLen * std::cos(angle + juce::MathConstants<float>::pi - arrowAngleRad),
                        end.y + arrowLen * std::sin(angle + juce::MathConstants<float>::pi - arrowAngleRad));
        }
    }
    
    return path;
}

//==============================================================================
// Property value helpers for arc bindings

float ShapeLayer::getCurrentPropertyValue(ValueBindingProperty property) const
{
    switch (property)
    {
        case ValueBindingProperty::ArcEnd:
            return arcConfig_.endAngle;
        case ValueBindingProperty::ArcStart:
            return arcConfig_.startAngle;
        case ValueBindingProperty::ArcThickness:
            return arcConfig_.thickness;
        default:
            return Layer::getCurrentPropertyValue(property);
    }
}

void ShapeLayer::restoreOriginalPropertyValue(ValueBindingProperty property, float value)
{
    switch (property)
    {
        case ValueBindingProperty::ArcEnd:
            arcConfig_.endAngle = value;
            repaint();
            break;
        case ValueBindingProperty::ArcStart:
            arcConfig_.startAngle = value;
            repaint();
            break;
        case ValueBindingProperty::ArcThickness:
            arcConfig_.thickness = value;
            repaint();
            break;
        default:
            Layer::restoreOriginalPropertyValue(property, value);
            break;
    }
}

//==============================================================================
// Helper Methods

juce::Rectangle<float> ShapeLayer::getShapeBounds(const juce::Rectangle<float>& componentBounds) const
{
    auto bounds = componentBounds;
    
    // Apply padding
    float totalPadding = padding_;
    if (paddingRatio_ > 0.0f)
    {
        totalPadding += std::min(bounds.getWidth(), bounds.getHeight()) * paddingRatio_;
    }
    
    if (totalPadding > 0.0f)
    {
        bounds = bounds.reduced(totalPadding);
    }
    
    // Apply size ratio
    if (sizeRatio_.x != 1.0f || sizeRatio_.y != 1.0f)
    {
        const float newWidth = bounds.getWidth() * sizeRatio_.x;
        const float newHeight = bounds.getHeight() * sizeRatio_.y;
        bounds = bounds.withSizeKeepingCentre(newWidth, newHeight);
    }
    
    return bounds;
}

//==============================================================================
// JSON Serialization

json ShapeLayer::toJson() const
{
    json j = Layer::toJson();
    
    // Shape-specific properties
    j["shapeType"] = shapeType_;
    j["sizeRatio"] = sizeRatio_;
    j["padding"] = padding_;
    j["paddingRatio"] = paddingRatio_;
    
    // Arc configuration
    j["arcConfig"] = {
        {"startAngle", arcConfig_.startAngle},
        {"endAngle", arcConfig_.endAngle},
        {"thickness", arcConfig_.thickness},
        {"connectToValue", arcConfig_.connectToValue},
        {"startFromValue", arcConfig_.startFromValue},
        {"endFromValue", arcConfig_.endFromValue},
        {"strokeOnly", arcConfig_.strokeOnly}
    };
    
    // Polygon configuration
    j["polygonConfig"] = {
        {"sides", polygonConfig_.sides},
        {"startAngle", polygonConfig_.startAngle},
        {"innerRadiusRatio", polygonConfig_.innerRadiusRatio}
    };
    
    // Line configuration
    j["lineConfig"] = {
        {"startX", lineConfig_.startX},
        {"startY", lineConfig_.startY},
        {"endX", lineConfig_.endX},
        {"endY", lineConfig_.endY},
        {"hasArrowStart", lineConfig_.hasArrowStart},
        {"hasArrowEnd", lineConfig_.hasArrowEnd},
        {"arrowSize", lineConfig_.arrowSize},
        {"arrowAngle", lineConfig_.arrowAngle}
    };
    
    return j;
}

void ShapeLayer::fromJson(const json& j)
{
    Layer::fromJson(j);
    
    // Shape-specific properties
    if (j.contains("shapeType"))
        shapeType_ = j["shapeType"].get<ShapeType>();
    
    if (j.contains("sizeRatio"))
        j["sizeRatio"].get_to(sizeRatio_);
    
    padding_ = j.value("padding", 0.0f);
    paddingRatio_ = j.value("paddingRatio", 0.0f);
    
    // Arc configuration
    if (j.contains("arcConfig"))
    {
        const auto& arc = j["arcConfig"];
        arcConfig_.startAngle = arc.value("startAngle", juce::MathConstants<float>::pi);
        arcConfig_.endAngle = arc.value("endAngle", juce::MathConstants<float>::twoPi);
        arcConfig_.thickness = arc.value("thickness", 0.2f);
        arcConfig_.connectToValue = arc.value("connectToValue", false);
        arcConfig_.startFromValue = arc.value("startFromValue", false);
        arcConfig_.endFromValue = arc.value("endFromValue", true);
        arcConfig_.strokeOnly = arc.value("strokeOnly", false);
    }
    
    // Polygon configuration
    if (j.contains("polygonConfig"))
    {
        const auto& poly = j["polygonConfig"];
        polygonConfig_.sides = poly.value("sides", 6);
        polygonConfig_.startAngle = poly.value("startAngle", 0.0f);
        polygonConfig_.innerRadiusRatio = poly.value("innerRadiusRatio", 0.0f);
    }
    
    // Line configuration
    if (j.contains("lineConfig"))
    {
        const auto& line = j["lineConfig"];
        lineConfig_.startX = line.value("startX", 0.0f);
        lineConfig_.startY = line.value("startY", 0.5f);
        lineConfig_.endX = line.value("endX", 1.0f);
        lineConfig_.endY = line.value("endY", 0.5f);
        lineConfig_.hasArrowStart = line.value("hasArrowStart", false);
        lineConfig_.hasArrowEnd = line.value("hasArrowEnd", true);
        lineConfig_.arrowSize = line.value("arrowSize", 0.1f);
        lineConfig_.arrowAngle = line.value("arrowAngle", 30.0f);
    }
}

} // namespace zaplab

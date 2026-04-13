/*
  ==============================================================================

    Layer.cpp
    Created: 30 Nov 2025
    Author:  Paolo Zappalà @ zaplab.dev

  ==============================================================================
*/

#include "Layer.h"

namespace zaplab
{

//==============================================================================
// Identifier constants for serialization
namespace IDs
{
    static const juce::Identifier Layer { "Layer" };
    static const juce::Identifier Name { "name" };
    static const juce::Identifier Type { "type" };
    static const juce::Identifier Visible { "visible" };
    static const juce::Identifier Locked { "locked" };
    static const juce::Identifier Index { "index" };
    
    // Transform
    static const juce::Identifier Transform { "transform" };
    static const juce::Identifier PositionX { "posX" };
    static const juce::Identifier PositionY { "posY" };
    static const juce::Identifier AnchorX { "anchorX" };
    static const juce::Identifier AnchorY { "anchorY" };
    static const juce::Identifier Rotation { "rotation" };
    static const juce::Identifier ScaleX { "scaleX" };
    static const juce::Identifier ScaleY { "scaleY" };
    static const juce::Identifier SizeX { "sizeX" };
    static const juce::Identifier SizeY { "sizeY" };
    
    // Style
    static const juce::Identifier Style { "style" };
    static const juce::Identifier FillMode { "fillMode" };
    static const juce::Identifier FillColour { "fillColour" };
    static const juce::Identifier HasFill { "hasFill" };
    static const juce::Identifier HasStroke { "hasStroke" };
    static const juce::Identifier StrokeColour { "strokeColour" };
    static const juce::Identifier StrokeWidth { "strokeWidth" };
    static const juce::Identifier StrokeCap { "strokeCap" };
    static const juce::Identifier StrokeJoin { "strokeJoin" };
    static const juce::Identifier StrokeAlignment { "strokeAlignment" };
    static const juce::Identifier CornerRadius { "cornerRadius" };
    static const juce::Identifier Opacity { "opacity" };
    static const juce::Identifier BlendModeID { "blendMode" };
    
    // Shadow
    static const juce::Identifier Shadow { "shadow" };
    static const juce::Identifier ShadowEnabled { "enabled" };
    static const juce::Identifier ShadowColour { "colour" };
    static const juce::Identifier ShadowOffsetX { "offsetX" };
    static const juce::Identifier ShadowOffsetY { "offsetY" };
    static const juce::Identifier ShadowRadius { "radius" };
    
    // Gradient
    static const juce::Identifier FillGradient { "fillGradient" };
    static const juce::Identifier GradientEnabled { "enabled" };
    static const juce::Identifier GradientType { "type" };
    static const juce::Identifier GradientIsRadial { "isRadial" };
    static const juce::Identifier GradientColour1 { "colour1" };
    static const juce::Identifier GradientColour2 { "colour2" };
    static const juce::Identifier GradientStartX { "startX" };
    static const juce::Identifier GradientStartY { "startY" };
    static const juce::Identifier GradientEndX { "endX" };
    static const juce::Identifier GradientEndY { "endY" };
    
    // Slider
    static const juce::Identifier SliderRole { "sliderRole" };
    static const juce::Identifier SizingModeID { "sizingMode" };
    
    // Value Binding
    static const juce::Identifier ValueBinding { "valueBinding" };
    static const juce::Identifier ValueBindings { "valueBindings" };  // Container for multiple bindings
    static const juce::Identifier BindingEnabled { "enabled" };
    static const juce::Identifier BindingProperty { "property" };
    static const juce::Identifier BindingCurve { "curve" };
    static const juce::Identifier BindingValueAtMin { "valueAtMin" };
    static const juce::Identifier BindingValueAtMax { "valueAtMax" };
    static const juce::Identifier BindingOriginalValue { "originalValue" };
    static const juce::Identifier BindingHasStoredOriginal { "hasStoredOriginal" };
    static const juce::Identifier BindingSteps { "steps" };
    static const juce::Identifier BindingIsRelative { "isRelative" };
    static const juce::Identifier BindingRotationOffset { "rotationOffset" };
    static const juce::Identifier BindingPivotX { "pivotX" };
    static const juce::Identifier BindingPivotY { "pivotY" };
    static const juce::Identifier BindingOrbitRadius { "orbitRadius" };
    static const juce::Identifier BindingBipolar { "bipolar" };
    
    // Base transform (for value binding)
    static const juce::Identifier BaseTransform { "baseTransform" };
    static const juce::Identifier BaseOpacity { "baseOpacity" };
    
    // Mask
    static const juce::Identifier Mask { "mask" };
    static const juce::Identifier MaskEnabled { "enabled" };
    static const juce::Identifier MaskInverted { "inverted" };
    static const juce::Identifier MaskLayerName { "maskLayerName" };
}

//==============================================================================
Layer::Layer(const juce::String& name)
    : name_(name)
{
    setInterceptsMouseClicks(false, false);
    setName(name);
}

//==============================================================================
// Visibility & Basic Properties

void Layer::setLayerVisible(bool visible) noexcept
{
    if (visible_ != visible)
    {
        visible_ = visible;
        setVisible(visible);
        
        if (onVisibilityChanged)
            onVisibilityChanged();
        
        repaint();
    }
}

void Layer::setLayerName(const juce::String& name)
{
    if (name_ != name)
    {
        name_ = name;
        setName(name);
        
        if (onNameChanged)
            onNameChanged(name);
    }
}

//==============================================================================
// Transform

void Layer::setTransform(const Transform& transform)
{
    if (transform_ != transform)
    {
        transform_ = transform;
        
        if (onTransformChanged)
            onTransformChanged();
        
        repaint();
    }
}

void Layer::setPosition(juce::Point<float> pos)
{
    if (transform_.position != pos)
    {
        transform_.position = pos;
        
        if (onTransformChanged)
            onTransformChanged();
        
        repaint();
    }
}

void Layer::setRotation(float radians)
{
    if (transform_.rotation != radians)
    {
        transform_.rotation = radians;
        
        if (onTransformChanged)
            onTransformChanged();
        
        repaint();
    }
}

void Layer::setScale(juce::Point<float> scale)
{
    if (transform_.scale != scale)
    {
        transform_.scale = scale;
        
        if (onTransformChanged)
            onTransformChanged();
        
        repaint();
    }
}

void Layer::setAnchor(juce::Point<float> anchor)
{
    if (transform_.anchor != anchor)
    {
        transform_.anchor = anchor;
        
        if (onTransformChanged)
            onTransformChanged();
        
        repaint();
    }
}

//==============================================================================
// Centering Utilities

void Layer::centerOnCanvas()
{
    // Center the layer's position to the center of the canvas
    transform_.position = { 0.5f, 0.5f };
    
    if (onTransformChanged)
        onTransformChanged();
    
    repaint();
}

void Layer::centerAnchorToLayer()
{
    // Move anchor to center of layer (0.5, 0.5 in layer space)
    // Adjust position so the layer visually stays in the same place
    
    // Current visual center of layer in canvas space
    const float layerW = transform_.size.x;
    const float layerH = transform_.size.y;
    
    // Calculate the offset from old anchor to new anchor (layer center)
    // The anchor is currently at transform_.anchor (normalized 0-1 within the layer)
    // We want it at (0.5, 0.5)
    const float dx = (0.5f - transform_.anchor.x) * layerW;
    const float dy = (0.5f - transform_.anchor.y) * layerH;
    
    // Rotate the delta vector by the layer's rotation
    const float cosR = std::cos(transform_.rotation);
    const float sinR = std::sin(transform_.rotation);
    
    const float rotatedDx = dx * cosR - dy * sinR;
    const float rotatedDy = dx * sinR + dy * cosR;
    
    // To keep the layer visually in place, we need to move position by the rotated offset
    transform_.position.x += rotatedDx;
    transform_.position.y += rotatedDy;
    
    // Set anchor to center
    transform_.anchor = { 0.5f, 0.5f };
    
    if (onTransformChanged)
        onTransformChanged();
    
    repaint();
}

void Layer::centerAnchorToCanvas(bool keepVisualPosition)
{
    if (keepVisualPosition)
    {
        const float layerW = transform_.size.x;
        const float layerH = transform_.size.y;

        if (layerW <= 0.0f || layerH <= 0.0f)
            return;

        const float targetX = 0.5f;
        const float targetY = 0.5f;

        const float dx = targetX - transform_.position.x;
        const float dy = targetY - transform_.position.y;

        const float cosR = std::cos(transform_.rotation);
        const float sinR = std::sin(transform_.rotation);

        // Transform canvas delta into layer space to determine anchor delta
        const float vLayerX = dx * cosR + dy * sinR;
        const float vLayerY = -dx * sinR + dy * cosR;

        const float newAnchorX = transform_.anchor.x + vLayerX / layerW;
        const float newAnchorY = transform_.anchor.y + vLayerY / layerH;

        transform_.anchor = { newAnchorX, newAnchorY };
        transform_.position = { targetX, targetY };
    }
    else
    {
        transform_.anchor = { 0.5f, 0.5f };
        transform_.position = { 0.5f, 0.5f };
    }

    if (onTransformChanged)
        onTransformChanged();
    
    repaint();
}

//==============================================================================
// Style

void Layer::setStyle(const LayerStyle& style)
{
    if (!(style_ == style))
    {
        style_ = style;
        
        if (onStyleChanged)
            onStyleChanged();
        
        repaint();
    }
}

void Layer::setOpacity(float opacity)
{
    opacity = juce::jlimit(0.0f, 1.0f, opacity);
    
    if (style_.opacity != opacity)
    {
        style_.opacity = opacity;
        
        if (onStyleChanged)
            onStyleChanged();
        
        repaint();
    }
}

void Layer::setBlendMode(BlendMode mode)
{
    if (style_.blendMode != mode)
    {
        style_.blendMode = mode;
        
        if (onStyleChanged)
            onStyleChanged();
        
        repaint();
    }
}

void Layer::setFillColour(juce::Colour colour)
{
    if (style_.fillColour != colour)
    {
        style_.fillColour = colour;
        style_.fillMode = FillMode::Solid;
        
        if (onStyleChanged)
            onStyleChanged();
        
        repaint();
    }
}

void Layer::setStrokeColour(juce::Colour colour)
{
    if (style_.strokeColour != colour)
    {
        style_.strokeColour = colour;
        style_.hasStroke = true;
        
        if (onStyleChanged)
            onStyleChanged();
        
        repaint();
    }
}

void Layer::setStrokeWidth(float width)
{
    if (style_.strokeWidth != width)
    {
        style_.strokeWidth = width;
        // Note: hasStroke is now controlled separately by UI toggle
        // Don't auto-enable stroke when width changes
        
        if (onStyleChanged)
            onStyleChanged();
        
        repaint();
    }
}

//==============================================================================
// Effects

void Layer::setEffects(const LayerEffects& effects)
{
    if (!(effects_ == effects))
    {
        effects_ = effects;
        effectsCacheDirty_ = true;
        
        if (onStyleChanged)
            onStyleChanged();
        
        repaint();
    }
}

void Layer::setDropShadowEnabled(bool enabled)
{
    if (effects_.dropShadow.enabled != enabled)
    {
        effects_.dropShadow.enabled = enabled;
        effectsCacheDirty_ = true;
        repaint();
    }
}

void Layer::setOuterGlowEnabled(bool enabled)
{
    if (effects_.outerGlow.enabled != enabled)
    {
        effects_.outerGlow.enabled = enabled;
        effectsCacheDirty_ = true;
        repaint();
    }
}

void Layer::setInnerGlowEnabled(bool enabled)
{
    if (effects_.innerGlow.enabled != enabled)
    {
        effects_.innerGlow.enabled = enabled;
        effectsCacheDirty_ = true;
        repaint();
    }
}

void Layer::setBevelEmbossEnabled(bool enabled)
{
    if (effects_.bevelEmboss.enabled != enabled)
    {
        effects_.bevelEmboss.enabled = enabled;
        effectsCacheDirty_ = true;
        repaint();
    }
}

void Layer::setGaussianBlurEnabled(bool enabled)
{
    if (effects_.gaussianBlur.enabled != enabled)
    {
        effects_.gaussianBlur.enabled = enabled;
        effectsCacheDirty_ = true;
        repaint();
    }
}

//==============================================================================
// Slider Integration

// Static empty binding for when there are no bindings
static ValueBinding emptyBinding_;

const ValueBinding& Layer::getValueBinding() const noexcept
{
    if (valueBindings_.empty())
        return emptyBinding_;
    return valueBindings_[0];
}

ValueBinding& Layer::getValueBinding() noexcept
{
    if (valueBindings_.empty())
        valueBindings_.push_back(ValueBinding{});
    return valueBindings_[0];
}

void Layer::setValueBinding(const ValueBinding& binding)
{
    if (valueBindings_.empty())
        valueBindings_.push_back(binding);
    else
        valueBindings_[0] = binding;
    
    // Store the current transform as the base (if not already stored)
    if (binding.enabled)
    {
        baseTransform_ = transform_;
        baseOpacity_ = style_.opacity;
    }
    
    repaint();
}

void Layer::addValueBinding(const ValueBinding& binding)
{
    valueBindings_.push_back(binding);
    
    if (binding.enabled)
    {
        baseTransform_ = transform_;
        baseOpacity_ = style_.opacity;
    }
    
    repaint();
}

void Layer::addValueBinding()
{
    ValueBinding binding;
    binding.enabled = false;  // Start disabled so user can configure first
    binding.property = ValueBindingProperty::None;
    addValueBinding(binding);
}

void Layer::removeValueBinding(size_t index)
{
    if (index < valueBindings_.size())
    {
        auto& binding = valueBindings_[index];
        
        // Restore original value if it was stored
        if (binding.enabled && binding.hasStoredOriginal)
        {
            restoreOriginalPropertyValue(binding.property, binding.originalValue);
        }
        
        valueBindings_.erase(valueBindings_.begin() + static_cast<long>(index));
        repaint();
    }
}

void Layer::setValueBindingEnabled(bool enabled)
{
    if (valueBindings_.empty())
        valueBindings_.push_back(ValueBinding{});
    
    if (valueBindings_[0].enabled != enabled)
    {
        if (enabled)
        {
            // Store current state as base
            baseTransform_ = transform_;
            baseOpacity_ = style_.opacity;
            
            // Store original property value before binding is applied
            if (!valueBindings_[0].hasStoredOriginal)
            {
                valueBindings_[0].originalValue = getCurrentPropertyValue(valueBindings_[0].property);
                valueBindings_[0].hasStoredOriginal = true;
            }
        }
        else
        {
            // Restore original value when disabling
            if (valueBindings_[0].hasStoredOriginal)
            {
                restoreOriginalPropertyValue(valueBindings_[0].property, valueBindings_[0].originalValue);
            }
            
            // Restore base state (only if no other bindings are enabled)
            bool anyEnabled = false;
            for (const auto& b : valueBindings_)
            {
                if (b.enabled)
                {
                    anyEnabled = true;
                    break;
                }
            }
            if (!anyEnabled)
            {
                transform_ = baseTransform_;
                style_.opacity = baseOpacity_;
            }
        }
        
        valueBindings_[0].enabled = enabled;
        repaint();
    }
}

bool Layer::isValueBindingEnabled() const noexcept
{
    for (const auto& b : valueBindings_)
    {
        if (b.enabled)
            return true;
    }
    return false;
}

void Layer::setValueBindingProperty(ValueBindingProperty property)
{
    if (valueBindings_.empty())
        valueBindings_.push_back(ValueBinding{});
    
    valueBindings_[0].property = property;
    
    // Set default range for the property
    auto [minVal, maxVal] = ValueBinding::getDefaultRange(property);
    valueBindings_[0].valueAtMin = minVal;
    valueBindings_[0].valueAtMax = maxVal;
    
    repaint();
}

void Layer::setValueBindingRange(float minValue, float maxValue)
{
    if (valueBindings_.empty())
        valueBindings_.push_back(ValueBinding{});
    
    valueBindings_[0].valueAtMin = minValue;
    valueBindings_[0].valueAtMax = maxValue;
    repaint();
}

void Layer::updateForSliderValue(float normalizedValue)
{
    sliderValue_ = juce::jlimit(0.0f, 1.0f, normalizedValue);
    
    // Apply all enabled value bindings
    for (const auto& binding : valueBindings_)
    {
        if (!binding.enabled || binding.property == ValueBindingProperty::None)
            continue;
            
        const float propertyValue = binding.computePropertyValue(sliderValue_);
        
        switch (binding.property)
        {
            case ValueBindingProperty::Rotation:
                if (binding.isRelative)
                    transform_.rotation = baseTransform_.rotation + propertyValue + binding.rotationOffset;
                else
                    transform_.rotation = propertyValue + binding.rotationOffset;
                break;
                
            case ValueBindingProperty::RotationAroundPoint:
            {
                // Orbit around a pivot point at a fixed radius
                // The propertyValue is the angle in radians, plus any offset
                const float angle = propertyValue + binding.rotationOffset;
                
                // Calculate position on the orbit circle
                // Subtract half pi to align 0 degrees with top (12 o'clock position)
                const float adjustedAngle = angle - juce::MathConstants<float>::halfPi;
                
                // The orbit radius is stored as a normalized value relative to the reference dimension
                // The visualization draws: orbitRadiusPixels = binding.orbitRadius * referenceDimension
                // For the layer to follow that circular path, we need to convert that pixel radius
                // back to normalized coordinates which are different for X and Y
                float radiusX = binding.orbitRadius;
                float radiusY = binding.orbitRadius;
                
                if (!cachedRenderBounds_.isEmpty())
                {
                    const float refDim = getReferenceDimension(cachedRenderBounds_, sizingMode_);
                    const float width = cachedRenderBounds_.getWidth();
                    const float height = cachedRenderBounds_.getHeight();
                    
                    // Convert: pixelRadius = binding.orbitRadius * refDim
                    // Then: normalizedX = pixelRadius / width, normalizedY = pixelRadius / height
                    // Combined: normalizedX = binding.orbitRadius * refDim / width
                    if (width > 0.0f && height > 0.0f)
                    {
                        radiusX = binding.orbitRadius * refDim / width;
                        radiusY = binding.orbitRadius * refDim / height;
                    }
                }
                
                const float x = binding.pivotPoint.x + std::cos(adjustedAngle) * radiusX;
                const float y = binding.pivotPoint.y + std::sin(adjustedAngle) * radiusY;
                
                transform_.position.x = x;
                transform_.position.y = y;
                break;
            }
                
            case ValueBindingProperty::PositionX:
                if (binding.isRelative)
                    transform_.position.x = baseTransform_.position.x + propertyValue;
                else
                    transform_.position.x = propertyValue;
                break;
                
            case ValueBindingProperty::PositionY:
                if (binding.isRelative)
                    transform_.position.y = baseTransform_.position.y + propertyValue;
                else
                    transform_.position.y = propertyValue;
                break;
                
            case ValueBindingProperty::Width:
                transform_.size.x = propertyValue;
                break;
                
            case ValueBindingProperty::Height:
                transform_.size.y = propertyValue;
                break;
                
            case ValueBindingProperty::ScaleX:
                transform_.scale.x = propertyValue;
                break;
                
            case ValueBindingProperty::ScaleY:
                transform_.scale.y = propertyValue;
                break;
                
            case ValueBindingProperty::ScaleUniform:
                transform_.scale.x = propertyValue;
                transform_.scale.y = propertyValue;
                break;
                
            case ValueBindingProperty::Opacity:
                style_.opacity = juce::jlimit(0.0f, 1.0f, propertyValue);
                break;
                
            case ValueBindingProperty::ArcEnd:
            case ValueBindingProperty::ArcStart:
            case ValueBindingProperty::ArcThickness:
                // These are handled in ShapeLayer::updateForSliderValue
                break;
                
            case ValueBindingProperty::OuterGlowBlur:
                // Outer glow blur radius (0-100 is reasonable range)
                effects_.outerGlow.blurRadius = static_cast<int>(juce::jlimit(0.0f, 100.0f, propertyValue));
                effectsCacheDirty_ = true;
                break;
                
            case ValueBindingProperty::OverlayOpacity:
                // Colour overlay opacity (0-1 range)
                effects_.colourOverlay.opacity = juce::jlimit(0.0f, 1.0f, propertyValue);
                effectsCacheDirty_ = true;
                break;
                
            case ValueBindingProperty::None:
                break;
        }
    }
    
    repaint();
}

//==============================================================================
// Helper methods for binding original value storage/restoration

float Layer::getCurrentPropertyValue(ValueBindingProperty property) const
{
    switch (property)
    {
        case ValueBindingProperty::Rotation:
            return transform_.rotation;
        case ValueBindingProperty::RotationAroundPoint:
            return transform_.rotation;
        case ValueBindingProperty::PositionX:
            return transform_.position.x;
        case ValueBindingProperty::PositionY:
            return transform_.position.y;
        case ValueBindingProperty::Width:
            return transform_.size.x;
        case ValueBindingProperty::Height:
            return transform_.size.y;
        case ValueBindingProperty::ScaleX:
            return transform_.scale.x;
        case ValueBindingProperty::ScaleY:
            return transform_.scale.y;
        case ValueBindingProperty::ScaleUniform:
            return transform_.scale.x;  // Use X as representative
        case ValueBindingProperty::Opacity:
            return style_.opacity;
        case ValueBindingProperty::ArcEnd:
        case ValueBindingProperty::ArcStart:
        case ValueBindingProperty::ArcThickness:
            // These are shape-specific, subclasses should handle
            return 0.0f;
        case ValueBindingProperty::OuterGlowBlur:
            return static_cast<float>(effects_.outerGlow.blurRadius);
        case ValueBindingProperty::OverlayOpacity:
            return effects_.colourOverlay.opacity;
        case ValueBindingProperty::None:
        default:
            return 0.0f;
    }
}

void Layer::restoreOriginalPropertyValue(ValueBindingProperty property, float value)
{
    switch (property)
    {
        case ValueBindingProperty::Rotation:
            transform_.rotation = value;
            break;
        case ValueBindingProperty::RotationAroundPoint:
            transform_.rotation = value;
            break;
        case ValueBindingProperty::PositionX:
            transform_.position.x = value;
            break;
        case ValueBindingProperty::PositionY:
            transform_.position.y = value;
            break;
        case ValueBindingProperty::Width:
            transform_.size.x = value;
            break;
        case ValueBindingProperty::Height:
            transform_.size.y = value;
            break;
        case ValueBindingProperty::ScaleX:
            transform_.scale.x = value;
            break;
        case ValueBindingProperty::ScaleY:
            transform_.scale.y = value;
            break;
        case ValueBindingProperty::ScaleUniform:
            transform_.scale.x = value;
            transform_.scale.y = value;
            break;
        case ValueBindingProperty::Opacity:
            style_.opacity = value;
            break;
        case ValueBindingProperty::ArcEnd:
        case ValueBindingProperty::ArcStart:
        case ValueBindingProperty::ArcThickness:
            // These are shape-specific, subclasses should handle
            break;
        case ValueBindingProperty::OuterGlowBlur:
            effects_.outerGlow.blurRadius = static_cast<int>(value);
            effectsCacheDirty_ = true;
            break;
        case ValueBindingProperty::OverlayOpacity:
            effects_.colourOverlay.opacity = value;
            effectsCacheDirty_ = true;
            break;
        case ValueBindingProperty::None:
        default:
            break;
    }
}

//==============================================================================
// Masking

void Layer::setMask(const LayerMask& mask)
{
    mask_ = mask;
    repaint();
}

void Layer::clearMask()
{
    mask_.clear();
    repaint();
}

//==============================================================================
// Interaction State

void Layer::setInteractionState(const InteractionState& state)
{
    interaction_ = state;
    repaint();
}

void Layer::setHovered(bool hovered)
{
    if (interaction_.isHovered != hovered)
    {
        interaction_.isHovered = hovered;
        repaint();
    }
}

void Layer::setPressed(bool pressed)
{
    if (interaction_.isPressed != pressed)
    {
        interaction_.isPressed = pressed;
        repaint();
    }
}

void Layer::setToggled(bool toggled)
{
    if (interaction_.isToggled != toggled)
    {
        interaction_.isToggled = toggled;
        repaint();
    }
}

//==============================================================================
// Serialization

juce::ValueTree Layer::toValueTree() const
{
    juce::ValueTree tree(IDs::Layer);
    
    // Basic properties
    tree.setProperty(IDs::Name, name_, nullptr);
    tree.setProperty(IDs::Type, static_cast<int>(getType()), nullptr);
    tree.setProperty(IDs::Visible, visible_, nullptr);
    tree.setProperty(IDs::Locked, locked_, nullptr);
    tree.setProperty(IDs::Index, index_, nullptr);
    
    // Transform
    juce::ValueTree transformTree(IDs::Transform);
    transformTree.setProperty(IDs::PositionX, transform_.position.x, nullptr);
    transformTree.setProperty(IDs::PositionY, transform_.position.y, nullptr);
    transformTree.setProperty(IDs::AnchorX, transform_.anchor.x, nullptr);
    transformTree.setProperty(IDs::AnchorY, transform_.anchor.y, nullptr);
    transformTree.setProperty(IDs::Rotation, transform_.rotation, nullptr);
    transformTree.setProperty(IDs::ScaleX, transform_.scale.x, nullptr);
    transformTree.setProperty(IDs::ScaleY, transform_.scale.y, nullptr);
    transformTree.setProperty(IDs::SizeX, transform_.size.x, nullptr);
    transformTree.setProperty(IDs::SizeY, transform_.size.y, nullptr);
    tree.appendChild(transformTree, nullptr);
    
    // Style
    juce::ValueTree styleTree(IDs::Style);
    styleTree.setProperty(IDs::FillMode, static_cast<int>(style_.fillMode), nullptr);
    styleTree.setProperty(IDs::FillColour, style_.fillColour.toString(), nullptr);
    styleTree.setProperty(IDs::HasFill, style_.hasFill, nullptr);
    styleTree.setProperty(IDs::HasStroke, style_.hasStroke, nullptr);
    styleTree.setProperty(IDs::StrokeColour, style_.strokeColour.toString(), nullptr);
    styleTree.setProperty(IDs::StrokeWidth, style_.strokeWidth, nullptr);
    styleTree.setProperty(IDs::StrokeCap, static_cast<int>(style_.strokeCap), nullptr);
    styleTree.setProperty(IDs::StrokeJoin, static_cast<int>(style_.strokeJoin), nullptr);
    styleTree.setProperty(IDs::StrokeAlignment, static_cast<int>(style_.strokeAlignment), nullptr);
    styleTree.setProperty(IDs::CornerRadius, style_.cornerRadius, nullptr);
    styleTree.setProperty(IDs::Opacity, style_.opacity, nullptr);
    styleTree.setProperty(IDs::BlendModeID, static_cast<int>(style_.blendMode), nullptr);
    
    // Shadow
    juce::ValueTree shadowTree(IDs::Shadow);
    shadowTree.setProperty(IDs::ShadowEnabled, style_.shadow.enabled, nullptr);
    shadowTree.setProperty(IDs::ShadowColour, style_.shadow.colour.toString(), nullptr);
    shadowTree.setProperty(IDs::ShadowOffsetX, style_.shadow.offset.x, nullptr);
    shadowTree.setProperty(IDs::ShadowOffsetY, style_.shadow.offset.y, nullptr);
    shadowTree.setProperty(IDs::ShadowRadius, style_.shadow.radius, nullptr);
    styleTree.appendChild(shadowTree, nullptr);
    
    // Gradient
    juce::ValueTree gradientTree(IDs::FillGradient);
    gradientTree.setProperty(IDs::GradientEnabled, style_.fillGradient.enabled, nullptr);
    gradientTree.setProperty(IDs::GradientType, static_cast<int>(style_.fillGradient.type), nullptr);
    gradientTree.setProperty(IDs::GradientIsRadial, style_.fillGradient.isRadial, nullptr);
    gradientTree.setProperty(IDs::GradientColour1, style_.fillGradient.colour1.toString(), nullptr);
    gradientTree.setProperty(IDs::GradientColour2, style_.fillGradient.colour2.toString(), nullptr);
    gradientTree.setProperty(IDs::GradientStartX, style_.fillGradient.start.x, nullptr);
    gradientTree.setProperty(IDs::GradientStartY, style_.fillGradient.start.y, nullptr);
    gradientTree.setProperty(IDs::GradientEndX, style_.fillGradient.end.x, nullptr);
    gradientTree.setProperty(IDs::GradientEndY, style_.fillGradient.end.y, nullptr);
    styleTree.appendChild(gradientTree, nullptr);
    
    tree.appendChild(styleTree, nullptr);
    
    // Effects (advanced effects like blur, glow, bevel)
    if (effects_.hasAnyEffect())
    {
        tree.appendChild(effects_.toValueTree(), nullptr);
    }
    
    // Slider role and sizing mode
    tree.setProperty(IDs::SliderRole, static_cast<int>(sliderRole_), nullptr);
    tree.setProperty(IDs::SizingModeID, static_cast<int>(sizingMode_), nullptr);
    
    // Value Bindings - save all bindings
    bool hasAnyBinding = false;
    for (const auto& binding : valueBindings_)
    {
        if (binding.enabled || binding.property != ValueBindingProperty::None)
        {
            hasAnyBinding = true;
            break;
        }
    }
    
    if (hasAnyBinding)
    {
        // For backward compatibility, still save primary binding as "valueBinding"
        // and additional bindings in "valueBindings" array
        const auto& primaryBinding = valueBindings_.empty() ? emptyBinding_ : valueBindings_[0];
        
        juce::ValueTree bindingTree(IDs::ValueBinding);
        bindingTree.setProperty(IDs::BindingEnabled, primaryBinding.enabled, nullptr);
        bindingTree.setProperty(IDs::BindingProperty, static_cast<int>(primaryBinding.property), nullptr);
        bindingTree.setProperty(IDs::BindingCurve, static_cast<int>(primaryBinding.curve), nullptr);
        bindingTree.setProperty(IDs::BindingValueAtMin, primaryBinding.valueAtMin, nullptr);
        bindingTree.setProperty(IDs::BindingValueAtMax, primaryBinding.valueAtMax, nullptr);
        bindingTree.setProperty(IDs::BindingOriginalValue, primaryBinding.originalValue, nullptr);
        bindingTree.setProperty(IDs::BindingHasStoredOriginal, primaryBinding.hasStoredOriginal, nullptr);
        bindingTree.setProperty(IDs::BindingSteps, primaryBinding.steps, nullptr);
        bindingTree.setProperty(IDs::BindingIsRelative, primaryBinding.isRelative, nullptr);
        bindingTree.setProperty(IDs::BindingRotationOffset, primaryBinding.rotationOffset, nullptr);
        bindingTree.setProperty(IDs::BindingPivotX, primaryBinding.pivotPoint.x, nullptr);
        bindingTree.setProperty(IDs::BindingPivotY, primaryBinding.pivotPoint.y, nullptr);
        bindingTree.setProperty(IDs::BindingOrbitRadius, primaryBinding.orbitRadius, nullptr);
        bindingTree.setProperty(IDs::BindingBipolar, primaryBinding.bipolar, nullptr);
        
        // Base transform
        juce::ValueTree baseTransformTree(IDs::BaseTransform);
        baseTransformTree.setProperty(IDs::PositionX, baseTransform_.position.x, nullptr);
        baseTransformTree.setProperty(IDs::PositionY, baseTransform_.position.y, nullptr);
        baseTransformTree.setProperty(IDs::Rotation, baseTransform_.rotation, nullptr);
        baseTransformTree.setProperty(IDs::ScaleX, baseTransform_.scale.x, nullptr);
        baseTransformTree.setProperty(IDs::ScaleY, baseTransform_.scale.y, nullptr);
        bindingTree.appendChild(baseTransformTree, nullptr);
        bindingTree.setProperty(IDs::BaseOpacity, baseOpacity_, nullptr);
        
        tree.appendChild(bindingTree, nullptr);
        
        // Save additional bindings if any
        if (valueBindings_.size() > 1)
        {
            juce::ValueTree bindingsArray(IDs::ValueBindings);
            for (size_t i = 1; i < valueBindings_.size(); ++i)
            {
                const auto& binding = valueBindings_[i];
                juce::ValueTree additionalBinding(IDs::ValueBinding);
                additionalBinding.setProperty(IDs::BindingEnabled, binding.enabled, nullptr);
                additionalBinding.setProperty(IDs::BindingProperty, static_cast<int>(binding.property), nullptr);
                additionalBinding.setProperty(IDs::BindingCurve, static_cast<int>(binding.curve), nullptr);
                additionalBinding.setProperty(IDs::BindingValueAtMin, binding.valueAtMin, nullptr);
                additionalBinding.setProperty(IDs::BindingValueAtMax, binding.valueAtMax, nullptr);
                additionalBinding.setProperty(IDs::BindingOriginalValue, binding.originalValue, nullptr);
                additionalBinding.setProperty(IDs::BindingHasStoredOriginal, binding.hasStoredOriginal, nullptr);
                additionalBinding.setProperty(IDs::BindingSteps, binding.steps, nullptr);
                additionalBinding.setProperty(IDs::BindingIsRelative, binding.isRelative, nullptr);
                additionalBinding.setProperty(IDs::BindingRotationOffset, binding.rotationOffset, nullptr);
                additionalBinding.setProperty(IDs::BindingPivotX, binding.pivotPoint.x, nullptr);
                additionalBinding.setProperty(IDs::BindingPivotY, binding.pivotPoint.y, nullptr);
                additionalBinding.setProperty(IDs::BindingOrbitRadius, binding.orbitRadius, nullptr);
                additionalBinding.setProperty(IDs::BindingBipolar, binding.bipolar, nullptr);
                bindingsArray.appendChild(additionalBinding, nullptr);
            }
            tree.appendChild(bindingsArray, nullptr);
        }
    }
    
    // Mask - save if enabled and has a valid mask layer
    if (mask_.enabled)
    {
        auto maskLayerPtr = mask_.maskLayer.lock();
        if (maskLayerPtr)
        {
            juce::ValueTree maskTree(IDs::Mask);
            maskTree.setProperty(IDs::MaskEnabled, mask_.enabled, nullptr);
            maskTree.setProperty(IDs::MaskInverted, mask_.inverted, nullptr);
            maskTree.setProperty(IDs::MaskLayerName, maskLayerPtr->getLayerName(), nullptr);
            tree.appendChild(maskTree, nullptr);
        }
    }
    
    return tree;
}

void Layer::fromValueTree(const juce::ValueTree& tree)
{
    if (!tree.hasType(IDs::Layer))
        return;
    
    // Basic properties
    name_ = tree.getProperty(IDs::Name, "Layer").toString();
    visible_ = tree.getProperty(IDs::Visible, true);
    locked_ = tree.getProperty(IDs::Locked, false);
    index_ = tree.getProperty(IDs::Index, 0);
    
    // Transform
    auto transformTree = tree.getChildWithName(IDs::Transform);
    if (transformTree.isValid())
    {
        transform_.position.x = transformTree.getProperty(IDs::PositionX, 0.0f);
        transform_.position.y = transformTree.getProperty(IDs::PositionY, 0.0f);
        transform_.anchor.x = transformTree.getProperty(IDs::AnchorX, 0.5f);
        transform_.anchor.y = transformTree.getProperty(IDs::AnchorY, 0.5f);
        transform_.rotation = transformTree.getProperty(IDs::Rotation, 0.0f);
        transform_.scale.x = transformTree.getProperty(IDs::ScaleX, 1.0f);
        transform_.scale.y = transformTree.getProperty(IDs::ScaleY, 1.0f);
        transform_.size.x = transformTree.getProperty(IDs::SizeX, 1.0f);
        transform_.size.y = transformTree.getProperty(IDs::SizeY, 1.0f);
    }
    
    // Style
    auto styleTree = tree.getChildWithName(IDs::Style);
    if (styleTree.isValid())
    {
        style_.fillMode = static_cast<FillMode>(static_cast<int>(styleTree.getProperty(IDs::FillMode, 0)));
        style_.fillColour = juce::Colour::fromString(styleTree.getProperty(IDs::FillColour, "ff808080").toString());
        style_.hasFill = styleTree.getProperty(IDs::HasFill, true);  // Default to true for backward compatibility
        style_.hasStroke = styleTree.getProperty(IDs::HasStroke, false);
        style_.strokeColour = juce::Colour::fromString(styleTree.getProperty(IDs::StrokeColour, "ffffffff").toString());
        style_.strokeWidth = styleTree.getProperty(IDs::StrokeWidth, 1.0f);
        style_.strokeCap = static_cast<juce::PathStrokeType::EndCapStyle>(static_cast<int>(styleTree.getProperty(IDs::StrokeCap, juce::PathStrokeType::rounded)));
        style_.strokeJoin = static_cast<juce::PathStrokeType::JointStyle>(static_cast<int>(styleTree.getProperty(IDs::StrokeJoin, juce::PathStrokeType::curved)));
        style_.strokeAlignment = static_cast<StrokeAlignment>(static_cast<int>(styleTree.getProperty(IDs::StrokeAlignment, static_cast<int>(StrokeAlignment::Center))));
        style_.cornerRadius = styleTree.getProperty(IDs::CornerRadius, 0.0f);
        style_.opacity = styleTree.getProperty(IDs::Opacity, 1.0f);
        style_.blendMode = static_cast<BlendMode>(static_cast<int>(styleTree.getProperty(IDs::BlendModeID, 0)));
        
        // Shadow
        auto shadowTree = styleTree.getChildWithName(IDs::Shadow);
        if (shadowTree.isValid())
        {
            style_.shadow.enabled = shadowTree.getProperty(IDs::ShadowEnabled, false);
            style_.shadow.colour = juce::Colour::fromString(shadowTree.getProperty(IDs::ShadowColour, "80000000").toString());
            style_.shadow.offset.x = shadowTree.getProperty(IDs::ShadowOffsetX, 2.0f);
            style_.shadow.offset.y = shadowTree.getProperty(IDs::ShadowOffsetY, 2.0f);
            style_.shadow.radius = shadowTree.getProperty(IDs::ShadowRadius, 4.0f);
        }
        
        // Gradient
        auto gradientTree = styleTree.getChildWithName(IDs::FillGradient);
        if (gradientTree.isValid())
        {
            style_.fillGradient.enabled = gradientTree.getProperty(IDs::GradientEnabled, false);
            style_.fillGradient.type = static_cast<GradientType>(static_cast<int>(gradientTree.getProperty(IDs::GradientType, 0)));
            style_.fillGradient.isRadial = gradientTree.getProperty(IDs::GradientIsRadial, false);
            style_.fillGradient.colour1 = juce::Colour::fromString(gradientTree.getProperty(IDs::GradientColour1, "ffffffff").toString());
            style_.fillGradient.colour2 = juce::Colour::fromString(gradientTree.getProperty(IDs::GradientColour2, "ff000000").toString());
            style_.fillGradient.start.x = gradientTree.getProperty(IDs::GradientStartX, 0.0f);
            style_.fillGradient.start.y = gradientTree.getProperty(IDs::GradientStartY, 0.0f);
            style_.fillGradient.end.x = gradientTree.getProperty(IDs::GradientEndX, 1.0f);
            style_.fillGradient.end.y = gradientTree.getProperty(IDs::GradientEndY, 1.0f);
        }
    }
    
    // Effects (advanced effects like blur, glow, bevel)
    auto effectsTree = tree.getChildWithName("Effects");
    if (effectsTree.isValid())
    {
        effects_.fromValueTree(effectsTree);
        effectsCacheDirty_ = true;
    }
    
    // Slider role and sizing mode
    sliderRole_ = static_cast<SliderLayerRole>(static_cast<int>(tree.getProperty(IDs::SliderRole, 0)));
    sizingMode_ = static_cast<SizingMode>(static_cast<int>(tree.getProperty(IDs::SizingModeID, 0)));
    
    // Value Bindings - load primary binding (backward compatible)
    valueBindings_.clear();
    auto bindingTree = tree.getChildWithName(IDs::ValueBinding);
    if (bindingTree.isValid())
    {
        ValueBinding binding;
        binding.enabled = bindingTree.getProperty(IDs::BindingEnabled, false);
        binding.property = static_cast<ValueBindingProperty>(static_cast<int>(bindingTree.getProperty(IDs::BindingProperty, 0)));
        binding.curve = static_cast<ValueBindingCurve>(static_cast<int>(bindingTree.getProperty(IDs::BindingCurve, 0)));
        binding.valueAtMin = bindingTree.getProperty(IDs::BindingValueAtMin, 0.0f);
        binding.valueAtMax = bindingTree.getProperty(IDs::BindingValueAtMax, 1.0f);
        binding.originalValue = bindingTree.getProperty(IDs::BindingOriginalValue, 0.0f);
        binding.hasStoredOriginal = bindingTree.getProperty(IDs::BindingHasStoredOriginal, false);
        binding.steps = bindingTree.getProperty(IDs::BindingSteps, 10);
        binding.isRelative = bindingTree.getProperty(IDs::BindingIsRelative, true);
        binding.rotationOffset = bindingTree.getProperty(IDs::BindingRotationOffset, 0.0f);
        binding.pivotPoint.x = bindingTree.getProperty(IDs::BindingPivotX, 0.5f);
        binding.pivotPoint.y = bindingTree.getProperty(IDs::BindingPivotY, 0.5f);
        binding.orbitRadius = bindingTree.getProperty(IDs::BindingOrbitRadius, 0.3f);
        binding.bipolar = bindingTree.getProperty(IDs::BindingBipolar, false);
        
        valueBindings_.push_back(binding);
        
        // Base transform
        auto baseTransformTree = bindingTree.getChildWithName(IDs::BaseTransform);
        if (baseTransformTree.isValid())
        {
            baseTransform_.position.x = baseTransformTree.getProperty(IDs::PositionX, 0.0f);
            baseTransform_.position.y = baseTransformTree.getProperty(IDs::PositionY, 0.0f);
            baseTransform_.rotation = baseTransformTree.getProperty(IDs::Rotation, 0.0f);
            baseTransform_.scale.x = baseTransformTree.getProperty(IDs::ScaleX, 1.0f);
            baseTransform_.scale.y = baseTransformTree.getProperty(IDs::ScaleY, 1.0f);
        }
        baseOpacity_ = bindingTree.getProperty(IDs::BaseOpacity, 1.0f);
    }
    
    // Load additional bindings if present
    auto bindingsArray = tree.getChildWithName(IDs::ValueBindings);
    if (bindingsArray.isValid())
    {
        for (int i = 0; i < bindingsArray.getNumChildren(); ++i)
        {
            auto additionalBindingTree = bindingsArray.getChild(i);
            if (additionalBindingTree.hasType(IDs::ValueBinding))
            {
                ValueBinding binding;
                binding.enabled = additionalBindingTree.getProperty(IDs::BindingEnabled, false);
                binding.property = static_cast<ValueBindingProperty>(static_cast<int>(additionalBindingTree.getProperty(IDs::BindingProperty, 0)));
                binding.curve = static_cast<ValueBindingCurve>(static_cast<int>(additionalBindingTree.getProperty(IDs::BindingCurve, 0)));
                binding.valueAtMin = additionalBindingTree.getProperty(IDs::BindingValueAtMin, 0.0f);
                binding.valueAtMax = additionalBindingTree.getProperty(IDs::BindingValueAtMax, 1.0f);
                binding.originalValue = additionalBindingTree.getProperty(IDs::BindingOriginalValue, 0.0f);
                binding.hasStoredOriginal = additionalBindingTree.getProperty(IDs::BindingHasStoredOriginal, false);
                binding.steps = additionalBindingTree.getProperty(IDs::BindingSteps, 10);
                binding.isRelative = additionalBindingTree.getProperty(IDs::BindingIsRelative, true);
                binding.rotationOffset = additionalBindingTree.getProperty(IDs::BindingRotationOffset, 0.0f);
                binding.pivotPoint.x = additionalBindingTree.getProperty(IDs::BindingPivotX, 0.5f);
                binding.pivotPoint.y = additionalBindingTree.getProperty(IDs::BindingPivotY, 0.5f);
                binding.orbitRadius = additionalBindingTree.getProperty(IDs::BindingOrbitRadius, 0.3f);
                binding.bipolar = additionalBindingTree.getProperty(IDs::BindingBipolar, false);
                
                valueBindings_.push_back(binding);
            }
        }
    }
    
    // Mask - load settings (actual layer reference will be resolved by LayerStack after all layers are loaded)
    auto maskTree = tree.getChildWithName(IDs::Mask);
    if (maskTree.isValid())
    {
        mask_.enabled = maskTree.getProperty(IDs::MaskEnabled, false);
        mask_.inverted = maskTree.getProperty(IDs::MaskInverted, false);
        pendingMaskLayerName_ = maskTree.getProperty(IDs::MaskLayerName, "").toString();
    }
    else
    {
        mask_.clear();
        pendingMaskLayerName_.clear();
    }
    
    setVisible(visible_);
    setName(name_);
}

//==============================================================================
// Component Overrides

void Layer::paint(juce::Graphics& g)
{
    if (!visible_ || style_.opacity <= 0.0f)
        return;
    
    paintLayer(g, getLocalBounds().toFloat(), true);
}

void Layer::paintLayer(juce::Graphics& g, const juce::Rectangle<float>& bounds, bool applyOpacity)
{
    if (!visible_ || style_.opacity <= 0.0f)
        return;
    
    // Cache bounds for orbit calculations
    cachedRenderBounds_ = bounds;
    
    // Ensure orbit bindings use the actual render bounds so the orbit path stays circular
    auto applyOrbitBinding = [&](const ValueBinding& binding) -> bool
    {
        if (!binding.enabled || binding.property != ValueBindingProperty::RotationAroundPoint)
            return false;

        const float propertyValue = binding.computePropertyValue(sliderValue_);
        const float angle = propertyValue + binding.rotationOffset;
        const float adjustedAngle = angle - juce::MathConstants<float>::halfPi;

        const float refDim = getReferenceDimension(bounds, sizingMode_);
        const float width = bounds.getWidth();
        const float height = bounds.getHeight();

        float radiusX = binding.orbitRadius;
        float radiusY = binding.orbitRadius;

        if (width > 0.0f && height > 0.0f)
        {
            radiusX = binding.orbitRadius * refDim / width;
            radiusY = binding.orbitRadius * refDim / height;
        }

        transform_.position.x = binding.pivotPoint.x + std::cos(adjustedAngle) * radiusX;
        transform_.position.y = binding.pivotPoint.y + std::sin(adjustedAngle) * radiusY;
        return true;
    };

    for (const auto& binding : valueBindings_)
    {
        if (applyOrbitBinding(binding))
            break; // Only one orbit binding is applied
    }

    // Set the render opacity for helper functions to use
    renderOpacity_ = applyOpacity ? style_.opacity : 1.0f;
    
    // Calculate target bounds based on Transform (normalized position/size) relative to 'bounds'
    // Use sizing mode to determine reference dimension - this preserves aspect ratio
    const float referenceDim = getReferenceDimension(bounds, sizingMode_);
    float w = referenceDim * transform_.size.x * transform_.scale.x;
    float h = referenceDim * transform_.size.y * transform_.scale.y;
    
    float pivotX = bounds.getX() + bounds.getWidth() * transform_.position.x;
    float pivotY = bounds.getY() + bounds.getHeight() * transform_.position.y;
    
    float x = pivotX - w * transform_.anchor.x;
    float y = pivotY - h * transform_.anchor.y;
    
    juce::Rectangle<float> targetBounds(x, y, w, h);
    
    juce::Graphics::ScopedSaveState saveState(g);
    
    // Apply rotation around pivot point
    if (transform_.rotation != 0.0f)
    {
        g.addTransform(juce::AffineTransform().rotated(transform_.rotation, pivotX, pivotY));
    }
    
    // Check if we need to apply layer effects OR masking
    bool hasMask = mask_.enabled && !mask_.maskLayer.expired();
    
    if (effects_.hasAnyEffect() || hasMask)
    {
        // For effects/masking, we need to render to an intermediate image
        // Calculate padding needed for effects that extend beyond the layer
        const int shadowPadding = effects_.dropShadow.enabled ? 
            effects_.dropShadow.blurRadius * 2 + 
            static_cast<int>(std::max(std::abs(effects_.dropShadow.offset.x), 
                                       std::abs(effects_.dropShadow.offset.y))) + 20 : 0;
        const int glowPadding = effects_.outerGlow.enabled ? 
            effects_.outerGlow.blurRadius * 2 + 10 : 0;
        const int effectPadding = std::max(shadowPadding, glowPadding);
        
        const int imgW = static_cast<int>(std::ceil(w)) + effectPadding * 2;
        const int imgH = static_cast<int>(std::ceil(h)) + effectPadding * 2;
        
        // Check if we can reuse the effects cache
        bool canUseCache = !effectsCacheDirty_ && effectsCache_.isValid() 
            && effectsCache_.getWidth() == imgW 
            && effectsCache_.getHeight() == imgH
            && lastEffectsBounds_ == targetBounds;
        
        if (imgW > 0 && imgH > 0)
        {
            juce::Image finalImage;
            
            if (canUseCache)
            {
                // Use cached effects image
                finalImage = effectsCache_;
            }
            else
            {
                // Need to regenerate the effects
                // Create intermediate image for layer content
                juce::Image layerImage(juce::Image::ARGB, imgW, imgH, true);
                
                {
                    juce::Graphics imgG(layerImage);
                    imgG.setOpacity(1.0f);  // Full opacity for intermediate render
                    
                    // Temporarily set renderOpacity_ to 1.0 for intermediate rendering
                    const float savedRenderOpacity = renderOpacity_;
                    renderOpacity_ = 1.0f;
                    
                    // Render layer content centered in the padded image
                    juce::Rectangle<float> imgBounds(
                        static_cast<float>(effectPadding), 
                        static_cast<float>(effectPadding), 
                        w, h
                    );
                    renderLayer(imgG, imgBounds);
                    
                    renderOpacity_ = savedRenderOpacity;
                }
                
                // Start with empty image
                finalImage = juce::Image(juce::Image::ARGB, imgW, imgH, true);
                
                // 1. Drop Shadow (rendered behind everything)
                if (effects_.dropShadow.enabled)
                {
                    const int offsetX = static_cast<int>(effects_.dropShadow.offset.x);
                    const int offsetY = static_cast<int>(effects_.dropShadow.offset.y);
                    const int blurRadius = effects_.dropShadow.blurRadius;
                    const auto shadowColour = effects_.dropShadow.colour;
                    
                    // Create shadow silhouette
                    juce::Image shadowSilhouette(juce::Image::ARGB, imgW, imgH, true);
                    
                    {
                        juce::Image::BitmapData srcData(layerImage, juce::Image::BitmapData::readOnly);
                        juce::Image::BitmapData shadowData(shadowSilhouette, juce::Image::BitmapData::readWrite);
                        
                        // Copy alpha channel with offset, colorize with shadow colour
                        for (int sy = 0; sy < imgH; ++sy)
                        {
                            int srcY = sy - offsetY;
                            if (srcY >= 0 && srcY < imgH)
                            {
                                for (int sx = 0; sx < imgW; ++sx)
                                {
                                    int srcX = sx - offsetX;
                                    if (srcX >= 0 && srcX < imgW)
                                    {
                                        auto srcPixel = srcData.getPixelColour(srcX, srcY);
                                        float alpha = srcPixel.getFloatAlpha();
                                        if (alpha > 0.0f)
                                        {
                                            shadowData.setPixelColour(sx, sy, shadowColour.withAlpha(alpha));
                                        }
                                    }
                                }
                            }
                        }
                    }
                    
                    // Blur the shadow
                    if (blurRadius > 1)
                        ImageEffects::applyStackBlur(shadowSilhouette, blurRadius);
                    
                    // Draw shadow to final image
                    juce::Graphics finalG(finalImage);
                    finalG.drawImageAt(shadowSilhouette, 0, 0);
                }
            
                // 2. Outer Glow (rendered behind layer but on top of shadow)
                if (effects_.outerGlow.enabled)
                {
                    juce::Image glowImage = ImageEffects::createOuterGlow(
                        layerImage,
                        effects_.outerGlow.colour,
                        effects_.outerGlow.blurRadius,
                        effects_.outerGlow.spread
                    );
                    
                    // Draw glow centered
                    juce::Graphics finalG(finalImage);
                    int glowOffsetX = (glowImage.getWidth() - imgW) / 2;
                    int glowOffsetY = (glowImage.getHeight() - imgH) / 2;
                    finalG.drawImageAt(glowImage, -glowOffsetX, -glowOffsetY);
                }
                
                // 3. Draw the layer content on top
                {
                    juce::Graphics finalG(finalImage);
                    finalG.drawImageAt(layerImage, 0, 0);
                }
                
                // 4. Apply effects that modify the layer itself (blur, color overlay, bevel, inner glow)
                finalImage = applyLayerEffects(finalImage, targetBounds);
                
                // 5. Apply Mask
                if (hasMask)
                {
                    auto maskLayerPtr = mask_.maskLayer.lock();
                    if (maskLayerPtr)
                    {
                        // Create mask image of same size as finalImage - use ARGB for proper color channel rendering
                        juce::Image maskImg(juce::Image::ARGB, imgW, imgH, true);
                        
                        {
                            juce::Graphics mg(maskImg);
                            const int drawXm = static_cast<int>(x) - effectPadding;
                            const int drawYm = static_cast<int>(y) - effectPadding;
                            mg.addTransform(juce::AffineTransform::translation(-drawXm, -drawYm));
                            // Render mask layer at full opacity to get proper alpha channel
                            maskLayerPtr->paintLayer(mg, bounds, true);
                        }
                        
                        // Apply mask to finalImage by using mask's alpha channel
                        juce::Image::BitmapData maskData(maskImg, juce::Image::BitmapData::readOnly);
                        juce::Image::BitmapData finalData(finalImage, juce::Image::BitmapData::readWrite);
                        
                        for (int py = 0; py < finalImage.getHeight(); ++py)
                        {
                            for (int px = 0; px < finalImage.getWidth(); ++px)
                            {
                                // Use the alpha channel of the mask to determine visibility
                                float maskAlpha = maskData.getPixelColour(px, py).getFloatAlpha();
                                
                                // For normal mask: visible where mask is opaque
                                // For inverted: visible where mask is transparent
                                if (mask_.inverted)
                                    maskAlpha = 1.0f - maskAlpha;
                                
                                auto col = finalData.getPixelColour(px, py);
                                finalData.setPixelColour(px, py, col.withMultipliedAlpha(maskAlpha));
                            }
                        }
                    }
                }
                
                // Cache the effects result for next frame
                effectsCache_ = finalImage;
                effectsCacheDirty_ = false;
                lastEffectsBounds_ = targetBounds;
            }
            
            // 6. Draw final result with opacity (blend mode is handled by LayerStack)
            const int drawX = static_cast<int>(x) - effectPadding;
            const int drawY = static_cast<int>(y) - effectPadding;
            
            g.setOpacity(renderOpacity_);
            g.drawImageAt(finalImage, drawX, drawY);
        }
    }
    else
    {
        // No effects - render directly
        // Note: opacity is applied inside renderLayer via applyFill/applyStroke using renderOpacity_
        renderLayer(g, targetBounds);
    }
}

void Layer::resized()
{
    // Base implementation - subclasses can override
}

//==============================================================================
// Rendering Helpers

void Layer::applyFill(juce::Graphics& g, const juce::Rectangle<float>& bounds) const
{
    // Use renderOpacity_ which is set by paintLayer (supports blend mode rendering)
    const float opacity = renderOpacity_;
    
    switch (style_.fillMode)
    {
        case FillMode::Solid:
            g.setColour(style_.fillColour.withMultipliedAlpha(opacity));
            break;
            
        case FillMode::LinearGradient:
        {
            auto gradient = style_.fillGradient.toColourGradient(bounds);
            // Multiply alpha of gradient colours by layer opacity
            for (int i = 0; i < gradient.getNumColours(); ++i)
            {
                auto colour = gradient.getColour(i).withMultipliedAlpha(opacity);
                gradient.setColour(i, colour);
            }
            g.setGradientFill(gradient);
            break;
        }
            
        case FillMode::RadialGradient:
        {
            auto gradient = style_.fillGradient.toColourGradient(bounds);
            for (int i = 0; i < gradient.getNumColours(); ++i)
            {
                auto colour = gradient.getColour(i).withMultipliedAlpha(opacity);
                gradient.setColour(i, colour);
            }
            g.setGradientFill(gradient);
            break;
        }
            
        case FillMode::ConicGradient:
        {
            // JUCE doesn't have native conic gradient, fallback to radial
            auto gradient = style_.fillGradient.toColourGradient(bounds);
            for (int i = 0; i < gradient.getNumColours(); ++i)
            {
                auto colour = gradient.getColour(i).withMultipliedAlpha(opacity);
                gradient.setColour(i, colour);
            }
            g.setGradientFill(gradient);
            break;
        }
            
        case FillMode::None:
        default:
            break;
    }
}

void Layer::applyStroke(juce::Graphics& g, const juce::Path& path, const juce::Rectangle<float>& bounds) const
{
    if (style_.hasStroke && style_.strokeWidth > 0.0f)
    {
        // Convert relative stroke width to pixels
        const float strokeWidthPixels = style_.getStrokeWidthPixels(bounds, sizingMode_);
        
        // Guard against invalid stroke width (must be positive for PathStrokeType)
        if (strokeWidthPixels <= 0.0f || !std::isfinite(strokeWidthPixels))
            return;
        
        // Guard against empty or degenerate paths
        if (path.isEmpty())
            return;
        
        const auto pathBounds = path.getBounds();
        // Relaxed check: allow small paths but prevent completely degenerate ones
        if (pathBounds.isEmpty() || (pathBounds.getWidth() <= 0.0f && pathBounds.getHeight() <= 0.0f))
            return;
        
        // Use renderOpacity_ which is set by paintLayer (supports blend mode rendering)
        g.setColour(style_.strokeColour.withMultipliedAlpha(renderOpacity_));
        
        // Note: Stroke alignment is handled by adjusting the shape bounds before path creation,
        // not by offsetting the path here. See ShapeLayer::renderLayer() for implementation.
        
        // For paths with very acute angles (like stars), Mitered joins can produce huge spikes
        // or cause assertions in JUCE's stroker. We clamp the miter limit or switch to Beveled
        // if the path is likely to be problematic.
        // For now, we just ensure a safe miter limit.
        juce::PathStrokeType strokeType(
            strokeWidthPixels,
            style_.strokeJoin,
            style_.strokeCap
        );
        
        if (!style_.strokeDashPattern.empty())
        {
            juce::Path dashedPath;
            strokeType.createDashedStroke(dashedPath, path, style_.strokeDashPattern.data(), static_cast<int>(style_.strokeDashPattern.size()));
            if (!dashedPath.isEmpty())
                g.fillPath(dashedPath);
        }
        else
        {
            // Use createStrokedPath + fillPath which is generally more robust than strokePath
            // and allows us to catch empty result paths
            juce::Path strokedPath;
            
            // Use a flattening tolerance based on stroke width for better quality
            const float tolerance = std::max(0.5f, strokeWidthPixels * 0.1f);
            strokeType.createStrokedPath(strokedPath, path, juce::AffineTransform(), tolerance);
            
            if (!strokedPath.isEmpty())
            {
                g.fillPath(strokedPath);
            }
        }
    }
}

void Layer::renderShadow(juce::Graphics& g, const juce::Path& shapePath, const juce::Rectangle<float>& bounds) const
{
    if (!style_.shadow.enabled)
        return;
    
    // Convert relative shadow values to pixels
    const auto offsetPixels = style_.shadow.getOffsetPixels(bounds, sizingMode_);
    const float radiusPixels = style_.shadow.getRadiusPixels(bounds, sizingMode_);
    
    // Create shadow image
    const auto shadowBounds = shapePath.getBounds().expanded(radiusPixels * 2);
    const int shadowSize = static_cast<int>(std::ceil(radiusPixels));
    
    if (shadowBounds.isEmpty() || shadowSize <= 0)
        return;
    
    juce::Image shadowImage(juce::Image::SingleChannel, 
                            static_cast<int>(shadowBounds.getWidth()) + shadowSize * 2, 
                            static_cast<int>(shadowBounds.getHeight()) + shadowSize * 2, 
                            true);
    
    {
        juce::Graphics shadowG(shadowImage);
        shadowG.setColour(juce::Colours::white);
        
        auto translatedPath = shapePath;
        translatedPath.applyTransform(juce::AffineTransform::translation(
            -shadowBounds.getX() + shadowSize + offsetPixels.x,
            -shadowBounds.getY() + shadowSize + offsetPixels.y
        ));
        
        shadowG.fillPath(translatedPath);
    }
    
    // Apply blur using JUCE's built-in shadow approach
    juce::DropShadow shadow(style_.shadow.colour, shadowSize, 
                           juce::Point<int>(static_cast<int>(offsetPixels.x), 
                                           static_cast<int>(offsetPixels.y)));
    shadow.drawForPath(g, shapePath);
}

void Layer::applyMask(juce::Graphics& g, const juce::Image& content) const
{
    if (!mask_.enabled || !mask_.maskImage.isValid())
        return;
    
    // Apply mask using alpha channel
    // This is a simplified implementation
    juce::Image masked = content.createCopy();
    
    for (int y = 0; y < masked.getHeight(); ++y)
    {
        for (int x = 0; x < masked.getWidth(); ++x)
        {
            auto maskPixel = mask_.maskImage.getPixelAt(x, y);
            auto contentPixel = masked.getPixelAt(x, y);
            
            float maskAlpha = mask_.inverted 
                ? 1.0f - maskPixel.getFloatAlpha() 
                : maskPixel.getFloatAlpha();
            
            masked.setPixelAt(x, y, contentPixel.withMultipliedAlpha(maskAlpha));
        }
    }
    
    g.drawImageAt(masked, 0, 0);
}

juce::Image Layer::applyLayerEffects(const juce::Image& layerImage, 
                                      const juce::Rectangle<float>& bounds) const
{
    if (!effects_.hasAnyEffect() || !layerImage.isValid())
        return layerImage;
    
    juce::Image result = layerImage.createCopy();
    const int w = result.getWidth();
    const int h = result.getHeight();
    
    // Apply effects in order (similar to Photoshop layer effects)
    
    // 1. Gaussian Blur (affects the layer itself)
    if (effects_.gaussianBlur.enabled && effects_.gaussianBlur.radius > 1)
    {
        ImageEffects::applyStackBlur(result, effects_.gaussianBlur.radius);
    }
    
    // 2. Colour Overlay
    if (effects_.colourOverlay.enabled)
    {
        ImageEffects::applyColourOverlay(result, effects_.colourOverlay.colour, 
                                         effects_.colourOverlay.opacity,
                                         effects_.colourOverlay.blendMode);
    }
    
    // 3. Bevel & Emboss (applied to layer content)
    if (effects_.bevelEmboss.enabled)
    {
        result = ImageEffects::createBevelEmboss(result,
                                                  effects_.bevelEmboss.depth,
                                                  effects_.bevelEmboss.lightAngle,
                                                  effects_.bevelEmboss.highlightColour,
                                                  effects_.bevelEmboss.shadowColour,
                                                  effects_.bevelEmboss.softness);
    }
    
    // 4. Inner Glow (drawn inside the shape)
    if (effects_.innerGlow.enabled)
    {
        result = ImageEffects::createInnerGlow(result,
                                                effects_.innerGlow.colour,
                                                effects_.innerGlow.blurRadius,
                                                effects_.innerGlow.intensity);
    }
    
    return result;
}

juce::Image Layer::renderToImage(const juce::Rectangle<float>& bounds) const
{
    const int w = static_cast<int>(std::ceil(bounds.getWidth()));
    const int h = static_cast<int>(std::ceil(bounds.getHeight()));
    
    if (w <= 0 || h <= 0)
        return {};
    
    juce::Image image(juce::Image::ARGB, w, h, true);
    
    {
        juce::Graphics g(image);
        // Translate so bounds.getTopLeft() is at origin
        g.addTransform(juce::AffineTransform::translation(-bounds.getX(), -bounds.getY()));
        
        // Render the layer without effects (we'll apply them afterwards)
        const_cast<Layer*>(this)->renderLayer(g, bounds);
    }
    
    return image;
}

juce::Rectangle<float> Layer::getLayerBounds() const noexcept
{
    return getLocalBounds().toFloat();
}

//==============================================================================
// JSON Serialization (Modern)

json Layer::toJson() const
{
    json j;
    
    // Basic properties
    j["name"] = name_.toStdString();
    j["type"] = static_cast<int>(getType());
    j["visible"] = visible_;
    j["locked"] = locked_;
    j["index"] = index_;
    
    // Transform
    j["transform"] = {
        {"position", transform_.position},
        {"anchor", transform_.anchor},
        {"rotation", transform_.rotation},
        {"scale", transform_.scale},
        {"size", transform_.size}
    };
    
    // Style
    j["style"] = style_;
    
    // Effects (only if any effect is enabled)
    if (effects_.hasAnyEffect())
    {
        json effectsJson;
        
        // Drop Shadow
        if (effects_.dropShadow.enabled)
        {
            effectsJson["dropShadow"] = {
                {"enabled", true},
                {"colour", effects_.dropShadow.colour},
                {"offset", effects_.dropShadow.offset},
                {"blurRadius", effects_.dropShadow.blurRadius},
                {"spread", effects_.dropShadow.spread}
            };
        }
        
        // Outer Glow
        if (effects_.outerGlow.enabled)
        {
            effectsJson["outerGlow"] = {
                {"enabled", true},
                {"colour", effects_.outerGlow.colour},
                {"blurRadius", effects_.outerGlow.blurRadius},
                {"spread", effects_.outerGlow.spread}
            };
        }
        
        // Inner Glow
        if (effects_.innerGlow.enabled)
        {
            effectsJson["innerGlow"] = {
                {"enabled", true},
                {"colour", effects_.innerGlow.colour},
                {"blurRadius", effects_.innerGlow.blurRadius},
                {"intensity", effects_.innerGlow.intensity}
            };
        }
        
        // Bevel & Emboss
        if (effects_.bevelEmboss.enabled)
        {
            effectsJson["bevelEmboss"] = {
                {"enabled", true},
                {"depth", effects_.bevelEmboss.depth},
                {"softness", effects_.bevelEmboss.softness},
                {"lightAngle", effects_.bevelEmboss.lightAngle},
                {"highlightColour", effects_.bevelEmboss.highlightColour},
                {"shadowColour", effects_.bevelEmboss.shadowColour}
            };
        }
        
        // Gaussian Blur
        if (effects_.gaussianBlur.enabled)
        {
            effectsJson["gaussianBlur"] = {
                {"enabled", true},
                {"radius", effects_.gaussianBlur.radius}
            };
        }
        
        // Colour Overlay
        if (effects_.colourOverlay.enabled)
        {
            effectsJson["colourOverlay"] = {
                {"enabled", true},
                {"colour", effects_.colourOverlay.colour},
                {"opacity", effects_.colourOverlay.opacity},
                {"blendMode", effects_.colourOverlay.blendMode}
            };
        }
        
        j["effects"] = effectsJson;
    }
    
    // Slider role and sizing mode
    j["sliderRole"] = static_cast<int>(sliderRole_);
    j["sizingMode"] = static_cast<int>(sizingMode_);
    
    // Value Bindings
    if (!valueBindings_.empty())
    {
        json bindingsArray = json::array();
        for (const auto& binding : valueBindings_)
        {
            if (binding.enabled || binding.property != ValueBindingProperty::None)
            {
                bindingsArray.push_back(binding);
            }
        }
        
        if (!bindingsArray.empty())
        {
            j["valueBindings"] = bindingsArray;
            
            // Base transform (for value binding)
            j["baseTransform"] = {
                {"position", baseTransform_.position},
                {"rotation", baseTransform_.rotation},
                {"scale", baseTransform_.scale}
            };
            j["baseOpacity"] = baseOpacity_;
        }
    }
    
    // Mask (if enabled and valid)
    if (mask_.enabled)
    {
        auto maskLayerPtr = mask_.maskLayer.lock();
        if (maskLayerPtr)
        {
            j["mask"] = {
                {"enabled", true},
                {"inverted", mask_.inverted},
                {"maskLayerName", maskLayerPtr->getLayerName().toStdString()}
            };
        }
    }
    
    return j;
}

void Layer::fromJson(const json& j)
{
    // Basic properties
    if (j.contains("name"))
        name_ = j["name"].get<std::string>();
    
    visible_ = j.value("visible", true);
    locked_ = j.value("locked", false);
    index_ = j.value("index", 0);
    
    // Transform
    if (j.contains("transform"))
    {
        const auto& trans = j["transform"];
        if (trans.contains("position"))
            trans["position"].get_to(transform_.position);
        if (trans.contains("anchor"))
            trans["anchor"].get_to(transform_.anchor);
        transform_.rotation = trans.value("rotation", 0.0f);
        if (trans.contains("scale"))
            trans["scale"].get_to(transform_.scale);
        if (trans.contains("size"))
            trans["size"].get_to(transform_.size);
    }
    
    // Style
    if (j.contains("style"))
    {
        j["style"].get_to(style_);
    }
    
    // Effects
    if (j.contains("effects"))
    {
        const auto& effectsJson = j["effects"];
        
        // Drop Shadow
        if (effectsJson.contains("dropShadow"))
        {
            const auto& ds = effectsJson["dropShadow"];
            effects_.dropShadow.enabled = ds.value("enabled", false);
            ds["colour"].get_to(effects_.dropShadow.colour);
            if (ds.contains("offset"))
                ds["offset"].get_to(effects_.dropShadow.offset);
            effects_.dropShadow.blurRadius = ds.value("blurRadius", 8);
            effects_.dropShadow.spread = ds.value("spread", 0.0f);
        }
        
        // Outer Glow
        if (effectsJson.contains("outerGlow"))
        {
            const auto& og = effectsJson["outerGlow"];
            effects_.outerGlow.enabled = og.value("enabled", false);
            og["colour"].get_to(effects_.outerGlow.colour);
            effects_.outerGlow.blurRadius = og.value("blurRadius", 10);
            effects_.outerGlow.spread = og.value("spread", 0.5f);
        }
        
        // Inner Glow
        if (effectsJson.contains("innerGlow"))
        {
            const auto& ig = effectsJson["innerGlow"];
            effects_.innerGlow.enabled = ig.value("enabled", false);
            ig["colour"].get_to(effects_.innerGlow.colour);
            effects_.innerGlow.blurRadius = ig.value("blurRadius", 5.0f);
            effects_.innerGlow.intensity = ig.value("intensity", 0.5f);
        }
        
        // Bevel & Emboss
        if (effectsJson.contains("bevelEmboss"))
        {
            const auto& be = effectsJson["bevelEmboss"];
            effects_.bevelEmboss.enabled = be.value("enabled", false);
            effects_.bevelEmboss.depth = be.value("depth", 3.0f);
            effects_.bevelEmboss.softness = be.value("softness", 2);
            effects_.bevelEmboss.lightAngle = be.value("lightAngle", juce::MathConstants<float>::pi * 0.75f);
            be["highlightColour"].get_to(effects_.bevelEmboss.highlightColour);
            be["shadowColour"].get_to(effects_.bevelEmboss.shadowColour);
        }
        
        // Gaussian Blur
        if (effectsJson.contains("gaussianBlur"))
        {
            const auto& gb = effectsJson["gaussianBlur"];
            effects_.gaussianBlur.enabled = gb.value("enabled", false);
            effects_.gaussianBlur.radius = gb.value("radius", 5.0f);
        }
        
        // Colour Overlay
        if (effectsJson.contains("colourOverlay"))
        {
            const auto& co = effectsJson["colourOverlay"];
            effects_.colourOverlay.enabled = co.value("enabled", false);
            co["colour"].get_to(effects_.colourOverlay.colour);
            effects_.colourOverlay.opacity = co.value("opacity", 1.0f);
            effects_.colourOverlay.blendMode = co.value("blendMode", BlendMode::Normal);
        }
    }
    
    // Slider role and sizing mode
    sliderRole_ = static_cast<SliderLayerRole>(j.value("sliderRole", 0));
    sizingMode_ = static_cast<SizingMode>(j.value("sizingMode", static_cast<int>(SizingMode::MinDimension)));
    
    // Value Bindings
    valueBindings_.clear();
    if (j.contains("valueBindings"))
    {
        const auto& bindingsArray = j["valueBindings"];
        for (const auto& bindingJson : bindingsArray)
        {
            ValueBinding binding;
            bindingJson.get_to(binding);
            valueBindings_.push_back(binding);
        }
        
        // Base transform
        if (j.contains("baseTransform"))
        {
            const auto& baseTrans = j["baseTransform"];
            if (baseTrans.contains("position"))
                baseTrans["position"].get_to(baseTransform_.position);
            baseTransform_.rotation = baseTrans.value("rotation", 0.0f);
            if (baseTrans.contains("scale"))
                baseTrans["scale"].get_to(baseTransform_.scale);
        }
        
        baseOpacity_ = j.value("baseOpacity", 1.0f);
    }
    
    // Mask (resolved later by LayerStack when all layers are loaded)
    if (j.contains("mask") && j["mask"].value("enabled", false))
    {
        mask_.enabled = true;
        mask_.inverted = j["mask"].value("inverted", false);
        pendingMaskLayerName_ = j["mask"].value("maskLayerName", "");
    }
    
    // Update visibility
    setVisible(visible_);
}

} // namespace zaplab

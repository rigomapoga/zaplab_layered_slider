/*
  ==============================================================================

    Layer.h
    Created: 30 Nov 2025
    Updated: 11 Dec 2025
    Author:  DevTools Refactored Layer System
    
    Description:
    A modern, clean layer system for slider/knob design. This provides the base
    architecture for composable visual layers using JUCE best practices and
    modern C++17/20 features.
    
    This file now includes modular headers for better organization:
    - LayerTypes.h:   Core enums and types (LayerType, ShapeType, Transform, etc.)
    - LayerStyle.h:   Style properties (fill, stroke, shadow, blend mode)
    - ValueBinding.h: Value binding configuration for slider integration
    - ImageEffects.h: Effect structs and blend mode utilities

  ==============================================================================
*/

#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "LayerTypes.h"
#include "LayerStyle.h"
#include "ValueBinding.h"
#include "ImageEffects.h"
#include "JsonSerialization.h"
#include <functional>
#include <vector>

namespace zaplab
{

// Note: The following types are now defined in separate headers for modularity:
// - LayerTypes.h: LayerType, ShapeType, SliderLayerRole, FillMode, SizingMode
//                 Transform, Shadow, Gradient, LayerMask, AnimationState, InteractionState
// - LayerStyle.h: LayerStyle
// - ValueBinding.h: ValueBindingProperty, ValueBindingCurve, ValueBinding
// - ImageEffects.h: BlendMode, LayerEffects

//==============================================================================
/**
 * @brief Base Layer class - the foundation for all visual layers
 * 
 * Design principles:
 * - Single responsibility: handles core layer properties only
 * - Composition over inheritance for styling/effects
 * - Value semantics where possible
 * - Clear separation between state and rendering
 */
class Layer : public juce::Component
{
public:
    //==========================================================================
    // Construction & Destruction
    
    explicit Layer(const juce::String& name = "Layer");
    ~Layer() override = default;
    
    // Non-copyable, non-movable (Component base is non-movable)
    Layer(const Layer&) = delete;
    Layer& operator=(const Layer&) = delete;
    
    //==========================================================================
    // Layer Type Information
    
    [[nodiscard]] virtual LayerType getType() const noexcept = 0;
    [[nodiscard]] virtual juce::String getTypeName() const noexcept = 0;
    
    //==========================================================================
    // Visibility & Basic Properties
    
    void setLayerVisible(bool visible) noexcept;
    [[nodiscard]] bool isLayerVisible() const noexcept { return visible_; }
    
    void setLocked(bool locked) noexcept { locked_ = locked; }
    [[nodiscard]] bool isLocked() const noexcept { return locked_; }
    
    void setLayerName(const juce::String& name);
    [[nodiscard]] const juce::String& getLayerName() const noexcept { return name_; }
    
    void setLayerIndex(int index) noexcept { index_ = index; }
    [[nodiscard]] int getLayerIndex() const noexcept { return index_; }
    
    //==========================================================================
    // Transform
    
    void setTransform(const Transform& transform);
    [[nodiscard]] const Transform& getTransform() const noexcept { return transform_; }
    [[nodiscard]] Transform& getTransform() noexcept { return transform_; }
    
    void setPosition(juce::Point<float> pos);
    void setRotation(float radians);
    void setScale(juce::Point<float> scale);
    void setAnchor(juce::Point<float> anchor);
    
    //==========================================================================
    // Centering Utilities
    
    /**
     * @brief Center the layer on the canvas (move position to center)
     * Sets the layer's position to 0.5, 0.5 (center of canvas)
     */
    void centerOnCanvas();
    
    /**
     * @brief Move anchor point to the center of the layer
     * Adjusts position so the layer visually remains in the same place
     */
    void centerAnchorToLayer();
    
    /**
     * @brief Move anchor point to the center of the canvas
     * @param keepVisualPosition If true, adjust position so the layer stays visually in place.
     *                           If false, layer moves with anchor to canvas center.
     */
    void centerAnchorToCanvas(bool keepVisualPosition = false);
    
    //==========================================================================
    // Style
    
    void setStyle(const LayerStyle& style);
    [[nodiscard]] const LayerStyle& getStyle() const noexcept { return style_; }
    [[nodiscard]] LayerStyle& getStyle() noexcept { return style_; }
    
    void setOpacity(float opacity);
    [[nodiscard]] float getOpacity() const noexcept { return style_.opacity; }
    
    void setBlendMode(BlendMode mode);
    [[nodiscard]] BlendMode getBlendMode() const noexcept { return style_.blendMode; }
    
    void setFillColour(juce::Colour colour);
    void setStrokeColour(juce::Colour colour);
    void setStrokeWidth(float width);
    
    //==========================================================================
    // Effects (Blur, Glow, Bevel, etc.)
    
    void setEffects(const LayerEffects& effects);
    [[nodiscard]] const LayerEffects& getEffects() const noexcept { return effects_; }
    [[nodiscard]] LayerEffects& getEffects() noexcept { return effects_; }
    
    void setDropShadowEnabled(bool enabled);
    void setOuterGlowEnabled(bool enabled);
    void setInnerGlowEnabled(bool enabled);
    void setBevelEmbossEnabled(bool enabled);
    void setGaussianBlurEnabled(bool enabled);
    
    void invalidateEffectsCache() { effectsCacheDirty_ = true; }
    
    //==========================================================================
    // Slider Integration
    
    void setSliderRole(SliderLayerRole role) noexcept { sliderRole_ = role; }
    [[nodiscard]] SliderLayerRole getSliderRole() const noexcept { return sliderRole_; }
    
    //==========================================================================
    // Sizing Mode (for relative size calculations)
    
    /**
     * @brief Set the sizing mode for this layer
     * Affects how relative values (corner radius, stroke, shadow) scale
     */
    void setSizingMode(SizingMode mode) noexcept { sizingMode_ = mode; }
    [[nodiscard]] SizingMode getSizingMode() const noexcept { return sizingMode_; }
    
    /**
     * @brief Set the value binding for this layer (replaces first binding or adds if empty)
     * This defines how the layer responds to slider value changes
     */
    void setValueBinding(const ValueBinding& binding);
    
    /**
     * @brief Get the primary (first) value binding - for backward compatibility
     */
    [[nodiscard]] const ValueBinding& getValueBinding() const noexcept;
    [[nodiscard]] ValueBinding& getValueBinding() noexcept;
    
    /**
     * @brief Get all value bindings
     */
    [[nodiscard]] const std::vector<ValueBinding>& getValueBindings() const noexcept { return valueBindings_; }
    [[nodiscard]] std::vector<ValueBinding>& getValueBindings() noexcept { return valueBindings_; }
    
    /**
     * @brief Add a new value binding
     */
    void addValueBinding(const ValueBinding& binding);
    
    /**
     * @brief Add a new default value binding
     */
    void addValueBinding();
    
    /**
     * @brief Remove a value binding by index
     */
    void removeValueBinding(size_t index);
    
    /**
     * @brief Get number of value bindings
     */
    [[nodiscard]] size_t getNumValueBindings() const noexcept { return valueBindings_.size(); }
    
    /**
     * @brief Enable/disable value binding (primary binding)
     */
    void setValueBindingEnabled(bool enabled);
    [[nodiscard]] bool isValueBindingEnabled() const noexcept;
    
    /**
     * @brief Convenience: set binding property with auto-range
     */
    void setValueBindingProperty(ValueBindingProperty property);
    
    /**
     * @brief Convenience: set binding range
     */
    void setValueBindingRange(float minValue, float maxValue);
    
    /**
     * @brief Update layer based on slider value (0-1 normalized)
     * Override in subclasses to implement value-dependent behavior
     */
    virtual void updateForSliderValue(float normalizedValue);
    
    //==========================================================================
    // Masking
    
    void setMask(const LayerMask& mask);
    [[nodiscard]] const LayerMask& getMask() const noexcept { return mask_; }
    void clearMask();
    
    /**
     * @brief Get the pending mask layer name (set during loading from ValueTree)
     * @return The name of the layer to use as mask, or empty if no mask is pending
     */
    [[nodiscard]] const juce::String& getPendingMaskLayerName() const noexcept { return pendingMaskLayerName_; }
    
    /**
     * @brief Clear the pending mask layer name after it has been resolved
     */
    void clearPendingMaskLayerName() { pendingMaskLayerName_.clear(); }
    
    //==========================================================================
    // Interaction State
    
    void setInteractionState(const InteractionState& state);
    [[nodiscard]] const InteractionState& getInteractionState() const noexcept { return interaction_; }
    
    void setHovered(bool hovered);
    void setPressed(bool pressed);
    void setToggled(bool toggled);
    
    //==========================================================================
    // Serialization (ValueTree - Legacy)
    
    [[nodiscard]] virtual juce::ValueTree toValueTree() const;
    virtual void fromValueTree(const juce::ValueTree& tree);
    
    //==========================================================================
    // Serialization (JSON - Modern)
    
    [[nodiscard]] virtual json toJson() const;
    virtual void fromJson(const json& j);
    
    //==========================================================================
    // Callbacks
    
    std::function<void()> onStyleChanged;
    std::function<void()> onTransformChanged;
    std::function<void()> onVisibilityChanged;
    std::function<void(const juce::String&)> onNameChanged;
    
    //==========================================================================
    // Component Overrides (public for composite rendering)
    
    void paint(juce::Graphics& g) override;
    
    /**
     * @brief Paint this layer to specified bounds (for use in canvas rendering)
     * @param g Graphics context to render to
     * @param bounds Target bounds to render within
     * @param applyOpacity If true, applies layer opacity. Set to false when rendering
     *                     to an intermediate image for blend mode compositing.
     */
    void paintLayer(juce::Graphics& g, const juce::Rectangle<float>& bounds, bool applyOpacity = true);
    
protected:
    //==========================================================================
    // Component Overrides
    
    void resized() override;
    
    //==========================================================================
    // Rendering Helpers (for subclasses)
    
    /**
     * @brief Main render method - override in subclasses
     * @param g Graphics context
     * @param bounds The area to render into (already transformed)
     */
    virtual void renderLayer(juce::Graphics& g, const juce::Rectangle<float>& bounds) = 0;
    
    /**
     * @brief Apply the current style's fill to graphics context
     */
    void applyFill(juce::Graphics& g, const juce::Rectangle<float>& bounds) const;
    
    /**
     * @brief Apply the current style's stroke to graphics context
     * @param g Graphics context
     * @param path Path to stroke
     * @param bounds Render bounds (for relative sizing calculations)
     */
    void applyStroke(juce::Graphics& g, const juce::Path& path, const juce::Rectangle<float>& bounds) const;
    
    /**
     * @brief Render shadow if enabled
     * @param g Graphics context
     * @param shapePath Path to create shadow from
     * @param bounds Render bounds (for relative sizing calculations)
     */
    void renderShadow(juce::Graphics& g, const juce::Path& shapePath, const juce::Rectangle<float>& bounds) const;
    
    /**
     * @brief Apply layer effects (glow, bevel, blur, etc.)
     * @param layerImage The rendered layer as an image
     * @param bounds The bounds to render effects into
     * @return Image with all effects applied
     */
    [[nodiscard]] juce::Image applyLayerEffects(const juce::Image& layerImage, 
                                                 const juce::Rectangle<float>& bounds) const;
    
    /**
     * @brief Render the layer to an image for effect processing
     */
    [[nodiscard]] juce::Image renderToImage(const juce::Rectangle<float>& bounds) const;
    
    /**
     * @brief Apply mask to rendered content
     */
    void applyMask(juce::Graphics& g, const juce::Image& content) const;
    
    /**
     * @brief Get computed bounds considering transform and padding
     */
    [[nodiscard]] juce::Rectangle<float> getLayerBounds() const noexcept;
    
    //==========================================================================
    // Member Variables
    
    juce::String name_;
    int index_ { 0 };
    bool visible_ { true };
    bool locked_ { false };
    
    Transform transform_;
    LayerStyle style_;
    LayerEffects effects_;  // Advanced effects (blur, glow, bevel, etc.)
    LayerMask mask_;
    juce::String pendingMaskLayerName_;  // Name of mask layer to resolve after loading
    InteractionState interaction_;
    AnimationState animation_;
    
    SliderLayerRole sliderRole_ { SliderLayerRole::None };
    std::vector<ValueBinding> valueBindings_;  // Multiple bindings supported
    float sliderValue_ { 0.0f };
    
    // Sizing mode for relative size calculations
    SizingMode sizingMode_ { SizingMode::MinDimension };
    
    // Base transform values (before value binding applied)
    Transform baseTransform_;
    float baseOpacity_ { 1.0f };
    
    // Cached effect rendering (for performance)
    mutable juce::Image effectsCache_;
    mutable bool effectsCacheDirty_ { true };
    mutable juce::Rectangle<float> lastEffectsBounds_;  // For cache invalidation on resize
    
    // Drop shadow cache (separate for faster updates when only layer content changes)
    mutable juce::Image dropShadowCache_;
    mutable bool dropShadowCacheDirty_ { true };
    
    // Current render opacity (set during paintLayer, used by helper functions)
    // This allows applyFill/applyStroke to know whether to apply opacity
    mutable float renderOpacity_ { 1.0f };
    
    // Cached render bounds for aspect-ratio-aware orbit calculations
    mutable juce::Rectangle<float> cachedRenderBounds_;
    
public:
    // Helper methods for binding original value storage/restoration
    // Public so LayerPropertyEditor can use them when enabling/disabling bindings
    [[nodiscard]] virtual float getCurrentPropertyValue(ValueBindingProperty property) const;
    virtual void restoreOriginalPropertyValue(ValueBindingProperty property, float value);
};

//==============================================================================
/**
 * @brief Smart pointer type for layers
 */
using LayerPtr = std::shared_ptr<Layer>;
using LayerWeakPtr = std::weak_ptr<Layer>;

} // namespace zaplab

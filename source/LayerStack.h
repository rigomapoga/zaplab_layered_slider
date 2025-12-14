/*
  ==============================================================================

    LayerStack.h
    Created: 30 Nov 2025
    Author:  DevTools Refactored Layer System
    
    Description:
    A container that manages a stack of layers for compositing.
    Handles layer ordering, visibility, and provides the composite rendering.

  ==============================================================================
*/

#pragma once

#include "Layer.h"
#include "ShapeLayer.h"
#include "TextLayer.h"
#include "ImageLayer.h"
#include <vector>
#include <algorithm>

namespace zaplab
{

//==============================================================================
/**
 * @brief LayerStack - manages a collection of layers with z-ordering
 * 
 * Provides:
 * - Layer management (add, remove, reorder)
 * - Composite rendering
 * - Slider value propagation to all layers
 * - Serialization of the complete layer structure
 */
class LayerStack : public juce::Component
{
public:
    //==========================================================================
    // Construction
    
    LayerStack();
    ~LayerStack() override = default;
    
    // Non-copyable
    LayerStack(const LayerStack&) = delete;
    LayerStack& operator=(const LayerStack&) = delete;
    
    //==========================================================================
    // Layer Management - Adding
    
    /**
     * @brief Add a layer to the top of the stack
     */
    void addLayer(LayerPtr layer);
    
    /**
     * @brief Add a layer at a specific index
     */
    void addLayerAt(LayerPtr layer, int index);
    
    /**
     * @brief Create and add a shape layer
     */
    ShapeLayer* addShapeLayer(const juce::String& name = "Shape",
                               ShapeType shapeType = ShapeType::Rectangle,
                               SliderLayerRole role = SliderLayerRole::None);
    
    /**
     * @brief Create and add a text layer
     */
    TextLayer* addTextLayer(const juce::String& name = "Text",
                             const juce::String& initialText = "",
                             SliderLayerRole role = SliderLayerRole::None);
    
    /**
     * @brief Create and add an image layer
     */
    ImageLayer* addImageLayer(const juce::String& name = "Image");
    
    //==========================================================================
    // Layer Management - Removal
    
    /**
     * @brief Remove a layer by pointer
     */
    void removeLayer(Layer* layer);
    
    /**
     * @brief Remove a layer by index
     */
    void removeLayerAt(int index);
    
    /**
     * @brief Remove all layers
     */
    void clearLayers();
    
    //==========================================================================
    // Layer Management - Reordering
    
    /**
     * @brief Move a layer to a new index
     */
    void moveLayer(int fromIndex, int toIndex);
    
    /**
     * @brief Move a layer to the top
     */
    void bringToFront(Layer* layer);
    
    /**
     * @brief Move a layer to the bottom
     */
    void sendToBack(Layer* layer);
    
    /**
     * @brief Move a layer up one level
     */
    void moveUp(Layer* layer);
    
    /**
     * @brief Move a layer down one level
     */
    void moveDown(Layer* layer);
    
    /**
     * @brief Duplicate a layer at index
     */
    Layer* duplicateLayerAt(int index);
    
    //==========================================================================
    // Layer Access
    
    /**
     * @brief Get number of layers
     */
    [[nodiscard]] int getNumLayers() const noexcept { return static_cast<int>(layers_.size()); }
    
    /**
     * @brief Get layer at index
     */
    [[nodiscard]] Layer* getLayer(int index) const;
    
    /**
     * @brief Get layer by name
     */
    [[nodiscard]] Layer* getLayerByName(const juce::String& name) const;
    
    /**
     * @brief Get layer by slider role
     */
    [[nodiscard]] Layer* getLayerByRole(SliderLayerRole role) const;
    
    /**
     * @brief Get index of a layer
     */
    [[nodiscard]] int getLayerIndex(const Layer* layer) const;
    
    /**
     * @brief Get all layers
     */
    [[nodiscard]] const std::vector<LayerPtr>& getLayers() const noexcept { return layers_; }
    
    /**
     * @brief Get layers of a specific type
     */
    template<typename T>
    [[nodiscard]] std::vector<T*> getLayersOfType() const
    {
        std::vector<T*> result;
        for (const auto& layer : layers_)
        {
            if (auto* typed = dynamic_cast<T*>(layer.get()))
                result.push_back(typed);
        }
        return result;
    }
    
    //==========================================================================
    // Selection (for editor UI)
    
    void selectLayer(Layer* layer);
    void selectLayerAt(int index);
    void deselectAll();
    void selectMultiple(const std::vector<Layer*>& layers);
    
    [[nodiscard]] Layer* getSelectedLayer() const;
    [[nodiscard]] std::vector<Layer*> getSelectedLayers() const;
    [[nodiscard]] bool isSelected(const Layer* layer) const;
    
    //==========================================================================
    // Slider Integration
    
    /**
     * @brief Update all layers with new slider value
     */
    void updateForSliderValue(float normalizedValue);
    
    /**
     * @brief Get current slider value
     */
    [[nodiscard]] float getSliderValue() const noexcept { return sliderValue_; }
    
    //==========================================================================
    // Rendering Options
    
    void setBackgroundColour(juce::Colour colour);
    [[nodiscard]] juce::Colour getBackgroundColour() const noexcept { return backgroundColour_; }
    
    void setBackgroundTransparent(bool transparent);
    [[nodiscard]] bool isBackgroundTransparent() const noexcept { return transparentBackground_; }
    
    //==========================================================================
    // Export
    
    /**
     * @brief Render all layers to an image
     */
    [[nodiscard]] juce::Image renderToImage(int width, int height) const;
    
    /**
     * @brief Render a filmstrip of all slider positions
     */
    [[nodiscard]] juce::Image renderFilmstrip(int frameWidth, int frameHeight, int numFrames, bool vertical = true) const;
    
    /**
     * @brief Paint all layers to a graphics context (public access for canvas rendering)
     */
    void paintLayers(juce::Graphics& g, const juce::Rectangle<float>& targetBounds) const;
    
    /**
     * @brief Mark the layer stack as needing a re-render
     */
    void markDirty();
    
    //==========================================================================
    // Serialization
    
    [[nodiscard]] juce::ValueTree toValueTree() const;
    void fromValueTree(const juce::ValueTree& tree);
    
    [[nodiscard]] json toJson() const;
    void fromJson(const json& j);
    
    //==========================================================================
    // Callbacks
    
    std::function<void()> onLayersChanged;
    std::function<void(Layer*)> onLayerAdded;
    std::function<void(Layer*)> onLayerRemoved;
    std::function<void(Layer*)> onLayerSelected;
    std::function<void()> onSelectionChanged;
    std::function<void()> onNeedsRepaint;  ///< Called when external listeners should repaint

protected:
    //==========================================================================
    // Component Overrides
    
    void paint(juce::Graphics& g) override;
    void resized() override;
    
private:
    //==========================================================================
    // Helper Methods
    
    void updateLayerIndices();
    void updateLayerBounds();
    [[nodiscard]] LayerPtr createLayerFromType(LayerType type);
    
    /**
     * @brief Check if a layer is being used as a mask by another layer
     * Mask layers should not be rendered as regular layers in the stack
     */
    [[nodiscard]] bool isLayerUsedAsMask(const Layer* layer) const;
    
    /**
     * @brief Resolve pending mask layer references after loading from ValueTree
     * Called after all layers are loaded to set up the mask weak_ptrs
     */
    void resolveMaskReferences();
    
    //==========================================================================
    // Members
    
    std::vector<LayerPtr> layers_;
    std::vector<Layer*> selectedLayers_;
    
    float sliderValue_ { 0.0f };
    
    juce::Colour backgroundColour_ { juce::Colours::transparentBlack };
    bool transparentBackground_ { true };
    
    // Caching for performance and blend mode support
    mutable juce::Image cachedImage_;
    mutable bool isDirty_ { true };
    
    JUCE_LEAK_DETECTOR(LayerStack)
};

} // namespace zaplab

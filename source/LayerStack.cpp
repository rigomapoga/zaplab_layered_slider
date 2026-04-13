/*
  ==============================================================================

    LayerStack.cpp
    Created: 30 Nov 2025
    Author:  Paolo Zappalà @ zaplab.dev

  ==============================================================================
*/

#include "LayerStack.h"
#include "ImageEffects.h"
#include "AssetManager.h"

namespace zaplab
{

//==============================================================================
// IDs for LayerStack serialization
namespace StackIDs
{
    static const juce::Identifier LayerStack { "LayerStack" };
    static const juce::Identifier Layers { "Layers" };
    static const juce::Identifier BackgroundColour { "backgroundColour" };
    static const juce::Identifier TransparentBackground { "transparentBackground" };
}

//==============================================================================
LayerStack::LayerStack()
{
    setOpaque(false);
}

//==============================================================================
// Rendering Optimization

void LayerStack::markDirty()
{
    isDirty_ = true;
    repaint();
    
    // Notify external listeners (e.g., DesignerCanvas, PreviewPanel)
    if (onNeedsRepaint)
        onNeedsRepaint();
}

//==============================================================================
// Layer Management - Adding

void LayerStack::addLayer(LayerPtr layer)
{
    if (!layer)
        return;
    
    // Make layer invisible to JUCE painting system so we can handle rendering manually
    // This allows us to support blend modes and caching
    layer->setAlpha(0.0f);
    
    // Hook into layer changes to trigger re-render
    layer->onTransformChanged = [this] { markDirty(); };
    layer->onStyleChanged = [this] { markDirty(); };
    layer->onVisibilityChanged = [this] { markDirty(); };
    // layer->onNameChanged is used for UI updates, we can chain it if needed
    
    layer->setLayerIndex(static_cast<int>(layers_.size()));
    layers_.push_back(layer);
    addAndMakeVisible(layer.get());
    
    updateLayerBounds();
    
    if (onLayerAdded)
        onLayerAdded(layer.get());
    
    if (onLayersChanged)
        onLayersChanged();
    
    markDirty();
}

void LayerStack::addLayerAt(LayerPtr layer, int index)
{
    if (!layer)
        return;
    
    // Make layer invisible to JUCE painting system
    layer->setAlpha(0.0f);
    
    // Hook into layer changes
    layer->onTransformChanged = [this] { markDirty(); };
    layer->onStyleChanged = [this] { markDirty(); };
    layer->onVisibilityChanged = [this] { markDirty(); };
    
    index = juce::jlimit(0, static_cast<int>(layers_.size()), index);
    
    layers_.insert(layers_.begin() + index, layer);
    addAndMakeVisible(layer.get());
    
    updateLayerIndices();
    updateLayerBounds();
    
    if (onLayerAdded)
        onLayerAdded(layer.get());
    
    if (onLayersChanged)
        onLayersChanged();
    
    markDirty();
}

ShapeLayer* LayerStack::addShapeLayer(const juce::String& name, ShapeType shapeType, SliderLayerRole role)
{
    auto layer = std::make_shared<ShapeLayer>(name);
    layer->setShapeType(shapeType);
    layer->setSliderRole(role);
    
    // Set visible defaults so the layer shows up immediately
    layer->setFillColour(juce::Colour(0xFF4A90D9));  // Nice blue color
    layer->setStrokeColour(juce::Colours::white);
    layer->setStrokeWidth(0.02f);  // 2% of reference dimension (relative)
    
    // Center the layer in the canvas using normalized 0-1 coordinates
    auto& transform = layer->getTransform();
    transform.setSize(0.5f, 0.5f);       // 50% of parent size
    transform.setPosition(0.5f, 0.5f);   // Centered (anchor is 0.5, 0.5 by default)
    
    addLayer(layer);
    return layer.get();
}

TextLayer* LayerStack::addTextLayer(const juce::String& name, const juce::String& initialText, SliderLayerRole role)
{
    auto layer = std::make_shared<TextLayer>(name);
    layer->setText(initialText.isEmpty() ? "Text" : initialText);
    layer->setSliderRole(role);
    layer->setTextColour(juce::Colours::white);
    layer->setFontSize(16.0f);
    
    // Center the layer using normalized 0-1 coordinates
    auto& transform = layer->getTransform();
    transform.setSize(0.8f, 0.2f);       // 80% width, 20% height
    transform.setPosition(0.5f, 0.5f);   // Centered
    
    addLayer(layer);
    return layer.get();
}

ImageLayer* LayerStack::addImageLayer(const juce::String& name)
{
    auto layer = std::make_shared<ImageLayer>(name);
    
    // Use normalized 0-1 coordinates
    auto& transform = layer->getTransform();
    transform.setSize(0.8f, 0.8f);       // 80% of parent size
    transform.setPosition(0.5f, 0.5f);   // Centered
    
    addLayer(layer);
    return layer.get();
}

//==============================================================================
// Layer Management - Removal

void LayerStack::removeLayer(Layer* layer)
{
    auto it = std::find_if(layers_.begin(), layers_.end(),
                           [layer](const LayerPtr& ptr) { return ptr.get() == layer; });
    
    if (it != layers_.end())
    {
        // Remove from selection
        auto selIt = std::find(selectedLayers_.begin(), selectedLayers_.end(), layer);
        if (selIt != selectedLayers_.end())
            selectedLayers_.erase(selIt);
        
        removeChildComponent(layer);
        
        if (onLayerRemoved)
            onLayerRemoved(layer);
        
        layers_.erase(it);
        updateLayerIndices();
        
        if (onLayersChanged)
            onLayersChanged();
        
        markDirty();
    }
}

void LayerStack::removeLayerAt(int index)
{
    if (index >= 0 && index < static_cast<int>(layers_.size()))
    {
        removeLayer(layers_[index].get());
    }
}

void LayerStack::clearLayers()
{
    for (auto& layer : layers_)
    {
        removeChildComponent(layer.get());
    }
    
    layers_.clear();
    selectedLayers_.clear();
    
    if (onLayersChanged)
        onLayersChanged();
    
    markDirty();
}

//==============================================================================
// Layer Management - Reordering

void LayerStack::moveLayer(int fromIndex, int toIndex)
{
    if (fromIndex < 0 || fromIndex >= static_cast<int>(layers_.size()) ||
        toIndex < 0 || toIndex >= static_cast<int>(layers_.size()) ||
        fromIndex == toIndex)
        return;
    
    auto layer = layers_[fromIndex];
    layers_.erase(layers_.begin() + fromIndex);
    layers_.insert(layers_.begin() + toIndex, layer);
    
    updateLayerIndices();
    
    if (onLayersChanged)
        onLayersChanged();
    
    markDirty();
}

void LayerStack::bringToFront(Layer* layer)
{
    const int index = getLayerIndex(layer);
    if (index >= 0)
        moveLayer(index, static_cast<int>(layers_.size()) - 1);
}

void LayerStack::sendToBack(Layer* layer)
{
    const int index = getLayerIndex(layer);
    if (index >= 0)
        moveLayer(index, 0);
}

void LayerStack::moveUp(Layer* layer)
{
    const int index = getLayerIndex(layer);
    if (index >= 0 && index < static_cast<int>(layers_.size()) - 1)
        moveLayer(index, index + 1);
}

void LayerStack::moveDown(Layer* layer)
{
    const int index = getLayerIndex(layer);
    if (index > 0)
        moveLayer(index, index - 1);
}

Layer* LayerStack::duplicateLayerAt(int index)
{
    if (index < 0 || index >= static_cast<int>(layers_.size()))
        return nullptr;
    
    // Get source layer
    auto* sourceLayer = layers_[static_cast<size_t>(index)].get();
    if (!sourceLayer)
        return nullptr;
    
    // Create duplicate via serialization
    auto tree = sourceLayer->toValueTree();
    
    // Create new layer based on type
    LayerPtr newLayer;
    switch (sourceLayer->getType())
    {
        case LayerType::Shape:
            newLayer = std::make_unique<ShapeLayer>(sourceLayer->getLayerName() + " Copy");
            break;
        case LayerType::Text:
            newLayer = std::make_unique<TextLayer>(sourceLayer->getLayerName() + " Copy");
            break;
        case LayerType::Image:
            newLayer = std::make_unique<ImageLayer>(sourceLayer->getLayerName() + " Copy");
            break;
        default:
            return nullptr;
    }
    
    // Load state from tree
    if (newLayer)
    {
        newLayer->fromValueTree(tree);
        newLayer->setLayerName(sourceLayer->getLayerName() + " Copy");
        
        // Insert right after source layer
        addLayerAt(std::move(newLayer), index + 1);
        
        return layers_[static_cast<size_t>(index + 1)].get();
    }
    
    return nullptr;
}

//==============================================================================
// Layer Access

Layer* LayerStack::getLayer(int index) const
{
    if (index >= 0 && index < static_cast<int>(layers_.size()))
        return layers_[index].get();
    return nullptr;
}

Layer* LayerStack::getLayerByName(const juce::String& name) const
{
    for (const auto& layer : layers_)
    {
        if (layer->getLayerName() == name)
            return layer.get();
    }
    return nullptr;
}

Layer* LayerStack::getLayerByRole(SliderLayerRole role) const
{
    for (const auto& layer : layers_)
    {
        if (layer->getSliderRole() == role)
            return layer.get();
    }
    return nullptr;
}

int LayerStack::getLayerIndex(const Layer* layer) const
{
    for (size_t i = 0; i < layers_.size(); ++i)
    {
        if (layers_[i].get() == layer)
            return static_cast<int>(i);
    }
    return -1;
}

//==============================================================================
// Selection

void LayerStack::selectLayer(Layer* layer)
{
    if (!layer)
        return;
    
    selectedLayers_.clear();
    selectedLayers_.push_back(layer);
    
    if (onLayerSelected)
        onLayerSelected(layer);
    
    if (onSelectionChanged)
        onSelectionChanged();
}

void LayerStack::selectLayerAt(int index)
{
    if (auto* layer = getLayer(index))
        selectLayer(layer);
}

void LayerStack::deselectAll()
{
    selectedLayers_.clear();
    
    if (onSelectionChanged)
        onSelectionChanged();
}

void LayerStack::selectMultiple(const std::vector<Layer*>& layers)
{
    selectedLayers_ = layers;
    
    if (onSelectionChanged)
        onSelectionChanged();
}

Layer* LayerStack::getSelectedLayer() const
{
    return selectedLayers_.empty() ? nullptr : selectedLayers_.front();
}

std::vector<Layer*> LayerStack::getSelectedLayers() const
{
    return selectedLayers_;
}

bool LayerStack::isSelected(const Layer* layer) const
{
    return std::find(selectedLayers_.begin(), selectedLayers_.end(), layer) != selectedLayers_.end();
}

//==============================================================================
// Slider Integration

void LayerStack::updateForSliderValue(float normalizedValue)
{
    sliderValue_ = juce::jlimit(0.0f, 1.0f, normalizedValue);
    
    for (auto& layer : layers_)
    {
        layer->updateForSliderValue(sliderValue_);
    }
    
    markDirty();
}

//==============================================================================
// Rendering Options

void LayerStack::setBackgroundColour(juce::Colour colour)
{
    backgroundColour_ = colour;
    markDirty();
}

void LayerStack::setBackgroundTransparent(bool transparent)
{
    transparentBackground_ = transparent;
    setOpaque(!transparent);
    markDirty();
}

//==============================================================================
// Export

void LayerStack::paintLayers(juce::Graphics& g, const juce::Rectangle<float>& targetBounds) const
{
   #if JUCE_DEBUG
//    DBG("paintLayers called: targetBounds=" << targetBounds.toString() 
//        << " transparentBg=" << (transparentBackground_ ? "true" : "false"));
   #endif
    
    // Check if any layer has a non-normal blend mode
    bool hasBlendModes = false;
    for (const auto& layer : layers_)
    {
        if (layer->isLayerVisible() && layer->getBlendMode() != BlendMode::Normal)
        {
            hasBlendModes = true;
            break;
        }
    }
    
    if (hasBlendModes)
    {
        // Composite with blend mode support
        const int imgW = static_cast<int>(std::ceil(targetBounds.getWidth()));
        const int imgH = static_cast<int>(std::ceil(targetBounds.getHeight()));
        
        if (imgW > 0 && imgH > 0)
        {
            // Create composite image
            juce::Image composite(juce::Image::ARGB, imgW, imgH, true);
            
            // Background
            if (!transparentBackground_)
            {
                juce::Graphics bg(composite);
                bg.setColour(backgroundColour_);
                bg.fillAll();

                // Force fully opaque alpha for opaque background so the
                // on-screen canvas never becomes semi-transparent.
                juce::Image::BitmapData data(composite, juce::Image::BitmapData::readWrite);
                for (int y = 0; y < imgH; ++y)
                {
                    for (int x = 0; x < imgW; ++x)
                    {
                        auto c = data.getPixelColour(x, y);
                        data.setPixelColour(x, y, c.withAlpha(1.0f));
                    }
                }
            }
            else
            {
                // Ensure it's fully transparent
                composite.clear(juce::Rectangle<int>(0, 0, imgW, imgH), juce::Colour(0x00000000));
            }
            
            // Render each layer with blend mode support
            juce::Rectangle<float> imgBounds(0.0f, 0.0f, 
                static_cast<float>(imgW), static_cast<float>(imgH));
            
            for (const auto& layer : layers_)
            {
                // Skip layers that are being used as masks (they should not render as regular layers)
                if (isLayerUsedAsMask(layer.get()))
                    continue;
                    
                if (layer->isLayerVisible())
                {
                    BlendMode blendMode = layer->getBlendMode();
                    float opacity = layer->getOpacity();
                    
                    if (blendMode != BlendMode::Normal)
                    {
                        // Render layer to intermediate image WITHOUT opacity
                        // (opacity will be applied by the blend mode compositor)
                        juce::Image layerImg(juce::Image::ARGB, imgW, imgH, true);
                        {
                            juce::Graphics layerG(layerImg);
                            layer->paintLayer(layerG, imgBounds, false);  // Don't apply opacity
                        }
                        
                        // Composite with blend mode (applies opacity here)
                        BlendModes::compositeWithBlendMode(composite, layerImg, 0, 0, blendMode, opacity);
                    }
                    else
                    {
                        // Normal blend - draw directly to composite with opacity
                        juce::Graphics cg(composite);
                        layer->paintLayer(cg, imgBounds, true);  // Apply opacity normally
                    }
                }
            }
            
            // With an opaque background we want the final composited image
            // to be fully opaque everywhere something has been drawn, so
            // JUCE doesn't fade it again when drawing onto the component.
            if (!transparentBackground_)
            {
                juce::Image::BitmapData data(composite, juce::Image::BitmapData::readWrite);
                for (int y = 0; y < imgH; ++y)
                {
                    for (int x = 0; x < imgW; ++x)
                    {
                        auto c = data.getPixelColour(x, y);
                        if (c.getAlpha() > 0)
                            data.setPixelColour(x, y, c.withAlpha(1.0f));
                    }
                }
            }
            
           #if JUCE_DEBUG
            // Temporary debug: inspect composite alpha range and a centre pixel
            {
                juce::Image::BitmapData dbgData(composite, juce::Image::BitmapData::readOnly);
                uint8 minA = 255, maxA = 0;
                int sumA = 0, count = 0;
                for (int yy = 0; yy < imgH; ++yy)
                {
                    for (int xx = 0; xx < imgW; ++xx)
                    {
                        auto c = dbgData.getPixelColour(xx, yy);
                        auto a = c.getAlpha();
                        if (a < minA) minA = a;
                        if (a > maxA) maxA = a;
                        sumA += a;
                        ++count;
                    }
                }
                auto centre = dbgData.getPixelColour(imgW / 2, imgH / 2);
                DBG("LayerStack::paintLayers composite alpha min=" << (int) minA
                    << " max=" << (int) maxA
                    << " avg=" << (count > 0 ? (double) sumA / (double) count : 0.0)
                    << " centre RGBA=" << (int) centre.getRed() << ","
                    << (int) centre.getGreen() << "," << (int) centre.getBlue()
                    << " a=" << (int) centre.getAlpha());
            }
           #endif

            // Draw composite to output with full opacity
            // Use ScopedSaveState to ensure we don't affect other drawing operations
            {
                juce::Graphics::ScopedSaveState saveState(g);
                g.setOpacity(1.0f);
                g.drawImageAt(composite, 
                    static_cast<int>(targetBounds.getX()), 
                    static_cast<int>(targetBounds.getY()));
            }
        }
    }
    else
    {
        // Fast path: no blend modes, render directly
        // Ensure full opacity for direct rendering
        juce::Graphics::ScopedSaveState outerState(g);
        g.setOpacity(1.0f);
        
        // Background
        if (!transparentBackground_)
        {
            g.setColour(backgroundColour_);
            g.fillRect(targetBounds);
        }
        
        // Render all layers from bottom to top
        for (const auto& layer : layers_)
        {
            // Skip layers that are being used as masks (they should not render as regular layers)
            if (isLayerUsedAsMask(layer.get()))
                continue;
                
            if (layer->isLayerVisible())
            {
                juce::Graphics::ScopedSaveState state(g);
                g.setOpacity(1.0f);  // Reset opacity for each layer
                layer->paintLayer(g, targetBounds);
            }
        }
    }
}

juce::Image LayerStack::renderToImage(int width, int height) const
{
    // Ensure valid dimensions
    width = juce::jmax(1, width);
    height = juce::jmax(1, height);
    
    juce::Image image(juce::Image::ARGB, width, height, true);
    juce::Graphics g(image);
    
    // Background
    if (!transparentBackground_)
    {
        g.fillAll(backgroundColour_);
    }
    
    // Use the target dimensions directly - render layers to fit the target size
    const juce::Rectangle<float> targetBounds(0.0f, 0.0f, static_cast<float>(width), static_cast<float>(height));
    
    // Render all layers directly to target bounds
    paintLayers(g, targetBounds);
    
    return image;
}

juce::Image LayerStack::renderFilmstrip(int frameWidth, int frameHeight, int numFrames, bool vertical) const
{
    const int totalWidth = vertical ? frameWidth : frameWidth * numFrames;
    const int totalHeight = vertical ? frameHeight * numFrames : frameHeight;
    
    juce::Image filmstrip(juce::Image::ARGB, totalWidth, totalHeight, true);
    juce::Graphics g(filmstrip);
    
    // Save current slider value
    const float originalValue = sliderValue_;
    
    for (int frame = 0; frame < numFrames; ++frame)
    {
        // Calculate position in filmstrip
        const int x = vertical ? 0 : frame * frameWidth;
        const int y = vertical ? frame * frameHeight : 0;
        
        // Render frame at this slider position
        const float value = static_cast<float>(frame) / static_cast<float>(numFrames - 1);
        
        // Update all layers for this value (const_cast needed for rendering)
        const_cast<LayerStack*>(this)->updateForSliderValue(value);
        
        // Render to sub-image
        auto frameImage = renderToImage(frameWidth, frameHeight);
        g.drawImageAt(frameImage, x, y);
    }
    
    // Restore original value
    const_cast<LayerStack*>(this)->updateForSliderValue(originalValue);
    
    return filmstrip;
}

//==============================================================================
// Serialization

juce::ValueTree LayerStack::toValueTree() const
{
    juce::ValueTree tree(StackIDs::LayerStack);
    
    tree.setProperty(StackIDs::BackgroundColour, backgroundColour_.toString(), nullptr);
    tree.setProperty(StackIDs::TransparentBackground, transparentBackground_, nullptr);
    
    // Save assets (images used by ImageLayers)
    tree.appendChild(AssetManager::getInstance().toValueTree(), nullptr);
    
    juce::ValueTree layersTree(StackIDs::Layers);
    
    for (const auto& layer : layers_)
    {
        layersTree.appendChild(layer->toValueTree(), nullptr);
    }
    
    tree.appendChild(layersTree, nullptr);
    
    return tree;
}

void LayerStack::fromValueTree(const juce::ValueTree& tree)
{
    if (!tree.hasType(StackIDs::LayerStack))
        return;
    
    clearLayers();
    
    // Load assets first (before layers reference them)
    auto assetsTree = tree.getChildWithName("Assets");
    if (assetsTree.isValid())
    {
        AssetManager::getInstance().fromValueTree(assetsTree);
    }
    
    backgroundColour_ = juce::Colour::fromString(
        tree.getProperty(StackIDs::BackgroundColour, "00000000").toString());
    transparentBackground_ = tree.getProperty(StackIDs::TransparentBackground, true);
    
    auto layersTree = tree.getChildWithName(StackIDs::Layers);
    if (layersTree.isValid())
    {
        for (int i = 0; i < layersTree.getNumChildren(); ++i)
        {
            auto layerTree = layersTree.getChild(i);
            
            // Determine layer type
            const auto typeInt = static_cast<int>(layerTree.getProperty("type", 0));
            const auto type = static_cast<LayerType>(typeInt);
            
            auto layer = createLayerFromType(type);
            if (layer)
            {
                layer->fromValueTree(layerTree);
                addLayer(layer);
            }
        }
    }
    
    // Resolve mask layer references now that all layers are loaded
    resolveMaskReferences();
    
    setOpaque(!transparentBackground_);
    markDirty();
}

//==============================================================================
// Component Overrides

void LayerStack::paint(juce::Graphics& g)
{
    // Update cache if needed
    if (isDirty_ || !cachedImage_.isValid() || 
        cachedImage_.getWidth() != getWidth() || cachedImage_.getHeight() != getHeight())
    {
        cachedImage_ = renderToImage(getWidth(), getHeight());
        isDirty_ = false;
    }
    
    // Draw cached image
    g.drawImageAt(cachedImage_, 0, 0);
}

void LayerStack::resized()
{
    updateLayerBounds();
    markDirty();
}

//==============================================================================
// Helper Methods

void LayerStack::updateLayerIndices()
{
    for (size_t i = 0; i < layers_.size(); ++i)
    {
        layers_[i]->setLayerIndex(static_cast<int>(i));
    }
}

void LayerStack::updateLayerBounds()
{
    const auto bounds = getLocalBounds();
    
    for (auto& layer : layers_)
    {
        layer->setBounds(bounds);
    }
}

LayerPtr LayerStack::createLayerFromType(LayerType type)
{
    switch (type)
    {
        case LayerType::Shape:
            return std::make_shared<ShapeLayer>();
        case LayerType::Text:
            return std::make_shared<TextLayer>();
        case LayerType::Image:
            return std::make_shared<ImageLayer>();
        case LayerType::Group:
            // TODO: Implement GroupLayer
            return nullptr;
        default:
            return nullptr;
    }
}

bool LayerStack::isLayerUsedAsMask(const Layer* layer) const
{
    if (layer == nullptr)
        return false;
    
    // Check if any layer in the stack uses this layer as a mask
    for (const auto& otherLayer : layers_)
    {
        const auto& mask = otherLayer->getMask();
        if (mask.enabled)
        {
            auto maskLayerPtr = mask.maskLayer.lock();
            if (maskLayerPtr && maskLayerPtr.get() == layer)
                return true;
        }
    }
    return false;
}

void LayerStack::resolveMaskReferences()
{
    // For each layer, check if it has a pending mask layer name to resolve
    for (auto& layer : layers_)
    {
        const auto& pendingName = layer->getPendingMaskLayerName();
        if (pendingName.isEmpty())
            continue;
        
        // Find the layer with the matching name
        for (auto& potentialMaskLayer : layers_)
        {
            if (potentialMaskLayer.get() != layer.get() && 
                potentialMaskLayer->getLayerName() == pendingName)
            {
                // Found the mask layer - set up the mask
                auto& mask = const_cast<LayerMask&>(layer->getMask());
                mask.maskLayer = potentialMaskLayer;
                // enabled and inverted flags were already loaded from ValueTree
                break;
            }
        }
        
        // Clear the pending name now that we've tried to resolve it
        layer->clearPendingMaskLayerName();
    }
}

//==============================================================================
// JSON Serialization

json LayerStack::toJson() const
{
    json j;
    
    j["version"] = "2.0";  // Modern JSON format version
    j["format"] = "slider-designer-json";
    j["backgroundColour"] = backgroundColour_;
    j["transparentBackground"] = transparentBackground_;
    
    // Save assets (images used by ImageLayers)
    j["assets"] = AssetManager::getInstance().toJson();
    
    // Save layers
    json layersArray = json::array();
    for (const auto& layer : layers_)
    {
        layersArray.push_back(layer->toJson());
    }
    j["layers"] = layersArray;
    
    return j;
}

void LayerStack::fromJson(const json& j)
{
    clearLayers();
    
    // Load assets first (before layers reference them)
    if (j.contains("assets"))
    {
        AssetManager::getInstance().fromJson(j["assets"]);
    }
    
    // Background settings
    if (j.contains("backgroundColour"))
        j["backgroundColour"].get_to(backgroundColour_);
    else
        backgroundColour_ = juce::Colours::transparentBlack;
    
    transparentBackground_ = j.value("transparentBackground", true);
    
    // Load layers
    if (j.contains("layers"))
    {
        for (const auto& layerJson : j["layers"])
        {
            // Determine layer type
            const auto typeInt = layerJson.value("type", 0);
            const auto type = static_cast<LayerType>(typeInt);
            
            auto layer = createLayerFromType(type);
            if (layer)
            {
                layer->fromJson(layerJson);
                addLayer(layer);
            }
        }
    }
    
    // Resolve mask layer references now that all layers are loaded
    resolveMaskReferences();
    
    setOpaque(!transparentBackground_);
    markDirty();
}

} // namespace zaplab

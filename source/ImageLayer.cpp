/*
  ==============================================================================

    ImageLayer.cpp
    Created: 30 Nov 2025
    Author:  DevTools Refactored Layer System

  ==============================================================================
*/

#include "ImageLayer.h"

namespace zaplab
{

//==============================================================================
// Additional IDs for ImageLayer serialization
namespace ImageIDs
{
    static const juce::Identifier ImagePath { "imagePath" };
    static const juce::Identifier ResourceName { "resourceName" };
    static const juce::Identifier ImageData { "imageData" };
    static const juce::Identifier ImageMode { "imageMode" };
    static const juce::Identifier Placement { "placement" };
    static const juce::Identifier SizeRatio { "sizeRatio" };
    static const juce::Identifier MaintainAspect { "maintainAspect" };
    static const juce::Identifier ImageAlpha { "imageAlpha" };
    
    // Filmstrip
    static const juce::Identifier Filmstrip { "filmstrip" };
    static const juce::Identifier IsVertical { "isVertical" };
    static const juce::Identifier FrameCount { "frameCount" };
    static const juce::Identifier ConnectToValue { "connectToValue" };
    
    // Overlay
    static const juce::Identifier Overlay { "overlay" };
    static const juce::Identifier OverlayEnabled { "enabled" };
    static const juce::Identifier OverlayColour { "colour" };
    static const juce::Identifier HoverColour { "hoverColour" };
    static const juce::Identifier PressedColour { "pressedColour" };
    static const juce::Identifier ToggledColour { "toggledColour" };
}

//==============================================================================
ImageLayer::ImageLayer(const juce::String& name)
    : Layer(name)
{
    // Subscribe to asset changes
    AssetManager::getInstance().addListener(this);
}

ImageLayer::~ImageLayer()
{
    // Unsubscribe from asset changes
    AssetManager::getInstance().removeListener(this);
    
    // Release asset reference
    if (assetId_.isNotEmpty())
    {
        AssetManager::getInstance().removeReference(assetId_, true);
    }
}

//==============================================================================
// Image Loading

void ImageLayer::setImage(const juce::Image& image, const juce::String& name)
{
    if (!image.isValid())
        return;
    
    // Remove old asset reference
    if (assetId_.isNotEmpty())
        AssetManager::getInstance().removeReference(assetId_, true);
    
    // Add to asset manager
    assetId_ = AssetManager::getInstance().addImage(name, image);
    repaint();
}

void ImageLayer::setImageFromFile(const juce::File& file)
{
    if (!file.existsAsFile())
        return;
    
    // Remove old asset reference
    if (assetId_.isNotEmpty())
        AssetManager::getInstance().removeReference(assetId_, true);
    
    // Add to asset manager
    assetId_ = AssetManager::getInstance().addImageFromFile(file);
    imagePath_ = file.getFullPathName();
    repaint();
}

void ImageLayer::setImageFromResource(const void* data, size_t dataSize)
{
    auto image = juce::ImageFileFormat::loadFrom(data, dataSize);
    if (image.isValid())
        setImage(image, "Resource");
}

void ImageLayer::setImageFromBase64(const juce::String& base64Data)
{
    juce::MemoryOutputStream decoded;
    juce::Base64::convertFromBase64(decoded, base64Data);
    
    auto image = juce::ImageFileFormat::loadFrom(decoded.getData(), decoded.getDataSize());
    if (image.isValid())
        setImage(image, "Base64");
}

void ImageLayer::setImageFromAsset(const juce::String& assetId)
{
    // Remove old asset reference
    if (assetId_.isNotEmpty())
        AssetManager::getInstance().removeReference(assetId_, true);
    
    // Use existing asset
    assetId_ = assetId;
    if (assetId_.isNotEmpty())
        AssetManager::getInstance().addReference(assetId_);
    
    repaint();
}

juce::Image ImageLayer::getImage() const
{
    if (assetId_.isEmpty())
        return {};
    
    return AssetManager::getInstance().getImage(assetId_);
}

void ImageLayer::clearImage()
{
    if (assetId_.isNotEmpty())
    {
        AssetManager::getInstance().removeReference(assetId_, true);
        assetId_.clear();
    }
    imagePath_.clear();
    resourceName_.clear();
    repaint();
}

//==============================================================================
// Image Path

void ImageLayer::setImagePath(const juce::String& path)
{
    imagePath_ = path;
}

void ImageLayer::setResourceName(const juce::String& name)
{
    resourceName_ = name;
}

//==============================================================================
// Display Mode

void ImageLayer::setImageMode(ImageMode mode)
{
    if (imageMode_ != mode)
    {
        imageMode_ = mode;
        repaint();
    }
}

void ImageLayer::setPlacement(ImagePlacement placement)
{
    if (placement_ != placement)
    {
        placement_ = placement;
        repaint();
    }
}

//==============================================================================
// Filmstrip Configuration

void ImageLayer::setFilmstripConfig(const FilmstripConfig& config)
{
    filmstrip_ = config;
    repaint();
}

void ImageLayer::setFrameCount(int count)
{
    filmstrip_.frameCount = std::max(1, count);
    repaint();
}

void ImageLayer::setCurrentFrame(int frame)
{
    frame = juce::jlimit(0, filmstrip_.frameCount - 1, frame);
    
    if (filmstrip_.currentFrame != frame)
    {
        filmstrip_.currentFrame = frame;
        repaint();
    }
}

//==============================================================================
// Sizing

void ImageLayer::setSizeRatio(float ratio)
{
    ratio = juce::jlimit(0.01f, 10.0f, ratio);
    
    if (sizeRatio_ != ratio)
    {
        sizeRatio_ = ratio;
        repaint();
    }
}

void ImageLayer::setMaintainAspectRatio(bool maintain)
{
    if (maintainAspectRatio_ != maintain)
    {
        maintainAspectRatio_ = maintain;
        repaint();
    }
}

//==============================================================================
// Overlay/Tinting

void ImageLayer::setOverlay(const ImageOverlay& overlay)
{
    overlay_ = overlay;
    repaint();
}

void ImageLayer::setOverlayEnabled(bool enabled)
{
    if (overlay_.enabled != enabled)
    {
        overlay_.enabled = enabled;
        repaint();
    }
}

void ImageLayer::setOverlayColour(juce::Colour colour)
{
    overlay_.colour = colour;
    overlay_.enabled = true;
    repaint();
}

//==============================================================================
// Image Alpha

void ImageLayer::setImageAlpha(float alpha)
{
    alpha = juce::jlimit(0.0f, 1.0f, alpha);
    
    if (imageAlpha_ != alpha)
    {
        imageAlpha_ = alpha;
        repaint();
    }
}

//==============================================================================
// Slider Integration

void ImageLayer::updateForSliderValue(float normalizedValue)
{
    Layer::updateForSliderValue(normalizedValue);
    
    // Update filmstrip frame if connected to value
    if (imageMode_ == ImageMode::Filmstrip && filmstrip_.connectToValue)
    {
        const int newFrame = filmstrip_.getFrameForValue(normalizedValue);
        if (filmstrip_.currentFrame != newFrame)
        {
            filmstrip_.currentFrame = newFrame;
            repaint();
        }
    }
}

//==============================================================================
// Serialization

juce::ValueTree ImageLayer::toValueTree() const
{
    auto tree = Layer::toValueTree();
    
    tree.setProperty(ImageIDs::ImagePath, imagePath_, nullptr);
    tree.setProperty(ImageIDs::ResourceName, resourceName_, nullptr);
    tree.setProperty(ImageIDs::ImageMode, static_cast<int>(imageMode_), nullptr);
    tree.setProperty(ImageIDs::Placement, static_cast<int>(placement_), nullptr);
    tree.setProperty(ImageIDs::SizeRatio, sizeRatio_, nullptr);
    tree.setProperty(ImageIDs::MaintainAspect, maintainAspectRatio_, nullptr);
    tree.setProperty(ImageIDs::ImageAlpha, imageAlpha_, nullptr);
    
    // Save asset ID (images are stored in AssetManager)
    if (assetId_.isNotEmpty())
    {
        tree.setProperty("assetId", assetId_, nullptr);
    }
    
    // Filmstrip
    juce::ValueTree filmstripTree(ImageIDs::Filmstrip);
    filmstripTree.setProperty(ImageIDs::IsVertical, filmstrip_.isVertical, nullptr);
    filmstripTree.setProperty(ImageIDs::FrameCount, filmstrip_.frameCount, nullptr);
    filmstripTree.setProperty(ImageIDs::ConnectToValue, filmstrip_.connectToValue, nullptr);
    tree.appendChild(filmstripTree, nullptr);
    
    // Overlay
    juce::ValueTree overlayTree(ImageIDs::Overlay);
    overlayTree.setProperty(ImageIDs::OverlayEnabled, overlay_.enabled, nullptr);
    overlayTree.setProperty(ImageIDs::OverlayColour, overlay_.colour.toString(), nullptr);
    overlayTree.setProperty(ImageIDs::HoverColour, overlay_.hoverColour.toString(), nullptr);
    overlayTree.setProperty(ImageIDs::PressedColour, overlay_.pressedColour.toString(), nullptr);
    overlayTree.setProperty(ImageIDs::ToggledColour, overlay_.toggledColour.toString(), nullptr);
    tree.appendChild(overlayTree, nullptr);
    
    return tree;
}

void ImageLayer::fromValueTree(const juce::ValueTree& tree)
{
    Layer::fromValueTree(tree);
    
    imagePath_ = tree.getProperty(ImageIDs::ImagePath, "").toString();
    resourceName_ = tree.getProperty(ImageIDs::ResourceName, "").toString();
    imageMode_ = static_cast<ImageMode>(static_cast<int>(tree.getProperty(ImageIDs::ImageMode, 0)));
    placement_ = static_cast<ImagePlacement>(static_cast<int>(tree.getProperty(ImageIDs::Placement, 1)));
    sizeRatio_ = tree.getProperty(ImageIDs::SizeRatio, 1.0f);
    maintainAspectRatio_ = tree.getProperty(ImageIDs::MaintainAspect, true);
    imageAlpha_ = tree.getProperty(ImageIDs::ImageAlpha, 1.0f);
    
    // Load asset ID (images are stored in AssetManager)
    const juce::String loadedAssetId = tree.getProperty("assetId", "").toString();
    if (loadedAssetId.isNotEmpty())
    {
        setImageFromAsset(loadedAssetId);
    }
    // Legacy: Load image from base64 or path (old presets)
    else
    {
        const juce::String imageData = tree.getProperty(ImageIDs::ImageData, "").toString();
        if (imageData.isNotEmpty())
        {
            setImageFromBase64(imageData);
        }
        else if (imagePath_.isNotEmpty())
        {
            setImageFromFile(juce::File(imagePath_));
        }
    }
    
    // Filmstrip
    auto filmstripTree = tree.getChildWithName(ImageIDs::Filmstrip);
    if (filmstripTree.isValid())
    {
        filmstrip_.isVertical = filmstripTree.getProperty(ImageIDs::IsVertical, true);
        filmstrip_.frameCount = filmstripTree.getProperty(ImageIDs::FrameCount, 1);
        filmstrip_.connectToValue = filmstripTree.getProperty(ImageIDs::ConnectToValue, true);
    }
    
    // Overlay
    auto overlayTree = tree.getChildWithName(ImageIDs::Overlay);
    if (overlayTree.isValid())
    {
        overlay_.enabled = overlayTree.getProperty(ImageIDs::OverlayEnabled, false);
        overlay_.colour = juce::Colour::fromString(overlayTree.getProperty(ImageIDs::OverlayColour, "ffffffff").toString());
        overlay_.hoverColour = juce::Colour::fromString(overlayTree.getProperty(ImageIDs::HoverColour, "00000000").toString());
        overlay_.pressedColour = juce::Colour::fromString(overlayTree.getProperty(ImageIDs::PressedColour, "00000000").toString());
        overlay_.toggledColour = juce::Colour::fromString(overlayTree.getProperty(ImageIDs::ToggledColour, "00000000").toString());
    }
}

//==============================================================================
// Rendering

void ImageLayer::renderLayer(juce::Graphics& g, const juce::Rectangle<float>& bounds)
{
    // Get image from asset manager
    auto image = getImage();
    if (!image.isValid())
        return;
    
    // Get source rectangle (entire image or filmstrip frame)
    juce::Rectangle<int> srcRect;
    if (imageMode_ == ImageMode::Filmstrip)
    {
        srcRect = filmstrip_.getFrameRect(image, filmstrip_.currentFrame);
    }
    else
    {
        srcRect = { 0, 0, image.getWidth(), image.getHeight() };
    }
    
    if (srcRect.isEmpty())
        return;
    
    // Calculate destination bounds
    const auto destBounds = calculateImageBounds(bounds);
    
    // Apply opacity
    g.setOpacity(imageAlpha_ * getStyle().opacity);
    
    // Draw with or without overlay
    drawImageWithOverlay(g, image, destBounds, srcRect);
}

//==============================================================================
// Helper Methods

juce::Rectangle<float> ImageLayer::calculateImageBounds(const juce::Rectangle<float>& componentBounds) const
{
    auto bounds = componentBounds;
    
    // Apply size ratio
    if (sizeRatio_ != 1.0f)
    {
        const float newWidth = bounds.getWidth() * sizeRatio_;
        const float newHeight = bounds.getHeight() * sizeRatio_;
        bounds = bounds.withSizeKeepingCentre(newWidth, newHeight);
    }
    
    // Get image from asset manager for aspect ratio calculation
    auto image = const_cast<ImageLayer*>(this)->getImage();
    if (!image.isValid() || !maintainAspectRatio_)
        return bounds;
    
    // Get source dimensions
    float srcWidth, srcHeight;
    if (imageMode_ == ImageMode::Filmstrip)
    {
        const auto frameRect = filmstrip_.getFrameRect(image, 0);
        srcWidth = static_cast<float>(frameRect.getWidth());
        srcHeight = static_cast<float>(frameRect.getHeight());
    }
    else
    {
        srcWidth = static_cast<float>(image.getWidth());
        srcHeight = static_cast<float>(image.getHeight());
    }
    
    if (srcWidth <= 0 || srcHeight <= 0)
        return bounds;
    
    const float srcAspect = srcWidth / srcHeight;
    const float destAspect = bounds.getWidth() / bounds.getHeight();
    
    switch (placement_)
    {
        case ImagePlacement::Contain:
        {
            if (srcAspect > destAspect)
            {
                // Image is wider, fit to width
                const float newHeight = bounds.getWidth() / srcAspect;
                return bounds.withSizeKeepingCentre(bounds.getWidth(), newHeight);
            }
            else
            {
                // Image is taller, fit to height
                const float newWidth = bounds.getHeight() * srcAspect;
                return bounds.withSizeKeepingCentre(newWidth, bounds.getHeight());
            }
        }
        
        case ImagePlacement::Cover:
        {
            if (srcAspect > destAspect)
            {
                // Image is wider, fit to height (will crop width)
                const float newWidth = bounds.getHeight() * srcAspect;
                return bounds.withSizeKeepingCentre(newWidth, bounds.getHeight());
            }
            else
            {
                // Image is taller, fit to width (will crop height)
                const float newHeight = bounds.getWidth() / srcAspect;
                return bounds.withSizeKeepingCentre(bounds.getWidth(), newHeight);
            }
        }
        
        case ImagePlacement::Center:
        {
            return juce::Rectangle<float>(srcWidth, srcHeight).withCentre(bounds.getCentre());
        }
        
        case ImagePlacement::Stretch:
        case ImagePlacement::Tile:
        default:
            return bounds;
    }
}

juce::Colour ImageLayer::getCurrentOverlayColour() const
{
    if (!overlay_.enabled)
        return juce::Colours::transparentWhite;
    
    const auto& interaction = getInteractionState();
    
    if (interaction.isPressed && overlay_.pressedColour.getAlpha() > 0)
        return overlay_.pressedColour;
    
    if (interaction.isHovered && overlay_.hoverColour.getAlpha() > 0)
        return overlay_.hoverColour;
    
    if (interaction.isToggled && overlay_.toggledColour.getAlpha() > 0)
        return overlay_.toggledColour;
    
    return overlay_.colour;
}

void ImageLayer::drawImageWithOverlay(juce::Graphics& g, const juce::Image& img,
                                       const juce::Rectangle<float>& destBounds,
                                       const juce::Rectangle<int>& srcBounds) const
{
    const auto overlayColour = getCurrentOverlayColour();
    const bool useOverlay = overlay_.enabled && overlayColour.getAlpha() > 0;
    
    if (useOverlay)
    {
        // Draw with colour overlay/tint
        g.setColour(overlayColour);
        g.drawImage(img, destBounds, juce::RectanglePlacement::centred, true);
    }
    else
    {
        // Draw normally
        g.drawImage(img, destBounds, juce::RectanglePlacement::centred);
    }
}

//==============================================================================
void ImageLayer::assetsChanged()
{
    // Check if our asset still exists in the AssetManager
    if (assetId_.isNotEmpty())
    {
        const auto* asset = AssetManager::getInstance().getAsset(assetId_);
        if (asset == nullptr)
        {
            // Asset was deleted, clear our reference
            clearImage();
            // Trigger a repaint
            repaint();
            if (onStyleChanged)
                onStyleChanged();
        }
    }
}

//==============================================================================
// JSON Serialization

json ImageLayer::toJson() const
{
    json j = Layer::toJson();
    
    // Image-specific properties
    j["assetId"] = assetId_.toStdString();
    j["imagePath"] = imagePath_.toStdString();
    j["resourceName"] = resourceName_.toStdString();
    j["imageMode"] = static_cast<int>(imageMode_);
    j["placement"] = static_cast<int>(placement_);
    j["sizeRatio"] = sizeRatio_;
    j["maintainAspectRatio"] = maintainAspectRatio_;
    j["imageAlpha"] = imageAlpha_;
    
    // Filmstrip configuration
    j["filmstrip"] = {
        {"isVertical", filmstrip_.isVertical},
        {"frameCount", filmstrip_.frameCount},
        {"currentFrame", filmstrip_.currentFrame},
        {"loopAnimation", filmstrip_.loopAnimation},
        {"connectToValue", filmstrip_.connectToValue}
    };
    
    // Overlay configuration
    j["overlay"] = {
        {"enabled", overlay_.enabled},
        {"colour", overlay_.colour},
        {"opacity", overlay_.opacity},
        {"hoverColour", overlay_.hoverColour},
        {"pressedColour", overlay_.pressedColour},
        {"toggledColour", overlay_.toggledColour}
    };
    
    return j;
}

void ImageLayer::fromJson(const json& j)
{
    Layer::fromJson(j);
    
    // Image-specific properties
    if (j.contains("assetId"))
    {
        assetId_ = j["assetId"].get<std::string>();
    }
    
    if (j.contains("imagePath"))
        imagePath_ = j["imagePath"].get<std::string>();
    
    if (j.contains("resourceName"))
        resourceName_ = j["resourceName"].get<std::string>();
    
    imageMode_ = static_cast<ImageMode>(j.value("imageMode", 0));
    placement_ = static_cast<ImagePlacement>(j.value("placement", 0));
    sizeRatio_ = j.value("sizeRatio", 1.0f);
    maintainAspectRatio_ = j.value("maintainAspectRatio", true);
    imageAlpha_ = j.value("imageAlpha", 1.0f);
    
    // Filmstrip configuration
    if (j.contains("filmstrip"))
    {
        const auto& fs = j["filmstrip"];
        filmstrip_.isVertical = fs.value("isVertical", true);
        filmstrip_.frameCount = fs.value("frameCount", 1);
        filmstrip_.currentFrame = fs.value("currentFrame", 0);
        filmstrip_.loopAnimation = fs.value("loopAnimation", false);
        filmstrip_.connectToValue = fs.value("connectToValue", true);
    }
    
    // Overlay configuration
    if (j.contains("overlay"))
    {
        const auto& ov = j["overlay"];
        overlay_.enabled = ov.value("enabled", false);
        if (ov.contains("colour")) ov["colour"].get_to(overlay_.colour);
        overlay_.opacity = ov.value("opacity", 1.0f);
        if (ov.contains("hoverColour")) ov["hoverColour"].get_to(overlay_.hoverColour);
        if (ov.contains("pressedColour")) ov["pressedColour"].get_to(overlay_.pressedColour);
        if (ov.contains("toggledColour")) ov["toggledColour"].get_to(overlay_.toggledColour);
    }
}

} // namespace zaplab

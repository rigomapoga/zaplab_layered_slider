/*
  ==============================================================================

    ImageLayer.h
    Created: 30 Nov 2025
    Author:  DevTools Refactored Layer System
    
    Description:
    An image layer that can display static images or filmstrip animations.
    Supports overlay colours, sizing modes, and slider-driven filmstrip animation.

  ==============================================================================
*/

#pragma once

#include "Layer.h"
#include "AssetManager.h"

namespace zaplab
{

//==============================================================================
/**
 * @brief Image display mode
 */
enum class ImageMode
{
    Static,      // Single static image
    Filmstrip    // Filmstrip animation (vertical or horizontal)
};

/**
 * @brief Image sizing/placement mode
 */
enum class ImagePlacement
{
    Stretch,      // Stretch to fill bounds
    Contain,      // Fit within bounds maintaining aspect ratio
    Cover,        // Fill bounds maintaining aspect ratio (may crop)
    Center,       // Center at original size
    Tile          // Tile the image
};

//==============================================================================
/**
 * @brief Filmstrip configuration
 */
struct FilmstripConfig
{
    bool isVertical { true };        // Vertical or horizontal strip
    int frameCount { 1 };            // Number of frames in strip
    int currentFrame { 0 };          // Current frame to display
    bool loopAnimation { false };    // Whether to loop
    bool connectToValue { true };    // Frame selection follows slider value
    
    // Calculate frame rectangle from image
    [[nodiscard]] juce::Rectangle<int> getFrameRect(const juce::Image& image, int frame) const noexcept
    {
        if (!image.isValid() || frameCount <= 0)
            return {};
        
        frame = juce::jlimit(0, frameCount - 1, frame);
        
        if (isVertical)
        {
            const int frameHeight = image.getHeight() / frameCount;
            return { 0, frame * frameHeight, image.getWidth(), frameHeight };
        }
        else
        {
            const int frameWidth = image.getWidth() / frameCount;
            return { frame * frameWidth, 0, frameWidth, image.getHeight() };
        }
    }
    
    // Get frame for normalized value (0-1)
    [[nodiscard]] int getFrameForValue(float normalizedValue) const noexcept
    {
        return static_cast<int>(std::floor(normalizedValue * (frameCount - 1) + 0.5f));
    }
};

//==============================================================================
/**
 * @brief Overlay configuration for image tinting
 */
struct ImageOverlay
{
    bool enabled { false };
    juce::Colour colour { juce::Colours::white };
    float opacity { 1.0f };
    
    // State-based overlays
    juce::Colour hoverColour { juce::Colours::transparentWhite };
    juce::Colour pressedColour { juce::Colours::transparentWhite };
    juce::Colour toggledColour { juce::Colours::transparentWhite };
};

//==============================================================================
/**
 * @brief Image layer - displays images with various modes and effects
 */
class ImageLayer : public Layer, public AssetManager::Listener
{
public:
    //==========================================================================
    // Construction
    
    explicit ImageLayer(const juce::String& name = "Image");
    ~ImageLayer() override;
    
    //==========================================================================
    // Layer Type
    
    [[nodiscard]] LayerType getType() const noexcept override { return LayerType::Image; }
    [[nodiscard]] juce::String getTypeName() const noexcept override { return "Image"; }
    
    //==========================================================================
    // Image Loading
    
    void setImage(const juce::Image& image, const juce::String& name = "Image");
    void setImageFromFile(const juce::File& file);
    void setImageFromResource(const void* data, size_t dataSize);
    void setImageFromBase64(const juce::String& base64Data);
    void setImageFromAsset(const juce::String& assetId);  ///< Use existing asset
    
    [[nodiscard]] juce::Image getImage() const;  ///< Get image from AssetManager
    [[nodiscard]] bool hasImage() const noexcept { return assetId_.isNotEmpty(); }
    [[nodiscard]] const juce::String& getAssetId() const noexcept { return assetId_; }
    
    void clearImage();
    
    //==========================================================================
    // Image Path (for serialization/resource management)
    
    void setImagePath(const juce::String& path);
    [[nodiscard]] const juce::String& getImagePath() const noexcept { return imagePath_; }
    
    void setResourceName(const juce::String& name);
    [[nodiscard]] const juce::String& getResourceName() const noexcept { return resourceName_; }
    
    //==========================================================================
    // Display Mode
    
    void setImageMode(ImageMode mode);
    [[nodiscard]] ImageMode getImageMode() const noexcept { return imageMode_; }
    
    void setPlacement(ImagePlacement placement);
    [[nodiscard]] ImagePlacement getPlacement() const noexcept { return placement_; }
    
    //==========================================================================
    // Filmstrip Configuration
    
    void setFilmstripConfig(const FilmstripConfig& config);
    [[nodiscard]] const FilmstripConfig& getFilmstripConfig() const noexcept { return filmstrip_; }
    [[nodiscard]] FilmstripConfig& getFilmstripConfig() noexcept { return filmstrip_; }
    
    void setFrameCount(int count);
    void setCurrentFrame(int frame);
    [[nodiscard]] int getCurrentFrame() const noexcept { return filmstrip_.currentFrame; }
    
    //==========================================================================
    // Sizing
    
    void setSizeRatio(float ratio);
    [[nodiscard]] float getSizeRatio() const noexcept { return sizeRatio_; }
    
    void setMaintainAspectRatio(bool maintain);
    [[nodiscard]] bool getMaintainAspectRatio() const noexcept { return maintainAspectRatio_; }
    
    //==========================================================================
    // Overlay/Tinting
    
    void setOverlay(const ImageOverlay& overlay);
    [[nodiscard]] const ImageOverlay& getOverlay() const noexcept { return overlay_; }
    
    void setOverlayEnabled(bool enabled);
    void setOverlayColour(juce::Colour colour);
    
    //==========================================================================
    // Image Alpha
    
    void setImageAlpha(float alpha);
    [[nodiscard]] float getImageAlpha() const noexcept { return imageAlpha_; }
    
    //==========================================================================
    // Slider Integration
    
    void updateForSliderValue(float normalizedValue) override;
    
    //==========================================================================
    // Serialization
    
    [[nodiscard]] juce::ValueTree toValueTree() const override;
    void fromValueTree(const juce::ValueTree& tree) override;
    
    [[nodiscard]] json toJson() const override;
    void fromJson(const json& j) override;
    
    //==========================================================================
    // AssetManager::Listener
    
    void assetsChanged() override;  ///< Called when assets are added/removed

protected:
    //==========================================================================
    // Rendering
    
    void renderLayer(juce::Graphics& g, const juce::Rectangle<float>& bounds) override;
    
private:
    //==========================================================================
    // Helper Methods
    
    [[nodiscard]] juce::Rectangle<float> calculateImageBounds(const juce::Rectangle<float>& componentBounds) const;
    [[nodiscard]] juce::Colour getCurrentOverlayColour() const;
    void drawImageWithOverlay(juce::Graphics& g, const juce::Image& img, 
                              const juce::Rectangle<float>& destBounds,
                              const juce::Rectangle<int>& srcBounds) const;
    
    //==========================================================================
    // Members
    
    juce::String assetId_;         ///< ID in AssetManager
    juce::String imagePath_;       ///< Original file path (for reference)
    juce::String resourceName_;    ///< Resource name (for embedded resources)
    
    ImageMode imageMode_ { ImageMode::Static };
    ImagePlacement placement_ { ImagePlacement::Contain };
    
    FilmstripConfig filmstrip_;
    ImageOverlay overlay_;
    
    float sizeRatio_ { 1.0f };
    bool maintainAspectRatio_ { true };
    float imageAlpha_ { 1.0f };
    
    JUCE_LEAK_DETECTOR(ImageLayer)
};

} // namespace zaplab

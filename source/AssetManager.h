/*
  ==============================================================================

    AssetManager.h
    Created: 11 Dec 2025
    Author:  Paolo Zappalà @ zaplab.dev
    
    Description:
    Central management system for image assets used in preset.
    Stores images as base64 data to avoid duplication and enable
    embedded storage within preset files.

  ==============================================================================
*/

#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_cryptography/juce_cryptography.h>
#include "JsonSerialization.h"
#include <unordered_map>
#include <memory>

namespace zaplab
{

//==============================================================================
/**
 * @brief Asset entry - stores image data and metadata
 */
struct ImageAsset
{
    juce::String id;              ///< Unique identifier (hash of image data)
    juce::String name;            ///< User-friendly name (original filename)
    juce::String base64Data;      ///< Base64-encoded image data
    juce::Image cachedImage;      ///< Cached decoded image
    juce::int64 fileSize { 0 };   ///< Original file size in bytes
    juce::String format;          ///< Image format (PNG, JPG, etc.)
    int width { 0 };              ///< Image width in pixels
    int height { 0 };             ///< Image height in pixels
    int referenceCount { 0 };     ///< Number of layers using this asset
    
    ImageAsset() = default;
    
    ImageAsset(const juce::String& name_, const juce::Image& img)
        : name(name_), cachedImage(img)
    {
        if (img.isValid())
        {
            width = img.getWidth();
            height = img.getHeight();
            
            // Encode to base64 (use PNG for lossless storage)
            juce::MemoryOutputStream memStream;
            juce::PNGImageFormat pngFormat;
            pngFormat.writeImageToStream(img, memStream);
            
            base64Data = juce::Base64::toBase64(memStream.getData(), memStream.getDataSize());
            fileSize = static_cast<juce::int64>(memStream.getDataSize());
            format = "PNG";
            
            // Generate ID from image data hash
            id = juce::MD5(memStream.getData(), memStream.getDataSize()).toHexString();
        }
    }
    
    /** Decode base64 to image if needed */
    void ensureImageLoaded()
    {
        if (!cachedImage.isValid() && base64Data.isNotEmpty())
        {
            juce::MemoryOutputStream decoded;
            juce::Base64::convertFromBase64(decoded, base64Data);
            cachedImage = juce::ImageFileFormat::loadFrom(decoded.getData(), decoded.getDataSize());
            
            if (cachedImage.isValid())
            {
                width = cachedImage.getWidth();
                height = cachedImage.getHeight();
            }
        }
    }
};

//==============================================================================
/**
 * @brief AssetManager - Central registry for image assets
 * 
 * Singleton that manages all image assets used in the current preset.
 * Handles deduplication, reference counting, and serialization.
 */
class AssetManager
{
public:
    //==========================================================================
    // Singleton Access
    
    static AssetManager& getInstance()
    {
        static AssetManager instance;
        return instance;
    }
    
    //==========================================================================
    // Asset Management
    
    /**
     * @brief Add an image from file
     * @return Asset ID (existing if duplicate detected)
     */
    juce::String addImageFromFile(const juce::File& file)
    {
        if (!file.existsAsFile())
            return {};
        
        auto image = juce::ImageFileFormat::loadFrom(file);
        if (!image.isValid())
            return {};
        
        return addImage(file.getFileNameWithoutExtension(), image);
    }
    
    /**
     * @brief Add an image
     * @return Asset ID (existing if duplicate detected via hash)
     */
    juce::String addImage(const juce::String& name, const juce::Image& image)
    {
        if (!image.isValid())
            return {};
        
        // Create temporary asset to generate ID
        ImageAsset tempAsset(name, image);
        
        // Check if this image already exists (by ID/hash)
        if (assets_.count(tempAsset.id) > 0)
        {
            // Image already exists, increment reference count
            assets_[tempAsset.id].referenceCount++;
            return tempAsset.id;
        }
        
        // New asset
        tempAsset.referenceCount = 1;
        assets_[tempAsset.id] = std::move(tempAsset);
        
        // Notify listeners
        notifyAssetsChanged();
        
        return tempAsset.id;
    }
    
    /**
     * @brief Get image by ID
     */
    juce::Image getImage(const juce::String& assetId)
    {
        auto it = assets_.find(assetId);
        if (it != assets_.end())
        {
            it->second.ensureImageLoaded();
            return it->second.cachedImage;
        }
        return {};
    }
    
    /**
     * @brief Get asset metadata
     */
    const ImageAsset* getAsset(const juce::String& assetId) const
    {
        auto it = assets_.find(assetId);
        return (it != assets_.end()) ? &it->second : nullptr;
    }
    
    /**
     * @brief Increment reference count
     */
    void addReference(const juce::String& assetId)
    {
        auto it = assets_.find(assetId);
        if (it != assets_.end())
            it->second.referenceCount++;
    }
    
    /**
     * @brief Decrement reference count (and optionally remove if unused)
     */
    void removeReference(const juce::String& assetId, bool autoRemoveUnused = false)
    {
        auto it = assets_.find(assetId);
        if (it != assets_.end())
        {
            it->second.referenceCount--;
            if (autoRemoveUnused && it->second.referenceCount <= 0)
            {
                assets_.erase(it);
                notifyAssetsChanged();
            }
        }
    }
    
    /**
     * @brief Remove a specific asset by ID (ignores reference count)
     * @return true if asset was found and removed
     */
    bool removeAsset(const juce::String& assetId)
    {
        auto it = assets_.find(assetId);
        if (it != assets_.end())
        {
            assets_.erase(it);
            notifyAssetsChanged();
            return true;
        }
        return false;
    }
    
    /**
     * @brief Remove unused assets (reference count = 0)
     */
    int removeUnusedAssets()
    {
        int removed = 0;
        for (auto it = assets_.begin(); it != assets_.end();)
        {
            if (it->second.referenceCount <= 0)
            {
                it = assets_.erase(it);
                removed++;
            }
            else
            {
                ++it;
            }
        }
        
        if (removed > 0)
            notifyAssetsChanged();
        
        return removed;
    }
    
    /**
     * @brief Get all assets
     */
    const std::unordered_map<juce::String, ImageAsset>& getAllAssets() const
    {
        return assets_;
    }
    
    /**
     * @brief Clear all assets
     */
    void clearAllAssets()
    {
        assets_.clear();
        notifyAssetsChanged();
    }
    
    /**
     * @brief Get total size of all assets in bytes
     */
    juce::int64 getTotalSize() const
    {
        juce::int64 total = 0;
        for (const auto& pair : assets_)
            total += pair.second.fileSize;
        return total;
    }
    
    /**
     * @brief Get number of assets
     */
    int getAssetCount() const
    {
        return static_cast<int>(assets_.size());
    }
    
    //==========================================================================
    // Serialization
    
    /** Save all assets to ValueTree */
    juce::ValueTree toValueTree() const
    {
        juce::ValueTree tree("Assets");
        
        for (const auto& pair : assets_)
        {
            const auto& asset = pair.second;
            juce::ValueTree assetTree("Asset");
            assetTree.setProperty("id", asset.id, nullptr);
            assetTree.setProperty("name", asset.name, nullptr);
            assetTree.setProperty("base64Data", asset.base64Data, nullptr);
            assetTree.setProperty("width", asset.width, nullptr);
            assetTree.setProperty("height", asset.height, nullptr);
            assetTree.setProperty("fileSize", asset.fileSize, nullptr);
            assetTree.setProperty("format", asset.format, nullptr);
            assetTree.setProperty("referenceCount", asset.referenceCount, nullptr);
            tree.appendChild(assetTree, nullptr);
        }
        
        return tree;
    }
    
    /** Load assets from ValueTree */
    void fromValueTree(const juce::ValueTree& tree)
    {
        clearAllAssets();
        
        if (!tree.hasType("Assets"))
            return;
        
        for (int i = 0; i < tree.getNumChildren(); ++i)
        {
            auto assetTree = tree.getChild(i);
            if (!assetTree.hasType("Asset"))
                continue;
            
            ImageAsset asset;
            asset.id = assetTree.getProperty("id").toString();
            asset.name = assetTree.getProperty("name").toString();
            asset.base64Data = assetTree.getProperty("base64Data").toString();
            asset.width = assetTree.getProperty("width", 0);
            asset.height = assetTree.getProperty("height", 0);
            asset.fileSize = assetTree.getProperty("fileSize", 0);
            asset.format = assetTree.getProperty("format", "PNG").toString();
            asset.referenceCount = assetTree.getProperty("referenceCount", 0);
            
            assets_[asset.id] = std::move(asset);
        }
        
        notifyAssetsChanged();
    }
    
    /** Save all assets to JSON */
    json toJson() const
    {
        json assetsArray = json::array();
        
        for (const auto& pair : assets_)
        {
            const auto& asset = pair.second;
            json assetJson = {
                {"id", asset.id.toStdString()},
                {"name", asset.name.toStdString()},
                {"base64Data", asset.base64Data.toStdString()},
                {"width", asset.width},
                {"height", asset.height},
                {"fileSize", asset.fileSize},
                {"format", asset.format.toStdString()},
                {"referenceCount", asset.referenceCount}
            };
            assetsArray.push_back(assetJson);
        }
        
        return assetsArray;
    }
    
    /** Load assets from JSON */
    void fromJson(const json& j)
    {
        clearAllAssets();
        
        if (!j.is_array())
            return;
        
        for (const auto& assetJson : j)
        {
            ImageAsset asset;
            asset.id = assetJson.value("id", "");
            asset.name = assetJson.value("name", "");
            asset.base64Data = assetJson.value("base64Data", "");
            asset.width = assetJson.value("width", 0);
            asset.height = assetJson.value("height", 0);
            asset.fileSize = assetJson.value("fileSize", 0);
            asset.format = assetJson.value("format", "PNG");
            asset.referenceCount = assetJson.value("referenceCount", 0);
            
            assets_[asset.id] = std::move(asset);
        }
        
        notifyAssetsChanged();
    }
    
    //==========================================================================
    // Listeners
    
    class Listener
    {
    public:
        virtual ~Listener() = default;
        virtual void assetsChanged() = 0;
    };
    
    void addListener(Listener* listener)
    {
        listeners_.add(listener);
    }
    
    void removeListener(Listener* listener)
    {
        listeners_.remove(listener);
    }
    
    void notifyAssetsChanged()
    {
        listeners_.call([](Listener& l) { l.assetsChanged(); });
    }
    
private:
    AssetManager() = default;
    ~AssetManager() = default;
    
    // Non-copyable
    AssetManager(const AssetManager&) = delete;
    AssetManager& operator=(const AssetManager&) = delete;
    
    std::unordered_map<juce::String, ImageAsset> assets_;
    juce::ListenerList<Listener> listeners_;
};

} // namespace zaplab

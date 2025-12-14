/*******************************************************************************
 
    zaplab_layered_slider - A JUCE module for layer-based slider graphics
    
    Copyright (c) 2024 ZapLab.dev
    
    Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files (the "Software"), to deal
    in the Software without restriction, including without limitation the rights
    to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
    copies of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions:
    
    The above copyright notice and this permission notice shall be included in all
    copies or substantial portions of the Software.
    
    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
    SOFTWARE.
    
    ==============================================================================
    
    BEGIN_JUCE_MODULE_DECLARATION
    
    ID:                 zaplab_layered_slider
    vendor:             zaplab
    version:            1.1.0
    name:               ZapLab Layered Slider
    description:        A layer-based slider component for creating custom knobs and sliders
    website:            https://github.com/zaplab-dev/layered-slider
    license:            MIT
    minimumCppStandard: 17
    
    dependencies:       juce_core, juce_graphics, juce_gui_basics, juce_cryptography
    
    END_JUCE_MODULE_DECLARATION
    
*******************************************************************************/

#pragma once

#define ZAPLAB_LAYERED_SLIDER_H_INCLUDED

//==============================================================================
// JUCE module includes
//==============================================================================
#include <juce_core/juce_core.h>
#include <juce_cryptography/juce_cryptography.h>
#include <juce_graphics/juce_graphics.h>
#include <juce_gui_basics/juce_gui_basics.h>

//==============================================================================
// Module source files
//==============================================================================
#include "source/LayerTypes.h"
#include "source/Layer.h"
#include "source/ShapeLayer.h"
#include "source/TextLayer.h"
#include "source/ImageLayer.h"
#include "source/LayerStack.h"
#include "source/ValueBinding.h"
#include "source/LayeredSlider.h"
#include "source/JsonSerialization.h"
#include "source/DevToolsUtilities.h"

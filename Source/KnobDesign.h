#pragma once
#include <juce_gui_basics/juce_gui_basics.h>

namespace KnobDesign
{
    // ── Colors (matching conji.us) ──
    inline const juce::Colour bgColour       { 0xff111111 };  // #111
    inline const juce::Colour accentColour   { 0xffd48300 };  // #d48300

    // ── Knob geometry (proportional to diameter) ──
    // All stroke/size values are fractions of the knob diameter
    inline constexpr float knobStrokeFrac    = 0.033f;   // circle stroke (~5px at 150px diameter)
    inline constexpr float indicatorWidthFrac= 0.040f;   // indicator stroke (~6px at 150px)
    inline constexpr float tickStrokeFrac    = 0.033f;   // tick stroke (~5px at 150px)

    // Indicator spans from 33% to 75% of radius (inner to outer — longer on circle side)
    inline constexpr float indicatorStart    = 0.33f;
    inline constexpr float indicatorEnd      = 0.75f;

    // Tick marks: start further out from circle, extend outward
    inline constexpr float tickGap           = 1.15f;   // start outside the circle (3x original gap)
    inline constexpr float tickLength        = 0.18f;   // length as fraction of radius

    // ── Rotation range ──
    // The knob rotates from -135° to +135° (270° total arc)
    inline constexpr float rotationStartAngle = -135.0f;  // degrees from 12 o'clock
    inline constexpr float rotationEndAngle   =  135.0f;

    // ── Label style (proportional to diameter / window) ──
    inline constexpr float labelFontScale    = 0.18f;   // "0"/"11" font size as fraction of diameter
    inline constexpr float gainLabelScale    = 0.06f;   // "Gain" font size as fraction of window width
    inline constexpr float dbTextScale       = 0.06f;   // dB readout as fraction of window width
    inline constexpr float latencyTextScale  = 0.03f;   // latency label as fraction of window width

    // ── Window ──
    inline constexpr int   defaultSize       = 450;
    inline constexpr int   minSize           = 200;
    inline constexpr int   maxSize           = 800;

    // ── Angle helpers ──
    // Convert a normalised 0–1 value to an angle in radians from 12 o'clock
    inline float normToAngleRad(float norm01)
    {
        float degrees = rotationStartAngle + norm01 * (rotationEndAngle - rotationStartAngle);
        return juce::degreesToRadians(degrees);
    }
}

// ── Custom LookAndFeel ──
class ConjusKnobLookAndFeel : public juce::LookAndFeel_V4
{
public:
    ConjusKnobLookAndFeel()
    {
        // Slider text box colours
        setColour(juce::Slider::textBoxTextColourId, KnobDesign::accentColour);
        setColour(juce::Slider::textBoxBackgroundColourId, juce::Colours::transparentBlack);
        setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);

        // Label colours
        setColour(juce::Label::textColourId, KnobDesign::accentColour);
    }

    // Call this after BinaryData is available to load Inconsolata
    void loadFonts(const void* boldData, int boldSize, const void* regularData, int regularSize)
    {
        boldTypeface = juce::Typeface::createSystemTypefaceFor(boldData, static_cast<size_t>(boldSize));
        regularTypeface = juce::Typeface::createSystemTypefaceFor(regularData, static_cast<size_t>(regularSize));
    }

    juce::Font getBoldFont(float height) const
    {
        if (boldTypeface != nullptr)
            return juce::Font(juce::FontOptions(boldTypeface).withHeight(height));
        return juce::Font(juce::FontOptions().withHeight(height).withStyle("Bold"));
    }

    juce::Font getRegularFont(float height) const
    {
        if (regularTypeface != nullptr)
            return juce::Font(juce::FontOptions(regularTypeface).withHeight(height));
        return juce::Font(juce::FontOptions().withHeight(height));
    }

    void drawRotarySlider(juce::Graphics& g,
                          int x, int y, int width, int height,
                          float sliderPosProportional,
                          float /*rotaryStartAngle*/, float /*rotaryEndAngle*/,
                          juce::Slider& slider) override
    {
        using namespace KnobDesign;

        auto bounds = juce::Rectangle<int>(x, y, width, height).toFloat();

        // Size knob based on window, not slider bounds (which shrink with text box)
        auto* parent = slider.getParentComponent();
        float windowW = parent ? static_cast<float>(parent->getWidth()) : bounds.getWidth();
        float windowH = parent ? static_cast<float>(parent->getHeight()) : bounds.getHeight();
        float windowSize = juce::jmin(windowW, windowH);
        float diameter = windowSize * 0.42f;
        float radius = diameter * 0.5f;
        // Centre the knob at the centre of the full window
        float cx = bounds.getCentreX();
        float sliderY = static_cast<float>(slider.getY());
        float cy = windowH * 0.5f - sliderY;

        // Scale strokes to diameter
        float strokeW = diameter * knobStrokeFrac;
        float indW = diameter * indicatorWidthFrac;
        float tickW = diameter * tickStrokeFrac;

        // ── Draw knob circle ──
        g.setColour(accentColour);
        g.drawEllipse(cx - radius + strokeW * 0.5f,
                      cy - radius + strokeW * 0.5f,
                      diameter - strokeW,
                      diameter - strokeW,
                      strokeW);

        // ── Draw indicator line ──
        float angle = normToAngleRad(sliderPosProportional);
        float innerR = radius * indicatorStart;
        float outerR = radius * indicatorEnd;

        juce::Path indicator;
        indicator.startNewSubPath(cx + std::sin(angle) * innerR,
                                 cy - std::cos(angle) * innerR);
        indicator.lineTo(cx + std::sin(angle) * outerR,
                         cy - std::cos(angle) * outerR);
        g.setColour(accentColour);
        g.strokePath(indicator,
                     juce::PathStrokeType(indW,
                                          juce::PathStrokeType::curved,
                                          juce::PathStrokeType::rounded));

        // ── Draw tick marks at -∞, 0 dB (top), and +24 dB positions ──
        float tickStartR = radius * tickGap;
        float tickEndR = radius * (tickGap + tickLength);

        float tickAngles[3] = {
            juce::degreesToRadians(rotationStartAngle),  // -∞ (left)
            0.0f,                                         // 0 dB (top, 12 o'clock)
            juce::degreesToRadians(rotationEndAngle)      // +24 dB (right)
        };

        for (int i = 0; i < 3; ++i)
        {
            juce::Path tick;
            tick.startNewSubPath(cx + std::sin(tickAngles[i]) * tickStartR,
                                cy - std::cos(tickAngles[i]) * tickStartR);
            tick.lineTo(cx + std::sin(tickAngles[i]) * tickEndR,
                        cy - std::cos(tickAngles[i]) * tickEndR);
            g.strokePath(tick,
                         juce::PathStrokeType(tickW,
                                              juce::PathStrokeType::curved,
                                              juce::PathStrokeType::rounded));
        }

        // ── Draw labels ──
        float fontSize = diameter * labelFontScale;
        float markerFontSize = fontSize * 0.85f;   // size for 0, +24, and the minus sign
        float infFontSize = fontSize * 1.8f;        // large infinity symbol (slightly thinner)
        g.setColour(accentColour);

        float labelR = tickEndR + fontSize * 0.8f;
        float labelYOffset = fontSize * 0.05f;

        // "−∞" label — below-left tick: minus at markerFontSize, ∞ at infFontSize
        float angle0 = juce::degreesToRadians(rotationStartAngle);
        float label0X = cx + std::sin(angle0) * labelR;
        float label0Y = cy - std::cos(angle0) * labelR + labelYOffset;
        {
            // Measure the minus sign width at marker size
            g.setFont(getBoldFont(markerFontSize));
            auto minusStr = juce::String(juce::CharPointer_UTF8("\xe2\x88\x92"));
            float minusW = g.getCurrentFont().getStringWidthFloat(minusStr);

            // Measure the infinity symbol width at inf size (thinner weight)
            g.setFont(getRegularFont(infFontSize));
            auto infStr = juce::String(juce::CharPointer_UTF8("\xe2\x88\x9e"));
            float infW = g.getCurrentFont().getStringWidthFloat(infStr);

            float totalW = minusW + infW;
            float startX = label0X - totalW * 0.5f;

            // Draw minus sign
            g.setFont(getBoldFont(markerFontSize));
            g.drawText(minusStr,
                       juce::Rectangle<float>(startX, label0Y - infFontSize * 0.5f,
                                              minusW, infFontSize),
                       juce::Justification::centred, false);

            // Draw infinity symbol (thinner, shifted slightly up and right)
            float infNudgeX = diameter * 0.01f;
            float infNudgeY = -diameter * 0.01f;
            g.setFont(getRegularFont(infFontSize));
            g.drawText(infStr,
                       juce::Rectangle<float>(startX + minusW + infNudgeX,
                                              label0Y - infFontSize * 0.5f + infNudgeY,
                                              infW, infFontSize),
                       juce::Justification::centred, false);
        }

        // "+24" label — below-right tick
        g.setFont(getBoldFont(markerFontSize));
        float angle24 = juce::degreesToRadians(rotationEndAngle);
        float label24X = cx + std::sin(angle24) * labelR - diameter * 0.012f;
        float label24Y = cy - std::cos(angle24) * labelR + labelYOffset - diameter * 0.012f;
        g.drawText("+24",
                   juce::Rectangle<float>(label24X - fontSize * 2.0f, label24Y - markerFontSize * 0.5f,
                                          fontSize * 4.0f, markerFontSize * 1.2f),
                   juce::Justification::centred, false);

        // "0" label — above top tick (12 o'clock), close to tick
        float topLabelR = tickEndR + markerFontSize * 0.3f;
        float topLabelX = cx;
        float topLabelY = cy - topLabelR - markerFontSize * 0.5f;
        g.drawText("0",
                   juce::Rectangle<float>(topLabelX - fontSize * 2.0f, topLabelY - markerFontSize * 0.5f,
                                          fontSize * 4.0f, markerFontSize * 1.2f),
                   juce::Justification::centred, false);
    }

    void drawLabel(juce::Graphics& g, juce::Label& label) override
    {
        // Check if this label belongs to a slider (the dB text box)
        bool isSliderTextBox = (dynamic_cast<juce::Slider*>(label.getParentComponent()) != nullptr);

        if (isSliderTextBox)
        {
            auto text = label.getText();
            auto font = label.getFont();
            float baseHeight = font.getHeight();
            bool hasInfinity = text.containsChar(0x221e);

            // ── Fixed pill dimensions ──
            // Pill layout: [padLeft | minus zone | gap | value zone | " dB" | padRight]
            // Minus is pinned to left edge; " dB" is pinned to right edge; value sits between.
            auto baseFont = getRegularFont(baseHeight);
            juce::String dbSuffix = " dB";
            float dbW = baseFont.getStringWidthFloat(dbSuffix);

            // Measure minus sign width (static left zone)
            auto minusStr = juce::String(juce::CharPointer_UTF8("\xe2\x88\x92"));
            float minusW = baseFont.getStringWidthFloat(minusStr);
            float minusGap = baseHeight * 0.15f;  // gap between minus and value

            // Measure widest possible value area (∞ path or "+24.0")
            // ∞ is drawn as a custom path for full stroke control
            float infSymW = baseHeight * 1.2f;   // width of the ∞ path
            float infSymH = baseHeight * 0.55f;  // height of the ∞ path
            float infW = infSymW;
            float numW = baseFont.getStringWidthFloat("+24.0");
            float valueZoneW = juce::jmax(infW, numW);

            float pillH = baseHeight * 1.4f;
            float padLeft = pillH * 0.45f;
            float padRight = pillH * 0.25f;
            float pillW = padLeft + minusW + minusGap + valueZoneW + dbW + padRight;

            // Centre the pill horizontally in the label
            float labelW = static_cast<float>(label.getWidth());
            float pillX = (labelW - pillW) * 0.5f;
            float pillY = (static_cast<float>(label.getHeight()) - pillH) * 0.5f + 5.0f;

            auto pillBounds = juce::Rectangle<float>(pillX, pillY, pillW, pillH);

            // Draw orange pill background with fully circular edges
            g.setColour(KnobDesign::accentColour);
            g.fillRoundedRectangle(pillBounds, pillH * 0.5f);

            g.setColour(KnobDesign::bgColour);
            float centreY = pillBounds.getCentreY();

            // ── Fixed right zone: " dB" always at same position ──
            float dbX = pillBounds.getRight() - padRight - dbW;
            g.setFont(baseFont);
            g.drawText(dbSuffix,
                       juce::Rectangle<float>(dbX, centreY - baseHeight * 0.5f,
                                              dbW, baseHeight),
                       juce::Justification::centred, false);

            // ── Fixed left zone: minus sign pinned to left edge (when negative) ──
            float minusX = pillBounds.getX() + padLeft;
            // Value zone sits right of the minus+gap area, right-aligned against " dB"
            float valueRight = dbX;
            float valueLeft = minusX + minusW + minusGap;

            if (hasInfinity)
            {
                // Draw minus pinned to left
                g.setFont(baseFont);
                g.drawText(minusStr,
                           juce::Rectangle<float>(minusX, centreY - baseHeight * 0.5f,
                                                  minusW, baseHeight),
                           juce::Justification::centred, false);

                // Draw ∞ as a custom path for precise stroke control
                float infCX = valueLeft + infSymW * 0.5f;
                float infCY = pillBounds.getCentreY();
                float hw = infSymW * 0.5f;  // half-width
                float hh = infSymH * 0.5f;  // half-height

                juce::Path infPath;
                // Each lobe is a near-circle using cubic bezier magic number
                float k = 0.5523f;  // cubic bezier circle approximation constant
                float lr = hw * 0.5f;  // lobe radius (each lobe is a circle)
                float lrx = lr;        // horizontal radius
                float lry = hh;        // vertical radius = half-height

                // Left lobe centre
                float lcx = infCX - lrx;

                // Left lobe: top half (from centre crossing → left peak → back)
                infPath.startNewSubPath(infCX, infCY);
                infPath.cubicTo(infCX - lrx * k, infCY - lry * k * 2.0f,
                                lcx + lrx * k,   infCY - lry,
                                lcx,              infCY - lry);
                infPath.cubicTo(lcx - lrx * k,   infCY - lry,
                                infCX - hw,       infCY - lry * k,
                                infCX - hw,       infCY);
                // Left lobe: bottom half
                infPath.cubicTo(infCX - hw,       infCY + lry * k,
                                lcx - lrx * k,   infCY + lry,
                                lcx,              infCY + lry);
                infPath.cubicTo(lcx + lrx * k,   infCY + lry,
                                infCX - lrx * k, infCY + lry * k * 2.0f,
                                infCX,            infCY);

                // Right lobe centre
                float rcx = infCX + lrx;

                // Right lobe: bottom half (continues from centre)
                infPath.cubicTo(infCX + lrx * k, infCY + lry * k * 2.0f,
                                rcx - lrx * k,   infCY + lry,
                                rcx,              infCY + lry);
                infPath.cubicTo(rcx + lrx * k,   infCY + lry,
                                infCX + hw,       infCY + lry * k,
                                infCX + hw,       infCY);
                // Right lobe: top half
                infPath.cubicTo(infCX + hw,       infCY - lry * k,
                                rcx + lrx * k,   infCY - lry,
                                rcx,              infCY - lry);
                infPath.cubicTo(rcx - lrx * k,   infCY - lry,
                                infCX + lrx * k, infCY - lry * k * 2.0f,
                                infCX,            infCY);

                float strokeW = baseHeight * 0.08f;
                g.strokePath(infPath, juce::PathStrokeType(strokeW, juce::PathStrokeType::curved));
            }
            else
            {
                // Parse the value text: strip " dB" suffix, split sign from digits
                juce::String valueStr = text.replace(" dB", "").trim();
                bool isNegative = valueStr.startsWith("-");
                bool isPositive = valueStr.getDoubleValue() > 0.0;
                juce::String digits = isNegative ? valueStr.substring(1) : valueStr;
                // Strip leading '+' if present in the digits
                if (digits.startsWith("+"))
                    digits = digits.substring(1);

                if (isNegative)
                {
                    // Draw minus pinned to left
                    g.setFont(baseFont);
                    g.drawText(minusStr,
                               juce::Rectangle<float>(minusX, centreY - baseHeight * 0.5f,
                                                      minusW, baseHeight),
                               juce::Justification::centred, false);
                }
                else if (isPositive)
                {
                    // Draw plus pinned to left
                    g.setFont(baseFont);
                    g.drawText("+",
                               juce::Rectangle<float>(minusX, centreY - baseHeight * 0.5f,
                                                      minusW, baseHeight),
                               juce::Justification::centred, false);
                }

                // Draw value digits left-aligned in the value zone
                g.setFont(baseFont);
                g.drawText(digits,
                           juce::Rectangle<float>(valueLeft, centreY - baseHeight * 0.5f,
                                                  valueRight - valueLeft, baseHeight),
                           juce::Justification::centredLeft, false);
            }
        }
        else
        {
            // Default rendering for other labels (Gain, Latency)
            LookAndFeel_V4::drawLabel(g, label);
        }
    }

private:
    juce::Typeface::Ptr boldTypeface;
    juce::Typeface::Ptr regularTypeface;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ConjusKnobLookAndFeel)
};

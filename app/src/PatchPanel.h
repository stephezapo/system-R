#pragma once

#include <juce_gui_extra/juce_gui_extra.h>

#include "system_r/core/Engine.h"
#include "system_r/core/Patch.h"
#include "system_r/core/ShowDocument.h"
#include "system_r/core/gdtf/FixtureLibrary.h"
#include "system_r/core/mvr/MvrImporter.h"

#include <functional>
#include <memory>
#include <set>
#include <vector>

namespace systemr
{
/** A ListBoxModel driven by std::function callbacks, so one small type backs several lists. */
struct LambdaListBoxModel final : juce::ListBoxModel
{
    std::function<int()> rows;
    std::function<void (int, juce::Graphics&, int, int, bool)> paint;
    std::function<void (int)> select;

    int getNumRows() override { return rows ? rows() : 0; }

    void paintListBoxItem (int row, juce::Graphics& g, int w, int h, bool selected) override
    {
        if (paint)
            paint (row, g, w, h, selected);
    }

    void selectedRowsChanged (int lastRow) override
    {
        if (select)
            select (lastRow);
    }
};

/** Patch panel: import GDTF/MVR into the library, browse fixture types, add them to the
    patch at the first free address, and see the patch with overlap conflicts flagged red.
    Structural edits are wrapped in Engine::pauseWhile so they never race the render. */
class PatchPanel final : public juce::Component
{
public:
    PatchPanel (ShowDocument& s, gdtf::FixtureLibrary& lib, Engine& eng)
        : show (s), library (lib), engine (eng)
    {
        importGdtfBtn.onClick = [this] { importGdtf(); };
        importMvrBtn.onClick  = [this] { importMvr(); };
        addBtn.onClick        = [this] { addSelected(); };
        for (auto* b : { &importGdtfBtn, &importMvrBtn, &addBtn })
            addAndMakeVisible (b);

        libLabel.setText ("Library", juce::dontSendNotification);
        patchLabel.setText ("Patch", juce::dontSendNotification);
        libLabel.setFont (juce::FontOptions (14.0f, juce::Font::bold));
        patchLabel.setFont (juce::FontOptions (14.0f, juce::Font::bold));
        addAndMakeVisible (libLabel);
        addAndMakeVisible (patchLabel);

        libModel.rows   = [this] { return static_cast<int> (libTypes.size()); };
        libModel.paint  = [this] (int r, juce::Graphics& g, int w, int h, bool sel) { paintLibRow (r, g, w, h, sel); };
        libModel.select = [this] (int r) { selectedType = r; };
        libList.setModel (&libModel);
        addAndMakeVisible (libList);

        patchModel.rows  = [this] { return static_cast<int> (patched.size()); };
        patchModel.paint = [this] (int r, juce::Graphics& g, int w, int h, bool sel) { paintPatchRow (r, g, w, h, sel); };
        patchList.setModel (&patchModel);
        addAndMakeVisible (patchList);

        refresh();
    }

    void resized() override
    {
        auto r = getLocalBounds().reduced (10);
        auto toolbar = r.removeFromTop (30);
        importGdtfBtn.setBounds (toolbar.removeFromLeft (110));
        toolbar.removeFromLeft (6);
        importMvrBtn.setBounds (toolbar.removeFromLeft (110));
        toolbar.removeFromLeft (6);
        addBtn.setBounds (toolbar.removeFromLeft (90));
        r.removeFromTop (8);

        auto left = r.removeFromLeft (r.getWidth() / 2);
        r.removeFromLeft (10);
        libLabel.setBounds (left.removeFromTop (20));
        libList.setBounds (left);
        patchLabel.setBounds (r.removeFromTop (20));
        patchList.setBounds (r);
    }

    void refresh()
    {
        libTypes  = library.all();
        patched   = show.fixtures();
        conflicts = patch::overlappingFixtures (patched);
        libList.updateContent();
        patchList.updateContent();
        repaint();
    }

private:
    void paintLibRow (int row, juce::Graphics& g, int w, int h, bool selected)
    {
        if (row < 0 || row >= (int) libTypes.size())
            return;
        const auto* t = libTypes[static_cast<std::size_t> (row)];
        if (selected)
            g.fillAll (juce::Colour (0xff2a4a6a));
        g.setColour (juce::Colours::white);
        g.setFont (juce::FontOptions (14.0f));
        g.drawText (t->manufacturer + " — " + t->name
                        + "  (" + juce::String (t->modes.size()) + " modes)",
                    6, 0, w - 8, h, juce::Justification::centredLeft);
    }

    void paintPatchRow (int row, juce::Graphics& g, int w, int h, bool selected)
    {
        if (row < 0 || row >= (int) patched.size())
            return;
        const auto& f = patched[static_cast<std::size_t> (row)];
        const bool clash = conflicts.count (f.fid) > 0;

        g.fillAll (clash ? juce::Colour (0xff5a1e1e)
                         : (selected ? juce::Colour (0xff2a4a6a) : juce::Colour (0xff1b1b1b)));
        g.setColour (clash ? juce::Colours::orange : juce::Colours::white);
        g.setFont (juce::FontOptions (13.0f));

        const auto type = f.gdtfTypeId.isNotEmpty() ? f.modeName : juce::String ("dimmer");
        g.drawText (juce::String (f.fid) + "  " + f.name
                        + "   U" + juce::String (f.universe) + "." + juce::String (f.address)
                        + "   " + juce::String (f.channelCount) + "ch"
                        + (type.isNotEmpty() ? "   [" + type + "]" : juce::String()),
                    6, 0, w - 8, h, juce::Justification::centredLeft);
    }

    int nextFid() const
    {
        int m = 0;
        for (const auto& f : show.fixtures())
            m = juce::jmax (m, f.fid);
        return m + 1;
    }

    void addSelected()
    {
        if (selectedType < 0 || selectedType >= (int) libTypes.size())
            return;

        const auto* type = libTypes[static_cast<std::size_t> (selectedType)];
        const gdtf::DmxMode* mode = type->modes.empty() ? nullptr : &type->modes.front();
        const int channelCount = mode != nullptr ? juce::jmax (1, mode->channelCount()) : 1;

        const auto current = show.fixtures();
        int address = patch::firstFreeAddress (current, 1, channelCount);
        if (address < 0)
            address = 1;

        const int fid = nextFid();
        engine.pauseWhile ([&]
        {
            show.addFixture (fid, type->name, type->id(),
                             mode != nullptr ? mode->name : juce::String(),
                             1, address, channelCount);
        });
        refresh();
    }

    void importGdtf()
    {
        chooser = std::make_unique<juce::FileChooser> ("Import GDTF fixture", juce::File(), "*.gdtf");
        chooser->launchAsync (juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles,
                              [this] (const juce::FileChooser& fc)
                              {
                                  const auto file = fc.getResult();
                                  if (! file.existsAsFile())
                                      return;
                                  engine.pauseWhile ([&] { library.importGdtf (file); });
                                  refresh();
                              });
    }

    void importMvr()
    {
        chooser = std::make_unique<juce::FileChooser> ("Import MVR scene", juce::File(), "*.mvr");
        chooser->launchAsync (juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles,
                              [this] (const juce::FileChooser& fc)
                              {
                                  const auto file = fc.getResult();
                                  if (! file.existsAsFile())
                                      return;
                                  const int fid = nextFid();
                                  engine.pauseWhile ([&]
                                  {
                                      if (auto scene = mvr::importMvrFile (file, library))
                                          mvr::applyToPatch (show, library, *scene, fid);
                                  });
                                  refresh();
                              });
    }

    ShowDocument& show;
    gdtf::FixtureLibrary& library;
    Engine& engine;

    std::vector<const gdtf::FixtureType*> libTypes;
    std::vector<ShowDocument::Fixture> patched;
    std::set<int> conflicts;
    int selectedType = -1;

    juce::TextButton importGdtfBtn { "Import GDTF" }, importMvrBtn { "Import MVR" }, addBtn { "Add ->" };
    juce::Label libLabel, patchLabel;
    LambdaListBoxModel libModel, patchModel;
    juce::ListBox libList, patchList;
    std::unique_ptr<juce::FileChooser> chooser;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PatchPanel)
};
}

/*
------------------------------------------------------------------

This file is part of a plugin for the Open Ephys GUI
Copyright (C) 2019 Translational NeuroEngineering Laboratory

------------------------------------------------------------------

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/

#include "CoherenceVisualizer.h"

CoherenceVisualizer::CoherenceVisualizer(CoherenceNode* n)
    : viewport  (new Viewport())
    , canvas    (new Component("canvas"))
    , processor (n)
    , freqStart (processor->freqStart)
    , freqEnd   (processor->freqEnd)
    , canvasBounds  (0, 0, 1, 1)
{
    refreshRate = 2;
    ;
    juce::Rectangle<int> bounds;
    curComb = 0;

    const int TEXT_HT = 18;

    // ------- Options Title ------- //
    int col1 = 15;
    int yPos = 30;
    optionsTitle = new Label("OptionsTitle", "Coherence Viewer Additional Settings");
    optionsTitle->setBounds(bounds = { col1 - 5, yPos, 400, 50 });
    optionsTitle->setFont(Font(20, Font::bold));
    canvas->addAndMakeVisible(optionsTitle);
    canvasBounds = canvasBounds.getUnion(bounds);

    channelGroupSet = new VerticalGroupSet("Channel Groups");
    canvas->addAndMakeVisible(channelGroupSet, 0);

    // ------- Combination Label ------- // 
    
    combinationGroupSet = new VerticalGroupSet("Combination Set");
    canvas->addAndMakeVisible(combinationGroupSet, 0);


    combinationLabel = new Label("CombinationLabel", "Comb To Graph");
    combinationLabel->setBounds(bounds = { col1 + 500, yPos, 90, TEXT_HT });
    //combinationLabel->setColour(Label::backgroundColourId, Colours::grey);
    combinationLabel->setFont(Font(14, Font::bold));
    canvas->addAndMakeVisible(combinationLabel);
    canvasBounds = canvasBounds.getUnion(bounds);


    yPos += TEXT_HT + 5;
    // ------- Combination Choice ------- //
    combinationBox = new ComboBox("Combination Selection Box");
    combinationBox->setTooltip("Combination to graph");
    combinationBox->setBounds(bounds = { col1 + 500, yPos, 90, TEXT_HT });
    combinationBox->addListener(this);
    canvas->addAndMakeVisible(combinationBox);
    canvasBounds = canvasBounds.getUnion(bounds);
    updateCombList();

    combinationGroupSet->addGroup({ combinationLabel, combinationBox });

    yPos = 90;
    //xPos = 15;
    // ------- Grouping Titles ------- //
    group1Title = new Label("Group1Title", "G1 Chans");
    group1Title->setBounds(bounds = { col1, yPos, 50, 50 });
    group1Title->setFont(Font(20, Font::bold));
    canvas->addAndMakeVisible(group1Title);
    canvasBounds = canvasBounds.getUnion(bounds);

    group2Title = new Label("Group2Title", "G2 Chans");
    group2Title->setBounds(bounds = { col1 + 50, yPos, 50, 50 });
    group2Title->setFont(Font(20, Font::bold));
    canvas->addAndMakeVisible(group2Title);
    canvasBounds = canvasBounds.getUnion(bounds);

    

    // ------- Group Boxes ------- //
	int numInputs = processor->getActiveInputs().size();
    group1Channels = processor->group1Channels;
    group2Channels = processor->group2Channels;
	for (int i = 0; i < numInputs; i+=1)
	{
        createElectrodeButton(i);
	}

    updateGroupState();
    channelGroupSet->addGroup({ group1Title, group2Title });


    
    static const String linearTip = "Linear weighting of coherence.";
    static const String expTip = "Exponential weighting of coherence. Set alpha using -1/alpha weighting.";
    static const String resetTip = "Clears and resets the algorithm. Must be done after changes are made on this page!";

    // Column 2
    yPos = 90;
    int col2 = 140;

    columnTwoSet = new VerticalGroupSet("Column 2");
    canvas->addAndMakeVisible(columnTwoSet, 0);
    // ------- Reset Button ------- //
    resetTFR = new TextButton("Reset");
    resetTFR->setBounds(bounds = { col2, yPos, 90, TEXT_HT + 15 });
    resetTFR->addListener(this);
    resetTFR->setTooltip(resetTip);
    Colour col = (processor->ready) ? Colours::green : Colours::red;
    resetTFR->setColour(TextButton::buttonColourId, col);
    canvas->addAndMakeVisible(resetTFR);
    canvasBounds = canvasBounds.getUnion(bounds);

    // ------- Clear Group Button ------- //
    yPos += 50;
    clearGroups = new TextButton("Clear Groups");
    clearGroups->setBounds(bounds = { col2, yPos, 90, TEXT_HT });
    clearGroups->addListener(this);
    canvas->addAndMakeVisible(clearGroups);
    canvasBounds = canvasBounds.getUnion(bounds);

    // ------- Default Group Button ------- //
    yPos += 40;
    defaultGroups = new TextButton("Default Groups");
    defaultGroups->setBounds(bounds = { col2, yPos, 90, TEXT_HT });
    defaultGroups->addListener(this);
    canvas->addAndMakeVisible(defaultGroups);
    canvasBounds = canvasBounds.getUnion(bounds);

    columnTwoSet->addGroup({ resetTFR, clearGroups, defaultGroups });

    // ------- Exponential or Linear Button ------- //
    yPos += 40;
    linearButton = new ToggleButton("Linear");
    linearButton->setBounds(bounds = { col2, yPos, 90, TEXT_HT });
    linearButton->setToggleState(true, dontSendNotification);
    linearButton->addListener(this);
    linearButton->setTooltip(linearTip);
    canvas->addAndMakeVisible(linearButton);
    canvasBounds = canvasBounds.getUnion(bounds);

    yPos += 20;
    expButton = new ToggleButton("Exponential");
    expButton->setBounds(bounds = { col2, yPos, 90, TEXT_HT });
    expButton->setToggleState(false, dontSendNotification);
    expButton->addListener(this);
    expButton->setTooltip(expTip);
    //expButton->setFont(Font(16, Font::bold)); // lets change the font/background here
    canvas->addAndMakeVisible(expButton);
    canvasBounds = canvasBounds.getUnion(bounds);

    // ------- Alpha ------- //
    //xPos += 15;
    yPos += 20;
    alpha = new Label("alpha", "Alpha: ");
    alpha->setBounds(bounds = { col2 + 15, yPos, 45, TEXT_HT });
    //alpha->setColour(Label::backgroundColourId, Colours::grey);
    canvas->addAndMakeVisible(alpha);
    canvasBounds = canvasBounds.getUnion(bounds);

    //xPos += 50;
    alphaE = new Label("alphaE", "0.3");
    alphaE->setEditable(true);
    alphaE->addListener(this);
    alphaE->setBounds(bounds = { col2 + 65, yPos, 30, TEXT_HT });
    alphaE->setColour(Label::backgroundColourId, Colours::grey);
    alphaE->setColour(Label::textColourId, Colours::white);
    canvas->addAndMakeVisible(alphaE);
    canvasBounds = canvasBounds.getUnion(bounds);

    columnTwoSet->addGroup({ linearButton, expButton, alpha, alphaE });

    // ------- Artifact Threshold ------- //
    static const String artifactTip = "Checks the current power value minus the last power value. If the change is too large it is considered an artifact and the current buffer will be reset.";
    static const String artifactNumTip = "Current number of buffers finished vs how many have been discarded because of artifacts.";

    //xPos -= 65;
    yPos += 40;
    artifactDesc = new Label("artifactDesc", "Artifact Threshold:");
    artifactDesc->setBounds(bounds = { col2, yPos, 120, TEXT_HT });
    //artifactDesc->setColour(Label::backgroundColourId, Colours::grey);
    artifactDesc->setTooltip(artifactTip);
    canvas->addAndMakeVisible(artifactDesc);
    canvasBounds = canvasBounds.getUnion(bounds);

    yPos += 20;
    artifactEq = new Label("artifactEq", "| x[k] - x[k-1] | >=");
    artifactEq->setBounds(bounds = { col2, yPos, 110, TEXT_HT });
    //artifactEq->setColour(Label::backgroundColourId, Colours::grey);
    artifactEq->setTooltip(artifactTip);
    canvas->addAndMakeVisible(artifactEq);
    canvasBounds = canvasBounds.getUnion(bounds);

    //xPos += 115;
    artifactE = new Label("artifactE", "3000");
    artifactE->setEditable(true);
    artifactE->addListener(this);
    artifactE->setBounds(bounds = { col2 + 115, yPos, 50, TEXT_HT });
    artifactE->setColour(Label::backgroundColourId, Colours::grey);
    artifactE->setColour(Label::textColourId, Colours::white);
    artifactE->setTooltip(artifactTip);
    canvas->addAndMakeVisible(artifactE);
    canvasBounds = canvasBounds.getUnion(bounds);

    yPos += 20;
    //xPos -= 135;
    artifactCount = new Label("artifactDesc", "UPDATE IF ARTIFACTS");
    artifactCount->setBounds(bounds = { col2 - 20, yPos, 200, TEXT_HT });
    artifactCount->setColour(Label::backgroundColourId, Colours::red);
    artifactE->setColour(Label::textColourId, Colours::white);
    artifactCount->setTooltip(artifactNumTip);
    canvasBounds = canvasBounds.getUnion(bounds);
    //canvas->addAndMakeVisible(artifactDesc);

    columnTwoSet->addGroup({ artifactDesc, artifactEq, artifactE });

    // ------- Frequencies of Interest ------- //
    yPos += 40;
    //xPos += 20;
    // Frequencies of interest
    foiLabel = new Label("foiLabel", "Frequencies of Interest");
    foiLabel->setBounds(bounds = { col2, yPos, 150, TEXT_HT });
    //foiLabel->setColour(Label::backgroundColourId, Colours::grey);
    canvas->addAndMakeVisible(foiLabel);
    canvasBounds = canvasBounds.getUnion(bounds);

    int freqLabelWidth = 100;
    // Start freq
    yPos += 20;
    fstartLabel = new Label("fstartLabel", "Freq Start(Hz):");
    fstartLabel->setBounds(bounds = { col2, yPos, freqLabelWidth, TEXT_HT });
    //fstartLabel->setColour(Label::backgroundColourId, Colours::grey);
    canvas->addAndMakeVisible(fstartLabel);
    canvasBounds = canvasBounds.getUnion(bounds);

    //xPos += freqLabelWidth + 10;
    fstartEditable = new Label("fstartEditable", "1");
    fstartEditable->setEditable(true);
    fstartEditable->addListener(this);
    fstartEditable->setBounds(bounds = { col2 + freqLabelWidth + 10, yPos, 40, TEXT_HT });
    fstartEditable->setColour(Label::backgroundColourId, Colours::grey);
    fstartEditable->setColour(Label::textColourId, Colours::white);
    canvas->addAndMakeVisible(fstartEditable);
    canvasBounds = canvasBounds.getUnion(bounds);
    //xPos -= freqLabelWidth + 10;

    // End Freq
    yPos += 20;
    fendLabel = new Label("fendLabel", "Freq End(Hz):");
    fendLabel->setBounds(bounds = { col2, yPos, freqLabelWidth, TEXT_HT });
    //fendLabel->setColour(Label::backgroundColourId, Colours::grey);
    canvas->addAndMakeVisible(fendLabel);
    canvasBounds = canvasBounds.getUnion(bounds);

    //xPos += freqLabelWidth + 10;
    fendEditable = new Label("fendEditable", "40");
    fendEditable->setEditable(true);
    fendEditable->addListener(this);
    fendEditable->setBounds(bounds = { col2 + freqLabelWidth + 10, yPos, 40, TEXT_HT });
    fendEditable->setColour(Label::backgroundColourId, Colours::grey);
    fendEditable->setColour(Label::textColourId, Colours::white);
    canvas->addAndMakeVisible(fendEditable);
    canvasBounds = canvasBounds.getUnion(bounds);
    //xPos -= freqLabelWidth + 10;

    columnTwoSet->addGroup({ foiLabel, fstartLabel, fstartEditable, fendLabel, fendEditable });

    // ------- Plot ------- //
    // col 3
    int col3 = 330;
    cohPlot = new MatlabLikePlot();
    cohPlot->setBounds(bounds = { col3, 90, 600, 500 });
    //cohPlot->setAuxiliaryString("Hz x Coh"); //Confusing with the base string on the graph.
    cohPlot->setTitle("Coherence at Selected Combination");
    cohPlot->setRange(freqStart, freqEnd, 0.0, 100, true);  
    cohPlot->setControlButtonsVisibile(false);

    canvas->addAndMakeVisible(cohPlot);
    canvasBounds = canvasBounds.getUnion(bounds);
    
    // some extra padding
    canvasBounds.setBottom(canvasBounds.getBottom() + 10);
    canvasBounds.setRight(canvasBounds.getRight() + 10);

    canvas->setBounds(canvasBounds);
    channelGroupSet->setBounds(canvasBounds);
    combinationGroupSet->setBounds(canvasBounds);
    columnTwoSet->setBounds(canvasBounds);
    viewport->setViewedComponent(canvas, false);
    viewport->setScrollBarsShown(true, true);
    addAndMakeVisible(viewport);

    startCallbacks();
}


CoherenceVisualizer::~CoherenceVisualizer()
{
    stopCallbacks();
}

void CoherenceVisualizer::resized()
{
    viewport->setSize(getWidth(), getHeight());
}

void CoherenceVisualizer::refreshState() {}
void CoherenceVisualizer::update() 
{
    int numInputs = processor->getActiveInputs().size();
    int numButtons = group1Buttons.size();
    updateElectrodeButtons(numInputs, numButtons);
    
    float alpha = processor->alpha;
    if (alpha != 0.0)
    {
        linearButton->setToggleState(false, dontSendNotification);

        expButton->setToggleState(true, dontSendNotification);
        alphaE->setText(String(alpha), dontSendNotification);
    }
}

void CoherenceVisualizer::updateElectrodeButtons(int numInputs, int numButtons)
{
    juce::Rectangle<int> bounds;

    juce::Rectangle<int> canvasBounds = canvas->getBounds();

    int xPos = 5;
    group1Channels = processor->group1Channels;
    group2Channels = processor->group2Channels;
    if (numInputs > numButtons)
    {
        for (int i = numButtons; i < numInputs; i++)
        {
            createElectrodeButton(i);
        }
    }
    else
    {
        for (int i = numInputs; i < numButtons; i++)
        {
            group1Buttons[i]->~ElectrodeButton();
            group2Buttons[i]->~ElectrodeButton();
        }

        group1Buttons.removeLast(numButtons - numInputs);
        group2Buttons.removeLast(numButtons - numInputs);
    }

    updateGroupState();
    updateCombList();
}

void CoherenceVisualizer::updateCombList()
{
    combinationBox->clear(dontSendNotification);
    combinationBox->addItem("Average across all combinations", 1);
    for (int i = 0, comb = 2; i < group1Channels.size(); i++)
    {
        for (int j = 0; j < group2Channels.size(); j++, ++comb)
        {
            // using 1-based comb ids since 0 is reserved for "nothing selected"
            combinationBox->addItem(String(group1Channels[i] + 1) + " x " + String(group2Channels[j] + 1), comb);
        }
    }
    if (group1Channels.size() > 0 && group2Channels.size() > 0)
    {
        combinationBox->setSelectedId(1);
    }  
}

void CoherenceVisualizer::updateGroupState()
{
    for (int i = 0; i < group1Buttons.size(); i++)
    {
        if (group1Channels.contains(group1Buttons[i]->getChannelNum()-1))
        {
            group1Buttons[i]->setToggleState(true, dontSendNotification);
        }
        else
        {
            group1Buttons[i]->setToggleState(false, dontSendNotification);
        }
    }

    for (int i = 0; i < group2Buttons.size(); i++)
    {
        if (group2Channels.contains(group2Buttons[i]->getChannelNum()-1))
        {
            group2Buttons[i]->setToggleState(true, dontSendNotification);
        }
        else
        {
            group2Buttons[i]->setToggleState(false, dontSendNotification);
        }
    } 
}

void CoherenceVisualizer::paint(Graphics& g)
{
    // To make background not black.
    ColourGradient editorBg = processor->getEditor()->getBackgroundGradient();
    g.fillAll(editorBg.getColourAtPosition(0.5)); // roughly matches editor background (without gradient)
}

void CoherenceVisualizer::refresh() 
{
    // If we have any artifacts let the user know
    if (processor->numArtifacts > 0)
    {
        artifactCount->setText(String("Buffers Handled: " + String(processor->numTrials) + " & Buffers Discarded: " + String(ceil(processor->numArtifacts))), dontSendNotification);
        if (!viewport->isParentOf(artifactCount))
        {
            addAndMakeVisible(artifactCount);
        }
    }
    else
    {
        removeChildComponent(getIndexOfChildComponent(artifactCount));
    }

    // Update plot if frequency has changed.
    if (freqStart != processor->freqStart || freqEnd != processor->freqEnd)
    {
        freqStart = processor->freqStart;
        freqEnd = processor->freqEnd;
        cohPlot->setRange(freqStart, freqEnd, 0, 100, true);
    }

    freqStep = processor->freqStep;
    Colour col = (processor->ready) ? Colours::green : Colours::red;
    resetTFR->setColour(TextButton::buttonColourId, col);

    // Get data from processor thread, then plot
    if (processor->meanCoherence.hasUpdate())
    {
        AtomicScopedReadPtr<std::vector<std::vector<double>>> coherenceReader(processor->meanCoherence);
        coherenceReader.pullUpdate();

        coh.resize(coherenceReader->size());
        
        for (int comb = 0; comb < processor->nGroup1Chans * processor->nGroup2Chans; comb++)
        {
            int vecSize = coherenceReader->at(comb).size();
            coh[comb].resize(vecSize);
            for (int i = 0; i < vecSize; i++)
            {
                coh[comb][i] = coherenceReader->at(comb)[i] * 100;
            }
        }
    }

    if (coh.size() > 0)
    {
        XYline cohLine(0, 1, 1, Colours::yellow);
        if (curComb >= 0)
        {
            cohLine = XYline(freqStart, freqStep, coh[curComb], 1, Colours::yellow);
        }
        else
        {
            // Average across all combinations
            std::vector<float> averageCoh(coh[0].size()) ;
            for (int comb = 0; comb < processor->nGroup1Chans * processor->nGroup2Chans; comb++)
            {
                for (int i = 0; i < averageCoh.size(); i++)
                {
                    averageCoh[i] = coh[comb][i] + averageCoh[i];
                }
            }
            for (int i = 0; i < averageCoh.size(); i++)
            {
                averageCoh[i] /= coh.size();
            }
            cohLine = XYline(freqStart, freqStep, averageCoh, 1, Colours::yellow);
        }
        

        cohPlot->clearplot();
        cohPlot->plotxy(cohLine);
        cohPlot->repaint();
    }
}

void CoherenceVisualizer::labelTextChanged(Label* labelThatHasChanged)
{
    if (labelThatHasChanged == artifactE)
    {
        float newVal;
        if (updateFloatLabel(labelThatHasChanged, 0, FLT_MAX, 3000, &newVal))
        {
            processor->setParameter(processor->ARTIFACT_THRESHOLD, newVal);
        }
    }
    else
    {
        processor->updateReady(false);
    }
    
    

    if (labelThatHasChanged == alphaE)
    {
        float newVal;
        if (updateFloatLabel(labelThatHasChanged, 0, FLT_MAX, .3, &newVal))
        {
            if (expButton->getState())
            {
                processor->updateAlpha(newVal);
                processor->updateReady(false);
            }
        }
    }

    if (labelThatHasChanged == fstartEditable)
    {
        int newVal;
        if (updateIntLabel(labelThatHasChanged, 0, INT_MAX, 8, &newVal))
        {
            processor->setParameter(CoherenceNode::START_FREQ, static_cast<int>(newVal));
        }
    }
    if (labelThatHasChanged == fendEditable)
    {
        int newVal;
        if (updateIntLabel(labelThatHasChanged, 0, INT_MAX, 8, &newVal))
        {
            processor->setParameter(CoherenceNode::END_FREQ, static_cast<int>(newVal));
        }
    }
}

void CoherenceVisualizer::comboBoxChanged(ComboBox* comboBoxThatHasChanged)
{
    if (comboBoxThatHasChanged == combinationBox)
    {
        curComb = static_cast<int>(combinationBox->getSelectedId() - 2);
    }
}

void CoherenceVisualizer::buttonClicked(Button* buttonClicked)
{
    if (buttonClicked == resetTFR)
    {
        processor->resetTFR();
    }
    // Button was clicked that wasn't reseting the TFR, something important has changed. Tell node that we need to reset.
    else
    {
        processor->updateReady(false);
    }

 
    if (buttonClicked == clearGroups)
    {
        group1Channels.clear();
        group2Channels.clear();

        processor->updateGroup(group1Channels, group2Channels);

        updateGroupState();
        updateCombList();
    }

    if (buttonClicked == defaultGroups)
    {
        group1Channels.clear();
        group2Channels.clear();

        int numInputs = processor->getNumInputs();
        for (int i = 0; i < numInputs; i++)
        {
            if (i < numInputs / 2)
            {
                group1Channels.add(i);
            }
            else
            {
                group2Channels.add(i);
            }
        }

        processor->updateGroup(group1Channels, group2Channels);

        updateGroupState();
        updateCombList();
    }

    if (buttonClicked == linearButton)
    {
        expButton->setToggleState(false, dontSendNotification);

        processor->updateAlpha(0);
    }
    
    if (buttonClicked == expButton)
    {
        linearButton->setToggleState(false, dontSendNotification);
        processor->updateAlpha(alphaE->getText().getFloatValue());
    }

    if (group1Buttons.contains((ElectrodeButton*)buttonClicked))
    {
        ElectrodeButton* eButton = static_cast<ElectrodeButton*>(buttonClicked);
        int buttonChan = eButton->getChannelNum() - 1;
        // Add to group 1 channels
        // Make sure to check that not in group2Buttons
        if (group1Channels.contains(buttonChan))
        {
            int it = group1Channels.indexOf(buttonChan);
            group1Channels.remove(it);
        }

        else
        {
            if (group2Channels.contains(buttonChan))
            {
                group2Buttons[buttonChan]->setToggleState(false, dontSendNotification);
                int it = group2Channels.indexOf(buttonChan);
                group2Channels.remove(it);
            }

            group1Channels.addUsingDefaultSort(buttonChan);
        }
        processor->updateGroup(group1Channels, group2Channels);
        updateCombList();
    }
    if (group2Buttons.contains((ElectrodeButton*)buttonClicked))
    {
        ElectrodeButton* eButton = static_cast<ElectrodeButton*>(buttonClicked);
        int buttonChan = eButton->getChannelNum() - 1;
        if (group2Channels.contains(buttonChan))
        {
            int it = group2Channels.indexOf(buttonChan);
            group2Channels.remove(it);
        }
        
        else
        {
            if (group1Channels.contains(buttonChan))
            {
                group1Buttons[buttonChan]->setToggleState(false, dontSendNotification);
                int it = group1Channels.indexOf(buttonChan);
                group1Channels.remove(it);
            }

            group2Channels.addUsingDefaultSort(buttonChan);
        }      
        processor->updateGroup(group1Channels, group2Channels);
        updateCombList();
    }

    Colour col = (processor->ready) ? Colours::green : Colours::red;
    resetTFR->setColour(TextButton::buttonColourId, col);
}

void CoherenceVisualizer::channelChanged(int chan, bool newState)
{
    int buttonChan = chan + 1;
   // Only do if not during data acquistion!
    if (newState)
    {
        // New channel, add button
        createElectrodeButton(chan);
    }
    else
    {
        for (int i = 0; i < group1Buttons.size(); i++)
        {
            // Channel unactivated
            if (group1Buttons[i]->getChannelNum() == buttonChan)
            {
                group1Buttons[i]->~ElectrodeButton();
                group1Buttons.remove(i);
                group2Buttons[i]->~ElectrodeButton();
                group2Buttons.remove(i);
            }
            if (group1Channels.contains(chan))
            {
                // Groups changed, update TFR
                processor->updateReady(false);
                // Remove from group
                group1Channels.removeFirstMatchingValue(chan);
                processor->updateGroup(group1Channels, group2Channels);
            }
            if (group2Channels.contains(chan))
            {
                // Groups changed, update TFR
                processor->updateReady(false);
                // Remove from group
                group2Channels.removeFirstMatchingValue(chan);
                processor->updateGroup(group1Channels, group2Channels);
            }
        }
    }
    updateGroupState();
    updateCombList();
}

void CoherenceVisualizer::createElectrodeButton(int chan)
{
    int xPos = 15;
    juce::Rectangle<int> bounds;

    // Group 1 buttons
    ElectrodeButton* button = new ElectrodeButton(chan + 1);
    button->setBounds(bounds = { xPos + 5, 140 + chan * 15, 20, 15 });
    button->setRadioGroupId(0);
    button->setButtonText(String(chan + 1));
    button->addListener(this);
    canvasBounds = canvasBounds.getUnion(bounds);

    canvas->addAndMakeVisible(button);

    // Group 2 buttons
    ElectrodeButton* button2 = new ElectrodeButton(chan + 1);
    button2->setBounds(bounds = { xPos + 55, 140 + chan * 15, 20, 15 });
    button2->setRadioGroupId(0);
    button2->setButtonText(String(chan + 1));
    button2->addListener(this);
    canvasBounds = canvasBounds.getUnion(bounds);

    canvas->addAndMakeVisible(button2);

    canvas->setBounds(canvasBounds);
    group1Buttons.insert(chan, button);
    group2Buttons.insert(chan, button2);

    channelGroupSet->addGroup({ button, button2 });
}

void CoherenceVisualizer::beginAnimation() 
{
    // Can't change things during data acq.
    for (int i = 0; i < group1Buttons.size(); i++)
    {
        group1Buttons[i]->setEnabled(false);
    }
    for (int i = 0; i < group2Buttons.size(); i++)
    {
        group2Buttons[i]->setEnabled(false);
    }

    resetTFR->setEnabled(false);
    clearGroups->setEnabled(false);
    defaultGroups->setEnabled(false);
    linearButton->setEnabled(false);
    expButton->setEnabled(false);
    alphaE->setEditable(false);
}
void CoherenceVisualizer::endAnimation() 
{
    // allow things to change again
    for (int i = 0; i < group1Buttons.size(); i++)
    {
        group1Buttons[i]->setEnabled(true);
    }
    for (int i = 0; i < group2Buttons.size(); i++)
    {
        group2Buttons[i]->setEnabled(true);
    }

    resetTFR->setEnabled(true);
    clearGroups->setEnabled(true);
    defaultGroups->setEnabled(true);
    linearButton->setEnabled(true);
    expButton->setEnabled(true);
    alphaE->setEditable(false);
}

bool CoherenceVisualizer::updateFloatLabel(Label* label, float min, float max,
    float defaultValue, float* out)
{
    const String& in = label->getText();
    float parsedFloat;
    try
    {
        parsedFloat = std::stof(in.toRawUTF8());
    }
    catch (const std::logic_error&)
    {
        label->setText(String(defaultValue), dontSendNotification);
        return false;
    }

    *out = jmax(min, jmin(max, parsedFloat));

    label->setText(String(*out), dontSendNotification);
    return true;
}

bool CoherenceVisualizer::updateIntLabel(Label* label, int min, int max, int defaultValue, int* out)
{
    const String& in = label->getText();
    int parsedInt;
    try
    {
        parsedInt = std::stoi(in.toRawUTF8());
    }
    catch (const std::logic_error&)
    {
        label->setText(String(defaultValue), dontSendNotification);
        return false;
    }

    *out = jmax(min, jmin(max, parsedInt));

    label->setText(String(*out), dontSendNotification);
    return true;
}


/************ VerticalGroupSet ****************/

VerticalGroupSet::VerticalGroupSet(Colour backgroundColor)
    : Component()
    , bgColor(backgroundColor)
    , leftBound(INT_MAX)
    , rightBound(INT_MIN)
{}

VerticalGroupSet::VerticalGroupSet(const String& componentName, Colour backgroundColor)
    : Component(componentName)
    , bgColor(backgroundColor)
    , leftBound(INT_MAX)
    , rightBound(INT_MIN)
{}

VerticalGroupSet::~VerticalGroupSet() {}

void VerticalGroupSet::addGroup(std::initializer_list<Component*> components)
{
    if (!getParentComponent())
    {
        jassertfalse;
        return;
    }

    DrawableRectangle* thisGroup;
    groups.add(thisGroup = new DrawableRectangle);
    addChildComponent(thisGroup);
    const RelativePoint cornerSize(CORNER_SIZE, CORNER_SIZE);
    thisGroup->setCornerSize(cornerSize);
    thisGroup->setFill(bgColor);

    int topBound = INT_MAX;
    int bottomBound = INT_MIN;
    for (auto component : components)
    {
        Component* componentParent = component->getParentComponent();
        if (!componentParent)
        {
            jassertfalse;
            return;
        }
        int width = component->getWidth();
        int height = component->getHeight();
        Point<int> positionFromItsParent = component->getPosition();
        Point<int> localPosition = getLocalPoint(componentParent, positionFromItsParent);

        // update bounds
        leftBound = jmin(leftBound, localPosition.x - PADDING);
        rightBound = jmax(rightBound, localPosition.x + width + PADDING);
        topBound = jmin(topBound, localPosition.y - PADDING);
        bottomBound = jmax(bottomBound, localPosition.y + height + PADDING);
    }

    // set new background's bounds
    auto bounds = juce::Rectangle<float>::leftTopRightBottom(leftBound, topBound, rightBound, bottomBound);
    thisGroup->setRectangle(bounds);
    thisGroup->setVisible(true);

    // update all other children
    for (DrawableRectangle* group : groups)
    {
        if (group == thisGroup) { continue; }

        topBound = group->getPosition().y;
        bottomBound = topBound + group->getHeight();
        bounds = juce::Rectangle<float>::leftTopRightBottom(leftBound, topBound, rightBound, bottomBound);
        group->setRectangle(bounds);
    }
}
void CoherenceVisualizer::setParameter(int, float) {}
void CoherenceVisualizer::setParameter(int, int, int, float) {}
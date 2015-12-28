/*==================================================================================
    Copyright (c) 2008, binaryzebra
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
    * Neither the name of the binaryzebra nor the
    names of its contributors may be used to endorse or promote products
    derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE binaryzebra AND CONTRIBUTORS "AS IS" AND
    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL binaryzebra BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
=====================================================================================*/


#include "Input/InputSystem.h"
#include "Input/KeyboardDevice.h"
#include "EditorSystems/SelectionSystem.h"
#include "Model/PackageHierarchy/ControlNode.h"
#include "UI/UIEvent.h"
#include "UI/UIControl.h"
#include "EditorSystems/EditorSystemsManager.h"
#include "EditorSystems/KeyboardProxy.h"
#include "Model/PackageHierarchy/PackageNode.h"
#include "Model/PackageHierarchy/PackageControlsNode.h"

using namespace DAVA;

SelectionSystem::SelectionSystem(EditorSystemsManager* parent)
    : BaseEditorSystem(parent)
{
    systemManager->GetPackage()->AddListener(this);
    systemManager->SelectionRectChanged.Connect(this, &SelectionSystem::OnSelectByRect);
    systemManager->SelectAllControls.Connect(this, &SelectionSystem::SelectAllControls);
    systemManager->FocusNextChild.Connect(this, &SelectionSystem::FocusNextChild);
    systemManager->FocusPreviousChild.Connect(this, &SelectionSystem::FocusPreviousChild);
}

SelectionSystem::~SelectionSystem()
{
    PackageNode* package = systemManager->GetPackage();
    if (nullptr != package)
    {
        systemManager->GetPackage()->RemoveListener(this);
    }
}

void SelectionSystem::OnActivated()
{
    if (!selectionContainer.selectedNodes.empty())
    {
        systemManager->SelectionChanged.Emit(selectionContainer.selectedNodes, SelectedNodes());
    }
    connectionID = systemManager->SelectionChanged.Connect(this, &SelectionSystem::OnSelectionChanged);
}

void SelectionSystem::OnDeactivated()
{
    systemManager->SelectionChanged.Disconnect(connectionID);
    if (!selectionContainer.selectedNodes.empty())
    {
        systemManager->SelectionChanged.Emit(SelectedNodes(), selectionContainer.selectedNodes);
    }
}

bool SelectionSystem::OnInput(UIEvent* currentInput)
{
    switch (currentInput->phase)
    {
    case UIEvent::Phase::BEGAN:
        mousePressed = true;
        return ProcessMousePress(currentInput->point, static_cast<UIEvent::eButtonID>(currentInput->tid));
    case UIEvent::Phase::ENDED:
        if (!mousePressed)
        {
            return ProcessMousePress(currentInput->point, static_cast<UIEvent::eButtonID>(currentInput->tid));
        }
        mousePressed = false;
        return false;
    default:
        return false;
    }
    return false;
}

void SelectionSystem::ControlWasRemoved(ControlNode* node, ControlsContainerNode*)
{
    SelectedNodes deselected;
    deselected.insert(node);
    SetSelection(SelectedNodes(), deselected);
}

void SelectionSystem::OnSelectByRect(const Rect& rect)
{
    SelectedNodes deselected;
    SelectedNodes selected;
    Set<ControlNode*> areaNodes;
    auto predicate = [rect](const UIControl* control) -> bool {
        return control->GetSystemVisible() && rect.RectContains(control->GetGeometricData().GetAABBox());
    };
    systemManager->CollectControlNodes(std::inserter(areaNodes, areaNodes.end()), predicate);
    if (!areaNodes.empty())
    {
        for (auto node : areaNodes)
        {
            selected.insert(node);
        }
    }
    if (!IsKeyPressed(KeyboardProxy::KEY_SHIFT))
    {
        //deselect all not selected by rect
        std::set_difference(selectionContainer.selectedNodes.begin(), selectionContainer.selectedNodes.end(), areaNodes.begin(), areaNodes.end(), std::inserter(deselected, deselected.end()));
    }
    SetSelection(selected, deselected);
}

void SelectionSystem::SelectAllControls()
{
    SelectedNodes selected;
    systemManager->CollectControlNodes(std::inserter(selected, selected.end()), [](const UIControl*) { return true; });
    SetSelection(selected, SelectedNodes());
}

void SelectionSystem::FocusNextChild()
{
    FocusToChild(true);
}

void SelectionSystem::FocusPreviousChild()
{
    FocusToChild(false);
}

void SelectionSystem::FocusToChild(bool next)
{
    PackageBaseNode* startNode = nullptr;
    if (!selectionContainer.selectedNodes.empty())
    {
        startNode = *selectionContainer.selectedNodes.rbegin();
    }
    PackageBaseNode* nextNode = nullptr;
    Vector<PackageBaseNode*> allNodes;
    systemManager->CollectControlNodes(std::back_inserter(allNodes), [](const UIControl*) { return true; });
    if (allNodes.empty())
    {
        return;
    }
    auto findIt = std::find(allNodes.begin(), allNodes.end(), startNode);
    if (findIt == allNodes.end())
    {
        nextNode = next ? allNodes.front() : allNodes.back();
    }
    else if (next)
    {
        ++findIt;
        nextNode = findIt == allNodes.end() ? allNodes.front() : *findIt;
    }
    else
    {
        nextNode = findIt == allNodes.begin() ? allNodes.back() : *(--findIt);
    }

    SelectedNodes newSelectedNodes;
    newSelectedNodes.insert(nextNode);
    SetSelection(newSelectedNodes, selectionContainer.selectedNodes);
}

bool SelectionSystem::ProcessMousePress(const DAVA::Vector2& point, UIEvent::eButtonID buttonID)
{
    SelectedNodes selected;
    SelectedNodes deselected;
    Vector<ControlNode*> nodesUnderPoint;
    auto predicate = [point](const UIControl* control) -> bool {
        return control->GetSystemVisible() && control->IsPointInside(point);
    };
    systemManager->CollectControlNodes(std::back_inserter(nodesUnderPoint), predicate);

    if (!nodesUnderPoint.empty())
    {
        auto node = nodesUnderPoint.back();
        if (buttonID == UIEvent::BUTTON_2)
        {
            ControlNode* selectedNode = systemManager->GetControlByMenu(nodesUnderPoint, point);
            if (nullptr != selectedNode)
            {
                node = selectedNode;
            }
            else
            {
                return true; //selection was required but cancelled
            }
        }
        if (!IsKeyPressed(KeyboardProxy::KEY_SHIFT) && !IsKeyPressed(KeyboardProxy::KEY_CTRL))
        {
            deselected = selectionContainer.selectedNodes;
        }
        if (IsKeyPressed(KeyboardProxy::KEY_CTRL) && selectionContainer.IsSelected(node))
        {
            deselected.insert(node);
        }
        else
        {
            selected.insert(node);
        }
    }
    for (auto controlNode : selected)
    {
        deselected.erase(controlNode);
    }
    SetSelection(selected, deselected);
    return !selected.empty() || !deselected.empty();
}

void SelectionSystem::OnSelectionChanged(const SelectedNodes& selected, const SelectedNodes& deselected)
{
    selectionContainer.MergeSelection(selected, deselected);
}

void SelectionSystem::SetSelection(const SelectedNodes& selected, const SelectedNodes& deselected)
{
    SelectedNodes reallySelected;
    SelectedNodes reallyDeselected;
    selectionContainer.GetOnlyExistedItems(deselected, reallyDeselected);
    selectionContainer.GetNotExistedItems(selected, reallySelected);
    selectionContainer.MergeSelection(reallySelected, reallyDeselected);

    if (!reallySelected.empty() || !reallyDeselected.empty())
    {
        systemManager->SelectionChanged.Emit(reallySelected, reallyDeselected);
    }
}

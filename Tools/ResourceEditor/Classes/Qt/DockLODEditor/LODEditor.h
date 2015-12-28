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


#ifndef __LOD_EDITOR_H__
#define __LOD_EDITOR_H__

#include <QWidget>
#include "Tools/QtPosSaver/QtPosSaver.h"


namespace Ui
{
    class LODEditor;
}

class QLabel;
class QDoubleSpinBox;
class QLineEdit;
class SceneEditor2;
class EditorLODSystem;
class EntityGroup;
class Command2;
class QPushButton;
class QFrame;

class LazyUpdater;
class LODEditor: public QWidget
{
    Q_OBJECT

public:
    explicit LODEditor(QWidget* parent = nullptr);
    ~LODEditor();

private slots:
    void LODEditorSettingsButtonReleased();
    void ViewLODButtonReleased();
    void EditLODButtonReleased();
    
    void ForceDistanceStateChanged(bool checked);
    void ForceDistanceChanged(int distance);

    void SceneActivated(SceneEditor2 *scene);
    void SceneDeactivated(SceneEditor2 *scene);
    void SceneSelectionChanged(SceneEditor2 *scene, const EntityGroup *selected, const EntityGroup *deselected);
    void SolidChanged(SceneEditor2 *scene, const DAVA::Entity *entity, bool value); 
    void CommandExecuted(SceneEditor2 *scene, const Command2* command, bool redo);

    void LODDataChanged(SceneEditor2 *scene = nullptr);

    void LODDistanceChangedBySpinbox(double value);
    void LODDistanceChangedBySlider(const QVector<int> &changedLayers, bool continious);

    void ForceLayerActivated(int index);
    void EditorModeChanged(int newMode);

    //TODO: remove after lod editing implementation
    void CopyLODToLod0Clicked();
    void CreatePlaneLODClicked();
    void DeleteFirstLOD();
    void DeleteLastLOD();

private:
    void SetupInternalUI();
    void InitDistanceSpinBox(QLabel *name, QDoubleSpinBox *spinbox, int index);
    void UpdateSpinboxesBorders();

    void SetupSceneSignals();
      
    void SetSpinboxValue(QDoubleSpinBox *spinbox, double value);
    void CreateForceLayerValues(int layersCount);
   
    void InvertFrameVisibility(QFrame *frame, QPushButton *frameButton);

    void SetForceLayerValues(const EditorLODSystem *editorLODSystem, int layersCount);
    void UpdateWidgetVisibility(const EditorLODSystem *editorLODSystem);
    void UpdateLODButtons(const EditorLODSystem *editorLODSystem);
    void UpdateForceLayer(const EditorLODSystem *editorLODSystem);
    void UpdateForceDistance(const EditorLODSystem *editorLODSystem);

    EditorLODSystem *GetCurrentEditorLODSystem();

    void UpdateUI();

private:
    Ui::LODEditor *ui;

    bool frameViewVisible;
    bool frameEditVisible;

    struct DistanceWidget
    {
        QLabel *name;
        QDoubleSpinBox *distance;
        void SetVisible(bool visible);
    };
    DAVA::Map<DAVA::uint32, DistanceWidget> distanceWidgets;

    LazyUpdater* uiUpdater = nullptr;
};

#endif //#ifndef __LOD_EDITOR_H__

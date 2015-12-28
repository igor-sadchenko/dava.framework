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


#ifndef __MATERIAL_EDITOR_H__
#define __MATERIAL_EDITOR_H__

#include <QDialog>
#include <QPointer>
#include <QStandardItemModel>

#include "DAVAEngine.h"

#include "MaterialTemplateModel.h"
#include "Scene/SceneSignals.h"
#include "Tools/QtPosSaver/QtPosSaver.h"
#include "DockProperties/PropertyEditorStateHelper.h"

namespace Ui {
	class MaterialEditor;
}

class QtPropertyDataInspDynamic;

class LazyUpdater;
class MaterialEditor : public QDialog, public DAVA::Singleton<MaterialEditor>
{
	Q_OBJECT

private:
    typedef QMap< int, bool > ExpandMap;

public:
	MaterialEditor(QWidget *parent = 0);
	~MaterialEditor();

	void SelectMaterial(DAVA::NMaterial *material);
	void SelectEntities(DAVA::NMaterial *material);

public slots:
	void sceneActivated(SceneEditor2 *scene);
	void sceneDeactivated(SceneEditor2 *scene);
	void commandExecuted(SceneEditor2 *scene, const Command2 *command, bool redo);
	void materialSelected(const QItemSelection & selected, const QItemSelection & deselected);

    void OnQualityChanged();

protected slots:
	void OnTemplateChanged(int index);
    void OnTemplateButton();
    void OnPropertyEdited(const QModelIndex&);
    void OnAddRemoveButton();

    void OnMaterialAddGlobal(bool checked);
    void OnMaterialRemoveGlobal(bool checked);
    void OnMaterialSave(bool checked);
    void OnMaterialLoad(bool checked);
    void OnMaterialPropertyEditorContextMenuRequest(const QPoint & pos);

protected:
	virtual void showEvent(QShowEvent * event);

	void SetCurMaterial(const QList< DAVA::NMaterial *>& materials);

    void FillBase();
    void FillDynamic(QtPropertyData *root, const FastName& dynamicName);
    void FillIllumination();
    void FillTemplates(const QList<DAVA::NMaterial *>& materials);

    void FillDynamicMember(QtPropertyData* root, DAVA::InspInfoDynamic* dynamic, DAVA::NMaterial* material, const FastName& memberName);
    void FillDynamicMemberInternal(QtPropertyData* root, DAVA::InspInfoDynamic* dynamic, DAVA::InspInfoDynamic::DynamicData& ddata, const FastName& memberName);
    void FillDynamicMembers(QtPropertyData* root, DAVA::InspInfoDynamic* dynamic, DAVA::NMaterial* material, bool isGlobal);

    void ApplyTextureValidator(QtPropertyDataInspDynamic *data);

    void UpdateAllAddRemoveButtons(QtPropertyData *root);
    void UpdateAddRemoveButtonState(QtPropertyDataInspDynamic *data);

    void ClearDynamicMembers(DAVA::NMaterial *material, const DAVA::InspMemberDynamic *dynamicInsp);

    void RefreshMaterialProperties();

private slots:
    void onFilterChanged();
    void onCurrentExpandModeChange( bool mode );
    void onContextMenuPrepare(QMenu *menu);
    void autoExpand();

private:
    enum 
    {
        CHECKED_NOTHING = 0x0,

        CHECKED_TEMPLATE = 0x1,
        CHECKED_GROUP = 0x2,
        CHECKED_PROPERTIES = 0x4,
        CHECKED_TEXTURES = 0x8,

        CHECKED_ALL = 0xff
    };

    QString GetTemplatePath(DAVA::int32 index) const;
    DAVA::uint32 ExecMaterialLoadingDialog(DAVA::uint32 initialState, const QString& inputFile);

    void initActions();
    void initTemplates();
    void setTemplatePlaceholder( const QString& text );

    void StoreMaterialToPreset(DAVA::NMaterial* material, DAVA::KeyedArchive* preset,
                               DAVA::SerializationContext* context) const;
    void StoreMaterialTextures(DAVA::NMaterial* material, const DAVA::InspMember* materialMember,
                               DAVA::KeyedArchive* texturesArchive, DAVA::SerializationContext* context) const;
    void StoreMaterialFlags(DAVA::NMaterial* material, const DAVA::InspMember* materialMember,
                            DAVA::KeyedArchive* flagsArchive) const;
    void StoreMaterialProperties(DAVA::NMaterial* material, const DAVA::InspMember* materialMember,
                                 DAVA::KeyedArchive* propertiesArchive) const;

    void UpdateMaterialFromPresetWithOptions(DAVA::NMaterial* material, DAVA::KeyedArchive* preset,
                                             DAVA::SerializationContext* context, uint32 options);
    void UpdateMaterialPropertiesFromPreset(DAVA::NMaterial* material, DAVA::KeyedArchive* properitesArchive);
    void UpdateMaterialFlagsFromPreset(DAVA::NMaterial* material, DAVA::KeyedArchive* flagsArchive);
    void UpdateMaterialTexturesFromPreset(DAVA::NMaterial* material, DAVA::KeyedArchive* texturesArchive,
                                          const DAVA::FilePath& scenePath);

    QtPropertyData* AddSection(const QString& sectionName);

    void AddMaterialFlagIfNeed(NMaterial* material, const FastName& flagName);
    bool HasMaterialProperty(NMaterial* material, const FastName& paramName);

private:
    QtPosSaver posSaver;
    QList<DAVA::NMaterial*> curMaterials;
    QtPropertyData* baseRoot = nullptr;
    QtPropertyData* flagsRoot = nullptr;
    QtPropertyData* illuminationRoot = nullptr;
    QtPropertyData* propertiesRoot = nullptr;
    QtPropertyData* texturesRoot = nullptr;
    QPointer<MaterialTemplateModel> templatesFilterModel;

    ExpandMap expandMap;
    PropertyEditorStateHelper* treeStateHelper = nullptr;
    Ui::MaterialEditor* ui = nullptr;

    DAVA::FilePath lastSavePath;
    DAVA::uint32 lastCheckState = 0;

    LazyUpdater* materialPropertiesUpdater;
};

#endif

#pragma once

#include "ui_mainwindow.h"

#include "Classes/Qt/Main/ModificationWidget.h"
#include "Classes/Qt/Tools/QtWaitDialog/QtWaitDialog.h"
#include "Classes/Qt/Scene/SceneEditor2.h"
#include "Classes/Qt/GlobalOperations.h"
#include "Classes/Beast/BeastProxy.h"

#include <TArc/Models/RecentMenuItems.h>
#include <TArc/DataProcessing/DataListener.h>
#include <TArc/DataProcessing/DataWrapper.h>
#include <TArc/WindowSubSystem/UI.h>

#include <QtTools/Utils/ShortcutChecker.h>
#include <QtTools/Utils/QtDelayedExecutor.h>

#include <QMainWindow>
#include <QDockWidget>
#include <QPointer>

class RECommandNotificationObject;
class AddSwitchEntityDialog;
class QtLabelWithActions;
class HangingObjectsHeight;
class DeveloperTools;
class VersionInfoWidget;
class DeviceListController;
class SpritesPackerModule;
class ErrorDialogOutput;

namespace DAVA
{
class RenderWidget;
}

class QtMainWindow : public QMainWindow, public GlobalOperations, private DAVA::TArc::DataListener
{
    Q_OBJECT

    static const int GLOBAL_INVALIDATE_TIMER_DELTA = 1000;

signals:
    void GlobalInvalidateTimeout();

public:
    explicit QtMainWindow(DAVA::TArc::UI* tarcUI, QWidget* parent = 0);
    ~QtMainWindow();

    void OnRenderingInitialized();
    void AfterInjectInit();

    void WaitStart(const QString& title, const QString& message, int min, int max);
    void WaitSetMessage(const QString& messsage);
    void WaitSetValue(int value);
    bool IsWaitDialogOnScreen() const;
    void WaitStop();

    void EnableGlobalTimeout(bool enable);

    bool ParticlesArePacking() const;

    void CallAction(ID id, DAVA::Any&& args) override;
    QWidget* GetGlobalParentWidget() const override;
    void ShowWaitDialog(const DAVA::String& tittle, const DAVA::String& message, DAVA::uint32 min, DAVA::uint32 max) override;
    bool IsWaitDialogVisible() const override;
    void HideWaitDialog() override;
    void ForEachScene(const DAVA::Function<void(SceneEditor2*)>& functor) override;

    // qt actions slots
public slots:
    void OnImportSpeedTreeXML();
    void RemoveSelection();

    void OnUndo();
    void OnRedo();

    void OnEditorGizmoToggle(bool show);
    void OnViewLightmapCanvas(bool show);
    void OnShowStaticOcclusionToggle(bool show);
    void OnEnableVisibilitySystemToggle(bool enabled);
    void OnRefreshVisibilitySystem();
    void OnFixVisibilityFrame();
    void OnReleaseVisibilityFrame();

    void OnEnableDisableShadows(bool enable);

    void EnableSounds(bool enable);

    void OnSelectMode();
    void OnMoveMode();
    void OnRotateMode();
    void OnScaleMode();
    void OnPivotCenterMode();
    void OnPivotCommonMode();
    void OnManualModifMode();
    void OnPlaceOnLandscape();
    void OnSnapToLandscape();
    void OnResetTransform();
    void OnLockTransform();
    void OnUnlockTransform();

    void OnCenterPivotPoint();
    void OnZeroPivotPoint();

    void OnMaterialEditor(DAVA::NMaterial* material = nullptr);
    void OnTextureBrowser();
    void OnSceneLightMode();

    void OnCubemapEditor();
    void OnImageSplitter();

    void OnAddLandscape();
    void OnAddVegetation();
    void OnLightDialog();
    void OnCameraDialog();
    void OnEmptyEntity();
    void OnAddWindEntity();
    void OnAddPathEntity();

    void OnUserNodeDialog();
    void OnSwitchEntityDialog();
    void OnParticleEffectDialog();
    void On2DCameraDialog();
    void On2DSpriteDialog();
    void OnAddEntityFromSceneTree();

    void OnOpenHelp();

    void OnSaveHeightmapToImage();
    void OnSaveTiledTexture();
    void OnTiledTextureRetreived(DAVA::Landscape* landscape, DAVA::Texture* landscapeTexture);

    void OnConvertModifiedTextures();

    void OnBeastAndSave();

    void OnBuildStaticOcclusion();
    void OnInavalidateStaticOcclusion();

    void OnLandscapeEditorToggled(SceneEditor2* scene);
    void OnForceFirstLod(bool);
    void OnCustomColorsEditor();
    void OnHeightmapEditor();
    void OnRulerTool();
    void OnTilemaskEditor();
    void OnNotPassableTerrain();
    void OnWayEditor();

    void OnObjectsTypeChanged(QAction* action);
    void OnObjectsTypeChanged(int type);

    void OnHangingObjects();
    void OnHangingObjectsHeight(double value);

    void OnMaterialLightViewChanged(bool);
    void OnCustomQuality();

    void OnSwitchWithDifferentLODs(bool checked);

    void OnGenerateHeightDelta();

    void OnBatchProcessScene();

    void OnSnapCameraToLandscape(bool);

    void SetupTitle(const DAVA::String& projectPath);

    bool SetVisibilityToolEnabledIfPossible(bool);
    void UpdateLandscapeRenderMode();

    void OnValidateScene();

protected:
    bool eventFilter(QObject* object, QEvent* event) override;
    void SetupWidget();
    void SetupMainMenu();
    void SetupThemeActions();
    void SetupToolBars();
    void SetupStatusBar();
    void SetupDocks();
    void SetupActions();

    void StartGlobalInvalidateTimer();

    void RunBeast(const QString& outputPath, BeastProxy::eBeastMode mode);

    void SynchronizeStateWithUI();

    static void SetActionCheckedSilently(QAction* action, bool checked);

private slots:
    void SceneCommandExecuted(SceneEditor2* scene, const RECommandNotificationObject& commandNotification);
    void SceneActivated(SceneEditor2* scene);
    void SceneDeactivated(SceneEditor2* scene);

    void OnGlobalInvalidateTimeout();
    void EditorLightEnabled(bool enabled);
    void OnSnapToLandscapeChanged(SceneEditor2* scene, bool isSpanToLandscape);
    void UnmodalDialogFinished(int);

    void DebugVersionInfo();
    void OnConsoleItemClicked(const QString& data);

    void UpdateUndoActionText(const DAVA::String& text);
    void UpdateRedoActionText(const DAVA::String& text);

private:
    std::unique_ptr<Ui::MainWindow> ui;
    QtWaitDialog* beastWaitDialog;
    QPointer<QDockWidget> dockActionEvent;
    QPointer<QDockWidget> dockConsole;

    bool globalInvalidate;

    ModificationWidget* modificationWidget;

    QComboBox* objectTypesWidget;

    AddSwitchEntityDialog* addSwitchEntityDialog;
    HangingObjectsHeight* hangingObjectsWidget;

    void EnableSceneActions(bool enable);
    void EnableProjectActions(bool enable);
    void UpdateModificationActionsState();
    void UpdateWayEditor(const RECommandNotificationObject& commandNotification);

    void LoadViewState(SceneEditor2* scene);
    void LoadModificationState(SceneEditor2* scene);
    void LoadEditorLightState(SceneEditor2* scene);
    void LoadLandscapeEditorState(SceneEditor2* scene);
    void LoadObjectTypes(SceneEditor2* scene);
    void LoadHangingObjects(SceneEditor2* scene);
    void LoadMaterialLightViewMode();

    // Landscape editor specific
    // TODO: remove later -->
    bool LoadAppropriateTextureFormat();
    // <--

    void OnDataChanged(const DAVA::TArc::DataWrapper& wrapper, const DAVA::Vector<DAVA::Any>& fields) override;

    //Need for any debug functionality
    QPointer<DeveloperTools> developerTools;
    QPointer<VersionInfoWidget> versionInfoWidget;

    std::shared_ptr<GlobalOperations> globalOperations;
    ErrorDialogOutput* errorLoggerOutput = nullptr;

#if defined(__DAVAENGINE_MACOS__)
    ShortcutChecker shortcutChecker;
#endif

    DAVA::TArc::UI* tarcUI = nullptr;
    std::unique_ptr<DAVA::TArc::WaitHandle> waitDialog;
    DAVA::TArc::DataWrapper projectDataWrapper;
    DAVA::TArc::DataWrapper selectionWrapper;
    QtDelayedExecutor delayedExecutor;
};

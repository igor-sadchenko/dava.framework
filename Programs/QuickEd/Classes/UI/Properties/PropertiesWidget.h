#pragma once

#include "ui_PropertiesWidget.h"
#include "EditorSystems/SelectionContainer.h"

#include <TArc/DataProcessing/DataWrapper.h>
#include <TArc/DataProcessing/DataListener.h>

#include <QtTools/Updaters/ContinuousUpdater.h>

#include <Base/BaseTypes.h>

#include <QDockWidget>

namespace DAVA
{
namespace TArc
{
class ContextAccessor;
class UI;
}
}

class Project;
class PackageNode;
class PackageBaseNode;
class PropertiesModel;
class PropertiesTreeItemDelegate;

class PropertiesWidget : public QDockWidget, public Ui::PropertiesWidget, private DAVA::TArc::DataListener
{
    Q_OBJECT
public:
    PropertiesWidget(QWidget* parent = nullptr);
    ~PropertiesWidget();

    void SetAccessor(DAVA::TArc::ContextAccessor* accessor);
    void SetUI(DAVA::TArc::UI* ui);

public slots:
    void SetProject(const Project* project);
    void UpdateModel(PackageBaseNode* node);

    void OnAddComponent(QAction* action);
    void OnAddStyleProperty(QAction* action);
    void OnAddStyleSelector();
    void OnRemove();

    void OnSelectionChanged(const QItemSelection& selected,
                            const QItemSelection& deselected);
    void OnModelUpdated();

private slots:
    void OnExpanded(const QModelIndex& index);
    void OnCollapsed(const QModelIndex& index);
    void OnComponentAdded(const QModelIndex& index);

private:
    QAction* CreateAddComponentAction();
    QAction* CreateAddStyleSelectorAction();
    QAction* CreateAddStylePropertyAction();
    QAction* CreateRemoveAction();
    QAction* CreateSeparator();

    void UpdateModelInternal();

    void UpdateActions();

    void ApplyExpanding();

    void OnDataChanged(const DAVA::TArc::DataWrapper& wrapper, const DAVA::Vector<DAVA::Any>& fields) override;

    QAction* addComponentAction = nullptr;
    QAction* addStylePropertyAction = nullptr;
    QAction* addStyleSelectorAction = nullptr;
    QAction* removeAction = nullptr;

    PropertiesModel* propertiesModel = nullptr;
    PropertiesTreeItemDelegate* propertiesItemsDelegate = nullptr;

    ContinuousUpdater nodeUpdater;

    DAVA::Map<DAVA::String, bool> itemsState;

    DAVA::String lastTopIndexPath;
    PackageBaseNode* selectedNode = nullptr; //node used to build model

    DAVA::TArc::DataWrapper documentDataWrapper;

    DAVA::TArc::ContextAccessor* accessor = nullptr;
    DAVA::TArc::UI* ui = nullptr;
};

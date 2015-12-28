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


#ifndef QUICKED_EDITOR_CORE_H
#define QUICKED_EDITOR_CORE_H

#include <QObject>
#include "UI/mainwindow.h"
#include "Project/Project.h"
#include "Base/BaseTypes.h"
#include "Base/Singleton.h"

class QAction;
class Document;
class DocumentGroup;
class Project;
class PackageNode;
class SpritesPacker;
class QFileSystemWatcher;

class EditorCore final : public QObject, public DAVA::Singleton<EditorCore>
{
    Q_OBJECT
public:
    explicit EditorCore(QObject *parent = nullptr);
    ~EditorCore();
    MainWindow* GetMainWindow() const;
    Project *GetProject() const;
    SpritesPacker* GetSpritesPacker() const;
    void Start();

private slots:
    void OnReloadSprites();
    void OnFilesChanged(const QStringList& changedFiles);
    void OnFilesRemoved(const QStringList& removedFiles);

    void OnOpenPackageFile(const QString &path);
    void OnProjectPathChanged(const QString &path);
    void OnGLWidgedInitialized();

    bool CloseAllDocuments();
    bool CloseOneDocument(int index);
    void SaveDocument(int index);
    void SaveAllDocuments();

    void Exit();
    void RecentMenu(QAction *);
    void OnCurrentTabChanged(int index);
    
    void UpdateLanguage();
   
    void OnRtlChanged(bool isRtl);
    void OnGlobalStyleClassesChanged(const QString &classesStr);

    void OnApplicationStateChanged(Qt::ApplicationState state);
    void OnFileChanged(const QString& path);

private:
    void ApplyFileChanges();
    Document* GetDocument(const QString& path) const;
    void OpenProject(const QString &path);
    bool CloseProject();
    int CreateDocument(int index, PackageNode* package);
    void SaveDocument(Document *document);

    void CloseDocument(int index);
    int GetIndexByPackagePath(const DAVA::FilePath& davaPath) const;

    std::unique_ptr<SpritesPacker> spritesPacker;
    Project* project = nullptr;
    QList<Document*> documents;
    DocumentGroup* documentGroup = nullptr;
    std::unique_ptr<MainWindow> mainWindow;
    DAVA::UIControl* rootControl = nullptr;
    QFileSystemWatcher* fileSystemWatcher = nullptr;
    QSet<QString> changedFiles;
};

inline EditorFontSystem *GetEditorFontSystem()
{
    return EditorCore::Instance()->GetProject()->GetEditorFontSystem();
}

#endif // QUICKED_EDITOR_CORE_H

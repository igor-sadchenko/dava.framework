#pragma once

#include <Tools/TextureCompression/TextureConverter.h>

#include "Base/BaseTypes.h"
#include "Render/RenderBase.h"
#include "FileSystem/FilePath.h"

#include <QObject>

namespace DAVA
{
class AssetCacheClient;
namespace TArc
{
class UI;
class WaitHandle;
}
}

class SpritesPacker;
class QAction;
class QDialog;
class GlobalOperations;

class SpritesPackerModule final : public QObject
{
    Q_OBJECT

public:
    SpritesPackerModule(DAVA::TArc::UI* ui);
    ~SpritesPackerModule() override;

    void RepackImmediately(const DAVA::FilePath& projectPath, DAVA::eGPUFamily gpu);
    void RepackWithDialog();

    bool IsRunning() const;

signals:
    void SpritesReloaded();

private:
    void SetupSpritesPacker(const DAVA::FilePath& projectPath);
    void ShowPackerDialog();
    void ReloadObjects();

    void ConnectCacheClient();
    void DisconnectCacheClient();
    void DisconnectCacheClientInternal(DAVA::AssetCacheClient* cacheClient);

    void ProcessSilentPacking(bool clearDirs, bool forceRepack, const DAVA::eGPUFamily gpu, const DAVA::TextureConverter::eConvertQuality quality);

    void CreateWaitDialog(const DAVA::FilePath& projectPath);
    void CloseWaitDialog();

    void SetCacheClientForPacker();

private:
    DAVA::AssetCacheClient* cacheClient = nullptr;

    std::unique_ptr<SpritesPacker> spritesPacker;
    QAction* reloadSpritesAction = nullptr;
    DAVA::TArc::UI* ui = nullptr;
    std::unique_ptr<DAVA::TArc::WaitHandle> waitDialogHandle;
};

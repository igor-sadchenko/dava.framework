#include "CommandLine/SceneExporterTool.h"
#include "CommandLine/Private/OptionName.h"
#include "CommandLine/Private/SceneConsoleHelper.h"

#include "Utils/SceneExporter/SceneExporter.h"
#include "TArc/Utils/ModuleCollection.h"

#include <Tools/AssetCache/AssetCache.h>
#include <Tools/TextureCompression/TextureConverter.h>

#include <Base/ScopedPtr.h>
#include <Logger/Logger.h>
#include <FileSystem/File.h>
#include <FileSystem/FileList.h>
#include <FileSystem/YamlNode.h>
#include <FileSystem/YamlParser.h>
#include <Platform/DeviceInfo.h>
#include <Render/GPUFamilyDescriptor.h>
#include <Render/Highlevel/Heightmap.h>
#include <Time/DateTime.h>
#include <Utils/UTF8Utils.h>

namespace SceneExporterToolDetail
{
void CollectObjectsFromFolder(const DAVA::FilePath& folderPathname, const DAVA::FilePath& inFolder, const SceneExporter::eExportedObjectType objectType, SceneExporter::ExportedObjectCollection& exportedObjects)
{
    using namespace DAVA;

    DVASSERT(folderPathname.IsDirectoryPathname());

    ScopedPtr<FileList> fileList(new FileList(folderPathname));
    for (int32 i = 0, count = fileList->GetCount(); i < count; ++i)
    {
        const FilePath& pathname = fileList->GetPathname(i);
        if (fileList->IsDirectory(i))
        {
            if (!fileList->IsNavigationDirectory(i))
            {
                CollectObjectsFromFolder(pathname, inFolder, objectType, exportedObjects);
            }
        }
        else if ((SceneExporter::OBJECT_SCENE == objectType) && (pathname.IsEqualToExtension(".sc2")))
        {
            String::size_type exportedPos = pathname.GetAbsolutePathname().find(".exported.sc2");
            if (exportedPos != String::npos)
            {
                Logger::Warning("[SceneExporterTool] Found temporary file: %s\nPlease delete it manualy", pathname.GetAbsolutePathname().c_str());
                continue;
            }

            exportedObjects.emplace_back(SceneExporter::OBJECT_SCENE, pathname.GetRelativePathname(inFolder));
        }
        else if ((SceneExporter::OBJECT_TEXTURE == objectType) && (pathname.IsEqualToExtension(".tex")))
        {
            exportedObjects.emplace_back(SceneExporter::OBJECT_TEXTURE, pathname.GetRelativePathname(inFolder));
        }
    }
}

SceneExporter::eExportedObjectType GetObjectType(const DAVA::FilePath& pathname)
{
    static const DAVA::Vector<std::pair<SceneExporter::eExportedObjectType, DAVA::String>> objectDefinition =
    {
      { SceneExporter::OBJECT_TEXTURE, ".tex" },
      { SceneExporter::OBJECT_SCENE, ".sc2" },
      { SceneExporter::OBJECT_HEIGHTMAP, DAVA::Heightmap::FileExtension() },
      { SceneExporter::OBJECT_EMITTER_CONFIG, ".yaml" },
    };

    DVASSERT(static_cast<DAVA::int32>(objectDefinition.size()) == SceneExporter::OBJECT_COUNT);

    for (const auto& def : objectDefinition)
    {
        if (pathname.IsEqualToExtension(def.second))
        {
            return def.first;
        }
    }

    return SceneExporter::OBJECT_NONE;
}

bool CollectObjectFromFileList(const DAVA::FilePath& fileListPath, const DAVA::FilePath& inFolder, SceneExporter::ExportedObjectCollection& exportedObjects)
{
    using namespace DAVA;

    ScopedPtr<File> fileWithLinks(File::Create(fileListPath, File::OPEN | File::READ));
    if (!fileWithLinks)
    {
        Logger::Error("[SceneExporterTool] cannot open file with links %s", fileListPath.GetAbsolutePathname().c_str());
        return false;
    }

    do
    {
        String link = fileWithLinks->ReadLine();
        if (link.empty())
        {
            Logger::Warning("[SceneExporterTool] found empty string in file %s", fileListPath.GetAbsolutePathname().c_str());
            break;
        }

        FilePath exportedPathname = inFolder + link;
        if (exportedPathname.IsDirectoryPathname())
        {
            CollectObjectsFromFolder(exportedPathname, inFolder, SceneExporter::OBJECT_SCENE, exportedObjects);
        }
        else
        {
            const SceneExporter::eExportedObjectType objType = GetObjectType(exportedPathname);
            if (objType != SceneExporter::OBJECT_NONE)
            {
                exportedObjects.emplace_back(objType, std::move(link));
            }
        }

    } while (!fileWithLinks->IsEof());

    return true;
}

bool ReadOutputsFromFile(const DAVA::FilePath& yamlConfig, DAVA::Vector<SceneExporter::Params::Output>& outputs)
{
    using namespace DAVA;

    ScopedPtr<YamlParser> parser(YamlParser::Create(yamlConfig));
    if (parser)
    {
        YamlNode* rootNode = parser->GetRootNode();
        if (rootNode != nullptr)
        {
            bool fileIsCorrect = true;

            const Vector<YamlNode*>& yamlNodes = rootNode->AsVector();
            for (YamlNode* propertyNode : yamlNodes)
            {
                bool outputIsCorrect = true;

                FilePath outDir;
                const YamlNode* outdirNode = propertyNode->Get("outdir");
                if (outdirNode != nullptr)
                {
                    outDir = outdirNode->AsString();
                    outDir.MakeDirectoryPathname();
                }
                else
                {
                    Logger::Error("[SceneExporterTool] Wrong file format: 'outdir' is missing");
                    outputIsCorrect = false;
                }

                Vector<eGPUFamily> requestedGPUs;
                const YamlNode* gpuListNode = propertyNode->Get("gpu");
                if (gpuListNode != nullptr)
                {
                    const Vector<YamlNode*>& gpuNodes = gpuListNode->AsVector();
                    for (const YamlNode* gpuNode : gpuNodes)
                    {
                        String gpuName = gpuNode->AsString();
                        eGPUFamily gpu = GPUFamilyDescriptor::GetGPUByName(gpuName);
                        if (gpu == eGPUFamily::GPU_INVALID)
                        {
                            Logger::Error("Wrong gpu name: %s", gpuName.c_str());
                            outputIsCorrect = false;
                        }
                        else
                        {
                            requestedGPUs.push_back(gpu);
                        }
                    }
                }
                else
                {
                    Logger::Error("[SceneExporterTool] Empty gpu list for %s", outDir.GetStringValue().c_str());
                    outputIsCorrect = false;
                }

                bool useHD = false;
                const YamlNode* hdNode = propertyNode->Get("useHD");
                if (hdNode != nullptr)
                {
                    useHD = hdNode->AsBool();
                }

                if (outputIsCorrect)
                {
                    outputs.emplace_back(outDir, requestedGPUs, DAVA::TextureConverter::ECQ_VERY_HIGH, useHD);
                }
                else
                {
                    fileIsCorrect = false;
                }
            }

            return fileIsCorrect;
        }
        else
        {
            Logger::Error("[SceneExporterTool] Wrong file format %s", yamlConfig.GetStringValue().c_str());
        }
    }
    else
    {
        Logger::Error("[SceneExporterTool] Cannot open %s", yamlConfig.GetStringValue().c_str());
    }

    return false;
}
}

SceneExporterTool::SceneExporterTool(const DAVA::Vector<DAVA::String>& commandLine)
    : CommandLineModule(commandLine, "-sceneexporter")
{
    using namespace DAVA;

    options.AddOption(OptionName::Scene, VariantType(false), "Target object is scene, so we need to export *.sc2 files from folder");
    options.AddOption(OptionName::Texture, VariantType(false), "Target object is texture, so we need to export *.tex files from folder");

    options.AddOption(OptionName::InDir, VariantType(String("")), "Absolute Path for Project/DataSource/3d/ folder");
    options.AddOption(OptionName::OutDir, VariantType(String("")), "Absolute Path for Project/Data/3d/ folder");
    options.AddOption(OptionName::Output, VariantType(String("")), "Absolute Path for config file (*.yaml) with output parameters");
    options.AddOption(OptionName::ProcessDir, VariantType(String("")), "Foldername from DataSource/3d/ for exporting");
    options.AddOption(OptionName::ProcessFile, VariantType(String("")), "Filename from DataSource/3d/ for exporting");
    options.AddOption(OptionName::ProcessFileList, VariantType(String("")), "Absolute Path to file with filenames for exporting");
    options.AddOption(OptionName::QualityConfig, VariantType(String("")), "Absolute Path for quality.yaml file");

    options.AddOption(OptionName::GPU, VariantType(String("origin")), "GPU family: PowerVR_iOS, PowerVR_Android, tegra, mali, adreno, origin, dx11. Can be multiple: -gpu mali,adreno,origin", true);

    options.AddOption(OptionName::SaveNormals, VariantType(false), "Disable removing of normals from vertexes");
    options.AddOption(OptionName::HDTextures, VariantType(false), "Use 0-mip level as texture.hd.ext");

    options.AddOption(OptionName::UseAssetCache, VariantType(useAssetCache), "Enables using AssetCache for scene");
    options.AddOption(OptionName::AssetCacheIP, VariantType(AssetCache::GetLocalHost()), "ip of adress of Asset Cache Server");
    options.AddOption(OptionName::AssetCachePort, VariantType(static_cast<uint32>(AssetCache::ASSET_SERVER_PORT)), "port of adress of Asset Cache Server");
    options.AddOption(OptionName::AssetCacheTimeout, VariantType(static_cast<uint32>(1)), "timeout for caching operations");
}

bool SceneExporterTool::PostInitInternal()
{
    using namespace DAVA;

    exportingParams.dataSourceFolder = options.GetOption(OptionName::InDir).AsString();
    if (exportingParams.dataSourceFolder.IsEmpty() == true)
    {
        Logger::Error("[SceneExporterTool] Input folder was not selected");
        return false;
    }
    exportingParams.dataSourceFolder.MakeDirectoryPathname();

    FilePath outputsFile = options.GetOption(OptionName::Output).AsString();
    if (outputsFile.IsEmpty() == false)
    { // new style of output params
        bool readOutputs = SceneExporterToolDetail::ReadOutputsFromFile(outputsFile, exportingParams.outputs);
        if (readOutputs == false)
        {
            return false;
        }
    }
    else // old style of output params
    {
        FilePath outFolder = options.GetOption(OptionName::OutDir).AsString();
        if (outFolder.IsEmpty() == true)
        {
            Logger::Error("[SceneExporterTool] Output folder or outputs file were not selected");
            return false;
        }
        outFolder.MakeDirectoryPathname();

        Vector<eGPUFamily> requestedGPUs;
        uint32 count = options.GetOptionValuesCount(OptionName::GPU);
        for (uint32 i = 0; i < count; ++i)
        {
            String gpuName = options.GetOption(OptionName::GPU, i).AsString();
            eGPUFamily gpu = GPUFamilyDescriptor::GetGPUByName(gpuName);
            if (gpu == eGPUFamily::GPU_INVALID)
            {
                Logger::Error("Wrong gpu name: %s", gpuName.c_str());
            }
            else
            {
                requestedGPUs.push_back(gpu);
            }
        }
        if (requestedGPUs.empty())
        {
            Logger::Error("[SceneExporterTool] Unsupported gpu parameter was selected");
            return false;
        }

        bool useHDTextures = options.GetOption(OptionName::HDTextures).AsBool();
        exportingParams.outputs.emplace_back(outFolder, requestedGPUs, DAVA::TextureConverter::ECQ_VERY_HIGH, useHDTextures);
    }

    filename = options.GetOption(OptionName::ProcessFile).AsString();
    foldername = options.GetOption(OptionName::ProcessDir).AsString();
    fileListPath = options.GetOption(OptionName::ProcessFileList).AsString();

    if (options.GetOption(OptionName::Texture).AsBool())
    {
        commandObject = SceneExporter::OBJECT_TEXTURE;
    }
    else if (options.GetOption(OptionName::Scene).AsBool())
    {
        commandObject = SceneExporter::OBJECT_SCENE;
    }

    const bool saveNormals = options.GetOption(OptionName::SaveNormals).AsBool();
    exportingParams.optimizeOnExport = !saveNormals;

    useAssetCache = options.GetOption(OptionName::UseAssetCache).AsBool();
    if (useAssetCache)
    {
        connectionsParams.ip = options.GetOption(OptionName::AssetCacheIP).AsString();
        connectionsParams.port = static_cast<uint16>(options.GetOption(OptionName::AssetCachePort).AsUInt32());
        connectionsParams.timeoutms = options.GetOption(OptionName::AssetCacheTimeout).AsUInt32() * 1000; //ms
    }

    if (filename.empty() == false)
    {
        commandAction = ACTION_EXPORT_FILE;
    }
    else if (foldername.empty() == false)
    {
        commandAction = ACTION_EXPORT_FOLDER;
    }
    else if (fileListPath.IsEmpty() == false)
    {
        commandAction = ACTION_EXPORT_FILELIST;
    }
    else
    {
        Logger::Error("[SceneExporterTool] Target for exporting was not selected");
        return false;
    }

    bool qualityInitialized = SceneConsoleHelper::InitializeQualitySystem(options, exportingParams.dataSourceFolder);
    if (!qualityInitialized)
    {
        DAVA::Logger::Error("Cannot create path to quality.yaml from %s", exportingParams.dataSourceFolder.GetStringValue().c_str());
        return false;
    }

    return true;
}

DAVA::TArc::ConsoleModule::eFrameResult SceneExporterTool::OnFrameInternal()
{
    DAVA::AssetCacheClient cacheClient;

    SceneExporter exporter;
    exporter.SetExportingParams(exportingParams);

    if (useAssetCache)
    {
        DAVA::AssetCache::Error connected = cacheClient.ConnectSynchronously(connectionsParams);
        if (connected == DAVA::AssetCache::Error::NO_ERRORS)
        {
            DAVA::String machineName = DAVA::UTF8Utils::EncodeToUTF8(DAVA::DeviceInfo::GetName());
            DAVA::DateTime timeNow = DAVA::DateTime::Now();
            DAVA::String timeString = DAVA::UTF8Utils::EncodeToUTF8(timeNow.GetLocalizedDate()) + "_" + DAVA::UTF8Utils::EncodeToUTF8(timeNow.GetLocalizedTime());

            exporter.SetCacheClient(&cacheClient, machineName, timeString, "Resource Editor. Export scene");
        }
        else
        {
            useAssetCache = false;
        }
    }

    if (commandAction == ACTION_EXPORT_FILE)
    {
        commandObject = SceneExporterToolDetail::GetObjectType(exportingParams.dataSourceFolder + filename);
        if (commandObject == SceneExporter::OBJECT_NONE)
        {
            DAVA::Logger::Error("[SceneExporterTool] found wrong filename %s", filename.c_str());
            return DAVA::TArc::ConsoleModule::eFrameResult::FINISHED;
        }
        exportedObjects.emplace_back(commandObject, std::move(filename));
    }
    else if (commandAction == ACTION_EXPORT_FOLDER)
    {
        DAVA::FilePath folderPathname(exportingParams.dataSourceFolder + foldername);
        folderPathname.MakeDirectoryPathname();

        SceneExporterToolDetail::CollectObjectsFromFolder(folderPathname, exportingParams.dataSourceFolder, commandObject, exportedObjects);
    }
    else if (commandAction == ACTION_EXPORT_FILELIST)
    {
        bool collected = SceneExporterToolDetail::CollectObjectFromFileList(fileListPath, exportingParams.dataSourceFolder, exportedObjects);
        if (!collected)
        {
            DAVA::Logger::Error("[SceneExporterTool] Can't collect links from file %s", fileListPath.GetStringValue().c_str());
            return DAVA::TArc::ConsoleModule::eFrameResult::FINISHED;
        }
    }

    exporter.ExportObjects(exportedObjects);

    if (useAssetCache)
    {
        cacheClient.Disconnect();
    }

    return DAVA::TArc::ConsoleModule::eFrameResult::FINISHED;
}

void SceneExporterTool::BeforeDestroyedInternal()
{
    SceneConsoleHelper::FlushRHI();
}

void SceneExporterTool::ShowHelpInternal()
{
    CommandLineModule::ShowHelpInternal();

    DAVA::Logger::Info("Examples:");
    DAVA::Logger::Info("\t-sceneexporter -indir /Users/SmokeTest/DataSource/3d/ -outdir /Users/SmokeTest/Data/3d/ -processfile Maps/scene.sc2 -gpu mali -qualitycfgpath Users/SmokeTest/Data/quality.yaml");
    DAVA::Logger::Info("\t-sceneexporter -indir /Users/SmokeTest/DataSource/3d/ -outdir /Users/SmokeTest/Data/3d/ -processfile Maps/image/texture.tex -gpu mali -qualitycfgpath Users/SmokeTest/Data/quality.yaml");
    DAVA::Logger::Info("\t-sceneexporter -indir /Users/SmokeTest/DataSource/3d/ -output /Users/config.yaml -processfile Maps/scene.sc2 -qualitycfgpath Users/SmokeTest/Data/quality.yaml");

    DAVA::Logger::Info("\t-sceneexporter -scene -indir /Users/SmokeTest/DataSource/3d/ -outdir /Users/SmokeTest/Data/3d/ -processdir Maps/ -gpu adreno");
    DAVA::Logger::Info("\t-sceneexporter -texture -indir /Users/SmokeTest/DataSource/3d/ -outdir /Users/SmokeTest/Data/3d/ -processdir Maps/ -gpu adreno");
    DAVA::Logger::Info("\t-sceneexporter -scene -indir /Users/SmokeTest/DataSource/3d/ -output /Users/config.yaml -processdir Maps/");

    DAVA::Logger::Info("\t-sceneexporter -scene -indir /Users/SmokeTest/DataSource/3d/ -outdir /Users/SmokeTest/Data/3d/ -processfilelist /Users/files.txt -gpu adreno");
    DAVA::Logger::Info("\t-sceneexporter -texture -indir /Users/SmokeTest/DataSource/3d/ -outdir /Users/SmokeTest/Data/3d/ -processfilelist /Users/files.txt -gpu adreno,PowerVR_iOS -useCache -ip 127.0.0.1");
    DAVA::Logger::Info("\t-sceneexporter -texture -indir /Users/SmokeTest/DataSource/3d/ -output /Users/config.yaml -processfilelist /Users/files.txt -useCache -ip 127.0.0.1");
}

DECL_CONSOLE_MODULE(SceneExporterTool, "-sceneexporter");

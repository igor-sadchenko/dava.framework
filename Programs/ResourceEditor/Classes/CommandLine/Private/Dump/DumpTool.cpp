#include "CommandLine/DumpTool.h"
#include "CommandLine/Private/OptionName.h"
#include "CommandLine/Private/SceneConsoleHelper.h"

#include "Logger/Logger.h"
#include "Utils/Utils.h"
#include "Utils/Dump/SceneDumper.h"

#include "TArc/Utils/ModuleCollection.h"

#include "Render/GPUFamilyDescriptor.h"
#include "FileSystem/FilePath.h"
#include "FileSystem/FileSystem.h"

DumpTool::DumpTool(const DAVA::Vector<DAVA::String>& commandLine)
    : CommandLineModule(commandLine, "-dump")
{
    using namespace DAVA;

    options.AddOption(OptionName::Links, VariantType(false), "Target for dumping is links");
    options.AddOption(OptionName::InDir, VariantType(String("")), "Path for Project/DataSource/3d/ folder");
    options.AddOption(OptionName::ProcessFile, VariantType(String("")), "Filename from DataSource/3d/ for dumping");
    options.AddOption(OptionName::QualityConfig, VariantType(String("")), "Full path for quality.yaml file");
    options.AddOption(OptionName::OutFile, VariantType(String("")), "Full path to file to write result of dumping");
    options.AddOption(OptionName::Mode, VariantType(String("e")), "Mode of dumping: r - required, e - extended. Extended mode is default.");
    options.AddOption(OptionName::GPU, VariantType(String("all")), "GPU family: PowerVR_iOS, PowerVR_Android, tegra, mali, adreno, origin, dx11. Can be multiple: -gpu mali,adreno,origin", true);
}

bool DumpTool::PostInitInternal()
{
    inFolder = options.GetOption(OptionName::InDir).AsString();
    if (inFolder.IsEmpty())
    {
        DAVA::Logger::Error("Input folder was not selected");
        return false;
    }
    inFolder.MakeDirectoryPathname();

    filename = options.GetOption(OptionName::ProcessFile).AsString();
    if (filename.empty())
    {
        DAVA::Logger::Error("Filename was not selected");
        return false;
    }

    outFile = options.GetOption(OptionName::OutFile).AsString();
    if (outFile.IsEmpty())
    {
        DAVA::Logger::Error("Out file was not selected");
        return false;
    }

    DAVA::String modeString = options.GetOption(OptionName::Mode).AsString();
    if (modeString == "r")
    {
        mode = SceneDumper::eMode::REQUIRED;
    }
    else
    { // now we use extended mode in case of empty string or in case of error
        mode = SceneDumper::eMode::EXTENDED;
    }

    DAVA::uint32 count = options.GetOptionValuesCount(OptionName::GPU);
    compressedGPUs.reserve(count);
    for (DAVA::uint32 i = 0; i < count; ++i)
    {
        DAVA::String gpuName = options.GetOption(OptionName::GPU, i).AsString();
        if (gpuName == "all")
        {
            compressedGPUs.clear();
            compressedGPUs.reserve(DAVA::eGPUFamily::GPU_DEVICE_COUNT);
            for (DAVA::int32 gpu = 0; gpu < DAVA::eGPUFamily::GPU_DEVICE_COUNT; ++gpu)
            {
                compressedGPUs.push_back(static_cast<DAVA::eGPUFamily>(gpu));
            }
            break;
        }
        else
        {
            DAVA::eGPUFamily gpu = DAVA::GPUFamilyDescriptor::GetGPUByName(gpuName);
            if (gpu == DAVA::eGPUFamily::GPU_INVALID)
            {
                DAVA::Logger::Error("Wrong gpu name: %s", gpuName.c_str());
            }
            else
            {
                compressedGPUs.push_back(gpu);
            }
        }
    }

    if (compressedGPUs.empty())
    {
        DAVA::Logger::Error("GPU was not selected");
        return false;
    }

    DAVA::FindAndRemoveExchangingWithLast(compressedGPUs, DAVA::eGPUFamily::GPU_ORIGIN);

    bool qualityInitialized = SceneConsoleHelper::InitializeQualitySystem(options, inFolder + filename);
    if (!qualityInitialized)
    {
        DAVA::Logger::Error("Cannot create path to quality.yaml from %s", inFolder.GetAbsolutePathname().c_str());
        return false;
    }

    if (options.GetOption(OptionName::Links).AsBool())
    {
        commandAction = ACTION_DUMP_LINKS;
    }
    else
    {
        DAVA::Logger::Error("Target for dumping was not selected");
        return false;
    }

    return true;
}

DAVA::TArc::ConsoleModule::eFrameResult DumpTool::OnFrameInternal()
{
    if (commandAction == ACTION_DUMP_LINKS)
    {
        DAVA::Set<DAVA::FilePath> links = SceneDumper::DumpLinks(inFolder + filename, mode, compressedGPUs);

        DAVA::FileSystem::Instance()->CreateDirectory(outFile.GetDirectory(), true);
        DAVA::ScopedPtr<DAVA::File> file(DAVA::File::Create(outFile, DAVA::File::WRITE | DAVA::File::CREATE));
        if (file)
        {
            for (const DAVA::FilePath& link : links)
            {
                if (!link.IsEmpty() && link.GetType() != DAVA::FilePath::PATH_IN_MEMORY)
                {
                    file->WriteLine(link.GetAbsolutePathname());
                }
            }
        }
    }

    return DAVA::TArc::ConsoleModule::eFrameResult::FINISHED;
}

void DumpTool::BeforeDestroyedInternal()
{
    SceneConsoleHelper::FlushRHI();
}

void DumpTool::ShowHelpInternal()
{
    CommandLineModule::ShowHelpInternal();

    DAVA::Logger::Info("Examples:");
    DAVA::Logger::Info("\t-dump -indir /Users/SmokeTest/DataSource/3d/ -processfile Maps/11-grass/test_scene.sc2 -outfile /Users/Test/dump.txt -links -mode e -gpu all");
    DAVA::Logger::Info("\t-dump -indir /Users/SmokeTest/DataSource/3d/ -processfile Maps/11-grass/test_scene.sc2 -outfile /Users/Test/dump.txt -links -mode r -gpu mali,adreno");
}

DECL_CONSOLE_MODULE(DumpTool, "-dump");
#include "TextDrawSystem.h"

// framework
#include "Render/RenderHelper.h"
#include "Render/DynamicBufferAllocator.h"
#include "Render/2D/Systems/RenderSystem2D.h"
#include "Render/2D/TextBlockGraphicRender.h"
#include "Utils/Utils.h"

using namespace DAVA;

TextDrawSystem::TextDrawSystem(Scene* scene, SceneCameraSystem* _cameraSystem)
    : SceneSystem(scene)
    , cameraSystem(_cameraSystem)
{
    FilePath fntPath = FilePath("~res:/ResourceEditor/Fonts/korinna_df.fnt");
    FilePath texPath = FilePath("~res:/ResourceEditor/Fonts/korinna_df.tex");
    font = GraphicFont::Create(fntPath, texPath);
    if (nullptr == font)
        return;

    if (font->GetFontType() == Font::TYPE_DISTANCE)
    {
        float32 cachedSpread = font->GetSpread();
        fontMaterial = new NMaterial();
        fontMaterial->SetFXName(FastName("~res:/Materials/2d.DistanceFont.material"));
        fontMaterial->SetMaterialName(FastName("DistanceFontMaterial"));
        fontMaterial->AddProperty(FastName("smoothing"), &cachedSpread, rhi::ShaderProp::TYPE_FLOAT1);
        fontMaterial->PreBuildMaterial(RenderSystem2D::RENDER_PASS_NAME);
    }
    else
    {
        fontMaterial = SafeRetain(RenderSystem2D::DEFAULT_2D_TEXTURE_MATERIAL);
    }
}

TextDrawSystem::~TextDrawSystem()
{
    SafeRelease(fontMaterial);
    SafeRelease(font);
}

Vector2 TextDrawSystem::ToPos2d(const Vector3& pos3d) const
{
    Vector3 pos2ddepth = cameraSystem->GetScreenPosAndDepth(pos3d);
    return (pos2ddepth.z >= 0.0f) ? Vector2(pos2ddepth.x, pos2ddepth.y) : Vector2(-1.0f, -1.0f);
}

void TextDrawSystem::Draw()
{
    bool shouldDrawAnything = !textToDraw.empty() && (nullptr != font);

    if (shouldDrawAnything)
    {
        for (const auto& textToDraw : textToDraw)
        {
            WideString wStr = UTF8Utils::EncodeToWideString(textToDraw.text);
            vertices.resize(4 * wStr.length());

            float32 x = textToDraw.pos.x;
            float32 y = textToDraw.pos.y;
            AdjustPositionBasedOnAlign(x, y, font->GetStringSize(wStr), textToDraw.align);

            int32 charactersDrawn = 0;
            font->DrawStringToBuffer(wStr, static_cast<int>(x), static_cast<int>(y), vertices.data(), charactersDrawn);

            PushNextBatch(textToDraw.color);
        }
    }

    textToDraw.clear();
}

void TextDrawSystem::PushNextBatch(const Color& color)
{
    uint32 vertexCount = static_cast<uint32>(vertices.size());
    uint32 indexCount = 6 * vertexCount / 4;

    RenderSystem2D::BatchDescriptor batchDescriptor;
    batchDescriptor.singleColor = color;
    batchDescriptor.vertexCount = vertexCount;
    batchDescriptor.indexCount = DAVA::Min(TextBlockGraphicRender::GetSharedIndexBufferCapacity(), indexCount);
    batchDescriptor.vertexPointer = vertices.front().position.data;
    batchDescriptor.vertexStride = TextBlockGraphicRender::TextVerticesDefaultStride;
    batchDescriptor.texCoordPointer[0] = vertices.front().texCoord.data;
    batchDescriptor.texCoordStride = TextBlockGraphicRender::TextVerticesDefaultStride;
    batchDescriptor.indexPointer = TextBlockGraphicRender::GetSharedIndexBuffer();
    batchDescriptor.material = fontMaterial;
    batchDescriptor.textureSetHandle = font->GetTexture()->singleTextureSet;
    batchDescriptor.samplerStateHandle = font->GetTexture()->samplerStateHandle;
    batchDescriptor.worldMatrix = &Matrix4::IDENTITY;
    RenderSystem2D::Instance()->PushBatch(batchDescriptor);
}

void TextDrawSystem::DrawText(int32 x, int32 y, const String& text, const Color& color, Align align)
{
    DrawText(Vector2((float32)x, (float32)y), text, color);
}

void TextDrawSystem::DrawText(const Vector2& pos2d, const String& text, const Color& color, Align align)
{
    if ((pos2d.x >= 0.0f) && (pos2d.y >= 0.0f))
        textToDraw.emplace_back(pos2d, text, color, align);
}

void TextDrawSystem::AdjustPositionBasedOnAlign(float32& x, float32& y, const Size2i& sSize, Align align)
{
    switch (align)
    {
    case Align::TopLeft:
        break;

    case Align::TopCenter:
        x -= sSize.dx / 2;
        break;

    case Align::TopRight:
        x -= sSize.dx;
        break;

    case Align::Left:
        y -= sSize.dy / 2;
        break;

    case Align::Center:
        x -= sSize.dx / 2;
        y -= sSize.dy / 2;
        break;

    case Align::Right:
        x -= sSize.dx;
        y -= sSize.dy / 2;
        break;

    case Align::BottomLeft:
        y -= sSize.dy;
        break;

    case Align::BottomCenter:
        x -= sSize.dx / 2;
        y -= sSize.dy;
        break;

    case Align::BottomRight:
        x -= sSize.dx;
        y -= sSize.dy;
        break;

    default:
        break;
    }
}

#pragma once

#include "Base/BaseTypes.h"
#include "Entity/Component.h"
#include "Debug/DVAssert.h"
#include "Base/Introspection.h"
#include "Scene3D/SceneFile/SerializationContext.h"
#include "Scene3D/Systems/EventSystem.h"
#include "Scene3D/Systems/GlobalEventSystem.h"
#include "Base/BaseMath.h"
#include "Render/Highlevel/StaticOcclusion.h"
#include "Reflection/Reflection.h"

namespace DAVA
{
class StaticOcclusionDataComponent : public Component
{
protected:
    ~StaticOcclusionDataComponent();

public:
    IMPLEMENT_COMPONENT_TYPE(STATIC_OCCLUSION_DATA_COMPONENT);

    StaticOcclusionDataComponent();

    Component* Clone(Entity* toEntity) override;
    void Serialize(KeyedArchive* archive, SerializationContext* serializationContext) override;
    void Deserialize(KeyedArchive* archive, SerializationContext* serializationContext) override;

    inline StaticOcclusionData& GetData();

    inline uint32 GetDataSize() const;
    inline void SetDataSize(uint32 bytes);

private:
    StaticOcclusionData data;

public:
    INTROSPECTION_EXTEND(StaticOcclusionDataComponent, Component,
                         PROPERTY("Size in kBytes", "Size of occlusion information in kBytes", GetDataSize, SetDataSize, I_VIEW | I_EDIT)
                         );

    DAVA_VIRTUAL_REFLECTION(StaticOcclusionDataComponent, Component);
};

class StaticOcclusionComponent : public Component
{
protected:
    ~StaticOcclusionComponent(){};

public:
    IMPLEMENT_COMPONENT_TYPE(STATIC_OCCLUSION_COMPONENT);

    StaticOcclusionComponent();

    Component* Clone(Entity* toEntity) override;
    void Serialize(KeyedArchive* archive, SerializationContext* serializationContext) override;
    void Deserialize(KeyedArchive* archive, SerializationContext* serializationContext) override;

    inline void SetBoundingBox(const AABBox3& newBBox);
    inline const AABBox3& GetBoundingBox() const;

    inline void SetSubdivisionsX(uint32 _sizeX);
    inline void SetSubdivisionsY(uint32 _sizeY);
    inline void SetSubdivisionsZ(uint32 _sizeZ);
    inline void SetPlaceOnLandscape(bool place);
    inline void SetOcclusionPixelThreshold(uint32 pixelThreshold);
    inline void SetOcclusionPixelThresholdForSpeedtree(uint32 pixelThreshold);

    inline uint32 GetSubdivisionsX() const;
    inline uint32 GetSubdivisionsY() const;
    inline uint32 GetSubdivisionsZ() const;
    inline bool GetPlaceOnLandscape() const;
    inline uint32 GetOcclusionPixelThreshold() const;
    inline uint32 GetOcclusionPixelThresholdForSpeedtree() const;
    inline const float32* GetCellHeightOffsets() const;

    //Vector<Vector3> renderPositions;

private:
    AABBox3 boundingBox;
    uint32 xSubdivisions;
    uint32 ySubdivisions;
    uint32 zSubdivisions;
    uint32 occlusionPixelThreshold;
    uint32 occlusionPixelThresholdForSpeedtree;
    bool placeOnLandscape;
    Vector<float32> cellHeightOffset; //x*y

    friend class StaticOcclusionBuildSystem;

public:
    INTROSPECTION_EXTEND(StaticOcclusionComponent, Component,
                         PROPERTY("Bounding box", "Bounding box of occlusion zone", GetBoundingBox, SetBoundingBox, I_VIEW | I_EDIT)
                         PROPERTY("Subdivisions X", "Number of subdivisions on X axis", GetSubdivisionsX, SetSubdivisionsX, I_VIEW | I_EDIT)
                         PROPERTY("Subdivisions Y", "Number of subdivisions on Y axis", GetSubdivisionsY, SetSubdivisionsY, I_VIEW | I_EDIT)
                         PROPERTY("Subdivisions Z", "Number of subdivisions on Z axis", GetSubdivisionsZ, SetSubdivisionsZ, I_VIEW | I_EDIT)
                         PROPERTY("Place on Landscape", "Place lowest occlusion cubes at landscape height", GetPlaceOnLandscape, SetPlaceOnLandscape, I_VIEW | I_EDIT)
                         PROPERTY("Occlusion Pixel Threshold", "Occlusion Pixel Threshold", GetOcclusionPixelThreshold, SetOcclusionPixelThreshold, I_VIEW | I_EDIT)
                         PROPERTY("Occlusion Pixel Threshold For Speedtree", "Occlusion Pixel Threshold For Speedtree", GetOcclusionPixelThresholdForSpeedtree, SetOcclusionPixelThresholdForSpeedtree, I_VIEW | I_EDIT)
                         );

    DAVA_VIRTUAL_REFLECTION(StaticOcclusionComponent, Component);
};

class StaticOcclusionDebugDrawComponent : public Component
{
    friend class StaticOcclusionDebugDrawSystem;

public:
    IMPLEMENT_COMPONENT_TYPE(STATIC_OCCLUSION_DEBUG_DRAW_COMPONENT);

    StaticOcclusionDebugDrawComponent(RenderObject* object = NULL);

    Component* Clone(Entity* toEntity) override;
    void Serialize(KeyedArchive* archive, SerializationContext* serializationContext) override;
    void Deserialize(KeyedArchive* archive, SerializationContext* serializationContext) override;

    RenderObject* GetRenderObject() const;

protected:
    ~StaticOcclusionDebugDrawComponent();

private:
    RenderObject* renderObject;
    rhi::HVertexBuffer vertices;
    rhi::HIndexBuffer gridIndices, coverIndices;
    uint32 vertexCount, gridIndexCount, coverIndexCount;
    AABBox3 bbox;

public:
    INTROSPECTION_EXTEND(StaticOcclusionDebugDrawComponent, Component,
                         NULL
                         );

    DAVA_VIRTUAL_REFLECTION(StaticOcclusionDebugDrawComponent, Component);
};

//

inline uint32 StaticOcclusionDataComponent::GetDataSize() const
{
    return (data.blockCount * data.objectCount / 32 * 4) / 1024;
};

inline void StaticOcclusionDataComponent::SetDataSize(uint32 bytes)
{

};

//

inline void StaticOcclusionComponent::SetBoundingBox(const AABBox3& newBBox)
{
    boundingBox = newBBox;
    GlobalEventSystem::Instance()->Event(this, EventSystem::STATIC_OCCLUSION_COMPONENT_CHANGED);
}

inline const AABBox3& StaticOcclusionComponent::GetBoundingBox() const
{
    return boundingBox;
}

inline void StaticOcclusionComponent::SetSubdivisionsX(uint32 _sizeX)
{
    xSubdivisions = _sizeX;
    GlobalEventSystem::Instance()->Event(this, EventSystem::STATIC_OCCLUSION_COMPONENT_CHANGED);
}

inline void StaticOcclusionComponent::SetSubdivisionsY(uint32 _sizeY)
{
    ySubdivisions = _sizeY;
    GlobalEventSystem::Instance()->Event(this, EventSystem::STATIC_OCCLUSION_COMPONENT_CHANGED);
}

inline void StaticOcclusionComponent::SetSubdivisionsZ(uint32 _sizeZ)
{
    zSubdivisions = _sizeZ;
    GlobalEventSystem::Instance()->Event(this, EventSystem::STATIC_OCCLUSION_COMPONENT_CHANGED);
}

inline void StaticOcclusionComponent::SetPlaceOnLandscape(bool place)
{
    placeOnLandscape = place;
    GlobalEventSystem::Instance()->Event(this, EventSystem::STATIC_OCCLUSION_COMPONENT_CHANGED);
}

inline uint32 StaticOcclusionComponent::GetSubdivisionsX() const
{
    return xSubdivisions;
}

inline uint32 StaticOcclusionComponent::GetSubdivisionsY() const
{
    return ySubdivisions;
}

inline uint32 StaticOcclusionComponent::GetSubdivisionsZ() const
{
    return zSubdivisions;
}

inline bool StaticOcclusionComponent::GetPlaceOnLandscape() const
{
    return placeOnLandscape;
}

inline StaticOcclusionData& StaticOcclusionDataComponent::GetData()
{
    return data;
}

inline const float32* StaticOcclusionComponent::GetCellHeightOffsets() const
{
    return placeOnLandscape ? &cellHeightOffset.front() : NULL;
}

inline void StaticOcclusionComponent::SetOcclusionPixelThreshold(uint32 pixelThreshold)
{
    occlusionPixelThreshold = pixelThreshold;
}

inline void StaticOcclusionComponent::SetOcclusionPixelThresholdForSpeedtree(uint32 pixelThreshold)
{
    occlusionPixelThresholdForSpeedtree = pixelThreshold;
}

inline uint32 StaticOcclusionComponent::GetOcclusionPixelThreshold() const
{
    return occlusionPixelThreshold;
}

inline uint32 StaticOcclusionComponent::GetOcclusionPixelThresholdForSpeedtree() const
{
    return occlusionPixelThresholdForSpeedtree;
}
}

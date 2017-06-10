#ifndef __DAVAENGINE_BASE_FAMILY_H__
#define __DAVAENGINE_BASE_FAMILY_H__

#include "Base/BaseTypes.h"
#include "Debug/DVAssert.h"

namespace DAVA
{
////////////////////////////////////////////////////////////////////////////////
// BaseFamily
////////////////////////////////////////////////////////////////////////////////

template <typename Component>
class BaseFamily
{
public:
    BaseFamily(const Vector<Component*>& components);

    uint32 GetComponentIndex(uint32 componentType, uint32 index) const;
    uint32 GetComponentsCount(uint32 componentType) const;
    uint64 GetComponentsFlags() const;

    bool operator==(const BaseFamily<Component>& rhs) const;

private:
    uint32 componentIndices[Component::COMPONENT_COUNT];
    uint32 componentCount[Component::COMPONENT_COUNT];
    uint64 componentsFlags;
    int32 refCount;

    template <typename EntityFamilyType>
    friend class BaseFamilyRepository;
};

////////////////////////////////////////////////////////////////////////////////
// BaseFamilyRepository
////////////////////////////////////////////////////////////////////////////////

template <typename EntityFamilyType>
class BaseFamilyRepository
{
public:
    BaseFamilyRepository() = default;
    ~BaseFamilyRepository();

    EntityFamilyType* GetOrCreate(const EntityFamilyType& localFamily);
    void ReleaseFamily(EntityFamilyType* family);

    void ReleaseAllFamilies();

private:
    Vector<EntityFamilyType*> families;
    int32 refCount = 0;
};

////////////////////////////////////////////////////////////////////////////////
// BaseFamily Impl
////////////////////////////////////////////////////////////////////////////////

template <typename Component>
BaseFamily<Component>::BaseFamily(const Vector<Component*>& components)
    : componentsFlags(0)
    , refCount(0)
{
    Memset(componentIndices, 0, sizeof(componentIndices));
    Memset(componentCount, 0, sizeof(componentCount));

    int32 size = static_cast<int32>(components.size());
    for (int32 i = size - 1; i >= 0; --i)
    {
        uint32 type = components[i]->GetType();
        componentIndices[type] = i;
        componentCount[type]++;
        componentsFlags |= 1ULL << type;
    }
}

template <typename Component>
inline uint32 BaseFamily<Component>::GetComponentIndex(uint32 componentType, uint32 index) const
{
    return componentIndices[componentType] + index;
}

template <typename Component>
inline uint32 BaseFamily<Component>::GetComponentsCount(uint32 componentType) const
{
    return componentCount[componentType];
}

template <typename Component>
inline uint64 BaseFamily<Component>::GetComponentsFlags() const
{
    return componentsFlags;
}

template <typename Component>
inline bool BaseFamily<Component>::operator==(const BaseFamily<Component>& rhs) const
{
    return (componentsFlags == rhs.componentsFlags) && (0 == Memcmp(componentCount, rhs.componentCount, sizeof(componentCount)));
}

////////////////////////////////////////////////////////////////////////////////
// BaseFamilyRepository Impl
////////////////////////////////////////////////////////////////////////////////

template <typename EntityFamilyType>
BaseFamilyRepository<EntityFamilyType>::~BaseFamilyRepository()
{
    // DVASSERT(refCount == 0);
    ReleaseAllFamilies();
}

template <typename EntityFamilyType>
EntityFamilyType* BaseFamilyRepository<EntityFamilyType>::GetOrCreate(const EntityFamilyType& localFamily)
{
    // Check whether family already is in cache
    auto iter = std::find_if(families.begin(), families.end(), [&localFamily](const EntityFamilyType* o) -> bool { return *o == localFamily; });
    if (iter == families.end())
    { // Family not found in cache so add it
        families.push_back(new EntityFamilyType(localFamily));
        iter = families.end() - 1;
    }
    (*iter)->refCount += 1; // Increase family ref counter
    refCount += 1; // Increase repository ref counter
    return *iter;
}

template <typename EntityFamilyType>
void BaseFamilyRepository<EntityFamilyType>::ReleaseFamily(EntityFamilyType* family)
{
    if (family != nullptr)
    {
        DVASSERT(refCount > 0);
        DVASSERT(family->refCount > 0);

        family->refCount--;
        refCount--;
        if (0 == refCount)
        {
            for (auto x : families)
            {
                DVASSERT(x->refCount == 0);
                delete x;
            }
            families.clear();
        }
    }
}

template <typename EntityFamilyType>
void BaseFamilyRepository<EntityFamilyType>::ReleaseAllFamilies()
{
    // Release all families from cache and force repository's ref counter to 0
    refCount = 0;
    for (auto x : families)
    {
        delete x;
    }
    families.clear();
}
}

#endif //__DAVAENGINE_BASE_FAMILY_H__

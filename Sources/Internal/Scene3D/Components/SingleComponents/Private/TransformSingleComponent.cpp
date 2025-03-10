#include "Scene3D/Components/SingleComponents/TransformSingleComponent.h"
#include "Utils/Utils.h"

namespace DAVA
{
namespace TSCDetails
{
void RemoveEntitiesFromVector(Vector<Entity*>& vector, const Entity* entity)
{
    size_t size = vector.size();
    for (size_t k = 0; k < size; ++k)
    {
        if (vector[k] == entity)
        {
            vector[k] = vector[size - 1];
            vector.pop_back();
            size--;
        }
    }
}
}

void TransformSingleComponent::Clear()
{
    localTransformChanged.clear();
    transformParentChanged.clear();
    worldTransformChanged.Clear();
    animationTransformChanged.clear();
}

void TransformSingleComponent::EraseEntity(const Entity* entity)
{
    TSCDetails::RemoveEntitiesFromVector(localTransformChanged, entity);
    TSCDetails::RemoveEntitiesFromVector(transformParentChanged, entity);
    TSCDetails::RemoveEntitiesFromVector(animationTransformChanged, entity);
    worldTransformChanged.EraseEntity(entity);
}
}
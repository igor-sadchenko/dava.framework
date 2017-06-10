#ifndef __DAVAENGINE_PARTICLE_EMITTER_H__
#define __DAVAENGINE_PARTICLE_EMITTER_H__

#include "Base/BaseTypes.h"
#include "Base/BaseMath.h"
#include "Base/BaseObject.h"
#include "Base/DynamicObjectCache.h"
#include "Base/RefPtr.h"
#include "Particles/ParticlePropertyLine.h"
#include "Animation/Animation.h"
#include "Render/Highlevel/RenderObject.h"
#include "Render/Highlevel/IRenderUpdatable.h"
#include "Particles/ParticleLayer.h"
#include "FileSystem/FilePath.h"
#include "Scene3D/SceneFile/SerializationContext.h"

namespace DAVA
{
/*this class is proxy to load old effect hierarchy
  */
class PartilceEmitterLoadProxy : public RenderObject
{
public:
    PartilceEmitterLoadProxy();
    String emitterFilename;
    void Load(KeyedArchive* archive, SerializationContext* serializationContext);
};

class ParticleEmitter : public BaseObject
{
public:
    static bool FORCE_DEEP_CLONE;
    static const float32 PARTICLE_EMITTER_DEFAULT_LIFE_TIME;
    static ParticleEmitter* LoadEmitter(const FilePath& filename);

    enum eType : uint32
    {
        EMITTER_POINT,
        EMITTER_RECT,
        EMITTER_ONCIRCLE_VOLUME,
        EMITTER_ONCIRCLE_EDGES,
        EMITTER_SHOCKWAVE
    };

    enum eState : uint32
    {
        STATE_PLAYING,
        STATE_STOPPING, //emitter is stopping - no new particle generation, still need to update and recalculate
        STATE_STOPPED //emitter is completely stopped - no processing at all
    };

    ParticleEmitter();
    ParticleEmitter* Clone();

    bool LoadFromYaml(const FilePath& pathName, bool preserveInheritPosition = false);
    void SaveToYaml(const FilePath& pathName);

    void AddLayer(ParticleLayer* layer);
    ParticleLayer* GetNextLayer(ParticleLayer* layer);
    virtual void InsertLayer(ParticleLayer* layer, ParticleLayer* beforeLayer);
    void RemoveLayer(ParticleLayer* layer);
    void RemoveLayer(int32 index);
    void MoveLayer(ParticleLayer* layer, ParticleLayer* beforeLayer);
    bool ContainsLayer(ParticleLayer* layer);

    void UpdateEmptyLayerNames();
    void UpdateLayerNameIfEmpty(ParticleLayer* layer, int32 index);
    void LoadParticleLayerFromYaml(const YamlNode* yamlNode, bool preserveInheritPosition);

    // Invert the emission vector coordinates for backward compatibility.
    void InvertEmissionVectorCoordinates();

    String GetEmitterTypeName();

    void GetModifableLines(List<ModifiablePropertyLineBase*>& modifiables);

    void Cleanup(bool needCleanupLayers = true);
    void CleanupLayers();

public:
    FastName name;
    FilePath configPath;
    eType emitterType = EMITTER_POINT;
    float32 lifeTime = PARTICLE_EMITTER_DEFAULT_LIFE_TIME;

    Vector<ParticleLayer*> layers;

    RefPtr<PropertyLine<Vector3>> size;
    RefPtr<PropertyLine<Vector3>> emissionVector;
    RefPtr<PropertyLine<float32>> emissionRange;
    RefPtr<PropertyLine<float32>> radius;
    RefPtr<PropertyLine<float32>> emissionAngle;
    RefPtr<PropertyLine<float32>> emissionAngleVariation;
    RefPtr<PropertyLine<Color>> colorOverLife;

    bool shortEffect = false;

protected:
    virtual ~ParticleEmitter();

private:
    bool requireDeepClone = true;

#if defined(USE_FILEPATH_IN_MAP)
    using EmitterCacheMap = Map<FilePath, ParticleEmitter*>;
#else
    using EmitterCacheMap = Map<String, ParticleEmitter*>;
#endif

    void ReleaseFromCache(const FilePath& name);
    static EmitterCacheMap emitterCache;

public:
    INTROSPECTION_EXTEND(ParticleEmitter, BaseObject, nullptr);
};
}

#endif // __DAVAENGINE_PARTICLE_EMITTER_H__

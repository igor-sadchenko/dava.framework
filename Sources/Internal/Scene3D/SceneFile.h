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


#ifndef __DAVAENGINE_SCENEFILE_H__
#define __DAVAENGINE_SCENEFILE_H__

#include "Base/BaseObject.h"
#include "Base/BaseMath.h"
#include "Scene3D/Scene.h"
#include "Render/3D/StaticMesh.h"
#include "Render/3D/PolygonGroup.h"
#include "Utils/Utils.h"
#include "FileSystem/File.h"
#include "FileSystem/FilePath.h"

namespace DAVA
{
/*
	Full description of scene file format (.sc)

	[Header]

	[TextureData]
	[MaterialData]
	[StaticMeshData]
	[LightData]
	[CameraData]
 
	[SceneGraphStructure]
*/
class SkeletonNode;
class SceneFile : public BaseObject
{
protected:
	~SceneFile(){}
public:
	SceneFile();
	
	bool LoadScene(const FilePath & filename, Scene * _scene, bool relToBundle = true);
	bool SaveScene(const FilePath & filename);
	
	bool ReadTexture();
	bool ReadMaterial();
	bool ReadStaticMesh();
	bool ReadAnimatedMesh();
	bool ReadSceneNode(Entity * parentNode, int level);
	
	bool ReadCamera();
	bool ReadAnimation();
	bool ReadLight();

	bool ReadSceneGraph();
	
	void SetDebugLog(bool debugLogEnabled);
	
	File *		sceneFP;
	Scene *		scene;
	SkeletonNode * currentSkeletonNode;
	bool		debugLogEnabled;


	struct Header
	{
		Header();

		char8	descriptor[4];			// DVSC
		uint32	version;				
		uint32	materialCount;
		uint32	lightCount;
		uint32	cameraCount;

		uint32	staticMeshCount;
		uint32	animatedMeshCount;

		uint32	nodeAnimationsCount;
	};
    
    Header  header;

	class ObjectDef
	{
	public:
		char8	name[512];		// for convinient search
	};

	class TextureDef : public ObjectDef
	{
	public:
		uint8   hasOpacity;
	};

	class MaterialDef : public ObjectDef
	{
	public:
        MaterialDef()
        {
            diffuseTexture[0] = 0;
            lightmapTexture[0] = 0;
            reflectiveTexture[0] = 0;
            specularTexture[0] = 0;
            normalMapTexture[0] = 0;
        }
        
		Vector4 ambient;
		Vector4 diffuse;
		Vector4 specular;
		Vector4 emission;
		float32	shininess;

		Vector4 reflective;
		float32	reflectivity;

		Vector4 transparent;
		float32	transparency; 
		float32	indexOfRefraction;

		char8	diffuseTexture[512];
        char8   lightmapTexture[512];       // decal texture as well
		char8	reflectiveTexture[512];
        char8   specularTexture[512];
        char8   normalMapTexture[512];
        
        uint8   hasOpacity;                 // means material has opacity and have to be sorted from back to front
	};

	struct LightDef : ObjectDef
	{	
	public:
		enum eLightType
		{
			AMBIENT,
			SPOT,
			DIRECTIONAL,
			POINT
		};
		
		eLightType type;
		
		/// position or direction (directional lights have w = 0)
		Vector4 position;
		Vector4 ambient;
		Vector4 diffuse;
		Vector4 specular;
		Vector4 spotDirection;
	
		float32 constanAttenuation;
		float32 linearAttenuation;
		float32 quadraticAttenuation;
		float32 spotCutOff;
		float32 spotExponent;
	};
	
	struct CameraDef : ObjectDef
	{	
	public:
		float32 znear;
		float32 zfar;
		float32 fovy;
		bool ortho;
		
		CameraDef()
		{
			znear = 1.0f;
			zfar = 2500.f;
			fovy = 35.f;
			ortho = false;
		}
	};
	
	struct SceneNodeDef
	{
		enum
		{
			SCENE_NODE_BASE = 0,		// base node without additional data
			SCENE_NODE_MESH,			// node with mesh instance
			SCENE_NODE_ANIMATED_MESH,	// node with animated mesh
			SCENE_NODE_CAMERA,			// node with camera
			SCENE_NODE_SKELETON,		// root skeleton node
			SCENE_NODE_BONE,			// other skeleton bones
		};
		
		int32	parentId;				// id of parent node
		int32	childCount;				// number of childs
		Matrix4 localTransform;			// local transform matrix
		int32	nodeType;				// type of node
		int32	customDataSize;			// custom data size
	};
	
	struct PolygonGroupInstanceDef
	{
		int32 materialId;	
		int32 polygonId;
	};

private:
    
    void ProcessLOD(Entity *forRootNode);
	String scenePath;
    FilePath rootNodePath;
    Entity *rootNode;
    
    //int32 textureIndexOffset;
	//int32 staticMeshIndexOffset;
	int32 animatedMeshIndexOffset;
	int32 cameraIndexOffset;

    Vector<NMaterial*> materials;
    Vector<StaticMesh*> staticMeshes;
    Vector<AnimatedMesh*> animatedMeshes;
    Vector<SceneNodeAnimationList*> animations;
};


}; // namespace DAVA

#endif // __DAVAENGINE_SCENEFORMAT_H__





// Copyright (c) 2016 Volodymyr Syvochka
#pragma once

namespace Rio
{
	class ResourceLoader;
	class ResourceManager;
	struct ResourcePackage;

	struct TextureResource;
	struct ShaderResource;
	struct MaterialResource;
	struct MeshResource;

	struct FontResource;
	struct SpriteAnimationResource;
	struct SpriteResource;

	struct UnitResource;
	struct LevelResource;
	struct PackageResource;

	struct SoundResource;

	struct ScriptResource;

	struct PhysicsConfigResource;
	struct PhysicsResource;
	struct ActorResource;
} // namespace Rio

#define RESOURCE_EXTENSION_CONFIG "config"
#define RESOURCE_EXTENSION_FONT "font"
#define RESOURCE_EXTENSION_LEVEL "level"
#define RESOURCE_EXTENSION_MATERIAL "material"
#define RESOURCE_EXTENSION_MESH "mesh"
#define RESOURCE_EXTENSION_PACKAGE "package"
#define RESOURCE_EXTENSION_PHYSICS_CONFIG "physicsConfig"
#define RESOURCE_EXTENSION_PHYSICS "physics"
#define RESOURCE_EXTENSION_SCRIPT "script"
#define RESOURCE_EXTENSION_SHADER "shader"
#define RESOURCE_EXTENSION_SOUND "sound"
#define RESOURCE_EXTENSION_SPRITE_ANIMATION "spriteAnimation"
#define RESOURCE_EXTENSION_SPRITE "sprite"
#define RESOURCE_EXTENSION_TEXTURE "texture"
#define RESOURCE_EXTENSION_UNIT "unit"

#define RESOURCE_TYPE_CONFIG StringId64("config")
#define RESOURCE_TYPE_FONT StringId64("font")
#define RESOURCE_TYPE_LEVEL StringId64("level")
#define RESOURCE_TYPE_MATERIAL StringId64("material")
#define RESOURCE_TYPE_MESH StringId64("mesh")
#define RESOURCE_TYPE_PACKAGE StringId64("package")
#define RESOURCE_TYPE_PHYSICS_CONFIG StringId64("phisicsConfig")
#define RESOURCE_TYPE_PHYSICS StringId64("physics")
#define RESOURCE_TYPE_SCRIPT StringId64("script")
#define RESOURCE_TYPE_SHADER StringId64("shader")
#define RESOURCE_TYPE_SOUND StringId64("sound")
#define RESOURCE_TYPE_SPRITE_ANIMATION StringId64("spriteAnimation")
#define RESOURCE_TYPE_SPRITE StringId64("sprite")
#define RESOURCE_TYPE_TEXTURE StringId64("texture")
#define RESOURCE_TYPE_UNIT StringId64("unit")

#define RESOURCE_VERSION_CONFIG uint32_t(1)
#define RESOURCE_VERSION_FONT uint32_t(1)
#define RESOURCE_VERSION_LEVEL uint32_t(1)
#define RESOURCE_VERSION_MATERIAL uint32_t(1)
#define RESOURCE_VERSION_MESH uint32_t(1)
#define RESOURCE_VERSION_PACKAGE uint32_t(1)
#define RESOURCE_VERSION_PHYSICS_CONFIG uint32_t(1)
#define RESOURCE_VERSION_PHYSICS uint32_t(1)
#define RESOURCE_VERSION_SCRIPT uint32_t(1)
#define RESOURCE_VERSION_SHADER uint32_t(1)
#define RESOURCE_VERSION_SOUND uint32_t(1)
#define RESOURCE_VERSION_SPRITE_ANIMATION uint32_t(1)
#define RESOURCE_VERSION_SPRITE uint32_t(1)
#define RESOURCE_VERSION_TEXTURE uint32_t(1)
#define RESOURCE_VERSION_UNIT uint32_t(1)

// Copyright (c) 2016 Volodymyr Syvochka
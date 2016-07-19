// Copyright (c) 2016 Volodymyr Syvochka
#pragma once

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

#define RESOURCE_TYPE_CONFIG           StringId64(0x82645835e6b73232)
#define RESOURCE_TYPE_FONT             StringId64(0x9efe0a916aae7880)
#define RESOURCE_TYPE_LEVEL            StringId64(0x2a690fd348fe9ac5)
#define RESOURCE_TYPE_MATERIAL         StringId64(0xeac0b497876adedf)
#define RESOURCE_TYPE_MESH             StringId64(0x48ff313713a997a1)
#define RESOURCE_TYPE_PACKAGE          StringId64(0xad9c6d9ed1e5e77a)
#define RESOURCE_TYPE_PHYSICS_CONFIG   StringId64(0x72e3cc03787a11a1)
#define RESOURCE_TYPE_PHYSICS          StringId64(0x5f7203c8f280dab8)
#define RESOURCE_TYPE_SCRIPT           StringId64(0xa14e8dfa2cd117e2)
#define RESOURCE_TYPE_SHADER           StringId64(0xcce8d5b5f5ae333f)
#define RESOURCE_TYPE_SOUND            StringId64(0x90641b51c98b7aac)
#define RESOURCE_TYPE_SPRITE_ANIMATION StringId64(0x487e78e3f87f238d)
#define RESOURCE_TYPE_SPRITE           StringId64(0x8d5871f9ebdb651c)
#define RESOURCE_TYPE_TEXTURE          StringId64(0xcd4238c6a0c69e32)
#define RESOURCE_TYPE_UNIT             StringId64(0xe0a48d0be9a7453f)

#define RESOURCE_VERSION_CONFIG           uint32_t(1)
#define RESOURCE_VERSION_FONT             uint32_t(1)
#define RESOURCE_VERSION_LEVEL            uint32_t(1)
#define RESOURCE_VERSION_MATERIAL         uint32_t(1)
#define RESOURCE_VERSION_MESH             uint32_t(1)
#define RESOURCE_VERSION_PACKAGE          uint32_t(1)
#define RESOURCE_VERSION_PHYSICS_CONFIG   uint32_t(1)
#define RESOURCE_VERSION_PHYSICS          uint32_t(1)
#define RESOURCE_VERSION_SCRIPT           uint32_t(1)
#define RESOURCE_VERSION_SHADER           uint32_t(1)
#define RESOURCE_VERSION_SOUND            uint32_t(1)
#define RESOURCE_VERSION_SPRITE_ANIMATION uint32_t(1)
#define RESOURCE_VERSION_SPRITE           uint32_t(1)
#define RESOURCE_VERSION_TEXTURE          uint32_t(1)
#define RESOURCE_VERSION_UNIT             uint32_t(1)

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
// Copyright (c) 2016 Volodymyr Syvochka
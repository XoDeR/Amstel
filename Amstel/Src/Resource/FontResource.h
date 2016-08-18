// Copyright (c) 2016 Volodymyr Syvochka
#pragma once

#include "Core/Base/Types.h"
#include "Core/Memory/MemoryTypes.h"
#include "Core/FileSystem/FileSystemTypes.h"
#include "Resource/CompilerTypes.h"

namespace Rio
{

struct FontResource
{
	uint32_t version = 0;
	uint32_t glyphCount = 0;
	uint32_t textureSize = 0;
	uint32_t fontSize = 0;
};

struct GlyphData
{
	float x = 0.0f;
	float y = 0.0f;
	float width = 0.0f;
	float height = 0.0f;
	float xOffset = 0.0f;
	float yOffset = 0.0f;
	float xAdvance = 0.0f;
};

using CodePoint = uint32_t;

namespace FontResourceInternalFn
{
	void compile(const char* path, CompileOptions& compileOptions);
	void* load(File& file, Allocator& a);
	void unload(Allocator& allocator, void* resource);
} // namespace FontResourceInternalFn

namespace FontResourceFn
{
	const GlyphData* getGlyph(const FontResource* fontResource, CodePoint codePoint);
} // namespace FontResourceFn

} // namespace Rio
// Copyright (c) 2016 Volodymyr Syvochka
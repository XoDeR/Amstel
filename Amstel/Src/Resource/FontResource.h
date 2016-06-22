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
	uint32_t version;
	uint32_t glyphCount;
	uint32_t textureSize;
	uint32_t fontSize;
};

struct GlyphData
{
	float x;
	float y;
	float width;
	float height;
	float xOffset;
	float yOffset;
	float xAdvance;
};

using CodePoint = uint32_t;

namespace FontResourceFn
{
	void compile(const char* path, CompileOptions& compileOptions);
	void* load(File& file, Allocator& a);
	void unload(Allocator& allocator, void* resource);
	const GlyphData* getGlyph(const FontResource* fontResource, CodePoint codePoint);
} // namespace FontResourceFn

} // namespace Rio
// Copyright (c) 2016 Volodymyr Syvochka
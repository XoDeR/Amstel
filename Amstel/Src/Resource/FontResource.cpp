// Copyright (c) 2016 Volodymyr Syvochka
#include "Resource/FontResource.h"
#include "Core/Json/JsonR.h"
#include "Core/Strings/StringUtils.h"
#include "Core/Memory/Allocator.h"
#include "Core/FileSystem/FileSystem.h"
#include "Core/Containers/Map.h"
#include "Resource/ResourceTypes.h"
#include "Resource/CompileOptions.h"

#include <algorithm>

namespace Rio
{

namespace FontResourceFn
{
	struct GlyphInfo
	{
		CodePoint codePoint;
		GlyphData glyphData;

		bool operator<(const GlyphInfo& a) const
		{
			return codePoint < a.codePoint;
		}
	};

	void parseGlyph(const char* json, GlyphInfo& glyph)
	{
		TempAllocator512 ta;
		JsonObject jsonObject(ta);
		JsonRFn::parse(json, jsonObject);

		glyph.codePoint = JsonRFn::parseInt(jsonObject["id"]);
		glyph.glyphData.x = JsonRFn::parseFloat(jsonObject["x"]);
		glyph.glyphData.y = JsonRFn::parseFloat(jsonObject["y"]);
		glyph.glyphData.width = JsonRFn::parseFloat(jsonObject["width"]);
		glyph.glyphData.height = JsonRFn::parseFloat(jsonObject["height"]);
		glyph.glyphData.xOffset = JsonRFn::parseFloat(jsonObject["xOffset"]);
		glyph.glyphData.yOffset = JsonRFn::parseFloat(jsonObject["yOffset"]);
		glyph.glyphData.xAdvance = JsonRFn::parseFloat(jsonObject["xAdvance"]);
	}

	void compile(const char* path, CompileOptions& compileOptions)
	{
		Buffer buffer = compileOptions.read(path);

		TempAllocator4096 ta;
		JsonObject object(ta);
		JsonArray glyphs(ta);

		JsonRFn::parse(buffer, object);
		JsonRFn::parseArray(object["glyphs"], glyphs);

		const uint32_t textureSize = JsonRFn::parseInt(object["size"]);
		const uint32_t fontSize    = JsonRFn::parseInt(object["fontSize"]);
		const uint32_t glyphCount   = ArrayFn::getCount(glyphs);

		Array<GlyphInfo> glyphInfoList(getDefaultAllocator());
		ArrayFn::resize(glyphInfoList, glyphCount);

		for (uint32_t i = 0; i < glyphCount; ++i)
		{
			parseGlyph(glyphs[i], glyphInfoList[i]);
		}

		std::sort(ArrayFn::begin(glyphInfoList), ArrayFn::end(glyphInfoList));

		compileOptions.write(RESOURCE_VERSION_FONT);
		compileOptions.write(glyphCount);
		compileOptions.write(textureSize);
		compileOptions.write(fontSize);

		for (uint32_t i = 0; i < ArrayFn::getCount(glyphInfoList); ++i)
		{
			compileOptions.write(glyphInfoList[i].codePoint);
		}

		for (uint32_t i = 0; i < ArrayFn::getCount(glyphInfoList); ++i)
		{
			compileOptions.write(glyphInfoList[i].glyphData.x);
			compileOptions.write(glyphInfoList[i].glyphData.y);
			compileOptions.write(glyphInfoList[i].glyphData.width);
			compileOptions.write(glyphInfoList[i].glyphData.height);
			compileOptions.write(glyphInfoList[i].glyphData.xOffset);
			compileOptions.write(glyphInfoList[i].glyphData.yOffset);
			compileOptions.write(glyphInfoList[i].glyphData.xAdvance);
		}
	}

	void* load(File& file, Allocator& a)
	{
		const uint32_t size = file.getSize();
		void* resource = a.allocate(size);
		file.read(resource, size);
		RIO_ASSERT(*(uint32_t*)resource == RESOURCE_VERSION_FONT, "Wrong version");
		return resource;
	}

	void unload(Allocator& allocator, void* resource)
	{
		allocator.deallocate(resource);
	}

	const GlyphData* getGlyph(const FontResource* fontResource, CodePoint codePoint)
	{
		RIO_ASSERT(codePoint < fontResource->glyphCount, "Index out of bounds");

		const CodePoint* codePointList = (CodePoint*)&fontResource[1];
		const GlyphData* data = (GlyphData*)(codePointList + fontResource->glyphCount);

		// TODO (could use binary search)
		for (uint32_t i = 0; i < fontResource->glyphCount; ++i)
		{
			if (codePointList[i] == codePoint)
			{
				return &data[i];
			}
		}

		RIO_FATAL("Glyph not found");
		return nullptr;
	}
} // namespace FontResourceFn

} // namespace Rio
// Copyright (c) 2016 Volodymyr Syvochka
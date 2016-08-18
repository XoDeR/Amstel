// Copyright (c) 2016 Volodymyr Syvochka
#include "Resource/ShaderResource.h"

#include "Config.h"
#include "Core/FileSystem/FileSystem.h"
#include "Core/Containers/Map.h"
#include "Core/Base/Os.h"
#include "Core/Json/JsonR.h"
#include "Core/Json/JsonObject.h"
#include "Core/Strings/StringStream.h"
#include "Core/Memory/TempAllocator.h"

#include "Device/Device.h"

#include "Resource/CompileOptions.h"
#include "Resource/ResourceManager.h"

#include "World/ShaderManager.h"

#if RIO_DEVELOPMENT
	#define SHADERC_NAME "shaderc-development-"
#elif RIO_DEBUG
	#define SHADERC_NAME "shaderc-debug-"
#else
	#define SHADERC_NAME "shaderc-release-"
#endif  // RIO_DEBUG
#if RIO_ARCH_32BIT
	#define SHADERC_BITS "32"
#elif RIO_ARCH_64BIT
	#define SHADERC_BITS "64"
#endif // RIO_ARCH_32BIT
#if RIO_PLATFORM_LINUX
	#define SHADERC_PATH "./" SHADERC_NAME "" SHADERC_BITS
#elif RIO_PLATFORM_WINDOWS
	#define SHADERC_PATH SHADERC_NAME "" SHADERC_BITS ".exe"
#else
	#define SHADERC_PATH ""
#endif // RIO_PLATFORM_LINUX

namespace Rio
{

namespace ShaderResourceInternalFn
{
	struct DepthFunction
	{
		enum Enum
		{
			LESS,
			LEQUAL,
			EQUAL,
			GEQUAL,
			GREATER,
			NOTEQUAL,
			NEVER,
			ALWAYS,

			COUNT
		};
	};

	struct BlendFunction
	{
		enum Enum
		{
			ZERO,
			ONE,
			SRC_COLOR,
			INV_SRC_COLOR,
			SRC_ALPHA,
			INV_SRC_ALPHA,
			DST_ALPHA,
			INV_DST_ALPHA,
			DST_COLOR,
			INV_DST_COLOR,
			SRC_ALPHA_SAT,
			FACTOR,
			INV_FACTOR,

			COUNT
		};
	};

	struct BlendEquation
	{
		enum Enum
		{
			ADD,
			SUB,
			REVSUB,
			MIN,
			MAX,

			COUNT
		};
	};

	struct CullMode
	{
		enum Enum
		{
			CW,
			CCW,
			NONE,

			COUNT
		};
	};

	struct PrimitiveType
	{
		enum Enum
		{
			PT_TRISTRIP,
			PT_LINES,
			PT_LINESTRIP,
			PT_POINTS,

			COUNT
		};
	};

	struct SamplerFilter
	{
		enum Enum
		{
			POINT,
			ANISOTROPIC,

			COUNT
		};
	};

	struct SamplerWrap
	{
		enum Enum
		{
			MIRROR,
			CLAMP,
			BORDER,

			COUNT
		};
	};

	struct DepthTestInfo
	{
		const char* name;
		DepthFunction::Enum value;
	};

	static DepthTestInfo depthTestMap[] =
	{
		{ "less",     DepthFunction::LESS     },
		{ "lequal",   DepthFunction::LEQUAL   },
		{ "equal",    DepthFunction::EQUAL    },
		{ "gequal",   DepthFunction::GEQUAL   },
		{ "greater",  DepthFunction::GREATER  },
		{ "notequal", DepthFunction::NOTEQUAL },
		{ "never",    DepthFunction::NEVER    },
		{ "always",   DepthFunction::ALWAYS   }
	};
	RIO_STATIC_ASSERT(RIO_COUNTOF(depthTestMap) == DepthFunction::COUNT);

	struct BlendFunctionInfo
	{
		const char* name;
		BlendFunction::Enum value;
	};

	static BlendFunctionInfo blendFunctionsMap[] =
	{
		{ "zero",          BlendFunction::ZERO },
		{ "one",           BlendFunction::ONE },
		{ "srcColor",     BlendFunction::SRC_COLOR },
		{ "invSrcColor", BlendFunction::INV_SRC_COLOR },
		{ "srcAlpha",     BlendFunction::SRC_ALPHA },
		{ "invSrcAlpha", BlendFunction::INV_SRC_ALPHA },
		{ "dstAlpha",     BlendFunction::DST_ALPHA },
		{ "invDstAlpha", BlendFunction::INV_DST_ALPHA },
		{ "dstColor",     BlendFunction::DST_COLOR },
		{ "invDstColor", BlendFunction::INV_DST_COLOR },
		{ "srcAlphaSat", BlendFunction::SRC_ALPHA_SAT },
		{ "factor",        BlendFunction::FACTOR },
		{ "invFactor",    BlendFunction::INV_FACTOR }
	};
	RIO_STATIC_ASSERT(RIO_COUNTOF(blendFunctionsMap) == BlendFunction::COUNT);

	struct BlendEquationInfo
	{
		const char* name;
		BlendEquation::Enum value;
	};

	static BlendEquationInfo blendEquationMap[] =
	{
		{ "add",    BlendEquation::ADD    },
		{ "sub",    BlendEquation::SUB    },
		{ "revSub", BlendEquation::REVSUB },
		{ "min",    BlendEquation::MIN    },
		{ "max",    BlendEquation::MAX    }
	};
	RIO_STATIC_ASSERT(RIO_COUNTOF(blendEquationMap) == BlendEquation::COUNT);

	struct CullModeInfo
	{
		const char* name;
		CullMode::Enum value;
	};

	static CullModeInfo cullModeInfoMap[] =
	{
		{ "cw",   CullMode::CW   },
		{ "ccw",  CullMode::CCW  },
		{ "none", CullMode::NONE }
	};
	RIO_STATIC_ASSERT(RIO_COUNTOF(cullModeInfoMap) == CullMode::COUNT);

	struct PrimitiveTypeInfo
	{
		const char* name;
		PrimitiveType::Enum value;
	};

	static PrimitiveTypeInfo primitiveTypeInfoMap[] =
	{
		{ "ptTristrip",  PrimitiveType::PT_TRISTRIP  },
		{ "ptLines",     PrimitiveType::PT_LINES     },
		{ "ptLinestrip", PrimitiveType::PT_LINESTRIP },
		{ "ptPoints",    PrimitiveType::PT_POINTS    }
	};
	RIO_STATIC_ASSERT(RIO_COUNTOF(primitiveTypeInfoMap) == PrimitiveType::COUNT);

	struct SamplerFilterInfo
	{
		const char* name;
		SamplerFilter::Enum value;
	};

	static SamplerFilterInfo samplerFilterInfoMap[] =
	{
		{ "point",       SamplerFilter::POINT       },
		{ "anisotropic", SamplerFilter::ANISOTROPIC }
	};
	RIO_STATIC_ASSERT(RIO_COUNTOF(samplerFilterInfoMap) == SamplerFilter::COUNT);

	struct SamplerWrapInfo
	{
		const char* name;
		SamplerWrap::Enum value;
	};

	static SamplerWrapInfo samplerWrapMap[] =
	{
		{ "mirror", SamplerWrap::MIRROR },
		{ "clamp",  SamplerWrap::CLAMP  },
		{ "border", SamplerWrap::BORDER }
	};
	RIO_STATIC_ASSERT(RIO_COUNTOF(samplerWrapMap) == SamplerWrap::COUNT);

	static uint64_t bgfxDepthFunctionsMap[] =
	{
		BGFX_STATE_DEPTH_TEST_LESS,     // DepthFunction::LESS
		BGFX_STATE_DEPTH_TEST_LEQUAL,   // DepthFunction::LEQUAL
		BGFX_STATE_DEPTH_TEST_EQUAL,    // DepthFunction::EQUAL
		BGFX_STATE_DEPTH_TEST_GEQUAL,   // DepthFunction::GEQUAL
		BGFX_STATE_DEPTH_TEST_GREATER,  // DepthFunction::GREATER
		BGFX_STATE_DEPTH_TEST_NOTEQUAL, // DepthFunction::NOTEQUAL
		BGFX_STATE_DEPTH_TEST_NEVER,    // DepthFunction::NEVER
		BGFX_STATE_DEPTH_TEST_ALWAYS    // DepthFunction::ALWAYS
	};
	RIO_STATIC_ASSERT(RIO_COUNTOF(bgfxDepthFunctionsMap) == DepthFunction::COUNT);

	static uint64_t bgfxBlendFunctionsMap[] =
	{
		BGFX_STATE_BLEND_ZERO,          // BlendFunction::ZERO
		BGFX_STATE_BLEND_ONE,           // BlendFunction::ONE
		BGFX_STATE_BLEND_SRC_COLOR,     // BlendFunction::SRC_COLOR
		BGFX_STATE_BLEND_INV_SRC_COLOR, // BlendFunction::INV_SRC_COLOR
		BGFX_STATE_BLEND_SRC_ALPHA,     // BlendFunction::SRC_ALPHA
		BGFX_STATE_BLEND_INV_SRC_ALPHA, // BlendFunction::INV_SRC_ALPHA
		BGFX_STATE_BLEND_DST_ALPHA,     // BlendFunction::DST_ALPHA
		BGFX_STATE_BLEND_INV_DST_ALPHA, // BlendFunction::INV_DST_ALPHA
		BGFX_STATE_BLEND_DST_COLOR,     // BlendFunction::DST_COLOR
		BGFX_STATE_BLEND_INV_DST_COLOR, // BlendFunction::INV_DST_COLOR
		BGFX_STATE_BLEND_SRC_ALPHA_SAT, // BlendFunction::SRC_ALPHA_SAT
		BGFX_STATE_BLEND_FACTOR,        // BlendFunction::FACTOR
		BGFX_STATE_BLEND_INV_FACTOR     // BlendFunction::INV_FACTOR
	};
	RIO_STATIC_ASSERT(RIO_COUNTOF(bgfxBlendFunctionsMap) == BlendFunction::COUNT);

	static uint64_t bgfxBlendEquationMap[] =
	{
		BGFX_STATE_BLEND_EQUATION_ADD,    // BlendEquation::ADD
		BGFX_STATE_BLEND_EQUATION_SUB,    // BlendEquation::SUB
		BGFX_STATE_BLEND_EQUATION_REVSUB, // BlendEquation::REVSUB
		BGFX_STATE_BLEND_EQUATION_MIN,    // BlendEquation::MIN
		BGFX_STATE_BLEND_EQUATION_MAX     // BlendEquation::MAX
	};
	RIO_STATIC_ASSERT(RIO_COUNTOF(bgfxBlendEquationMap) == BlendEquation::COUNT);

	static uint64_t bgfxCullModeMap[] =
	{
		BGFX_STATE_CULL_CW,  // CullMode::CW
		BGFX_STATE_CULL_CCW, // CullMode::CCW
		0                    // CullMode::NONE
	};
	RIO_STATIC_ASSERT(RIO_COUNTOF(bgfxCullModeMap) == CullMode::COUNT);

	static uint64_t bgfxPrimitiveTypeMap[] =
	{
		BGFX_STATE_PT_TRISTRIP,  // PrimitiveType::PT_TRISTRIP
		BGFX_STATE_PT_LINES,     // PrimitiveType::PT_LINES
		BGFX_STATE_PT_LINESTRIP, // PrimitiveType::PT_LINESTRIP
		BGFX_STATE_PT_POINTS     // PrimitiveType::PT_POINTS
	};
	RIO_STATIC_ASSERT(RIO_COUNTOF(bgfxPrimitiveTypeMap) == PrimitiveType::COUNT);

	static uint32_t bgfxSamplerFilterMinMap[] =
	{
		BGFX_TEXTURE_MIN_POINT,       // SamplerFilter::POINT
		BGFX_TEXTURE_MIN_ANISOTROPIC  // SamplerFilter::ANISOTROPIC
	};
	RIO_STATIC_ASSERT(RIO_COUNTOF(bgfxSamplerFilterMinMap) == SamplerFilter::COUNT);

	static uint32_t bgfxSamplerFilterMagMap[] =
	{
		BGFX_TEXTURE_MAG_POINT,      // SamplerFilter::POINT
		BGFX_TEXTURE_MAG_ANISOTROPIC // SamplerFilter::ANISOTROPIC
	};
	RIO_STATIC_ASSERT(RIO_COUNTOF(bgfxSamplerFilterMagMap) == SamplerFilter::COUNT);

	static uint32_t bgfxSamplerWrapUMap[] =
	{
		BGFX_TEXTURE_U_MIRROR, // SamplerWrap::MIRROR
		BGFX_TEXTURE_U_CLAMP,  // SamplerWrap::CLAMP
		BGFX_TEXTURE_U_BORDER  // SamplerWrap::BORDER
	};
	RIO_STATIC_ASSERT(RIO_COUNTOF(bgfxSamplerWrapUMap) == SamplerWrap::COUNT);

	static uint32_t bgfxSamplerWrapVMap[] =
	{
		BGFX_TEXTURE_V_MIRROR, // SamplerWrap::MIRROR
		BGFX_TEXTURE_V_CLAMP,  // SamplerWrap::CLAMP
		BGFX_TEXTURE_V_BORDER  // SamplerWrap::BORDER
	};
	RIO_STATIC_ASSERT(RIO_COUNTOF(bgfxSamplerWrapVMap) == SamplerWrap::COUNT);

	static uint32_t bgfxSamplerWrapWMap[] =
	{
		BGFX_TEXTURE_W_MIRROR, // SamplerWrap::MIRROR
		BGFX_TEXTURE_W_CLAMP,  // SamplerWrap::CLAMP
		BGFX_TEXTURE_W_BORDER  // SamplerWrap::BORDER
	};
	RIO_STATIC_ASSERT(RIO_COUNTOF(bgfxSamplerWrapWMap) == SamplerWrap::COUNT);

	static DepthFunction::Enum getDepthFunctionFromName(const char* name)
	{
		for (uint32_t i = 0; i < RIO_COUNTOF(depthTestMap); ++i)
		{
			if (strcmp(name, depthTestMap[i].name) == 0)
			{
				return depthTestMap[i].value;
			}
		}

		return DepthFunction::COUNT;
	}

	static BlendFunction::Enum getBlendFunctionFromName(const char* name)
	{
		for (uint32_t i = 0; i < RIO_COUNTOF(blendFunctionsMap); ++i)
		{
			if (strcmp(name, blendFunctionsMap[i].name) == 0)
			{
				return blendFunctionsMap[i].value;
			}
		}

		return BlendFunction::COUNT;
	}

	static BlendEquation::Enum getBlendEquationFromName(const char* name)
	{
		for (uint32_t i = 0; i < RIO_COUNTOF(blendEquationMap); ++i)
		{
			if (strcmp(name, blendEquationMap[i].name) == 0)
			{
				return blendEquationMap[i].value;
			}
		}

		return BlendEquation::COUNT;
	}

	static CullMode::Enum getCullModeFromName(const char* name)
	{
		for (uint32_t i = 0; i < RIO_COUNTOF(cullModeInfoMap); ++i)
		{
			if (strcmp(name, cullModeInfoMap[i].name) == 0)
			{
				return cullModeInfoMap[i].value;
			}
		}

		return CullMode::COUNT;
	}

	static PrimitiveType::Enum getPrimitiveTypeFromMap(const char* name)
	{
		for (uint32_t i = 0; i < RIO_COUNTOF(primitiveTypeInfoMap); ++i)
		{
			if (strcmp(name, primitiveTypeInfoMap[i].name) == 0)
			{
				return primitiveTypeInfoMap[i].value;
			}
		}

		return PrimitiveType::COUNT;
	}

	static SamplerFilter::Enum getSamplerFilterFromName(const char* name)
	{
		for (uint32_t i = 0; i < RIO_COUNTOF(samplerFilterInfoMap); ++i)
		{
			if (strcmp(name, samplerFilterInfoMap[i].name) == 0)
			{
				return samplerFilterInfoMap[i].value;
			}
		}

		return SamplerFilter::COUNT;
	}

	static SamplerWrap::Enum getSamplerWrapFromName(const char* name)
	{
		for (uint32_t i = 0; i < RIO_COUNTOF(samplerWrapMap); ++i)
		{
			if (strcmp(name, samplerWrapMap[i].name) == 0)
			{
				return samplerWrapMap[i].value;
			}
		}

		return SamplerWrap::COUNT;
	}

	static int runExternalCompiler(const char* inputFile, const char* outputFile, const char* varying, const char* type, const char* platform, StringStream& output)
	{
		TempAllocator512 ta;
		StringStream arguments(ta);
		arguments << " -f " << inputFile;
		arguments << " -o " << outputFile;
		arguments << " --varyingdef " << varying;
		arguments << " --type " << type;
		arguments << " --platform " << platform;
		if (strcmp("windows", platform) == 0)
		{
			arguments << " --profile ";
			arguments << ((strcmp(type, "vertex") == 0) ? "vs_3_0" : "ps_3_0");
		}

		return OsFn::executeProcess(SHADERC_PATH, StringStreamFn::getCStr(arguments), output);
	}

	struct RenderState
	{
		void reset()
		{
			rgbWriteEnable = false;
			alphaWriteEnable = false;
			depthWriteEnable = false;
			depthEnable = false;
			blendEnable = false;
			depthFunction = DepthFunction::COUNT;
			blendFunctionSrc = BlendFunction::COUNT;
			blendFunctionDst = BlendFunction::COUNT;
			blendEquation = BlendEquation::COUNT;
			cullMode = CullMode::COUNT;
			primitiveType = PrimitiveType::COUNT;
		}

		uint64_t encode() const
		{
			const uint64_t depthFunction = (this->depthEnable
				? bgfxDepthFunctionsMap[this->depthFunction]
				: 0
				);
			const uint64_t blendFunction = (this->blendEnable
				? BGFX_STATE_BLEND_FUNC(bgfxBlendFunctionsMap[this->blendFunctionSrc], bgfxBlendFunctionsMap[this->blendFunctionDst])
				: 0
				);
			const uint64_t blendEquation = (this->blendEnable
				? BGFX_STATE_BLEND_EQUATION(bgfxBlendEquationMap[this->blendEquation])
				: 0
				);
			const uint64_t cullMode = (this->cullMode != CullMode::COUNT
				? bgfxCullModeMap[this->cullMode]
				: 0
				);
			const uint64_t primitiveType = (this->primitiveType != PrimitiveType::COUNT
				? bgfxPrimitiveTypeMap[this->primitiveType]
				: 0
				);

			uint64_t state = 0;
			state |= (this->rgbWriteEnable ? BGFX_STATE_RGB_WRITE   : 0);
			state |= (this->alphaWriteEnable ? BGFX_STATE_ALPHA_WRITE : 0);
			state |= (this->depthWriteEnable ? BGFX_STATE_DEPTH_WRITE : 0);
			state |= depthFunction;
			state |= blendFunction;
			state |= blendEquation;
			state |= cullMode;
			state |= primitiveType;

			return state;
		}

		bool rgbWriteEnable = false;
		bool alphaWriteEnable = false;
		bool depthWriteEnable = false;
		bool depthEnable = false;
		bool blendEnable = false;
		DepthFunction::Enum depthFunction = DepthFunction::COUNT;
		BlendFunction::Enum blendFunctionSrc = BlendFunction::COUNT;
		BlendFunction::Enum blendFunctionDst = BlendFunction::COUNT;
		BlendEquation::Enum blendEquation = BlendEquation::COUNT;
		CullMode::Enum cullMode = CullMode::COUNT;
		PrimitiveType::Enum primitiveType = PrimitiveType::COUNT;
	};

	struct SamplerState
	{
		void reset()
		{
			sampleFilterMin = SamplerFilter::COUNT;
			sampleFilterMag = SamplerFilter::COUNT;
			sampleWrapU = SamplerWrap::COUNT;
			sampleWrapV = SamplerWrap::COUNT;
			sampleWrapW = SamplerWrap::COUNT;
		}

		uint32_t encode() const
		{
			uint32_t state = 0;
			state |= sampleFilterMin == SamplerFilter::COUNT ? 0 : bgfxSamplerFilterMinMap[sampleFilterMin];
			state |= sampleFilterMag == SamplerFilter::COUNT ? 0 : bgfxSamplerFilterMagMap[sampleFilterMag];
			state |= sampleWrapU == SamplerWrap::COUNT ? 0 : bgfxSamplerWrapUMap[sampleWrapU];
			state |= sampleWrapV == SamplerWrap::COUNT ? 0 : bgfxSamplerWrapVMap[sampleWrapV];
			state |= sampleWrapW == SamplerWrap::COUNT ? 0 : bgfxSamplerWrapWMap[sampleWrapW];
			return state;
		}

		SamplerFilter::Enum sampleFilterMin = SamplerFilter::COUNT;
		SamplerFilter::Enum sampleFilterMag = SamplerFilter::COUNT;
		SamplerWrap::Enum sampleWrapU = SamplerWrap::COUNT;
		SamplerWrap::Enum sampleWrapV = SamplerWrap::COUNT;
		SamplerWrap::Enum sampleWrapW = SamplerWrap::COUNT;
	};

	struct BgfxShader
	{
		ALLOCATOR_AWARE;

		BgfxShader()
			: includes(getDefaultAllocator())
			, code(getDefaultAllocator())
			, vertexShaderCode(getDefaultAllocator())
			, fragmentShaderCode(getDefaultAllocator())
			, varying(getDefaultAllocator())
			, vertexShaderInputOutput(getDefaultAllocator())
			, fragmentShaderInputOutput(getDefaultAllocator())
			, samplersMap(getDefaultAllocator())
		{
		}

		BgfxShader(Allocator& a)
			: includes(a)
			, code(a)
			, vertexShaderCode(a)
			, fragmentShaderCode(a)
			, varying(a)
			, vertexShaderInputOutput(a)
			, fragmentShaderInputOutput(a)
			, samplersMap(a)
		{
		}

		DynamicString includes;
		DynamicString code;
		DynamicString vertexShaderCode;
		DynamicString fragmentShaderCode;
		DynamicString varying;
		DynamicString vertexShaderInputOutput;
		DynamicString fragmentShaderInputOutput;
		Map<DynamicString, DynamicString> samplersMap;
	};

	struct ShaderPermutation
	{
		ALLOCATOR_AWARE;

		ShaderPermutation()
			: bgfxShader(getDefaultAllocator())
			, renderState(getDefaultAllocator())
		{
		}

		ShaderPermutation(Allocator& a)
			: bgfxShader(a)
			, renderState(a)
		{
		}

		DynamicString bgfxShader;
		DynamicString renderState;
	};

	struct StaticCompile
	{
		StaticCompile()
			: shaderName(getDefaultAllocator())
			, defines(getDefaultAllocator())
		{
		}

		StaticCompile(Allocator& a)
			: shaderName(a)
			, defines(a)
		{
		}

		DynamicString shaderName;
		Vector<DynamicString> defines;
	};

	struct ShaderCompiler
	{
		ShaderCompiler(CompileOptions& compileOptions)
			: compileOptions(compileOptions)
			, renderStatesMap(getDefaultAllocator())
			, samplerStatesMap(getDefaultAllocator())
			, bgfxShadersMap(getDefaultAllocator())
			, shaderPermutationsMap(getDefaultAllocator())
			, staticCompileList(getDefaultAllocator())
			, vertexShaderSourcePath(getDefaultAllocator())
			, fragmentShaderSourcePath(getDefaultAllocator())
			, varyingPath(getDefaultAllocator())
			, vertexShaderCompiledPath(getDefaultAllocator())
			, fragmentShaderCompiledPath(getDefaultAllocator())
		{
			compileOptions.getTemporaryPath("vertexShaderSource.sc", vertexShaderSourcePath);
			compileOptions.getTemporaryPath("fragmentShaderSource.sc", fragmentShaderSourcePath);
			compileOptions.getTemporaryPath("varying.sc", varyingPath);
			compileOptions.getTemporaryPath("vertexShaderCompiled.bin", vertexShaderCompiledPath);
			compileOptions.getTemporaryPath("fragmentShaderCompiled.bin", fragmentShaderCompiledPath);
		}

		void parse(const char* path)
		{
			parse(compileOptions.read(path));
		}

		void parse(Buffer buffer)
		{
			TempAllocator4096 ta;
			JsonObject jsonObject(ta);
			JsonRFn::parse(buffer, jsonObject);

			if (JsonObjectFn::has(jsonObject, "include"))
			{
				JsonArray jsonArray(ta);
				JsonRFn::parseArray(jsonObject["include"], jsonArray);

				for (uint32_t i = 0; i < ArrayFn::getCount(jsonArray); ++i)
				{
					DynamicString path(ta);
					JsonRFn::parseString(jsonArray[i], path);
					parse(path.getCStr());
				}
			}

			if (JsonObjectFn::has(jsonObject, "renderStates"))
			{
				parseRenderStates(jsonObject["renderStates"]);
			}

			if (JsonObjectFn::has(jsonObject, "samplerStates"))
			{
				parseSamplerStates(jsonObject["samplerStates"]);
			}

			if (JsonObjectFn::has(jsonObject, "bgfxShaders")))
			{
				parseBgfxShaders(jsonObject["bgfxShaders"]);
			}

			if (JsonObjectFn::has(jsonObject, "shaders"))
			{
				parseShaders(jsonObject["shaders"]);
			}

			if (JsonObjectFn::has(jsonObject, "staticCompile"))
			{
				parseStaticCompile(jsonObject["staticCompile"]);
			}
		}

		void parseRenderStates(const char* json)
		{
			TempAllocator4096 ta;
			JsonObject renderStates(ta);
			JsonRFn::parseObject(json, renderStates);

			auto begin = JsonObjectFn::begin(renderStates);
			auto end = JsonObjectFn::end(renderStates);
			for (; begin != end; ++begin)
			{
				JsonObject jsonObject(ta);
				JsonRFn::parseObject(begin->pair.second, jsonObject);

				const bool rgbWriteEnable = JsonRFn::parseBool(jsonObject["rgbWriteEnable"]);
				const bool alphaWriteEnable = JsonRFn::parseBool(jsonObject["alphaWriteEnable"]);
				const bool depthWriteEnable = JsonRFn::parseBool(jsonObject["depthWriteEnable"]);
				const bool depthEnable = JsonRFn::parseBool(jsonObject["depthEnable"]);
				const bool blendEnable = JsonRFn::parseBool(jsonObject["blendEnable"]);

				const bool hasDepthFunction = JsonObjectFn::has(jsonObject, "depthFunction");
				const bool hasBlendSrc = JsonObjectFn::has(jsonObject, "blendSrc");
				const bool hasBlendDst = JsonObjectFn::has(jsonObject, "blendDst");
				const bool hasBlendEquation = JsonObjectFn::has(jsonObject, "blendEquation");
				const bool hasCullMode = JsonObjectFn::has(jsonObject, "cullMode");
				const bool hasPrimitiveType = JsonObjectFn::has(jsonObject, "primitiveType");

				RenderState renderState;
				renderState.reset();
				renderState.rgbWriteEnable = rgbWriteEnable;
				renderState.alphaWriteEnable = alphaWriteEnable;
				renderState.depthWriteEnable = depthWriteEnable;
				renderState.depthEnable = depthEnable;
				renderState.blendEnable = blendEnable;

				DynamicString depthFunction(ta);
				DynamicString blendSrc(ta);
				DynamicString blendDst(ta);
				DynamicString blendEquation(ta);
				DynamicString cullMode(ta);
				DynamicString primitiveType(ta);

				if (hasDepthFunction == true)
				{
					JsonRFn::parseString(jsonObject["depthFunction"], depthFunction);
					renderState.depthFunction = getDepthFunctionFromName(depthFunction.getCStr());
					RESOURCE_COMPILER_ASSERT(renderState.depthFunction != DepthFunction::COUNT
						, compileOptions
						, "Unknown depth test: '%s'"
						, depthFunction.getCStr()
						);
				}

				if (hasBlendSrc == true)
				{
					JsonRFn::parseString(jsonObject["blendSrc"], blendSrc);
					renderState.blendFunctionSrc = getBlendFunctionFromName(blendSrc.getCStr());
					RESOURCE_COMPILER_ASSERT(renderState.blendFunctionSrc != BlendFunction::COUNT
						, compileOptions
						, "Unknown blend function: '%s'"
						, blendSrc.getCStr()
						);
				}

				if (hasBlendDst == true)
				{
					JsonRFn::parseString(jsonObject["blendDst"], blendDst);
					renderState.blendFunctionDst = getBlendFunctionFromName(blendDst.getCStr());
					RESOURCE_COMPILER_ASSERT(renderState.blendFunctionDst != BlendFunction::COUNT
						, compileOptions
						, "Unknown blend function: '%s'"
						, blendDst.getCStr()
						);
				}

				if (hasBlendEquation == true)
				{
					JsonRFn::parseString(jsonObject["blendEquation"], blendEquation);
					renderState.blendEquation = getBlendEquationFromName(blendEquation.getCStr());
					RESOURCE_COMPILER_ASSERT(renderState.blendEquation != BlendEquation::COUNT
						, compileOptions
						, "Unknown blend equation: '%s'"
						, blendEquation.getCStr()
						);
				}

				if (hasCullMode == true)
				{
					JsonRFn::parseString(jsonObject["cullMode"], cullMode);
					renderState.cullMode = getCullModeFromName(cullMode.getCStr());
					RESOURCE_COMPILER_ASSERT(renderState.cullMode != CullMode::COUNT
						, compileOptions
						, "Unknown cull mode: '%s'"
						, cullMode.getCStr()
						);
				}

				if (hasPrimitiveType == true)
				{
					JsonRFn::parseString(jsonObject["primitiveType"], primitiveType);
					renderState.primitiveType = getPrimitiveTypeFromMap(primitiveType.getCStr());
					RESOURCE_COMPILER_ASSERT(renderState.primitiveType != PrimitiveType::COUNT
						, compileOptions
						, "Unknown primitive type: '%s'"
						, primitiveType.getCStr()
						);
				}

				DynamicString key(ta);
				key = begin->pair.first;

				RESOURCE_COMPILER_ASSERT(!MapFn::has(renderStatesMap, key)
					, compileOptions
					, "Render state redefined: '%s'"
					, key.getCStr()
					);
				MapFn::set(renderStatesMap, key, renderState);
			}
		}

		void parseSamplerStates(const char* json)
		{
			TempAllocator4096 ta;
			JsonObject samplerStates(ta);
			JsonRFn::parseObject(json, samplerStates);

			auto begin = JsonObjectFn::begin(samplerStates);
			auto end = JsonObjectFn::end(samplerStates);
			for (; begin != end; ++begin)
			{
				JsonObject jsonObject(ta);
				JsonRFn::parseObject(begin->pair.second, jsonObject);

				const bool hasFilterMin = JsonObjectFn::has(jsonObject, "filterMin");
				const bool hasFilterMag = JsonObjectFn::has(jsonObject, "filterMag");
				const bool hasWrapU = JsonObjectFn::has(jsonObject, "wrapU");
				const bool hasWrapV = JsonObjectFn::has(jsonObject, "wrapV");
				const bool hasWrapW = JsonObjectFn::has(jsonObject, "wrapW");

				SamplerState samplerState;
				samplerState.reset();

				DynamicString filterMin(ta);
				DynamicString filterMag(ta);
				DynamicString wrapU(ta);
				DynamicString wrapV(ta);
				DynamicString wrapW(ta);

				if (hasFilterMin == true)
				{
					JsonRFn::parseString(jsonObject["filterMin"], filterMin);
					samplerState.sampleFilterMin = getSamplerFilterFromName(filterMin.getCStr());
					RESOURCE_COMPILER_ASSERT(samplerState.sampleFilterMin != SamplerFilter::COUNT
						, compileOptions
						, "Unknown sampler filter: '%s'"
						, filterMin.getCStr()
						);
				}

				if (hasFilterMag == true)
				{
					JsonRFn::parseString(jsonObject["filterMag"], filterMag);
					samplerState.sampleFilterMag = getSamplerFilterFromName(filterMag.getCStr());
					RESOURCE_COMPILER_ASSERT(samplerState.sampleFilterMag != SamplerFilter::COUNT
						, compileOptions
						, "Unknown sampler filter: '%s'"
						, filterMag.getCStr()
						);
				}

				if (hasWrapU == true)
				{
					JsonRFn::parseString(jsonObject["wrapU"], wrapU);
					samplerState.sampleWrapU = getSamplerWrapFromName(wrapU.getCStr());
					RESOURCE_COMPILER_ASSERT(samplerState.sampleWrapU != SamplerWrap::COUNT
						, compileOptions
						, "Unknown wrap mode: '%s'"
						, wrapU.getCStr()
						);
				}

				if (hasWrapV == true)
				{
					JsonRFn::parseString(jsonObject["wrapV"], wrapV);
					samplerState.sampleWrapV = getSamplerWrapFromName(wrapV.getCStr());
					RESOURCE_COMPILER_ASSERT(samplerState.sampleWrapV != SamplerWrap::COUNT
						, compileOptions
						, "Unknown wrap mode: '%s'"
						, wrapV.getCStr()
						);
				}

				if (hasWrapW == true)
				{
					JsonRFn::parseString(jsonObject["wrapW"], wrapW);
					samplerState.sampleWrapW = getSamplerWrapFromName(wrapW.getCStr());
					RESOURCE_COMPILER_ASSERT(samplerState.sampleWrapW != SamplerWrap::COUNT
						, compileOptions
						, "Unknown wrap mode: '%s'"
						, wrapW.getCStr()
						);
				}

				DynamicString key(ta);
				key = begin->pair.first;

				RESOURCE_COMPILER_ASSERT(!MapFn::has(samplerStatesMap, key)
					, compileOptions
					, "Sampler state redefined: '%s'"
					, key.getCStr()
					);
				MapFn::set(samplerStatesMap, key, samplerState);
			}
		}

		void parseBgfxShaders(const char* json)
		{
			TempAllocator4096 ta;
			JsonObject bgfxShaderList(ta);
			JsonRFn::parseObject(json, bgfxShaderList);

			auto begin = JsonObjectFn::begin(bgfxShaderList);
			auto end = JsonObjectFn::end(bgfxShaderList);
			for (; begin != end; ++begin)
			{
				JsonObject shader(ta);
				JsonRFn::parseObject(begin->pair.second, shader);

				BgfxShader bgfxShader(getDefaultAllocator());
				if (JsonObjectFn::has(shader, "includes"))
				{
					JsonRFn::parseString(shader["includes"], bgfxShader.includes);
				}
				if (JsonObjectFn::has(shader, "code"))
				{
					JsonRFn::parseString(shader["code"], bgfxShader.code);
				}
				if (JsonObjectFn::has(shader, "vertexShaderCode"))
				{
					JsonRFn::parseString(shader["vertexShaderCode"], bgfxShader.vertexShaderCode);
				}
				if (JsonObjectFn::has(shader, "fragmentShaderCode"))
				{
					JsonRFn::parseString(shader["fragmentShaderCode"], bgfxShader.fragmentShaderCode);
				}
				if (JsonObjectFn::has(shader, "varying"))
				{
					JsonRFn::parseString(shader["varying"], bgfxShader.varying);
				}
				if (JsonObjectFn::has(shader, "vertexShaderInputOutput"))
				{
					JsonRFn::parseString(shader["vertexShaderInputOutput"], bgfxShader.vertexShaderInputOutput);
				}
				if (JsonObjectFn::has(shader, "fragmentShaderInputOutput"))
				{
					JsonRFn::parseString(shader["fragmentShaderInputOutput"], bgfxShader.fragmentShaderInputOutput);
				}
				if (JsonObjectFn::has(shader, "samplers"))
				{
					parseBgfxSamplers(shader["samplers"], bgfxShader);
				}

				DynamicString key(ta);
				key = begin->pair.first;

				RESOURCE_COMPILER_ASSERT(!MapFn::has(bgfxShadersMap, key)
					, compileOptions
					, "Bgfx shader redefined: '%s'"
					, key.getCStr()
					);
				MapFn::set(bgfxShadersMap, key, bgfxShader);
			}
		}

		void parseBgfxSamplers(const char* json, BgfxShader& bgfxShader)
		{
			TempAllocator4096 ta;
			JsonObject bgfxSamplers(ta);
			JsonRFn::parseObject(json, bgfxSamplers);

			auto begin = JsonObjectFn::begin(bgfxSamplers);
			auto end = JsonObjectFn::end(bgfxSamplers);
			for (; begin != end; ++begin)
			{
				JsonObject sampler(ta);
				JsonRFn::parseObject(begin->pair.second, sampler);

				DynamicString samplerState(ta);
				JsonRFn::parseString(sampler["samplerState"], samplerState);

				RESOURCE_COMPILER_ASSERT(MapFn::has(samplerStatesMap, samplerState)
					, compileOptions
					, "Unknown sampler state: '%s'"
					, samplerState.getCStr()
					);

				DynamicString key(ta);
				key = begin->pair.first;

				RESOURCE_COMPILER_ASSERT(!MapFn::has(bgfxShader.samplersMap, key)
					, compileOptions
					, "Bgfx sampler redefined: '%s'"
					, key.getCStr()
					);
				MapFn::set(bgfxShader.samplersMap, key, samplerState);
			}
		}

		void parseShaders(const char* json)
		{
			TempAllocator4096 ta;
			JsonObject shaders(ta);
			JsonRFn::parseObject(json, shaders);

			auto begin = JsonObjectFn::begin(shaders);
			auto end = JsonObjectFn::end(shaders);
			for (; begin != end; ++begin)
			{
				JsonObject jsonObject(ta);
				JsonRFn::parseObject(begin->pair.second, jsonObject);

				ShaderPermutation shader(getDefaultAllocator());
				JsonRFn::parseString(jsonObject["bgfxShader"], shader.bgfxShader);
				JsonRFn::parseString(jsonObject["renderState"], shader.renderState);

				DynamicString key(ta);
				key = begin->pair.first;

				RESOURCE_COMPILER_ASSERT(!MapFn::has(shaderPermutationsMap, key)
					, compileOptions
					, "Shader redefined: '%s'"
					, key.getCStr()
					);
				MapFn::set(shaderPermutationsMap, key, shader);
			}
		}

		void parseStaticCompile(const char* json)
		{
			TempAllocator4096 ta;
			JsonArray staticCompileJsonArray(ta);
			JsonRFn::parseArray(json, staticCompileJsonArray);

			for (uint32_t i = 0; i < ArrayFn::getCount(staticCompileJsonArray); ++i)
			{
				JsonObject jsonObject(ta);
				JsonRFn::parseObject(staticCompileJsonArray[i], jsonObject);

				StaticCompile staticCompile(getDefaultAllocator());
				JsonRFn::parseString(jsonObject["shader"], staticCompile.shaderName);

				JsonArray defines(ta);
				JsonRFn::parseArray(jsonObject["defines"], defines);
				for (uint32_t i = 0; i < ArrayFn::getCount(defines); ++i)
				{
					DynamicString define(ta);
					JsonRFn::parseString(defines[i], define);
					VectorFn::pushBack(staticCompile.defines, define);
				}

				VectorFn::pushBack(staticCompileList, staticCompile);
			}
		}

		void deleteTempFiles()
		{
			const char* vertexShaderSourcePath = this->vertexShaderSourcePath.getCStr();
			const char* fragmentShaderSourcePath = this->fragmentShaderSourcePath.getCStr();
			const char* varyingPath = this->varyingPath.getCStr();
			const char* vertexShaderCompiledPath = this->vertexShaderCompiledPath.getCStr();
			const char* fragmentShaderCompiledPath = this->fragmentShaderCompiledPath.getCStr();

			if (compileOptions.doesFileExist(vertexShaderSourcePath))
			{
				compileOptions.deleteFile(vertexShaderSourcePath);
			}

			if (compileOptions.doesFileExist(fragmentShaderSourcePath))
			{
				compileOptions.deleteFile(fragmentShaderSourcePath);
			}

			if (compileOptions.doesFileExist(varyingPath))
			{
				compileOptions.deleteFile(varyingPath);
			}

			if (compileOptions.doesFileExist(vertexShaderCompiledPath))
			{
				compileOptions.deleteFile(vertexShaderCompiledPath);
			}

			if (compileOptions.doesFileExist(fragmentShaderCompiledPath))
			{
				compileOptions.deleteFile(fragmentShaderCompiledPath);
			}
		}

		void compile()
		{
			compileOptions.write(RESOURCE_VERSION_SHADER);
			compileOptions.write(VectorFn::getCount(staticCompileList));

			for (uint32_t i = 0; i < VectorFn::getCount(staticCompileList); ++i)
			{
				const StaticCompile& staticCompile = staticCompileList[i];
				const DynamicString& shader = staticCompile.shaderName;
				const Vector<DynamicString>& defines = staticCompile.defines;

				TempAllocator1024 ta;
				DynamicString shaderNameStr(ta);
				shaderNameStr = shader;
				for (uint32_t i = 0; i < VectorFn::getCount(defines); ++i)
				{
					shaderNameStr += "+";
					shaderNameStr += defines[i];
				}
				const StringId32 shaderNameHash(shaderNameStr.getCStr());

				RESOURCE_COMPILER_ASSERT(MapFn::has(shaderPermutationsMap, staticCompile.shaderName)
					, compileOptions
					, "Unknown shader: '%s'"
					, shader.getCStr()
					);
				const ShaderPermutation& shaderPermutation = shaderPermutationsMap[shader];
				const DynamicString& bgfxShader = shaderPermutation.bgfxShader;
				const DynamicString& renderStateName = shaderPermutation.renderState;

				RESOURCE_COMPILER_ASSERT(MapFn::has(bgfxShadersMap, shaderPermutation.bgfxShader)
					, compileOptions
					, "Unknown bgfx shader: '%s'"
					, bgfxShader.getCStr()
					);
				RESOURCE_COMPILER_ASSERT(MapFn::has(renderStatesMap, shaderPermutation.renderState)
					, compileOptions
					, "Unknown render state: '%s'"
					, renderStateName.getCStr()
					);

				const RenderState& renderState = renderStatesMap[renderStateName];

				compileOptions.write(shaderNameHash.id); // Shader name
				compileOptions.write(renderState.encode()); // Render state
				compile(bgfxShader.getCStr(), defines); // Shader code
			}
		}

		void compileSamplerStates(const char* bgfxShader)
		{
			TempAllocator512 ta;
			DynamicString key(ta);
			key = bgfxShader;
			const BgfxShader& shader = bgfxShadersMap[key];

			compileOptions.write(MapFn::getCount(shader.samplersMap));

			auto begin = MapFn::begin(shader.samplersMap);
			auto end = MapFn::end(shader.samplersMap);
			for (; begin != end; ++begin)
			{
				const DynamicString& name = begin->pair.first;
				const DynamicString& samplerStateName = begin->pair.second;
				const SamplerState& samplerState = samplerStatesMap[samplerStateName];

				compileOptions.write(name.getStringId());
				compileOptions.write(samplerState.encode());
			}
		}

		void compile(const char* bgfxShader, const Vector<DynamicString>& defines)
		{
			TempAllocator512 taa;
			DynamicString key(taa);
			key = bgfxShader;
			const BgfxShader& shader = bgfxShadersMap[key];

			DynamicString includedCode(getDefaultAllocator());
			if (!(shader.includes == ""))
			{
				const BgfxShader& included = bgfxShadersMap[shader.includes];
				includedCode = included.code;
			}

			StringStream vertexShaderCode(getDefaultAllocator());
			StringStream fragmentShaderCode(getDefaultAllocator());
			vertexShaderCode << shader.vertexShaderInputOutput.getCStr();
			for (uint32_t i = 0; i < VectorFn::getCount(defines); ++i)
			{
				vertexShaderCode << "#define " << defines[i].getCStr() << "\n";
			}
			vertexShaderCode << includedCode.getCStr();
			vertexShaderCode << shader.code.getCStr();
			vertexShaderCode << shader.vertexShaderCode.getCStr();
			fragmentShaderCode << shader.fragmentShaderInputOutput.getCStr();
			for (uint32_t i = 0; i < VectorFn::getCount(defines); ++i)
			{
				fragmentShaderCode << "#define " << defines[i].getCStr() << "\n";
			}
			fragmentShaderCode << includedCode.getCStr();
			fragmentShaderCode << shader.code.getCStr();
			fragmentShaderCode << shader.fragmentShaderCode.getCStr();

			compileOptions.writeTemporary(vertexShaderSourcePath.getCStr(), vertexShaderCode);
			compileOptions.writeTemporary(fragmentShaderSourcePath.getCStr(), fragmentShaderCode);
			compileOptions.writeTemporary(varyingPath.getCStr(), shader.varying.getCStr(), shader.varying.getLength());

			TempAllocator4096 ta;
			StringStream output(ta);

			int exitCode = runExternalCompiler(vertexShaderSourcePath.getCStr()
				, vertexShaderCompiledPath.getCStr()
				, varyingPath.getCStr()
				, "vertex"
				, compileOptions.getPlatform()
				, output
				);
			if (exitCode != 0)
			{
				deleteTempFiles();
				RESOURCE_COMPILER_ASSERT(false
					, compileOptions
					, "Failed to compile vertex shader:\n%s"
					, StringStreamFn::getCStr(output)
					);
			}

			ArrayFn::clear(output);
			exitCode = runExternalCompiler(fragmentShaderSourcePath.getCStr()
				, fragmentShaderCompiledPath.getCStr()
				, varyingPath.getCStr()
				, "fragment"
				, compileOptions.getPlatform()
				, output
				);
			if (exitCode != 0)
			{
				deleteTempFiles();
				RESOURCE_COMPILER_ASSERT(false
					, compileOptions
					, "Failed to compile fragment shader:\n%s"
					, StringStreamFn::getCStr(output)
					);
			}

			Buffer tempVertexShader = compileOptions.readTemporary(vertexShaderCompiledPath.getCStr());
			Buffer tempFragmentShader = compileOptions.readTemporary(fragmentShaderCompiledPath.getCStr());

			deleteTempFiles();

			// Write
			compileOptions.write(ArrayFn::getCount(tempVertexShader));
			compileOptions.write(tempVertexShader);
			compileOptions.write(ArrayFn::getCount(tempFragmentShader));
			compileOptions.write(tempFragmentShader);
		}

		CompileOptions& compileOptions;
		Map<DynamicString, RenderState> renderStatesMap;
		Map<DynamicString, SamplerState> samplerStatesMap;
		Map<DynamicString, BgfxShader> bgfxShadersMap;
		Map<DynamicString, ShaderPermutation> shaderPermutationsMap;
		Vector<StaticCompile> staticCompileList;

		DynamicString vertexShaderSourcePath;
		DynamicString fragmentShaderSourcePath;
		DynamicString varyingPath;
		DynamicString vertexShaderCompiledPath;
		DynamicString fragmentShaderCompiledPath;
	};

	void compile(const char* path, CompileOptions& compileOptions)
	{
		ShaderCompiler shaderCompiler(compileOptions);
		shaderCompiler.parse(path);
		shaderCompiler.compile();
	}

	void* load(File& file, Allocator& a)
	{
		return getDevice()->getShaderManager()->load(file, a);
	}

	void online(StringId64 id, ResourceManager& resourceManager)
	{
		getDevice()->getShaderManager()->online(id, resourceManager);
	}

	void offline(StringId64 id, ResourceManager& resourceManager)
	{
		getDevice()->getShaderManager()->offline(id, resourceManager);
	}

	void unload(Allocator& a, void* resource)
	{
		getDevice()->getShaderManager()->unload(a, resource);
	}
} // namespace ShaderResourceInternalFn

} // namespace Rio
// Copyright (c) 2016 Volodymyr Syvochka

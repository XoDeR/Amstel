// Copyright (c) 2016 Volodymyr Syvochka
#include "Config.h"

#if RIO_BUILD_UNIT_TESTS

#include "Core/Base/Macros.h"
#include "Core/Base/Murmur.h"
#include "Core/Base/CommandLine.h"
#include "Core/Base/Guid.h"

#include "Core/Memory/Memory.h"
#include "Core/Memory/TempAllocator.h"

#include "Core/Containers/Array.h"
#include "Core/Containers/Vector.h"
#include "Core/Containers/HashMap.h"

#include "Core/Math/MathUtils.h"
#include "Core/Math/Matrix3x3.h"
#include "Core/Math/Matrix4x4.h"
#include "Core/Math/Vector2.h"
#include "Core/Math/Vector3.h"
#include "Core/Math/Vector4.h"
#include "Core/Math/Aabb.h"
#include "Core/Math/Color4.h"
#include "Core/Math/Quaternion.h"
#include "Core/Math/Sphere.h"

#include "Core/Strings/StringId.h"
#include "Core/Strings/StringUtils.h"
#include "Core/Strings/DynamicString.h"

#include "Core/FileSystem/Path.h"

#include "Core/Json/Json.h"
#include "Core/Json/JsonR.h"

#define ENSURE(condition) do { if (!(condition)) {\
	printf("Assertion failed: '%s' in %s:%d\n\n", #condition, __FILE__, __LINE__); abort(); }} while (0)

namespace Rio
{

static void testMemory()
{
	MemoryGlobalFn::init();
	Allocator& a = getDefaultAllocator();

	void* p = a.allocate(64);
	ENSURE(a.getAllocatedSize(p) >= 64);
	a.deallocate(p);

	MemoryGlobalFn::shutdown();
}

static void testArray()
{
	MemoryGlobalFn::init();
	Allocator& a = getDefaultAllocator();
	{
		Array<int> v(a);

		ENSURE(ArrayFn::getCount(v) == 0);
		ArrayFn::pushBack(v, 1);
		ENSURE(ArrayFn::getCount(v) == 1);
		ENSURE(v[0] == 1);
	}
	MemoryGlobalFn::shutdown();
}

static void testVector()
{
	MemoryGlobalFn::init();
	Allocator& a = getDefaultAllocator();
	{
		Vector<int> v(a);

		ENSURE(VectorFn::getCount(v) == 0);
		VectorFn::pushBack(v, 1);
		ENSURE(VectorFn::getCount(v) == 1);
		ENSURE(v[0] == 1);
	}
	MemoryGlobalFn::shutdown();
}

static void testHashMap()
{
	MemoryGlobalFn::init();
	Allocator& a = getDefaultAllocator();
	{
		HashMap<int32_t, int32_t> m(a);

		ENSURE(HashMapFn::getCount(m) == 0);
		ENSURE(HashMapFn::get(m, 0, 77) == 77);
		ENSURE(!HashMapFn::has(m, 10));

		for (int32_t i = 0; i < 100; ++i)
		{
			HashMapFn::set(m, i, i*i);
		}
		for (int32_t i = 0; i < 100; ++i)
		{
			ENSURE(HashMapFn::get(m, i, 0) == i*i);
		}

		HashMapFn::remove(m, 20);
		ENSURE(!HashMapFn::has(m, 20));

		HashMapFn::clear(m);

		for (int32_t i = 0; i < 100; ++i)
		{
			ENSURE(!HashMapFn::has(m, i));
		}
	}
	MemoryGlobalFn::shutdown();
}

static void testVector2()
{
	{
		const Vector2 a = createVector2(1.5f,  4.0f);
		const Vector2 b = createVector2(3.0f, -1.5f);
		const Vector2 c = a - b;
		ENSURE(getAreFloatsEqual(c.x, -1.5f, 0.0001f));
		ENSURE(getAreFloatsEqual(c.y, 5.5f, 0.0001f));
	}
	{
		const Vector2 a = createVector2(1.2f,  4.2f);
		const Vector2 b = createVector2(2.7f, -1.9f);
		const Vector2 c = a + b;
		ENSURE(getAreFloatsEqual(c.x, 3.9f, 0.0001f));
		ENSURE(getAreFloatsEqual(c.y, 2.3f, 0.0001f));
	}
	{
		const Vector2 a = createVector2(1.2f,  4.2f);
		const Vector2 b = a * 2.0f;
		ENSURE(getAreFloatsEqual(b.x, 2.4f, 0.0001f));
		ENSURE(getAreFloatsEqual(b.y, 8.4f, 0.0001f));
	}
	{
		const Vector2 a = createVector2(1.2f, 4.2f);
		const Vector2 b = createVector2(2.7f, -1.9f);
		const float c = dot(a, b);
		ENSURE(getAreFloatsEqual(c, -4.74f, 0.0001f));
	}
	{
		const Vector2 a = createVector2(1.2f,  4.2f);
		const float c = getLengthSquared(a);
		ENSURE(getAreFloatsEqual(c, 19.08f, 0.0001f));
	}
	{
		const Vector2 a = createVector2(1.2f,  4.2f);
		const float c = getLength(a);
		ENSURE(getAreFloatsEqual(c,  4.36806f, 0.0001f));
	}
	{
		Vector2 a = createVector2(1.2f,  4.2f);
		normalize(a);
		ENSURE(getAreFloatsEqual(getLength(a), 1.0f, 0.00001f));
	}
	{
		const Vector2 a = createVector2(1.2f,  4.2f);
		const Vector2 b = createVector2(2.7f, -1.9f);
		const float c = getDistanceSquared(a, b);
		ENSURE(getAreFloatsEqual(c, 39.46f, 0.00001f));
	}
	{
		const Vector2 a = createVector2(1.2f,  4.2f);
		const Vector2 b = createVector2(2.7f, -1.9f);
		const float c = getDistance(a, b);
		ENSURE(getAreFloatsEqual(c, 6.28171f, 0.00001f));
	}
	{
		const Vector2 a = createVector2(1.2f,  4.2f);
		const Vector2 b = createVector2(2.7f, -1.9f);
		const Vector2 c = max(a, b);
		ENSURE(getAreFloatsEqual(c.x,  2.7f, 0.00001f));
		ENSURE(getAreFloatsEqual(c.y,  4.2f, 0.00001f));
	}
	{
		const Vector2 a = createVector2(1.2f,  4.2f);
		const Vector2 b = createVector2(2.7f, -1.9f);
		const Vector2 c = min(a, b);
		ENSURE(getAreFloatsEqual(c.x,  1.2f, 0.00001f));
		ENSURE(getAreFloatsEqual(c.y, -1.9f, 0.00001f));
	}
}

static void testVector3()
{
	{
		const Vector3 a = createVector3(1.2f,  4.2f, -2.3f);
		const Vector3 b = createVector3(2.7f, -1.9f, -4.1f);
		const Vector3 c = a - b;
		ENSURE(getAreFloatsEqual(c.x, -1.5f, 0.0001f));
		ENSURE(getAreFloatsEqual(c.y,  6.1f, 0.0001f));
		ENSURE(getAreFloatsEqual(c.z,  1.8f, 0.0001f));
	}
	{
		const Vector3 a = createVector3(1.2f,  4.2f, -2.3f);
		const Vector3 b = createVector3(2.7f, -1.9f, -4.1f);
		const Vector3 c = a + b;
		ENSURE(getAreFloatsEqual(c.x,  3.9f, 0.0001f));
		ENSURE(getAreFloatsEqual(c.y,  2.3f, 0.0001f));
		ENSURE(getAreFloatsEqual(c.z, -6.4f, 0.0001f));
	}
	{
		const Vector3 a = createVector3(1.2f,  4.2f, -2.3f);
		const Vector3 b = a * 2.0f;
		ENSURE(getAreFloatsEqual(b.x, 2.4f, 0.0001f));
		ENSURE(getAreFloatsEqual(b.y, 8.4f, 0.0001f));
		ENSURE(getAreFloatsEqual(b.z, -4.6f, 0.0001f));
	}
	{
		const Vector3 a = createVector3(1.2f,  4.2f, -2.3f);
		const Vector3 b = createVector3(2.7f, -1.9f, -4.1f);
		const float c = dot(a, b);
		ENSURE(getAreFloatsEqual(c, 4.69f, 0.0001f));
	}
	{
		const Vector3 a = createVector3(1.2f,  4.2f, -2.3f);
		const Vector3 b = createVector3(2.7f, -1.9f, -4.1f);
		const Vector3 c = cross(a, b);
		ENSURE(getAreFloatsEqual(c.x, -21.59f, 0.0001f));
		ENSURE(getAreFloatsEqual(c.y,  -1.29f, 0.0001f));
		ENSURE(getAreFloatsEqual(c.z, -13.62f, 0.0001f));
	}
	{
		const Vector3 a = createVector3(1.2f,  4.2f, -2.3f);
		const float c = getLengthSquared(a);
		ENSURE(getAreFloatsEqual(c, 24.37f, 0.0001f));
	}
	{
		const Vector3 a = createVector3(1.2f,  4.2f, -2.3f);
		const float c = getLength(a);
		ENSURE(getAreFloatsEqual(c,  4.93659f, 0.0001f));
	}
	{
		Vector3 a = createVector3(1.2f,  4.2f, -2.3f);
		normalize(a);
		ENSURE(getAreFloatsEqual(getLength(a), 1.0f, 0.00001f));
	}
	{
		const Vector3 a = createVector3(1.2f,  4.2f, -2.3f);
		const Vector3 b = createVector3(2.7f, -1.9f, -4.1f);
		const float c = getDistanceSquared(a, b);
		ENSURE(getAreFloatsEqual(c, 42.70f, 0.00001f));
	}
	{
		const Vector3 a = createVector3(1.2f,  4.2f, -2.3f);
		const Vector3 b = createVector3(2.7f, -1.9f, -4.1f);
		const float c = getDistance(a, b);
		ENSURE(getAreFloatsEqual(c, 6.53452f, 0.00001f));
	}
	{
		const Vector3 a = createVector3(1.2f,  4.2f, -2.3f);
		const Vector3 b = createVector3(2.7f, -1.9f, -4.1f);
		const Vector3 c = max(a, b);
		ENSURE(getAreFloatsEqual(c.x,  2.7f, 0.00001f));
		ENSURE(getAreFloatsEqual(c.y,  4.2f, 0.00001f));
		ENSURE(getAreFloatsEqual(c.z, -2.3f, 0.00001f));
	}
	{
		const Vector3 a = createVector3(1.2f,  4.2f, -2.3f);
		const Vector3 b = createVector3(2.7f, -1.9f, -4.1f);
		const Vector3 c = min(a, b);
		ENSURE(getAreFloatsEqual(c.x,  1.2f, 0.00001f));
		ENSURE(getAreFloatsEqual(c.y, -1.9f, 0.00001f));
		ENSURE(getAreFloatsEqual(c.z, -4.1f, 0.00001f));
	}
}

static void testVector4()
{
	{
		const Vector4 a = createVector4(1.2f,  4.2f, -2.3f, 5.5f);
		const Vector4 b = createVector4(2.7f, -1.9f, -4.1f, 1.0f);
		const Vector4 c = a - b;
		ENSURE(getAreFloatsEqual(c.x, -1.5f, 0.0001f));
		ENSURE(getAreFloatsEqual(c.y, 6.1f, 0.0001f));
		ENSURE(getAreFloatsEqual(c.z, 1.8f, 0.0001f));
		ENSURE(getAreFloatsEqual(c.w, 4.5f, 0.0001f));
	}
	{
		const Vector4 a = createVector4(1.2f,  4.2f, -2.3f, 5.5f);
		const Vector4 b = createVector4(2.7f, -1.9f, -4.1f, 1.0f);
		const Vector4 c = a + b;
		ENSURE(getAreFloatsEqual(c.x, 3.9f, 0.0001f));
		ENSURE(getAreFloatsEqual(c.y, 2.3f, 0.0001f));
		ENSURE(getAreFloatsEqual(c.z, -6.4f, 0.0001f));
		ENSURE(getAreFloatsEqual(c.w, 6.5f, 0.0001f));
	}
	{
		const Vector4 a = createVector4(1.2f, 4.2f, -2.3f, 1.5f);
		const Vector4 b = a * 2.0f;
		ENSURE(getAreFloatsEqual(b.x, 2.4f, 0.0001f));
		ENSURE(getAreFloatsEqual(b.y, 8.4f, 0.0001f));
		ENSURE(getAreFloatsEqual(b.z, -4.6f, 0.0001f));
		ENSURE(getAreFloatsEqual(b.w, 3.0f, 0.0001f));
	}
	{
		const Vector4 a = createVector4(1.2f, 4.2f, -2.3f, 5.5f);
		const Vector4 b = createVector4(2.7f, -1.9f, -4.1f, 1.0f);
		const float c = dot(a, b);
		ENSURE(getAreFloatsEqual(c,  10.19f, 0.0001f));
	}
	{
		const Vector4 a = createVector4(1.2f, 4.2f, -2.3f, 5.5f);
		const float c = getLengthSquared(a);
		ENSURE(getAreFloatsEqual(c, 54.62f, 0.0001f));
	}
	{
		const Vector4 a = createVector4(1.2f, 4.2f, -2.3f, 5.5f);
		const float c = getLength(a);
		ENSURE(getAreFloatsEqual(c,  7.39053f, 0.0001f));
	}
	{
		Vector4 a = createVector4(1.2f,  4.2f, -2.3f, 5.5f);
		normalize(a);
		ENSURE(getAreFloatsEqual(getLength(a), 1.0f, 0.00001f));
	}
	{
		const Vector4 a = createVector4(1.2f, 4.2f, -2.3f, 5.5f);
		const Vector4 b = createVector4(2.7f, -1.9f, -4.1f, 1.0f);
		const float c = getDistanceSquared(a, b);
		ENSURE(getAreFloatsEqual(c, 62.95f, 0.00001f));
	}
	{
		const Vector4 a = createVector4(1.2f, 4.2f, -2.3f, 5.5f);
		const Vector4 b = createVector4(2.7f, -1.9f, -4.1f, 1.0f);
		const float c = getDistance(a, b);
		ENSURE(getAreFloatsEqual(c, 7.93410f, 0.00001f));
	}
	{
		const Vector4 a = createVector4(1.2f, 4.2f, -2.3f, 5.5f);
		const Vector4 b = createVector4(2.7f, -1.9f, -4.1f, 1.0f);
		const Vector4 c = max(a, b);
		ENSURE(getAreFloatsEqual(c.x, 2.7f, 0.00001f));
		ENSURE(getAreFloatsEqual(c.y, 4.2f, 0.00001f));
		ENSURE(getAreFloatsEqual(c.z, -2.3f, 0.00001f));
		ENSURE(getAreFloatsEqual(c.w, 5.5f, 0.00001f));
	}
	{
		const Vector4 a = createVector4(1.2f, 4.2f, -2.3f, 5.5f);
		const Vector4 b = createVector4(2.7f, -1.9f, -4.1f, 1.0f);
		const Vector4 c = min(a, b);
		ENSURE(getAreFloatsEqual(c.x,  1.2f, 0.00001f));
		ENSURE(getAreFloatsEqual(c.y, -1.9f, 0.00001f));
		ENSURE(getAreFloatsEqual(c.z, -4.1f, 0.00001f));
		ENSURE(getAreFloatsEqual(c.w, 1.0f, 0.00001f));
	}
}

static void testQuaternion()
{
	{
		const Quaternion a = createQuaternion(0.0f, 0.0f, 0.0f, 1.0f);
		ENSURE(getAreFloatsEqual(a.x, 0.0f, 0.00001f));
		ENSURE(getAreFloatsEqual(a.y, 0.0f, 0.00001f));
		ENSURE(getAreFloatsEqual(a.z, 0.0f, 0.00001f));
		ENSURE(getAreFloatsEqual(a.w, 1.0f, 0.00001f));
	}
}

static void testColor4()
{
	{
		const Color4 a = createColor4(1.3f, 2.6f, 0.2f, 0.6f);
		ENSURE(getAreFloatsEqual(a.x, 1.3f, 0.00001f));
		ENSURE(getAreFloatsEqual(a.y, 2.6f, 0.00001f));
		ENSURE(getAreFloatsEqual(a.z, 0.2f, 0.00001f));
		ENSURE(getAreFloatsEqual(a.w, 0.6f, 0.00001f));
	}
	{
		const Color4 a = createColorRgba(63, 231, 12, 98);
		ENSURE(getAreFloatsEqual(a.x, 0.24705f, 0.00001f));
		ENSURE(getAreFloatsEqual(a.y, 0.90588f, 0.00001f));
		ENSURE(getAreFloatsEqual(a.z, 0.04705f, 0.00001f));
		ENSURE(getAreFloatsEqual(a.w, 0.38431f, 0.00001f));
	}
	{
		const Color4 a = createColorRgb(63, 231, 12);
		ENSURE(getAreFloatsEqual(a.x, 0.24705f, 0.00001f));
		ENSURE(getAreFloatsEqual(a.y, 0.90588f, 0.00001f));
		ENSURE(getAreFloatsEqual(a.z, 0.04705f, 0.00001f));
		ENSURE(getAreFloatsEqual(a.w, 1.0f    , 0.00001f));
	}
	{
		const Color4 a = createColorRgba(0x3fe70c62);
		ENSURE(getAreFloatsEqual(a.x, 0.24705f, 0.00001f));
		ENSURE(getAreFloatsEqual(a.y, 0.90588f, 0.00001f));
		ENSURE(getAreFloatsEqual(a.z, 0.04705f, 0.00001f));
		ENSURE(getAreFloatsEqual(a.w, 0.38431f, 0.00001f));
	}
	{
		const Color4 a = createColorRgba(63, 231, 12, 98);

		const uint32_t rgba = getRgba(a);
		ENSURE(rgba == 0x3fe70c62);

		const uint32_t rgb = getRgb(a);
		ENSURE(rgb == 0x3fe70cff);

		const uint32_t bgr = getBgr(a);
		ENSURE(bgr == 0xff0ce73f);

		const uint32_t abgr = getAbgr(a);
		ENSURE(abgr == 0x620ce73f);
	}
}

static void testMatrix3x3()
{
	{
		const Matrix3x3 a = createMatrix3x3(1.2f, -2.3f, 5.1f
			,  2.2f, -5.1f,  1.1f
			,  3.2f,  3.3f, -3.8f
			);
		const Matrix3x3 b = createMatrix3x3(3.2f, 4.8f, 6.0f
			, -1.6f, -7.1f, -2.4f
			, -3.1f, -2.2f,  8.9f
			);
		const Matrix3x3 c = a + b;
		ENSURE(getAreFloatsEqual(c.x.x,   4.4f, 0.00001f));
		ENSURE(getAreFloatsEqual(c.x.y,   2.5f, 0.00001f));
		ENSURE(getAreFloatsEqual(c.x.z,  11.1f, 0.00001f));
		ENSURE(getAreFloatsEqual(c.y.x,   0.6f, 0.00001f));
		ENSURE(getAreFloatsEqual(c.y.y, -12.2f, 0.00001f));
		ENSURE(getAreFloatsEqual(c.y.z,  -1.3f, 0.00001f));
		ENSURE(getAreFloatsEqual(c.z.x,   0.1f, 0.00001f));
		ENSURE(getAreFloatsEqual(c.z.y,   1.1f, 0.00001f));
		ENSURE(getAreFloatsEqual(c.z.z,   5.1f, 0.00001f));
	}
	{
		const Matrix3x3 a = createMatrix3x3(1.2f, -2.3f, 5.1f
			,  2.2f, -5.1f,  1.1f
			,  3.2f,  3.3f, -3.8f
			);
		const Matrix3x3 b = createMatrix3x3(3.2f, 4.8f, 6.0f
			, -1.6f, -7.1f, -2.4f
			, -3.1f, -2.2f,  8.9f
			);
		const Matrix3x3 c = a - b;
		ENSURE(getAreFloatsEqual(c.x.x,  -2.0f, 0.00001f));
		ENSURE(getAreFloatsEqual(c.x.y,  -7.1f, 0.00001f));
		ENSURE(getAreFloatsEqual(c.x.z,  -0.9f, 0.00001f));
		ENSURE(getAreFloatsEqual(c.y.x,   3.8f, 0.00001f));
		ENSURE(getAreFloatsEqual(c.y.y,   2.0f, 0.00001f));
		ENSURE(getAreFloatsEqual(c.y.z,   3.5f, 0.00001f));
		ENSURE(getAreFloatsEqual(c.z.x,   6.3f, 0.00001f));
		ENSURE(getAreFloatsEqual(c.z.y,   5.5f, 0.00001f));
		ENSURE(getAreFloatsEqual(c.z.z, -12.7f, 0.00001f));
	}
	{
		const Matrix3x3 a = createMatrix3x3(1.2f, -2.3f, 5.1f
			,  2.2f, -5.1f,  1.1f
			,  3.2f,  3.3f, -3.8f
			);
		const Matrix3x3 b = createMatrix3x3(3.2f, 4.8f, 6.0f
			, -1.6f, -7.1f, -2.4f
			, -3.1f, -2.2f,  8.9f
			);
		const Matrix3x3 c = a * b;
		ENSURE(getAreFloatsEqual(c.x.x,  -8.29f, 0.00001f));
		ENSURE(getAreFloatsEqual(c.x.y,  10.87f, 0.00001f));
		ENSURE(getAreFloatsEqual(c.x.z,  58.11f, 0.00001f));
		ENSURE(getAreFloatsEqual(c.y.x,  11.79f, 0.00001f));
		ENSURE(getAreFloatsEqual(c.y.y,  44.35f, 0.00001f));
		ENSURE(getAreFloatsEqual(c.y.z,  35.23f, 0.00001f));
		ENSURE(getAreFloatsEqual(c.z.x,  16.74f, 0.00001f));
		ENSURE(getAreFloatsEqual(c.z.y,   0.29f, 0.00001f));
		ENSURE(getAreFloatsEqual(c.z.z, -22.54f, 0.00001f));
	}
	{
		const Matrix3x3 a = createMatrix3x3(1.2f, -2.3f, 5.1f
			,  2.2f, -5.1f,  1.1f
			,  3.2f,  3.3f, -3.8f
			);
		const float det = getDeterminant(a);
		ENSURE(getAreFloatsEqual(det, 111.834f, 0.00001f));
	}
	{
		const Matrix3x3 a = createMatrix3x3(1.2f, -2.3f, 5.1f
			,  2.2f, -5.1f,  1.1f
			,  3.2f,  3.3f, -3.8f
			);
		const Matrix3x3 b = getInverted(a);
		ENSURE(getAreFloatsEqual(b.x.x,  0.140833f, 0.00001f));
		ENSURE(getAreFloatsEqual(b.x.y,  0.072339f, 0.00001f));
		ENSURE(getAreFloatsEqual(b.x.z,  0.209954f, 0.00001f));
		ENSURE(getAreFloatsEqual(b.y.x,  0.106228f, 0.00001f));
		ENSURE(getAreFloatsEqual(b.y.y, -0.186705f, 0.00001f));
		ENSURE(getAreFloatsEqual(b.y.z,  0.088524f, 0.00001f));
		ENSURE(getAreFloatsEqual(b.z.x,  0.210848f, 0.00001f));
		ENSURE(getAreFloatsEqual(b.z.y, -0.101221f, 0.00001f));
		ENSURE(getAreFloatsEqual(b.z.z, -0.009478f, 0.00001f));
	}
	{
		const Matrix3x3 a = createMatrix3x3(1.2f, -2.3f, 5.1f
			,  2.2f, -5.1f,  1.1f
			,  3.2f,  3.3f, -3.8f
			);
		const Matrix3x3 b = getTransposed(a);
		ENSURE(getAreFloatsEqual(b.x.x,  1.2f, 0.00001f));
		ENSURE(getAreFloatsEqual(b.x.y,  2.2f, 0.00001f));
		ENSURE(getAreFloatsEqual(b.x.z,  3.2f, 0.00001f));
		ENSURE(getAreFloatsEqual(b.y.x, -2.3f, 0.00001f));
		ENSURE(getAreFloatsEqual(b.y.y, -5.1f, 0.00001f));
		ENSURE(getAreFloatsEqual(b.y.z,  3.3f, 0.00001f));
		ENSURE(getAreFloatsEqual(b.z.x,  5.1f, 0.00001f));
		ENSURE(getAreFloatsEqual(b.z.y,  1.1f, 0.00001f));
		ENSURE(getAreFloatsEqual(b.z.z, -3.8f, 0.00001f));
	}
}

static void testMatrix4x4()
{
	{
		const Matrix4x4 a = createMatrix4x4(1.2f, -2.3f, 5.1f, -1.2f
			,  2.2f, -5.1f,  1.1f, -7.4f
			,  3.2f,  3.3f, -3.8f, -9.2f
			, -6.8f, -2.9f,  1.0f,  4.9f
			);
		const Matrix4x4 b = createMatrix4x4(3.2f, 4.8f, 6.0f, 5.3f
			, -1.6f, -7.1f, -2.4f, -6.2f
			, -3.1f, -2.2f,  8.9f,  8.3f
			,  3.8f,  9.1f, -3.1f, -7.1f
			);
		const Matrix4x4 c = a + b;
		ENSURE(getAreFloatsEqual(c.x.x,   4.4f, 0.00001f));
		ENSURE(getAreFloatsEqual(c.x.y,   2.5f, 0.00001f));
		ENSURE(getAreFloatsEqual(c.x.z,  11.1f, 0.00001f));
		ENSURE(getAreFloatsEqual(c.x.w,   4.1f, 0.00001f));
		ENSURE(getAreFloatsEqual(c.y.x,   0.6f, 0.00001f));
		ENSURE(getAreFloatsEqual(c.y.y, -12.2f, 0.00001f));
		ENSURE(getAreFloatsEqual(c.y.z,  -1.3f, 0.00001f));
		ENSURE(getAreFloatsEqual(c.y.w, -13.6f, 0.00001f));
		ENSURE(getAreFloatsEqual(c.z.x,   0.1f, 0.00001f));
		ENSURE(getAreFloatsEqual(c.z.y,   1.1f, 0.00001f));
		ENSURE(getAreFloatsEqual(c.z.z,   5.1f, 0.00001f));
		ENSURE(getAreFloatsEqual(c.z.w,  -0.9f, 0.00001f));
		ENSURE(getAreFloatsEqual(c.t.x,  -3.0f, 0.00001f));
		ENSURE(getAreFloatsEqual(c.t.y,   6.2f, 0.00001f));
		ENSURE(getAreFloatsEqual(c.t.z,  -2.1f, 0.00001f));
		ENSURE(getAreFloatsEqual(c.t.w,  -2.2f, 0.00001f));
	}
	{
		const Matrix4x4 a = createMatrix4x4(1.2f, -2.3f, 5.1f, -1.2f
			,  2.2f, -5.1f,  1.1f, -7.4f
			,  3.2f,  3.3f, -3.8f, -9.2f
			, -6.8f, -2.9f,  1.0f,  4.9f
			);
		const Matrix4x4 b = createMatrix4x4(3.2f, 4.8f, 6.0f, 5.3f
			, -1.6f, -7.1f, -2.4f, -6.2f
			, -3.1f, -2.2f,  8.9f,  8.3f
			,  3.8f,  9.1f, -3.1f, -7.1f
			);
		const Matrix4x4 c = a - b;
		ENSURE(getAreFloatsEqual(c.x.x,  -2.0f, 0.00001f));
		ENSURE(getAreFloatsEqual(c.x.y,  -7.1f, 0.00001f));
		ENSURE(getAreFloatsEqual(c.x.z,  -0.9f, 0.00001f));
		ENSURE(getAreFloatsEqual(c.x.w,  -6.5f, 0.00001f));
		ENSURE(getAreFloatsEqual(c.y.x,   3.8f, 0.00001f));
		ENSURE(getAreFloatsEqual(c.y.y,   2.0f, 0.00001f));
		ENSURE(getAreFloatsEqual(c.y.z,   3.5f, 0.00001f));
		ENSURE(getAreFloatsEqual(c.y.w,  -1.2f, 0.00001f));
		ENSURE(getAreFloatsEqual(c.z.x,   6.3f, 0.00001f));
		ENSURE(getAreFloatsEqual(c.z.y,   5.5f, 0.00001f));
		ENSURE(getAreFloatsEqual(c.z.z, -12.7f, 0.00001f));
		ENSURE(getAreFloatsEqual(c.z.w, -17.5f, 0.00001f));
		ENSURE(getAreFloatsEqual(c.t.x, -10.6f, 0.00001f));
		ENSURE(getAreFloatsEqual(c.t.y, -12.0f, 0.00001f));
		ENSURE(getAreFloatsEqual(c.t.z,   4.1f, 0.00001f));
		ENSURE(getAreFloatsEqual(c.t.w,  12.0f, 0.00001f));
	}
	{
		const Matrix4x4 a = createMatrix4x4(1.2f, -2.3f, 5.1f, -1.2f
			,  2.2f, -5.1f,  1.1f, -7.4f
			,  3.2f,  3.3f, -3.8f, -9.2f
			, -6.8f, -2.9f,  1.0f,  4.9f
			);
		const Matrix4x4 b = createMatrix4x4(3.2f, 4.8f, 6.0f, 5.3f
			, -1.6f, -7.1f, -2.4f, -6.2f
			, -3.1f, -2.2f,  8.9f,  8.3f
			,  3.8f,  9.1f, -3.1f, -7.1f
			);
		const Matrix4x4 c = a * b;
		ENSURE(getAreFloatsEqual(c.x.x, -12.85, 0.00001f));
		ENSURE(getAreFloatsEqual(c.x.y,  -0.05, 0.00001f));
		ENSURE(getAreFloatsEqual(c.x.z,  61.83, 0.00001f));
		ENSURE(getAreFloatsEqual(c.x.w,  71.47, 0.00001f));
		ENSURE(getAreFloatsEqual(c.y.x, -16.33, 0.00001f));
		ENSURE(getAreFloatsEqual(c.y.y, -22.99, 0.00001f));
		ENSURE(getAreFloatsEqual(c.y.z,  58.17, 0.00001f));
		ENSURE(getAreFloatsEqual(c.y.w, 104.95, 0.00001f));
		ENSURE(getAreFloatsEqual(c.z.x, -18.22, 0.00001f));
		ENSURE(getAreFloatsEqual(c.z.y, -83.43, 0.00001f));
		ENSURE(getAreFloatsEqual(c.z.z,   5.98, 0.00001f));
		ENSURE(getAreFloatsEqual(c.z.w,  30.28, 0.00001f));
		ENSURE(getAreFloatsEqual(c.t.x,  -1.60, 0.00001f));
		ENSURE(getAreFloatsEqual(c.t.y,  30.34, 0.00001f));
		ENSURE(getAreFloatsEqual(c.t.z, -40.13, 0.00001f));
		ENSURE(getAreFloatsEqual(c.t.w, -44.55, 0.00001f));
	}
	{
		const Matrix4x4 a = createMatrix4x4(1.2f, -2.3f, 5.1f, -1.2f
			,  2.2f, -5.1f,  1.1f, -7.4f
			,  3.2f,  3.3f, -3.8f, -9.2f
			, -6.8f, -2.9f,  1.0f,  4.9f
			);
		const float det = getDeterminant(a);
		ENSURE(getAreFloatsEqual(det, -1379.14453f, 0.00001f));
	}
	{
		const Matrix4x4 a = createMatrix4x4(1.2f, -2.3f, 5.1f, -1.2f
			,  2.2f, -5.1f,  1.1f, -7.4f
			,  3.2f,  3.3f, -3.8f, -9.2f
			, -6.8f, -2.9f,  1.0f,  4.9f
			);
		const Matrix4x4 b = getInverted(a);
		ENSURE(getAreFloatsEqual(b.x.x, -0.08464f, 0.00001f));
		ENSURE(getAreFloatsEqual(b.x.y,  0.06129f, 0.00001f));
		ENSURE(getAreFloatsEqual(b.x.z, -0.15210f, 0.00001f));
		ENSURE(getAreFloatsEqual(b.x.w, -0.21374f, 0.00001f));
		ENSURE(getAreFloatsEqual(b.y.x,  0.14384f, 0.00001f));
		ENSURE(getAreFloatsEqual(b.y.y, -0.18486f, 0.00001f));
		ENSURE(getAreFloatsEqual(b.y.z,  0.14892f, 0.00001f));
		ENSURE(getAreFloatsEqual(b.y.w,  0.03565f, 0.00001f));
		ENSURE(getAreFloatsEqual(b.z.x,  0.26073f, 0.00001f));
		ENSURE(getAreFloatsEqual(b.z.y, -0.09877f, 0.00001f));
		ENSURE(getAreFloatsEqual(b.z.z,  0.07063f, 0.00001f));
		ENSURE(getAreFloatsEqual(b.z.w,  0.04729f, 0.00001f));
		ENSURE(getAreFloatsEqual(b.t.x, -0.08553f, 0.00001f));
		ENSURE(getAreFloatsEqual(b.t.y, -0.00419f, 0.00001f));
		ENSURE(getAreFloatsEqual(b.t.z, -0.13735f, 0.00001f));
		ENSURE(getAreFloatsEqual(b.t.w, -0.08108f, 0.00001f));
	}
	{
		const Matrix4x4 a = createMatrix4x4(1.2f, -2.3f, 5.1f, -1.2f
			,  2.2f, -5.1f,  1.1f, -7.4f
			,  3.2f,  3.3f, -3.8f, -9.2f
			, -6.8f, -2.9f,  1.0f,  4.9f
			);
		const Matrix4x4 b = getTransposed(a);
		ENSURE(getAreFloatsEqual(b.x.x,  1.2f, 0.00001f));
		ENSURE(getAreFloatsEqual(b.x.y,  2.2f, 0.00001f));
		ENSURE(getAreFloatsEqual(b.x.z,  3.2f, 0.00001f));
		ENSURE(getAreFloatsEqual(b.x.w, -6.8f, 0.00001f));
		ENSURE(getAreFloatsEqual(b.y.x, -2.3f, 0.00001f));
		ENSURE(getAreFloatsEqual(b.y.y, -5.1f, 0.00001f));
		ENSURE(getAreFloatsEqual(b.y.z,  3.3f, 0.00001f));
		ENSURE(getAreFloatsEqual(b.y.w, -2.9f, 0.00001f));
		ENSURE(getAreFloatsEqual(b.z.x,  5.1f, 0.00001f));
		ENSURE(getAreFloatsEqual(b.z.y,  1.1f, 0.00001f));
		ENSURE(getAreFloatsEqual(b.z.z, -3.8f, 0.00001f));
		ENSURE(getAreFloatsEqual(b.z.w,  1.0f, 0.00001f));
		ENSURE(getAreFloatsEqual(b.t.x, -1.2f, 0.00001f));
		ENSURE(getAreFloatsEqual(b.t.y, -7.4f, 0.00001f));
		ENSURE(getAreFloatsEqual(b.t.z, -9.2f, 0.00001f));
		ENSURE(getAreFloatsEqual(b.t.w,  4.9f, 0.00001f));
	}
}

static void testAabb()
{
	{
		Aabb a;
		AabbFn::reset(a);
		ENSURE(a.min == VECTOR3_ZERO);
		ENSURE(a.max == VECTOR3_ZERO);
	}
	{
		Aabb a;
		a.min = createVector3(-2.3f, 1.2f, -4.5f);
		a.max = createVector3( 3.7f, 5.3f, -2.9f);
		const Vector3 c = AabbFn::getCenter(a);
		ENSURE(getAreFloatsEqual(c.x,  0.70f, 0.00001f));
		ENSURE(getAreFloatsEqual(c.y,  3.25f, 0.00001f));
		ENSURE(getAreFloatsEqual(c.z, -3.70f, 0.00001f));
	}
	{
		Aabb a;
		a.min = createVector3(-2.3f, 1.2f, -4.5f);
		a.max = createVector3( 3.7f, 5.3f, -2.9f);
		const float c = AabbFn::getVolume(a);
		ENSURE(getAreFloatsEqual(c, 39.36f, 0.00001f));
	}
	{
		Aabb a;
		AabbFn::reset(a);

		const Vector3 points[] =
		{
			{ -1.2f,  3.4f,  5.5f },
			{  8.2f, -2.4f, -1.5f },
			{ -5.9f,  9.2f,  6.0f }
		};
		AabbFn::addPoints(a, RIO_COUNTOF(points), points);
		ENSURE(getAreFloatsEqual(a.min.x, -5.9f, 0.00001f));
		ENSURE(getAreFloatsEqual(a.min.y, -2.4f, 0.00001f));
		ENSURE(getAreFloatsEqual(a.min.z, -1.5f, 0.00001f));
		ENSURE(getAreFloatsEqual(a.max.x,  8.2f, 0.00001f));
		ENSURE(getAreFloatsEqual(a.max.y,  9.2f, 0.00001f));
		ENSURE(getAreFloatsEqual(a.max.z,  6.0f, 0.00001f));
	}
	{
		Aabb boxes[3];
		AabbFn::reset(boxes[0]);
		AabbFn::reset(boxes[1]);
		AabbFn::reset(boxes[2]);

		const Vector3 points[] =
		{
			{ -1.2f,  3.4f,  5.5f },
			{  8.2f, -2.4f, -1.5f },
			{ -5.9f,  9.2f,  6.0f },

			{ -2.8f, -3.5f,  1.9f },
			{ -8.3f, -3.1f,  1.9f },
			{  4.0f, -3.9f, -1.4f },

			{ -0.4f, -1.8f, -2.2f },
			{ -8.6f, -4.8f,  2.8f },
			{  4.1f,  4.7f, -0.4f }
		};
		AabbFn::addPoints(boxes[0], RIO_COUNTOF(points)/3, &points[0]);
		AabbFn::addPoints(boxes[1], RIO_COUNTOF(points)/3, &points[3]);
		AabbFn::addPoints(boxes[2], RIO_COUNTOF(points)/3, &points[6]);

		Aabb d;
		AabbFn::reset(d);
		AabbFn::addBoxes(d, RIO_COUNTOF(boxes), boxes);
		ENSURE(getAreFloatsEqual(d.min.x, -8.6f, 0.00001f));
		ENSURE(getAreFloatsEqual(d.min.y, -4.8f, 0.00001f));
		ENSURE(getAreFloatsEqual(d.min.z, -2.2f, 0.00001f));
		ENSURE(getAreFloatsEqual(d.max.x,  8.2f, 0.00001f));
		ENSURE(getAreFloatsEqual(d.max.y,  9.2f, 0.00001f));
		ENSURE(getAreFloatsEqual(d.max.z,  6.0f, 0.00001f));
	}
	{
		Aabb a;
		a.min = createVector3(-2.3f, 1.2f, -4.5f);
		a.max = createVector3( 3.7f, 5.3f, -2.9f);
		ENSURE( AabbFn::containsPoint(a, createVector3(1.2f,  3.0f, -4.4f)));
		ENSURE(!AabbFn::containsPoint(a, createVector3(3.8f,  3.0f, -4.4f)));
		ENSURE(!AabbFn::containsPoint(a, createVector3(1.2f, -1.0f, -4.4f)));
		ENSURE(!AabbFn::containsPoint(a, createVector3(1.2f,  3.0f, -4.6f)));
	}
}

static void testSphere()
{
	{
		Sphere a;
		SphereFn::reset(a);
		ENSURE(a.c == VECTOR3_ZERO);
		ENSURE(getAreFloatsEqual(a.r, 0.0f, 0.00001f));
	}
	{
		Sphere a;
		a.c = VECTOR3_ZERO;
		a.r = 1.61f;
		const float b = SphereFn::getVolume(a);
		ENSURE(getAreFloatsEqual(b, 17.48099f, 0.00001f));
	}
	{
		Sphere a;
		SphereFn::reset(a);

		const Vector3 points[] =
		{
			{ -1.2f,  3.4f,  5.5f },
			{  8.2f, -2.4f, -1.5f },
			{ -5.9f,  9.2f,  6.0f }
		};
		SphereFn::addPoints(a, RIO_COUNTOF(points), points);
		ENSURE(getAreFloatsEqual(a.c.x, 0.0f, 0.00001f));
		ENSURE(getAreFloatsEqual(a.c.y, 0.0f, 0.00001f));
		ENSURE(getAreFloatsEqual(a.c.z, 0.0f, 0.00001f));
		ENSURE(getAreFloatsEqual(a.r, 12.46795f, 0.00001f));
	}
	{
		Sphere spheres[3];
		SphereFn::reset(spheres[0]);
		SphereFn::reset(spheres[1]);
		SphereFn::reset(spheres[2]);

		const Vector3 points[] =
		{
			{  6.6f,  3.5f, -5.7f },
			{ -5.3f, -9.1f, -7.9f },
			{ -1.5f,  4.4f, -5.8f },

			{  7.2f, -2.4f, -9.5f },
			{  4.0f, -8.1f,  6.6f },
			{ -8.2f,  2.2f,  4.6f },

			{  2.9f, -4.8f, -6.8f },
			{ -7.6f, -7.0f,  0.8f },
			{  8.2f,  2.8f, -4.8f }
		};
		SphereFn::addPoints(spheres[0], RIO_COUNTOF(points)/3, &points[0]);
		SphereFn::addPoints(spheres[1], RIO_COUNTOF(points)/3, &points[3]);
		SphereFn::addPoints(spheres[2], RIO_COUNTOF(points)/3, &points[6]);

		Sphere d;
		SphereFn::reset(d);
		SphereFn::addSpheres(d, RIO_COUNTOF(spheres), spheres);
		ENSURE(getAreFloatsEqual(d.r, 13.16472f, 0.00001f));
	}
	{
		Sphere a;
		a.c = createVector3(-2.3f, 1.2f, -4.5f);
		a.r = 1.0f;
		ENSURE( SphereFn::containsPoint(a, createVector3(-2.9f, 1.6f, -4.0f)));
		ENSURE(!SphereFn::containsPoint(a, createVector3(-3.9f, 1.6f, -4.0f)));
		ENSURE(!SphereFn::containsPoint(a, createVector3(-2.9f, 2.6f, -4.0f)));
		ENSURE(!SphereFn::containsPoint(a, createVector3(-2.9f, 1.6f, -6.0f)));
	}
}

static void testMurmur()
{
	const uint32_t m = getMurmurHash32("murmur32", 8, 0);
	ENSURE(m == 0x7c2365dbu);
	const uint64_t n = getMurmurHash64("murmur64", 8, 0);
	ENSURE(n == 0x90631502d1a3432bu);
}

static void testStringId()
{
	MemoryGlobalFn::init();
	{
		StringId32 a("murmur32");
		ENSURE(a.id == 0x7c2365dbu);

		StringId32 b("murmur32", 8);
		ENSURE(a.id == 0x7c2365dbu);

		TempAllocator64 ta;
		DynamicString str(ta);
		a.toString(str);
		ENSURE(strcmp(str.getCStr(), "7c2365db") == 0);
	}
	{
		StringId64 a("murmur64");
		ENSURE(a.id == 0x90631502d1a3432bu);

		StringId64 b("murmur64", 8);
		ENSURE(a.id == 0x90631502d1a3432bu);

		TempAllocator64 ta;
		DynamicString str(ta);
		a.toString(str);
		ENSURE(strcmp(str.getCStr(), "90631502d1a3432b") == 0);
	}
	MemoryGlobalFn::shutdown();
}

static void testDynamicString()
{
	MemoryGlobalFn::init();
	{
		TempAllocator1024 ta;
		DynamicString str(ta);
		ENSURE(str.getIsEmpty());

		str.set("murmur32", 8);
		ENSURE(str.getLength() == 8);

		const StringId32 id = str.getStringId();
		ENSURE(id.id == 0x7c2365dbu);
	}
	{
		TempAllocator1024 ta;
		DynamicString str("Test ", ta);
		str += "string.";
		ENSURE(strcmp(str.getCStr(), "Test string.") == 0);
	}
	{
		TempAllocator1024 ta;
		DynamicString str("   \tSushi\t   ", ta);
		str.trimLeading();
		ENSURE(strcmp(str.getCStr(), "Sushi\t   ") == 0);
	}
	{
		TempAllocator1024 ta;
		DynamicString str("   \tSushi\t   ", ta);
		str.trimTrailing();
		ENSURE(strcmp(str.getCStr(), "   \tSushi") == 0);
	}
	{
		TempAllocator1024 ta;
		DynamicString str("   \tSushi\t   ", ta);
		str.trim();
		ENSURE(strcmp(str.getCStr(), "Sushi") == 0);
	}
	{
		TempAllocator1024 ta;
		DynamicString str("Hello everyone!", ta);
		ENSURE(str.hasPrefix("Hello"));
		ENSURE(!str.hasPrefix("hello"));
		ENSURE(str.hasSuffix("one!"));
		ENSURE(!str.hasSuffix("one"));
		ENSURE(!str.hasPrefix("Hello everyone!!!"));
		ENSURE(!str.hasSuffix("Hello everyone!!!"));
	}
	MemoryGlobalFn::shutdown();
}

static void testGuid()
{
	MemoryGlobalFn::init();
	{
		Guid guid = GuidFn::createGuid();
		TempAllocator1024 ta;
		DynamicString str(ta);
		GuidFn::toString(guid, str);
		Guid parsed = GuidFn::parse(str.getCStr());
		ENSURE(guid == parsed);
	}
	{
		Guid guid;
		ENSURE(GuidFn::tryParse("961f8005-6a7e-4371-9272-8454dd786884", guid));
		ENSURE(!GuidFn::tryParse("961f80056a7e-4371-9272-8454dd786884", guid));
	}
	MemoryGlobalFn::shutdown();
}

static void testJson()
{
	MemoryGlobalFn::init();
	{
		JsonValueType::Enum t = JsonFn::getType("null");
		ENSURE(t == JsonValueType::NIL);
	}
	{
		JsonValueType::Enum t = JsonFn::getType("true");
		ENSURE(t == JsonValueType::BOOL);
	}
	{
		JsonValueType::Enum t = JsonFn::getType("false");
		ENSURE(t == JsonValueType::BOOL);
	}
	{
		JsonValueType::Enum t = JsonFn::getType("3.14");
		ENSURE(t == JsonValueType::NUMBER);
	}
	{
		JsonValueType::Enum t = JsonFn::getType("\"foo\"");
		ENSURE(t == JsonValueType::STRING);
	}
	{
		JsonValueType::Enum t = JsonFn::getType("[]");
		ENSURE(t == JsonValueType::ARRAY);
	}
	{
		JsonValueType::Enum t = JsonFn::getType("{}");
		ENSURE(t == JsonValueType::OBJECT);
	}
	{
		const int32_t a = JsonFn::parseInt("3.14");
		ENSURE(a == 3);
	}
	{
		const float a = JsonFn::parseFloat("3.14");
		ENSURE(getAreFloatsEqual(a, 3.14f));
	}
	{
		const bool a = JsonFn::parseBool("true");
		ENSURE(a == true);
	}
	{
		const bool a = JsonFn::parseBool("false");
		ENSURE(a == false);
	}
	{
		TempAllocator1024 ta;
		DynamicString str(ta);
		JsonFn::parseString("\"This is JSON\"", str);
		ENSURE(strcmp(str.getCStr(), "This is JSON") == 0);
	}
	MemoryGlobalFn::shutdown();
}

static void testJsonR()
{
	MemoryGlobalFn::init();
	{
		JsonValueType::Enum t = JsonRFn::getType("null");
		ENSURE(t == JsonValueType::NIL);
	}
	{
		JsonValueType::Enum t = JsonRFn::getType("true");
		ENSURE(t == JsonValueType::BOOL);
	}
	{
		JsonValueType::Enum t = JsonRFn::getType("false");
		ENSURE(t == JsonValueType::BOOL);
	}
	{
		JsonValueType::Enum t = JsonRFn::getType("3.14");
		ENSURE(t == JsonValueType::NUMBER);
	}
	{
		JsonValueType::Enum t = JsonRFn::getType("\"foo\"");
		ENSURE(t == JsonValueType::STRING);
	}
	{
		JsonValueType::Enum t = JsonRFn::getType("[]");
		ENSURE(t == JsonValueType::ARRAY);
	}
	{
		JsonValueType::Enum t = JsonRFn::getType("{}");
		ENSURE(t == JsonValueType::OBJECT);
	}
	{
		const int32_t a = JsonRFn::parseInt("3.14");
		ENSURE(a == 3);
	}
	{
		const float a = JsonRFn::parseFloat("3.14");
		ENSURE(getAreFloatsEqual(a, 3.14f));
	}
	{
		const bool a = JsonRFn::parseBool("true");
		ENSURE(a == true);
	}
	{
		const bool a = JsonRFn::parseBool("false");
		ENSURE(a == false);
	}
	{
		TempAllocator1024 ta;
		DynamicString str(ta);
		JsonRFn::parseString("\"This is JSON\"", str);
		ENSURE(strcmp(str.getCStr(), "This is JSON") == 0);
	}
	{
		const Vector2 a = JsonRFn::parseVector2("[ 1.2 -2.5 ]");
		ENSURE(getAreFloatsEqual(a.x,  1.2f));
		ENSURE(getAreFloatsEqual(a.y, -2.5f));
	}
	{
		const Vector3 a = JsonRFn::parseVector3("[ 3.1 0.5 -5.7]");
		ENSURE(getAreFloatsEqual(a.x,  3.1f));
		ENSURE(getAreFloatsEqual(a.y,  0.5f));
		ENSURE(getAreFloatsEqual(a.z, -5.7f));
	}
	{
		const Vector4 a = JsonRFn::parseVector4("[ 6.7 -1.3 2.9 -0.4 ]");
		ENSURE(getAreFloatsEqual(a.x,  6.7f));
		ENSURE(getAreFloatsEqual(a.y, -1.3f));
		ENSURE(getAreFloatsEqual(a.z,  2.9f));
		ENSURE(getAreFloatsEqual(a.w, -0.4f));
	}
	{
		const Quaternion a = JsonRFn::parseQuaternion("[ -1.5 -3.4 9.1 -3.5 ]");
		ENSURE(getAreFloatsEqual(a.x, -1.5f));
		ENSURE(getAreFloatsEqual(a.y, -3.4f));
		ENSURE(getAreFloatsEqual(a.z,  9.1f));
		ENSURE(getAreFloatsEqual(a.w, -3.5f));
	}
	{
		const Matrix4x4 a = JsonRFn::parseMatrix4x4(
			"["
			"-3.2  5.3 -0.7  4.1 "
			" 5.6  7.0 -3.2 -1.2 "
			"-6.3  9.0  3.9  1.1 "
			" 0.4 -7.3  8.9 -0.1 "
			"]"
			);
		ENSURE(getAreFloatsEqual(a.x.x, -3.2f));
		ENSURE(getAreFloatsEqual(a.x.y,  5.3f));
		ENSURE(getAreFloatsEqual(a.x.z, -0.7f));
		ENSURE(getAreFloatsEqual(a.x.w,  4.1f));
		ENSURE(getAreFloatsEqual(a.y.x,  5.6f));
		ENSURE(getAreFloatsEqual(a.y.y,  7.0f));
		ENSURE(getAreFloatsEqual(a.y.z, -3.2f));
		ENSURE(getAreFloatsEqual(a.y.w, -1.2f));
		ENSURE(getAreFloatsEqual(a.z.x, -6.3f));
		ENSURE(getAreFloatsEqual(a.z.y,  9.0f));
		ENSURE(getAreFloatsEqual(a.z.z,  3.9f));
		ENSURE(getAreFloatsEqual(a.z.w,  1.1f));
		ENSURE(getAreFloatsEqual(a.t.x,  0.4f));
		ENSURE(getAreFloatsEqual(a.t.y, -7.3f));
		ENSURE(getAreFloatsEqual(a.t.z,  8.9f));
		ENSURE(getAreFloatsEqual(a.t.w, -0.1f));
	}
	{
		const StringId32 a = JsonRFn::parseStringId("\"murmur32\"");
		ENSURE(a.id == 0x7c2365dbu);
	}
	{
		const ResourceId a = JsonRFn::parseResourceId("\"murmur64\"");
		ENSURE(a.id == 0x90631502d1a3432bu);
	}
	MemoryGlobalFn::shutdown();
}

static void testPath()
{
#if RIO_PLATFORM_POSIX
	{
		const bool a = PathFn::getIsAbsolute("/home/foo");
		ENSURE(a == true);
		const bool b = PathFn::getIsAbsolute("home/foo");
		ENSURE(b == false);
	}
	{
		const bool a = PathFn::getIsRelative("/home/foo");
		ENSURE(a == false);
		const bool b = PathFn::getIsRelative("home/foo");
		ENSURE(b == true);
	}
	{
		const bool a = PathFn::getIsRoot("/");
		ENSURE(a == true);
		const bool b = PathFn::getIsRoot("/home");
		ENSURE(b == false);
	}
#else
	{
		const bool a = PathFn::getIsAbsolute("C:\\Users\\foo");
		ENSURE(a == true);
		const bool b = PathFn::getIsAbsolute("Users\\foo");
		ENSURE(b == false);
	}
	{
		const bool a = PathFn::getIsRelative("D:\\Users\\foo");
		ENSURE(a == false);
		const bool b = PathFn::getIsRelative("Users\\foo");
		ENSURE(b == true);
	}
	{
		const bool a = PathFn::getIsRoot("E:\\");
		ENSURE(a == true);
		const bool b = PathFn::getIsRoot("E:\\Users");
		ENSURE(b == false);
	}
#endif // RIO_PLATFORM_POSIX
	{
		const char* p = PathFn::getBasename("");
		ENSURE(strcmp(p, "") == 0);
		const char* q = PathFn::getBasename("/");
		ENSURE(strcmp(q, "") == 0);
		const char* r = PathFn::getBasename("boot.config");
		ENSURE(strcmp(r, "boot.config") == 0);
		const char* s = PathFn::getBasename("foo/boot.config");
		ENSURE(strcmp(s, "boot.config") == 0);
		const char* t = PathFn::getBasename("/foo/boot.config");
		ENSURE(strcmp(t, "boot.config") == 0);
	}
	{
		const char* p = PathFn::getExtension("");
		ENSURE(p == NULL);
		const char* q = PathFn::getExtension("boot");
		ENSURE(q == NULL);
		const char* r = PathFn::getExtension("boot.bar.config");
		ENSURE(strcmp(r, "config") == 0);
	}
}

static void testCommandLine()
{
	const char* argumentList[] =
	{
		"args",
		"-s",
		"--switch",
		"--argument",
		"orange"
	};

	CommandLine commandLine(RIO_COUNTOF(argumentList), argumentList);
	ENSURE(commandLine.hasArgument("switch", 's'));
	const char* orange = commandLine.getParameter("argument");
	ENSURE(orange != NULL && strcmp(orange, "orange") == 0);
}

static void runUnitTests()
{
	testMemory();
	testArray();
	testVector();
	testHashMap();
	testVector2();
	testVector3();
	testVector4();
	testQuaternion();
	testColor4();
	testMatrix3x3();
	testMatrix4x4();
	testAabb();
	testSphere();
	testMurmur();
	testStringId();
	testDynamicString();
	testGuid();
	testJson();
	testJsonR();
	testPath();
	testCommandLine();
}

} // namespace Rio

#endif // RIO_BUILD_UNIT_TESTS
// Copyright (c) 2016 Volodymyr Syvochka
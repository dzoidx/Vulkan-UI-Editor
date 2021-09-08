#pragma once
#include <cmath>
#include "Types.h"


constexpr float PI = 3.14159265358979323846f;

struct vector2
{
	union
	{
		struct
		{
			real32 x;
			real32 y;
		};
		struct
		{
			real32 u;
			real32 v;
		};
	};

	vector2() :x(), y() {}
	vector2(real32 x, real32 y)
		:x(x), y(y)
	{}

	real32 length()
	{
		return sqrtf(x * x + y * y);
	}

	void normalize()
	{
		real32 len = length();
		x /= len;
		y /= len;
	}
};

struct aabb
{
	real32 x, y, w, h;

	aabb() :x(), y(), w(), h() {}
	aabb(real32 x, real32 y, real32 w, real32 h) :x(x), y(y), w(w), h(h) {}
};

typedef struct vector4
{
	union
	{
		struct
		{
			real32 x, y, z, w;
		};
		struct
		{
			real32 r, g, b, a;
		};
	};

	vector4(const vector2 v) : x(v.x), y(v.y), z(), w(1) {}
	vector4()
		:x(), y(), z(), w(1) {}
	vector4(real32 x, real32 y, real32 z, real32 w)
		:x(x), y(y), z(z), w(w)
	{}

} color;

struct matrix2x2
{
	real32 m_11, m_12;
	real32 m_21, m_22;
};

struct matrix4x4
{
	real32 m_11, m_12, m_13, m_14;
	real32 m_21, m_22, m_23, m_24;
	real32 m_31, m_32, m_33, m_34;
	real32 m_41, m_42, m_43, m_44;
};

inline matrix4x4 Transpose(const matrix4x4& mat)
{
	matrix4x4 result;
	result.m_11 = mat.m_11; result.m_12 = mat.m_21; result.m_13 = mat.m_31; result.m_14 = mat.m_41;
	result.m_21 = mat.m_12; result.m_22 = mat.m_22; result.m_23 = mat.m_32; result.m_24 = mat.m_42;
	result.m_31 = mat.m_13; result.m_32 = mat.m_23; result.m_33 = mat.m_33; result.m_34 = mat.m_43;
	result.m_41 = mat.m_14; result.m_42 = mat.m_24; result.m_43 = mat.m_34; result.m_44 = mat.m_44;
	return result;
}

inline matrix4x4 translate(real32 x, real32 y, real32 z)
{
	matrix4x4 result;
	result.m_11 = 1; result.m_12 = 0; result.m_13 = 0; result.m_14 = x;
	result.m_21 = 0; result.m_22 = 1; result.m_23 = 0; result.m_24 = y;
	result.m_31 = 0; result.m_32 = 0; result.m_33 = 1; result.m_34 = z;
	result.m_41 = 0; result.m_42 = 0; result.m_43 = 0; result.m_44 = 1;
	return result;
}

inline matrix4x4 scale(real32 x, real32 y, real32 z)
{
	matrix4x4 result;
	result.m_11 = x; result.m_12 = 0; result.m_13 = 0; result.m_14 = 0;
	result.m_21 = 0; result.m_22 = y; result.m_23 = 0; result.m_24 = 0;
	result.m_31 = 0; result.m_32 = 0; result.m_33 = z; result.m_34 = 0;
	result.m_41 = 0; result.m_42 = 0; result.m_43 = 0; result.m_44 = 1;
	return result;
}

inline matrix4x4 operator*(const matrix4x4& m1, const matrix4x4& m2)
{
	matrix4x4 result;
	result.m_11 = m1.m_11 * m2.m_11 + m1.m_12 * m2.m_21 + m1.m_13 * m2.m_31 + m1.m_14 * m2.m_41;
	result.m_12 = m1.m_11 * m2.m_12 + m1.m_12 * m2.m_22 + m1.m_13 * m2.m_32 + m1.m_14 * m2.m_42;
	result.m_13 = m1.m_11 * m2.m_13 + m1.m_12 * m2.m_23 + m1.m_13 * m2.m_33 + m1.m_14 * m2.m_43;
	result.m_14 = m1.m_11 * m2.m_14 + m1.m_12 * m2.m_24 + m1.m_13 * m2.m_34 + m1.m_14 * m2.m_44;

	result.m_21 = m1.m_21 * m2.m_11 + m1.m_22 * m2.m_21 + m1.m_23 * m2.m_31 + m1.m_24 * m2.m_41;
	result.m_22 = m1.m_21 * m2.m_12 + m1.m_22 * m2.m_22 + m1.m_23 * m2.m_32 + m1.m_24 * m2.m_42;
	result.m_23 = m1.m_21 * m2.m_13 + m1.m_22 * m2.m_23 + m1.m_23 * m2.m_33 + m1.m_24 * m2.m_43;
	result.m_24 = m1.m_21 * m2.m_14 + m1.m_22 * m2.m_24 + m1.m_23 * m2.m_34 + m1.m_24 * m2.m_44;

	result.m_31 = m1.m_31 * m2.m_11 + m1.m_32 * m2.m_21 + m1.m_33 * m2.m_31 + m1.m_34 * m2.m_41;
	result.m_32 = m1.m_31 * m2.m_12 + m1.m_32 * m2.m_22 + m1.m_33 * m2.m_32 + m1.m_34 * m2.m_42;
	result.m_33 = m1.m_31 * m2.m_13 + m1.m_32 * m2.m_23 + m1.m_33 * m2.m_33 + m1.m_34 * m2.m_43;
	result.m_34 = m1.m_31 * m2.m_14 + m1.m_32 * m2.m_24 + m1.m_33 * m2.m_34 + m1.m_34 * m2.m_44;

	result.m_41 = m1.m_41 * m2.m_11 + m1.m_42 * m2.m_21 + m1.m_43 * m2.m_31 + m1.m_44 * m2.m_41;
	result.m_42 = m1.m_41 * m2.m_12 + m1.m_42 * m2.m_22 + m1.m_43 * m2.m_32 + m1.m_44 * m2.m_42;
	result.m_43 = m1.m_41 * m2.m_13 + m1.m_42 * m2.m_23 + m1.m_43 * m2.m_33 + m1.m_44 * m2.m_43;
	result.m_44 = m1.m_41 * m2.m_14 + m1.m_42 * m2.m_24 + m1.m_43 * m2.m_34 + m1.m_44 * m2.m_44;

	return result;
}

constexpr matrix4x4 identity = {
	1, 0, 0, 0,
	0, 1, 0, 0,
	0, 0, 1, 0,
	0, 0, 0, 1
};

inline matrix4x4 orthographicLeftHanded(real32 left, real32 right, real32 top, real32 bottom, real32 near_, real32 far_)
{
	matrix4x4 result;
	real32 rl = right - left;
	real32 tb = top - bottom;
	real32 fn = far_ - near_;

	result.m_11 = 2 / rl;	result.m_12 = 0;		result.m_13 = 0;		result.m_14 = -(right + left) / rl;
	result.m_21 = 0;		result.m_22 = -2 / tb;	result.m_23 = 0;		result.m_24 = (top + bottom) / tb;
	result.m_31 = 0;		result.m_32 = 0;		result.m_33 = 2 / fn;	result.m_34 = -(far_ + near_) / fn; // for right handed coords negate m_33
	result.m_41 = 0;		result.m_42 = 0;		result.m_43 = 0;		result.m_44 = 1;

	return result;
}

// eg. vulkan
inline matrix4x4 orthographicLeftHandedYFlipped(real32 left, real32 right, real32 top, real32 bottom, real32 near_, real32 far_)
{
	matrix4x4 result;
	real32 rl = right - left;
	real32 tb = top - bottom;
	real32 fn = far_ - near_;

	result.m_11 = 2 / rl;	result.m_12 = 0;		result.m_13 = 0;		result.m_14 = -(right + left) / rl;
	result.m_21 = 0;		result.m_22 = -2 / tb;	result.m_23 = 0;		result.m_24 = (top + bottom) / tb;
	result.m_31 = 0;		result.m_32 = 0;		result.m_33 = 2 / fn;	result.m_34 = -(far_ + near_) / fn; // for right handed coords negate m_33
	result.m_41 = 0;		result.m_42 = 0;		result.m_43 = 0;		result.m_44 = 1;

	return result;
}

inline matrix4x4 orthographicRightHanded(real32 left, real32 right, real32 top, real32 bottom, real32 near_, real32 far_)
{
	matrix4x4 result;
	real32 rl = right - left;
	real32 tb = top - bottom;
	real32 fn = far_ - near_;

	result.m_11 = 2 / rl;	result.m_12 = 0;		result.m_13 = 0;		result.m_14 = -(right + left) / rl;
	result.m_21 = 0;		result.m_22 = 2 / tb;	result.m_23 = 0;		result.m_24 = -(top + bottom) / tb;
	result.m_31 = 0;		result.m_32 = 0;		result.m_33 = -2 / fn;	result.m_34 = -(far_ + near_) / fn;
	result.m_41 = 0;		result.m_42 = 0;		result.m_43 = 0;		result.m_44 = 1;

	return result;
}

inline matrix2x2 rotate(real32 degree)
{
	float angleInRadians = degree * (PI / 180.0f);
	matrix2x2 result;
	real32 cosA = cosf(angleInRadians);
	real32 sinA = sinf(angleInRadians);

	result.m_11 = cosA; result.m_12 = -sinA;
	result.m_21 = sinA; result.m_22 = cosA;

	return result;
}


inline vector2 operator*(const vector2& v1, real32 w)
{
	vector2 result;
	result.x = v1.x * w;
	result.y = v1.y * w;
	return result;
}

inline vector2 operator*(const vector2& v1, const vector2& v2)
{
	vector2 result;
	result.x = v1.x * v2.x;
	result.y = v1.y * v2.y;
	return result;
}

inline vector2 operator*(const vector2& v, const matrix2x2& m)
{
	vector2 result;
	result.x = m.m_11 * v.x + m.m_12 * v.y;
	result.y = m.m_21 * v.x + m.m_22 * v.y;

	return result;
}

inline vector4 operator*(const vector4& v, const matrix4x4& m)
{
	vector4 result;
	result.x = m.m_11 * v.x + m.m_12 * v.y + m.m_13 * v.z + m.m_14 * v.w;
	result.y = m.m_21 * v.x + m.m_22 * v.y + m.m_23 * v.z + m.m_24 * v.w;
	result.z = m.m_31 * v.x + m.m_32 * v.y + m.m_33 * v.z + m.m_34 * v.w;
	result.w = m.m_41 * v.x + m.m_42 * v.y + m.m_43 * v.z + m.m_44 * v.w;

	return result;
}

inline vector2 operator +(const vector2& v1, const vector2& v2)
{
	vector2 result;
	result.x = v1.x + v2.x;
	result.y = v1.y + v2.y;
	return result;
}

inline vector2 operator-(const vector2& v1, const vector2& v2)
{
	vector2 result;
	result.x = v1.x - v2.x;
	result.y = v1.y - v2.y;
	return result;
}

inline vector2 rotate(vector2 p, real32 angle)
{
	matrix2x2 m = rotate(angle);
	return p * m;
}

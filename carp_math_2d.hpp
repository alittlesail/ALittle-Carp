#ifndef CARP_MATRIX_2D_INCLUDED
#define CARP_MATRIX_2D_INCLUDED

class CarpMatrix2D
{
public:
	CarpMatrix2D() { SetIdentity(); }

public:
	CarpMatrix2D& SetIdentity()
	{
		m[0][0] = 1.0f; m[0][1] = 0.0f; m[0][2] = 0.0f;
		m[1][0] = 0.0f; m[1][1] = 1.0f; m[1][2] = 0.0f;
		m[2][0] = 0.0f; m[2][1] = 0.0f; m[2][2] = 1.0f;

		return *this;
	}

	CarpMatrix2D& SetRotate(float rad)
	{
		m[0][0] = static_cast<float>(cos(rad)); m[0][1] = static_cast<float>(sin(rad));	m[0][2] = 0.0f;
		m[1][0] = -m[0][1]; m[1][1] = m[0][0];	m[1][2] = 0.0f;
		m[2][0] = 0.0f;		m[2][1] = 0.0f;		m[2][2] = 1.0f;

		return *this;
	}

	CarpMatrix2D& Rotate(float rad)
	{
		CarpMatrix2D m; m.SetRotate(rad);
		return Multiply(m);
	}

	CarpMatrix2D& SetTranslation(float x, float y)
	{
		m[0][0] = 1.0f; m[0][1] = 0.0f; m[0][2] = 0.0f;
		m[1][0] = 0.0f; m[1][1] = 1.0f; m[1][2] = 0.0f;
		m[2][0] = x;	m[2][1] = y;	m[2][2] = 1.0f;

		return *this;
	}

	CarpMatrix2D& Translation(float x, float y)
	{
		CarpMatrix2D m; m.SetTranslation(x, y);
		return Multiply(m);
	}

	CarpMatrix2D& SetScale(float x, float y)
	{
		m[0][0] = x;	m[0][1] = 0.0f; m[0][2] = 0.0f;
		m[1][0] = 0.0f; m[1][1] = y;	m[1][2] = 0.0f;
		m[2][0] = 0.0f;	m[2][1] = 0.0f;	m[2][2] = 1.0f;

		return *this;
	}

	CarpMatrix2D& Scale(float x, float y)
	{
		CarpMatrix2D m; m.SetScale(x, y);
		return Multiply(m);
	}

public:
	CarpMatrix2D& Multiply(const CarpMatrix2D& right)
	{
		float r[3][3];

		r[0][0] = m[0][0] * right.m[0][0] + m[0][1] * right.m[1][0] + m[0][2] * right.m[2][0];
		r[0][1] = m[0][0] * right.m[0][1] + m[0][1] * right.m[1][1] + m[0][2] * right.m[2][1];
		r[0][2] = m[0][0] * right.m[0][2] + m[0][1] * right.m[1][2] + m[0][2] * right.m[2][2];

		r[1][0] = m[1][0] * right.m[0][0] + m[1][1] * right.m[1][0] + m[1][2] * right.m[2][0];
		r[1][1] = m[1][0] * right.m[0][1] + m[1][1] * right.m[1][1] + m[1][2] * right.m[2][1];
		r[1][2] = m[1][0] * right.m[0][2] + m[1][1] * right.m[1][2] + m[1][2] * right.m[2][2];

		r[2][0] = m[2][0] * right.m[0][0] + m[2][1] * right.m[1][0] + m[2][2] * right.m[2][0];
		r[2][1] = m[2][0] * right.m[0][1] + m[2][1] * right.m[1][1] + m[2][2] * right.m[2][1];
		r[2][2] = m[2][0] * right.m[0][2] + m[2][1] * right.m[1][2] + m[2][2] * right.m[2][2];

		memcpy(m, r, sizeof(float) * 9);

		return *this;
	}

public:
	float m[3][3];
};

class CarpVector2D
{
public:
	CarpVector2D() : x(0.0f), y(0.0f) {}
	CarpVector2D(float xx, float yy) : x(xx), y(yy) {}

public:
	CarpVector2D& Multiply(const CarpMatrix2D& right)
	{
		const float xx = x * right.m[0][0] + y * right.m[1][0] + 1 * right.m[2][0];
		const float yy = x * right.m[0][1] + y * right.m[1][1] + 1 * right.m[2][1];

		x = xx; y = yy;

		return *this;
	}

public:
	float x, y;
};

class CarpColor4
{
public:
	CarpColor4() : r(255), g(255), b(255), a(255) {}

	bool operator == (const CarpColor4& rr) const
	{
		return r == rr.r && g == rr.g && b == rr.b && a == rr.a;
	}

	bool operator != (const CarpColor4& rr) const
	{
		return r != rr.r || g != rr.g || b != rr.b || a != rr.a;
	}

public:
	unsigned char r, g, b, a;
};

#endif

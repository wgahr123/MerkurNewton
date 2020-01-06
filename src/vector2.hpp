/*
 *  vector2.hpp
 *  simple 2-dimensional vectors
 *
 */

#ifndef _VECTOR_H_
#define _VECTOR_H_

#include <ostream>
#include "real.hpp"

#ifndef _WIN32
#include <cstring>
#endif

class vector2
{

public:

	vector2(const char* x = "0", const char* y = "0")
	{
		m_vec.x = real(x);
		m_vec.y = real(y);
	}

	inline vector2(const real& x, const real& y)
	{
 		m_vec.x = x;
		m_vec.y = y;
	}

	~vector2()
	{
	}

	vector2(const vector2& vec)
    : m_vec(vec.m_vec)
	{
	}

	const vector2& operator+= (const vector2& vec)
	{
    m_vec.x += vec.m_vec.x;
    m_vec.y += vec.m_vec.y;
		return *this;
	}

	const vector2& operator-= (const vector2& vec)
	{
		m_vec.x -= vec.m_vec.x;
		m_vec.y -= vec.m_vec.y;
		return *this;
	}

	vector2 operator- (const vector2& vec) const
	{
		vector2 result;
		result.m_vec.x = m_vec.x - vec.m_vec.x;
		result.m_vec.y = m_vec.y - vec.m_vec.y;
		return result;
	}

	const real& x() const
	{
		return m_vec.x;
	}

	const real& y() const
	{
		return m_vec.y;
	}

	friend const vector2 operator*(const real& val, const vector2& vec)
        {
            return vector2(val * vec.m_vec.x, val * vec.m_vec.y);
        }

        friend const vector2 operator*(const vector2& vec, const real& val)
        {
            return vector2(val * vec.m_vec.x, val * vec.m_vec.y);
        }

        friend const vector2 operator/(const vector2& vec, const real& val)
        {
            return vector2(vec.m_vec.x / val, vec.m_vec.y / val);
        }

        const vector2 operator+(const vector2& vec) const
	{
		return vector2(m_vec.x + vec.m_vec.x, m_vec.y + vec.m_vec.y);
	}

	// Skalarprodukt
	const real operator* (const vector2& vec) const
	{
		return m_vec.x*vec.x() + m_vec.y*vec.y();
	}

	const real sqr() const
	{
		return m_vec.x.sqr() + m_vec.y.sqr();
	}

	const real abs() const
	{
		return this->sqr().sqrt();
	}

	friend const real abs(const vector2& vec)
	{
		return vec.sqr().sqrt();
	}

  vector2 operator- () const
  {
    return vector2(-m_vec.x, -m_vec.y);
  }

  vector2& operator*= (const real& val)
  {
    m_vec.x *= val;
    m_vec.y *= val;
    return *this;
  }

  vector2& operator/= (const real& val)
  {
    m_vec.x /= val;
    m_vec.y /= val;
    return *this;
  }

  bool operator==(const vector2& a)
	{
		return m_vec.x == a.m_vec.x && m_vec.y == a.m_vec.y;
	}

	bool operator!=(const vector2& a)
	{
		return m_vec.x != a.m_vec.x || m_vec.y != a.m_vec.y;
	}

	bool operator<(const vector2& a)
	{
		return sqr() < a.sqr();		// sqrt < sqrt <=> sqr < sqr
	}

	bool operator<=(const vector2& a)
	{
		return sqr() <= a.sqr();
	}

	bool operator>(const vector2& a)
	{
		return sqr() > a.sqr();
	}

	bool operator>=(const vector2& a)
	{
		return sqr() >= a.sqr();
	}

	int print(const char* format = "%Rf") const
	{
		printf("( ");
		int r = mpfr_printf(format, m_vec.x);
		if (r < 0) return r;
		printf(", ");
		r = mpfr_printf(format, m_vec.y);
		printf(" )");
		return r;
	}

	const char* snprintf(char* str, size_t strmaxsize, const char* format = "%Rf") const
	{
		::snprintf(&str[0], strmaxsize, "( ");
		int r = mpfr_snprintf(&str[strlen(str)], strmaxsize - strlen(str), format, m_vec.x);
		if (r < 0) return "<ERROR>";
		::snprintf(&str[strlen(str)], strmaxsize-strlen(str), ", ");
		r = mpfr_snprintf(&str[strlen(str)], strmaxsize - strlen(str), format, m_vec.y);
		if (r < 0) return "<ERROR>";
		::snprintf(&str[strlen(str)], strmaxsize - strlen(str), " )");
		// std::cout << str << std::endl;
		return str;
	}

private:

	struct vector2_t
	{
		real x;
		real y;
	} m_vec;
	
};

#endif // _VECTOR_H_



/*
 *  real.h
 *
 */

#ifndef _REAL_H_
#define _REAL_H_

#include "mpfr.h"
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>

const int REAL_PRECISION = 160; // bits
const int SIZE_TEXTLINE = 2000;
const int NUMBER_OF_LINES = 20;

#ifdef DEBUG_PRINT
#define PRINTF2(OBJ, TEXT)                                                                         \
    mpfr_printf("%s[%d]: %p -> %Rf %s\n", __FILE__, __LINE__, (OBJ).m_value, (OBJ).m_value, (TEXT))
#define PRINTF1(TEXT) PRINTF2(*this, (TEXT))
#define PRINTF PRINTF1("")
#else
#define PRINTF2(OBJ, TEXT)
#define PRINTF1(TEXT)
#define PRINTF
#endif

class CReal
{
protected:
#ifdef WIN32
    __declspec(align(32))
#endif
    mpfr_t m_value;
    static int m_precision;

public:
    static void set_precision(int precision)
    {
        m_precision = precision;
    }

    CReal(const mpfr_t& val)
    {
        mpfr_init2(m_value, m_precision);
        int r = mpfr_set(m_value, val, MPFR_RNDF);
        PRINTF1("new");
    }

    CReal(const char* str = "0")
    {
        mpfr_init2(m_value, m_precision);
        int r = mpfr_set_str(m_value, str, 10, MPFR_RNDF);
        PRINTF1("new");
    }

    CReal(const int val)
    {
        mpfr_init2(m_value, m_precision);
        int r = mpfr_set_si(m_value, val, MPFR_RNDF);
        PRINTF1("new");
    }

    CReal(const size_t val)
    {
        mpfr_init2(m_value, m_precision);
        int r = mpfr_set_ui(m_value, static_cast<unsigned long>(val), MPFR_RNDF);
        PRINTF1("new");
    }

    CReal(const CReal& val)
    {
        mpfr_init2(m_value, m_precision);
        int r = mpfr_set(m_value, val.m_value, MPFR_RNDF);
        PRINTF2(val, "old from CReal");
        PRINTF1("new from CReal");
    }

    CReal& operator=(const CReal& a)
    {
        int res = mpfr_set(m_value, a.m_value, MPFR_RNDF);
        PRINTF;
        return *this;
    }

    ~CReal()
    {
        PRINTF1("cleared");
        mpfr_clear(m_value);
    }

    void value(mpfr_ptr p_value)
    {
         mpfr_set(p_value, m_value, MPFR_RNDN);
    }

    int printf(const char* format) const
    {
        int r = mpfr_printf(format, m_value);
        return r;
    }

    const char* snprintf(char* str, size_t strmaxsize, const char* format) const
    {
        int res = mpfr_snprintf(str, strmaxsize, format, m_value);
        if (res < 0) return "<ERROR>";
        return str;
    }

    CReal operator-() const
    {
        CReal neg(m_value);
        // mpfr_dump(neg.m_value);
        // mpfr_dump(m_value);
        int r = mpfr_neg(neg.m_value, neg.m_value, MPFR_RNDF); // ??
        // mpfr_dump(neg.m_value);
        // PRINTF2(neg, "neg: out");
        // PRINTF1("neg: in");
        return neg;
    }

    CReal operator+(const CReal& a) const
    {
        CReal result;
        int res = mpfr_add(result.m_value, m_value, a.m_value, MPFR_RNDF);
        PRINTF;
        return result;
    }

    CReal operator-(const CReal& a) const
    {
        CReal result;
        int res = mpfr_sub(result.m_value, m_value, a.m_value, MPFR_RNDF);
        PRINTF;
        return result;
    }

    CReal operator*(const CReal& a) const
    {
        CReal result;
        int res = mpfr_mul(result.m_value, m_value, a.m_value, MPFR_RNDF);
        PRINTF;
        return result;
    }

    CReal operator/(const CReal& a) const
    {
        CReal result;
        int res = mpfr_div(result.m_value, m_value, a.m_value, MPFR_RNDF);
        PRINTF;
        return result;
    }

    CReal operator%(const CReal& a) const
    {
        CReal result;
        int res = mpfr_fmod(result.m_value, m_value, a.m_value, MPFR_RNDF);
        PRINTF;
        return result;
    }

    CReal& operator+=(const CReal& a)
    {
        int res = mpfr_add(m_value, m_value, a.m_value, MPFR_RNDF);
        PRINTF;
        return *this;
    }

    CReal& operator-=(const CReal& a)
    {
        int res = mpfr_sub(m_value, m_value, a.m_value, MPFR_RNDF);
        PRINTF;
        return *this;
    }

    CReal& operator*=(const CReal& a)
    {
        int res = mpfr_mul(m_value, m_value, a.m_value, MPFR_RNDF);
        PRINTF;
        return *this;
    }

    CReal& operator/=(const CReal& a)
    {
        int res = mpfr_div(m_value, m_value, a.m_value, MPFR_RNDF);
        PRINTF;
        return *this;
    }

    CReal& operator%=(const CReal& a)
    {
        int res = mpfr_fmod(m_value, m_value, a.m_value, MPFR_RNDF);
        PRINTF;
        return *this;
    }

    int operator==(const CReal& a) const { return mpfr_cmp(m_value, a.m_value) == 0; }

    int operator!=(const CReal& a) const { return mpfr_cmp(m_value, a.m_value) != 0; }

    bool operator>(const CReal& a) const { return mpfr_cmp(m_value, a.m_value) > 0; }

    bool operator>=(const CReal& a) const { return mpfr_cmp(m_value, a.m_value) >= 0; }

    bool operator<(const CReal& a) const { return mpfr_cmp(m_value, a.m_value) < 0; }

    bool operator<=(const CReal& a) const { return mpfr_cmp(m_value, a.m_value) <= 0; }

    operator int() const { return mpfr_get_si(m_value, MPFR_RNDF); }

    operator double() const { return mpfr_get_d(m_value, MPFR_RNDF); }

    const CReal abs() const
    {
        if (mpfr_signbit(m_value) == 1) { return -*this; }
        else
        {
            return *this;
        }
    }

    const CReal sqr() const
    {
        CReal result;
        int res = mpfr_sqr(result.m_value, m_value, MPFR_RNDF);
        PRINTF;
        return result;
    }

    void inv_me()
    {
        int res = mpfr_si_div(m_value, 1, m_value, MPFR_RNDF);
        PRINTF;
    }

    void sqr_me() { int res = mpfr_sqr(m_value, m_value, MPFR_RNDF); }

    void pow3_me()
    {
        int res;
        res = mpfr_pow_ui(m_value, m_value, 3, MPFR_RNDF);
    }

    const CReal sqrt() const
    {
        CReal result;
        int res = mpfr_sqrt(result.m_value, m_value, MPFR_RNDF);
        PRINTF;
        return result;
    }

    void sqrt_me() { int res = mpfr_sqrt(m_value, m_value, MPFR_RNDF); }

    friend const CReal sqrt(const CReal& a)
    {
        CReal result;
        mpfr_sqrt(result.m_value, a.m_value, MPFR_RNDF);
        return result;
    }

    friend const CReal atan2(const CReal& y, const CReal& x) // (y,x)
    {
        CReal result;
        mpfr_atan2(result.m_value, y.m_value, x.m_value, MPFR_RNDF);
        return result;
    }

    friend const CReal sin(const CReal& x)
    {
        CReal result;
        mpfr_sin(result.m_value, x.m_value, MPFR_RNDF);
        return result;
    }

    friend const CReal cos(const CReal& x)
    {
        CReal result;
        mpfr_cos(result.m_value, x.m_value, MPFR_RNDF);
        return result;
    }

    const std::string write(const char* format = "%.20Rf")
    {
        std::string line;
        char text[200];

#ifdef DEBUG_PRINT
        int length = mpfr_sprintf(text, "%s[%d]: %p -> %Rf\n", __FILE__, __LINE__, this, m_value);
#else
        int length = mpfr_sprintf(text, format, m_value);
#endif
        if (length < 0) { line = "<error>"; }
        else
        {
            line = text;
        }
        /*
                    else if (width < length+2)
                    {
                            text[length] = '*';
                            text[length + 1] = '\0';
              }
        */
        return line;
    }
};

int CReal::m_precision = REAL_PRECISION;

typedef CReal real;

#endif // _REAL_H_

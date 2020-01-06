// test_real

#include <iostream>
#include <stdio.h>
#include "real.hpp"

using std::cout;
using std::cerr;
using std::endl;

const char* write(real& value, const int width, const char* format = "%e")
{
	char* text = new char[500];
	int real_width = min(width, sizeof(text) - 1);
	int length = mpfr_snprintf(&text[0], real_width, format, value);
	if (length < 0)
	{
		strncpy_s(text, 500, "<error>", 8);
	}
	else if (real_width == length + 1)
	{
		text[length] = '*';
		text[length + 1] = '\0';
	}
	return text;
}

bool check(const int N, const char* id, const std::string one, const std::string two)
{
	if ( one == two )
	{
		cout << "[" << N << "]" << id << ": OK" << endl;
		return true;
	}
	else
	{

		cout << "[" << N << "]" << id << ": FAILED"
             << " " << one << " " << two << endl;
		return false;
	}
}

int main()
{
	int number_of_errors = 0;
	const char* format = "%20.20Rf";
	const int width = 10;
	int r;
	char text[1000];
    int N = 0;

	real a;
	if (not check(++N, "real a", write(a, width, format), "0.0000")) number_of_errors++;

	real b(a);
	if (not check(++N, "real b(a)", write(b, width, format), "0.0000")) number_of_errors++;

	mpfr_t m;
	mpfr_init2(m, REAL_PRECISION);
	mpfr_set_si(m, -2345, MPFR_RNDN);
	real c(m);
        if (not check(++N,
                      "mpfr_t m; mpfr_init2(m, REAL_PRECISION); mpfr_set_si(m, -2345, MPFR_RNDN); "
                      "real c(m)",
                      write(c, width, format), "-2345."))
            number_of_errors++;

	real d = "12.34";
        if (not check(++N, "real d = \"12.34\";", write(d, width, format), "12.340"))
            number_of_errors++;

	real e = 123;
        if (not check(++N, "real e = 123;", write(e, width, format), "123.00")) number_of_errors++;

	real f = e;
        if (not check(++N, "real f = e;", write(f, width, format), "123.00")) number_of_errors++;

	r = f.printf(format);
	sprintf_s(text, "%d", r);
        if (not check(++N, "r = f.printf(format);", text, "24")) number_of_errors++;
        if (not check(++N, "f.snprintf(text, sizeof(text), format);",
                      f.snprintf(text, sizeof(text), format), "123.00000000000000000000"))
            number_of_errors++;

	real g(-d);
        if (not check(++N, "real g(-d);", g.write(), "-12.34000000000000000000")) number_of_errors++;

	real h(d);
        if (not check(++N, "real h(-d);", h.write(), "12.34000000000000000000")) number_of_errors++;

	g = d;
        if (not check(++N, "g = d;", g.write(), "12.34000000000000000000")) number_of_errors++;

	g = -d;
        if (not check(++N, "g = -d;", g.write(), "-12.34000000000000000000")) number_of_errors++;

	d = "1";
	e = "2";

	g = d + e;
        if (not check(++N, "d=\"1\"; e=\"2\"; g = d + e;", g.write(), "3.00000000000000000000"))
            number_of_errors++;

	g = d - e;
        if (not check(++N, "g = d - e;", g.write(), "-1.00000000000000000000")) number_of_errors++;

	g = d * e;
        if (not check(++N, "g = d * e;", g.write(), "2.00000000000000000000")) number_of_errors++;

	g = d / e;
        if (not check(++N, "g = d / e;", g.write(), "0.50000000000000000000")) number_of_errors++;

	g = d; g += e;
        if (not check(++N, "g = d; g += e;", g.write(), "3.00000000000000000000")) number_of_errors++;

	g = d; g -= e;
        if (not check(++N, "g = d; g -= e;", g.write(), "-1.00000000000000000000")) number_of_errors++;

	if (not check(++N, "d == d", d == d ? "true" : "false", "true")) number_of_errors++;
        if (not check(++N, "d == e", d == e ? "false" : "true", "true")) number_of_errors++;
        if (not check(++N, "d != e", d != e ? "true" : "false", "true")) number_of_errors++;
        if (not check(++N, "d > e", d > e ? "false" : "true", "true")) number_of_errors++;
        if (not check(++N, "d >= e", d >= e ? "false" : "true", "true")) number_of_errors++;
        if (not check(++N, "d < e", d < e ? "true" : "false", "true")) number_of_errors++;
        if (not check(++N, "d <= e", d <= e ? "true" : "false", "true")) number_of_errors++;

	cout << number_of_errors << " error found" << endl;
}

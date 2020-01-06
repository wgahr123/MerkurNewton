// test_vector

#include <iostream>
#include <stdio.h>
#include "vector.hpp"

using std::cout;
using std::cerr;
using std::endl;

bool check(const char* id, const char* one, const char* two)
{
	if (strlen(one) == strlen(two) and strncmp(one, two, strlen(one)) == 0)
	{
		cout << id << ": OK" << endl;
		return true;
	}
	else
	{
		cout << id << ": FAILED" << " " << one << " " << two << endl;
		return false;
	}
}

int main()
{
	int number_of_errors = 0;
	const char* format = "%.2Rf";
	//int r;
	//char text[1000];
	
	vector a;
	if (not check("vector a", a.write(format), "( 0.00, 0.00 )")) number_of_errors++;

	vector b(a);
	if (not check("vector b(a)", b.write(format), "( 0.00, 0.00 )")) number_of_errors++;

	vector c("12.3", "34.5");
	if (not check("vector c = ( \"12.3\", \"34.5\")", c.write("%.4Rf"), "( 12.3000, 34.5000 )")) number_of_errors++;

	vector d = vector("12", "34");
	if (not check("vector d = ( \"12\", \"34\" )", d.write(format), "( 12.00, 34.00 )")) number_of_errors++;

	vector f = d;
	if (not check("vector f = d", f.write(format), "( 12.00, 34.00 )")) number_of_errors++;

	const char* p_text = f.write("%.0Rf");
	if (not check("r = f.write(format)", p_text, "( 12, 34 )")) number_of_errors++;
	
 	vector g(-d);
	if (not check("vector g(-d)", g.write(format), "( -12.00, -34.00 )")) number_of_errors++;

	vector h(d);
	if (not check("vector h(d)", h.write(format), "( 12.00, 34.00 )")) number_of_errors++;

	g = d;
	if (not check("g = d", g.write(format), "( 12.00, 34.00 )")) number_of_errors++;

	g = -d;
	if (not check("g = -d", g.write(format), "( -12.00, -34.00 )")) number_of_errors++;

	d = vector("1", "2");
	vector e = vector("3", "4");

	g = d + e;
	if (not check("d + e == (4, 6)", g.write(format), "( 4.00, 6.00 )")) number_of_errors++;

	g = d - e;
	if (not check("g = d - e", g.write(format), "( -2.00, -2.00 )")) number_of_errors++;

	g = d; g += e;
	if (not check("g = d; g += e", g.write(format), "( 4.00, 6.00 )")) number_of_errors++;

	g = d; g -= e;
	if (not check("g = d; g -= e", g.write(format), "( -2.00, -2.00 )")) number_of_errors++;

	if (not check("d == d", (d == d ? "true" : "false"), "true")) number_of_errors++;
	if (not check("d == e", (d == e ? "false" : "true"), "true")) number_of_errors++;
	if (not check("d != e", (d != e ? "true" : "false"), "true")) number_of_errors++;
	if (not check("d > e", (d > e ? "false" : "true"), "true")) number_of_errors++;
	if (not check("d >= e", (d >= e ? "false" : "true"), "true")) number_of_errors++;
	if (not check("d < e", (d < e ? "true" : "false"), "true")) number_of_errors++;
	if (not check("d <= e", (d <= e ? "true" : "false"), "true")) number_of_errors++;

	cout << number_of_errors << " error found" << endl;
}

#include <stdio.h>
#include <string.h>
#include <iostream>
#include <string>

using namespace std;

class Rational {
public:
    //explicit 
    Rational(int numerator=0, int denominator=1)
	:m_numerator(numerator)
	,m_denominator(denominator){}

public:
    int up() const { return m_numerator; }
    int down() const { return m_denominator; }

    const Rational operator *(const Rational& rhs)
    {
	printf("member\n");
	return Rational(rhs.up()*m_numerator, rhs.down()*m_denominator);
    }

    double value() { return 1.0*m_numerator/m_denominator; }

private:
    int m_numerator;
    int m_denominator;
};

inline const Rational operator*(const Rational &lhs, const Rational &rhs)
{
    printf("non-member\n");
    return Rational(lhs.up()*rhs.up(), lhs.down()*rhs.down());
}

int main(int argc, char *argv[])
{
    Rational a(1,8);
    Rational b(1,2);

    printf("a = %.3f\n", a.value());
    printf("b = %.3f\n", b.value());

    a = a*2; // implicit type conversion, cast 2 to Ratinal, you can avoid it by explicit Rational's contructor
    printf("a = %.3f\n", a.value());

    a = 2*a; // wrong, when Rational's operator* is member
    printf("a = %.3f\n", a.value());

    return 0;
}

import sys


def gcd(a,b):
    q = a/b; r = a%b
    while r != 0:
        a = b; b = r
        q = a/b; r = a%b

    return b


# gcd(a,b) = gcd(a, a mod b)
def gcd_revised(a,b):
    r = a % b
    if (r == 0):
        return b
    else:
        return gcd_revised(b, r)


# ax + by = d = gcd(a, b)
def gcd_extended(a,b):
    x_1 = 1; x0 = 0;
    y_1 = 0; y0 = 1
    r = a%b; q = a/b;

    x = x_1 - q*x0; y = y_1 - q*y0
    while r != 0:
        a = b; b = r
        x_1 = x0; x0 = x
        y_1 = y0; y0 = y
        r = a%b; q = a/b
        if (r == 0): break
        x = x_1 - q*x0; y = y_1 - q*y0

    return (b,x,y)

if __name__ == "__main__":
    a = int(sys.argv[1])
    b = int(sys.argv[2])
    if a < b: tmp = a; a = b; b = tmp
    print 'GCD of %d and %d is:' % (a,b)
    print gcd(a,b)
    print gcd_revised(a,b)
    d,x,y=gcd_extended(a,b)
    if (x*a+y*b != d):
        print 'failed'
    else:
        print d,'= gcd(a,b) = a*x + b*y (x=%d,y=%d)' % (x, y)

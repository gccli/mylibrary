#include <stdio.h>
#include <stdlib.h>

void find_maxmin()
{
    // 3*(n-2)/2
    const int n = 10;
    int a[n] = {-1, 1, 2, 4, 13, 13, 7, 3, 0, 11};

    int i, min = a[0], max = a[0];
    for (i=2; i<sizeof(a)/sizeof(int); i+=2) {
	if (a[i-1] < a[i]) {
	    if (a[i-1] < min)
		min = a[i-1];
	    if (a[i] > max)
		max = a[i];
	} else if (a[i-1] > a[i]) {
	    if (a[i] < min)
		min = a[i];
	    if (a[i-1] > max)
		max = a[i-1];
	} else {
	    if (a[i] < min)
		min = a[i];
	    else if (a[i] > max)
		max = a[i];
	}
    }
    if (i == sizeof(a)/sizeof(int)) {
	if (a[i-1] < min)
	    min = a[i-1];
	else if (a[i-1] > max)
	    max = a[i-1];
    }

    printf("%d %d\n", min, max);
}

void rescur(int *res, int n)
{
    if (n >= 10)
	rescur(res+1, n/10);
    *res = n%10;
}

void inv()
{
    int x=123456700, a[9];
    rescur(a,x);
    for(int i=0; i<9; ++i) printf("%d\n", a[i]);
}


int palindrome(const char *s, int n) 
{
    if (n<=1)
	return 1;
    if (s[0] == s[n-1]) return palindrome(s+1, n-2);
    return 0;
}

void odd_even()
{
    // O(n)
    auto a[10] = {1,2,3,4,5,6,7,8,9,0}; // -> {1,3,5,7,9,2,4,6,8,0}
}

int main(int argc, char *argv[])
{
    inv();
    printf(palindrome("abcdedcba", 9)?"Yes\n":"No\n");
    printf(palindrome("aa", 2)?"Yes\n":"No\n");
    return 0;
}

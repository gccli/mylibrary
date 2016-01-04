#include <algorithm>
#include <iterator>
#include <iostream>
#include <vector>
#include <map>

#include <stdio.h>

int main()
{
    unsigned char start = '0';
    unsigned char end   = '9';

    unsigned char i;
    std::map<char, char> m1, m2;
    std::vector<char> v;
    for (i=start; i<=end; ++i) {
        v.push_back(i);
    }
    std::random_shuffle(v.begin(), v.end());
    std::vector<char>::iterator it = v.begin();
    for(i=start; it != v.end(); ++it, ++i) {
        m1[i] = *it;
        m2[*it] = i;
    }

    std::map<char,char>::iterator ii = m1.begin();
    for(; ii != m1.end(); ++ii) {
        printf("'%c', ", ii->second);
    }
    printf("\n");
    for(ii = m2.begin(); ii != m2.end(); ++ii) {
        printf("'%c', ", ii->second);
    }
    printf("\n");

    return 0;
}

#! /usr/bin/env python
import sys

def id_verify(idnum):
    if (len(idnum) != 18): return False

    print idnum
    __Wi = [7, 9, 10, 5, 8, 4, 2, 1, 6, 3, 7, 9, 10, 5, 8, 4, 2]
    __Ti = ['1', '0', 'x', '9', '8', '7', '6', '5', '4', '3', '2']

    sum = 0
    for i in range(17):
        sum += int(idnum[i]) * __Wi[i]
    if __Ti[sum % 11] != idnum[-1]:
        return False
    return True

if __name__ == '__main__':
    print id_verify(sys.argv[1])

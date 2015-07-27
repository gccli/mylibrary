#! /usr/bin/python 
import sys

def calc_buy(curr_price, curr_number, expect_price, deduct_number):
    if (curr_number < deduct_number):
        return

    tax_ratio = 0.001
    yongjin = 0.0005
    min_cost = 5.0
    left_number = curr_number - deduct_number
    sell_price = (curr_number*curr_price-left_number*expect_price)/deduct_number
    return sell_price

def main():
    print calc_buy(16.1, 2500, 15.5, 500)

if __name__ == '__main__':
    main()

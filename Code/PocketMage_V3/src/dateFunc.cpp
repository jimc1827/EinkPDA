/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */

#include "globals.h"

bool isLeapYear(int year) {
  return ((year % 4 == 0) && (year % 100 != 0)) || (year % 400 == 0);
}

int daysInMonth(int year, int month) {
  constexpr int table[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

  if (month == 2 && isLeapYear(year)) {
    return 29;
  } else {
    return table[month - 1]; // Assumes months are numbered 1-12
  }
}

bool validDate(int year, int month, int day) {
  return ((1 <= month && month <= 12) &&
          (1 <= day   && day <= daysInMonth(year, month)));
}

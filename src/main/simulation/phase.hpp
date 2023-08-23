#pragma once

#include <iostream>

enum class Phase {
  unspecified = 0,
  learning,
  testing,
  mixing,
  spontaneous,
  pulse,
};

inline std::istream &operator>>(std::istream &is, Phase &p) {
  std::string s;
  is >> s;
  p =
      (s == "learn"   ? Phase::learning
       : s == "test"  ? Phase::testing
       : s == "mix"   ? Phase::mixing
       : s == "spont" ? Phase::spontaneous
       : s == "pulse" ? Phase::pulse
                      : Phase::unspecified);
  return is;
}

#include <Eigen/Dense>
#include <boost/range/counting_range.hpp>

#include "delays.hpp"

Eigen::ArrayXXi generateDelays(unsigned const totalNeuronNumber, int const delayParameter, double const maximumDelay) {
  Eigen::ArrayXXi delays(totalNeuronNumber, totalNeuronNumber);

  // We generate the delays:

  // We use a trick to generate an exponential distribution, median should be small (maybe 2-4ms) The mental image is
  // that you pick a uniform value in the unit line, repeatedly check if it falls below a certain threshold - if not,
  // you cut out the portion of the unit line below that threshold and stretch the remainder (including the random
  // value) to fill the unit line again. Each time you increase a counter, stopping when the value finally falls below
  // the threshold. The counter at the end of this process has exponential distribution. There's very likely simpler
  // ways to do it.

  // DELAYPARAM should be a small value (3 to 6). It controls the median of the exponential.
  for (auto const ni : boost::counting_range<unsigned>(0, totalNeuronNumber)) {
    for (auto const nj : boost::counting_range<unsigned>(0, totalNeuronNumber)) {

      double val = (double)rand() / (double)RAND_MAX;
      double crit = 1.0 / delayParameter; // .1666666666;
      int delay;
      for (delay = 1; delay <= maximumDelay; delay++) {
        if (val < crit)
          break;
        // "Cutting" and "Stretching"
        val = delayParameter * (val - crit) / (delayParameter - 1.0);
      }

      if (delay > maximumDelay)
        delay = 1;
      delays(nj, ni) = delay;
    }
  }

  return delays;
}

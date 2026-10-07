#include <stdio.h>

// INFO: ***********************************************************
/* print a Fahrenheit-Celsius table
 *  for fahr = 0, 20, ..., 300
 */
// ***********************************************************

int main(void) {
  float fahr, celsius;
  float lower, upper, step;

  lower = 0;   /* lower limit of temperature scale */
  upper = 300; /* upper limit */
  step = 20;   /* step size */

  fahr = lower;
  while (fahr <= upper) {
    celsius = 5 * (fahr - 32) / 9;
    printf("%f\t%f\n", fahr, celsius);
    fahr = fahr + step;
  }
}

// ***********************************************************
// NOTE: A more accurate output due to the increased precision
// of the floating point
// ***********************************************************
// 0.000000        -17.777779
// 20.000000       -6.666667
// 40.000000       4.444445
// 60.000000       15.555555
// 80.000000       26.666666
// 100.000000      37.777779
// 120.000000      48.888889
// 140.000000      60.000000
// 160.000000      71.111115
// 180.000000      82.222221
// 200.000000      93.333336
// 220.000000      104.444443
// 240.000000      115.555557
// 260.000000      126.666664
// 280.000000      137.777771
// 300.000000      148.888885

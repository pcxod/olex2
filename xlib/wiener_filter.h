/******************************************************************************
* Copyright (c) 2004-2026 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#ifndef __olx_wiener_filter_H
#define __olx_wiener_filter_H
#include <vector>
#include <algorithm>
#include <cstddef>

/* Wiener weights per sin(theta)/lambda shell for difference map coefficients:
c = S/(S + sigma_w(F)^2), S = max(<|dF|^2 - sigma_w(F)^2>, 0), sigma_w(F) =
1/(2F sqrt(w)) from the refined weights (deposited sigmas are too small at low
angle); no model term, so it cannot paint in non-spherical density.
equal-count shells: numpy.quantile edges + searchsorted, as a Python check does
*/
namespace olx_wiener {
  struct shell_info {
    double stl_max,  // the shell's upper sin(theta)/lambda
      S,  // signal power
      noise,  // <sigma_w(F)^2>
      c;  // mean weight
    size_t n;
  };

  // returns c per reflection, the shell table goes to shells_out if given
  inline std::vector<double> shell_weights(const std::vector<double>& stl,
    const std::vector<double>& dF2, const std::vector<double>& sw2,
    size_t shell_n, std::vector<shell_info>* shells_out = 0)
  {
    const size_t n = stl.size();
    std::vector<double> c(n, 1.0);
    if (n == 0) {
      return c;
    }
    // a shell needs some reflections for its mean to mean anything
    shell_n = (std::max)(size_t(1), (std::min)(shell_n, n / 10));
    std::vector<double> s(stl);
    std::sort(s.begin(), s.end());
    std::vector<double> edges(shell_n - 1);
    for (size_t k = 1; k < shell_n; k++) {
      const double p = (double)k / shell_n * (n - 1);
      const size_t i = (size_t)p;
      edges[k - 1] = i + 1 < n ? s[i] + (p - i) * (s[i + 1] - s[i]) : s[n - 1];
    }
    std::vector<size_t> sh(n);
    std::vector<double> S(shell_n, 0), N(shell_n, 0), cnt(shell_n, 0);
    for (size_t i = 0; i < n; i++) {
      sh[i] = std::lower_bound(edges.begin(), edges.end(), stl[i]) - edges.begin();
      S[sh[i]] += dF2[i] - sw2[i];
      N[sh[i]] += sw2[i];
      cnt[sh[i]]++;
    }
    for (size_t k = 0; k < shell_n; k++) {
      S[k] = cnt[k] > 0 ? (std::max)(S[k] / cnt[k], 0.0) : 0;
    }
    std::vector<double> csum(shell_n, 0);
    for (size_t i = 0; i < n; i++) {
      const double t = S[sh[i]] + sw2[i];
      c[i] = t > 0 ? S[sh[i]] / t : 0;
      csum[sh[i]] += c[i];
    }
    if (shells_out != 0) {
      shells_out->resize(shell_n);
      for (size_t k = 0; k < shell_n; k++) {
        shell_info& si = (*shells_out)[k];
        si.stl_max = k + 1 < shell_n ? edges[k] : s[n - 1];
        si.S = S[k];
        si.n = (size_t)cnt[k];
        si.noise = cnt[k] > 0 ? N[k] / cnt[k] : 0;
        si.c = cnt[k] > 0 ? csum[k] / cnt[k] : 0;
      }
    }
    return c;
  }
}  // end namespace olx_wiener
#endif

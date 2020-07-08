#include "symb.h"
#include <cstdio>
#include <random>
#include <cmath>

// numerical evaluation of derivative.
template<typename F>
double deriv(const F& func, double x)
{
  constexpr double eps = std::numeric_limits<double>::epsilon();
  double dx = std::cbrt(eps);

  return (func(x+dx) - func(x-dx)) / (2.0*dx);
}

// The function to be integrated
double func(double x)
{
  return std::cos(x)-std::tan(x);
}

int main()
{
  std::vector<double> xs = {0.2, 0.5, 0.9, 1.5, 2.0, 3.0};

  std::mt19937 rng(15);
  std::uniform_int_distribution<int> uniform;

  fprintf(stderr, "Search started\n");
  for (int attempt = 1; ; ++attempt) {

    int len = uniform(rng)%20 + 2;
    auto expr = symb::GenExpr(len);
    auto comp_expr = symb::Compile(expr);

    double loss = 0.0;
    for (auto x : xs) {
      double pred_deriv = deriv( [&comp_expr](double y) {
          return symb::Run(comp_expr, y); },
          x);

      double delta = pred_deriv - func(x);
      loss += delta * delta;
    }

    if (loss < 1e-5) {
      printf("%d: %s loss: %.15e\n", attempt, expr.c_str(), loss);
      break;
    }

    if (attempt%500'000 == 0) {
      fprintf(stderr, "%10dk attemps\n", attempt/1000);
    }
  }
}


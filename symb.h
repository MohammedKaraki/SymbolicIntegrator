#include <vector>
#include <string>

namespace symb {
  // Each character in an expression corresponds to a function pointer of this
  // type.
  typedef void(*FuncPtr)();

  // Generate random expression of a given length. Example: "ab+x/".
  std::string GenExpr(int len);

  // Compile an expression. That is, convert a vector of symbols
  // into a vector of the corresponding function addresses.
  std::vector<FuncPtr> Compile(const std::string& expr);

  void SetParams(double a, double b, double c);

  // Run a compiled expression and return the resulting number.
  double Run(const std::vector<FuncPtr>& compiled_expr);
  double Run(const std::vector<FuncPtr>& compiled_expr, double x);
  double Run(const std::vector<FuncPtr>& compiled_expr, double x, double y);
  double Run(const std::vector<FuncPtr>& compiled_expr, double x, double y,
             double z);
}

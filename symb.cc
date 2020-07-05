#include <cstdio>
#include <array>
#include <vector>
#include <cmath>
#include <map>
#include <random>
#include <boost/circular_buffer.hpp>

constexpr double pi = 3.141592653589793;

constexpr int max_num_params = 3;
constexpr int max_num_vars= 3;
std::array<double, max_num_params> params;
std::array<double, max_num_vars> vars;

constexpr int stack_capacity = 50;
boost::circular_buffer<double> stack(stack_capacity);


typedef void(*FuncPtr)();

// available operator implementations
namespace ops {
  template<typename F>
  void unary(const F& f)
  {
    f(stack.back());
  }

  template<typename F>
  void binary(const F& f)
  {
    double back = stack.back();
    stack.pop_back();
    f(stack.back(), back);
  }

  // nullary operators
  void a() { stack.push_back(params[0]); }
  void b() { stack.push_back(params[1]); }
  void c() { stack.push_back(params[2]); }
  void x() { stack.push_back(vars[0]); }
  void y() { stack.push_back(vars[1]); }
  void z() { stack.push_back(vars[2]); }
  void zero() { stack.push_back(0.0); }
  void one() { stack.push_back(1.0); }
  void pi() { stack.push_back(::pi); }

  // unary operators
  void invert() { unary([](double& a) { a = 1.0 / a; }); }
  void invert_sign() { unary([](double& a) { a *= -1.0; }); }
  void increment() { unary([](double& a) { a += 1.0; }); }
  void decrement() { unary([](double& a) { a -= 1.0; }); }
  void sin() { unary([](double& a) { a = std::sin(a); }); }
  void cos() { unary([](double& a) { a = std::cos(a); }); }
  void tan() { unary([](double& a) { a = std::tan(a); }); }
  void square() { unary([](double& a) { a *= a; }); }
  void root() { unary([](double& a) { a = std::sqrt(a); }); }

  // binary operators
  void add() { binary([](double& a, const double& b) { a += b; }); }
  void subtract() { binary([](double& a, const double& b) { a -= b; }); }
  void multiply() { binary([](double& a, const double& b) { a *= b; }); }
  void divide() { binary([](double& a, const double& b) { a /= b; }); }
}

// to generate random expressions
std::mt19937 rng(14);
std::uniform_int_distribution<int> uniform;

// random expression generation pools
std::array pool_0 = {'1', 'a', 'x', 'P'};
std::array pool_1 = {'\\', '~', '>', '<', 'S', 'C', '2', 'R'};
std::array pool_2 = {'+', '-', '/', '*'};

// returns a random member of an array
template<typename T>
static char rand_draw(const T& arr)
{
  return arr[uniform(rng) % arr.size()];
}

std::string gen_expr(int len)
{
  std::string result;

  int stack_size = 0;
  for (int i = 0; i < len; ++i) {
    int roof = stack_size>=2 ? 3 : stack_size+1;
    int choice = uniform(rng) % roof;

    if (i == len-1)
    {
      if (stack_size == 1)
        choice = 1;
      else
        choice = 2;
    }

    switch (choice) {
      case 0:
        result += rand_draw(pool_0);
        stack_size++;
        break;
      case 1:
        result += rand_draw(pool_1);
        break;
      case 2:
        result += rand_draw(pool_2);
        stack_size--;
        break;
    }
  }

  while (stack_size > 1) {
    result += rand_draw(pool_2);
    stack_size--;
  }

  return result;
}

// dictionary from symbol of operator to the corresponding function address.
// For example, '+' maps to &add.
std::map<char, FuncPtr> op_dict = []() {
  std::map<char, FuncPtr> ret;
  ret['a'] = ops::a;
  ret['b'] = ops::b;
  ret['c'] = ops::c;
  ret['x'] = ops::x;
  ret['y'] = ops::y;
  ret['z'] = ops::z;
  ret['0'] = ops::zero;
  ret['1'] = ops::one;
  ret['P'] = ops::pi;

  ret['\\'] = ops::invert; // x -> 1/x
  ret['~'] = ops::invert_sign; // x -> -x
  ret['>'] = ops::increment; // x -> x+1
  ret['<'] = ops::decrement; // ...etc.
  ret['S'] = ops::sin;
  ret['C'] = ops::cos;
  ret['T'] = ops::tan;
  ret['2'] = ops::square;
  ret['R'] = ops::root;

  ret['+'] = ops::add;
  ret['-'] = ops::subtract;
  ret['*'] = ops::multiply;
  ret['/'] = ops::divide;

  return ret;
}();

// compile an expression. I.e., convert a vector of operation symbols
// into a vector of the corresponding function addresses
std::vector<FuncPtr> compile(const std::string& expr)
{
  std::vector<FuncPtr> ret;

  for (char c : expr) {
    ret.push_back(op_dict[c]);
  }

  return ret;
}


// run a compiled expression and return the resulting number
double run(const std::vector<FuncPtr>& compiled_expr)
{
  for (auto func : compiled_expr) {
    func();
  }

  double ret = stack.back();
  stack.pop_back();
  return ret;
}

double run(const std::vector<FuncPtr>& compiled_expr, double x)
{
  vars[0] = x;
  return run(compiled_expr);
}


int main(int argc, char *argv[])
{
  if (argc != 2) {
    printf("Usage: %s expression\n", argv[0]);
    return 1;
  }

  auto expr = compile(argv[1]);

  double integral = 0.0;
  double dx = 1e-8;
  for (double x = 0.0; x <= pi/2.0; x += dx) {
    integral += sin(x) + cos(x) + tan(x);
    // integral += run(expr, x);
  }
  integral *= dx;

  printf("Integral from x=0.0 to x=pi: %f\n", integral);
}

#include "symb.h"
#include <array>
#include <vector>
#include <string>
#include <map>
#include <cmath>
#include <random>
#include <boost/circular_buffer.hpp>


namespace symb {
  constexpr double pi = 3.141592653589793;

  // Currently, params and vars should be hard-coded.
  constexpr int max_num_params = 3; // params are a, b, c.
  constexpr int max_num_vars= 3; // vars are x, y, z.
  std::array<double, max_num_params> params;
  std::array<double, max_num_vars> vars;

  // the stack stores the operands.
  constexpr int stack_capacity = 50;
  boost::circular_buffer<double> stack(stack_capacity);

  // Available operator implementations,
  // and currently can only be specified in this file.
  namespace operators {
    template<typename F>
    void ApplyUnary(const F& f)
    {
      f(stack.back());
    }

    template<typename F>
    void ApplyBinary(const F& f)
    {
      double back = stack.back();
      stack.pop_back();
      f(stack.back(), back);
    }

    // nullary operators
    void a() { stack.push_back(params[0]); } // push param to stack
    void b() { stack.push_back(params[1]); } // ...
    void c() { stack.push_back(params[2]); } // ...
    void x() { stack.push_back(vars[0]); }   // push var to stack
    void y() { stack.push_back(vars[1]); }   // ...
    void z() { stack.push_back(vars[2]); }   // ...
    void Zero() { stack.push_back(0.0); }    // push the number 0
    void One() { stack.push_back(1.0); }     // push the number 1
    void Pi() { stack.push_back(symb::pi); } // push the number pi

    // unary operators
    void Invert() { ApplyUnary([](double& v) { v = 1.0 / v; }); }
    void InvertSign() { ApplyUnary([](double& v) { v *= -1.0; }); }
    void Increment() { ApplyUnary([](double& v) { v += 1.0; }); }
    void Decrement() { ApplyUnary([](double& v) { v -= 1.0; }); }
    void Sin() { ApplyUnary([](double& v) { v = std::sin(v); }); }
    void Cos() { ApplyUnary([](double& v) { v = std::cos(v); }); }
    void Tan() { ApplyUnary([](double& v) { v = std::tan(v); }); }
    void Square() { ApplyUnary([](double& v) { v *= v; }); }
    void Root() { ApplyUnary([](double& v) { v = std::sqrt(v); }); }
    void Log() { ApplyUnary([](double& v) { v = std::log(v); }); }
    void Half() { ApplyUnary([](double& v) { v /= 2.0; }); }

    // binary operators
    void Add() { ApplyBinary([](double& a, const double& b) { a += b; }); }
    void Subtract() { ApplyBinary([](double& a, const double& b) { a -= b; }); }
    void Multiply() { ApplyBinary([](double& a, const double& b) { a *= b; }); }
    void Divide() { ApplyBinary([](double& a, const double& b) { a /= b; }); }
  }

  // to generate random expressions
  std::mt19937 rng(14);
  std::uniform_int_distribution<int> uniform;

  // random expression generation pools
  std::array nullary_pool = {'1', 'x', 'P'};
  std::array unary_pool = {'\\', '~', '>', '<', 'C', 'S', '2', 'R', 'L', 'H'};
  std::array binary_pool = {'+', '-', '/', '*'};

  // returns a random member of an array
  template<typename T>
  static char RandDraw(const T& arr)
  {
    return arr[uniform(rng) % arr.size()];
  }

  std::string GenExpr(int len)
  {
    std::string result;

    int stack_size = 0; // Used to ensure that the generated expression,
                        // after getting fully executed, leaves a stack
                        // with size 1, i.e., a stack containing only
                        // the return value.

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
          result += RandDraw(nullary_pool);
          stack_size++;
          break;
        case 1:
          result += RandDraw(unary_pool);
          break;
        case 2:
          result += RandDraw(binary_pool);
          stack_size--;
          break;
      }
    }

    while (stack_size > 1) {
      result += RandDraw(binary_pool);
      stack_size--;
    }

    return result;
  }

  std::map<char, FuncPtr> operator_dict = []() {
    std::map<char, FuncPtr> ret;
    ret['a'] = operators::a;
    ret['b'] = operators::b;
    ret['c'] = operators::c;
    ret['x'] = operators::x;
    ret['y'] = operators::y;
    ret['z'] = operators::z;
    ret['0'] = operators::Zero;
    ret['1'] = operators::One;
    ret['P'] = operators::Pi;

    ret['\\'] = operators::Invert; // x -> 1/x
    ret['~'] = operators::InvertSign; // x -> -x
    ret['>'] = operators::Increment; // x -> x+1
    ret['<'] = operators::Decrement; // ...etc.
    ret['S'] = operators::Sin;
    ret['C'] = operators::Cos;
    ret['T'] = operators::Tan;
    ret['2'] = operators::Square;
    ret['R'] = operators::Root;
    ret['L'] = operators::Log;
    ret['H'] = operators::Half;

    ret['+'] = operators::Add;
    ret['-'] = operators::Subtract;
    ret['*'] = operators::Multiply;
    ret['/'] = operators::Divide;

    return ret;
  }();

  void SetParam(double a, double b, double c)
  {
    params[0] = a;
    params[1] = b;
    params[2] = c;
  }

  std::vector<FuncPtr> Compile(const std::string& expr)
  {
    std::vector<FuncPtr> ret;

    for (char c : expr) {
      ret.push_back(operator_dict[c]);
    }

    return ret;
  }

  double Run(const std::vector<FuncPtr>& compiled_expr)
  {
    for (auto func : compiled_expr) {
      func();
    }

    double ret = stack.back();
    stack.pop_back();
    return ret;
  }

  double Run(const std::vector<FuncPtr>& compiled_expr, double x)
  {
    vars[0] = x;
    return Run(compiled_expr);
  }

  double Run(const std::vector<FuncPtr>& compiled_expr, double x, double y)
  {
    vars[0] = x;
    vars[1] = y;
    return Run(compiled_expr);
  }

  double Run(const std::vector<FuncPtr>& compiled_expr, double x, double y,
             double z)
  {
    vars[0] = x;
    vars[1] = y;
    vars[2] = z;
    return Run(compiled_expr);
  }
}

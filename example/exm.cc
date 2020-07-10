#include<stdio.h>
#include<pybind11/pybind11.h>
namespace py = pybind11;

int add (int a, int b)
{
  return a+b;
}
PYBIND11_MODULE(example,m)
{
  //optional module docstring
  m.doc() = "pybind11 example plugin"
    //expose add function, and add ketword arguments and default arguments
  m.def("add",&add,"A function which adds two numbers",py::arg("a"=1,py::arg("b")=2));
  //exporting variables
  m.attr("the_answer")=42;
  py::object world = py::cast("World");
  m.attr("what")= world;
}



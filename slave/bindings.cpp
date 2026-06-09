#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include "calculator.hpp"

namespace py = pybind11;

PYBIND11_MODULE(calculator, m) {
    m.doc() = "Calculator module implemented in C++"; // Module docstring

    m.def("add", &calculator::add, "Add two numbers");
    m.def("subtract", &calculator::subtract, "Subtract two numbers");
    m.def("multiply", &calculator::multiply, "Multiply two numbers");
    m.def("divide", &calculator::divide, "Divide two numbers");
    //////////////////////////nuevo///////////////////////
  
    m.def("fill", &calculator::fill);
  
    m.def("paddling", &calculator::paddling);

    m.def("split", &calculator::split);

    py::class_<calculator::UDPClient>(m, "UDPClient")
      .def(py::init<std::string, int>())
      .def("send_data", &calculator::UDPClient::send_data);
    
    m.def("buildProtocol", &calculator::buildProtocol);
} 
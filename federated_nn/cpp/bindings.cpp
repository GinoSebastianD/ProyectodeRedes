#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/functional.h>
#include "rdt.h"

namespace py = pybind11;



PYBIND11_MODULE(comm_module, m) {
    m.doc() = "Módulo RDT sobre UDP – aprendizaje federado";

    py::class_<rdt::RDTNode>(m, "RDTNode")
        .def(py::init<int, uint16_t, rdt::LogFn>(),
             py::arg("listen_port"),
             py::arg("my_node_id") = static_cast<uint16_t>(0),
             py::arg("log_fn")     = rdt::LogFn{},
             "Nodo RDT escuchando en listen_port.\n"
             "my_node_id: 0=maestro, 1..N=esclavos\n"
             "log_fn: callback Python para display (pasa print)")

        //Maestro envía pesos al esclavo (TYPE = 'M')
        .def("send_matrix",
            [](rdt::RDTNode& self, const std::string& ip, int port,
               uint16_t dest_id, py::bytes data) -> int {
                std::string raw(data);
                return self.send(ip, port, dest_id, rdt::TYPE_MATRIX,
                    std::vector<uint8_t>(raw.begin(), raw.end()));
            },
            py::arg("dest_ip"), py::arg("dest_port"),
            py::arg("dest_id"), py::arg("data"),
            "Maestro → Esclavo: envía matriz de pesos (TYPE='M')")

        //Maestro envía porción de dataset al esclavo (TYPE = 'D')
        .def("send_data",
            [](rdt::RDTNode& self, const std::string& ip, int port,
               uint16_t dest_id, py::bytes data) -> int {
                std::string raw(data);
                return self.send(ip, port, dest_id, rdt::TYPE_DATA,
                    std::vector<uint8_t>(raw.begin(), raw.end()));
            },
            py::arg("dest_ip"), py::arg("dest_port"),
            py::arg("dest_id"), py::arg("data"),
            "Maestro → Esclavo: envía porción del dataset (TYPE='D')")

        //Esclavo devuelve pesos al maestro (TYPE = 'm')
        .def("send_matrix_slave",
            [](rdt::RDTNode& self, const std::string& ip, int port,
               uint16_t dest_id, py::bytes data) -> int {
                std::string raw(data);
                return self.send(ip, port, dest_id, rdt::TYPE_MATRIX_SLAVE,
                    std::vector<uint8_t>(raw.begin(), raw.end()));
            },
            py::arg("dest_ip"), py::arg("dest_port"),
            py::arg("dest_id"), py::arg("data"),
            "Esclavo → Maestro: devuelve pesos actualizados (TYPE='m')")

        // Recibir cualquier mensaje
   
        .def("recv_any",
            [](rdt::RDTNode& self, int timeout_ms) -> py::object {
                auto msg = self.recv(timeout_ms);
                if (msg.empty()) return py::none();
                return py::make_tuple(
                    std::string(1, static_cast<char>(msg.type)),
                    py::bytes(reinterpret_cast<const char*>(msg.data.data()),
                              msg.data.size()));
            },
            py::arg("timeout_ms") = 20000,
            "Recibe un mensaje completo.\n"
            "Retorna (tipo:str, bytes) o None en timeout.\n"
            "  tipo='M' → pesos del maestro\n"
            "  tipo='D' → dataset\n"
            "  tipo='m' → pesos del esclavo");
}

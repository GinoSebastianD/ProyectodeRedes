
from setuptools import setup, Extension
import pybind11

sources = [
    "udp_socket.cpp",
    "rdt.cpp",
    "bindings.cpp",
]

ext = Extension(
    name="comm_module",
    sources=sources,
    include_dirs=[pybind11.get_include()],
    language="c++",
    extra_compile_args=[
        "-std=c++17",
        "-O2",
        "-Wall",
        "-Wextra",
        "-fvisibility=hidden",   
    ],
)

setup(
    name="comm_module",
    version="1.0.0",
    description="Módulo RDT/UDP para aprendizaje federado",
    ext_modules=[ext],
    install_requires=["pybind11>=2.10.0"],
    python_requires=">=3.8",
)

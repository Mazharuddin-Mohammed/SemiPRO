# distutils: language = c++
# distutils: sources = ../cpp/renderer/vulkan_renderer.cpp ../cpp/core/wafer.cpp

from libcpp.memory cimport shared_ptr
from libcpp cimport bool
from cython.operator cimport dereference as deref

cdef extern from "../cpp/core/wafer.hpp":
    cppclass Wafer:
        pass

cdef extern from "../cpp/renderer/vulkan_renderer.hpp":
    cppclass VulkanRenderer:
        VulkanRenderer(unsigned int, unsigned int) except +
        void initialize() except +
        void render(shared_ptr[Wafer]) except +
        void* getWindow()

cdef class PyVulkanRenderer:
    cdef VulkanRenderer* thisptr
    def __cinit__(self, width: int, height: int):
        self.thisptr = new VulkanRenderer(width, height)
    def __dealloc__(self):
        del self.thisptr
    def initialize(self):
        self.thisptr.initialize()
    def render(self, wafer: PyWafer):
        self.thisptr.render(wafer.thisptr)
    def get_window(self):
        return <size_t>self.thisptr.getWindow()
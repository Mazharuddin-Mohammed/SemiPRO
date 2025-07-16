# Minimal Cython test module to validate integration
# Author: Dr. Mazharuddin Mohammed

from libcpp.string cimport string
from libcpp.memory cimport shared_ptr
import numpy as np

# Simple test function
def test_cython_integration():
    """Test basic Cython functionality"""
    return "Cython integration working!"

def test_numpy_integration():
    """Test NumPy integration"""
    arr = np.array([1, 2, 3, 4, 5])
    return f"NumPy array sum: {np.sum(arr)}"

# Simple C++ string test
cdef extern from "<string>" namespace "std":
    cppclass string:
        string()
        string(const char*)
        const char* c_str()

def test_cpp_string():
    """Test C++ string integration"""
    cdef string cpp_str = string(b"Hello from C++!")
    return cpp_str.c_str().decode('utf-8')

# Test class
cdef class SimpleTest:
    cdef double value
    
    def __init__(self, double val):
        self.value = val
    
    def get_value(self):
        return self.value
    
    def multiply(self, double factor):
        return self.value * factor

# Test function that uses the class
def create_test_object(double val):
    """Create and test a simple Cython class"""
    test_obj = SimpleTest(val)
    return {
        'original': test_obj.get_value(),
        'doubled': test_obj.multiply(2.0),
        'squared': test_obj.multiply(test_obj.get_value())
    }

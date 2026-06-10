# pybind11 >= 2.12 is required for numpy 2.x support; older system packages
# (e.g. Ubuntu's 2.11) segfault in the Eigen type casters under numpy 2.
find_package(pybind11 2.12 CONFIG QUIET)
if(NOT pybind11_FOUND)
  include(FetchContent)
  FetchContent_Declare(
    pybind11
    URL https://github.com/pybind/pybind11/archive/refs/tags/v3.0.4.tar.gz
    URL_HASH SHA256=74b6a2c2b4573a400cafb6ecbf60c98df300cd3d0041296b913d02b2cbbb2676
  )
  FetchContent_MakeAvailable(pybind11)
endif()

# NO_EXTRAS: skip pybind11's LTO/strip extras — unnecessary for a thin wrapper
# and it keeps link times short.
pybind11_add_module(localization_zoo_py NO_EXTRAS src/localization_zoo_py.cpp)
set_target_properties(localization_zoo_py PROPERTIES OUTPUT_NAME localization_zoo)
target_link_libraries(localization_zoo_py PRIVATE kiss_icp litamin2 ndt gicp)

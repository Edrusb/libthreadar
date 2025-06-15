noinst_PROGRAMS = hello_world test_barrier stack_sizer
dist_noinst_DATA = fast_tampon_example.cpp thread_example.cpp barrier_example.cpp condition_example.cpp

LDADD = -L../../src -lthreadar

hello_world_SOURCES = hello_world.cpp
hello_world_DEPENDENCIES = ../../src/libthreadar.la

test_barrier_SOURCES = test_barrier.cpp
test_barrier_LDFLAGS = -all-static
test_barrier_DEPENDENCIES = ../../src/libthreadar.la

stack_sizer_SOURCES = stack_sizer.cpp
stack_sizer_LDFLAGS = -all-static
stack_sizer_DEPENDENCIES = ../../src/libthreadar.la

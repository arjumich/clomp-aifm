# AIFM RUNTIME CONFIGURATION
AIFM_PATH=../../AIFM
SHENANGO_PATH=$(AIFM_PATH)/shenango

include $(SHENANGO_PATH)/shared.mk
librt_libs = $(SHENANGO_PATH)/bindings/cc/librt++.a
libaifm = $(AIFM_PATH)/aifm/libaifm.a

INC += -I$(SHENANGO_PATH)/bindings/cc -I$(AIFM_PATH)/aifm/inc -I$(SHENANGO_PATH)/ksched -I$(SHENANGO_PATH)/inc/runtime
LDFLAGS += -lmlx5 -lpthread


CXXFLAGS := $(filter-out -std=gnu++17,$(CXXFLAGS))
override CXXFLAGS += -std=gnu++2a -fconcepts -Wno-unused-function -mcmodel=medium
CXXFLAGS += -g -fopenmp -O0 -I. $(INC)

LDFLAGS += -g -O0 

all:
	g++-9 $(CXXFLAGS) clomp.cpp $(librt_libs) $(LDFLAGS) $(libaifm) $(RUNTIME_LIBS) -lm -o clomp $(librt_libs) $(RUNTIME_LIBS)

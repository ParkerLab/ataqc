#
# VARIABLES
#

VERSION = 0.1.2

# PATHS
SRC_DIR = src
CPP_DIR = $(SRC_DIR)/cpp
SCRIPTS_DIR = $(SRC_DIR)/scripts
BUILD_DIR = build

SRC_CPP := $(wildcard $(CPP_DIR)/*.cpp)
OBJECTS := $(patsubst $(CPP_DIR)/%.cpp,$(BUILD_DIR)/%.o,$(SRC_CPP))
SCRIPTS := $(wildcard $(SCRIPTS_DIR)/*)

INCLUDES  := $(addprefix -I,$(SRC_DIR))

MODULES_ROOT = /lab/sw/modules

# template for environment module file
define MODULEFILE
#%Module1.0
set           version          $(VERSION)
set           app              ataqc
set           modroot          $(MODULES_ROOT)/$$app/$$version
setenv        ATAQC_MODULE     $$modroot

prepend-path  PATH             $$modroot/bin

conflict      ataqc

module-whatis "ATAC-seq QC toolkit, version $(VERSION)."
module-whatis "URL:  https://github.com/ParkerLab/ataqc"
module-whatis "Installation directory: $$modroot"
endef
export MODULEFILE


#
# FLAGS
#
CXXFLAGS = -std=c++11 -I.

UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Linux)  # Lab server: RHEL6
	HTSLIB = $(MODULES_ROOT)/htslib/1.2.1
	CXXFLAGS += -D LINUX -pthread -O3 -g
	INCLUDES += -I $(HTSLIB)/include
	LDFLAGS = -static # -Wl,-rpath $(HTSLIB)/lib
	LDLIBS = -L $(HTSLIB)/lib -lhts -lz
	PREFIX = $(MODULES_ROOT)/ataqc/$(VERSION)
	MODULEFILES_ROOT = /lab/sw/modulefiles
	MODULEFILE_PATH = $(MODULEFILES_ROOT)/ataqc/$(VERSION)
endif
ifeq ($(UNAME_S),Darwin)  # Mac with Homebrew
	CXXFLAGS += -D OSX -O3
	INCLUDES += -I /usr/local/Cellar/htslib/1.2.1/include/htslib/
	LDLIBS = -L /usr/local/Cellar/htslib/1.2.1/lib/ -lhts
	PREFIX = /usr/local
endif

UNAME_P := $(shell uname -p)
ifeq ($(UNAME_P),x86_64)
	CXXFLAGS += -D AMD64
endif
ifneq ($(filter %86,$(UNAME_P)),)
	CXXFLAGS += -D IA32
endif
ifneq ($(filter arm%,$(UNAME_P)),)
	CXXFLAGS += -D ARM
endif

#
# TARGETS
#

all: checkdirs $(BUILD_DIR)/ataqc

checkdirs: $(BUILD_DIR)

$(BUILD_DIR):
	@mkdir -p $@

$(BUILD_DIR)/ataqc: $(OBJECTS)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -o $@ $^ $(LDLIBS) $(LDFLAGS)

$(BUILD_DIR)/%.o: $(CPP_DIR)/%.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -o $@ -c $<

clean:
	@rm -rf $(BUILD_DIR)

install: checkdirs install-ataqc install-scripts

install-ataqc: $(BUILD_DIR)/ataqc
	strip $(BUILD_DIR)/ataqc
	install -d -m 0755 $(PREFIX)/bin
	install -m 0755 build/ataqc $(PREFIX)/bin

install-scripts: $(SCRIPTS)
	install -m 0755 $^ $(PREFIX)/bin

ifdef MODULEFILE_PATH
install-module:
	mkdir -p `dirname $(MODULEFILE_PATH)`
	echo "$$MODULEFILE" > $(MODULEFILE_PATH)
endif

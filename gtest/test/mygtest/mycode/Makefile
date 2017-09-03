
#CXX = arm-poky-linux-gnueabi-g++ 
CXX = g++ 

# Where to find user code.
#USER_DIR = ipcuart
USER_DIR = didata
#USER_DIR = read_mcu

#coverage and gtest reports directory
OUTPUT = output

#shell file : generate coverage reports
GCOVR_HTML = gcovr_html.sh

GTEST_BIN=didata_unittest

#########################################
#	parameter	
#########################################
# Flags passed to the preprocessor.
CPPFLAGS := -fprofile-arcs -ftest-coverage -fPIC -O2 

# Flags passed to the C++ compiler.
CXXFLAGS := -g -Wextra -pthread -fpermissive 

# Flags passed to the C compiler.
CFLAGS := -g -Wextra -pthread

# Flag passed to linker.
LIBS := -ldbus-glib-1 -ldbus-1 -lgobject-2.0 -lglib-2.0 -lpthread -lrt
LIBS += ../lib/gtest_main.a 
LIBS += -lgcov 

export CPPFLAGS CXXFLAGS CFLAGS

.PHONY:all
all : $(GTEST_BIN)
	@echo "***** begin generate *****"
	/bin/bash ./$(GCOVR_HTML) $(USER_DIR) $^


$(GTEST_BIN) : $(USER_DIR)
	@echo "***** begin ld ****" 
	$(CXX) $^/*.o -o $^/$@ $(LIBS) 

$(USER_DIR) : echo 
	@$(MAKE) -C $@ 

.PHONY:echo
echo :
	@echo "***** begin compile *****"


.PHONY:clean
clean:
	rm -f $(OUTPUT)/* *.gcda $(USER_DIR)/*.gcda $(USER_DIR)/*.gcno 
	@cd $(USER_DIR) && make clean


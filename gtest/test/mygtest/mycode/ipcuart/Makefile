
#CXX = arm-poky-linux-gnueabi-g++ 
CXX = g++ 

# Flags passed to the preprocessor.
CPPFLAGS := -c -g -fprofile-arcs -ftest-coverage -fPIC -O0 -fpermissive

# Flags passed to the C++ compiler.
CXXFLAGS := -g -Wextra -pthread -fpermissive

export CPPFLAGS CXXFLAGS


LIBS := -ldbus-glib-1 -ldbus-1 -lgobject-2.0 -lglib-2.0 -lpthread -lrt
LIBS += ../../lib/gtest_main.a 
LIBS += -fPIC -lgcov 

#target
TARGET = uart_unittest

GCOVR_HTML = gcovr_html.sh

#target makefile
PRO_MAKE = Makefile_ori

.PHONY:all
all : $(TARGET)

OBJ_DIR = .

.PHONY:$(OBJ_DIR)
$(OBJ_DIR) : 
	$(MAKE) -f $(PRO_MAKE) 

$(TARGET) : $(OBJ_DIR)
	$(CXX) *.o $(LIBS) -o $@ 
	@./$(GCOVR_HTML) $@


.PHONY:clean
clean:
	rm -f $(TARGET) *.gcno *.gcda *.html
	@make -f $(PRO_MAKE) clean

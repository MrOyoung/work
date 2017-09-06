
vpath %.h /usr/include

CC=gcc

TARGET=main

override CFLAGS+=-fgnu89-inline

OBJS:=main.o
OBJS+=hello.o

export OBJS

#条件编译 ifeq
ifeq (0,${MAKELEVEL})
cur-dir:=$(shell pwd)
whoami:=$(shell whoami)
host-type:=$(shell arch)
#MAKE:=$(MAKE) host-type=$(host-type) whoami=$(whoami)
endif

$(TARGET):$(OBJS)
	$(CC) $^ -o $@ 
	$(MAKE) -C mk


$(OBJS):%.o:%.c
	$(CC) $(CFLAGS) -c $< -o $@

#包含.d文件，搜索并根据规则生成.d文件
-include $(OBJS:.o=.d)

#create dependency file *.d
%.d:%.c
	@set -e; rm -f $@;\
	$(CC) -MM $(CPPFLAGS) $< > $@.;\
		sed 's,\($*\)\.o[:]*,\1.o $@:,g' < $@. > $@;\
		rm -f $@.

#定义命令包 define cmd pkg
define run-echo
@echo $@
@echo $@
endef
.PHONY:foo.c
foo.c:
	$(run-echo)


#条件编译 ifdef测试一个变量是否有值
ifdef OBJS
object_file=yes
else
object_file=no
endif
.PHONY:ifdef_test
ifdef_test:
	@echo $(object_file)

hello=hello
world=world
#产生空格字符
nullstring:=
space:=$(nullstring) #end of the line
.PHONY:space_test
space_test:
	@echo $(hello)$(space)$(world)


#subst字符替换函数
string=aabbccdd
output=$(subst b,a,$(string))
.PHONY:subst_test
subst_test:
	@echo $(output)
	
#patsubst 模式字符串替换函数
output=$(patsubst %.c,%.o,func.c main.c)
.PHONY:patsubst_test
patsubst_test:
	@echo $(output)

#findstring 查找字符串函数
aa_define=$(findstring aa,aabbccdd)
.PHONY:findstring_test
findstring_test:
	@echo $(aa_define)

#figter 过滤函数
output=$(filter %.c %.o,main.c main.o test.S)
.PHONY:filter_test
filter_test:
	@echo $(output)

######################################################
#  目录操作函数
######################################################

#dir  取目录函数
output=$(dir src/foo.c main.c)
.PHONY:dir_test
dir_test:
	@echo $(output)

#foreach函数
work_dir=.
cfile=$(foreach file,$(work_dir),$(wildcard $(work_dir)/*.c))
objs=$(patsubst %.c,%.o,$(cfile))
.PHONY:foreach_test
foreach_test:
	@echo $(objs)


#origin函数 返回变量的来源
place=$(origin OBJS)
.PHONY:origin_test
origin_test:
	@echo $(place)

.PHONY:clean
clean:
	-rm $(TARGET) *.o *.d


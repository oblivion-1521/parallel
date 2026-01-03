CXX=g++ -m64
CXXFLAGS=-I../common -Iobjs/ -O3 -Wall
ISPC=ispc
# note: change target to sse4 for SSE capable machines
ISPCFLAGS=-O2 --target=avx1-i32x8 --arch=x86-64

APP_NAME=mandelbrot_ispc  # 最终可执行文件
OBJDIR=objs  # 存放中间文件的目录
COMMONDIR=../common # 公共代码目录

# 依赖文件配置
PPM_CXX=$(COMMONDIR)/ppm.cpp
PPM_OBJ=$(addprefix $(OBJDIR)/, $(subst $(COMMONDIR)/,, $(PPM_CXX:.cpp=.o)))

# 系统任务库
TASKSYS_CXX=$(COMMONDIR)/tasksys.cpp
TASKSYS_LIB=-lpthread
TASKSYS_OBJ=$(addprefix $(OBJDIR)/, $(subst $(COMMONDIR)/,, $(TASKSYS_CXX:.cpp=.o)))

# 编译目标定义
default: $(APP_NAME)

.PHONY: dirs clean

dirs:
		/bin/mkdir -p $(OBJDIR)/

clean:
		/bin/rm -rf $(OBJDIR) *.ppm *~ $(APP_NAME)

# 对象文件列表
OBJS=$(OBJDIR)/main.o $(OBJDIR)/mandelbrotSerial.o $(OBJDIR)/mandelbrot_ispc.o $(PPM_OBJ) $(TASKSYS_OBJ)

# 可执行文件在这里生成
$(APP_NAME): dirs $(OBJS)
		$(CXX) $(CXXFLAGS) -o $@ $(OBJS) -lm $(TASKSYS_LIB)

# cpp文件编译规则
$(OBJDIR)/%.o: %.cpp
		$(CXX) $< $(CXXFLAGS) -c -o $@

$(OBJDIR)/%.o: $(COMMONDIR)/%.cpp
	$(CXX) $< $(CXXFLAGS) -c -o $@

# 依赖关系
$(OBJDIR)/main.o: $(OBJDIR)/mandelbrot_ispc.h $(COMMONDIR)/CycleTimer.h

# ispc编译规则
$(OBJDIR)/%_ispc.h $(OBJDIR)//%_ispc.o: %.ispc
		$(ISPC) $(ISPCFLAGS) $< -o $(OBJDIR)/$*_ispc.o -h $(OBJDIR)/$*_ispc.h

# 解释一下recipe：
# 命令中的变量
# $(ISPC)：ISPC 编译器的路径
# $(ISPCFLAGS)：传递给 ISPC 的编译选项
# $<：Makefile 自动变量，表示第一个依赖文件（即 .ispc 文件）
# $*：Makefile 自动变量，表示模式规则中 % 匹配的部分（不含扩展名）

# 核心是-h $(OBJDIR)/$*_ispc.h
# 可以直接通过.ispc来生成对应的头文件，这是ispc特有的功能
CXX=g++ -m64
CXXFLAGS=-I../common -Iobjs/ -O3 -Wall
ISPC=ispc
# note: change target to sse4 for SSE capable machines
ISPCFLAGS=-O2 --target=avx1-i32x8 --arch=x86-64

# --- 修正点 1: 移除行尾注释，防止引入多余空格 ---
APP_NAME=mandelbrot_ispc
OBJDIR=objs
COMMONDIR=../common

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

# --- 修正点 2: 使用 vpath 解决规则冲突 ---
# 告诉 Make 在这些目录里找 .cpp 文件
vpath %.cpp $(COMMONDIR)

# 统一的 cpp 编译规则
# 因为有了 vpath，这里只需要写一个规则即可同时匹配当前目录和 common 目录的源码
$(OBJDIR)/%.o: %.cpp
	$(CXX) $< $(CXXFLAGS) -c -o $@

$(OBJDIR)/main.o: $(OBJDIR)/mandelbrot_ispc.h $(COMMONDIR)/CycleTimer.h

# --- 修正点 3: 移除路径中多余的双斜杠 // ---
# ispc编译规则
$(OBJDIR)/%_ispc.h $(OBJDIR)/%_ispc.o: %.ispc
	$(ISPC) $(ISPCFLAGS) $< -o $(OBJDIR)/$*_ispc.o -h $(OBJDIR)/$*_ispc.h
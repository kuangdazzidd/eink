#开始满足这个条件  
ifeq ($(KERNELRELEASE),)     
#指定Linux内核目录位置  
KERNELDIR ?= /home/xdn/Desktop/lichee/linux/out/lib/modules/4.10.15-licheepi-zero+/build 
#打印当前驱动源码的本地位置  
PWD := $(shell pwd)    
#编译为驱动模块  
modules:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules  
#-C 指定内核Makefile的路径，可以使用相对路径。  
#-M 指定要编译的文件的路径，同样可以使用相对路径。  
modules_install:  
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules_install  
clean:  
	rm -rf *.o *.cmd *.ko *.mod.c .tmp_versions modules.order Module.symvers  
  
.PHONY: modules modules_install clean

#再次读取这个Makefile文件时，下面的执行  
else
#指定驱动模块的名字
MODULE_NAME := epd
# 注意：驱动模块的名字 千万 不能和本文件夹内的任何文件同名！
#指定驱动模块的核心文件（有init 和 exit）
RESMAIN_CORE_OBJS := eink.o
#依赖文件
RESMAIN_GLUE_OBJS := hc595.o
$(MODULE_NAME)-objs := $(RESMAIN_GLUE_OBJS) $(RESMAIN_CORE_OBJS)
#xxx-objs := 指定驱动模块的所有依赖文件  
#最终由xxx-objs链接生成ecat.o，再生成ecat.ko 
obj-m := epd.o
endif
# change by myself for test   
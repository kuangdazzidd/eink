# Makefile2.6  
ifneq ($(KERNELRELEASE),)  
#kbuild syntax. dependency relationshsip of files and target modules are listed here.  

mymodule-objs := epd.o
obj-m := epd.o
  
else  
PWD  := $(shell pwd)  
KDIR := /home/xdn/Desktop/lichee/linux/out/lib/modules/4.10.15-licheepi-zero+/build
all:  
	$(MAKE) -C $(KDIR) M=$(PWD)  
  
clean:  
	rm -rf *.cmd *.o *.mod.c *.ko .tmp_versions  *.order *.symvers
  
endif  
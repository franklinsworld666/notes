[TOC]

## linux 启动过程的主要过程
+ 固件阶段（BIOS/UEFI）
+ 执行 boot Loader
+ 内核初始化阶段
+ 根文件系统切换
+ 用户空间初始化


## 固件阶段

BIOS（传统）：执行 POST（加电自检），检查 CPU、内存、硬盘等硬件是否正常；完成后读取主板 BIOS 中的启动顺序（如硬盘、U 盘、光盘），从第一个有效启动设备的 MBR（主引导记录）加载引导程序。

UEFI（现代）：替代 BIOS 的新型固件，支持更大容量的启动设备（突破 MBR 的 2TB 限制），自带驱动库，可直接识别 GPT 分区；启动时读取 ESP（EFI 系统分区, FAT32格式）中的引导程序（如 Ubuntu 的启动文件可能为 /boot/efi/EFI/ubuntu/grubx64.efi）

## 执行 boot Loader

固件阶段结束后，读取并执行第一个开机设备内的引导程序（如: GRUB2、LILO）负责加载内核、文件系统（initramfs/initrd）。

以 GRUB2 为例：
+ 先加载自身的核心镜像（core.img），识别系统中的分区和内核文件（通常在/boot/vmlinuz-<version>）；
+ 读取 /boot/grub/grub.cfg 配置文件，显示启动菜单（若有多个系统）；
+ 用户选择后，GRUB2 将内核（vmlinuz）和解压后的 initramfs（初始内存文件系统）加载到内存中，然后将控制权交给内核。

UEFI 模式下：
+ 在 UEFI 模式下，grubx64.efi 被 UEFI 固件加载后，会默认读取与自身位于同一目录下的 grub.cfg 文件作为配置文件。
+ 例如，在 CentOS 系统中，UEFI 模式下 GRUB 的有效配置文件是 /boot/efi/EFI/centos/grub.cfg；


## 内核初始化阶段

内核（vmlinuz）接管后，开始初始化硬件、建立进程管理和文件系统框架，核心步骤包括：
+ 解压与自举：内核镜像本身是压缩的，首先解压并执行自举代码（head.S），初始化 CPU、内存管理（页表）、中断处理等底层机制。
+ 硬件探测与驱动加载：通过内置驱动或 initramfs 中的模块，探测并初始化硬盘、网卡、显卡等硬件（可通过dmesg查看过程）。Linux 内核会以自己的功能重新侦测一遍硬件。
+ 挂载初始根文件系统（initramfs）：内核启动时无法直接识别复杂的根文件系统（如 ext4、xfs），因此需要先加载 initramfs（内存中的临时文件系统，包含必要的驱动和工具），由 initramfs 完成后续根文件系统的准备。

## 根文件系统切换（switch_root）

initramfs 的核心任务是找到并挂载真正的根文件系统，然后切换到该系统：

+ initramfs 中的脚本（如/init 脚本）会根据内核启动参数（如root=/dev/sda1）查找根设备，加载对应的文件系统驱动（如 ext4 模块），挂载根文件系统到/sysroot；
+ 执行switch_root命令，将根目录从 initramfs 切换到/sysroot，并销毁 initramfs 释放内存。

## 用户空间初始化

根文件系统就绪后，内核启动第一个用户态进程（init），后续由 init 进程完成系统服务的启动：
+ 传统系统中，init 进程是/sbin/init（如 sysvinit），通过运行级（runlevel）启动服务（如/etc/rc.d/rc3.d）；
+ 现代系统（如 CentOS 7+、Ubuntu 16.04+）使用systemd作为 init 进程，通过target（如multi-user.target、graphical.target）管理服务，并行启动效率更高。
+ 最终启动登录管理器（如 gdm、lightdm）或命令行终端，用户可登录系统。
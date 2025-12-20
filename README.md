# 在 macOS 上编译与调试 Linux-0.11（QEMU + GDB，现代工具链可用）

本仓库基于 Linux 0.11 的源码（教学/考古用途），补齐了在现代 macOS（含 Apple Silicon）上使用较新 GCC/binutils 时常见的构建问题，并提供可复现的 QEMU + GDB 调试流程。

目标是：**让“能编译、能启动、能断点调试”在 macOS 上变成一条顺滑的路**。

> 免责声明 / 边界说明  
> - Linux 0.11 是 1991 年代码，很多写法依赖旧式 C 语义与旧工具链行为。  
> - 本仓库的改动主要为了 **现代工具链兼容**（例如 GCC 13/15），并不追求将代码全面现代化。  
> - macOS 下 Homebrew 的交叉工具链通常使用 `i686-elf-*` 前缀，而原工程常写死 `i386-elf-*`，因此需要做“前缀兼容”（见下文脚本/说明）。  
> - 如果你使用的 brew 前缀路径不同（Intel 机器通常是 `/usr/local`，Apple Silicon 通常是 `/opt/homebrew`），请按你的实际路径调整。

---

## 1. 环境要求

### 1.1 macOS 版本
- macOS 12+ 推荐（更旧也可能可行，但不保证）

### 1.2 必要工具
- Homebrew
- QEMU（用于运行 i386 虚拟机）
- i686-elf 交叉工具链（gcc + binutils）
- GDB（用于连接 QEMU 的 gdbstub，做远程调试）

---

## 2. 安装依赖（Homebrew）

### 2.1 安装 QEMU
```bash
brew install qemu
```

### **2.2 安装交叉工具链（推荐 i686-elf）**

```
brew install i686-elf-binutils i686-elf-gcc
```

安装完成后你应该能看到这些工具（至少一部分）：

```
which i686-elf-gcc
which i686-elf-as
which i686-elf-ld
which i686-elf-ar
```

### **2.3 安装 GDB（可选但推荐）**

不同 brew/环境下可能有不同名称：

- 你可能会有 i386-elf-gdb（最理想）
- 或者使用系统 gdb（只要能 target remote 即可）

可先尝试：

```
brew install gdb
```

> macOS 上 gdb 可能涉及签名（codesign）才能 attach；但我们这里是 remote 调试 QEMU（target remote :1234），通常不需要 attach 本机进程，所以一般问题较少。

## **3. 工具链前缀兼容（重点：i386-elf-\* vs i686-elf-\*）**

原工程在 Makefile 中常用 i386-elf-gcc / i386-elf-ar / i386-elf-cpp 等前缀。

但 Homebrew 很常见的是提供 i686-elf-*。

你有两种方式解决：

### **方式 A（推荐）：使用脚本创建软链接（一次配置，后续无脑 make）**

在仓库里新增脚本：tools/macos-link-toolchain.sh（建议直接用本仓库提供的版本）。

运行：

```
chmod +x tools/macos-link-toolchain.sh
./tools/macos-link-toolchain.sh
```

脚本会在你的 brew bin 目录（通常 /opt/homebrew/bin 或 /usr/local/bin）里，把 i686-elf-* 链接成 i386-elf-*。

例如：i386-elf-ar -> i686-elf-ar。

> 注意：这个操作需要你对 brew bin 目录有写权限；通常不需要 sudo。

> 如果你安装在特殊路径，脚本里可以调整 PREFIX。

### **方式 B：修改 Makefile.header / Makefile，直接把工具变量改成 i686-elf- 前缀**

这种方式更“纯粹”，但需要改动 Makefile，后续与上游同步时可能更麻烦。

如果你想尽量少改 Makefile，推荐方式 A。

## **4. 编译 Linux-0.11**

### **4.1 清理并编译**

```
make clean
make -B
```

如果成功，你通常会看到最后生成镜像的输出（dd ... records in/out），并生成 Image 文件。

检查：

```
ls -lh Image
file tools/system
```

说明：

- tools/system：带符号的 ELF（给 GDB 用）
- Image：可启动的软盘镜像（QEMU 用）

## **5. 运行（QEMU）**

### **5.1 启动**

```
make start
```

Makefile 默认会用 QEMU 启动软盘 Image，并挂载硬盘镜像 hdc-0.11.img 作为系统磁盘。

## **6. 调试（QEMU + GDB）**

### **6.1 启动调试模式（QEMU 停在起点等待 GDB）**

终端 A：

```
make debug
```

通常会使用：

- -s：在 :1234 打开 gdbstub
- -S：启动后立即暂停，等待 GDB 连接

### **6.2 连接 GDB 并下断点**

终端 B：

```
gdb tools/system
```

在 gdb 里：

```
(gdb) target remote :1234
(gdb) b main
(gdb) c
```

常用技巧：

- 查看当前指令：

```
(gdb) x/10i $eip
```

- 查看寄存器：

```
(gdb) info registers
```

- 查看栈：

```
(gdb) x/32wx $esp
```

> 注意：Linux 0.11 会经历引导阶段（bootsect/setup/head），你打 b main 的 main 指的是内核初始化阶段的 main（init/main.c）。

> 如果你想从更早的 boot 过程开始调试，需要对符号/地址更熟悉（属于进阶玩法）。

## **7. 常见错误与排障**

### **7.1 i386-elf-ar: No such file or directory**

原因：工具链前缀不匹配（你装的是 i686-elf-ar）。

解决：运行前缀兼容脚本：

```
./tools/macos-link-toolchain.s
```

### **7.2 i386-elf-cpp: No such file or directory**

原因：同上，或某些工具链未单独提供 cpp wrapper。

解决：

- 优先用脚本建立软链接；
- 若仍无 i686-elf-cpp，可以在脚本里为 i386-elf-cpp 创建 wrapper，用 i386-elf-gcc -E 替代（本仓库脚本可覆盖该情况）。

### **7.3 implicit declaration of function ...**

### **（如 memset/strncpy/strchr）**

原因：Linux 0.11 自带头文件在现代编译器下会触发更严格检查；或头文件原型与实现签名不一致。

解决：本仓库已对 include/string.h 进行补全/对齐，确保可通过现代 GCC 编译。

### **7.4 asm operand has impossible constraints**

原因：现代 GCC 对旧式 inline asm 约束更严格，尤其是 0.11 里对 GDT/TSS/LDT descriptor 的写法。

解决：本仓库将关键的 descriptor 写入逻辑改为等价 C 实现，避免 GCC 约束失败。

### **7.5 tools/build.sh: unary operator expected**

原因：老脚本在现代 /bin/sh 下对空变量判断不严谨，会打印提示。

影响：通常不影响最终 Image 生成。

验证方式：检查 Image 是否生成、make start 是否能启动。

## **8. 本仓库对 macOS 的主要改动概览**

你可以在 README 或 CHANGELOG 里进一步展开，这里只列要点：

- **构建工具链兼容**
  - 处理 Homebrew 常见的 i686-elf-* 与工程 i386-elf-* 前缀差异（脚本/变量映射）
- **现代 GCC 兼容**
  - 统一使用 -std=gnu89 以保持旧式函数声明语义
  - 必要时抑制一些现代警告导致的误伤
- **头文件原型补全与对齐**
  - 补齐/对齐 include/string.h 与 lib/string.c 的函数签名，避免 implicit declaration 与类型冲突
- **inline asm 兼容**
  - 处理 GCC 15 对旧式 inline asm 的约束问题（等价 C 写 descriptor）

## **9. 进阶玩法（可选）**

- 研究 execve() 如何从用户态切换并加载新程序：fs/exec.c
- 理解 “ELF loader/ a.out loader” 的古早实现：fs/exec.c + include/a.out.h
- 从 bootsect/setup/head 追踪：实模式 -> 保护模式 -> 内核入口
- 在 gdb 中对 system_call、schedule()、do_execve() 下断点，观察进程切换与 exec 替换地址空间

## **10. 快速命令速查**

```
# 依赖
brew install qemu i686-elf-binutils i686-elf-gcc gdb

# 工具链前缀兼容
./tools/macos-link-toolchain.sh

# 编译
make clean
make -B

# 运行
make start

# 调试
make debug
# 另一个终端
gdb tools/system
(gdb) target remote :1234
(gdb) b main
(gdb) c
```
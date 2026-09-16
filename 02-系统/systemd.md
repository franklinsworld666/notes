## 一、认识 systemd：现代 Linux 的"大管家"

### 1. 什么是 systemd？

**systemd（系统守护进程）是 Linux 操作系统的系统和服务管理器**，作为内核空间之后的第一个用户空间进程（PID=1），它负责初始化系统、启动和维护用户空间服务。


### 2. 核心概念解析

#### 单元（Unit）

在 systemd 中，**一切可管理的资源都被抽象为单元**，systemd 通过单元文件（.unit）描述如何管理这些资源。常见的单元类型包括：

- **.service**：服务单元，定义后台守护进程或一次性任务
- **.socket**：套接字单元，管理网络或文件系统套接字
- **.timer**：定时器单元，用于计划执行任务，替代 crontab
- **.mount**：挂载单元，管理文件系统挂载点
- **.automount**：自动挂载单元，按需挂载文件系统
- **.target**：目标单元，用于将多个单元分组，模拟传统运行级别
- **.device**：设备单元，管理硬件设备
- **.slice**：资源切片单元，管理进程组的资源分配
- **.scope**：外部进程组单元，管理外部进程组的资源

#### 目标（Target）

**目标单元（.target）是一组单元的集合**，通常作为同步点使用，模拟传统 Linux 的运行级别。常见的目标包括：

- `default.target`：系统默认启动目标
- `graphical.target`：图形界面模式（对应传统运行级别 5）
- `multi-user.target`：多用户命令行模式（对应传统运行级别 3）
- `rescue.target`：救援模式
- `emergency.target`：紧急模式
- `power-off.target`：关机状态
- `reboot.target`：重启状态

### 3. 两大核心工具

systemd 提供了两个核心命令行工具，用于日常管理与排错：

- **systemctl**：核心管理命令，负责服务的启停、状态查看、配置修改等操作
- **journalctl**：日志管理系统，提供结构化日志记录与查询功能，支持实时监控

## 二、核心实操：systemctl 常用命令大全

### 1. 服务生命周期管理

#### 启动/停止服务

```bash
# 启动单个服务
systemctl start <service_name>

# 停止服务（默认发送SIGTERM信号）
systemctl stop <service_name>
# 强制立即终止服务（发送SIGKILL信号）
systemctl kill <service_name>

# 先停止再启动服务
systemctl restart <service_name>
# 仅当服务正在运行时才重启（不中断未启动的服务）
systemctl try-restart <service_name>
```


#### 重新加载配置（不中断服务）

```bash
# 重新加载服务配置（不中断服务）
systemctl reload <service_name>

# 修改服务文件后必须执行，使新配置生效
systemctl daemon-reload
```

### 2. 服务状态查询

#### 查看服务详细状态

```bash
# 查看服务状态（包含最近日志片段）
systemctl status <service_name>
```

**status 输出关键字段解析**：

```bash
ssh.service - OpenSSH server daemon
   Loaded: loaded (/usr/lib/systemd/system/sshd.service; enabled; vendor preset: enabled)
   Active: active (running) since Sun 2023-08-20 12:00:00 UTC; 2 days ago
     Docs: man:sshd(8)
           man:sshd_config(5)
  Main PID: 1234 (sshd)
    Tasks: 2 (limit: 4915)
   Memory: 15.2M
   CGroup: /system.slice/sshd.service
           └─1234 /usr/sbin/sshd -D
```

- **Loaded**：单元文件加载状态，显示是否启用开机自启
- **Active**：服务当前运行状态，常见值：active(running)、inactive/dead、failed
- **Main PID**：服务主进程的 PID
- **Memory**：服务占用的内存大小
- **CGroup**：服务所属的控制组
- **Result**：服务上次退出的结果（如 success、exit Code 等）

#### 列出所有服务

```bash
# 列出所有已加载的服务（包括运行和未运行）
systemctl list-units --type=service
# 列出所有服务（包括未加载的）
systemctl list-unit-files --type=service
# 列出所有已启用的服务
systemctl list-unit-files --type=service --state=enabled/avtive/failed
```

### 3. 开机自启管理

#### 启用/禁用服务

```bash
# 设置服务开机自启
systemctl enable <service_name>
# 取消服务开机自启
systemctl disable <service_name>
```

#### 彻底屏蔽服务

```bash
# 彻底屏蔽服务（禁止任何方式启动）
systemctl mask <service_name>
# 取消屏蔽服务
systemctl unmask <service_name>
```

### 4. 服务配置管理

#### 查看服务配置

```bash
# 查看服务单元文件内容
systemctl cat <service_name>
# 查看服务所有属性
systemctl show <service_name>
# 查看服务特定属性
systemctl show <service_name> -p <property_name>
```

#### 服务启动时间查询

```bash
# 查看服务最近一次启动时间
systemctl show <service_name> -p ExecMainStartTimestamp
# 查看服务最近一次进入活动状态的时间
systemctl show <service_name> -p ActiveEnterTimestamp
# 查看服务主进程
systemctl show <service_name> -p MainPID-
```

**服务启动时间解析**：

`systemctl status <service>` 输出中的 ` since` 字段显示服务启动以来的时间，如：
```
Active: active (running) since Sun 2023-08-20 12:00:00 UTC; 2 days ago
```

`systemctl show <service> -p ExecMainStartTimestamp` 显示精确到微秒的启动时间戳：
```
ExecMainStartTimestamp=Mon 2023-08-21 09:15:23 UTC
```

### 5. 服务依赖关系分析

#### 查看服务依赖关系

```bash
# 查看服务依赖的所有单元
systemctl list-dependencies <service_name>
# 查看服务的反向依赖（哪些服务依赖它）
systemctl list-dependencies --reverse <service_name>
```

### 6. 服务故障诊断

#### 查看服务失败原因

```bash
# 查看服务失败原因（包括退出代码和日志片段）
systemctl status <failed_service>
# 查看服务退出代码（判断是否正常退出）
systemctl show <failed_service> -p ExitCode
```

#### 服务退出代码解析

systemd 服务退出代码格式为 `<status>:<signal>`，其中：

- `status`：0 表示正常退出，非 0 表示异常退出
- `signal`：如果非零，表示服务被信号终止

常见退出代码：

- `137`：表示服务被 ` SIGKILL`（信号 9）强制终止
- `139`：表示服务被 ` SIGSEGV`（信号 11）终止（段错误）
- `143`：表示服务被 ` SIGHUP`（信号 1）终止（重新加载配置）

#### 服务日志快速查看

```bash
# 查看服务最近日志（包含在status中）
systemctl status <service_name>
# 查看服务全部日志
journalctl -u <service_name>
# 实时查看服务日志（类似tail -f）
journalctl -fu <service_name>
```

## 三、进阶实战：自定义 systemd 服务

### 1. service 文件结构拆解

一个标准的 systemd 服务单元文件（.service）采用 INI 格式，通常包含三个段落：

```ini
[Unit]
# 描述服务
Description=My Custom Application
# 启动顺序依赖（确保网络服务已启动）
After=network.target
# 强依赖（必须的服务）
Requires=mysql.service
# 弱依赖（推荐的服务，但不强制）
Wants=logrotate.service

[Service]
# 服务类型（决定如何管理服务进程）
Type=simple
# 服务运行用户
User=myapp
# 服务运行组
Group=myapp
# 工作目录
WorkingDirectory=/opt/myapp
# 启动命令
ExecStart=/opt/myapp/bin/myapp
# 停止命令
ExecStop=/bin/kill -s QUIT $MAINPID
# 服务异常退出时自动重启策略
Restart=on-failure
# 重启间隔时间
RestartSec=5
# 启动超时时间（超过时间视为失败）
TimeoutStartSec=30
# 停止超时时间（超过时间强制终止）
TimeoutStopSec=90
# 服务失败后重试次数限制
StartLimitBurst=5
# 服务失败后重试间隔时间
StartLimitIntervalSec=10
```

### 2. 部署标准流程

```bash
# 创建服务目录（可选）
mkdir -p /etc/systemd/system/
# 创建服务单元文件
vim /etc/systemd/system/myapp.service
# 重新加载systemd配置
systemctl daemon-reload
# 启动服务并设置开机自启
systemctl enable --now myapp
# 验证服务状态
systemctl status myapp
```

### 3. 服务文件优先级

systemd 服务文件的优先级从低到高依次为：

1. 系统默认配置：`/usr/lib/systemd/system/`（软件包安装，默认配置）
2. 软件包覆盖配置：`/usr/local/lib/systemd/system/`（高优先级，但重启后存在）
3. 管理员自定义配置：`/etc/systemd/system/`（最高优先级，重启后存在）
4. 覆盖片段文件：`/etc/systemd/system/<service_name>.d/<fragment_name>.conf`（按字母顺序加载）

**修改配置后必须执行 `systemctl daemon-reload` 使新配置生效**。

## 四、服务启动时间分析与性能优化

### 1. 宏观分析：查看系统整体启动时间

```bash
# 查看系统启动总时间
systemd-analyze
# 查看各阶段启动时间细分
systemd-analyze time
# 查看内核启动时间
systemd-analyze kernel-time
# 查看用户空间启动时间
systemd-analyze userspace-time
```

**系统启动阶段解析**：

```bash
# 典型输出示例
Startup finished in 3.2s (kernel) + 48.1s (userspace) = 51.3s
graphical.target reached after 48.1s in userspace
```

- `kernel`：内核初始化阶段所用时间
- `userspace`：用户空间服务启动阶段所用时间
- `graphical.target`：图形界面目标达成时间（即系统可交互时间）


### 2. 微观分析：找出最耗时的服务

```bash
# 按启动耗时从高到低排序所有服务
systemd-analyze blame
# 只显示耗时超过10秒的服务
systemd-analyze blame | grep '\s\+[0-9]\+\.[0-9]\+s'
# 查看关键路径上的服务启动耗时（最长依赖链）
systemd-analyze critical-chain
# 查看指定服务的关键路径
systemd-analyze critical-chain <service_name>
# 生成SVG格式的启动时间图谱
systemd-analyze plot > startup.svg
# 查看某个服务的启动时间
systemd-analyze blame | grep <service_name>
```

### 3. 服务启动时间优化方法

#### 减少启动依赖

```ini
# 在单元文件中禁用默认依赖
DefaultDependencies=no
# 显式声明必要依赖
After=network.target
```

#### 调整服务类型

根据服务特性选择合适的 `Type` 值：

```ini
# 默认类型，适用于前台运行的服务
Type=simple
# 适用于后台守护进程（如Apache、MySQL）
Type=forking
# 适用于一次性任务
Type=oneshot
# 适用于需要通知systemd就绪的服务
Type=notify
# 适用于依赖D-Bus注册的服务
Type=dbus
# 适用于空闲时启动的服务
Type id=e
```

#### 设置超时参数

```ini
# 设置启动超时时间（超过时间视为失败）
TimeoutStartSec=30
# 设置停止超时时间（超过时间强制终止）
TimeoutStopSec=90
# 设置服务失败后重试次数限制
StartLimitBurst=5
# 设置服务失败后重试间隔时间
StartLimitIntervalSec=10
```

#### 资源限制优化

```ini
# 限制CPU使用（适用于高负载服务）
CPUQuota=50%
# 限制内存使用
MemoryMax=2G
# 限制文件描述符数量
LimitNOFILE=65536
# 设置启动优先级
Nice=-5
# 设置实时优先级
Priority=10
```

#### 启动顺序优化

```ini
# 显式声明启动顺序
After=network.target
Before=webserver.service
# 仅在特定条件下启动
ConditionPathExists=/etc/myapp/enabled
ConditionPathIsSymbolicLink=/etc/myapp/config
ConditionDirectoryEmpty=/var/run/myapp
```

### 4. 内核启动参数优化

在`/etc/default/grub` 中调整内核参数：

```ini
# 编辑配置文件
GRUB_CMDLINE_LINUX="quiet splash"
# 修改后生成新配置
grub-mkconfig -o /boot/grub/grub.cfg
# 重启生效
reboot
```

**常见优化参数**：

- `quiet`：减少内核启动日志输出
- `nomodeset`：禁用显卡驱动模式切换（解决某些图形卡启动问题）
- `noacpi`：禁用 ACPI 功能（解决某些电源管理问题）
- `debug`：启用内核调试模式（增加日志输出，但可能延长启动时间）
- `nogpumanager`：禁用 GPU 管理器（减少启动项）


## 五、服务故障排查指南：四步诊断法

### 1. 第一步：查看服务状态

```bash
# 查看服务详细状态
systemctl status <service_name>

```

**状态字段解析**：

- `Active: active(running)`：服务正常运行
- `Active: inactive/dead`：服务未启动
- `Active: failed`：服务启动失败
- `Exit Code: 0`：服务正常退出
- `Exit Code: <status>:<signal>`：服务异常退出，`<signal>`为终止信号

### 2. 第二步：分析服务日志

```bash
# 查看服务全部日志
journalctl -u <service_name>
# 实时查看服务日志（调试服务启动）
journalctl -fu <service_name>
# 查看服务最近一次启动的日志
journalctl -u <service_name> -b

# 按进程名筛选日志
journalctl _COMM=<process_name>
# 按进程ID筛选日志
journalctl _PID=<pid>

```


### 3. 第三步：验证服务配置

```bash
# 检查服务单元文件语法
systemctl cat <service_name>
# 检查服务单元文件依赖
systemctl list-dependencies <service_name>
# 检查服务单元文件是否启用开机自启
systemctl is-enabled <service_name>
# 检查服务单元文件是否加载
systemctl is-loaded <service_name>
```

**常见配置问题**：

- **路径问题**：`ExecStart` 路径错误或不可执行
- **权限问题**：服务运行用户权限不足
- **依赖缺失**：`After `或` Requires` 指定的服务未启动
- **环境变量缺失**：`Environment `或` EnvironmentFile` 未正确设置
- **资源限制过严**：`MemoryMax `或` CPUQuota` 设置过小导致服务无法启动

### 4. 第四步：手动模拟服务启动

```bash
# 切换到服务运行用户，手动执行启动命令
sudo -u <user> /bin/bash
cd <working_directory>
<exec_start_command>
# 检查服务退出代码
echo $?
# 查看错误信息
<exec_start_command> 2>&1
# 检查端口占用
netstat -tuln | grep <port>
ss -tuln | grep <port>
# 检查文件权限
ls -l <file_path>
# 检查SELinux策略
getenforce
ls -Zd <file_path>
# 检查cgroups限制
cat /sys/fs/cgroup/system.slice/<service_name>/memory.limit_in_bytes
```

**手动模拟注意事项**：

- 使用与服务相同的用户身份执行
- 切换到服务的工作目录
- 设置与服务相同的环境变量
- 检查错误信息输出（可能被重定向到日志或丢弃）

### 5. 典型故障案例库

#### 权限问题

**症状**：服务无法启动，日志显示"Permission denied"或"Access denied"

**诊断命令**：
```bash
# 查看服务运行用户
systemctl show <service_name> -p User
# 检查配置文件权限
ls -l /etc/systemd/system/<service_name>.service
# 检查工作目录权限
ls -ld <working_directory>
# 检查目标文件权限
ls -l <target_file>
# 检查SELinux状态
getenforce
# 检查SELinux策略
ls -Zd <file_path>
```

**解决方案**：
- 修改文件/目录权限：`chmod`、`chown`
- 临时禁用 SELinux：`setenforce 0`（生产环境慎用）
- 检查 SELinux 日志：`审计日志`
- 添加 SELinux 策略：`semanage`、`setsebool`

#### 端口冲突

**症状**：服务无法启动，日志显示"Address already in use"

**诊断命令**：
```bash
# 查看端口占用情况
netstat -tuln | grep <port>
# 查看监听该端口的进程
lsof -i :<port>
# 查看服务是否绑定正确端口
systemctl show <service_name> -p ListenStream
```

**解决方案**：
- 停止占用端口的其他服务
- 修改服务配置，使用其他端口
- 检查防火墙设置，确保端口开放

#### 依赖缺失

**症状**：服务无法启动，日志显示"Dependency failed"或"Required service not running"

**诊断命令**：
```bash
# 查看服务依赖关系
systemctl list-dependencies <service_name>
# 查看服务依赖的其他服务状态
systemctl status <dependency_service>
# 检查服务是否声明了正确的依赖
systemctl cat <service_name> | grep -E "After|Before|Requires|Wants"
```

**解决方案**：
- 启动依赖缺失的服务：`systemctl start <dependency_service>`
- 确保依赖服务已启用开机自启：`systemctl enable <dependency_service>`
- 在单元文件中添加缺失的依赖：`Requires=<dependency_service>`

#### 资源不足

**症状**：服务无法启动，日志显示"Out of memory"或"Cannot allocate memory"

**诊断命令**：
```bash
# 查看系统内存使用
free -h
# 查看服务内存限制
systemctl show <service_name> -p MemoryMax
# 查看服务CPU限制
systemctl show <service_name> -p CPUQuota
# 查看服务文件描述符限制
systemctl show <service_name> -p LimitNOFILE
```

**解决方案**：
- 调整服务内存限制：`MemoryMax=4 G`
- 调整服务 CPU 限制：`CPUQuota=75%`
- 调整服务文件描述符限制：`LimitNOFILE=100000`
- 检查系统全局资源限制：`/etc/systemd/system.conf`

#### 服务崩溃

**症状**：服务频繁崩溃重启，日志显示"Crash"或"Segmentation fault"

#### 排查故障最佳实践

- **遵循四步诊断法**：status → 日志 → 配置 → 手动模拟
- **分析关键路径**：`systemd-analyze critical-chain`，定位启动瓶颈
- **禁用非必要服务**：`systemctl disable <service_name>`，减少启动时间
- **优化依赖关系**：合理使用 `After`、`Before`、`Requires`、`Wants` 等参数，避免不必要的依赖
- **设置合理的超时时间**：`TimeoutStartSec=30`，防止服务因初始化慢而被错误终止
- 
## 六、systemd 服务配置参数详解

### 1. [Unit]区块参数

#### 启动顺序参数

```ini
# 启动顺序依赖（仅控制顺序，不控制依赖）
After=network.target
# 启动顺序依赖（在之前启动）
Before=logrotate.service
```

#### 依赖关系参数

```ini
# 强依赖（必须的服务，若失败则本服务失败）
Requires=mysql.service
# 弱依赖（推荐的服务，但不强制）
Wants=logrotate.service
# 冲突关系（若服务已运行，则停止本服务）
Conflicts=old-service.service
```

#### 启动条件参数

```ini
# 启动条件（路径存在时启动）
ConditionPathExists=/etc/myapp/enabled
# 启动条件（路径不存在时启动）
ConditionPathIsMissing=/etc/myapp/disabled
# 启动条件（路径是符号链接时启动）
ConditionPathIsSymbolicLink=/etc/myapp/config
# 启动条件（路径是目录且为空时启动）
ConditionDirectoryEmpty=/var/run/myapp
```

#### 服务状态参数

```ini
# 服务状态（激活时执行）
OnActive=systemctl restart dependent-service
# 服务状态（失活时执行）
OnDead=systemctl stop dependent-service
```

### 2. [Service]区块参数

#### 核心执行参数

```ini
# 服务类型（决定如何管理服务进程）
Type=simple
# 启动命令（必填）
ExecStart=/opt/myapp/bin/myapp
# 停止命令（默认发送SIGTERM）
ExecStop=/bin/kill -s QUIT $MAINPID
# 重载配置命令
ExecReload=/bin/kill -s HUP $MAINPID
# 服务异常退出时自动重启策略
Restart=on-failure
# 重启间隔时间
RestartSec=5
# 重启失败后重试次数限制
StartLimitBurst=5
# 重启失败后重试间隔时间
StartLimitIntervalSec=10
```

#### 运行环境参数

```ini
# 服务运行用户
User=myapp
# 服务运行组
Group=myapp
# 服务工作目录
WorkingDirectory=/opt/myapp
# 服务环境变量
Environment=PATH=/usr/local/bin:/usr/bin:/bin
# 从文件加载环境变量
EnvironmentFile=-/etc/default/myapp
# 服务启动优先级
Nice=-5
# 服务实时优先级
Priority=10
# 服务CPU亲和性
CPUAffinity=1-2
```

#### 资源限制参数

```ini
# CPU时间权重（0-10000）
CPUWeight=100
# CPU启动权重（0-10000）
StartupCPUWeight=100
# CPU使用限制（百分比）
CPUQuota=50%
# CPU使用限制（毫秒/周期）
CPUAccounting= yes
CPUQuotaPerSec= 50ms
# 内存使用限制（最大内存）
MemoryMax=2G
# 内存使用限制（交换内存）
MemorySwapMax=1G
# 内存使用限制（软限制）
MemoryLimit=1.8G
# 内存使用限制（高水位）
MemoryHigh=1.5G
# 内存使用限制（低水位）
MemoryLow=1G
# 内存使用限制（完全限制）
MemoryDenyWriteExecute= yes
# 文件描述符限制
LimitNOFILE=65536
# 进程数限制
LimitNPROC=1024
# 内存锁定限制
LimitMEMLOCK=16M
```

数据来源：

#### 启动超时参数

```ini
# 启动超时时间（超过时间视为失败）
TimeoutStartSec=30
# 停止超时时间（超过时间强制终止）
TimeoutStopSec=90
# 进程启动超时时间
TimeoutStartPreSec=10
# 进程停止超时时间
TimeoutStopPreSec=10
```

#### 日志管理参数

```ini
# 标准输出重定向
StandardOutput=journal
# 标准错误重定向
StandardError=journal
# 日志标识（journalctl过滤使用）
SyslogIdentifier=myapp
```

#### 安全隔离参数

```ini
# 创建私有临时文件系统
PrivateTemporalFS= yes
# 禁止获取新权限
NoNewPrivileges= yes
# 创建私有临时目录
PrivateTmp= yes
# 创建私有网络命名空间
PrivateNetwork= yes
# 创建私有挂载命名空间
PrivateMounts= yes
# 创建私有进程命名空间
PrivatePID= yes
# 创建私有用户命名空间
PrivateUser= yes
# 创建私有UTS命名空间
PrivateUTS= yes
# 禁用写入/执行内存
MemoryDenyWriteExecute= yes
# 禁用写入/执行/复制内存
MemoryNoExec= yes
MemoryNoWrite= yes
MemoryNoCopy= yes
# 禁用CLONE菲尔德
MemoryNoClones= yes
```

#### 服务类型参数

```ini
# 前台运行（默认）
Type=simple
# 后台守护进程（需配合PIDFile）
Type=forking
# 一次性任务（需配合RemainAfterExit）
Type=oneshot
# 通过sd-notify通知就绪
Type=notify
# 依赖D-Bus注册就绪
Type=dbus
# 空闲时启动（需配合Condition）
Type id=e
# 通过套接字激活（需配合Socket）
Type=socket
# 通过路径变化激活（需配合Path）
Type=pat
```

#### PID 文件参数（仅适用于 Type=forking）

```ini
# PID文件路径（systemd跟踪主进程）
PIDFile=/run/myapp.pid
```

#### 服务重启参数

```ini
# 重启策略
Restart=on-failure
# 可选值：
# no（默认，不重启）
# on-abnormal（非正常退出时重启）
# on-failure（失败退出时重启）
# on-success（成功退出时重启）
# on-exit（任何退出时重启）
# always（总是重启）
# 重启间隔时间
RestartSec=5
# 重启失败后等待时间
StartLimitIntervalSec=10
# 重启失败后最大重试次数
StartLimitBurst=5
```

#### 服务恢复参数

```ini
# 服务失败后恢复操作
OnFailure=systemctl restart dependent-service
# 服务成功退出后执行操作
OnSuccess=systemctl stop dependent-service
```

### 3. [Install]区块参数

#### 开机自启参数

```ini
# 服务开机自启绑定的目标
WantedBy=multi-user.target
# 服务开机自启绑定的目标（强依赖）
RequiredBy=webserver.service
# 服务别名
Alias=myappDaemon.service
# 服务掩码（禁止启动）
Mask= yes
# 服务掩码（禁止开机自启）
MaskBeforeBoot= yes
```

#### 服务安装参数

```ini
# 服务安装方式
Also= yes
# 服务安装方式（覆盖安装）
Also= no
# 服务安装方式（仅启用）
Also= enable
```


## 七、附录：常用命令速查表

### 1. 服务管理常用命令

| 功能       | 命令                          | 参数                                       | 说明                |
| -------- | --------------------------- | ---------------------------------------- | ----------------- |
| 启动服务     | systemctl start             | <service_name>                           | 启动指定服务            |
| 停止服务     | systemctl stop              | <service_name>                           | 停止指定服务            |
| 重启服务     | systemctl restart           | <service_name>                           | 先停止再启动服务          |
| 重载配置     | systemctl reload            | <service_name>                           | 重新加载服务配置（不中断服务）   |
| 启用开机自启   | systemctl enable            | <service_name>                           | 设置服务开机自启          |
| 禁用开机自启   | systemctl disable           | <service_name>                           | 取消服务开机自启          |
| 查看服务状态   | systemctl status            | <service_name>                           | 查看服务详细状态，包括最近日志片段 |
| 重新加载配置   | systemctl daemon-reload     |                                          | 重新加载所有服务单元文件      |
| 查看服务依赖关系 | systemctl list-dependencies | <service_name>                           | 查看服务依赖的所有单元       |
| 查看文件     | Systemctl cat               | <service_name>                           |                   |
| 查看服务启动时间 | systemctl show              | <service_name> -p ExecMainStartTimestamp | 查看服务最近一次启动时间戳     |


### 2. 日志管理常用命令

| 功能 | 命令 | 参数 | 说明 |
|------|------|------|------|
| 查看服务日志 | journalctl -u | <service_name> | 查看指定服务的日志 |
| 实时监控日志 | journalctl -fu | <service_name> | 实时跟踪服务日志（类似 tail -f） |
| 按时间筛选日志 | journalctl --since | "1 hour ago" | 查看指定时间段内的日志 |
| 按错误级别筛选 | journalctl -p | err | 仅显示 Error 及以上级别的日志 |
| 按进程名筛选 | journalctl _COMM | <process_name> | 仅显示指定进程的日志 |
| 按进程 ID 筛选 | journalctl _PID | <pid> | 仅显示指定进程的日志 |
| 查看系统日志 | journalctl | | 查看系统全部日志 |
| 查看系统日志时间轴 | journalctl -b | | 查看当前启动周期的日志 |



### 3. 服务文件参数速查表

| 区块 | 参数 | 类型 | 默认值 | 说明 |
|------|------|------|--------|------|
| [Unit] | Description | 字符串 | 无 | 服务描述 |
| [Unit] | After | 字符串 | 无 | 启动顺序依赖（在之后启动） |
| [Unit] | Before | 字符串 | 无 | 启动顺序依赖（在之前启动） |
| [Unit] | Requires | 字符串 | 无 | 强依赖（必须的服务） |
| [Unit] | wants | 字符串 | 无 | 弱依赖（推荐的服务） |
| [Service] | Type | 字符串 | simple | 服务类型（simple/forking/oneshot 等） |
| [Service] | ExecStart | 字符串 | 无 | 服务启动命令（必填） |
| [Service] | ExecStop | 字符串 | /bin/kill -s QUIT $MAINPID | 服务停止命令 |
| [Service] | ExecReload | 字符串 | 无 | 服务重载配置命令 |
| [Service] | Restart | 字符串 | no | 重启策略（on-failure/always 等） |
| [Service] | RestartSec | 时间 | 100 ms | 重启间隔时间 |
| [Service] | User | 字符串 | root | 服务运行用户 |
| [Service] | Group | 字符串 | root | 服务运行组 |
| [Service] | WorkingDirectory | 字符串 | / | 服务工作目录 |
| [Service] | Environment | 字符串 | 无 | 服务环境变量 |
| [Service] | EnvironmentFile | 字符串 | 无 | 从文件加载环境变量 |
| [Service] | PIDFile | 字符串 | 无 | 服务 PID 文件路径（Type=forking 时需设置） |
| [Service] | TimeoutStartSec | 时间 | 90 s | 服务启动超时时间 |
| [Service] | TimeoutStopSec | 时间 | 90 s | 服务停止超时时间 |
| [Service] | StartLimitBurst | 整数 | 5 | 服务失败后最大重试次数 |
| [Service] | StartLimitIntervalSec | 时间 | 10 s | 服务失败后重试间隔时间 |
| [Service] | CPUQuota | 百分比 | 无 | CPU 使用限制（百分比） |
| [Service] | MemoryMax | 内存大小 | 无 | 内存使用限制 |
| [Service] | LimitNOFILE | 整数 | 无 | 文件描述符限制 |
| [Install] | WantedBy | 字符串 | 无 | 服务开机自启绑定的目标 |

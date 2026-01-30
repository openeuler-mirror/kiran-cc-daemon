# 单元测试执行说明

本文档说明如何构建和执行 kiran-cc-daemon 项目的单元测试。

## 测试模块

项目包含以下测试模块：

- **test-accounts**: 账户管理模块测试
- **test-appearance**: 外观设置模块测试
- **test-audio**: 音频设置模块测试
- **test-groups**: 用户组管理模块测试
- **test-systeminfo**: 系统信息模块测试
- **test-timedate**: 时间日期模块测试

## 前置条件

### 依赖要求

确保已安装以下依赖包：

```bash
# 基础构建工具
cmake
gcc-c++
make

# Qt5 测试框架
qt5-qtbase-devel
qt5-qttest-devel

# 项目依赖（参考主项目 README.md）
libxml++-devel
glibmm24-devel
glib2-devel
systemd-devel
libselinux-devel
...
```

## 构建测试

### 1. 配置构建目录

```bash
cd /home/tangjie02/git/kiran/kiran-cc-daemon
mkdir -p build
cd build
cmake -DCMAKE_INSTALL_PREFIX=/usr ..
```

### 2. 编译测试

```bash
# 编译所有测试
make

# 或者只编译测试目标
make kcd-test-accounts
make kcd-test-appearance
make kcd-test-audio
make kcd-test-groups
make kcd-test-systeminfo
make kcd-test-timedate
```

## 执行测试

### 方法一：使用 CTest（推荐）

CTest 是 CMake 的测试工具，可以统一管理和执行所有测试：

```bash
# 在 build 目录下执行所有测试
cd build
ctest

# 执行所有测试并显示详细输出
ctest --output-on-failure

# 执行所有测试并显示更详细的信息
ctest -V

# 执行特定标签的测试
ctest -L system-daemon    # 执行系统后端相关测试
ctest -L session-daemon   # 执行会话后端相关测试

# 执行特定测试
ctest -R test-accounts
ctest -R test-appearance
ctest -R test-audio
ctest -R test-groups
ctest -R test-systeminfo
ctest -R test-timedate

# 并行执行测试（使用 N 个线程）
ctest -j N

# 显示测试列表（不执行）
ctest -N
```

### 方法二：直接运行测试可执行文件

```bash
cd build

# 运行单个测试
./test/accounts/kcd-test-accounts
./test/appearance/kcd-test-appearance
./test/audio/kcd-test-audio
./test/groups/kcd-test-groups
./test/systeminfo/kcd-test-systeminfo
./test/timedate/kcd-test-timedate
```

### 方法三：使用 Qt Test 命令行参数

Qt Test 框架支持多种命令行参数来控制测试执行：

```bash
# 执行所有测试函数
./test/accounts/kcd-test-accounts

# 执行特定测试函数
./test/accounts/kcd-test-accounts testFunctionName

# 执行测试并输出到文件
./test/accounts/kcd-test-accounts -o test-results.xml,xml

# 执行测试并输出到控制台（文本格式）
./test/accounts/kcd-test-accounts -o -,txt

# 执行测试并显示详细输出
./test/accounts/kcd-test-accounts -v2

# 执行测试并显示信号输出
./test/accounts/kcd-test-accounts -vs

# 执行测试并设置事件延迟（毫秒）
./test/accounts/kcd-test-accounts -eventdelay 500
```

## 测试输出格式

### XML 格式

```bash
# 生成 XML 格式的测试报告
./test/accounts/kcd-test-accounts -o test-results.xml,xml
```

### 文本格式

```bash
# 生成文本格式的测试报告
./test/accounts/kcd-test-accounts -o test-results.txt,txt
```

### Light XML 格式（用于持续集成）

```bash
# 生成 Light XML 格式（适合 CI/CD）
./test/accounts/kcd-test-accounts -o test-results.lightxml,lightxml
```

## 常见问题

### 1. 测试需要 root 权限

某些系统相关的测试可能需要 root 权限才能执行：

```bash
sudo ./test/systeminfo/kcd-test-systeminfo
```

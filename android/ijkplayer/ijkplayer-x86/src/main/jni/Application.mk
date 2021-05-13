
## Application.mk 指定 ndk-build 的项目级设置


APP_OPTIM := release
# 构建此应用所面向的 Android API 级别，并对应于应用的 minSdkVersion
APP_PLATFORM := android-9
# 为指定API生成代码
APP_ABI := x86
NDK_TOOLCHAIN_VERSION=4.9
APP_PIE := false

# 用于此应用的 C++ 标准库。
# 默认情况下使用 system STL。其他选项包括 c++_shared、c++_static 和 none
APP_STL := stlport_static

# 关闭编译优化
APP_CFLAGS := -O0 -Wall -pipe \
    -ffast-math \
    -fstrict-aliasing -Werror=strict-aliasing \
    -Wno-psabi -Wa,--noexecstack \
    -DANDROID -DNDEBUG

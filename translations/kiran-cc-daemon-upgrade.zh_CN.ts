<?xml version="1.0" encoding="utf-8"?>
<!DOCTYPE TS>
<TS version="2.1" language="zh_CN">
<context>
    <name>Kiran::DepSolver</name>
    <message>
        <location filename="../plugins/upgrade/dep-solver.cpp" line="100"/>
        <source>No matching packages found for the given package IDs</source>
        <translation>未找到与所提供的软件包 ID 匹配的软件包</translation>
    </message>
</context>
<context>
    <name>Kiran::DnfWrapper</name>
    <message>
        <location filename="../plugins/upgrade/dnf/dnf-wrapper.cpp" line="375"/>
        <location filename="../plugins/upgrade/dnf/dnf-wrapper.cpp" line="438"/>
        <location filename="../plugins/upgrade/dnf/dnf-wrapper.cpp" line="570"/>
        <location filename="../plugins/upgrade/dnf/dnf-wrapper.cpp" line="675"/>
        <source>Dnf context is not initialized, please initialize it first.</source>
        <translation>Dnf 上下文没有初始化，请先初始化。</translation>
    </message>
    <message>
        <location filename="../plugins/upgrade/dnf/dnf-wrapper.cpp" line="383"/>
        <location filename="../plugins/upgrade/dnf/dnf-wrapper.cpp" line="446"/>
        <location filename="../plugins/upgrade/dnf/dnf-wrapper.cpp" line="578"/>
        <location filename="../plugins/upgrade/dnf/dnf-wrapper.cpp" line="683"/>
        <source>Failed to get sack.</source>
        <translation>获取dnf数据库失败！</translation>
    </message>
    <message>
        <location filename="../plugins/upgrade/dnf/dnf-wrapper.cpp" line="432"/>
        <source>No packages to install.</source>
        <translation>没有需要安装的软件包。</translation>
    </message>
    <message>
        <location filename="../plugins/upgrade/dnf/dnf-wrapper.cpp" line="504"/>
        <source>Failed to init transaction state! error message: %1</source>
        <translation>初始化事务状态失败！%1</translation>
    </message>
    <message>
        <location filename="../plugins/upgrade/dnf/dnf-wrapper.cpp" line="522"/>
        <location filename="../plugins/upgrade/dnf/dnf-wrapper.cpp" line="600"/>
        <source>Failed to solve dep, error message: %1</source>
        <translation>解析依赖失败！%1</translation>
    </message>
    <message>
        <location filename="../plugins/upgrade/dnf/dnf-wrapper.cpp" line="533"/>
        <source>Failed to download, error message: %1</source>
        <translation>下载软件包失败！%1</translation>
    </message>
    <message>
        <location filename="../plugins/upgrade/dnf/dnf-wrapper.cpp" line="551"/>
        <source>Failed to commit, error message: %1</source>
        <translation>提交安装事务失败！%1</translation>
    </message>
    <message>
        <location filename="../plugins/upgrade/dnf/dnf-wrapper.cpp" line="564"/>
        <source>No packages to solve dependencies.</source>
        <translation>没有软件包需要解析依赖。</translation>
    </message>
    <message>
        <location filename="../plugins/upgrade/dnf/dnf-wrapper.cpp" line="626"/>
        <source>Failed to get installs packages: %1</source>
        <translation>获取已安装的软件包失败！%1</translation>
    </message>
    <message>
        <location filename="../plugins/upgrade/dnf/dnf-wrapper.cpp" line="669"/>
        <source>Package name is empty.</source>
        <translation>软件包名为空。</translation>
    </message>
    <message>
        <location filename="../plugins/upgrade/dnf/dnf-wrapper.cpp" line="701"/>
        <source>Package %1 is not installed.</source>
        <translation>未安装软件包%1。</translation>
    </message>
    <message>
        <location filename="../plugins/upgrade/dnf/dnf-wrapper.cpp" line="715"/>
        <source>Unknown</source>
        <translation>未知</translation>
    </message>
    <message>
        <location filename="../plugins/upgrade/dnf/dnf-wrapper.cpp" line="717"/>
        <source>Downloading packages</source>
        <translation>正在下载软件包</translation>
    </message>
    <message>
        <location filename="../plugins/upgrade/dnf/dnf-wrapper.cpp" line="719"/>
        <source>Downloading metadata</source>
        <translation>正在下载源数据</translation>
    </message>
    <message>
        <location filename="../plugins/upgrade/dnf/dnf-wrapper.cpp" line="721"/>
        <source>Loading cache</source>
        <translation>正在加载缓存</translation>
    </message>
    <message>
        <location filename="../plugins/upgrade/dnf/dnf-wrapper.cpp" line="723"/>
        <source>Testing transaction</source>
        <translation>正在模拟事务</translation>
    </message>
    <message>
        <location filename="../plugins/upgrade/dnf/dnf-wrapper.cpp" line="725"/>
        <source>Requesting data</source>
        <translation>正在请求数据</translation>
    </message>
    <message>
        <location filename="../plugins/upgrade/dnf/dnf-wrapper.cpp" line="727"/>
        <source>Removing packages</source>
        <translation>正在移除软件包</translation>
    </message>
    <message>
        <location filename="../plugins/upgrade/dnf/dnf-wrapper.cpp" line="729"/>
        <source>Installing packages</source>
        <translation>正在安装软件包</translation>
    </message>
    <message>
        <location filename="../plugins/upgrade/dnf/dnf-wrapper.cpp" line="731"/>
        <source>Updating packages</source>
        <translation>正在升级软件包</translation>
    </message>
    <message>
        <location filename="../plugins/upgrade/dnf/dnf-wrapper.cpp" line="733"/>
        <source>Cleaning packages</source>
        <translation>正在清理软件包</translation>
    </message>
    <message>
        <location filename="../plugins/upgrade/dnf/dnf-wrapper.cpp" line="735"/>
        <source>Obsoleting packages</source>
        <translation>正在废弃软件包</translation>
    </message>
    <message>
        <location filename="../plugins/upgrade/dnf/dnf-wrapper.cpp" line="737"/>
        <source>Reinstalling packages</source>
        <translation>正在重新安装软件包</translation>
    </message>
    <message>
        <location filename="../plugins/upgrade/dnf/dnf-wrapper.cpp" line="739"/>
        <source>Downgrading packages</source>
        <translation>正在降级软件包</translation>
    </message>
    <message>
        <location filename="../plugins/upgrade/dnf/dnf-wrapper.cpp" line="741"/>
        <source>Querying for results</source>
        <translation>正在查询结果</translation>
    </message>
    <message>
        <location filename="../plugins/upgrade/dnf/dnf-wrapper.cpp" line="743"/>
        <source>Unknown action</source>
        <translation>未知阶段</translation>
    </message>
</context>
<context>
    <name>Kiran::Installer</name>
    <message>
        <location filename="../plugins/upgrade/installer.cpp" line="97"/>
        <source>No matching packages found for the given package IDs</source>
        <translation>未找到与所提供的软件包 ID 匹配的软件包</translation>
    </message>
</context>
</TS>

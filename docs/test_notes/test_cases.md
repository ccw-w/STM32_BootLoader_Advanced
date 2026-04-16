# Bootloader 测试用例

## 1. 测试环境

- MCU：STM32F103
- IDE：Keil MDK5
- 配置工具：STM32CubeMX
- 下载工具：ST-Link
- 通信方式：UART
- Bootloader 起始地址：`0x08000000`
- Slot A 起始地址：`0x08004000`
- Slot B 起始地址：`0x08010000`
- Meta 起始地址：`0x0801C000`

---

## 2. Flash 分区

0x08000000 ~ 0x08003FFF   Bootloader   16 KB
0x08004000 ~ 0x0800FFFF   Slot A       48 KB
0x08010000 ~ 0x0801BFFF   Slot B       48 KB
0x0801C000 ~ 0x0801CFFF   Meta         4 KB
0x0801D000 ~ 0x0801FFFF   Reserved    12 KB

---

## 3. 测试项列表

测试项 1：Bootloader 正常启动

测试目的：
验证 Bootloader 上电后能够正常启动、打印启动信息，并正确读取 Meta 信息。

测试步骤：
1. 将 Bootloader 烧录到 MCU
2. 复位开发板
3. 打开串口工具
4. 观察启动日志

预期结果：
Bootloader 打印启动信息
Meta 信息显示正常
Active app 地址显示正常

---

测试项 2：升级到非活动槽

测试目的：
验证 Bootloader 能够把新镜像写入非当前运行槽，而不是覆盖当前运行槽。

测试步骤：
1. 确认当前运行槽
2. 发送固件头
3. 发送固件数据
4. 观察目标槽打印信息

预期结果：
Bootloader 选择非活动槽作为目标槽
串口打印目标槽地址
固件正确写入目标槽

---

测试项 3：整包 CRC 校验成功

测试目的：
验证 Bootloader 能够对目标槽整包计算 CRC32，并接受正确固件。

测试步骤：
1. 准备一个正确的固件镜像
2. 发送正确的固件头
3. 发送完整固件数据
4. 观察 CRC 日志

预期结果：
Bootloader 打印计算 CRC 和期望 CRC
CRC 校验通过
Meta 被正确更新
升级进入成功状态

---

测试项 4：整包 CRC 校验失败

测试目的：
验证 Bootloader 在 CRC 不匹配时能拒绝错误固件。

测试步骤：
1. 准备一个固件镜像
2. 修改头中的 CRC，或者故意破坏部分数据
3. 发送固件头
4. 发送固件数据
5. 观察日志

预期结果：
Bootloader 检测到 CRC mismatch
Bootloader 进入错误状态
新镜像不会被当成有效镜像

---

测试项 5：首启确认成功

测试目的：
验证新镜像首启后能够被 App 正常确认，并成为稳定运行镜像。

测试步骤：
1. 将正确的新镜像升级到非活动槽
2. 复位开发板
3. Bootloader 进入 TESTING 流程
4. 新 App 成功启动
5. App 执行确认逻辑
6. 再次复位开发板

预期结果：
Bootloader 试启动新镜像
App 成功执行确认
Meta 被更新为确认状态
下次复位后，Bootloader 直接启动新槽

---

测试项 6：未确认自动回滚

测试目的：
验证新镜像如果没有确认成功，Bootloader 能自动回滚到旧槽。

测试步骤：
1. 在 App 中临时注释掉 APP_ConfirmBootIfNeeded()
2. 将该测试镜像升级到非活动槽
3. 复位开发板
4. Bootloader 试启动新镜像
5. 再次复位开发板
6. 观察回滚日志

预期结果：
第一次复位：Bootloader 进入 TESTING 流程
新镜像启动，但没有确认
第二次复位：Bootloader 检测到未确认状态
Bootloader 回滚到旧槽
旧槽重新启动

---

测试项 7：A / B 槽切换

测试目的：
验证升级成功后，活动槽能够在 A 和 B 之间正确切换。

测试步骤：
1. 从 Slot A 启动
2. 升级 Slot B
3. 确认启动成功
4. 再次复位
5. 再升级 Slot A
6. 再次确认启动成功
7. 再次复位

预期结果：
Active slot 能根据升级结果正确切换
A / B 双向切换逻辑正常
Meta 信息保持一致

---

5. 当前已验证结果

[x] 正常升级到非活动槽

[x] 整包 CRC 校验成功

[x] 整包 CRC 校验失败检测

[x] 首启确认成功

[x] 未确认自动回滚

[x] A / B 槽切换正常
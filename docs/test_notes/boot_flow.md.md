# Bootloader 启动与升级流程说明

## 1. 总体说明

本项目采用 A/B 双槽升级方案。

Flash 分区如下：
0x08000000 ~ 0x08003FFF   Bootloader   16 KB
0x08004000 ~ 0x0800FFFF   Slot A       48 KB
0x08010000 ~ 0x0801BFFF   Slot B       48 KB
0x0801C000 ~ 0x0801CFFF   Meta         4 KB
0x0801D000 ~ 0x0801FFFF   Reserved    12 KB

其中：
Bootloader 负责升级、校验、跳转、回滚
Slot A / Slot B 用于存放应用镜像
Meta 区用于保存升级状态、活动槽、目标槽、回滚槽、确认标志等信息

---

## 2. 上电启动流程

Bootloader 上电后的主要流程如下：

1. 初始化基础运行环境
2. 打印启动信息
3. 从 Meta 区读取状态信息
4. 根据 Meta 状态决定当前启动动作
5. 进入状态机，等待升级或执行跳转

---

## 3. 启动决策逻辑

情况 1：新镜像处于 TESTING，且还没试启动过

条件：
status = TESTING
confirmed = 0
boot_pending = 1

处理逻辑：
1. 读取当前活动槽地址
2. 检查该槽中的应用是否有效
3. 如果无效：
回退到 rollback 槽
更新 Meta
留在 Bootloader

4. 如果有效：
将 boot_pending 置为 0
更新 Meta
跳转到新镜像执行首启测试

---

情况 2：新镜像处于 TESTING，但首启后仍未确认

条件：
status = TESTING
confirmed = 0
boot_pending = 0

处理逻辑：
1. 认为新镜像首启后没有确认成功
2. 将 active_slot 恢复为 rollback_slot
3. 更新 target_slot
4. 将状态写回为 OK
5. 跳转回旧槽运行

这就是“未确认自动回滚”的核心逻辑。

---

情况 3：升级中断或升级失败

条件：
status = UPDATING
或 status = ERROR

处理逻辑：
1. 不直接跳 App
2. 留在 Bootloader
3. 等待新的升级包

---

情况 4：收到升级命令

如果串口检测到升级命令，则：

1. 进入升级模式
2. 状态切换到等待固件头
3. 开始接收新的升级包

---

情况 5：正常启动当前活动槽

如果当前不是测试状态，也不是错误状态，并且活动槽中的 App 有效，则：
1. 读取当前活动槽地址
2. 检查应用有效性
3. 切换向量表
4. 设置 MSP
5. 跳转到 App 运行

---

## 4. 升级流程

4.1 等待固件头

Bootloader 进入 WAIT_HEADER 状态后：
1. 等待接收固定长度的固件头
2. 打印固件头内容
3. 检查固件头是否合法

固件头主要包含：
magic
size
crc32
version

---

4.2 确定目标槽

当固件头合法后：
1. 根据当前 active_slot 计算 inactive_slot
2. 将其作为本次升级目标槽
3. 记录目标槽地址和大小
4. 擦除目标槽对应 Flash 区域
5. 更新 Meta，状态记为 UPDATING

---

4.3 接收固件数据

Bootloader 进入 RECV_DATA 状态后：
1. 按块接收固件数据
2. 每次接收一块后写入 Flash
3. 更新：
当前写地址
剩余大小
已接收大小
4. 全部接收完成后切换到 CRC 校验状态

---

4.4 整包 CRC 校验

Bootloader 进入 VERIFY_CRC 状态后：
1. 从目标槽起始地址开始
2. 对整包固件重新计算 CRC32
3. 与固件头里的期望 CRC 比较

如果 CRC 一致
4. 认为升级成功
5. 新槽设为 active_slot
6. 旧槽设为 rollback_slot
7. 下一个目标槽设为新的 inactive_slot
8. 将状态写为 TESTING
9. 设置：
boot_pending = 1
confirmed = 0

如果 CRC 不一致
1. 认为升级失败
2. 状态写为 ERROR
3. 回到 Bootloader 等待重新升级

---

5. 首启确认流程

当 Bootloader 首次跳转到新镜像后：
1. App 进入 main()
2. App 调用 APP_ConfirmBootIfNeeded()
3. 读取 Meta
4. 如果发现：
当前状态为 TESTING
当前活动槽就是自己
confirmed = 0
5. 则将 Meta 改为：
status = OK
boot_pending = 0
confirmed = 1

这样这个新镜像就从“测试镜像”变成“正式镜像”。

---

6. 回滚流程

如果新镜像已经首启过，但没有完成确认：
1. Bootloader 下次上电时检测到：
status = TESTING
boot_pending = 0
confirmed = 0

2. 判定新镜像不可靠
3. 恢复 active_slot = rollback_slot
4. 更新 target_slot
5. 更新 Meta 为 OK
6. 跳转回旧槽运行

---

7. 应用有效性判断

Bootloader 跳转前会先检查应用镜像是否合法，主要检查：
1. 初始栈顶地址是否位于 SRAM 区间
2. Reset_Handler 是否不是空值
3. Reset_Handler 是否落在当前槽地址范围内
只有通过这些检查，才允许跳转。

---

# STM32_BootLoader_Advanced

<p align="center">
  <b>STM32F103 Bootloader project with full-image CRC, A/B dual-image update, boot confirmation, and rollback.</b><br/>
  <b>基于 STM32F103 的 Bootloader 项目，支持整包 CRC、A/B 双镜像升级、首启确认与自动回滚。</b>
</p>

<p align="center">
  <a href="#english">English</a> | <a href="#中文">中文</a>
</p>

---

# English

## Overview

`STM32_BootLoader_Advanced` is an STM32F103 bootloader project developed step by step from a minimal bootloader into a more complete firmware update system.

The current version already supports:

- UART firmware update
- Full-image CRC32 verification
- Upgrade state machine
- A/B dual-image slots
- First-boot confirmation
- Automatic rollback on unconfirmed boot

This project is not only meant to run successfully, but also to be explainable as an embedded engineering project for documentation, interviews, and resume presentation.

---

## Features

### Implemented

- UART firmware header reception
- Chunked firmware data reception
- Chunked Flash programming
- Full-image CRC32 verification
- Upgrade metadata management in Flash
- A/B dual-slot firmware layout
- Boot selection based on active slot
- First-boot confirmation from application
- Automatic rollback when the new image is not confirmed

### Main Design Points

- Bootloader, Slot A, Slot B, and Meta area are separated clearly
- New firmware is always written into the inactive slot
- After CRC passes, the new image enters `TESTING` state
- Bootloader test-boots the new image once
- Application confirms successful boot by updating Meta
- If confirmation does not happen, Bootloader rolls back to the previous slot

---

## Flash Layout

Target MCU flash size: **128 KB**

```text
0x08000000 ~ 0x08003FFF   Bootloader   16 KB
0x08004000 ~ 0x0800FFFF   Slot A       48 KB
0x08010000 ~ 0x0801BFFF   Slot B       48 KB
0x0801C000 ~ 0x0801CFFF   Meta         4 KB
0x0801D000 ~ 0x0801FFFF   Reserved    12 KB
```

---

## Repository Structure

```text
project/
├─ bootloader/
│  └─ bootloader/          # Bootloader project
├─ app_A/
│  └─ app_demo/            # Application project for Slot A
└─ app_B/
   └─ app_demo/            # Application project for Slot B

docs/                      # Notes and helper documents
App_test_AandeB/           # Test binaries and header examples
```

---

## Build Configuration

### Bootloader
- Start address: `0x08000000`

### App_A
- IROM start: `0x08004000`
- `VECT_TAB_OFFSET = 0x00004000U`

### App_B
- IROM start: `0x08010000`
- `VECT_TAB_OFFSET = 0x00010000U`

> Important: App_A and App_B must be built separately.  
> A Slot-A image cannot be directly reused as a Slot-B image.

---

## Upgrade Flow

### Normal Update Flow

1. Bootloader starts and reads Meta information
2. It determines the current `active_slot`
3. New firmware is received through UART
4. The firmware is written into the `inactive_slot`
5. Full-image CRC32 is calculated and verified
6. If CRC passes, Meta is updated:
   - `active_slot = target_slot`
   - status becomes `TESTING`
   - `confirmed = 0`
   - `boot_pending = 1`
7. On next boot, Bootloader test-boots the new image
8. The application confirms successful boot in `main()`
9. Meta is updated to `OK`, and the new slot becomes the stable running slot

### Rollback Flow

1. New image is marked as `TESTING`
2. Bootloader attempts first boot of the new slot
3. If the application does **not** confirm boot success
4. On next reset, Bootloader detects:
   - `status = TESTING`
   - `confirmed = 0`
   - first boot already attempted
5. Bootloader rolls back to the previous slot

---

## Metadata

The Meta area stores upgrade state, including:

- current status
- active slot
- target slot
- rollback slot
- boot pending flag
- confirmed flag
- firmware size
- CRC32
- firmware version

This metadata is the core of the A/B update and rollback mechanism.

---

## Test Scenarios

The project has been tested around the following scenarios:

- Normal firmware update to inactive slot
- Full-image CRC pass
- Full-image CRC mismatch
- First boot of new image
- Boot confirmation by application
- Rollback on unconfirmed new image
- Slot A / Slot B switching

Recommended test sequence:

1. Boot from Slot A
2. Upgrade Slot B
3. Reboot and test-boot Slot B
4. Confirm boot inside App_B
5. Reboot again and verify Slot B becomes stable
6. Disable confirmation temporarily
7. Upgrade again and verify rollback behavior

---

## Development Environment

- MCU: STM32F103 series
- IDE: Keil MDK5
- Configuration tool: STM32CubeMX
- Programmer: ST-Link
- Language: C
- Library: STM32 HAL

---

## Project Value

This project demonstrates the following embedded development abilities:

- Bootloader design
- Flash partition planning
- Firmware update mechanism
- CRC verification
- State machine design
- A/B dual-image update strategy
- Rollback recovery mechanism
- Engineering-oriented debugging and validation

---

## Future Improvements

Possible future improvements:

- Version check and downgrade protection
- Better host-side update tool
- Timeout and retry optimization
- Better log abstraction
- More complete diagrams and test reports
- Ymodem or custom packet protocol refinement

---

## License

This repository is licensed under the MIT License.

---

# 中文

## 项目简介

`STM32_BootLoader_Advanced` 是一个基于 STM32F103 的 Bootloader 项目，从最小可跳转的 Bootloader 逐步扩展为一个更完整的固件升级系统。

当前版本已经支持：

- 串口固件升级
- 整包 CRC32 校验
- 升级状态机
- A/B 双镜像分区
- 首启确认
- 未确认自动回滚

这个项目的目标不只是“能跑”，还希望做到：

- 结构清晰
- 过程完整
- 可以写进简历
- 可以在面试中讲清楚设计思路

---

## 当前已实现功能

### 已完成

- 固件头接收
- 固件分块接收
- 固件分块写入 Flash
- 整包 CRC32 校验
- Flash 中 Meta 信息管理
- A/B 双槽升级
- 按 active slot 启动应用
- App 首启确认
- 新镜像未确认自动回滚

### 当前方案特点

- Bootloader 区、A 槽、B 槽、Meta 区分离明确
- App_A / App_B 分别独立构建
- 升级时总是写入非当前运行槽
- CRC 校验通过后，新镜像先进入 `TESTING` 状态
- App 启动成功后主动回写确认
- 若新镜像未确认，则自动回滚到旧槽

---

## Flash 分区

目标芯片 Flash 大小：**128 KB**

```text
0x08000000 ~ 0x08003FFF   Bootloader   16 KB
0x08004000 ~ 0x0800FFFF   Slot A       48 KB
0x08010000 ~ 0x0801BFFF   Slot B       48 KB
0x0801C000 ~ 0x0801CFFF   Meta         4 KB
0x0801D000 ~ 0x0801FFFF   Reserved    12 KB
```

---

## 仓库结构

```text
project/
├─ bootloader/
│  └─ bootloader/          # Bootloader 工程
├─ app_A/
│  └─ app_demo/            # A 槽版 App 工程
└─ app_B/
   └─ app_demo/            # B 槽版 App 工程

docs/                      # 说明文档
App_test_AandeB/           # 测试 bin 和头示例
```

---

## 编译配置说明

### Bootloader
- 起始地址：`0x08000000`

### App_A
- IROM 起始：`0x08004000`
- `VECT_TAB_OFFSET = 0x00004000U`

### App_B
- IROM 起始：`0x08010000`
- `VECT_TAB_OFFSET = 0x00010000U`

> 注意：App_A 和 App_B 必须分别编译。  
> A 槽版本的 bin 不能直接当作 B 槽版本使用。

---

## 升级流程

### 正常升级流程

1. Bootloader 上电，读取 Meta 信息
2. 判断当前 `active_slot`
3. 通过串口接收新固件
4. 将新固件写入 `inactive_slot`
5. 对目标槽整包计算 CRC32
6. 若 CRC 校验通过，更新 Meta：
   - `active_slot = target_slot`
   - 状态置为 `TESTING`
   - `confirmed = 0`
   - `boot_pending = 1`
7. 下次重启时，Bootloader 试启动新镜像
8. App 成功进入主程序后执行确认逻辑
9. Meta 更新为 `OK`，新槽成为稳定运行镜像

### 回滚流程

1. 新镜像被标记为 `TESTING`
2. Bootloader 允许其首启
3. 如果 App 没有成功完成确认
4. 下次上电时 Bootloader 检测到：
   - `status = TESTING`
   - `confirmed = 0`
   - 且首启已经试过
5. 自动回滚到旧槽运行

---

## Meta 信息

Meta 区用于保存升级关键状态，包括：

- 当前状态
- 当前活动槽
- 目标槽
- 回滚槽
- boot pending 标志
- confirmed 标志
- 固件大小
- CRC32
- 固件版本号

Meta 是整个 A/B 升级和回滚机制的核心。

---

## 测试场景

当前项目已经围绕这些场景做过验证：

- 正常升级到非活动槽
- 整包 CRC 校验成功
- 整包 CRC 校验失败
- 新镜像首启
- App 首启确认
- 未确认自动回滚
- A / B 双槽切换

推荐测试顺序：

1. 从 A 槽启动
2. 升级 B 槽
3. 重启并试启动 B 槽
4. 在 App_B 中执行确认
5. 再次重启，确认 B 槽成为稳定运行槽
6. 临时关闭确认逻辑
7. 再次升级并验证未确认回滚

---

## 开发环境

- MCU：STM32F103 系列
- IDE：Keil MDK5
- 配置工具：STM32CubeMX
- 下载工具：ST-Link
- 开发语言：C
- 库：STM32 HAL

---

## 项目价值

这个项目可以体现下面这些嵌入式能力：

- Bootloader 设计
- Flash 分区规划
- 固件升级机制设计
- CRC 校验
- 状态机设计
- A/B 双镜像升级策略
- 回滚恢复机制
- 工程化调试与验证能力

---

## 后续可扩展方向

后续还能继续扩展的方向包括：

- 版本号检查与禁止降级
- 更完整的上位机升级工具
- 超时与重试机制优化
- 更统一的日志抽象
- 完整流程图与测试报告
- Ymodem 或自定义分包协议优化

---

## 开源协议

本仓库使用 MIT License。

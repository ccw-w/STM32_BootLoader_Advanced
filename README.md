# STM32_BootLoader_Advanced

<p align="center">
  <b>STM32F103 Bootloader with full-image CRC, A/B dual-image update, boot confirmation, and rollback.</b><br/>
  <b>基于 STM32F103 的 Bootloader，支持整包 CRC、A/B 双镜像升级、首启确认与自动回滚。</b>
</p>

<p align="center">
  <a href="#english">English</a> | <a href="#中文">中文</a>
</p>

---

# English

## Overview

`STM32_BootLoader_Advanced` is an STM32F103 bootloader project developed step by step from a minimal bootloader into a more complete firmware update system.

Current functions:

- UART firmware update
- Full-image CRC32 verification
- Upgrade state machine
- A/B dual-image slots
- First-boot confirmation
- Automatic rollback on unconfirmed boot

---

## Current Status

Implemented:

- Firmware header reception
- Chunked firmware reception
- Chunked Flash write
- Full-image CRC32 verification
- Metadata storage in Flash
- A/B slot switching
- Boot selection by active slot
- Application-side boot confirmation
- Rollback on unconfirmed boot

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
test_notes/                # Test binaries and header examples
```

---

## Document Navigation

- `docs/test_notes/test_cases.md`: test cases and validation items
- `docs/test_notes/boot_flow.md`: boot and upgrade flow description

## Main Projects

- `project/bootloader/bootloader/`: Bootloader project
- `project/app_A/app_demo/`: application project for Slot A
- `project/app_B/app_demo/`: application project for Slot B

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

> App_A and App_B must be built separately.

---

## Upgrade Flow

### Normal Update Flow

1. Bootloader starts and reads Meta
2. Determine current `active_slot`
3. Receive firmware through UART
4. Write the new image into `inactive_slot`
5. Calculate and verify full-image CRC32
6. Update Meta and mark the new slot as `TESTING`
7. On next boot, Bootloader test-boots the new image
8. Application confirms successful boot
9. Meta becomes `OK`

### Rollback Flow

1. New image is marked as `TESTING`
2. Bootloader allows first boot of the new slot
3. If the application does not confirm boot success
4. On next reset, Bootloader rolls back to the previous slot

---

## Metadata

The Meta area stores:

- status
- active slot
- target slot
- rollback slot
- boot pending flag
- confirmed flag
- firmware size
- CRC32
- firmware version

---

## Test Items

Tested items:

- Normal update to inactive slot
- Full-image CRC pass
- Full-image CRC mismatch
- First boot of new image
- Boot confirmation by application
- Rollback on unconfirmed boot
- Slot A / Slot B switching

---

## Development Environment

- MCU: STM32F103 series
- IDE: Keil MDK5
- Configuration tool: STM32CubeMX
- Programmer: ST-Link
- Language: C
- Library: STM32 HAL

---

## License

This repository is licensed under the MIT License.

---

# 中文

## 项目简介

`STM32_BootLoader_Advanced` 是一个基于 STM32F103 的 Bootloader 项目，从最小可跳转的 Bootloader 逐步扩展为一个更完整的固件升级系统。

当前功能包括：

- 串口固件升级
- 整包 CRC32 校验
- 升级状态机
- A/B 双镜像分区
- 首启确认
- 未确认自动回滚

---

## 当前状态

已完成：

- 固件头接收
- 固件分块接收
- 固件分块写入 Flash
- 整包 CRC32 校验
- Flash 中 Meta 信息管理
- A/B 双槽切换
- 按 active slot 启动应用
- App 首启确认
- 新镜像未确认自动回滚

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
test_notes/                # 测试 bin 和头示例
```

---

## 文档导航

- `docs/test_notes/test_cases.md`：测试用例与测试项说明
- `docs/test_notes/boot_flow.md`：Bootloader 启动与升级流程说明

## 主要工程

- `project/bootloader/bootloader/`：Bootloader 工程
- `project/app_A/app_demo/`：A 槽版应用工程
- `project/app_B/app_demo/`：B 槽版应用工程

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

> App_A 和 App_B 需要分别编译。

---

## 升级流程

### 正常升级流程

1. Bootloader 上电后读取 Meta
2. 判断当前 `active_slot`
3. 通过 UART 接收新固件
4. 将新镜像写入 `inactive_slot`
5. 对目标槽整包计算 CRC32
6. 更新 Meta，并将新槽标记为 `TESTING`
7. 下次重启时，Bootloader 试启动新镜像
8. App 成功启动后执行确认
9. Meta 更新为 `OK`

### 回滚流程

1. 新镜像被标记为 `TESTING`
2. Bootloader 允许其首启
3. 如果 App 没有确认启动成功
4. 下次复位时，Bootloader 回滚到旧槽

---

## Meta 信息

Meta 区保存以下内容：

- 状态
- 当前活动槽
- 目标槽
- 回滚槽
- boot pending 标志
- confirmed 标志
- 固件大小
- CRC32
- 固件版本号

---

## 测试项

已测试项目：

- 正常升级到非活动槽
- 整包 CRC 校验成功
- 整包 CRC 校验失败
- 新镜像首启
- App 首启确认
- 未确认自动回滚
- A / B 双槽切换

---

## 开发环境

- MCU：STM32F103 系列
- IDE：Keil MDK5
- 配置工具：STM32CubeMX
- 下载工具：ST-Link
- 开发语言：C
- 库：STM32 HAL

---

## 开源协议

本仓库使用 MIT License。

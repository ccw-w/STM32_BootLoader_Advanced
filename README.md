# STM32_BootLoader_Advanced

A structured STM32 BootLoader project based on STM32F103, developed step by step from a minimal bootloader to a robust firmware update solution.

## Project Goals

This project will be implemented in stages:

1. Minimal BootLoader
2. Full-image CRC verification
3. Upgrade state machine
4. A/B dual-image slots
5. Rollback mechanism

## Directory Structure

- `docs/` design notes, flowcharts, test cases
- `tools/` host-side scripts and helper tools
- `project/bootloader/` bootloader firmware
- `project/app_demo/` application firmware demo
- `output/` generated binary files

## Hardware Platform

- MCU: STM32F103 series
- Flash size: 128 KB
- Download tool: ST-Link
- IDE: Keil5
- Config tool: STM32CubeMX

## Progress

- [x] Repository initialized
- [ ] BootLoader project created
- [ ] App demo project created
- [ ] Jump to application verified
- [ ] Full-image CRC verified
- [ ] Upgrade state machine finished
- [ ] A/B and rollback finished
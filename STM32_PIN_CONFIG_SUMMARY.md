# STM32 VGA 引脚配置总结

> 状态说明（2026-03-24）：本文件基于 `reference_repo/stm32_vga` 的旧实现。当前 `try` 工程的 Hackaday 路线迁移和最新引脚约定请看 `HACKADAY_ROUTE_UPDATE_LOG.md`。

## 1. 适用范围

本总结基于你新加的参考仓库：

- `reference_repo/stm32_vga`
- 目标板卡：`nucleo_f091rc`（见 `platformio.ini`）

## 2. VGA 必需引脚

| MCU 引脚 | 功能 | 信号角色 | 软件驱动方式 | 证据 |
|---|---|---|---|---|
| `PB0` | `HSYNC` | 行同步 | 在 `TIM1` 更新中断里拉低/拉高 | `README.md:23`, `src/vga_driver.c:46-50` |
| `PB1` | `VSYNC` | 场同步 | 在 `TIM3` 更新中断里拉低/拉高 | `README.md:24`, `src/vga_driver.c:61-66` |
| `PC0` | Color bit0 | RGB 数据位（README 接到 RED） | DMA 把帧缓冲字节写到 `GPIOC->ODR` | `README.md:25`, `src/vga_driver.c:128-130` |
| `PC1` | Color bit1 | RGB 数据位（README 接到 GREEN） | DMA 把帧缓冲字节写到 `GPIOC->ODR` | `README.md:26`, `src/vga_driver.c:128-130` |
| `PC2` | Color bit2 | RGB 数据位（README 接到 BLUE） | DMA 把帧缓冲字节写到 `GPIOC->ODR` | `README.md:27`, `src/vga_driver.c:128-130` |
| `GND` | 地 | VGA 回流 | 直接共地 | `README.md:28` |

硬件注意：RGB 每路串联电阻至少 `270 ohm`（`README.md:30`）。

## 3. GPIO 配置方式

### 3.1 同步脚（GPIOB）

- `PB0`、`PB1` 配置为普通推挽输出。
- 输出速度配置为高速。
- 初始化先置高电平。

证据：`src/vga_driver.c:23`, `src/vga_driver.c:26`, `src/vga_driver.c:29`。

### 3.2 像素数据脚（GPIOC）

- `PC0` 到 `PC7` 全部被配置为输出+高速（for 循环配置）。
- 即使 README 只接了 `PC0..PC2`，软件层面仍会驱动 `PC0..PC7`。

证据：`src/vga_driver.c:31-39`，以及 README 关于 8-bit DMA 颜色总线说明 `README.md:9`。

## 4. 定时器与 DMA 如何决定引脚行为

系统时钟：

- 使用 `HSI48`，主频 `48 MHz`。
- 证据：`src/mci_clock.c:7`, `src/mci_clock.c:27`。

时序链路：

- `TIM1` 触发中断，在 ISR 内软件翻转 `PB0` 生成 HSYNC。
  - 配置：`TIM1_PSC=0`, `TIM1_ARR=1279`（`include/vga_driver.h:16-17`）
  - 理论更新频率：`48MHz / ((0+1)*(1279+1)) = 37.5kHz`（约 `26.67us`）
- `TIM3` 触发中断，在 ISR 内软件翻转 `PB1` 生成 VSYNC。
  - 配置：`TIM3_PSC=46`, `TIM3_ARR=17020`（`include/vga_driver.h:20-21`）
  - 理论更新频率：`48MHz / ((46+1)*(17020+1)) ≈ 60Hz`（约 `16.67ms`）
- `TIM2` 产生 DMA 请求，把像素数据流送到 GPIOC。
  - 配置：`TIM2_PSC=1`, `TIM2_ARR=4`（`include/vga_driver.h:24-25`）
  - 理论请求频率：`48MHz / ((1+1)*(4+1)) = 4.8MHz`
- DMA 目标寄存器是 `GPIOC->ODR`，通道为 `DMA1_Channel2`（`src/vga_driver.c:125-138`）。

## 5. 仅在示例中使用的额外引脚（`pong.c`）

| MCU 引脚 | 功能 | 模式 | 证据 |
|---|---|---|---|
| `PA0` | 左挡板模拟输入（`adc_read_channel(0)`） | 模拟输入 | `examples/pong.c:62-63`, `examples/pong.c:117` |
| `PA1` | 右挡板模拟输入（`adc_read_channel(1)`） | 模拟输入 | `examples/pong.c:62-63`, `examples/pong.c:118` |

结论：基础 VGA 输出只需要 `PB0/PB1/PC0..PC2/GND`；`PA0/PA1` 是 Pong 示例的可选输入。

## 6. 工程实现风险与建议

- 该项目的 HSYNC/VSYNC 不是走定时器复用输出脚，而是 ISR 里软件翻转 GPIO（`src/vga_driver.c:42-70`）。对中断时延较敏感。
- DMA 直接写 `GPIOC->ODR` 低 8 位，因此视频输出期间建议把 `PC0..PC7` 全部视为保留引脚（`src/vga_driver.c:128-130`）。
- 仓库里存在颜色位定义不一致风险：
  - README 写的是 `PC0->RED, PC1->GREEN, PC2->BLUE`（`README.md:25-27`）。
  - `pong.c` 注释里 `0b001` 标成 blue、`0b100` 标成 red（`examples/pong.c:121-122`）。
  - 建议按“位到引脚”的真实映射校准：`bit0->PC0`, `bit1->PC1`, `bit2->PC2`，然后实测屏幕颜色再定最终连线/软件定义。

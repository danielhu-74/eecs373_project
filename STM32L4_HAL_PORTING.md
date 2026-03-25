# STM32L4 + STM32CubeIDE(HAL) 迁移指南（VGA方案）

> 状态说明（2026-03-24）：本文件描述的是早期 `TIM1/TIM3 + GPIOC` 路线。当前 Hackaday 路线迁移请以 `HACKADAY_ROUTE_UPDATE_LOG.md` 为准，并在该文件持续追加更新记录。

## 1. 目标

把 `reference_repo/stm32_vga` 的裸寄存器实现迁移到 `STM32L4 + HAL`，核心仍然是：

- `PB0` 输出 `HSYNC`
- `PB1` 输出 `VSYNC`
- `PC0..PC2` 输出 RGB（软件仍可驱动 `PC0..PC7`）
- `TIM2 + DMA` 把 framebuffer 流式写入 `GPIOC->ODR`

## 2. CubeMX 配置清单

## 2.1 GPIO

- `PB0` -> `GPIO_Output`，Very High Speed，No Pull
- `PB1` -> `GPIO_Output`，Very High Speed，No Pull
- `PC0..PC7` -> `GPIO_Output`，Very High Speed，No Pull

## 2.2 TIM

- `TIM1`：Base + Update Interrupt（用于 HSYNC 节拍）
- `TIM3`：Base + Update Interrupt（用于 VSYNC 节拍）
- `TIM2`：Base + DMA Request on Update（用于像素时钟触发 DMA）

## 2.3 DMA

- 给 `TIM2_UP` 绑定一个 DMA 通道（CubeMX 自动分配即可）
- 方向：`Memory to Peripheral`
- 外设地址：`GPIOC->ODR`
- 内存递增：Enable
- 外设递增：Disable
- 模式：Circular
- 数据宽度：Byte/Byte

## 2.4 NVIC

- 使能 `TIM1`、`TIM3` 中断
- 建议 `TIM1` 优先级高于 `TIM3`

## 3. 时钟和参数建议

为了最大程度复用参考仓库参数，建议先把定时器时钟跑在 `48MHz`（和原仓库一致）：

- `TIM1`: `PSC=0`, `ARR=1279` -> 约 `37.5kHz`（行周期约 `26.67us`）
- `TIM2`: `PSC=1`, `ARR=4` -> `4.8MHz`（像素节拍）
- `TIM3`: `PSC=46`, `ARR=17020` -> 约 `60Hz`

如果你使用 `80MHz` 主频，也能做，但需要重算以上参数。

## 4. HAL 代码骨架

```c
// vga_port.c
#include "main.h"

#define H_PIXEL  (1024/8)
#define V_PIXEL  (632/8)
#define RESOLUTION (H_PIXEL * V_PIXEL)

extern TIM_HandleTypeDef htim1;
extern TIM_HandleTypeDef htim2;
extern TIM_HandleTypeDef htim3;
extern DMA_HandleTypeDef hdma_tim2_up; // 名字以 Cube 生成为准

static uint8_t frame_buffer[RESOLUTION];
static volatile uint16_t current_line = 0;

void VGA_Start(void)
{
    // 启动 TIM2 DMA: frame_buffer -> GPIOC->ODR
    HAL_DMA_Start(&hdma_tim2_up,
                  (uint32_t)frame_buffer,
                  (uint32_t)&GPIOC->ODR,
                  H_PIXEL);
    __HAL_TIM_ENABLE_DMA(&htim2, TIM_DMA_UPDATE);

    HAL_TIM_Base_Start(&htim2);
    HAL_TIM_Base_Start_IT(&htim1);
    HAL_TIM_Base_Start_IT(&htim3);
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    if (htim->Instance == TIM1) {
        // HSYNC pulse on PB0
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_RESET);
        __NOP(); __NOP(); __NOP(); __NOP(); __NOP(); __NOP();
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_SET);

        current_line++;

        // 每 8 行更新 DMA 源指针（和参考仓库逻辑一致）
        hdma_tim2_up.Instance->CMAR =
            (uint32_t)(frame_buffer + ((current_line >> 3) * H_PIXEL));
    }

    if (htim->Instance == TIM3) {
        // VSYNC pulse on PB1
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_1, GPIO_PIN_RESET);
        // 可以先用短延时，后续再改成更精确的定时方案
        for (volatile int i = 0; i < 500; i++) { __NOP(); }
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_1, GPIO_PIN_SET);

        current_line = 0;
    }
}
```

## 5. 从工程角度给你的建议

- 第一步先“跑起来”：
  - 先保持 1bit/通道（`PC0..PC2`）
  - 先固定颜色条/测试图，不要一开始就上复杂图形逻辑
- 第二步再“抠稳定性”：
  - 把 HSYNC/VSYNC 从 ISR 翻转 GPIO，升级为 Timer PWM/OC 输出（抖动更小）
  - 优化 DMA 与行切换时机
- 第三步再“扩功能”：
  - 扩到更多颜色位（`PC0..PC7`）
  - 再加按键/ADC 输入逻辑

## 6. 你下一步最关键的信息

要把这份方案直接变成可编译工程，我还需要你给我一个信息：

- 你现在用的 **STM32L4 具体型号**（例如 `STM32L476RG` / `STM32L432KC`）

型号确定后，我可以直接给你：

- 对应 `TIM2_UP` 的 DMA 通道选择
- 可直接粘贴的 `MX_TIMx_Init` 参数
- 一版可编译的 `vga_port.c/.h` 初始文件

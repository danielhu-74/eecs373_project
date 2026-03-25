# Hackaday Route Migration Update Log

## Purpose

Track every migration update toward the Hackaday-style video pipeline so changes are easy to audit and roll back.

## Current Runtime Configuration (Safe Fallback Active)

- `PB9` -> `HSYNC` (`TIM4_CH4`, hardware PWM pulse)
- `PB8` -> `VSYNC` (GPIO toggled in line callback)
- `PA5`, `PA7` -> active parallel pixel data (`GPIOA->ODR` DMA route)
- `PD15` -> held low by software (legacy blue pin, currently not active data)
- `GND` -> VGA GND

Available experimental serial routes in code (not active by default):

- `PB5` as `SPI3_MOSI` (Stage 3 mode)
- `PB5` as `OCTOSPIM_P1` (Stage 4 prototype mode)

## Update Entries

### 2026-03-24 - Stage 1: Dual-DMA Skeleton Aligned to Hackaday Logic

Files changed:

- `try/Core/Src/main.c`

What changed:

- Added a new DMA handle `hdma_tim4_up_kick` on `DMA1_Channel1`.
- Configured `DMA1_Channel1` with request `DMA_REQUEST_TIM4_UP` for one-byte line-start kick.
- Kept existing scanline stream DMA (`DMA1_Channel2`, request `DMA_REQUEST_TIM2_UP`) as CH2.
- Linked TIM4 update DMA to CH1 (`__HAL_LINKDMA(&htim4, hdma[TIM_DMA_ID_UPDATE], hdma_tim4_up_kick)`).
- Enabled TIM4 DMA update request and armed CH1 in `VGA_StartDMAAndTimers()`.
- Added `line_kick_word` (line-start sync byte, currently black) as CH1 memory source.
- Kept per-line CH2 re-arm in `TIM4` callback; this preserves current behavior while moving toward Hackaday's two-stage DMA structure.
- Documented that blue channel (`PD15`) is not part of `GPIOA->ODR` DMA stream and is currently held low.

Why:

- This is a low-risk migration step that introduces the Hackaday-style "single-word sync DMA + line stream DMA" structure without breaking the existing board wiring.

Validation:

- `try/Debug/makefile` is currently generated with a Windows linker script path (`C:\...`), so local `make` in this environment stops at parse stage before compile.
- No runtime verification yet in this turn; next step should be CubeIDE regenerate/build or fix linker path in local make artifacts.

### 2026-03-24 - Stage 2: Continuous CH2 Stream + Hardware Line Re-Alignment

Files changed:

- `try/Core/Src/main.c`

What changed:

- Migrated CH1 from GPIO byte kick to a timer-alignment function:
  - `TIM4_UP -> DMA1_Channel1 -> TIM2->CNT`
  - Writes `0` each line start to re-align TIM2 phase in hardware.
- Changed CH1 data width from byte to word to match `TIM2->CNT`.
- Changed CH2 (`DMA1_Channel2`) from `DMA_NORMAL` to `DMA_CIRCULAR`.
- Removed per-line stop/restart sequence for TIM2/CH2.
- Startup now enables CH1 + CH2 once, then starts TIM2 continuously.
- `TIM4` callback now updates only CH2 base pointer (`CMAR`) per scanline period.
- Removed the old `VGA_ArmLineDMA()` stop/start helper path.

Why:

- This is closer to Hackaday’s hardware-pipeline idea: CH1 handles line timing alignment in hardware, CH2 handles pixel stream continuously, CPU only performs lightweight line pointer scheduling.

Validation:

- Local `make` still blocked by pre-existing Windows path in `try/Debug/makefile`.
- Runtime behavior pending on-target test (monitor lock, line stability, visible jitter).

### 2026-03-24 - Stage 2.1: Lower ISR Register Churn

Files changed:

- `try/Core/Src/main.c`

What changed:

- Added `active_line_src` cache for CH2 circular stream base pointer.
- `TIM4` callback now updates `DMA1_Channel2->CMAR` only when source row changes.
- Startup initializes CH2 base pointer from `active_line_src`.

Why:

- Cuts unnecessary per-line register writes and keeps CPU work closer to Hackaday’s “minimal line scheduler” style.

Validation:

- Compile/test still pending on-target due local build environment limits.

### 2026-03-24 - Stage 3: Serial Pixel Shifter Route via SPI3

Files changed:

- `try/Core/Src/main.c`

What changed:

- Added pixel path abstraction:
  - `PIXEL_PATH_GPIO_ODR` (legacy)
  - `PIXEL_PATH_SPI3_SERIAL` (Stage 3 default)
- Added `VGA_InitPixelPath()` + `VGA_StartPixelPath()`.
- Configured SPI3 as 8-bit master shifter and enabled TX-DMA mode.
- Repointed CH2 destination in Stage 3 to `SPI3->DR` instead of `GPIOA->ODR`.
- Kept CH1 line-alignment DMA (`TIM4_UP -> DMA1_Channel1 -> TIM2->CNT`) unchanged.

Why:

- This is the first practical move from direct GPIO bus output to a serial peripheral pixel shifter, which aligns with Hackaday’s “peripheral pipeline over CPU bit-banging” direction.

Validation:

- Local `make` still blocked by Windows linker path in generated makefile.
- Runtime verification pending on hardware (needs MOSI-based video wiring on `PB5`).

### 2026-03-24 - Stage 4: OCTOSPI-Centric Pixel Path (Prototype)

Files changed:

- `try/Core/Src/main.c`

What changed:

- Added `PIXEL_PATH_OCTOSPI1` mode and switched default route to Stage 4.
- Added OCTOSPI1/OCTOSPIM pixel-path init sequence:
  - enable clocks
  - configure OSPIM PCR
  - configure OCTOSPI1 for 1-line data-only stream style
- Repointed CH2 DMA destination to `OCTOSPI1->DR` in Stage 4 mode.
- Added Stage 4 start sequence enabling OCTOSPI indirect-write functional mode with DMA.
- Added an experimental pin override in `MX_GPIO_Init()`:
  - `PB5` forced to `GPIO_AF10_OCTOSPIM_P1` when Stage 4 is active.

Why:

- This shifts the architecture from SPI prototype toward an OCTOSPI-first data plane, matching the intended Hackaday migration direction.

Validation:

- Compile still not runnable in this environment due generated makefile/toolchain issues.
- GPIO pin mapping for full OCTOSPI clock/data lanes is still provisional and must be validated/tuned on hardware.

### 2026-03-24 - Stage 4.1: Display Recovery Hotfix

Files changed:

- `try/Core/Src/main.c`
- `HACKADAY_ROUTE_UPDATE_LOG.md`

What changed:

- Switched `PIXEL_PATH_MODE` default back to `PIXEL_PATH_GPIO_ODR`.
- Kept Stage 3/4 serial code paths in place for iterative bring-up.
- Updated this log section to reflect current active runtime pins.

Why:

- Stage 4 OCTOSPI route is still experimental; without validated OCTOSPI clock/data pin mux on hardware, monitor output can go black.

Validation:

- Expected immediate behavior: legacy wiring (PA5/PA7 + sync pins) should recover visible output once reflashed.

## Next Planned Steps

- Move VSYNC fully to hardware timer output to remove software timing dependence.
- Finalize OCTOSPI pin mux (clock + data lane set) in CubeMX / board wiring.
- Add per-line phase tuning parameters and update log entries with measured monitor lock behavior.

## Entry Template

Use this section format for each new update:

### YYYY-MM-DD - Short Title

Files changed:

- `path/to/file`

What changed:

- item

Why:

- item

Validation:

- build result
- runtime/monitor behavior

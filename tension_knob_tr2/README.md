# tension_knob_tr2

基于 Giraffe 官方 `TR2_480480_waterfountain_circle` 示例适配的 2.1 英寸、480 × 480 圆形旋钮屏工程。

## 硬件接口

- 编码器 A：PC4
- 编码器 B：PC5
- 编码器按键：PE12，低电平按下
- 拉力机通信：UART1，115200，8N1
- Windows 模拟串口：COM2

这些是 MCU/BSP 端口定义。后部 6 针插座的物理针序和信号电平仍需向屏厂确认或实测。

## 控制逻辑

- `apps/tension/`：BeeDrive 5.0.1 帧、遥测解析和安全状态机。
- `apps/hardware/knob/`：实体旋钮、短按、按住旋转和长按手势。
- `apps/ui/tension/`：圆屏 UI、触摸事件和遥测显示。
- `res/config/view/tension.g`：设备运行时加载的 480 × 480 UI 配置。

最终 UI 配置是直接生成的设备运行时资源，不依赖 `.cui` 文件；如需在 GiraffeIDE 可视化编辑器中拖拽修改，应在 IDE 中新建同名页面后重新导出。

## 重建固件

需要 xPack RISC-V Embedded GCC 10.2.0-1.2 和 mtools。工具链解压后执行：

```sh
TR2_TOOLCHAIN=/path/to/xpack-riscv-none-embed-gcc-10.2.0-1.2 \
  ../scripts/build_tr2_firmware.sh
```

脚本会严格编译 RISC-V 模块、写入官方 TR2 FAT12 数据镜像，并输出到 `../firmware/`。

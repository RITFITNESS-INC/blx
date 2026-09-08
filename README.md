# BLgiraffeide

拉力机 2.1 英寸、480 × 480 圆形旋钮屏控制程序与 TR2 可烧录固件。

当前已完成：

- BeeDrive 5.0.1 电机控制帧生成。
- V3/V5 固定 21 字节反馈帧解析；不使用固件已知不可靠的反馈 CHK 阻断状态读取。
- 参数范围校验。
- 480 × 480 深色圆屏 UI；触摸箭头切模式，实体旋钮调参和启停。
- TR2 UART1（115200 8N1）、PC4/PC5 编码器和 PE12 按键适配。
- 所有力值以 lb 输入和显示，发送前自动换算为电机协议的 0.1 N。
- 上电第一帧发送 `02 0E 0E 03`，优先读取并显示设备原始状态，再发送零输出停机帧；之后每 50 ms 查询主板。
- 长按启动时立即发送当前模式指令；400 ms 内无有效遥测则自动发送停机帧。
- 上电零输出、有效遥测后才能启动、400 ms 失联停机及故障锁定。
- 可直接用 TF 卡更新的 `firmware/grf_update/tr2grfdata.bin`。

本地测试：

```sh
cc -std=c11 -Wall -Wextra -Werror -Iinclude src/motor_protocol.c tests/test_motor_protocol.c -o /tmp/test_motor_protocol
/tmp/test_motor_protocol

cc -std=c11 -Wall -Wextra -Werror -Iinclude \
  src/motor_protocol.c src/tension_controller.c tests/test_tension_controller.c \
  -o /tmp/test_tension_controller
/tmp/test_tension_controller
```

最终 TR2 工程位于 `tension_knob_tr2/`，烧录说明见 `firmware/烧录说明.md`。
`scripts/build_tr2_firmware.sh` 可用 xPack RISC-V GCC 10.2 和 mtools 重建固件。
UI 预览见 `docs/ui-preview.png`。

旋钮背面 PH2.0-6P 插座正视、Pin1 到 Pin6 为 5V、5V、RX、TX、GND、GND。
UART 的逻辑高电平仍应实测确认，拉力机必须保留独立硬件急停。

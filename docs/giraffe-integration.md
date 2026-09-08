# Giraffe 工程接入说明

## 采用的示例

- UI、动画和控件 API 以 `all_control_display_anim`（Giraffe 1.2.8）为准。
- UART 打开和接收回调结构参考 `uart_demo`，不复用其 `5A A5` 寄存器协议。
- 两个示例中的 UART 测试解析存在越界或缓存边界风险，不直接复制。

## 建议目录

在 Giraffe IDE 新建实际硬件对应的 480 x 480 工程，再加入：

```text
inc/
  motor_protocol.h
  tension_controller.h
  tension_giraffe_adapter.h
  tension_giraffe_ui.h
apps/hardware/motor/
  motor_protocol.c
  tension_controller.c
  tension_giraffe_adapter.c
  tension_giraffe_ui.c
```

适配层应在 `grf_prj_create()` 之后初始化：

```c
void grf_main(void)
{
    grf_prj_create(grf_views_fun, sizeof(grf_views_fun) / sizeof(grf_view_fun_t));
    grf_hw_init();
    grf_tension_app_init();
    grf_tension_ui_bind(&tension_ui_ids);
}
```

如果 Giraffe 工程没有自动包含 `include/`，把三个头文件放入该工程的 `inc/`。

## UI 事件映射

```c
/* 编码器驱动确认后，在相应回调内调用： */
grf_tension_knob_rotate(+1, 0); /* 顺时针 */
grf_tension_knob_rotate(-1, 0); /* 逆时针 */
grf_tension_knob_short_press();
grf_tension_knob_long_press();

/* 触摸事件：只保留左右模式箭头。 */
grf_tension_touch_select_mode(TENSION_MODE_BASE);
```

正式 TR2 工程在 UART 初始化后首先发送 `02 0E 0E 03`。收到固定 21 字节反馈并保存原始设备状态后，才发送零输出停机帧建立安全基线；400 ms 无回包时执行延迟停机兜底。此后每 50 ms 重复发送遥测查询。短按和长按操作只通过实体旋钮触发。

V3/V5 反馈 CHK 的固件实现不一致，解析器不会因 CHK 不匹配丢弃反馈，但会分别记录它是否匹配“完整载荷 XOR”或“CMD + 前四个载荷字节 XOR”。帧头、命令、固定长度和帧尾仍必须全部正确。

`tension_giraffe_ui.c` 已经实现 Label、Arc 和遥测值刷新。页面生成后，用实际页面和控件 ID 填写：

```c
static const grf_tension_ui_ids_t tension_ui_ids = {
    GRF_MAIN_ID,
    MAIN_STATE_LABEL_ID,
    MAIN_MODE_LABEL_ID,
    MAIN_PARAMETER_LABEL_ID,
    MAIN_VALUE_LABEL_ID,
    MAIN_UNIT_LABEL_ID,
    MAIN_ARC0_ID,
    MAIN_FORCE_LABEL_ID,
    MAIN_SPEED_LABEL_ID,
    MAIN_POSITION_LABEL_ID
};
```

不需要的控件 ID 填 `GRF_TENSION_CTRL_UNUSED`。推荐映射：

- `state`：顶部连接/停止/运行/故障状态。
- `mode`：当前输出模式。
- `selected_parameter`：当前由旋钮调整的参数。
- `settings`：大号设定值和 Arc 进度。
- `telemetry`：实时力、速度与位置。

## 仍需按硬件修改的内容

1. `GRF_TENSION_UART_PORT`：TR2 正式工程使用 UART1。
2. 旋钮 6 针接口为 Pin1/2=5V、Pin3=RX、Pin4=TX、Pin5/6=GND；仍需确认 UART 逻辑电平。
3. 编码器使用 PC4/PC5，按键使用 PE12。
4. 适配层按 Giraffe 1.2.8 的 `grf_task_create()` 周期任务 API 编写；导入实际空白工程后再做一次目标固件编译核对。
5. UI 力上限为 50.0 lb，发送前自动换算成协议的 0.1 N；实机验证前不要提高。
6. 文档没有给出 IDLE、RUNNING、POSITION、BENCH_TEST 的确切数值；当前 UI 显示 `设备0xXX`，确认固件映射前不要按顺序猜测。

软件停止会发送基础阻力模式、双向力为零、SSR 关闭的帧。这是协议范围内的停机请求，不能替代独立物理急停和电机主板失联保护。

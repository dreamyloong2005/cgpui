# Independent Standards review — X11 test display `-noreset`

2026-11-24。**结论：0 confirmed actionable findings；无文档规范违例，无需要提出的基线 code-smell finding。**

## 范围与标准

只读审查实际工作树相对 HEAD 的这两个文件：

```text
git diff HEAD -- scripts/dev/test-display.sh tests/architecture/phase_i_x11_data_transfer_structure_test.cpp
```

HEAD：`ce3b2e8f402741c972c7af00f9c09ca8b5784870`。补读同一 diff 的完整上下文；标准为当前 `AGENTS.md` 的 modular-first、focused ownership/结构门、mcpp-only。仅承担 Standards 轴，不代替独立 Spec/运行验收。

## 审查结果

- **归属/范围正确。** `scripts/dev/test-display.sh` 只在已有私有 Xvfb 启动命令增加 `-noreset` 及两行说明。没有把 X11 测试环境策略塞进 platform/renderer 实现，没有改 native clipboard、xcb connection 行为，也没有引入环境开关、公共 API 或额外后台服务。
- **配套结构门。** 现有 `phase_i_x11_data_transfer_structure_test.cpp` 同一显示隔离检查块增加启动参数断言，符合现有 textual structure-test 风格。原有 focused module 240行上限、mcpp discover断言和启动器委托检查未放宽。没有新源文件/测试，因此无需新增 discover项；无xmake变化。
- **owned Xvfb cleanup 保持。** `-noreset` 不替换进程归属：PID仍取本次后台启动的 `$!`；退出trap仍对该PID执行kill/wait并清本次临时log；启动失败路径也仍先kill/wait再清trap。改动没有改为按名称结束进程、操作用户session X server，或通过exec绕过shell EXIT清理。
- **host clipboard isolation 保持且范围明确。** 成功启动私有server后仍只向当前脚本及子进程export该DISPLAY，`-nolisten tcp`保留。改动不连接或修改host clipboard。原有显式`CGPUI_TEST_DISPLAY=session`、缺少Xvfb及全部启动失败时回退session并告警的边界完全未变；因此不能宣传为任何环境下无条件隔离host clipboard。
- **未见不必要抽象/职责扩张。** 此变更在已拥有该责任的两个focused文件内完成，没有重复状态机、额外参数或无关重构；无需为了三行启动策略拆新模块。

## 证据边界

父任务提供同一native binary的original/noreset对照、结构guard A exit4/B pass；这些是父任务的运行证据，本审查未重跑或扩大其结论。仅凭本diff不宣称WSL full、platform55、Windows full或任意host环境全部通过。没有读取/操作父验收session，没有执行build/test/GPU或Xvfb命令。

本次唯一写入为此review报告；工程文件未修改。

## 被审查工作树指纹

- `D:/Dev/Projects/cgpui/scripts/dev/test-display.sh`：`172a6a68d7fdc7ccf68763c0a4189de3594eff479e4bfc7813ba96affb8cc436`
- `D:/Dev/Projects/cgpui/tests/architecture/phase_i_x11_data_transfer_structure_test.cpp`：`30d1fa16f6da11be0bdd33174f8ff7b5b39e44a870591c395da0b976f5357c8a`

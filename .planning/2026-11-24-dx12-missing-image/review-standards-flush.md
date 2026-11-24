# Independent Standards review — native-reuse stdout flush

2026-11-24。**0 confirmed actionable findings；无文档规范违例或需要提出的code-smell finding。**

## 被审范围

`D:/Dev/Projects/cgpui/tests/renderer/vulkan_missing_image_native_reuse_test.cpp` 55–56行：末尾既有success marker之后增加说明和`std::fflush(stdout)` / `std::ferror(stdout)`检查，失败返回27。

读取时HEAD为`6b2de26494b34ebf5ef67a54f7433632e71fe96b`。该test目前为untracked，因此`git diff HEAD -- <path>`为空；没有把空diff误当无改动。使用只读`git diff --no-index`对照已冻结revised candidate：

`D:/Dev/shots/cgpui-dx12-missing-image-20261124/native-reuse/revised/candidate/tests/renderer/vulkan_missing_image_native_reuse_test.cpp`

确认实际差异仅为上述两行。当前实际test SHA-256：`7ae41e41d2ad1f8e98266967e53fa470f3b352b015862dda3076cb62c4a38f7b`。

## Standards结论

- **正确归属、最小范围。** 这是该验收test自身stdout证据完成条件，不是renderer行为；留在test入口末尾合理，无需另建模块或修改公有API。现有`<cstdio>`已提供所用接口。
- **失败闭合。** flush失败或stdout已有错误时返回非零，避免退出0被误当完整日志证据；成功marker、native调用、phase/ordering/pixel predicates均未变化。flush位于全部验证之后，不参与GPU等待/timeout或缓存命中判定。
- **模块/结构不回退。** 文件58行，现有结构门100行cap未放宽；没有新test/source需要discover注册。mcpp、ASan入口、production native lifetime和observer均未改。
- **不是数据完整性的过度承诺。** flush仅要求C流缓冲输出；不保证下游日志采集器/磁盘持久化，也不能仅凭flush成功替代父任务的marker及退出码联合审计。

## 只读证据与限制

读取`scripts/dev/asan_entry_windows.cpp`确认该入口调用`_cexit()`后`TerminateProcess`，不是普通CRT进程退出；新增注释没有声称跳过全部CRT清理。此审查不把缺失日志进一步归因于某个已证明的CRT内部缺陷。

读取external `verified/asan-native-reuse-direct-original.json`及stdout尾：原样binary退出0、stdout4148字节、marker为0，尾行截断；`verified/audit-original-failure.log`是ASan完整marker断言失败。它们支持这是证据输出缺口，但不是本次补丁已通过的新运行结果。

父任务负责focused ASan与整个矩阵重新冻结。本次只读源/已保存日志，仅写此review；无build/test/GPU/spawn/工程编辑或root session操作。

# YRppQuickBoard

本项目提供了一个QuickBoard按键命令（快速登乘），在选中的单位之间自动配对并执行装载命令。
- 基于Syringe，设计上可以与其他模组一起工作，在线游戏中不会导致同步错误
- 自动计算空位，就近配对装载
- 分配成功的自动取消选择，无处可去的单位或仍有空位的载具则保持选中状态
- 在“键盘”界面的“选项”类别下，默认按键为Ctrl+X
- I18n key：
  - 新增：`TXT_QUICK_BOARD`（“键盘”界面中的命令名），`TXT_QUICK_BOARD_DESC`（命令描述）
  - 复用：`TXT_SELECTION`（“键盘”界面中的“选项”类别），`MSG:NothingSelected`（游戏内提示“未选取”）

## Credits

[Phobos-developers/YRpp](https://github.com/Phobos-developers/YRpp)
[Phobos-developers/SyringeEx](https://github.com/Phobos-developers/SyringeEx)
[Phobos-developers/Phobos](https://github.com/Phobos-developers/Phobos)

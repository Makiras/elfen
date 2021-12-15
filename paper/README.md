# 作图

## 数据解析

`lib.py`通过读取`data/`目录下对应策略的`conclusion.csv`，使用csv库解析数组。
具体的策略名称在`coco.py`（LC&BE均有）`single.py`（仅有LC）的函数中配置。

## 数据绘制

使用`matplotlib`及`TkAgg`绘制。使用时请启用 `X11Forward` （直接mobaxterm就行）。
会在 `paper/picture`目录下绘制
- process_time：程序的total process time以及cpu utilization关系
- ipc：LC应用的ipc，以及BE应用的等效IPC（以单跑BE的每秒matrix计算数量为4，按比例映射至0-4）
- cpi：
    * BE应用的延时比例。以单独运行BE的每秒matrix次数为基准（1）,越慢越高（耗时为n倍）。
    * 分别计算了running时间（分配到的时间片）内变慢，和总时间（处理完所有请求所需时间）变慢的比率。  
    例如5s内，调度器分配了1s的时间片，计算了1基准的matrix次数。那么running ratio就是1，total ratio就是5.
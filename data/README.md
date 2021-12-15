# 数据处理

## 重新解析

如果发现原始数据解析出错，可以修改`parseTask.py`脚本后执行，会遍历目录并重新解析。
```bash
./reparse.sh
```

## 生成csv报告

为了在画图时提升效率，降低文件读取数量。使用该脚本会将策略下的所有文件汇集成一个csv日志文件。
日志文件包含如下数据，p50代表前50%的cpu process time，l50代表前50%的total process time。
matrix代表计算矩阵的次数，user_t代表用户态时间，sys_t代表内核态时间（让出时间）
recv time代表从处理完所有请求的时间，可以认为是BE应用的运行时间。

```bash
./policy2csv.sh

# QPS,RealQPS,p50,p95,p99,l50,l95,l99,utilization,IPC,send time,recv time,matrix,user_t,sys_t,b_utilization
```

## 命名规则

`coco-data`：美团colocation策略
`idle-co-dat`：elfen borrow idle策略
`refresh-*-data`：corunning时间为 * ms，策略为refresh
`dynamic-a*-b*-data`：corunning时间为 a* ms，目标ipc为 b*，策略为dynamic



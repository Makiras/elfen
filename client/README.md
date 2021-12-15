# 运行测试

## 启动lucene

### elfen nanonap kernel

1. 查看内核, 如果不是nanonap kernel则更换
```bash
sudo grubby --set-default=/boot/vmlinuz-4.18.0-240.el8.nanonap.x86_64
```
2. 新建screen，启动lucene，然后退出screen
```bash
cd elfen/lucene/util
./scripts/runSearchServer.sh
[ctrl-A-D]
```

### colocation

1. 查看内核, 如果不是240 kernel则更换
```bash
sudo grubby --set-default=/boot/vmlinuz-4.18.0-240.el8.x86_64
```
2. 新建screen，启动lucene，然后退出screen
```bash
cd elfen/client/runTemplate
./init-coco.sh
[ctrl-A-D]
```

### 以下操作相同

3. 预热lucene（文件缓存等）
```bash
cd elfen/client/runTemplate
./run-single.sh
```
等待运行一轮（比如测完360qps下的请求）后ctrl-c。  
然后注意kill掉发送请求的python进程(sendTask.py)，该进程不随kill脚本结束而结束。
```bash
ps -x|grep python2
```


4. 运行对应策略的脚本。如果要修改参数，在`microbench`中修改，并重新编译。
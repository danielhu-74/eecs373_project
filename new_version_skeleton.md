# 代码框架重建
## 3种模式
start_page --> play_mode --> final_stage
- 可能需要处理的是加入中间暂停的mode
- 这部分可能需要在fpga上加
- 需要在fpga上加交互和按键 的部分

### start_page
现在在这个代码上要做的是 做一个tap to start。
1. 做一个简单的显示的tap to start
2. I want to do a division for two players can both tap and that means you are ready
3. after pressing the button, it will change into the version "ready!", and after both players have tapped, the game begin
4. make the tap to start to sprinkle
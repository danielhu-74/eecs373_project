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
4. when it changes into ready for one person, it has to show another line beneath it, says "tap to quit", and literally wnhen tapping, it will changes back to teh "tap to start" mode.
5. (optional, the final stage)make the tap to start to sprinkle

### play_mode
1. must have the score for both the player and have signal like "player 1" and "player 2".
2. we need a ":" between these two numbers.
3. (optional, the final stage) color rendering and background embelishment 

### final stage 
1. changes P1 to player 1
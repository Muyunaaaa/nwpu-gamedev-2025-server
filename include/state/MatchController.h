#pragma once
#include <cstdint>
#include <memory>
#include <span>
#include <string>

#include "RoomContext.h"
#include "state/GameState.h"

class MatchController {
private:
    std::unique_ptr<GameState> currentState;


    //全局状态
    //权限
    bool game_over = false;
    bool a_fire = false;
    bool a_purchase = false;
    bool a_move = false;
public:
    using MilliSecDuration = std::chrono::duration<float, std::chrono::milliseconds::period>;
    //常量
    static constexpr MilliSecDuration PURCHASE_TIMER = MilliSecDuration(10.0f * 1000.0f);
    static constexpr MilliSecDuration ROUND_TIMER = MilliSecDuration(120.0f * 1000.0f); // 120.0s;
    static constexpr MilliSecDuration C4_TIMER = MilliSecDuration(45.0f * 1000.0f);

    //全局状态
    //计数
    int currentRound = 1;

    //单局状态
    bool c4_planted = false;
    bool round_running = false;
    bool c4_defused = false;
    int players_alive = RoomContext::getInstance().getTotalPlayerCount();
    int ct_alive = RoomContext::getInstance().getTotalPlayerCount()/2;
    int t_alive = RoomContext::getInstance().getTotalPlayerCount()/2;
    int planter_id = -1; //-1:无效id
    int defuser_id = -1;
    uint16_t winner_team = 0; //0:无胜利，1:恐怖分子胜利，2:反恐精英胜利
    uint16_t c4_planted_site = 0; //0:未下包，1:A点，2:B点

    void ChangeState(std::unique_ptr<GameState> newState);

    void Tick(float deltaTime);
    void BroadcastMessage(std::span<const uint8_t> message);
    void CheckWinCondition();
    //TODO:补充其他功能
    //round
    void plantC4();
    void roundStart();
    void roundEnd();
    void defuseC4();
    void ctWin();
    void tWin();
    void initRound();
    void resetRound();
    void killACt();
    void killAT();
    //enable
    void enableFire();
    void enablePurchase();
    void enableMove();
    //disable
    void disableFire();
    void disablePurchase();
    void disableMove();
    //able
    bool fire_able();
    bool purchase_able();
    bool move_able();

    void gameStart();
    void gameEnd();
};

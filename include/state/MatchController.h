#pragma once
#include <cstdint>
#include <memory>
#include <span>
#include <string>

#include "RoomContext.h"
#include "game/PlantSite.h"
#include "game/PlayerTeam.h"
#include "state/GameState.h"

/*
 *MathchController 负责管理游戏游戏开始后的整体状态机和全局状态
 */
class MatchController {
private:
    MatchController() = default;

public:
    static MatchController& Instance() {
        static MatchController instance;
        return instance;
    }

    MatchController(const MatchController&) = delete;
    MatchController& operator=(const MatchController&) = delete;

private:
    std::unique_ptr<GameState> currentState;

    bool game_over = false;
    bool a_fire = false;
    bool a_purchase = false;
    bool a_move = false;

public:
    using MilliSecDuration =
        std::chrono::duration<float, std::chrono::milliseconds::period>;

    //TODO:改为配置文件加载
    static constexpr MilliSecDuration PURCHASE_TIMER =
        MilliSecDuration(10.0f * 1000.0f);

    static constexpr MilliSecDuration ROUND_TIMER =
        MilliSecDuration(10.0f * 1000.0f);

    static constexpr MilliSecDuration C4_TIMER =
        MilliSecDuration(45.0f * 1000.0f);

    static constexpr MilliSecDuration END_UNTIL_SERVER_CLOSE_TIMER =
        MilliSecDuration(15.0f * 1000.0f);

    static constexpr int WIN_PRIZE = 2500;
    static constexpr int LOSE_PRIZE = 2000;
    static constexpr int KILL_PRIZE = 300;
    static constexpr int PLANT_PRIZE = 200;
    static constexpr int DEFUSE_PRIZE = 200;
    static constexpr int MAX_BALANCE = 6000;

    //比赛状态
    int currentRound = 1;
    int ct_win_round = 0;
    int t_win_round = 0;

    //回合状态
    bool c4_planted = false;
    bool round_running = false;
    bool c4_defused = false;
    PlantSite c4_plant_site = PlantSite::None;

    PlayerTeam winner_team = PlayerTeam::NONE;

    void ChangeState(std::unique_ptr<GameState> newState);

    void Tick(float deltaTime);
    void BroadcastMessage(std::span<const uint8_t> message);

    // match
    uint16_t checkMatchWin();

    // round
    void plantC4(PlantSite c4_plant_site);
    void roundStart();
    void roundEnd();
    void defuseC4();
    void ctWin();
    void tWin();
    void initRound();
    void resetRound();

    // enable
    void enableFire();
    void enablePurchase();
    void enableMove();

    // disable
    void disableFire();
    void disablePurchase();
    void disableMove();

    // able
    bool fire_able() const;
    bool purchase_able() const;
    bool move_able() const;
    bool plant_able() const;
    bool defuse_able() const;

    // lifecycle
    void gameStart();
    void gameEnd();
};
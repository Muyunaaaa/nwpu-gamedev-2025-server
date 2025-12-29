#pragma once
#include <cstdint>
#include <memory>
#include <span>

#include "RoomContext.h"
#include "game/PlantSite.h"
#include "game/PlayerTeam.h"
#include "math/common.h"
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

    //比赛状态
    int currentRound = 1;
    int ct_win_round = 0;
    int t_win_round = 0;

    //回合状态
    bool c4_planted = false;
    bool c4_planted_and_counting = false;//炸弹计时器开始
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
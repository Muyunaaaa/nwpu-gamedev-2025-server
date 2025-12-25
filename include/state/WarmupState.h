#include "state/GameState.h"
#include "state/MatchController.h"
class WarmupState : public GameState {
public:
    void OnEnter(MatchController *controller) override;
    void Update(MatchController *controller, float deltaTime) override;
    void OnExit(MatchController *controller) override;
    WarmupState() = default;
    ~WarmupState() override = default;
};

#ifndef EGODATTACKEVENT_H
#define EGODATTACKEVENT_H

#include "egameevent.h"

class eSanctuary;
enum class eGodType;

class eGodAttackEvent : public eGameEvent {
public:
    eGodAttackEvent(const eCityId cid,
                    const eGameEventBranch branch,
                    eGameBoard& board);

    void setSanctuary(const stdptr<eSanctuary>& s);

    const std::vector<eGodType>& types() const { return mTypes; }
    void setTypes(const std::vector<eGodType>& types);

    void setRandom(const bool r);
    bool random() const { return mRandom; }

    void loadResources() const override;

    void trigger() override;
    bool oraclePrediction(eEventData& ed, eMessageType& msg) override;
    std::string longName() const override;

    void write(eWriteStream& dst) const override;
    void read(eReadStream& src) override;
private:
    stdptr<eSanctuary> mSanctuary;
    std::vector<eGodType> mTypes;
    int mNextId = 0;
    bool mRandom = false;
    int mOracleGod = -1; // random god already chosen by the Oracle
};


#endif // EGODATTACKEVENT_H

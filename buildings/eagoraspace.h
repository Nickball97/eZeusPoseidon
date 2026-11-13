#ifndef EAGORASPACE_H
#define EAGORASPACE_H

#include "ebuilding.h"

class eAgoraBase;

class eAgoraSpace : public eBuilding {
public:
    eAgoraSpace(const stdsptr<eAgoraBase>& agora,
                eGameBoard& board,
                const eCityId cid);

    stdsptr<eTexture> getTexture(const eTileSize size) const;

    eAgoraBase* agora() const { return mAgora.get(); }

    // an empty stall space is part of its agora too (see eVendor)
    bool isOnFire() override;
    void setOnFire(const bool f) override;
    int provide(const eProvide p, const int n) override;
private:
    const stdsptr<eAgoraBase> mAgora;
};

#endif // EAGORASPACE_H

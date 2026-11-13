#ifndef ETRIREME_H
#define ETRIREME_H

#include "eboatbase.h"
#include "efightingcharacter.h"

class eTriremeAction;

class eTrireme : public eBoatBase,
                 public eFightingCharacter {
public:
    eTrireme(eGameBoard& board);

    static void sPlace(std::vector<eTrireme*> bs,
                       const int ctx, const int cty,
                       eGameBoard& board, const int dist,
                       const int minDistFromEdge);

    eTriremeAction* triremeAction() const;

    eOverlay getSecondaryTexture(const eTileSize size) const override;

    void setSelected(const bool s) { mSelected = s; }
    bool selected() const { return mSelected; }
    bool selectable() const;

    void goHome();

    // Poseidon manual (Atlantean Navy: Frigates): an Atlantean city's
    // trireme is a frigate; with a load of orichalc its wharf fits an
    // Atlantean Fire launcher, which sets targets ablaze and makes the
    // frigate stronger
    bool frigate() const { return atlantean(); }
    bool atlanteanFire() const { return mAtlanteanFire; }
    void setAtlanteanFire(const bool f) { mAtlanteanFire = f; }

    void read(eReadStream& src) override;
    void write(eWriteStream& dst) const override;
protected:
    eCharTexs charTexs() const override;
private:
    bool mSelected = false;
    bool mAtlanteanFire = false;
};

#endif // ETRIREME_H

#ifndef ETAXCOLLECTOR_H
#define ETAXCOLLECTOR_H

#include "ebasicpatroler.h"

class eTaxCollector : public eBasicPatroler {
public:
    eTaxCollector(eGameBoard& board);

    void provideToBuilding(eBuilding* const b) override;

    // sent out by a blessed tax office: the houses he visits pay 4 times
    // their tax (original exe 0x004A67C0 and 0x00488250)
    void setBlessed(const bool b) { mBlessed = b; }
private:
    bool mBlessed = false;
};

#endif // ETAXCOLLECTOR_H

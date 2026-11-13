#ifndef EWOLFSPAWNER_H
#define EWOLFSPAWNER_H

#include "eanimalspawner.h"

class eWolfSpawner : public eAnimalSpawner {
public:
    eWolfSpawner(const int id, eTile* const tile,
                 eGameBoard& board);

    stdsptr<eWildAnimal> create(eGameBoard& board) override;

    // Nicolás's mod rule: no new wolves once the city's Artemis sanctuary
    // is finished
    void incTime(const int by) override;
};

#endif // EWOLFSPAWNER_H

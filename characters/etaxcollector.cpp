#include "etaxcollector.h"

#include "textures/egametextures.h"

#include "elimits.h"
#include "buildings/ebuilding.h"
#include "engine/egameboard.h"

eTaxCollector::eTaxCollector(eGameBoard& board) :
    eBasicPatroler(board, &eCharacterTextures::fTaxCollector,
                   eCharacterType::taxCollector) {
    eGameTextures::loadTaxCollector();
    setProvide(eProvide::taxes, __INT_MAX__);
}

void eTaxCollector::provideToBuilding(eBuilding* const b) {
    const int r = b->provide(eProvide::taxes, provideCount());
    if(r > 0 && mBlessed) getBoard().payTaxes(cityId(), 3*r, 0);
}

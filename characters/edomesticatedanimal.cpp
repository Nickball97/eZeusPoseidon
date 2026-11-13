#include "edomesticatedanimal.h"

#include "enumbers.h"
#include "fileIO/efileformat.h"

eDomesticatedAnimal::eDomesticatedAnimal(
        eGameBoard& board,
        const eCharTexs charTexs,
        const eCharacterType type,
        const int maxGroom) :
    eAnimal(board, charTexs, type),
    mMaxGroom(maxGroom) {}

void eDomesticatedAnimal::groom() {
    // tending no longer decides when an animal is ready; time does (see
    // incTime). Kept so herders still look after their animals.
}

void eDomesticatedAnimal::incTime(const int by) {
    eAnimal::incTime(by);
    if(mResource > 0) return;
    mGrowTime += by;
    const int year = 365*eNumbers::sDayLength;
    const int need = mFirstCycle ? year/2 : year;
    if(mGrowTime >= need) {
        mGrowTime = 0;
        mFirstCycle = false;
        mResource = 1;
        setFleecedTexture();
    }
}

int eDomesticatedAnimal::collect() {
    if(mResource <= 0) return 0;
    const int r = mResource;
    mResource = 0;
    setNakedTexture();
    return r;
}

void eDomesticatedAnimal::read(eReadStream& src) {
    eAnimal::read(src);
    src >> mGroomed;
    src >> mResource;
    if(src.formatVersion() >= eFileFormat::domesticatedYearly) {
        src >> mGrowTime;
        src >> mFirstCycle;
    }
    if(mResource == 0) {
        setNakedTexture();
    } else {
        setFleecedTexture();
    }
}

void eDomesticatedAnimal::write(eWriteStream& dst) const {
    eAnimal::write(dst);
    dst << mGroomed;
    dst << mResource;
    dst << mGrowTime;
    dst << mFirstCycle;
}

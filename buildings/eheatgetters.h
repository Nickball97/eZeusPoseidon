#ifndef EHEATGETTERS_H
#define EHEATGETTERS_H

#include "buildings/ebuilding.h"
#include "engine/boardData/eheatmap.h"

enum class eGodType;
enum class eMonsterTarget;

namespace eHeatGetters {
    using eHeatGetter = std::function<eHeat(eBuildingType)>;

    eHeat appeal(const eBuildingType type);
    eHeat housing(const eBuildingType type);
    eHeat culture(const eBuildingType type);
    eHeat industry(const eBuildingType type);
    eHeat farming(const eBuildingType type);
    eHeat storage(const eBuildingType type);

    eHeatGetter godLeaning(const eGodType gt);

    // buildings a monster aimed at target t goes for (reading of the
    // original target names; the manuals don't describe them)
    bool isMonsterTarget(const eMonsterTarget t, const eBuildingType bt);
    eHeatGetter monsterTarget(const eMonsterTarget t);

    eHeat empty(const eBuildingType type);
    eHeat any(const eBuildingType type);

    eHeat fertile(eTileBase* const tile);
    eHeat notFertile(eTileBase* const tile);
}

#endif // EHEATGETTERS_H

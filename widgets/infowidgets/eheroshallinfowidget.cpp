#include "eheroshallinfowidget.h"

#include "buildings/eheroshall.h"
#include "widgets/elabel.h"
#include "widgets/eframedbutton.h"
#include "textures/egametextures.h"
#include "engine/egameboard.h"
#include "elanguage.h"
#include "estringhelpers.h"
#include "characters/heroes/ehero.h"
#include "characters/monsters/emonster.h"

void eHerosHallInfoWidget::initialize(eHerosHall* const b) {
    b->updateRequirementsStatus();
    const auto ht = b->heroType();
    const auto title = eBuilding::sNameForBuilding(b);
    eInfoWidget::initialize(title);

    const int p = padding();
    const auto& reqs = b->requirements();
    const auto heroName = eHero::sHeroName(ht);

    {   // as in the original: where the hero stands, centred under the
        // title (185:63-68), then "Theseus Requires ... Status" (185:59-60)
        int metAll = 0;
        for(const auto& r : reqs) if(r.met()) metAll++;
        const auto stage = b->stage();
        int string;
        if(stage == eHeroSummoningStage::summoned) {
            string = 65; // travelling to the city
        } else if(stage == eHeroSummoningStage::arrived) {
            string = b->heroOnQuest() ? 66 : 68; // away / in the city
        } else {
            string = metAll >= int(reqs.size()) ? 64 : 63;
        }
        auto txt = eLanguage::zeusText(185, string);
        eStringHelpers::replace(txt, "[hero_name]", heroName);
        const auto sub = new eLabel(window());
        sub->setNoPadding();
        sub->setSmallFontSize();
        sub->setText(txt);
        sub->fitContent();
        const auto subW = addRegularWidget(sub->height());
        subW->addWidget(sub);
        sub->align(eAlignment::hcenter);

        const auto reqL = new eLabel(window());
        reqL->setNoPadding();
        reqL->setSmallFontSize();
        reqL->setText(heroName + " " + eLanguage::zeusText(185, 59));
        reqL->fitContent();
        const auto stL = new eLabel(window());
        stL->setNoPadding();
        stL->setSmallFontSize();
        stL->setText(eLanguage::zeusText(185, 60));
        stL->fitContent();
        const auto head = addRegularWidget(reqL->height());
        head->addWidget(reqL);
        head->addWidget(stL);
        stL->setX(head->width() - stL->width());
    }

    int met = 0;
    int y = 0;
    std::vector<eWidget*> wids;
    for(const auto& r : reqs) {
        const auto w = new eWidget(window());
        wids.push_back(w);
        w->setNoPadding();
        w->setWidth(widgetWidth() - 2*p);

        const bool finished = r.met();
        if(finished) met++;

        int x = 0;

        {
            const auto checkBox = new eLabel(window());
            checkBox->setNoPadding();

            const auto res = resolution();
            const auto uiScale = res.uiScale();
            const int iRes = static_cast<int>(uiScale);
            const auto& intrfc = eGameTextures::interface();
            const auto& texs = intrfc[iRes].fCheckBox;

            if(finished) {
                checkBox->setTexture(texs.getTexture(0));
            } else {
                checkBox->setTexture(texs.getTexture(1));
            }
            checkBox->fitContent();
            x += checkBox->width() + p;

            w->addWidget(checkBox);
        }

        const auto cid = b->cityId();
        auto& board = b->getBoard();
        const auto c = board.boardCityWithId(cid);
        {
            const auto reqStr = eHerosHall::sHeroRequirementText(r, *c);
            const auto reqLabel = new eLabel(window());
            reqLabel->setNoPadding();
            reqLabel->setSmallFontSize();
            if(!finished) reqLabel->setYellowFontColor();
            reqLabel->setText(reqStr);
            reqLabel->fitContent();

            w->addWidget(reqLabel);
            reqLabel->setX(x);
        }

        {
            const auto statusStr = eHerosHall::sHeroRequirementStatusText(r, *c);
            const auto sttsLabel = new eLabel(window());
            sttsLabel->setNoPadding();
            sttsLabel->setSmallFontSize();
            if(!finished) sttsLabel->setYellowFontColor();
            sttsLabel->setText(statusStr);
            sttsLabel->fitContent();

            w->addWidget(sttsLabel);
            sttsLabel->align(eAlignment::right);
        }

        w->setY(y);
        w->fitContent();
        y += w->height() + p;
    }

    const auto wid = addFramedWidget(y + p);
    for(const auto w : wids) {
        wid->addWidget(w);
    }

    {   // "Hercules is needed to do battle with the Hydra." (185:71-86)
        // when one of the hero's monsters is in the city, as in the original
        int string = -1;
        auto& board = b->getBoard();
        for(const auto m : board.monsters(b->cityId())) {
            if(!m) continue;
            bool valid = false;
            const auto mt = eMonster::sCharacterToMonsterType(m->type(), &valid);
            if(!valid || eMonster::sSlayer(mt) != ht) continue;
            switch(mt) {
            case eMonsterType::hector: string = 71; break;
            case eMonsterType::maenads: string = 72; break;
            case eMonsterType::cerberus: string = 73; break;
            case eMonsterType::hydra: string = 74; break;
            case eMonsterType::dragon: string = 75; break;
            case eMonsterType::talos: string = 76; break;
            case eMonsterType::cyclops: string = 77; break;
            case eMonsterType::scylla: string = 78; break;
            case eMonsterType::kraken: string = 79; break;
            case eMonsterType::medusa: string = 80; break;
            case eMonsterType::calydonianBoar: string = 81; break;
            case eMonsterType::minotaur: string = 82; break;
            case eMonsterType::sphinx: string = 83; break;
            case eMonsterType::harpies: string = 84; break;
            case eMonsterType::chimera: string = 85; break;
            case eMonsterType::echidna: string = 86; break;
            default: break;
            }
            if(string != -1) break;
        }
        if(string != -1) {
            const auto l = new eLabel(window());
            l->setNoPadding();
            l->setSmallFontSize();
            l->setText(eLanguage::zeusText(185, string));
            l->fitContent();
            const auto w = addRegularWidget(l->height());
            w->addWidget(l);
            l->align(eAlignment::hcenter);
        }
    }

    const int reqC = reqs.size();
    const auto stage = b->stage();
    if(met >= reqC && stage == eHeroSummoningStage::none) {
        const auto summonButton = new eFramedButton(window());
        const auto txt = eLanguage::zeusText(185, 62) + " " + eHero::sHeroName(ht);
        summonButton->setText(txt);
        summonButton->setSmallFontSize();
        summonButton->fitContent();
        summonButton->setUnderline(false);
        const stdptr<eHerosHall> bptr(b);
        summonButton->setPressAction([bptr, summonButton]() {
            if(!bptr) return;
            bptr->summon();
            summonButton->deleteLater();
        });
        const auto wid = addRegularWidget(summonButton->height());
        wid->addWidget(summonButton);
        summonButton->align(eAlignment::center);
    }
}

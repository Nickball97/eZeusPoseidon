#ifndef EWORLDMAPWIDGET_H
#define EWORLDMAPWIDGET_H

#include "elabel.h"

#include "engine/eworldboard.h"

class eWorldMapWidget : public eLabel {
public:
    eWorldMapWidget(eMainWindow* const window);

    using eColonySelection = std::vector<stdsptr<eWorldCity>>;
    void setSelectColonyMode(const bool scm, const eColonySelection& s);

    void setBoard(eGameBoard* const b);
    void setWorldBoard(eWorldBoard* const b);

    using eSelectCityAction = std::function<void(const stdsptr<eWorldCity>&)>;
    void setSelectCityAction(const eSelectCityAction& s);
    using eSetTextAction = std::function<void(const std::string&)>;
    void setSetTextAction(const eSetTextAction& s);

    void updateWidgets();

    void setMap(const eWorldMap map);
    // sizes the map to fill w x h keeping its shape, drawn from the
    // smallest of eZeus's map copies (1x, 1.5x, 2x) that is big enough
    void fitTo(const int w, const int h);
protected:
    void paintEvent(ePainter& p);
    bool mousePressEvent(const eMouseEvent& e);
private:
    void paintEdgeFade(ePainter& p) const;
    void armyDrawXY(eWorldCity& c1, eWorldCity& c2,
                    const double frac, int& x, int& y);

    bool cityVisible(const stdsptr<eWorldCity>& c,
                     const bool editor) const;

    bool mSelectColonyMode = false;
    std::vector<stdsptr<eWorldCity>> mColonySelection;

    eGameBoard* mGameBoard = nullptr;
    eWorldBoard* mWorldBoard = nullptr;
    int mFrame = 0;

    std::string mMapFile; // e.g. "Zeus_MapOfGreece01.JPG"
    stdsptr<eTexture> mDrawTex; // the copy drawn, scaled to the widget

    std::map<std::string, stdsptr<eTexture>> mNames;

    eSelectCityAction mSelectCityAction;
    eSetTextAction mSetTextAction;
};

#endif // EWORLDMAPWIDGET_H

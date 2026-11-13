#ifndef EMESSAGEBOX_H
#define EMESSAGEBOX_H

#include "eframedwidget.h"

#include "emessage.h"
#include "engine/edate.h"
#include "engine/emessagehistory.h"

class eWorldCity;
struct eEventData;
enum class eResourceType;
enum class eCityId;
class eLabel;
class eGameWidget;

class eMessageBox : public eFramedWidget {
public:
    using eFramedWidget::eFramedWidget;

    void initialize(eGameBoard &board,
                    const eEventData& ed,
                    const eAction& viewTile,
                    const eAction& closeFunc,
                    eMessage msg);

    // a message from the city's history (Messages window, side panel)
    void initializeRecord(const eMessageRecord& r,
                          const std::string& playerName,
                          const eAction& closeFunc);

    // the message with names, goods, numbers etc. filled in from ed
    static eMessage sFillIn(const eEventData& ed, eMessage msg);

    void close();
    bool closable() const { return mClosable; }

    eWidget* createTributeWidget(const eResourceType type,
                                 const int count, const int space,
                                 const int months = -1,
                                 eLabel** spaceLabelPtr = nullptr);
protected:
//    void paintEvent(ePainter& p);
    bool keyPressEvent(const eKeyPressEvent& e);
    bool mousePressEvent(const eMouseEvent& e);
private:
    bool mClosable = false;
    eAction mCloseFunc;
//    eAction mDone;
};

#endif // EMESSAGEBOX_H

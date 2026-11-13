#ifndef EMESSAGESWIDGET_H
#define EMESSAGESWIDGET_H

#include "eframedwidget.h"

class eGameBoard;
class eButton;
class eLabel;

// The Messages window (text group 63, zeus.exe 0x00522340 / 0x00522620):
// every message the city received, newest first, 10 rows at a time with
// arrows to scroll. Left-click a row to read it, right-click to delete it;
// "Delete opened messages" removes every message already read.
class eMessagesWidget : public eFramedWidget {
public:
    using eFramedWidget::eFramedWidget;

    using eOpenAction = std::function<void(int)>;
    void initialize(eGameBoard* const board, const eOpenAction& open);
private:
    void updateRows();

    static const int sRows = 10;

    eGameBoard* mBoard = nullptr;
    eOpenAction mOpen;
    int mTop = 0;
    eWidget* mRowsW = nullptr;
    eWidget* mEmptyW = nullptr;
    int mDateW = 0;
    int mIconW = 0;
    int mRowW = 0;
};

// The side panel's message box under the event icons: the short version
// of the latest message; the arrows step through older ones and a click
// opens the full message (Zeus manual, "Messages button and Condensed
// Message Box").
class eCondensedMessages : public eWidget {
public:
    using eWidget::eWidget;

    using eOpenAction = std::function<void(int)>;
    void initialize(eGameBoard* const board, const eOpenAction& open);
    // the number of messages, drawn on the Messages button as the
    // original does when there are any (zeus.exe 0x00517CD0)
    void setCountLabel(eLabel* const l, eWidget* const button);
protected:
    void paintEvent(ePainter& p) override;
private:
    void refresh();
    eLabel* mCountLabel = nullptr;
    eWidget* mCountButton = nullptr;

    eGameBoard* mBoard = nullptr;
    eOpenAction mOpen;
    eButton* mText = nullptr;
    int mIndex = 0;
    int mRevision = -1;
    int mCount = 0;
    std::vector<int> mPanel; // history indices shown in the box
};

#endif // EMESSAGESWIDGET_H

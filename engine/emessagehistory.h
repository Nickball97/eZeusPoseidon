#ifndef EMESSAGEHISTORY_H
#define EMESSAGEHISTORY_H

#include <string>
#include <deque>

#include "edate.h"

// One message the player received, with names and numbers already filled
// in, so it can be read again later and saved with the city.
struct eMessageRecord {
    int fId = 0;
    eDate fDate;
    std::string fTitle;
    std::string fText;
    std::string fCondensed; // the short version shown in the side panel
    std::string fCityName; // the "To" line's city, when there is one
    bool fRead = false;
};

// The city's message history, as in the original (zeus.exe 0x00521A70,
// 0x005221B0): newest first, at most 1000; once full, new messages aren't
// kept. Opening a message marks it read; messages can be deleted one by
// one or all read ones at once (Messages window, text group 63).
class eMessageHistory {
public:
    static const int sMax = 1000;

    // returns the record's id, or -1 when the history is full
    int add(eMessageRecord r);
    void remove(const int index);
    void removeRead();
    void markRead(const int id);

    int count() const { return mRecords.size(); }
    const eMessageRecord& at(const int index) const { return mRecords[index]; }
    void setRead(const int index) { mRecords[index].fRead = true; mRevision++; }

    // grows whenever the history changes, for widgets that show it
    int revision() const { return mRevision; }

    void read(eReadStream& src);
    void write(eWriteStream& dst) const;
private:
    std::deque<eMessageRecord> mRecords;
    int mNextId = 1;
    int mRevision = 0;
};

#endif // EMESSAGEHISTORY_H

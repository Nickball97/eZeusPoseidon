#include "emessagehistory.h"

#include <algorithm>

#include "fileIO/efileformat.h"

int eMessageHistory::add(eMessageRecord r) {
    if(count() >= sMax) return -1;
    r.fId = mNextId++;
    mRecords.push_front(r);
    mRevision++;
    return r.fId;
}

void eMessageHistory::remove(const int index) {
    if(index < 0 || index >= count()) return;
    mRecords.erase(mRecords.begin() + index);
    mRevision++;
}

void eMessageHistory::removeRead() {
    const auto it = std::remove_if(mRecords.begin(), mRecords.end(),
                                   [](const eMessageRecord& r) {
        return r.fRead;
    });
    mRecords.erase(it, mRecords.end());
    mRevision++;
}

void eMessageHistory::markRead(const int id) {
    for(auto& r : mRecords) {
        if(r.fId != id) continue;
        r.fRead = true;
        mRevision++;
        return;
    }
}

void eMessageHistory::read(eReadStream& src) {
    mRecords.clear();
    src >> mNextId;
    int n;
    src >> n;
    for(int i = 0; i < n; i++) {
        auto& r = mRecords.emplace_back();
        src >> r.fId;
        r.fDate.read(src);
        src >> r.fTitle;
        src >> r.fText;
        src >> r.fCondensed;
        src >> r.fCityName;
        src >> r.fRead;
        if(src.formatVersion() >= eFileFormat::messagePanel) {
            src >> r.fPanel;
        }
    }
    mRevision++;
}

void eMessageHistory::write(eWriteStream& dst) const {
    dst << mNextId;
    dst << static_cast<int>(mRecords.size());
    for(const auto& r : mRecords) {
        dst << r.fId;
        r.fDate.write(dst);
        dst << r.fTitle;
        dst << r.fText;
        dst << r.fCondensed;
        dst << r.fCityName;
        dst << r.fRead;
        dst << r.fPanel;
    }
}

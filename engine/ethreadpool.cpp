#include "ethreadpool.h"

#include <chrono>
#include <algorithm>

#include "evectorhelpers.h"
#include "engine/egameboard.h"

bool eThreadPool::sSynchronous = false;

eThreadPool::eThreadPool(eGameBoard& board) :
    mBoard(board) {}

eThreadPool::~eThreadPool() {
    {
        mQuit = true;
        for(const auto d : mThreadData) {
            { std::unique_lock<std::mutex> lock(d->fTasksMutex); }
            d->fCv.notify_all();
        }
    }
    for(auto& thread : mThreads) {
        thread.join();
    }
}

void eThreadPool::initialize(const int w, const int h) {
    if(sSynchronous) {
        // one copy of the board, no worker threads (see queueTask)
        if(mThreadData.empty()) mThreadData.push_back(new eThreadData);
        mThreadData[0]->initialize(w, h);
        return;
    }
    if(mThreads.empty()) {
        const int hc = std::thread::hardware_concurrency();
        const int threads = std::clamp(hc, 2, 4);
        for(int i = 0; i < threads; i++) {
            const auto b = new eThreadData;
            b->initialize(w, h);
            mThreadData.push_back(b);
            mThreads.emplace_back(std::bind(&eThreadPool::threadEntry, this, b));
        }
    } else {
        for(auto& t : mThreadData) {
            t->initialize(w, h);
        }
    }
}

void eThreadPool::addBoard(const eCityId cid) {
    waitFinished();
    for(auto& t : mThreadData) {
        t->addBoard(cid);
    }
}

void eThreadPool::removeBoard(const eCityId cid) {
    waitFinished();
    for(auto& t : mThreadData) {
        t->removeBoard(cid);
    }
}

//double gThreadedTime = 0.;

void eThreadPool::threadEntry(eThreadData* data) {
    eTask* task = nullptr;
    while(!mQuit) {
        {
            std::unique_lock<std::mutex> lock(data->fTasksMutex);

            while(!mQuit && data->fTasks.empty()) {
                data->fCv.wait(lock);
            }

            if(data->fTasks.empty()) return;

            data->fBusy = true;
            task = data->fTasks.front();
            data->fTasks.pop();
        }
        if(task) {
            {
//                using std::chrono::high_resolution_clock;
//                using std::chrono::duration_cast;
//                using std::chrono::duration;
//                using std::chrono::milliseconds;

//                const auto t1 = high_resolution_clock::now();

                const auto cid = task->cid();
                data->updateBoard(cid);
//                std::printf("Run task %p\n", task);
                auto& b = data->board(cid);
//                if(task->expectedState() > b.state()) {
//                    printf("Old state!\n");
//                }
                task->run(b);

//                const auto t2 = high_resolution_clock::now();

//                const duration<double, std::milli> ms = t2 - t1;
//                gThreadedTime += ms.count();
//                printf("gThreadedTime %f ms\n", gThreadedTime);
//                printf("run task: %f ms\n", ms.count());
            }
            {
                std::lock_guard lock(mFinishedTasksMutex);
                mFinishedTasks.push_back(task);
            }
            {
                std::unique_lock<std::mutex> lock(data->fTasksMutex);
                data->fBusy = false;
            }
//            std::printf("Task finished %p in %p\n", task, data);
            data->fCvFinished.notify_one();
        }
    }
}

//double gMainThreadTime = 0.;

void eThreadPool::queueTask(eTask* const task) {
    task->setExpectedState(mBoard.state());
    const auto cid = task->cid();
    if(sSynchronous) {
        // run it now, on the board as it is at this moment; its result is
        // still delivered by the next handleFinished(), as with threads
        const auto d = mThreadData[0];
        d->update(mBoard, cid, task->relevance());
        d->updateBoard(cid);
        task->run(d->board(cid));
        mFinishedTasks.push_back(task);
        return;
    }
//    if(cid == eCityId::neutralAggresive ||
//       cid == eCityId::neutralFriendly) {
//        printf("Wrong city id\n");
//    }
    const int threadId = mTaskId++ % mThreadData.size();
    const auto d = mThreadData[threadId];
//    std::printf("Que task %p in %p\n", task, d);

//    using std::chrono::high_resolution_clock;
//    using std::chrono::duration_cast;
//    using std::chrono::duration;
//    using std::chrono::milliseconds;

//    const auto t1 = high_resolution_clock::now();
    d->update(mBoard, cid, task->relevance());
//    const auto t2 = high_resolution_clock::now();

//    const duration<double, std::milli> ms = t2 - t1;
//    gMainThreadTime += ms.count();
//    printf("gMainThreadTime %f ms\n", gMainThreadTime);

    std::unique_lock<std::mutex> lock(d->fTasksMutex);
    d->fTasks.emplace(task);
    d->fCv.notify_one();
}

void eThreadPool::handleFinished() {
    std::vector<eTask*> tasks;
    {
        std::lock_guard lock(mFinishedTasksMutex);
        std::swap(tasks, mFinishedTasks);
    }
    for(const auto t : tasks) {
        t->finish();
        delete t;
    }
}

void eThreadPool::scheduleDataUpdate() {
    mBoard.incState();
//    for(const auto d : mThreadData) {
//        for(auto& f : d->fDataUpdateScheduled) {
//            f.second.fV = true;
//        }
//        std::vector<eCityId> taskCid;
//        std::queue<eTask*> tasks;
//        {
//            std::lock_guard lock(d->fTasksMutex);
//            tasks = d->fTasks;
//        }
//        while(!tasks.empty()) {
//            const auto task = tasks.front();
//            const auto cid = task->cid();
//            const bool r = eVectorHelpers::contains(taskCid, cid);
//            if(!r) taskCid.push_back(cid);
//            tasks.pop();
//        }
//        for(auto& f : d->fDataUpdateScheduled) {
//            const auto cid = f.first;
//            const bool r = eVectorHelpers::contains(taskCid, cid);
//            if(r) {
//                d->scheduleUpdate(mBoard, cid);
//            } else {
//                f.second.fV = true;
//            }
//        }
//    }
}

bool eThreadPool::finished() {
    for(const auto d : mThreadData) {
        std::lock_guard lock(d->fTasksMutex);
        const bool f = d->fTasks.empty() && !d->fBusy;
        if(!f) return false;
    }
    return true;
}

void eThreadPool::waitFinished() {
    for(const auto d : mThreadData) {
        std::unique_lock<std::mutex> lock(d->fTasksMutex);
        d->fCvFinished.wait(lock, [d]() {
            return d->fTasks.empty() && !d->fBusy;
        });
    }
}

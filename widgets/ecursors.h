#ifndef ECURSORS_H
#define ECURSORS_H

// The original's mouse pointers, read at run time from the player's own
// zeus.exe (Windows cursor resources; nothing is copied into eZeus).
// zeus.exe 0x0050B980 loads them, 0x0050B6F0 picks one: 0x66 the normal
// arrow, 0x82 the shovel while clearing land (build mode 0x1D, chosen in
// 0x00513580). Where the file or a cursor is missing, SDL's own arrow stays.
namespace eCursors {
    void load();

    enum class eCursor {
        arrow,
        shovel
    };
    // changes the pointer only when it is a different one
    void set(const eCursor c);
}

#endif // ECURSORS_H

#ifndef EGAMECHOOSER_H
#define EGAMECHOOSER_H

#include "emainmenubase.h"

// The first screen, as in the original: the Zeus and Poseidon panels with
// their logos and "Play Zeus" / "Play Poseidon" under them, then the
// Adventure Editor, Options (eZeus's display settings) and Quit.
class eGameChooser : public eMainMenuBase {
public:
    using eMainMenuBase::eMainMenuBase;

    void initialize(const eAction& playZeusA,
                    const eAction& playPoseidonA,
                    const eAction& editorA,
                    const eAction& optionsA,
                    const eAction& quitA);
};

#endif // EGAMECHOOSER_H

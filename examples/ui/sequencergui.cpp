#include "al/core.hpp"

#include "al/util/ui/al_PresetSequencer.hpp"
#include "al/util/ui/al_Parameter.hpp"
#include "al/util/ui/al_Preset.hpp"
#include "al/util/ui/al_ControlGUI.hpp"

#include <fstream>

using namespace al;
using namespace std;


struct MyApp : App
{
    Mesh m;

    Parameter X{"x", "", 0.0, "", -2, 2};
    Parameter Y{"y", "", 0.0, "", -2, 2};

    PresetHandler presetHandler{"sequencerDir", true};
    PresetSequencer sequencer;

    ControlGUI gui;

    void onCreate() override {
        addSphere(m, 0.2);
        nav().pullBack(4);

        navControl().disable();

        presetHandler << X << Y; // Register parameters with preset handler
        sequencer << presetHandler; // Register preset handler with sequencer
        gui << sequencer;
        gui.init();
    }

    void onDraw(Graphics &g) override {
        g.clear(0);
        if (sequencer.running()) {
            g.translate(X.get(), Y.get(), 0);
            g.color(0.0,1.0,0.0);
        } else {
            g.color(0.0,0.0,1.0);
        }
        g.draw(m);
        gui.draw(g);
    }
};

void writeExamplePresets()
{
    string sequence = R"(preset1:0.0:0.5
preset2:3.0:1.0
preset3:1.0:0.0
preset2:1.5:2.0
::
)";
    ofstream fseq("sequencerDir/seq.sequence");
    fseq << sequence;
    fseq.close();

    string preset1 = R"(::preset1
/x f 0.4
/y f 0.2
::
)";
    ofstream f1("sequencerDir/preset1.preset");
    f1 << preset1;
    f1.close();

    string preset2 = R"(::preset2
/x f 0.6
/y f -0.9
::
)";
    ofstream f2("sequencerDir/preset2.preset");
    f2 << preset2;
    f2.close();

    string preset3 = R"(::preset3
/x f -0.1
/y f 1.0
::
)";
    ofstream f3("sequencerDir/preset3.preset");
    f3 << preset3;
    f3.close();

}

int main(int argc, char* argv[])
{
    writeExamplePresets();
    MyApp().start();
}


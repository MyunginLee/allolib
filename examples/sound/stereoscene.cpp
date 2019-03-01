// AudioScene example using the Stereo panner class
// By Andrés Cabrera mantaraya36@gmail.com
// March 7 2017 updated March 2018

#include <iostream>

#include "al/core.hpp"
#include "al/core/sound/al_AudioScene.hpp"
#include "al/core/sound/al_StereoPanner.hpp"

#define BLOCK_SIZE 256
#define SAMPLE_RATE 44100

using namespace al;

#include "Gamma/Oscillator.h"
using namespace gam;

// Create an agent (graphic marker + sound source)
class MyAgent : public SoundSource, Nav {
 public:
  SineD<> osc;
  Accum<> tmr;
  MyAgent() {
    tmr.finish();
    tmr.period(1);
    osc.set(550.0, 0.3, 1.0);
    // The agent is displayed as a tetrahedron
    addTetrahedron(m, 0.2);
    m.generateNormals();
  }

  // the onProcess function will be used in the audio callback
  // to update the agent's sound buffer
  void onProcess(AudioIOData &io) {
    // The agent's sound will be a repeating burst of noise
    while (io()) {
      if (tmr()) {
        osc.set(550.0, 0.3, 1.0);
      }
      writeSample(osc());  // writeSample() sets the agent's sound
    }
  }

  // the onUpdateNav() function will be called whenever the listener
  // navigation changes. This will adjust the spatial position of
  // the sound source for AudioScene
  void onUpdateNav() { SoundSource::pose(*this); }

  // The onDraw() function will be called in the graphics render callback
  // We only need to specify what the agent should draw. The graphics
  // scene takes care of positioning and rotation
  void onDraw(Graphics &g) {
    g.pushMatrix();
    g.draw(m);
    g.popMatrix();
  }

 private:
  float mEnvelope{0.3};
  Mesh m;
};

class MyApp : public App {
  AudioScene scene{BLOCK_SIZE};
  // To use an AudioScene, we need a Listener, a Panner and a SpeakerLayout
  Listener *listener;
  StereoPanner *panner;
  SpeakerLayout speakerLayout;

  double mX, mY;
  Light mLight;
  MyAgent agent;

 public:
  void onCreate() override {
    // Create a stereo speaker layouy
    speakerLayout = StereoSpeakerLayout();
    // Use a simple stereo panning algorithm
    panner = new StereoPanner(speakerLayout);
    // Create a listener for the AudioScene that uses stereo panning
    listener = scene.createListener(panner);
    // Add our agent to the AudioScene
    scene.addSource(agent);

    nav().pos() -= {0, 0, -1};
    Sync::master().spu(audioIO().fps());
  }

  void onSound(AudioIOData &io) override {
    // Update the lister position (once per audio block)
    listener->pose(nav());
    // Update the agent's position to let it know how audio should be rendered
    agent.onUpdateNav();
    // Update agent audio buffer
    agent.onProcess(io);
    // Now that agent information has been updated we render the audio scene
    scene.render(io);
  }

  void onDraw(Graphics &g) override {
    g.clear(0);
    g.lighting(true);
    g.light(mLight);
    g.color(1);

    agent.onDraw(g);  // Draw the agent in the scene
  }
};

int main(int argc, char *argv[]) {
  MyApp app;
  app.dimensions(640, 480);
  app.title("Stereo Audio Scene");
  app.fps(30);
  app.initAudio(SAMPLE_RATE, BLOCK_SIZE, 2, 0);
  app.start();
  return 0;
}

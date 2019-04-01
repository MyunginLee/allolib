#ifndef AL_PRESET_H
#define AL_PRESET_H

/*	Allocore --
	Multimedia / virtual environment application class library

	Copyright (C) 2009. AlloSphere Research Group, Media Arts & Technology, UCSB.
	Copyright (C) 2016. The Regents of the University of California.
	All rights reserved.

	Redistribution and use in source and binary forms, with or without
	modification, are permitted provided that the following conditions are met:

		Redistributions of source code must retain the above copyright notice,
		this list of conditions and the following disclaimer.

		Redistributions in binary form must reproduce the above copyright
		notice, this list of conditions and the following disclaimer in the
		documentation and/or other materials provided with the distribution.

		Neither the name of the University of California nor the names of its
		contributors may be used to endorse or promote products derived from
		this software without specific prior written permission.

	THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
	AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
	IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
	ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
	LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
	CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
	SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
	INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
	CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
	ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
	POSSIBILITY OF SUCH DAMAGE.

	File description:
	Preset classes that encapsulates storing values for groups of parameters
	File author(s):
	Andrés Cabrera mantaraya36@gmail.com
*/

#include <string>
#include <vector>
#include <mutex>
#include <map>
#include <thread>
#include <atomic>
#include <condition_variable>
#include <functional>
#include <algorithm>

#include "al/core/protocol/al_OSC.hpp"
#include "al/util/ui/al_Parameter.hpp"
#include "al/util/ui/al_ParameterServer.hpp"
#include "al/core/system/al_Time.hpp"

namespace  al
{

/**
 * @brief The PresetHandler class handles sorting and recalling of presets.
 *
 * Presets are saved by name with the ".preset" suffix.
 */
class PresetHandler
{
public:

    typedef std::map<std::string, std::vector<float> > ParameterStates;
	/**
	 * @brief PresetHandler contructor
	 *
	 * @param rootDirectory sets the root directory for preset and preset map storage
	 * @param verbose if true, print diagnostic messages
	 *
	 */
	PresetHandler(std::string rootDirectory = "presets", bool verbose = false);

	~PresetHandler();

	/**
	 * @brief Stores preset
	 * @param name the name of the preset
	 *
	 * Since all presets are assinged an index, calling this function will give
	 * the preset a free index and then call storePreset(index, name). If the
	 * preset name already exists, it will overwrite the existing preset without
	 * assigning a new index.
	 */
	void storePreset(std::string name);

	/**
	 * @brief Store preset at index. The name argument specifies the preset name
	 *
	 * @param index
	 * @param name
	 * @param overwrite if false, and preset file exists it is not overwritten and a number is appended
	 *
	 * The preset name also determines the filename under which the preset is saved, so
	 * it must be unique. If name is empty, a unique name is generated. If name
	 * exists, a number is appended to the preset name. The link between preset
	 * index and preset name is store within the preset map file that is
	 * stored in the path for the PresetHandler, see getCurrentPath()
	 *
	 */
	void storePreset(int index, std::string name = "", bool overwrite = true);
	/**
	 * @brief Recall a preset by name
	 * @param name
	 *
	 * The preset should be a file on disk in the PresetHandler's root
	 * directory and should have the ".preset" extension. See also
	 * setSubDirectory().
	 */
	void recallPreset(std::string name);

	/**
	 * @brief Recall a preset by index number
	 * @param index
	 * @return the name of the preset corresponding to index. Empty if index not valid.
	 *
	 * The preset map file (by default called default.presetMap)
	 * is used to map preset names on disk to indeces. You can
	 * set alternative preset map files using setCurrentPresetMap().
	 * See also PresetMapper for handling and archiving preset maps.
	 */
	std::string recallPreset(int index);

	void recallPresetSynchronous(std::string name);

	/**
	 * @brief recall immediately (not using the morph thread)
	 * @param index
	 */
	std::string recallPresetSynchronous(int index);

	/**
	 * @brief Set parameters to values interpolated between two presets
	 * @param index1 index of the first preset
	 * @param index2 index of the second preset
	 * @param factor A value between 0-1 to determine interpolation
     * @param synchronous The values are set instantly and synchronous to this call
	 *
	 * A factor of 0 uses preset 1 and a factor of 1 uses preset 2. Values
	 * in between result in linear interpolation of the values.
	 */
	void setInterpolatedPreset(int index1, int index2, double factor, bool synchronous = true);

    void setInterpolatedPreset(std::string presetName1, std::string presetName2, double factor, bool synchronous = true);

    /** 
     * @brief Interpolate between current values and new values according to factor
    */
    static void setParameterValues(ParameterMeta *param, std::vector<float> &values, double factor = 1.0);

	void morphTo(ParameterStates &parameterStates, float morphTime);
	void stopMorph();

	std::map<int, std::string> availablePresets();
	std::string getPresetName(int index);
	std::string getCurrentPresetName() {return mCurrentPresetName; }

    /**
     * @brief Add or remove a parameter address from group that will be skipped when recalling presets
     * @param parameter address of parameter to skip
     * @param skip set to false if you want to remove from skip list
     *
     * This is used within loadPresetValues(), so it will affect both synchronous and
     * asynchronous recall.
     */
    void skipParameter(std::string parameterAddr, bool skip = true);

    int getCurrentPresetIndex();

	float getMorphTime();
	void setMorphTime(float time);

	void setSubDirectory(std::string directory);
	std::string getSubDirectory() {return mSubDir;}

	/// Path including subdirectory if any
	std::string getCurrentPath();

    void setRootPath(std::string path);

	/// Base path without appending sub directory
	std::string getRootPath();

	/** Display information about preset handler, including path and
	 * registered parameters.
	 */
	void print();

	/**
	 * @brief Register a callback to be notified when a preset is loaded
	 * @param cb The callback function
	 * @param userData data to be passed to the callback
	 */
	void registerPresetCallback(std::function<void(int index, void *sender, void *userData)> cb,
	                            void *userData = nullptr);

	/**
	 * @brief Register a callback to be notified when a preset is stored
	 * @param cb The callback function
	 * @param userData data to be passed to the callback
	 */
	void registerStoreCallback(std::function<void(int index, std::string name, void *userData)> cb,
	                           void *userData = nullptr);

	/**
	 * @brief Register a callback to be notified when morph time parameter is changed
	 * @param cb The callback function
	 */
    void registerMorphTimeCallback(Parameter::ParameterChangeCallback cb);


    typedef const std::function<void (std::string)> PresetMapCallback;
    /**
	 * @brief Register a callback to be notified when preset map cahges
	 * @param cb The callback function
	 */
    void registerPresetMapCallback(PresetMapCallback cb);

    PresetHandler &registerParameter(ParameterMeta &parameter);

    PresetHandler &operator << (ParameterMeta &param) { return this->registerParameter(param); }

    PresetHandler &registerParameterBundle(ParameterBundle &bundle);

    PresetHandler &operator << (ParameterBundle &bundle) { return this->registerParameterBundle(bundle); }

    std::vector<ParameterMeta *> parameters() { return  mParameters;}

	std::string buildMapPath(std::string mapName, bool useSubDirectory = false);

    std::vector<std::string> availablePresetMaps();

	std::map<int, std::string> readPresetMap(std::string mapName = "default");

	void setCurrentPresetMap(std::string mapName = "default", bool autoCreate = false);

    void setPresetMap(std::map<int, std::string> presetsMap) {
        mPresetsMap = presetsMap;
    }

    void storeCurrentPresetMap(std::string mapName = "", bool useSubDirectory = false);

	/**
	 * @brief useCallbacks determines whether to call the internal callbacks
	 * @param use
	 *
	 * The callbacks set by registerStoreCallback() and registerPresetCallback()
	 * are only called if this is set to true. The value is true by default.
	 */
	void useCallbacks(bool use) { mUseCallbacks = use; }

	void changeParameterValue(std::string presetName, std::string parameterPath,
	                          float newValue);

    static int asciiToPresetIndex(int ascii, int offset = 0) {
        int index = -1;

        switch (ascii) {
        case '1': index = 0; break;
        case '2': index = 1; break;
        case '3': index = 2; break;
        case '4': index = 3; break;
        case '5': index = 4; break;
        case '6': index = 5; break;
        case '7': index = 6; break;
        case '8': index = 7; break;
        case '9': index = 8; break;
        case '0': index = 9; break;
        case 'q': index = 10; break;
        case 'w': index = 11; break;
        case 'e': index = 12; break;
        case 'r': index = 13; break;
        case 't': index = 14; break;
        case 'y': index = 15; break;
        case 'u': index = 16; break;
        case 'i': index = 17; break;
        case 'o': index = 18; break;
        case 'p': index = 19; break;
        case 'a': index = 20; break;
        case 's': index = 21; break;
        case 'd': index = 22; break;
        case 'f': index = 23; break;
        case 'g': index = 24; break;
        case 'h': index = 25; break;
        case 'j': index = 26; break;
        case 'k': index = 27; break;
        case 'l': index = 28; break;
        case ';': index = 29; break;;
        case 'z': index = 30; break;
        case 'x': index = 31; break;
        case 'c': index = 32; break;
        case 'v': index = 33; break;
        case 'b': index = 34; break;
        case 'n': index = 35; break;
        case 'm': index = 36; break;
        case ',': index = 37; break;
        case '.': index = 38; break;
        case '/': index = 39; break;
        }
        if (index >= 0) {
            index += offset;
        }

        return index;
    }

    void verbose(bool isVerbose) { mVerbose = isVerbose; }
    bool verbose() {return mVerbose;}

    /**
     * @brief load preset into parameter states data structure without setting values
     * @param name name of the preset to load
     * @return the state of the parameters in the loaded prese
     */
    ParameterStates loadPresetValues(std::string name);

    /**
     * @brief save list of parameter states into text preset file
     * @param values the values of parameters to store
     * @param presetName name of preset to store
     * @param overwrite true overwrites otherwise append unique number
     * @return true if no errors.
     */
	bool savePresetValues(const ParameterStates &values, std::string presetName,
	                       bool overwrite = true);
private:

    std::vector<float> getParameterValue(ParameterMeta *p);
    void setParametersInBundle(ParameterBundle *bundle, std::string bundlePrefix, PresetHandler *handler, float factor = 1.0);
	static void morphingFunction(PresetHandler *handler);

    ParameterStates getBundleStates(ParameterBundle *bundle, std::string id);

    bool mVerbose {false};
	bool mUseCallbacks {false};
	std::string mRootDir;
	std::string mSubDir; // Optional sub directory, e.g. for preset map archives
	std::string mFileName;
	std::string mCurrentMapName;
	std::vector<ParameterMeta *> mParameters;
    std::vector<std::string> mSkipParameters;
    std::mutex mSkipParametersLock;

    std::map<std::string, std::vector<ParameterBundle *>> mBundles;

	std::mutex mFileLock;
	bool mRunning {false}; // To keep the morphing thread alive
	// bool mMorph; // To be able to trip and stop morphing at any time.
	std::atomic<int> mMorphRemainingSteps;
	float mMorphInterval;
	Parameter mMorphTime;

	// std::mutex mMorphLock;
	std::mutex mTargetLock;
	std::condition_variable mMorphConditionVar;
    ParameterStates mTargetValues;

	std::thread mMorphingThread;

	std::vector<std::function<void(int index, void *sender, void *userData)>> mCallbacks;
	std::vector<void *> mCallbackUdata;
	std::vector<std::function<void(int index, std::string name, void *userData)>> mStoreCallbacks;
	std::vector<void *> mStoreCallbackUdata;
    std::vector<std::function<void (std::string)>> mPresetsMapCbs;

	std::map<int, std::string> mPresetsMap;
	std::string mCurrentPresetName;
};

class PresetServer : public osc::PacketHandler, public OSCNotifier
{
public:
	/**
	 * @brief PresetServer constructor
	 *
	 * @param oscAddress The network address on which to listen to. If empty use all available network interfaces. Defaults to "127.0.0.1".
	 * @param oscPort The network port on which to listen. Defaults to 9011.
	 *
	 */

	PresetServer(std::string oscAddress = "127.0.0.1",
	             int oscPort = 9011);
	/**
	 * @brief using this constructor reuses the existing osc::Recv server from the
	 * ParameterServer object
	 * @param paramServer an existing ParameterServer object
	 *
	 * You will want to reuse an osc::Recv server when you want to expose the
	 * interface thorugh the same network port. Since network ports are exclusive,
	 * once a port is bound, it can't be used. You might need to expose the
	 * parameters on the same network port when using things like interface.simpleserver.js
	 * That must connect all interfaces to the same network port.
	 */
	PresetServer(ParameterServer &paramServer);
	~PresetServer();

	/**
	 * @brief print prints information about the server to std::out
	 */
	void print();

	/**
	 * @brief stopServer stops the OSC server thread. Calling this function
	 * is sometimes required when this object is destroyed abruptly and the
	 * destructor is not called
	 */
	void stopServer();

	bool serverRunning();

	void allowStore(bool allow) {mAllowStore = allow;}
	bool allowStore() {return mAllowStore;}

	virtual void onMessage(osc::Message& m);

	PresetServer &registerPresetHandler(PresetHandler &presetHandler) {
		mPresetHandlers.push_back(&presetHandler);
		presetHandler.registerPresetCallback(PresetServer::changeCallback,
		                                       (void *) this);

		presetHandler.registerMorphTimeCallback([&](float value){
			this->notifyListeners(this->mOSCpath + "/morphTime", value);
		});
		return *this;
	}

	PresetServer &operator <<(PresetHandler &presetHandler) {return registerPresetHandler(presetHandler);}

	void setAddress(std::string address);
	std::string getAddress();

	void notifyPresetChange(bool notify) {mNotifyPresetChange = notify;}

	void attachPacketHandler(osc::PacketHandler *handler);

protected:
	static void changeCallback(int value, void *sender, void *userData);

private:
	osc::Recv *mServer;
	std::vector<PresetHandler *> mPresetHandlers;
//	std::mutex mServerLock;
	std::string mOSCpath;
	std::mutex mHandlerLock;
	std::vector<osc::PacketHandler *> mHandlers;
	bool mAllowStore;
	bool mStoreMode;
	bool mNotifyPresetChange;

	std::mutex mPresetChangeLock;
	std::string mPresetChangeSenderAddr;

	std::vector<std::string> mDisabledListeners;

	ParameterServer *mParameterServer;
};


}

#endif // AL_PRESET_H

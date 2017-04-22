//
// Created by abc on 2017/4/21.
//

#include <jni.h>
#include "IAgoraMediaEngine.h"
#include "IAgoraRtcEngine.h"

class AgoraAudioFrameObserver : public agora::media::IAudioFrameObserver {
public:
    bool onRecordAudioFrame(AudioFrame &audioFrame);

    bool onPlaybackAudioFrame(AudioFrame &audioFrame);

    bool onPlaybackAudioFrameBeforeMixing(unsigned int uid, AudioFrame &audioFrame);

    void setJObserver(JNIEnv *env, jobject jobj);

private:
    jobject jAgoraAudioFrameObserverImpl;
    JNIEnv *envParams;
};
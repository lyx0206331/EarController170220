//
// Created by abc on 2017/4/21.
//

#include "AgoraAudioFrameObserver.h"

using namespace agora::media;

bool AgoraAudioFrameObserver::onRecordAudioFrame(IAudioFrameObserver::AudioFrame &audioFrame) {
    /*
    *从c层audioFrame拿出数据传递给java层处理
    *1、生成java层数据对象 jdata.  //jdata 对应java类型 AgoraAudioFrame.java
    *2、把数据赋值给java对象 jdata.setData(audioFrame);
    *3、java接口回调 jAgoraAudioFrameObserverImpl->method（??? jdata）；//对应java类型IAudioFrameListener.java 回调方法:void onRecordAudioFrame(AgoraAudioFrame var1);
    *4、从jdata获取java层处理后的数据，并赋值给audioFrame
    *
    *
    */
    jclass jclazz = (*envParams)->FindClass(envParams, "io/iwhere/AgoraAudioFrame");   //查找Java层的类
    if (jclazz == NULL) {
        return;
    }
    jmethodID mid = (*envParams)->GetMethodID(envParams, jclazz, "<init>", "()V"); //构造CwjThread类
    if (mid == NULL) {
        return;
    }

    jobject obj = (*envParams)->NewObject(envParams, jclazz, mid, NULL);

    if (obj == NULL) {
        return;
    }
    jclass iAudioFrameListenerClazz = (*envParams)->FindClass(envParams,
                                                              "io/iwhere/IAudioFrameListener");
    if (iAudioFrameListenerClazz == NULL) {
        return;
    }
    jmethodID onRecordAudioFrameM = (*envParams)->GetMethodID(envParams, iAudioFrameListenerClazz,
                                                              "onRecordAudioFrame", "()V");
    if (onRecordAudioFrameM == NULL) {
        return;
    }
    /**给obj java的frame赋值 开始**/
    jclass jobjCls = envParams->GetObjectClass(obj);
    if (jobjCls) {
        jfieldID samplesId = envParams->GetFieldID(jobjCls, "samples", "I");
//        jint samples = (int)envParams->GetIntField(obj, samplesId);
        jfieldID bytesPerSampleId = envParams->GetFieldID(jobjCls, "bytesPerSample", "I");
//        jint bytesPerSample = (int)envParams->GetIntField(obj, bytesPerSampleId);
        jfieldID channelsId = envParams->GetFieldID(jobjCls, "channels", "I");
//        jint channels = (int)envParams->GetIntField(obj, channelsId);
        jfieldID samplesPerSecId = envParams->GetFieldID(jobjCls, "samplesPerSec", "I");
//        jint samplesPerSec = (int)envParams->GetIntField(obj, samplesPerSecId);
        jfieldID bufferId = envParams->GetFieldID(jobjCls, "buffer", "[B");
//        jbyteArray buffer = (jbyteArray)envParams->GetObjectField(obj, bufferId);

        envParams->SetIntField(obj, samplesId, audioFrame.samples);
        envParams->SetIntField(obj, bytesPerSampleId, audioFrame.bytesPerSample);
        envParams->SetIntField(obj, channelsId, audioFrame.channels);
        envParams->SetIntField(obj, samplesPerSecId, audioFrame.samplesPerSec);
        envParams->SetObjectField(obj, bufferId, audioFrame.buffer);
    }
    /**给obj java的frame赋值 结束**/
    (*envParams)->CallVoidMethod(envParams, jAgoraAudioFrameObserverImpl, onRecordAudioFrameM,
                                 obj);//调用java接口

    /**从obj java的frame取值赋予AudioFrame 开始**/

    /**从obj java的frame取值赋予AudioFrame 结束**/

    return true;
}


bool AgoraAudioFrameObserver::onPlaybackAudioFrame(IAudioFrameObserver::AudioFrame &audioFrame) {
    return true;
}

bool AgoraAudioFrameObserver::onPlaybackAudioFrameBeforeMixing(unsigned int uid,
                                                               IAudioFrameObserver::AudioFrame &audioFrame) {
    return true;
}

void AgoraAudioFrameObserver::setJObserver(JNIEnv *env, jobject jobj) {//设置java层接口对象
    jAgoraAudioFrameObserverImpl = jobj;
    envParams = env;
}

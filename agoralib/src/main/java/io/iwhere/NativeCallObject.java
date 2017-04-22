package io.iwhere;

/**
 * Created by abc on 2017/4/18.
 */
public class NativeCallObject {

    public NativeCallObject() {

    }

    public native void registerAudioFrameObserver(IAudioFrameListener var1);

    public native void unregisterAudioFrameObserver();

    public native AgoraAudioFrame getAudioFrame();

    public native void setData(AgoraAudioFrame frame);

}

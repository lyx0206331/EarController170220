package io.iwhere;

/**
 * Created by abc on 2017/4/18.
 */
public class AgoraAudioFrame {
    public int samples;
    public int bytesPerSample;
    public int channels;
    public int samplesPerSec;
    public byte[] buffer;

    public AgoraAudioFrame() {
    }

    public byte[] getBuffer() {
        return this.buffer;
    }
}

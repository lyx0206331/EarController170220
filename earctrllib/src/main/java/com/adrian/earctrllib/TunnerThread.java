package com.adrian.earctrllib;

import android.media.AudioFormat;
import android.media.AudioRecord;
import android.media.MediaRecorder;
import android.util.Log;
import android.view.KeyEvent;

/**
 * 通过调用FFT方法来实时计算输入音频的频率
 */
public class TunnerThread extends Thread {

    static {
        System.loadLibrary("FFT");
    }

    public native int[] processSampleData(byte[] sample, int sampleLength);

    private static final int[] OPT_SAMPLE_RATES = { /*11025, 8000, 22050,*/ 32000}; //8000, 44100
    private static final int[] BUFFERSIZE_PER_SAMPLE_RATE = { /*8 * 1024,
            4 * 1024, 16 * 1024,*/ 512};    //512, 4*1024

    private int SAMPLE_RATE = 8000;
    private int READ_BUFFERSIZE = 4 * 1024;
    private int[] currentFrequency;

    private AudioRecord audioRecord;

    private boolean isListening = false;

    private ICurFreqListener listener;

    public TunnerThread(ICurFreqListener listener) {
        this.listener = listener;
        initAudioRecord();
    }

    // 每个device的初始化参数可能不同
    private void initAudioRecord() {
        int counter = 0;
        for (int sampleRate : OPT_SAMPLE_RATES) {
            initAudioRecord(sampleRate);
            if (audioRecord.getState() == AudioRecord.STATE_INITIALIZED) {
                SAMPLE_RATE = sampleRate;
                READ_BUFFERSIZE = BUFFERSIZE_PER_SAMPLE_RATE[counter];
//				READ_BUFFERSIZE = audioRecord.getMinBufferSize(sampleRate, AudioFormat.CHANNEL_CONFIGURATION_MONO, AudioFormat.ENCODING_PCM_16BIT);
                //Log.e("SIZE", READ_BUFFERSIZE + "");
                break;
            }
            counter++;
        }
        isListening = true;
    }

    @SuppressWarnings("deprecation")
    private void initAudioRecord(int sampleRate) {
        audioRecord = new AudioRecord(MediaRecorder.AudioSource.MIC,
                sampleRate, AudioFormat.CHANNEL_CONFIGURATION_MONO,
                AudioFormat.ENCODING_PCM_16BIT, sampleRate);
    }

    public int getSampleRate() {
        return SAMPLE_RATE;
    }

    @Override
    public void run() {
        try {
            audioRecord.startRecording();
            byte[] bufferRead = new byte[READ_BUFFERSIZE];
            int count = 0;
            while (isListening && (count = audioRecord.read(bufferRead, 0, READ_BUFFERSIZE)) > 0) {
//            for (int i = 0; i < count; i++) {
//                Log.e("BUFFER", "buffer:" + bufferRead[i]);
//            }
                processBuffer(bufferRead, count);
            }

        } catch (IllegalStateException e) {
            e.printStackTrace();
            if (listener != null) {
                listener.onException(e);
            }
        }
    }

    public void processBuffer(byte[] bufferRead, int count) {
        currentFrequency = processSampleData(bufferRead, count);
//            currentFrequency = 65;    //test data
        if (currentFrequency == null || currentFrequency.length == 0) {
//                Log.e("FRE", "curFre is null");
            return;
        }
        //Log.e("FRE", "curFre Len:" + currentFrequency.length);
        for (int freq :
                currentFrequency) {
            Log.e("FRE", "curFre:" + freq);
            if (listener != null) {
                listener.onCurKey(getKey(freq));
//                    Log.e("FRE", "curFre:" + currentFrequency);
            }
//                try {
//                    if (isListening && audioRecord.getState() == AudioRecord.STATE_INITIALIZED) {
//                        audioRecord.stop();
//                    }
//                    Thread.sleep(1);
//                    if (isListening) {
//                        audioRecord.startRecording();
//                    }
//                } catch (InterruptedException e) {
//                    e.printStackTrace();
//                }
        }
    }

    public void close() {
        if (audioRecord != null
                && audioRecord.getState() == AudioRecord.STATE_INITIALIZED) {
            isListening = false;
            audioRecord.stop();
            audioRecord.release();
            audioRecord = null;
        }
    }

//    public double getCurrentFrequency() {
//        return currentFrequency;
//    }

    private int lastSig = -1;

    /**
     * 根据频率获取键值
     * @param currentFrequency
     * @return
     */
    private FreqKeyInfo getKey(int currentFrequency) {
        FreqKeyInfo info = new FreqKeyInfo();

        int sig = (currentFrequency&0x0F) | (currentFrequency&0xC0);

        if (lastSig != sig) {
            info.keyCode = KeyEvent.KEYCODE_0 + (sig & 0x07);
            info.action = ((sig & 0x08) == 0x08) ? KeyEvent.ACTION_DOWN : KeyEvent.ACTION_UP;
            Log.e("sig", "sig:" + sig + " action:" + info.toString());
        }

        lastSig = sig;

        return info;
    }

    /**
     * 键值响应接口
     */
    public interface ICurFreqListener {
        void onCurKey(FreqKeyInfo keyInfo);
        void onException(Exception e);
    }

    /**
     * 键值信息类
     */
    public class FreqKeyInfo {
        int keyCode = -1;
        int action = -1;

        public FreqKeyInfo() {
        }

        public int getKeyCode() {
            return keyCode;
        }

        public int getAction() {
            return action;
        }

        @Override
        public String toString() {
            return "FreqKeyInfo{" +
                    "keyCode=" + keyCode +
                    ", action=" + action +
                    '}';
        }
    }

}

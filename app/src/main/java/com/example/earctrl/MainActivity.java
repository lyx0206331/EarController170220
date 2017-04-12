package com.example.earctrl;

import android.app.Activity;
import android.content.Intent;
import android.os.Bundle;
import android.view.View;
import android.widget.Button;

/**
 * Created by adrian on 16-12-5.
 */

public class MainActivity extends Activity implements View.OnClickListener {

    private Button mVoiceFreqCtrlBtn;
    private Button mSoundTrackCtrlBtn;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        mVoiceFreqCtrlBtn = (Button) findViewById(R.id.btn_voice_freq_ctrl);
        mSoundTrackCtrlBtn = (Button) findViewById(R.id.btn_sound_track_ctrl);

        mVoiceFreqCtrlBtn.setOnClickListener(this);
        mSoundTrackCtrlBtn.setOnClickListener(this);
    }

    public void onClick(View v) {
        switch (v.getId()) {
            case R.id.btn_voice_freq_ctrl:
                openAct(VoiceFreqCtrlActivity.class);
                break;
            case R.id.btn_sound_track_ctrl:
                openAct(SoundTrackCtrlActivity.class);
                break;
        }
    }

    private void openAct(Class<? extends Activity> act) {
        Intent intent = new Intent(this, act);
        startActivity(intent);
    }
}

package com.example.earctrl;

import android.app.Activity;
import android.os.Bundle;
import android.util.Log;
import android.view.KeyEvent;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.Window;
import android.widget.Button;
import android.widget.TextView;

import com.adrian.earctrllib.TunnerThread;
import com.example.earctrl.Utils.KeepScreenon;

public class VoiceFreqCtrlActivity extends Activity implements TunnerThread.ICurFreqListener {

	private boolean startRecording = true;

	private TunnerThread tunner;

	private Button tunning_button = null;
	private Button mClearTopHalfBtn, mClearBottomHalfBtn;
	private TextView frequencyView = null;
	private TextView frequencyView0;

	@Override
	public void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		KeepScreenon.keepScreenOn(this, true);
		this.requestWindowFeature(Window.FEATURE_NO_TITLE);
		setContentView(R.layout.activity_voice_freq_ctrl);

		tunning_button = (Button) findViewById(R.id.tunning_button);
		frequencyView = (TextView) findViewById(R.id.frequency);
		frequencyView0 = (TextView) findViewById(R.id.frequency0);
		mClearTopHalfBtn = (Button) findViewById(R.id.btn_clear_top_half);
		mClearBottomHalfBtn = (Button) findViewById(R.id.btn_clear_bottom_half);
		tunning_button.setOnClickListener(new OnClickListener() {

			public void onClick(View v) {
				onRecord(startRecording);
				if (startRecording) {
					tunning_button.setText(R.string.stop_tunning);
				} else {
					tunning_button.setText(R.string.start_tunning);
				}
				startRecording = !startRecording;
			}

		});
		mClearTopHalfBtn.setOnClickListener(new OnClickListener() {
			public void onClick(View v) {
				frequencyView0.setText("");
			}
		});
		mClearBottomHalfBtn.setOnClickListener(new OnClickListener() {
			public void onClick(View v) {
				frequencyView.setText("");
			}
		});
	}

	@Override
	protected void onDestroy() {
		super.onDestroy();
		onRecord(false);
	}

	private void onRecord(boolean startRecording) {
		if (startRecording) {
			startTunning();
		} else {
			stopTunning();
		}
	}

	private void startTunning() {
		tunner = new TunnerThread(this);
		tunner.start();
	}
	
	private void stopTunning() {
		if (tunner != null) {
			tunner.close();
		}
	}

	public void onCurKey(final TunnerThread.FreqKeyInfo keyInfo) {
		Log.e("KEY", "key:" + keyInfo.toString());
		runOnUiThread(new Runnable() {
			public void run() {

				if (keyInfo.getKeyCode() >= 0) {
					frequencyView.append(keyInfo.toString());
					switch (keyInfo.getAction()) {
						case KeyEvent.ACTION_UP:
							Log.e("ACTION", "up");
							break;
						case KeyEvent.ACTION_DOWN:
							Log.e("ACTION", "down");
							break;
					}
				}
			}
		});
	}

	public void onException(Exception e) {
		e.printStackTrace();
	}
}

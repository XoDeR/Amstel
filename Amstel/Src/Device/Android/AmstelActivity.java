package Rio.android;

import android.app.NativeActivity;
import android.os.Bundle;

public class RioActivity extends NativeActivity
{
	static
	{
		System.loadLibrary("Rio");
	}

	RioActivity rioActivity;

	@Override
	public void onCreate(Bundle savedInstanceState)
	{
		super.onCreate(savedInstanceState);
		rioActivity = this;
		// Init additional stuff here (ads, etc.)
    }

	@Override
	public void onDestroy()
	{
		// Destroy additional stuff here (ads, etc)
		super.onDestroy();
	}
}

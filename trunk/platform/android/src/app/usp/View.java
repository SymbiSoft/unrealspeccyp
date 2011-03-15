package app.usp;

import java.nio.ByteBuffer;

import android.graphics.Canvas;
import android.graphics.Bitmap;
import android.media.AudioTrack;
import android.media.AudioFormat;
import android.media.AudioManager;
import android.view.SurfaceHolder;
import android.view.SurfaceHolder.Callback;
import android.view.SurfaceView;
import android.content.Context;

public class View extends SurfaceView  implements Callback
{
	protected SurfaceHolder sh;
	protected Bitmap bmp;
	protected ByteBuffer buf_video;
	protected ByteBuffer buf_audio;
	private static class Lock {};
	private final Object lock_scr = new Lock();
	protected AudioTrack audio;
	protected byte[] aud;
	public View(Context context)
	{
		super(context);
		bmp = Bitmap.createBitmap(320, 240, Bitmap.Config.RGB_565);
		buf_video = ByteBuffer.allocateDirect(320*240*2);
		buf_audio = ByteBuffer.allocateDirect(32768);
		sh = null;
		aud = new byte[32768];
		getHolder().addCallback(this);
		final int freq = 44100;
		final int channels = AudioFormat.CHANNEL_CONFIGURATION_STEREO;
		final int format = AudioFormat.ENCODING_PCM_16BIT;
		final int buf_size = AudioTrack.getMinBufferSize(freq, channels, format);
		audio = new AudioTrack(	AudioManager.STREAM_MUSIC,
								freq, channels, format, buf_size*4,
								AudioTrack.MODE_STREAM);
		audio.play();
		StartRenderThread();
	}
	private void StartRenderThread()
	{
		Thread t = new Thread(new Runnable()
		{
			public void run()
			{
				for(;;)
				{
					if(sh != null)
						Draw();
					else
						Thread.yield();
				}
			}
		},"draw");
		t.start();
	}
	public void surfaceCreated(SurfaceHolder holder)
	{
		synchronized(lock_scr) { sh = holder; }
	}
	public void surfaceDestroyed(SurfaceHolder holder)
	{
		synchronized(lock_scr) { sh = null; }
	}
	public void surfaceChanged(SurfaceHolder holder, int format, int width, int height)
	{
		synchronized(lock_scr) { sh = holder; }
	}
	@Override
	protected void onMeasure(int w, int h)
	{
		setMeasuredDimension(320, 240);
	}
	private void Draw()
	{
		Emulator.the.UpdateVideo(buf_video);
		buf_video.rewind();
		bmp.copyPixelsFromBuffer(buf_video);
		final int audio_bytes_ready = Emulator.the.UpdateAudio(buf_audio);
		if(audio_bytes_ready != 0)
		{
			buf_audio.rewind();
			buf_audio.get(aud);
			audio.write(aud, 0, audio_bytes_ready);
		}
		synchronized(lock_scr)
		{
			if(sh != null)
			{
				Canvas c = sh.lockCanvas();
				c.drawBitmap(bmp, 0, 0, null);
				sh.unlockCanvasAndPost(c);
			}
		}
	}
}

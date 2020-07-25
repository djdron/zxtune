package app.zxtune.core.jni;

import java.lang.annotation.Native;
import java.nio.ByteBuffer;

import app.zxtune.core.Module;
import app.zxtune.core.ModuleDetectCallback;
import app.zxtune.core.Player;
import app.zxtune.core.Properties;
import app.zxtune.core.ResolvingException;

public final class JniModule implements Module {

  static {
    JniLibrary.load();
  }

  @Native
  private final int handle;

  JniModule(int handle) {
    this.handle = handle;
    JniGC.register(this, handle);
  }

  public static native JniModule load(ByteBuffer data, String subpath) throws ResolvingException;

  public static native void detect(ByteBuffer data, ModuleDetectCallback callback);

  @Override
  public final void release() {
    close(handle);
  }

  static native void close(int handle);

  @Override
  public long getDurationInMs() {
    final long frameDuration = getProperty(Properties.Sound.FRAMEDURATION,
        Properties.Sound.FRAMEDURATION_DEFAULT) / 1000;
    return frameDuration * getDuration();
  }

  private native int getDuration();

  @Override
  public native Player createPlayer();

  @Override
  public native long getProperty(String name, long defVal);

  @Override
  public native String getProperty(String name, String defVal);

  @Override
  public native String[] getAdditionalFiles();

  @Override
  public native void resolveAdditionalFile(String name, ByteBuffer data);
}

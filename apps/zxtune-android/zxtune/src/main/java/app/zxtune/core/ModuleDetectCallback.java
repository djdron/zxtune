package app.zxtune.core;

public interface ModuleDetectCallback {
  void onModule(String subpath, Module obj);
  void onProgress(int progress);
}

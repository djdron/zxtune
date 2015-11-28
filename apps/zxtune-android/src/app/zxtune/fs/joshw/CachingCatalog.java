/**
 * 
 * @file
 *
 * @brief
 *
 * @author vitamin.caig@gmail.com
 * 
 */

package app.zxtune.fs.joshw;

import java.io.File;
import java.io.IOException;
import java.nio.ByteBuffer;
import java.util.List;

import android.text.TextUtils;
import app.zxtune.fs.VfsCache;

class CachingCatalog extends Catalog {

  //private final static String TAG = CachingCatalog.class.getName();
  private final static String CACHE_HTML_FILE = File.separator + "index.html";

  private final Catalog remote;
  private final VfsCache cacheDir;
  
  public CachingCatalog(Catalog remote, VfsCache cacheDir) {
    this.remote = remote;
    this.cacheDir = cacheDir;
  }

  @Override
  public ByteBuffer getFileContent(List<String> path) throws IOException {
    final String relPath = TextUtils.join(File.separator, path);
    final ByteBuffer cache = cacheDir.getCachedFileContent(relPath);
    if (cache != null) {
      return cache;
    }
    final String relPathHtml = relPath + CACHE_HTML_FILE;
    final ByteBuffer cacheHtml = cacheDir.getCachedFileContent(relPathHtml);
    if (cacheHtml != null && isDirContent(cacheHtml)) {
      return cacheHtml;
    }
    final ByteBuffer content = remote.getFileContent(path);
    if (isDirContent(content)) {
      cacheDir.putAnyCachedFileContent(relPathHtml, content);
    } else {
      cacheDir.putCachedFileContent(relPath, content);
    }
    return content;
  }
  
  @Override
  public boolean isDirContent(ByteBuffer buffer) {
    return remote.isDirContent(buffer);
  }

  @Override
  public void parseDir(ByteBuffer data, DirVisitor visitor) throws IOException {
    remote.parseDir(data, visitor);
  }
}

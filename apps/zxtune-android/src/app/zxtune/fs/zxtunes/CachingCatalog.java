/**
 *
 * @file
 *
 * @brief Caching catalog implementation
 *
 * @author vitamin.caig@gmail.com
 *
 */

package app.zxtune.fs.zxtunes;

import java.io.IOException;
import java.nio.ByteBuffer;
import java.util.concurrent.TimeUnit;

import app.zxtune.Log;
import app.zxtune.TimeStamp;
import app.zxtune.fs.VfsCache;
import app.zxtune.fs.dbhelpers.QueryCommand;
import app.zxtune.fs.dbhelpers.Timestamps;
import app.zxtune.fs.dbhelpers.Transaction;
import app.zxtune.fs.dbhelpers.Utils;
import app.zxtune.fs.zxtunes.Catalog.FoundTracksVisitor;

final class CachingCatalog extends Catalog {
  
  private final static String TAG = CachingCatalog.class.getName();
  
  private final TimeStamp AUTHORS_TTL = TimeStamp.createFrom(30, TimeUnit.DAYS);
  
  private final Catalog remote;
  private final Database db;
  private final VfsCache cacheDir;
  
  public CachingCatalog(Catalog remote, Database db, VfsCache cacheDir) {
    this.remote = remote;
    this.db = db;
    this.cacheDir = cacheDir;
  }
  
  @Override
  public void queryAuthors(final AuthorsVisitor visitor) throws IOException {
    Utils.executeQueryCommand(new QueryCommand() {
      @Override
      public Timestamps.Lifetime getLifetime() {
        return db.getAuthorsLifetime(AUTHORS_TTL);
      }
      
      @Override
      public Transaction startTransaction() {
        return db.startTransaction();
      }

      @Override
      public boolean queryFromCache() {
        return db.queryAuthors(visitor);
      }

      @Override
      public void queryFromRemote() throws IOException {
        Log.d(TAG, "Authors cache is empty/expired");
        remote.queryAuthors(new CachingAuthorsVisitor());
      }
    });
  }
  
  @Override
  public void queryAuthorTracks(final Author author, final TracksVisitor visitor) throws IOException {
    Utils.executeQueryCommand(new QueryCommand() {
      @Override
      public Timestamps.Lifetime getLifetime() {
        return db.getAuthorTracksLifetime(author, AUTHORS_TTL);
      }
      
      @Override
      public Transaction startTransaction() {
        return db.startTransaction();
      }

      @Override
      public boolean queryFromCache() {
        return db.queryAuthorTracks(author, visitor);
      }

      @Override
      public void queryFromRemote() throws IOException {
        Log.d(TAG, "Tracks cache is empty/expired for author=%d", author.id);
        remote.queryAuthorTracks(author, new CachingTracksVisitor(author));
      }
    });
  }
  
  @Override
  public boolean searchSupported() {
    //TODO: rework logic to more clear
    if (remote.searchSupported()) {
      //network is available, so direct scanning may be more comprehensive
      //TODO: check out if all the cache is not expired
      Log.d(TAG, "Disable fast search due to enabled network");
      return false;
    }
    return true;
  }
  
  @Override
  public void findTracks(String query, FoundTracksVisitor visitor) throws IOException {
    //TODO: query also remote catalog when search will be enabled
    db.findTracks(query, visitor);
  }
  
  @Override
  public ByteBuffer getTrackContent(int id) throws IOException {
    final String strId = Integer.toString(id);
    final ByteBuffer cachedContent = cacheDir.getCachedFileContent(strId);
    if (cachedContent != null) {
      return cachedContent;
    } else {
      final ByteBuffer content = remote.getTrackContent(id);
      cacheDir.putCachedFileContent(strId, content);
      return content;
    }
  }

  private class CachingAuthorsVisitor extends AuthorsVisitor {

    @Override
    public void accept(Author obj) {
      try {
        db.addAuthor(obj);
      } catch (Exception e) {
        Log.d(TAG, e, "acceptAuthor()");
      }
    }
  }
  
  private class CachingTracksVisitor extends TracksVisitor {
    
    private final Author author;
    
    CachingTracksVisitor(Author author) {
      this.author = author;
    }
    
    @Override
    public void accept(Track track) {
      try {
        db.addTrack(track);
        db.addAuthorTrack(author, track);
      } catch (Exception e) {
        Log.d(TAG, e, "acceptTrack()");
      }
    }
  }
}

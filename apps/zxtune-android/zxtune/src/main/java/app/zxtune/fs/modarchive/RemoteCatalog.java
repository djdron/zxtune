/**
 * @file
 * @brief Remote implementation of catalog
 * @author vitamin.caig@gmail.com
 */

package app.zxtune.fs.modarchive;

import android.content.Context;
import android.content.pm.ApplicationInfo;
import android.content.pm.PackageManager;
import android.content.pm.PackageManager.NameNotFoundException;
import android.net.Uri;
import android.os.Bundle;
import android.sax.Element;
import android.sax.EndElementListener;
import android.sax.EndTextElementListener;
import android.sax.RootElement;
import android.text.Html;
import android.util.Xml;

import androidx.annotation.Nullable;

import org.xml.sax.SAXException;

import java.io.BufferedInputStream;
import java.io.IOException;
import java.io.InputStream;

import app.zxtune.Log;
import app.zxtune.StubProgressCallback;
import app.zxtune.fs.HtmlUtils;
import app.zxtune.fs.ProgressCallback;
import app.zxtune.fs.api.Cdn;
import app.zxtune.fs.http.MultisourceHttpProvider;

/**
 * API entry point:
 *   http://api.modarchive.org
 *
 * Authors:
 *   ${API}/xml-tools.php?key=${key}&request=search_artist&page=${page}
 *
 * references author's tracks at
 *   ${API}/xml-tools.php?key=${key}&request=view_modules_by_artistid&query=${id}&page=${page}
 *
 * references tracks at
 *   ${API}/downloads.php?moduleid=${moduleid}
 *
 * Genres:
 *   ${API}/xml-tools.php?key=${key}&request=view_genres (no paging)
 *
 * references genre's tracks at
 *   ${API}/xml-tools.php?key=${key}&request=search&type=genre&query=${genreid}&page=${page}
 *
 * Search:
 *   ${API}/xml-tools.php?key=${key}&request=search&type=filename_or_songtitle&query=*${query}*&page=${page}
 */

public class RemoteCatalog extends Catalog {

  private static final String TAG = RemoteCatalog.class.getName();

  private static class ApiUriBuilder {
    private final Uri.Builder delegate;

    private ApiUriBuilder() {
      delegate = new Uri.Builder();
      delegate.scheme("https");
      delegate.authority("api.modarchive.org");
    }

    final ApiUriBuilder setRequest(String request) {
      delegate.appendQueryParameter("request", request);
      return this;
    }

    final ApiUriBuilder setQuery(String query) {
      delegate.appendQueryParameter("query", query);
      return this;
    }

    final ApiUriBuilder setQuery(int query) {
      return setQuery(Integer.toString(query));
    }

    final ApiUriBuilder setType(String type) {
      delegate.appendQueryParameter("type", type);
      return this;
    }

    final Uri buildUri() {
      return delegate.build();
    }

    final String build() {
      return buildUri().toString();
    }

    static ApiUriBuilder forQuery(String key) {
      final ApiUriBuilder result = new ApiUriBuilder();
      result.delegate.path("xml-tools.php");
      result.delegate.appendQueryParameter("key", key);
      return result;
    }

    static ApiUriBuilder forDownload(int trackId) {
      final ApiUriBuilder result = new ApiUriBuilder();
      result.delegate.path("downloads.php");
      result.delegate.appendQueryParameter("moduleid", Integer.toString(trackId));
      return result;
    }
  }

  private final MultisourceHttpProvider http;
  private final String key;

  RemoteCatalog(Context context, MultisourceHttpProvider http) {
    this.http = http;
    final Bundle metaData = getAppMetadata(context);
    this.key = metaData.getString("key.modarchive");
  }

  private static Bundle getAppMetadata(Context context) {
    try {
      final ApplicationInfo info = context.getPackageManager().getApplicationInfo(context.getPackageName(), PackageManager.GET_META_DATA);
      return info.metaData;
    } catch (NameNotFoundException e) {
      return new Bundle();
    }
  }

  @Override
  public void queryAuthors(AuthorsVisitor visitor, ProgressCallback progress) throws IOException {
    Log.d(TAG, "queryAuthors()");
    final String uri = ApiUriBuilder.forQuery(key).setRequest("search_artist").build();
    final RootElement root = createAuthorsParserRoot(visitor);
    loadPages(uri, root, progress);
    visitor.accept(Author.UNKNOWN);
  }

  @Override
  public void queryGenres(GenresVisitor visitor) throws IOException {
    Log.d(TAG, "queryGenres()");
    final String uri = ApiUriBuilder.forQuery(key).setRequest("view_genres").build();
    final RootElement root = createGenresParserRoot(visitor);
    loadSinglePage(uri, root);
  }

  @Override
  public void queryTracks(Author author, TracksVisitor visitor, ProgressCallback progress) throws IOException {
    Log.d(TAG, "queryTracks(author=%d)", author.id);
    final String uri = ApiUriBuilder.forQuery(key).setRequest("view_modules_by_artistid").setQuery(author.id).build();
    final RootElement root = createTracksParserRoot(visitor);
    loadPages(uri, root, progress);
  }

  @Override
  public void queryTracks(Genre genre, TracksVisitor visitor, ProgressCallback progress) throws IOException {
    Log.d(TAG, "queryTracks(genre=%d)", genre.id);
    final String uri = ApiUriBuilder.forQuery(key).setRequest("search").setType("genre").setQuery(genre.id).build();
    final RootElement root = createTracksParserRoot(visitor);
    loadPages(uri, root, progress);
  }

  final boolean searchSupported() {
    return http.hasConnection();
  }

  @Override
  public void findTracks(String query, FoundTracksVisitor visitor) throws IOException {
    Log.d(TAG, "findTracks(query=%s)", query);
    final String uri = ApiUriBuilder.forQuery(key).setRequest("search").setType("filename_or_songtitle").setQuery("*" + query + "*").build();
    final RootElement root = createFoundTracksParserRoot(visitor);
    loadPages(uri, root, StubProgressCallback.instance());
  }

  @Override
  public void findRandomTracks(TracksVisitor visitor) throws IOException {
    Log.d(TAG, "findRandomTracks()");
    final String uri = ApiUriBuilder.forQuery(key).setRequest("random").build();
    final RootElement root = createTracksParserRoot(visitor);
    loadSinglePage(uri, root);
  }

  public static Uri[] getTrackUris(int id) {
    return new Uri[]{
        Cdn.modarchive(id),
        ApiUriBuilder.forDownload(id).buildUri()
    };
  }

  private static RootElement createAuthorsParserRoot(final AuthorsVisitor visitor) {
    final AuthorBuilder builder = new AuthorBuilder();
    final RootElement root = createRootElement();
    root.getChild("total_results").setEndTextElementListener(new EndTextElementListener() {
      @Override
      public void end(String body) {
        final Integer result = HtmlUtils.tryGetInteger(body);
        if (result != null) {
          visitor.setCountHint(result);
        }
      }
    });
    final Element item = root.getChild("items").getChild("item");
    item.setEndElementListener(new EndElementListener() {
      @Override
      public void end() {
        final Author res = builder.captureResult();
        if (res != null) {
          visitor.accept(res);
        }
      }
    });
    bindAuthorFields(item, builder);
    return root;
  }

  private static class AuthorBuilder {
    @Nullable
    private Integer id;
    @Nullable
    private String alias;

    final void setId(String val) {
      id = HtmlUtils.tryGetInteger(val);
    }

    final void setAlias(String val) {
      alias = val;
    }

    @Nullable
    final Author captureResult() {
      final Author res = isValid() ? new Author(id, alias) : null;
      id = null;
      alias = null;
      return res;
    }

    private boolean isValid() {
      return id != null && alias != null;
    }
  }

  private static RootElement createGenresParserRoot(final GenresVisitor visitor) {
    final GenreBuilder builder = new GenreBuilder();
    final RootElement root = createRootElement();
    root.getChild("results").setEndTextElementListener(new EndTextElementListener() {
      @Override
      public void end(String body) {
        final Integer result = HtmlUtils.tryGetInteger(body);
        if (result != null) {
          visitor.setCountHint(result);
        }
      }
    });
    final Element item = root.getChild("parent").getChild("children").getChild("child");
    item.setEndElementListener(new EndElementListener() {
      @Override
      public void end() {
        final Genre result = builder.captureResult();
        if (result != null) {
          visitor.accept(result);
        }
      }
    });
    item.getChild("id").setEndTextElementListener(new EndTextElementListener() {
      @Override
      public void end(String body) {
        builder.setId(body);
      }
    });
    item.getChild("text").setEndTextElementListener(new EndTextElementListener() {
      @Override
      public void end(String body) {
        builder.setText(body);
      }
    });
    item.getChild("files").setEndTextElementListener(new EndTextElementListener() {
      @Override
      public void end(String body) {
        builder.setFiles(body);
      }
    });
    return root;
  }

  private static class GenreBuilder {
    @Nullable
    private Integer id;
    @Nullable
    private String text;
    @Nullable
    private Integer files;

    final void setId(String val) {
      id = HtmlUtils.tryGetInteger(val);
    }

    final void setText(String val) {
      text = val;
    }

    final void setFiles(String val) {
      files = HtmlUtils.tryGetInteger(val);
    }

    @Nullable
    final Genre captureResult() {
      final Genre res = isValid() ? new Genre(id, text, files) : null;
      id = null;
      files = null;
      text = null;
      return res;
    }

    private boolean isValid() {
      return id != null && text != null && files != null;
    }
  }

  private static RootElement createTracksParserRoot(final TracksVisitor visitor) {
    final TrackBuilder builder = new TrackBuilder();
    final RootElement root = createRootElement();
    root.getChild("total_results").setEndTextElementListener(new EndTextElementListener() {
      @Override
      public void end(String body) {
        final Integer result = HtmlUtils.tryGetInteger(body);
        if (result != null) {
          visitor.setCountHint(result);
        }
      }
    });
    final Element item = root.getChild("module");
    item.setEndElementListener(new EndElementListener() {
      @Override
      public void end() {
        final Track result = builder.captureResult();
        if (result != null) {
          visitor.accept(result);
        }
      }
    });
    bindTrackFields(item, builder);
    return root;
  }

  private static class TrackBuilder {
    @Nullable
    private Integer id;
    @Nullable
    private String filename;
    @Nullable
    private String title;
    @Nullable
    private Integer size;

    final void setId(String val) {
      id = HtmlUtils.tryGetInteger(val);
    }

    final void setFilename(String val) {
      filename = val;
    }

    final void setTitle(String val) {
      title = val;
    }

    final void setSize(String val) {
      size = HtmlUtils.tryGetInteger(val);
    }

    @Nullable
    final Track captureResult() {
      final Track res = isValid() ? new Track(id, filename, title, size) : null;
      id = null;
      size = null;
      filename = null;
      title = null;
      return res;
    }

    private boolean isValid() {
      return id != null && filename != null && title != null && size != null;
    }
  }

  private static RootElement createFoundTracksParserRoot(final FoundTracksVisitor visitor) {
    final TrackBuilder trackBuilder = new TrackBuilder();
    final AuthorBuilder authorBuilder = new AuthorBuilder();
    final RootElement root = createRootElement();
    root.getChild("results").setEndTextElementListener(new EndTextElementListener() {
      @Override
      public void end(String body) {
        final Integer result = HtmlUtils.tryGetInteger(body);
        if (result != null) {
          visitor.setCountHint(result);
        }
      }
    });
    final Element item = root.getChild("module");
    item.setEndElementListener(new EndElementListener() {
      @Override
      public void end() {
        final Track track = trackBuilder.captureResult();
        final Author author = authorBuilder.captureResult();
        if (track != null) {
          visitor.accept(author != null ? author : Author.UNKNOWN, track);
        }
      }
    });
    bindTrackFields(item, trackBuilder);
    bindAuthorFields(item.getChild("artist_info").getChild("artist"), authorBuilder);
    return root;
  }

  private static void bindTrackFields(Element item, final TrackBuilder builder) {
    item.getChild("id").setEndTextElementListener(new EndTextElementListener() {
      @Override
      public void end(String body) {
        builder.setId(body);
      }
    });
    item.getChild("filename").setEndTextElementListener(new EndTextElementListener() {
      @Override
      public void end(String body) {
        builder.setFilename(body);
      }
    });
    item.getChild("bytes").setEndTextElementListener(new EndTextElementListener() {
      @Override
      public void end(String body) {
        builder.setSize(body);
      }
    });
    item.getChild("songtitle").setEndTextElementListener(new EndTextElementListener() {
      @Override
      public void end(String body) {
        //CDATA
        builder.setTitle(Html.fromHtml(body).toString());
      }
    });
  }

  private static void bindAuthorFields(Element item, final AuthorBuilder builder) {
    item.getChild("id").setEndTextElementListener(new EndTextElementListener() {
      @Override
      public void end(String body) {
        builder.setId(body);
      }
    });
    item.getChild("alias").setEndTextElementListener(new EndTextElementListener() {
      @Override
      public void end(String body) {
        builder.setAlias(body);
      }
    });
  }

  private void loadPages(String baseUri, RootElement root, final ProgressCallback progress) throws IOException {
    final int[] totalPages = new int[]{1};
    root.getChild("totalpages").setEndTextElementListener(new EndTextElementListener() {
      @Override
      public void end(String body) {
        final Integer result = HtmlUtils.tryGetInteger(body);
        if (totalPages[0] == 1 && result != null) {
          Log.d(TAG, "Loading %d pages", result);
          totalPages[0] = result;
        }
      }
    });
    for (int page = 1; page <= totalPages[0]; ++page) {
      final String pageUri = baseUri + "&page=" + page;
      loadSinglePage(pageUri, root);
      progress.onProgressUpdate(page - 1, totalPages[0]);
    }
  }

  private void loadSinglePage(String uri, RootElement root) throws IOException {
    final InputStream stream = http.getInputStream(Uri.parse(uri));
    performXmlQuery(stream, root);
  }

  private void performXmlQuery(InputStream httpStream, RootElement root) throws IOException {
    try {
      final InputStream stream = new BufferedInputStream(httpStream);
      Xml.parse(stream, Xml.Encoding.UTF_8, root.getContentHandler());
    } catch (SAXException e) {
      throw new IOException(e);
    } finally {
      httpStream.close();
    }
  }

  private static RootElement createRootElement() {
    return new RootElement("modarchive");
  }
}

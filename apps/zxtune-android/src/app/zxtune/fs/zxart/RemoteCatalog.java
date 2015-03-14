/**
 *
 * @file
 *
 * @brief Remote implementation of catalog
 *
 * @author vitamin.caig@gmail.com
 *
 */

package app.zxtune.fs.zxart;

import java.io.BufferedInputStream;
import java.io.IOException;
import java.io.InputStream;
import java.net.HttpURLConnection;
import java.nio.ByteBuffer;
import java.util.Locale;

import org.xml.sax.SAXException;

import android.content.Context;
import android.sax.Element;
import android.sax.EndElementListener;
import android.sax.EndTextElementListener;
import android.sax.RootElement;
import android.util.Log;
import android.util.Xml;
import app.zxtune.Util;
import app.zxtune.fs.HttpProvider;

final class RemoteCatalog extends Catalog {

  private static final String TAG = RemoteCatalog.class.getName();

  //no www. prefix!!!
  private static final String SITE = "http://zxart.ee";
  private static final String API = SITE + "/zxtune/language:eng";
  private static final String ACTION_AUTHORS = "/action:authors";
  private static final String ACTION_PARTIES = "/action:parties";
  private static final String ACTION_TRACKS = "/action:tunes";
  private static final String ACTION_TOP = "/action:topTunes";
  private static final String LIMIT = "/limit:%d";
  private static final String AUTHOR_ID = "/authorId:%d";
  private static final String PARTY_ID = "/partyId:%d";
  private static final String TRACK_ID = "/tuneId:%d";
  private static final String ALL_TRACKS_QUERY = API + ACTION_TRACKS;
  private static final String TRACK_QUERY = ALL_TRACKS_QUERY + TRACK_ID;
  private static final String DOWNLOAD_QUERY = SITE + "/file/id:%d";
  private static final String ALL_AUTHORS_QUERY = API + ACTION_AUTHORS;
  private static final String AUTHOR_QUERY = ALL_AUTHORS_QUERY + AUTHOR_ID;
  private static final String AUTHOR_TRACKS_QUERY = ALL_TRACKS_QUERY + AUTHOR_ID;
  private static final String ALL_PARTIES_QUERY = API + ACTION_PARTIES;
  private static final String PARTY_QUERY = ALL_PARTIES_QUERY + PARTY_ID;
  private static final String PARTY_TRACKS_QUERY = ALL_TRACKS_QUERY + PARTY_ID;
  private static final String TOP_TRACKS_QUERY = API + ACTION_TOP + LIMIT;

  private final HttpProvider http;

  public RemoteCatalog(Context context) {
    this.http = new HttpProvider(context);
  }

  @Override
  public void queryAuthors(AuthorsVisitor visitor, Integer id) throws IOException {
    final String query =
        id == null ? ALL_AUTHORS_QUERY : String.format(Locale.US, AUTHOR_QUERY, id);
    final HttpURLConnection connection = http.connect(query);
    final RootElement root = createAuthorsParserRoot(visitor);
    performQuery(connection, root);
  }

  @Override
  public void queryAuthorTracks(TracksVisitor visitor, Author author, Integer id) throws IOException {
    if (id != null) {
      queryTracks(visitor, String.format(Locale.US, TRACK_QUERY, id));
    } else {
      queryTracks(visitor, String.format(Locale.US, AUTHOR_TRACKS_QUERY, author.id));
    }
  }

  public void queryParties(PartiesVisitor visitor, Integer id) throws IOException {
    final String query =
        id == null ? ALL_PARTIES_QUERY : String.format(Locale.US, PARTY_QUERY, id);
    final HttpURLConnection connection = http.connect(query);
    final RootElement root = createPartiesParserRoot(visitor);
    performQuery(connection, root);
  }
  
  public void queryPartyTracks(TracksVisitor visitor, Party party, Integer id) throws IOException {
    if (id != null) {
      queryTracks(visitor, String.format(Locale.US, TRACK_QUERY, id));
    } else {
      queryTracks(visitor, String.format(Locale.US, PARTY_TRACKS_QUERY, party.id));
    }
  }
  
  public void queryTopTracks(TracksVisitor visitor, Integer id, int limit) throws IOException {
    if (id != null) {
      queryTracks(visitor, String.format(Locale.US, TRACK_QUERY, id));
    } else {
      queryTracks(visitor, String.format(Locale.US, TOP_TRACKS_QUERY, limit));
    }
  }
  
  private void queryTracks(TracksVisitor visitor, String query) throws IOException {
    final HttpURLConnection connection = http.connect(query);
    final RootElement root = createModulesParserRoot(visitor);
    performQuery(connection, root);
  }

  @Override
  public ByteBuffer getTrackContent(int id) throws IOException {
    try {
      final String query = String.format(Locale.US, DOWNLOAD_QUERY, id);
      return http.getContent(query);
    } catch (IOException e) {
      Log.d(TAG, "getModuleContent(" + id + ")", e);
      throw e;
    }
  }

  private void performQuery(HttpURLConnection connection, RootElement root)
      throws IOException {
    try {
      final InputStream stream = new BufferedInputStream(connection.getInputStream());
      Xml.parse(stream, Xml.Encoding.UTF_8, root.getContentHandler());
    } catch (SAXException e) {
      throw new IOException(e);
    } catch (IOException e) {
      http.checkConnectionError();
      throw e;
    } finally {
      connection.disconnect();
    }
  }
  
  private static Integer asInt(String str) {
    if (str == null) {
      return null;
    } else try {
      return Integer.parseInt(str);
    } catch (NumberFormatException e) {
      return null;
    }
  }

  private static RootElement createAuthorsParserRoot(final AuthorsVisitor visitor) {
    final AuthorBuilder builder = new AuthorBuilder();
    final RootElement result = createRootElement();
    final Element data = result.getChild("responseData");
    data.getChild("totalAmount").setEndTextElementListener(new EndTextElementListener() {
      @Override
      public void end(String body) {
        final Integer count = asInt(body);
        if (count != null) {
          visitor.setCountHint(count);
        }
      }
    });
    final Element item = data.getChild("authors").getChild("author");
    item.setEndElementListener(new EndElementListener() {
      @Override
      public void end() {
        final Author res = builder.captureResult();
        if (res != null) {
          visitor.accept(res);
        }
      }
    });
    item.getChild("id").setEndTextElementListener(new EndTextElementListener() {
      @Override
      public void end(String body) {
        builder.setId(body);
      }
    });
    item.getChild("title").setEndTextElementListener(new EndTextElementListener() {
      @Override
      public void end(String body) {
        builder.setNickname(body);
      }
    });
    item.getChild("realName").setEndTextElementListener(new EndTextElementListener() {
      @Override
      public void end(String body) {
        builder.setName(body);
      }
    });
    return result;
  }

  private static class AuthorBuilder {

    private Integer id;
    private String nickname;
    private String name;

    final void setId(String val) {
      id = Integer.valueOf(val);
    }

    final void setNickname(String val) {
      nickname = val;
    }

    final void setName(String val) {
      name = val;
    }

    final Author captureResult() {
      final Author res = isValid() ? new Author(id, nickname, name) : null;
      id = null;
      nickname = name = null;
      return res;
    }
    
    private boolean isValid() {
      return id != null && nickname != null;
    }
  }

  private static RootElement createPartiesParserRoot(final PartiesVisitor visitor) {
    final PartiesBuilder builder = new PartiesBuilder();
    final RootElement result = createRootElement();
    final Element data = result.getChild("responseData");
    data.getChild("totalAmount").setEndTextElementListener(new EndTextElementListener() {
      @Override
      public void end(String body) {
        final Integer count = asInt(body);
        if (count != null) {
          visitor.setCountHint(count);
        }
      }
    });
    final Element item = data.getChild("parties").getChild("party");
    item.setEndElementListener(new EndElementListener() {
      @Override
      public void end() {
        final Party res = builder.captureResult();
        if (res != null) {
          visitor.accept(res);
        }
      }
    });
    item.getChild("id").setEndTextElementListener(new EndTextElementListener() {
      @Override
      public void end(String body) {
        builder.setId(body);
      }
    });
    item.getChild("title").setEndTextElementListener(new EndTextElementListener() {
      @Override
      public void end(String body) {
        builder.setName(body);
      }
    });
    item.getChild("year").setEndTextElementListener(new EndTextElementListener() {
      @Override
      public void end(String body) {
        builder.setYear(body);
      }
    });
    return result;
  }

  private static class PartiesBuilder {

    private Integer id;
    private String name;
    private int year;

    final void setId(String val) {
      id = Integer.valueOf(val);
    }

    final void setName(String val) {
      name = val;
    }
    
    final void setYear(String val) {
      try {
        year = Integer.valueOf(val);
      } catch (NumberFormatException e) {
        year = 0;
      }
    }

    final Party captureResult() {
      final Party res = isValid() ? new Party(id, name, year) : null;
      id = null;
      name = null;
      year = 0;
      return res;
    }
    
    private boolean isValid() {
      return id != null && name != null;
    }
  }
  
  private static RootElement createModulesParserRoot(final TracksVisitor visitor) {
    final ModuleBuilder builder = new ModuleBuilder();
    final RootElement result = createRootElement();
    final Element data = result.getChild("responseData");
    data.getChild("totalAmount").setEndTextElementListener(new EndTextElementListener() {
      @Override
      public void end(String body) {
        final Integer count = asInt(body);
        if (count != null) {
          visitor.setCountHint(count);
        }
      }
    });
    final Element item = data.getChild("tunes").getChild("tune");
    item.setEndElementListener(new EndElementListener() {
      @Override
      public void end() {
        final Track result = builder.captureResult();
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
    item.getChild("originalFileName").setEndTextElementListener(new EndTextElementListener() {
      @Override
      public void end(String body) {
        builder.setFilename(body);
      }
    });
    item.getChild("title").setEndTextElementListener(new EndTextElementListener() {
      @Override
      public void end(String body) {
        builder.setTitle(body);
      }
    });
    item.getChild("internalAuthor").setEndTextElementListener(new EndTextElementListener() {
      @Override
      public void end(String body) {
        builder.setInternalAuthor(body);
      }
    });
    item.getChild("internalTitle").setEndTextElementListener(new EndTextElementListener() {
      @Override
      public void end(String body) {
        builder.setInternalTitle(body);
      }
    });
    item.getChild("votes").setEndTextElementListener(new EndTextElementListener() {
      @Override
      public void end(String body) {
        builder.setVotes(body);
      }
    });
    item.getChild("time").setEndTextElementListener(new EndTextElementListener() {
      @Override
      public void end(String body) {
        builder.setDuration(body);
      }
    });
    item.getChild("year").setEndTextElementListener(new EndTextElementListener() {
      @Override
      public void end(String body) {
        builder.setYear(body);
      }
    });
    item.getChild("compo").setEndTextElementListener(new EndTextElementListener() {
      @Override
      public void end(String body) {
        builder.setCompo(body);
      }
    });
    item.getChild("partyplace").setEndTextElementListener(new EndTextElementListener() {
      @Override
      public void end(String body) {
        builder.setPartyplace(body);
      }
    });
    return result;
  }

  private static class ModuleBuilder {

    private Integer id;
    private String filename;
    private String title;
    private String internalAuthor;
    private String internalTitle;
    private String votes;
    private String duration;
    private int year;
    private String compo;
    private int partyplace;
    
    ModuleBuilder() {
      reset();
    }

    final void setId(String val) {
      id = Integer.valueOf(val);
    }

    final void setFilename(String val) {
      filename = val.trim();
      if (filename.length() == 0) {
        filename = "unknown";
      }
    }

    final void setTitle(String val) {
      title = val;
    }
    
    final void setInternalAuthor(String val) {
      internalAuthor = val;
    }
    
    final void setInternalTitle(String val) {
      internalTitle = val;
    }
    
    final void setVotes(String val) {
      votes = val;
    }

    final void setDuration(String val) {
      duration = val;
    }

    final void setYear(String val) {
      try {
        year = Integer.valueOf(val);
      } catch (NumberFormatException e) {
        year = 0;
      }
    }
    
    final void setCompo(String val) {
      compo = val;
    }
    
    final void setPartyplace(String val) {
      try {
        partyplace = Integer.valueOf(val);
      } catch (NumberFormatException e) {
        partyplace = 0;
      }
    }

    final Track captureResult() {
      title = Util.formatTrackTitle(internalAuthor, internalTitle, title);
      final Track res = isValid()
        ? new Track(id, filename, title, votes, duration, year, compo, partyplace)
        : null;
      reset();
      return res;
    }
    
    private void reset() {
      id = null;
      filename = null;
      year = partyplace = 0;
      votes = duration = title = internalAuthor = internalTitle = compo = "".intern();
    }
    
    private boolean isValid() {
      return id != null && filename != null;
    }
  }

  private static RootElement createRootElement() {
    return new RootElement("response");
  }
}

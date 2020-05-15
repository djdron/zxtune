package app.zxtune.ui.playlist;

import android.app.Application;
import android.database.Cursor;

import androidx.annotation.NonNull;
import androidx.fragment.app.Fragment;
import androidx.lifecycle.AndroidViewModel;
import androidx.lifecycle.LiveData;
import androidx.lifecycle.MutableLiveData;
import androidx.lifecycle.ViewModelProviders;

import java.util.ArrayList;
import java.util.List;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;
import java.util.concurrent.TimeUnit;

import app.zxtune.TimeStamp;
import app.zxtune.core.Identifier;
import app.zxtune.playlist.Database;
import app.zxtune.playlist.ProviderClient;

// public for provider
public class Model extends AndroidViewModel {

  private final ProviderClient client;
  private final ExecutorService async;
  private MutableLiveData<List<Entry>> items;

  static Model of(Fragment owner) {
    return ViewModelProviders.of(owner).get(Model.class);
  }

  public Model(@NonNull Application application) {
    super(application);
    this.client = new ProviderClient(application);
    this.async = Executors.newSingleThreadExecutor();
    client.registerObserver(new ProviderClient.ChangesObserver() {
      @Override
      public void onChange() {
        if (items == null) {
          items = new MutableLiveData<>();
        }
        loadAsync();
      }
    });
  }

  final LiveData<List<Entry>> getItems() {
    if (items == null) {
      items = new MutableLiveData<>();
      loadAsync();
    }
    return items;
  }

  private void loadAsync() {
    async.execute(new Runnable() {
      @Override
      public void run() {
        load();
      }
    });
  }

  private void load() {
    final Cursor cursor = client.query(null);
    if (cursor != null) {
      try {
        final ArrayList<Entry> items = new ArrayList<>(cursor.getCount());
        while (cursor.moveToNext()) {
          items.add(createItem(cursor));
        }
        this.items.postValue(items);
      } finally {
        cursor.close();
      }
    }
  }

  private static Entry createItem(Cursor cursor) {
    final Entry item = new Entry();
    item.id = cursor.getLong(Database.Tables.Playlist.Fields._id.ordinal());
    item.location = Identifier.parse(cursor.getString(Database.Tables.Playlist.Fields.location.ordinal()));
    item.title = cursor.getString(Database.Tables.Playlist.Fields.title.ordinal());
    item.author = cursor.getString(Database.Tables.Playlist.Fields.author.ordinal());
    item.duration = TimeStamp.createFrom(cursor.getLong(Database.Tables.Playlist.Fields.duration.ordinal()),
        TimeUnit.MILLISECONDS);
    return item;
  }
}
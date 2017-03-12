/**
 *
 * @file
 *
 * @brief Playlist fragment component
 *
 * @author vitamin.caig@gmail.com
 *
 */

package app.zxtune.ui;

import android.app.Activity;
import android.net.Uri;
import android.os.Bundle;
import android.support.v4.app.Fragment;
import android.view.LayoutInflater;
import android.view.Menu;
import android.view.MenuInflater;
import android.view.MenuItem;
import android.view.SubMenu;
import android.view.View;
import android.view.ViewGroup;
import android.widget.AdapterView;
import android.widget.TextView;

import app.zxtune.Identifier;
import app.zxtune.Log;
import app.zxtune.PlaybackServiceConnection;
import app.zxtune.Preferences;
import app.zxtune.R;
import app.zxtune.Releaseable;
import app.zxtune.playback.CallbackStub;
import app.zxtune.playback.CallbackSubscription;
import app.zxtune.playback.Item;
import app.zxtune.playback.PlaybackService;
import app.zxtune.playback.PlaybackServiceStub;
import app.zxtune.playback.PlaylistControl;
import app.zxtune.playlist.PlaylistQuery;

public class PlaylistFragment extends Fragment implements PlaybackServiceConnection.Callback {

  private static final String TAG = PlaylistFragment.class.getName();
  private PlaybackService service;
  private Releaseable connection;
  private PlaylistState state;
  private PlaylistView listing;
  private final NowPlayingState playingState;

  public static Fragment createInstance() {
    return new PlaylistFragment();
  }
  
  public PlaylistFragment() {
    this.service = PlaybackServiceStub.instance();
    this.playingState = new NowPlayingState();
    setHasOptionsMenu(true);
  }
  
  @Override
  public void onAttach(Activity activity) {
    super.onAttach(activity);
    
    state = new PlaylistState(Preferences.getDefaultSharedPreferences(activity));
  }
  
  @Override
  public void onCreate(Bundle savedInstanceState) {
    super.onCreate(savedInstanceState);
    
    setHasOptionsMenu(true);
  }
  
  @Override
  public void onCreateOptionsMenu (Menu menu, MenuInflater inflater) {
    super.onCreateOptionsMenu(menu, inflater);

    inflater.inflate(R.menu.playlist, menu);
    final MenuItem sortItem = menu.findItem(R.id.action_sort);
    final SubMenu sortMenuRoot = sortItem.getSubMenu();
    for (PlaylistControl.SortBy sortBy : PlaylistControl.SortBy.values()) {
      for (PlaylistControl.SortOrder sortOrder : PlaylistControl.SortOrder.values()) {
        final MenuItem item = sortMenuRoot.add(getMenuTitle(sortBy));
        final PlaylistControl.SortBy by = sortBy;
        final PlaylistControl.SortOrder order = sortOrder;
        item.setOnMenuItemClickListener(new MenuItem.OnMenuItemClickListener() {
          @Override
          public boolean onMenuItemClick(MenuItem item) {
            getService().getPlaylistControl().sort(by, order);
            return true;
          }
        });
        item.setIcon(getMenuIcon(sortOrder));
      }
    }
  }
  
  private static int getMenuTitle(PlaylistControl.SortBy by) {
    if (PlaylistControl.SortBy.title.equals(by)) {
      return R.string.information_title;
    } else if (PlaylistControl.SortBy.author.equals(by)) {
      return R.string.information_author;
    } else if (PlaylistControl.SortBy.duration.equals(by)) {
      return R.string.statistics_duration;//TODO: extract
    } else {
      throw new RuntimeException();
    }
  }
  
  private static int getMenuIcon(PlaylistControl.SortOrder order) {
    if (PlaylistControl.SortOrder.asc.equals(order)) {
      return android.R.drawable.arrow_up_float;
    } else if (PlaylistControl.SortOrder.desc.equals(order)) {
      return android.R.drawable.arrow_down_float;
    } else {
      throw new RuntimeException();
    }
  }
  
  @Override
  public boolean onOptionsItemSelected (MenuItem item) {
    switch (item.getItemId()) {
      case R.id.action_clear:
        getService().getPlaylistControl().deleteAll();
        break;
      case R.id.action_save:
        savePlaylist(null);
        break;
      case R.id.action_statistics:
        showStatistics(null);
        break;
      default:
        return super.onOptionsItemSelected(item);
    }
    return true;
  }
  
  private void savePlaylist(long[] ids) {
    PlaylistSaveFragment.createInstance(ids).show(getFragmentManager(), "save");
  }
  
  private void showStatistics(long[] ids) {
    PlaylistStatisticsFragment.createInstance(ids).show(getFragmentManager(), "statistics");
  }
  
  @Override
  public View onCreateView(LayoutInflater inflater, ViewGroup container, Bundle savedInstanceState) {
    return container != null ? inflater.inflate(R.layout.playlist, container, false) : null;
  }
  
  @Override
  public synchronized void onViewCreated(View view, Bundle savedInstanceState) {
    super.onViewCreated(view, savedInstanceState);

    listing = (PlaylistView) view.findViewById(R.id.playlist_content);
    listing.setOnItemClickListener(new OnItemClickListener());
    listing.setPlayitemStateSource(playingState);
    listing.setEmptyView(view.findViewById(R.id.playlist_stub));
    listing.setMultiChoiceModeListener(new MultiChoiceModeListener());
    setEmptyText(R.string.starting);

    if (savedInstanceState == null) {
      Log.d(TAG, "Loading persistent state");
      listing.storeViewPosition(state.getCurrentViewPosition());
    }
    listing.setRemoveListener(new PlaylistView.RemoveListener() {
      @Override
      public void remove(int which) {
        final long[] id = {listing.getItemIdAtPosition(which)};
        getService().getPlaylistControl().delete(id);
      }
    });
    listing.setDropListener(new PlaylistView.DropListener() {
      @Override
      public void drop(int from, int to) {
        if (from != to) {
          //TODO: perform in separate thread
          final long id = listing.getItemIdAtPosition(from);
          getService().getPlaylistControl().move(id, to - from);
        }
      }
    });
  }
  
  @Override
  public synchronized void onStart() {
    super.onStart();
    
    bindViewToConnectedService();
  }
  
  @Override
  public synchronized void onStop() {
    super.onStop();
    
    Log.d(TAG, "Saving persistent state");
    state.setCurrentViewPosition(listing.getFirstVisiblePosition());
    unbindFromService();//may affect playlistState
  }
  
  @Override
  public synchronized void onServiceConnected(PlaybackService service) {
    this.service = service;
    
    bindViewToConnectedService();
  }
  
  private synchronized PlaybackService getService() {
    return this.service;
  }
  
  private void bindViewToConnectedService() {
    assert service != null;
    final boolean serviceConnected = service != PlaybackServiceStub.instance();
    final boolean viewCreated = listing != null;
    if (serviceConnected && viewCreated) {
      Log.d(TAG, "Subscribe to service events");
      connection = new CallbackSubscription(service, playingState);
      //do not display anything before service started to prevent empty clicks
      loadListing();
    }
  }
  
  private void loadListing() {
    listing.load(getLoaderManager());
    setEmptyText(R.string.playlist_empty);
  }
  
  private void setEmptyText(int res) {
    ((TextView) listing.getEmptyView()).setText(res);
  }
  
  private void unbindFromService() {
    try {
      if (connection != null) {
        Log.d(TAG, "Unsubscribe from service events");
        connection.release();
      }
    } finally {
      connection = null;
    }
  }
  
  private class OnItemClickListener implements PlaylistView.OnItemClickListener {

    @Override
    public void onItemClick(AdapterView<?> parent, View view, int pos, long id) {
      final Uri[] toPlay = {PlaylistQuery.uriFor(id)};
      service.setNowPlaying(toPlay);
    }
  }
  
  private class NowPlayingState extends CallbackStub implements PlaylistView.PlayitemStateSource {
    
    private boolean isPlaying;
    private Uri nowPlayingPlaylist;
    
    public NowPlayingState() {
      isPlaying = false;
      nowPlayingPlaylist = Uri.EMPTY;
    }
    
    @Override
    public void onStatusChanged(boolean nowPlaying) {
      if (isPlaying != nowPlaying) {
        isPlaying = nowPlaying;
        updateView();
      }
    }
    
    @Override
    public void onItemChanged(Item item) {
      final Uri id = item.getId();
      final Identifier contentId = item.getDataId();
      final Uri playlistId = 0 == id.compareTo(contentId.getFullLocation()) ? Uri.EMPTY : id;
      if (0 != playlistId.compareTo(nowPlayingPlaylist)) {
        nowPlayingPlaylist = playlistId;
        updateView();
      }
    }

    @Override
    public boolean isPlaying(Uri playlistUri) {
      return isPlaying && 0 == playlistUri.compareTo(nowPlayingPlaylist);
    }
    
    private void updateView() {
      getActivity().getContentResolver().notifyChange(PlaylistQuery.ALL, null);
    }
  }
  
  private String getActionModeTitle() {
    final int count = listing.getCheckedItemsCount();
    return getResources().getQuantityString(R.plurals.tracks, count, count);
  }
  
  private class MultiChoiceModeListener implements ListViewCompat.MultiChoiceModeListener {

    @Override
    public boolean onCreateActionMode(ListViewCompat.ActionMode mode, Menu menu) {
      final MenuInflater inflater = mode.getMenuInflater();
      inflater.inflate(R.menu.selection, menu);
      inflater.inflate(R.menu.playlist_items, menu);
      mode.setTitle(getActionModeTitle());
      return true;
    }

    @Override
    public boolean onPrepareActionMode(ListViewCompat.ActionMode mode, Menu menu) {
      return false;
    }

    @Override
    public boolean onActionItemClicked(ListViewCompat.ActionMode mode, MenuItem item) {
      if (listing.processActionItemClick(item.getItemId())) {
        return true;
      } else {
        switch (item.getItemId()) {
          case R.id.action_delete:
            service.getPlaylistControl().delete(listing.getCheckedItemIds());
            break;
          case R.id.action_save:
            savePlaylist(listing.getCheckedItemIds());
            break;
          case R.id.action_statistics:
            showStatistics(listing.getCheckedItemIds());
            break;
          default:
            return false;
        }
        mode.finish();
        return true;
      }
    }

    @Override
    public void onDestroyActionMode(ListViewCompat.ActionMode mode) {
    }

    @Override
    public void onItemCheckedStateChanged(ListViewCompat.ActionMode mode, int position, long id,
        boolean checked) {
      mode.setTitle(getActionModeTitle());
    }
  }
}

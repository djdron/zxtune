/**
 *
 * @file
 *
 * @brief Vfs browser fragment component
 *
 * @author vitamin.caig@gmail.com
 *
 */

package app.zxtune.ui;

import android.app.Activity;
import android.net.Uri;
import android.os.Bundle;
import android.support.v4.app.Fragment;
import android.support.v7.widget.SearchView;
import android.util.Log;
import android.util.SparseBooleanArray;
import android.view.LayoutInflater;
import android.view.Menu;
import android.view.MenuInflater;
import android.view.MenuItem;
import android.view.View;
import android.view.ViewGroup;
import android.view.ViewGroup.LayoutParams;
import android.widget.AdapterView;
import android.widget.LinearLayout;
import android.widget.ListAdapter;
import android.widget.ProgressBar;
import app.zxtune.PlaybackServiceConnection;
import app.zxtune.R;
import app.zxtune.fs.Vfs;
import app.zxtune.fs.VfsDir;
import app.zxtune.fs.VfsFile;
import app.zxtune.fs.VfsObject;
import app.zxtune.playback.PlaybackService;
import app.zxtune.playback.PlaybackServiceStub;
import app.zxtune.ui.browser.BreadCrumbsView;
import app.zxtune.ui.browser.BrowserController;
import app.zxtune.ui.browser.BrowserView;

public class BrowserFragment extends Fragment implements PlaybackServiceConnection.Callback {

  private static final String TAG = BrowserFragment.class.getName();
  private static final String SEARCH_QUERY_KEY = "search_query";
  private static final String SEARCH_FOCUSED_KEY = "search_focused";
  
  private BrowserController controller;
  private View sources;
  private SearchView search;
  private BrowserView listing;
  private PlaybackService service;

  public static BrowserFragment createInstance() {
    return new BrowserFragment();
  }
  
  public BrowserFragment() {
    this.service = PlaybackServiceStub.instance();
  }

  @Override
  public void onAttach(Activity activity) {
    super.onAttach(activity);

    this.controller = new BrowserController(this);
  }

  @Override
  public View onCreateView(LayoutInflater inflater, ViewGroup container, Bundle savedInstanceState) {
    return container != null ? inflater.inflate(R.layout.browser, container, false) : null;
  }

  @Override
  public synchronized void onViewCreated(View view, Bundle savedInstanceState) {
    super.onViewCreated(view, savedInstanceState);
    
    sources = view.findViewById(R.id.browser_sources);
    search = setupSearchView(view);
    setupRootsView(view);
    final BreadCrumbsView position = setupPositionView(view);
    final ProgressBar progress = (ProgressBar) view.findViewById(R.id.browser_loading);
    listing = setupListing(view);
    
    controller.setViews(position, progress, listing);
    
    controller.loadState();
  }

  @Override
  public synchronized void onDestroyView() {
    super.onDestroyView();
    
    Log.d(TAG, "Saving persistent state");
    controller.storeCurrentViewPosition();
    controller.resetViews();
  }
  
  private BrowserView setupListing(View view) {
    final BrowserView listing = (BrowserView) view.findViewById(R.id.browser_content);
    listing.setOnItemClickListener(new OnItemClickListener());
    listing.setEmptyView(view.findViewById(R.id.browser_stub));
    listing.setMultiChoiceModeListener(new MultiChoiceModeListener());
    return listing;
  }

  private BreadCrumbsView setupPositionView(View view) {
    final BreadCrumbsView position  = (BreadCrumbsView) view.findViewById(R.id.browser_breadcrumb);
    position.setDirSelectionListener(new BreadCrumbsView.DirSelectionListener() {
      @Override
      public void onDirSelection(VfsDir dir) {
        controller.setCurrentDir(dir);
      }
    });
    return position;
  }

  private void setupRootsView(View view) {
    final View roots = view.findViewById(R.id.browser_roots);
    roots.setOnClickListener(new View.OnClickListener() {
      @Override
      public void onClick(View v) {
        controller.setCurrentDir(Vfs.getRoot());
      }
    });
  }

  private SearchView setupSearchView(View view) {
    final SearchView search = (SearchView) view.findViewById(R.id.browser_search);

    search.setOnSearchClickListener(new SearchView.OnClickListener() {
      @Override
      public void onClick(View view) {
        final LinearLayout.LayoutParams params = (LinearLayout.LayoutParams) search.getLayoutParams();
        params.width = LayoutParams.MATCH_PARENT;
        search.setLayoutParams(params);
      }
    });
    search.setOnQueryTextListener(new SearchView.OnQueryTextListener() {
      @Override
      public boolean onQueryTextSubmit(String query) {
        controller.search(query);
        search.clearFocus();
        return true;
      }
      
      @Override
      public boolean onQueryTextChange(String query) {
        return false;
      }
    });
    search.setOnCloseListener(new SearchView.OnCloseListener() {
      @Override
      public boolean onClose() {
        final LinearLayout.LayoutParams params = (LinearLayout.LayoutParams) search.getLayoutParams();
        params.width = LayoutParams.WRAP_CONTENT;
        search.setLayoutParams(params);
        search.post(new Runnable() {
          @Override
          public void run() {
            search.clearFocus();
            if (controller.isInSearch()) {
              controller.loadCurrentDir();
            }
          }
        });
        return false;
      }
    });
    search.setOnFocusChangeListener(new View.OnFocusChangeListener() {
      @Override
      public void onFocusChange(View v, boolean hasFocus) {
        if (0 == search.getQuery().length()) {
          search.setIconified(true);
        }
      }
    });
    return search;
  }
  
  @Override
  public void onSaveInstanceState(Bundle state) {
      super.onSaveInstanceState(state);
      if (!search.isIconified()) {
        final String query = search.getQuery().toString();
        state.putString(SEARCH_QUERY_KEY, query);
        state.putBoolean(SEARCH_FOCUSED_KEY, search.hasFocus());
      }
  }

  @Override
  public void onViewStateRestored(Bundle state) {
      super.onViewStateRestored(state);
      if (state == null || !state.containsKey(SEARCH_QUERY_KEY)) {
        return;
      }
      final String query = state.getString(SEARCH_QUERY_KEY);
      final boolean isFocused = state.getBoolean(SEARCH_FOCUSED_KEY);
      search.post(new Runnable() {
        @Override
        public void run() {
          search.setIconified(false);
          search.setQuery(query, false);
          if (!isFocused) {
            search.clearFocus();
          }
        }
      });
  }  
  
  public final void moveUp() {
    if (!search.isIconified()) {
      search.setIconified(true);
    } else {
      controller.moveToParent();
    }
  }

  @Override
  public synchronized void onServiceConnected(PlaybackService service) {
    this.service = service;
  }
  
  private synchronized PlaybackService getService() {
    return this.service;
  }
  
  private String getActionModeTitle() {
    final int count = listing.getCheckedItemsCount();
    return getResources().getQuantityString(R.plurals.items, count, count);
  }

  class OnItemClickListener implements AdapterView.OnItemClickListener {

    @Override
    public void onItemClick(AdapterView<?> parent, View view, int position, long id) {
      final Object obj = parent.getItemAtPosition(position);
      if (obj instanceof VfsDir) {
        controller.setCurrentDir((VfsDir) obj);
      } else if (obj instanceof VfsFile) {
        final Uri[] toPlay = getUrisFrom(position);
        getService().setNowPlaying(toPlay);
      }
    }

    private Uri[] getUrisFrom(int position) {
      final ListAdapter adapter = listing.getAdapter();
      final Uri[] result = new Uri[adapter.getCount() - position];
      for (int idx = 0; idx != result.length; ++idx) {
        final VfsObject obj = (VfsObject) adapter.getItem(position + idx);
        result[idx] = obj.getUri();
      }
      return result;
    }
  }

  private class MultiChoiceModeListener implements ListViewCompat.MultiChoiceModeListener {

    @Override
    public boolean onCreateActionMode(ListViewCompat.ActionMode mode, Menu menu) {
      setEnabledRecursive(sources, false);
      final MenuInflater inflater = mode.getMenuInflater();
      inflater.inflate(R.menu.selection, menu);
      inflater.inflate(R.menu.browser, menu);
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
          case R.id.action_play:
            getService().setNowPlaying(getSelectedItemsUris());
            break;
          case R.id.action_add:
            getService().getPlaylistControl().add(getSelectedItemsUris());
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
      setEnabledRecursive(sources, true);
    }

    @Override
    public void onItemCheckedStateChanged(ListViewCompat.ActionMode mode, int position, long id,
        boolean checked) {
      mode.setTitle(getActionModeTitle());
    }
    
    private Uri[] getSelectedItemsUris() {
      final Uri[] result = new Uri[listing.getCheckedItemsCount()];
      final SparseBooleanArray selected = listing.getCheckedItemPositions();
      final ListAdapter adapter = listing.getAdapter();
      int pos = 0;
      for (int i = 0, lim = selected.size(); i != lim; ++i) {
        if (selected.valueAt(i)) {
          final VfsObject obj = (VfsObject) adapter.getItem(selected.keyAt(i));
          result[pos++] = obj.getUri();
        }
      }
      return result;
    }
  }
  
  private static void setEnabledRecursive(View view, boolean enabled) {
    if (view instanceof ViewGroup) {
      final ViewGroup group = (ViewGroup) view;
      for (int idx = 0, lim = group.getChildCount(); idx != lim; ++idx) {
        setEnabledRecursive(group.getChildAt(idx), enabled);
      }
    } else {
      view.setEnabled(enabled);
    }
  }
}

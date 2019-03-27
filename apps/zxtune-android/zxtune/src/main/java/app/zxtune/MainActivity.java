/**
 * @file
 * @brief Main application activity
 * @author vitamin.caig@gmail.com
 */

package app.zxtune;

import android.Manifest;
import android.app.PendingIntent;
import android.arch.lifecycle.LiveData;
import android.arch.lifecycle.Observer;
import android.arch.lifecycle.ViewModelProviders;
import android.content.ActivityNotFoundException;
import android.content.Context;
import android.content.Intent;
import android.net.Uri;
import android.os.Build;
import android.os.Bundle;
import android.support.annotation.Nullable;
import android.support.v4.app.DialogFragment;
import android.support.v4.app.FragmentManager;
import android.support.v4.app.FragmentTransaction;
import android.support.v4.media.session.MediaControllerCompat;
import android.support.v4.view.ViewPager;
import android.support.v7.app.AppCompatActivity;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.widget.Toast;
import app.zxtune.models.MediaSessionConnection;
import app.zxtune.models.MediaSessionModel;
import app.zxtune.ui.AboutFragment;
import app.zxtune.ui.BrowserFragment;
import app.zxtune.ui.NowPlayingFragment;
import app.zxtune.ui.PlaylistFragment;
import app.zxtune.ui.ViewPagerAdapter;

public class MainActivity extends AppCompatActivity {

  private static final int NO_PAGE = -1;
  private ViewPager pager;
  private int browserPageIndex;
  private BrowserFragment browser;
  private MediaSessionConnection sessionConnection;

  public static PendingIntent createPendingIntent(Context ctx) {
    final Intent intent = new Intent(ctx, MainActivity.class);
    intent.setFlags(Intent.FLAG_ACTIVITY_CLEAR_TOP | Intent.FLAG_ACTIVITY_SINGLE_TOP);
    return PendingIntent.getActivity(ctx, 0, intent, 0);
  }

  @Override
  public void onCreate(Bundle savedInstanceState) {
    super.onCreate(savedInstanceState);
    setContentView(R.layout.main_activity);

    fillPages();
    if (Build.VERSION.SDK_INT >= 16) {
      Permission.request(this, Manifest.permission.READ_EXTERNAL_STORAGE);
    }
    Permission.request(this, Manifest.permission.WRITE_EXTERNAL_STORAGE);

    sessionConnection = new MediaSessionConnection(this);

    if (savedInstanceState == null) {
      subscribeForPendingOpenRequest();
    }
  }

  @Override
  public void onStart() {
    super.onStart();
    sessionConnection.connect();
  }

  @Override
  public void onStop() {
    super.onStop();
    sessionConnection.disconnect();
  }

  @Override
  public boolean onCreateOptionsMenu(Menu menu) {
    super.onCreateOptionsMenu(menu);
    getMenuInflater().inflate(R.menu.main, menu);
    return true;
  }

  @Override
  public boolean onOptionsItemSelected(MenuItem item) {
    switch (item.getItemId()) {
      case R.id.action_prefs:
        showPreferences();
        break;
      case R.id.action_about:
        showAbout();
        break;
      case R.id.action_rate:
        rateApplication();
        break;
      case R.id.action_quit:
        quit();
        break;
      default:
        return super.onOptionsItemSelected(item);
    }
    return true;
  }

  @Override
  public void onBackPressed() {
    if (pager != null && pager.getCurrentItem() == browserPageIndex) {
      browser.moveUp();
    } else {
      super.onBackPressed();
    }
  }

  private void subscribeForPendingOpenRequest() {
    final Intent intent = getIntent();
    if (intent != null && Intent.ACTION_VIEW.equals(intent.getAction())) {
      final Uri uri = intent.getData();
      if (uri != null) {
        final MediaSessionModel model = ViewModelProviders.of(this).get(MediaSessionModel.class);
        final LiveData<MediaControllerCompat> ctrl = model.getMediaController();
        ctrl.observe(this,
            new Observer<MediaControllerCompat>() {
              @Override
              public void onChanged(@Nullable MediaControllerCompat mediaControllerCompat) {
                if (mediaControllerCompat != null) {
                  mediaControllerCompat.getTransportControls().playFromUri(uri, null);
                  ctrl.removeObserver(this);
                }
              }
            });
      }
    }
  }

  private void fillPages() {
    final FragmentManager manager = getSupportFragmentManager();
    final FragmentTransaction transaction = manager.beginTransaction();
    if (null == manager.findFragmentById(R.id.now_playing)) {
      transaction.replace(R.id.now_playing, NowPlayingFragment.createInstance());
    }
    browser = (BrowserFragment) manager.findFragmentById(R.id.browser_view);
    if (null == browser) {
      browser = BrowserFragment.createInstance();
      transaction.replace(R.id.browser_view, browser);
    }
    if (null == manager.findFragmentById(R.id.playlist_view)) {
      transaction.replace(R.id.playlist_view, PlaylistFragment.createInstance());
    }
    transaction.commit();
    setupViewPager();
  }

  private void setupViewPager() {
    pager = findViewById(R.id.view_pager);
    if (null != pager) {
      final ViewPagerAdapter adapter = new ViewPagerAdapter(pager);
      pager.setAdapter(adapter);
      browserPageIndex = adapter.getCount() - 1;
      while (browserPageIndex >= 0 && !hasBrowserView(adapter.instantiateItem(pager, browserPageIndex))) {
        --browserPageIndex;
      }
    } else {
      browserPageIndex = NO_PAGE;
    }
  }

  private static boolean hasBrowserView(Object view) {
    return ((View) view).findViewById(R.id.browser_view) != null;
  }

  private void showPreferences() {
    final Intent intent = new Intent(this, PreferencesActivity.class);
    startActivity(intent);
    Analytics.sendUIEvent("Preferences");
  }

  private void rateApplication() {
    final Intent intent = new Intent(Intent.ACTION_VIEW);
    intent.setData(Uri.parse("market://details?id=" + getPackageName()));
    if (safeStartActivity(intent)) {
      Analytics.sendUIEvent("Rate");
    } else {
      intent.setData(Uri.parse("https://play.google.com/store/apps/details?id=" + getPackageName()));
      if (safeStartActivity(intent)) {
        Analytics.sendUIEvent("Rate");
      } else {
        Toast.makeText(this, "Error", Toast.LENGTH_LONG).show();
      }
    }
  }

  private boolean safeStartActivity(Intent intent) {
    try {
      startActivity(intent);
      return true;
    } catch (ActivityNotFoundException e) {
      return false;
    }
  }

  private void showAbout() {
    final DialogFragment fragment = AboutFragment.createInstance();
    fragment.show(getSupportFragmentManager(), "about");
    Analytics.sendUIEvent("About");
  }

  private void quit() {
    final Intent intent = MainService.createIntent(this, null);
    stopService(intent);
    finish();
  }
}

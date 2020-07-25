/**
 *
 * @file
 *
 * @brief Permission request helper
 *
 * @author vitamin.caig@gmail.com
 *
 */

package app.zxtune;

import android.annotation.TargetApi;
import android.content.Context;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.net.Uri;
import android.os.Build;
import android.provider.Settings;
import androidx.core.app.ActivityCompat;
import androidx.fragment.app.FragmentActivity;
import androidx.core.content.ContextCompat;

public class Permission {
  
  static void request(FragmentActivity act, String id) {
    if (Build.VERSION.SDK_INT >= 23) {
      requestMarshmallow(act, id);
    }
  }
  
  public static boolean requestSystemSettings(FragmentActivity act) {
    if (Build.VERSION.SDK_INT >= 23) {
      return requestSystemSettingsMarshmallow(act);
    } else {
      return true;
    }
  }
  
  @TargetApi(23)
  private static void requestMarshmallow(FragmentActivity act, String id) {
    if (ContextCompat.checkSelfPermission(act, id) != PackageManager.PERMISSION_GRANTED) {
      ActivityCompat.requestPermissions(act, new String[]{id}, id.hashCode() & 0xffff);
    }
  }
  
  @TargetApi(23)
  private static boolean requestSystemSettingsMarshmallow(Context ctx) {
    if (Settings.System.canWrite(ctx)) {
      return true;
    }
    final Intent intent = new Intent(Settings.ACTION_MANAGE_WRITE_SETTINGS);
    intent.setData(Uri.parse("package:" + ctx.getPackageName()));
    ctx.startActivity(intent);
    return false;
  }
}

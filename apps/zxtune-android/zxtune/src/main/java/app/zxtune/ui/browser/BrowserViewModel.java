/**
 *
 * @file
 *
 * @brief File browser model interface
 *
 * @author vitamin.caig@gmail.com
 *
 */

package app.zxtune.ui.browser;

import android.view.View;
import android.view.ViewGroup;

interface BrowserViewModel {
  
  int getCount();
  
  boolean isEmpty();
  
  Object getItem(int position);
  
  View getView(int position, View convertView, ViewGroup parent);
}

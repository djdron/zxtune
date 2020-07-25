/**
 *
 * @file
 *
 * @brief Seek controller interface
 *
 * @author vitamin.caig@gmail.com
 *
 */

package app.zxtune.playback;

import app.zxtune.TimeStamp;

public interface SeekControl {
  
  TimeStamp getDuration();

  TimeStamp getPosition();

  void setPosition(TimeStamp position);
}

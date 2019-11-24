package app.zxtune.core;

import androidx.annotation.NonNull;

import app.zxtune.Releaseable;

/**
 * Player interface
 */
public interface Player extends PropertiesAccessor, PropertiesModifier, Releaseable {

  /**
   * @return Index of next rendered frame
   */
  int getPosition();

  /**
   * @param levels Array of levels to store
   * @return Count of actually stored entries
   */
  int analyze(@NonNull byte levels[]);

  /**
   * Render next result.length bytes of sound data
   *
   * @param result Buffer to put data
   * @return Is there more data to render
   */
  boolean render(@NonNull short[] result);

  /**
   * @param pos Index of next rendered frame
   */
  void setPosition(int pos);

  /**
   * @return Rendering performance in percents
   */
  int getPerformance();

  /**
   * @return Playback progress in percents (may be >100!!!)
   */
  int getProgress();
}

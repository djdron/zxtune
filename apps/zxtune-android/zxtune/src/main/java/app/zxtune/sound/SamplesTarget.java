/**
 *
 * @file
 *
 * @brief Samples target interface
 *
 * @author vitamin.caig@gmail.com
 *
 */

package app.zxtune.sound;

public interface SamplesTarget {

  /**
   * @return target sample rate in Hz
   */
  public int getSampleRate();
  
  /**
   * @return buffer size in samples
   */
  public int getPreferableBufferSize();

  /**
   * Initialize target
   */
  public void start();
  
  /**
   * @param buffer sound data in S16/stereo/interleaved format
   */
  public void writeSamples(short[] buffer);

  /**
   * Deinitialize target
   */
  public void stop();
  
  /**
   * Release all internal resources
   */
  public void release();
}

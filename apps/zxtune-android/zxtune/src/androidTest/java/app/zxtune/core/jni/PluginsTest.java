package app.zxtune.core.jni;

import static org.junit.Assert.*;

import androidx.annotation.NonNull;
import org.junit.*;

import java.util.HashSet;

public class PluginsTest {

  @Test
  public void testPlugins() {
    final int[] flags = {0, 0};
    final int[] counts = {0, 0};
    final HashSet<String> players = new HashSet<>();
    final HashSet<String> containers = new HashSet<>();
    Plugins.enumerate(new Plugins.Visitor() {
      @Override
      public void onPlayerPlugin(int devices, @NonNull String id, @NonNull String description) {
        flags[0] |= devices;
        ++counts[0];
        players.add(id);
      }

      @Override
      public void onContainerPlugin(int type, @NonNull String id, @NonNull String description) {
        flags[1] |= 1 << type;
        ++counts[1];
        containers.add(id);
      }
    });

    assertEquals(0b0011_1111_1111_0111, flags[0]);
    assertEquals(0b0011_0001, flags[1]);
    assertEquals(105, counts[0]);
    assertEquals(23, counts[1]);
  }
}
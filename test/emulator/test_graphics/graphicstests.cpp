#include "graphicstests.h"
#include <unity.h>
#include "graphics/graphics.h"
#include "types.h"
#include "state.h"


void testGetMode ()
{
  Byte current_line;
  State *state = new State;

  // Test mode 0: HBlank
  state->cycles = 70224*10 + 456*4 + 280 + 10;
  TEST_ASSERT_EQUAL(0, getMode(state, current_line));

  // Test mode 1: VBlank
  state->cycles = 70224*10 + 456*150;
  TEST_ASSERT_EQUAL(1, getMode(state, current_line));

  // Test mode 2: OAM scan
  state->cycles = 70224*10 + 456*4 + 10;
  TEST_ASSERT_EQUAL(2, getMode(state, current_line));

  // Test mode 3: drawing
  state->cycles = 70224*10 + 456*4 + 80 + 10;
  TEST_ASSERT_EQUAL(3, getMode(state, current_line));
}
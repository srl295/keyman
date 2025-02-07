/*
  Copyright:    © 2018 SIL International.
  Description:  Tests for kmx integration
  Create Date:  30 Oct 2018
  Authors:      Marc Durdin (MD), Tim Eves (TSE)

  Note: Exit codes will be 100*LINE + ERROR CODE, e.g. 25005 is code 5 on line 250
*/
#include <algorithm>
#include <cctype>
#include <fstream>
#include <iostream>
#include <iomanip>
#include <iterator>
#include <list>
#include <sstream>
#include <string>
#include <type_traits>

#include <kmx/kmx_processevent.h>
#include <kmx/kmx_xstring.h>

#include "path.hpp"
#include "state.hpp"
#include "utfcodec.hpp"

#include "../test_assert.h"
#include "../test_color.h"

#include "kmx_test_source.hpp"

namespace {

bool g_beep_found = false;

km_kbp_option_item test_env_opts[] =
{
  KM_KBP_OPTIONS_END
};

std::string
string_to_hex(const std::u16string &input) {
  std::ostringstream result;
  result << std::setfill('0') << std::hex << std::uppercase;

  for (size_t i = 0; i < input.length(); i++) {
    unsigned int ch = input[i];
    if (i < input.length() - 1 && Uni_IsSurrogate1(input[i]) && Uni_IsSurrogate2(input[i + 1])) {
      ch = Uni_SurrogateToUTF32(input[i], input[i + 1]);
      i++;
    }

    result << "U+" << std::setw(4) << ch << " ";
  }
  return result.str();
}

void
apply_action(
    km_kbp_state const *,
    km_kbp_action_item const &act,
    std::u16string &text_store,
    std::vector<km_kbp_context_item> &context,
    km::tests::kmx_options &options,
    km::tests::KmxTestSource &test_source) {
  switch (act.type) {
  case KM_KBP_IT_END:
    assert(false);
    break;
  case KM_KBP_IT_ALERT:
    g_beep_found = true;
    // std::cout << "beep" << std::endl;
    break;
  case KM_KBP_IT_CHAR:
    context.push_back(km_kbp_context_item{
        KM_KBP_CT_CHAR,
        {
            0,
        },
        {act.character}});
    if (Uni_IsSMP(act.character)) {
      text_store.push_back(Uni_UTF32ToSurrogate1(act.character));
      text_store.push_back(Uni_UTF32ToSurrogate2(act.character));
    } else {
      text_store.push_back(act.character);
    }
    // std::cout << "char(" << act.character << ") size=" << cp->size() << std::endl;
    break;
  case KM_KBP_IT_MARKER:
    // std::cout << "deadkey(" << act.marker << ")" << std::endl;
    context.push_back(km_kbp_context_item{
        KM_KBP_CT_MARKER,
        {
            0,
        },
        {(uint32_t)act.marker}});
    break;
  case KM_KBP_IT_BACK:
    // It is valid for a backspace to be received with an empty text store
    // as the user can press backspace with no text in the store and Keyman
    // will pass that back to the client, as the client may do additional
    // processing at start of a text store, e.g. delete from a previous cell
    // in a table. Or, if Keyman has a cached context, then there may be
    // additional text in the text store that Keyman can't see.
    if (act.backspace.expected_type == KM_KBP_BT_MARKER) {
      assert(!context.empty());
      assert(context.back().type == KM_KBP_CT_MARKER);
      context.pop_back();
    } else if (text_store.length() > 0) {
      assert(!context.empty() && !text_store.empty());
      km_kbp_usv ch = text_store.back();
      text_store.pop_back();
      if (text_store.length() > 0 && Uni_IsSurrogate2(ch)) {
        auto ch1 = text_store.back();
        if (Uni_IsSurrogate1(ch1)) {
          // We'll only pop the next character off it is actually a
          // surrogate pair
          ch = Uni_SurrogateToUTF32(ch1, ch);
          text_store.pop_back();
        }
      }
      assert(ch == act.backspace.expected_value);

      assert(context.back().type == KM_KBP_CT_CHAR);
      assert(context.back().character == ch);
      context.pop_back();
    }
    break;
  case KM_KBP_IT_PERSIST_OPT: {
    bool found = false;
    for (auto it = options.begin(); it != options.end(); it++) {
      if (it->type == km::tests::KOT_SAVED) {
        if (it->key.compare(act.option->key) == 0) {
          found           = true;
          it->saved_value = act.option->value;
          break;
        }
      }
    }
    std::cout << "action: option " << (act.option->scope == KM_KBP_OPT_ENVIRONMENT ? "environment " : "keyboard ")
              << act.option->key << "=" << act.option->value << " persistence requested" << std::endl;
    if (!found) {
      std::cout << "option " << act.option->key
                << " saved but no expected output found. Suggestion: update test to include saved option value." << std::endl;
    }
  } break;
  case KM_KBP_IT_INVALIDATE_CONTEXT:
    std::cout << "action: context invalidated (markers cleared)" << std::endl;
    break;
  case KM_KBP_IT_EMIT_KEYSTROKE:
    std::cout << "action: emit keystroke" << std::endl;
    break;
  case KM_KBP_IT_CAPSLOCK:
    std::cout << "action: capsLock " << act.capsLock << std::endl;
    test_source.set_caps_lock_on(act.capsLock);
    break;
  default:
    assert(false);  // NOT SUPPORTED
    break;
  }
}

int
run_test(const km::kbp::path &source, const km::kbp::path &compiled) {
  std::string keys = "";
  std::u16string expected = u"", context = u"";
  km::tests::kmx_options options;
  bool expected_beep = false;
  km::tests::KmxTestSource test_source;

  int result = test_source.load_source(source, keys, expected, context, options, expected_beep);
  if (result != 0) return result;

  std::cout << "source file   = " << source << std::endl
            << "compiled file = " << compiled << std::endl;

  km_kbp_keyboard * test_kb = nullptr;
  km_kbp_state * test_state = nullptr;

  try_status(km_kbp_keyboard_load(compiled.c_str(), &test_kb));

  // Setup state, environment
  try_status(km_kbp_state_create(test_kb, test_env_opts, &test_state));

  // Setup keyboard options
  if (options.size() > 0) {
    km_kbp_option_item *keyboard_opts = test_source.get_keyboard_options(options);

    try_status(km_kbp_state_options_update(test_state, keyboard_opts));

    delete [] keyboard_opts;
  }

  // Setup context
  km_kbp_context_item *citems = nullptr;
  try_status(km_kbp_context_items_from_utf16(context.c_str(), &citems));
  try_status(km_kbp_context_set(km_kbp_state_context(test_state), citems));

  // Make a copy of the setup context for the test
  std::vector<km_kbp_context_item> test_context;
  for(km_kbp_context_item *ci = citems; ci->type != KM_KBP_CT_END; ci++) {
    test_context.emplace_back(*ci);
  }

  km_kbp_context_items_dispose(citems);

  // Setup baseline text store
  std::u16string text_store = context;

  // Run through key events, applying output for each event
  for (auto p = test_source.next_key(keys); p.vk != 0; p = test_source.next_key(keys)) {
    // Because a normal system tracks caps lock state itself,
    // we mimic that in the tests. We assume caps lock state is
    // updated on key_down before the processor receives the
    // event.
    if (p.vk == KM_KBP_VKEY_CAPS) {
      test_source.toggle_caps_lock_state();
    }

    for (auto key_down = 1; key_down >= 0; key_down--) {
      try_status(km_kbp_process_event(test_state, p.vk, p.modifier_state | test_source.caps_lock_state(), key_down));

      for (auto act = km_kbp_state_action_items(test_state, nullptr); act->type != KM_KBP_IT_END; act++) {
        apply_action(test_state, *act, text_store, test_context, options, test_source);
      }
    }

    // Compare context and text store at each step - should be identical
    size_t n = 0;
    try_status(km_kbp_context_get(km_kbp_state_context(test_state), &citems));
    try_status(km_kbp_context_items_to_utf16(citems, nullptr, &n));
    km_kbp_cp *buf = new km_kbp_cp[n];
    try_status(km_kbp_context_items_to_utf16(citems, buf, &n));

    // Verify that both our local test_context and the core's test_state.context have
    // not diverged
    auto ci = citems;
    for(auto test_ci = test_context.begin(); ci->type != KM_KBP_CT_END || test_ci != test_context.end(); ci++, test_ci++) {
      assert(ci->type != KM_KBP_CT_END && test_ci != test_context.end()); // Verify that both lists are same length
      assert(test_ci->type == ci->type && test_ci->marker == ci->marker);
    }

    km_kbp_context_items_dispose(citems);
    if (text_store != buf) {
      std::cerr << "text store has diverged from buf" << std::endl;
      std::cerr << "text store: " << string_to_hex(text_store) << " [" << text_store << "]" << std::endl;
      std::cerr << "context   : " << string_to_hex(buf) << " [" << buf << "]" << std::endl;
      assert(false);
    }
  }

  // Test if the beep action was as expected
  if (g_beep_found != expected_beep)
    return __LINE__;

  // Compare final output - retrieve internal context
  size_t n = 0;
  try_status(km_kbp_context_get(km_kbp_state_context(test_state), &citems));
  try_status(km_kbp_context_items_to_utf16(citems, nullptr, &n));
  km_kbp_cp *buf = new km_kbp_cp[n];
  try_status(km_kbp_context_items_to_utf16(citems, buf, &n));

  // Verify that both our local test_context and the core's test_state.context have
  // not diverged
  auto ci = citems;
  for(auto test_ci = test_context.begin(); ci->type != KM_KBP_CT_END || test_ci != test_context.end(); ci++, test_ci++) {
    assert(ci->type != KM_KBP_CT_END && test_ci != test_context.end()); // Verify that both lists are same length
    assert(test_ci->type == ci->type && test_ci->marker == ci->marker);
  }

  km_kbp_context_items_dispose(citems);

  std::cout << "expected  : " << string_to_hex(expected) << " [" << expected << "]" << std::endl;
  std::cout << "text store: " << string_to_hex(text_store) << " [" << text_store << "]" << std::endl;
  std::cout << "context   : " << string_to_hex(buf) << " [" << buf << "]" << std::endl;

  // Compare internal context with expected result
  if (buf != expected) return __LINE__;

  // Compare text store with expected result
  if (text_store != expected) return __LINE__;

  // Test resultant options
  for (auto it = options.begin(); it != options.end(); it++) {
    if (it->type == km::tests::KOT_OUTPUT) {
      std::cout << "output option-key: " << it->key << " expected: " << it->value;
      km_kbp_cp const *value;
      try_status(km_kbp_state_option_lookup(test_state, KM_KBP_OPT_KEYBOARD, it->key.c_str(), &value));
      std::cout << " actual: " << value << std::endl;
      if (it->value.compare(value) != 0) return __LINE__;
    } else if (it->type == km::tests::KOT_SAVED) {
      std::cout << "persisted option-key: " << it->key << " expected: " << it->value << " actual: " << it->saved_value << std::endl;
      if (it->value.compare(it->saved_value) != 0) return __LINE__;
    }
  }

  // Destroy them
  km_kbp_state_dispose(test_state);
  km_kbp_keyboard_dispose(test_kb);

  return 0;
}

constexpr const auto help_str =
    "\
kmx [--color] <KMN_FILE> <KMX_FILE>\n\
help:\n\
\tKMN_FILE:\tThe source file for the keyboard under test.\n\
\tKMX_FILE:\tThe corresponding compiled kmx file produced from KMN_FILE.\n";

}  // namespace

int error_args() {
    std::cerr << "kmx: Not enough arguments." << std::endl;
    std::cout << help_str;
    return 1;
}

int main(int argc, char *argv[]) {
  int first_arg = 1;

  if (argc < 3) {
    return error_args();
  }

  auto arg_color = std::string(argv[1]) == "--color";
  if(arg_color) {
    first_arg++;
    if(argc < 4) {
      return error_args();
    }
  }
  console_color::enabled = console_color::isaterminal() || arg_color;

  km::kbp::kmx::g_debug_ToConsole = TRUE;
  return run_test(argv[first_arg], argv[first_arg + 1]);
}

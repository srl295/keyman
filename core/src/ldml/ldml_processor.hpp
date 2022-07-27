/*
  Copyright:    © 2022 SIL International.
  Description:  LDML Engine class
  Create Date:  27 Jul 2022
  Authors:      Steven R. Loomis (SRL)
*/

#pragma once

#include <string>
#include <unordered_map>
#include <keyman/keyboardprocessor.h>

#include "processor.hpp"
#include "option.hpp"

namespace km {
namespace ldml
{
  class ldml_processor : public km::kbp::abstract_processor
  {
    std::unordered_map<std::u16string, std::u16string> _options;

  public:
    ldml_processor(km::kbp::path const &);

    km_kbp_status
    process_event(
      km_kbp_state *state,
      km_kbp_virtual_key vk,
      uint16_t modifier_state,
      uint8_t is_key_down
    ) override;

    virtual km_kbp_attr const & attributes() const override;
    km_kbp_status               validate() const override;


    char16_t const *
    lookup_option(
      km_kbp_option_scope,
      std::u16string const & key
    ) const override;

    km::kbp::option
    update_option(
      km_kbp_option_scope,
      std::u16string const & key,
      std::u16string const & value
    ) override;

    km_kbp_status process_queued_actions( km_kbp_state *state) override;

    bool queue_action(
      km_kbp_state * state,
      km_kbp_action_item const* action_item
    ) override;

    km_kbp_context_item * get_intermediate_context() override;

    km_kbp_keyboard_key  * get_key_list() const override;

    km_kbp_keyboard_imx  * get_imx_list() const override;

  };

} // namespace kbp
} // namespace km

/**
 *  @file
 *  @copyright defined in eos/LICENSE.txt
 */
#pragma once
#include <appbase/application.hpp>

namespace eosio {

  using namespace appbase;

  /**
   *  This is a template plugin, intended to serve as a starting point for making new plugins
   */
  class account_snapshot_plugin : public appbase::plugin<account_snapshot_plugin> {
  public:
    account_snapshot_plugin();
    virtual ~account_snapshot_plugin();

    APPBASE_PLUGIN_REQUIRES()
    virtual void set_program_options(options_description&, options_description& cfg) override;

    void plugin_initialize(const variables_map& options);
    void plugin_startup();
    void plugin_shutdown();

  private:
    std::unique_ptr<class account_snapshot_plugin_impl> my;
    
  };

}

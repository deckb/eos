/**
 *  @file
 *  @copyright defined in eos/LICENSE.txt
 */
#include <eosio/account_snapshot_plugin/account_snapshot_plugin.hpp>
#include <string>
#include <fc/io/json.hpp>

#include <eosio/chain/controller.hpp>
#include <eosio/chain/trace.hpp>
#include <eosio/chain_plugin/chain_plugin.hpp>
#include <iostream>
#include <fstream>

namespace eosio {
  using namespace chain;
  using boost::signals2::scoped_connection;

  static appbase::abstract_plugin& _account_snapshot_plugin = app().register_plugin<account_snapshot_plugin>();

  class account_snapshot_plugin_impl {
  public:
    chain_plugin*          chain_plug = nullptr;

    //static const int64_t DEFAULT_TRANSACTION_TIME_LIMIT;
    //int64_t transactions_time_limit = DEFAULT_TRANSACTION_TIME_LIMIT;
    
    fc::microseconds       abi_serializer_max_time;

    fc::optional<scoped_connection> applied_transaction_connection;
    
    account_snapshot_plugin_impl()
    {
    }

    void open_log_file(std::string fo) 
    {
      ilog("Opening file ${u}", ("u", fo));
      account_file.open(fo, std::ios_base::app);
    }

    void cleanup()
    {
      account_file.flush();
      account_file.close();
    }
      
    void on_applied_transaction( const transaction_trace_ptr& trace )
    {
      for( const auto& atrace : trace->action_traces )
	{
	  on_action_trace( atrace );
	}
    }

    void on_action_trace( const action_trace& at )
    {
      if (at.act.name == NEW_ACCOUNT) {
	const auto create = at.act.data_as<chain::newaccount>();
	//ilog("Created ${u}", ("u",create.name));
	account_file << reinterpret_cast<const char*>(&at.elapsed) << create.name.to_string() << std::endl;
	account_file.flush();
      }
    }

  private:
    const account_name NEW_ACCOUNT = "newaccount";
    std::ofstream account_file;
  };

  account_snapshot_plugin::account_snapshot_plugin():my(new account_snapshot_plugin_impl()){}
  account_snapshot_plugin::~account_snapshot_plugin(){}

  void account_snapshot_plugin::set_program_options(options_description&, options_description& cfg)
  {
    cfg.add_options()
      ("account-snapshot-file", bpo::value<std::string>()->composing(),
       "File to append account creation information.")
      ;
  }

  void account_snapshot_plugin::plugin_initialize(const variables_map& options)
  {
    
    my->chain_plug = app().find_plugin<chain_plugin>();
    my->abi_serializer_max_time = my->chain_plug->get_abi_serializer_max_time();

    EOS_ASSERT(options.count("account-snapshot-file"), fc::invalid_arg_exception, "Missing value for --account-snapshot-file");
    // get option
    std::string fo = options.at( "account-snapshot-file" ).as<std::string>();

    my->open_log_file(fo);
    
    auto& chain = my->chain_plug->chain();

    my->applied_transaction_connection.emplace(chain.applied_transaction.connect( [&]( const transaction_trace_ptr& p ){
	  my->on_applied_transaction(p);
	}));
  }

  void account_snapshot_plugin::plugin_startup() {

  }

  void account_snapshot_plugin::plugin_shutdown()
  {
    my->cleanup();
  }

}


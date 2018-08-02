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
#include <boost/thread/mutex.hpp>

namespace eosio {
  using namespace chain;
  using boost::signals2::scoped_connection;
  static appbase::abstract_plugin& _account_snapshot_plugin = app().register_plugin<account_snapshot_plugin>();

  class account_snapshot_plugin_impl {
  public:
    chain_plugin*          chain_plug = nullptr;
    
    fc::microseconds       abi_serializer_max_time;
    fc::optional<scoped_connection> accepted_block_connection;
    fc::optional<scoped_connection> irreversible_block_connection;
    fc::optional<scoped_connection> accepted_transaction_connection;
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
      process_account_traces(trace->action_traces);
    }

    void process_account_traces( const vector<action_trace> traces)
    {
      for( const auto& atrace : traces )
      {
	on_action_trace( atrace );
	process_account_traces(atrace.inline_traces);
      }
    }
    
    void on_action_trace( const action_trace& at )
    {
      write_account(at.act);
    }
    
    void write_account(const action& act)
    {
      if (act.name == NEW_ACCOUNT) {
	const auto create = act.data_as<chain::newaccount>();
	ilog("Created ${u}", ("u",create.name));
	boost::mutex::scoped_lock lock(flock);
	account_file << create.name.to_string() << std::endl;
	account_file.flush();
      }
    }
    
  private:
    const account_name NEW_ACCOUNT = "newaccount";
    std::ofstream account_file;
    boost::mutex flock;
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
    /*
    my->accepted_block_connection.emplace( chain.accepted_block.connect( [&]( const chain::block_state_ptr& bs ) {
	  my->accepted_block( bs );
    } ));

    my->irreversible_block_connection.emplace( chain.irreversible_block.connect( [&]( const chain::block_state_ptr& bs ) {
      	  my->applied_irreversible_block( bs );
    } ));
    
    my->accepted_transaction_connection.emplace( chain.accepted_transaction.connect( [&]( const chain::transaction_metadata_ptr& t ) {
	  my->on_accepted_transaction( t );
    } ));
    */
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


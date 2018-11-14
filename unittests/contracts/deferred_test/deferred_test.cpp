/**
 *  @file
 *  @copyright defined in eos/LICENSE.txt
 */

#include <eosiolib/eosio.hpp>
#include <eosiolib/transaction.hpp>
#include <eosiolib/dispatcher.hpp>

using namespace eosio;

CONTRACT deferred_test : public contract {
   public:
      using contract::contract;
      deferred_test(name self) : contract(self, _code, _ds) {}

      struct deferfunc_args {
         uint64_t payload;
      };

      ACTION defercall( name payer, uint64_t sender_id, name contract, uint64_t payload ) {
         print( "defercall called on ", name{_self}, "\n" );
         require_auth( payer );

         print( "deferred send of deferfunc action to ", name{contract}, " by ", name{payer}, " with sender id ", sender_id );
         transaction trx;
         deferfunc_args a = {.payload = payload};
         trx.actions.emplace_back(permission_level{_self, name{"active"}}, contract, name{"deferfunc"}, a);
         trx.send( (static_cast<uint128_t>(payer.value) << 64) | sender_id, payer);
      }

      ACTION deferfunc( uint64_t payload ) {
         print("deferfunc called on ", name{_self}, " with payload = ", payload, "\n");
         eosio_assert( payload != 13, "value 13 not allowed in payload" );
      }

   private:
};

void apply_onerror(uint64_t receiver, const onerror& error ) {
   print("onerror called on ", name{receiver}, "\n");
}

extern "C" {
    /// The apply method implements the dispatch of events to this contract
    void apply( uint64_t receiver, uint64_t code, uint64_t action ) {
       if( code == name{"eosio"}.value && action == name{"onerror"}.value ) {
         apply_onerror( receiver, onerror::from_current_action() );
      } else if( code == receiver ) {
          deferred_test thiscontract(name{receiver});
         if( action == name{"defercall"}.value ) {
            execute_action( name{receiver}, name{code}, &deferred_test::defercall );
         } else if( action == name{"deferfunc"}.value ) {
            execute_action( name{receiver}, name{code}, &deferred_test::deferfunc );
         }
      }
   }
}

//EOSIO_ABI( deferred_test, (defercall)(deferfunc) )

// #include "mine.hpp"
#include <string>
#include <eosio/eosio.hpp>
#include <eosio/print.hpp>
#include <eosio/asset.hpp>
#include <eosio/system.hpp>
#include <eosio/transaction.hpp>
#include <math.h>

using namespace eosio;
using namespace std;

#define EOS_CONTRACT "eosio.token"_n
#define BOX_CONTRACT "swap.defi"_n
#define DFS_CONTRACT "defisswapcnt"_n
#define NEWDEX_CONTRACT "newdexpublic"_n
#define HBG_CONTRACT "hamburgerswp"_n
#define ROME_CONTRACT "swap.rome"_n

#define EOS_SYMBOL symbol("EOS",4)


struct token {
	name   contrat_name;
  symbol token_symbol;
};

struct ndx_symbol {
	name   contrat_name;
  symbol token_symbol;
};
struct exsymbol {
  symbol sym;
  name contract;
};
struct st_coin {
	name   contrat_name;
  symbol token_symbol;
};


class [[eosio::contract("mine")]] mine : public eosio::contract {

  public:     
    using contract::contract;

    [[eosio::action]]
    void notify(name user, std::string msg) {
      require_auth(get_self());
      require_recipient(user);
    }

    [[eosio::action]] //defibox-dfs
    void detect(int64_t time) {                        
      require_auth(get_self()); 
      auto flag =0;
      target_index targets( "youraccount"_n, "youraccount"_n.value );
      for(auto itr = targets.begin();itr != targets.end();itr++){

          pair_index pairs("swap.defi"_n,"swap.defi"_n.value);
          auto iterator_BOX = pairs.find(itr->BOX_id);
          market_index markets("defisswapcnt"_n,"defisswapcnt"_n.value);
          auto iterator_DFS = markets.find(itr->DFS_id);

          if( iterator_BOX != pairs.end() && iterator_DFS != markets.end() ){        //iterator_BOX != pairs.end() && iterator_DFS != markets.end()       
              double reserve_BOX_EOS;
              double reserve_BOX_TOKEN;
              if(iterator_BOX->reserve0.symbol == EOS_SYMBOL){
                reserve_BOX_EOS = (iterator_BOX->reserve0.amount)*0.0001;   //.amount 后类型变为uint64_t了,精度为4
                reserve_BOX_TOKEN = (iterator_BOX->reserve1.amount)*itr->token_precision;
              }
              else{
                reserve_BOX_EOS = (iterator_BOX->reserve1.amount)*0.0001;   //.amount 后类型变为uint64_t了,精度为4
                reserve_BOX_TOKEN = (iterator_BOX->reserve0.amount)*itr->token_precision;
              }
              double reserve_DFS_EOS;
              double reserve_DFS_TOKEN;
              if(iterator_DFS->reserve0.symbol == EOS_SYMBOL){
                reserve_DFS_EOS = (iterator_DFS->reserve0.amount)*0.0001;
                reserve_DFS_TOKEN = (iterator_DFS->reserve1.amount)*itr->token_precision;              
              }
              else{
                reserve_DFS_EOS = (iterator_DFS->reserve1.amount)*0.0001;
                reserve_DFS_TOKEN = (iterator_DFS->reserve0.amount)*itr->token_precision;
              }

              auto Price_BOX = reserve_BOX_EOS/reserve_BOX_TOKEN;
              auto Price_DFS = reserve_DFS_EOS/reserve_DFS_TOKEN;

              if(Price_BOX > Price_DFS ){ //Price_BOX > Price_DFS
                auto M = (sqrt(reserve_DFS_EOS*reserve_DFS_TOKEN*reserve_BOX_EOS*reserve_BOX_TOKEN) - reserve_BOX_TOKEN*reserve_DFS_EOS)/(reserve_BOX_TOKEN + reserve_DFS_TOKEN);
                auto trade_price_dfs = (reserve_DFS_EOS + M)/reserve_DFS_TOKEN;
                auto trade_price_box = reserve_BOX_EOS*(reserve_DFS_EOS + M)/(reserve_BOX_TOKEN*reserve_DFS_EOS + reserve_BOX_TOKEN*M + reserve_DFS_TOKEN*M);
                auto Profit = (trade_price_box - trade_price_dfs)*reserve_DFS_TOKEN/(reserve_DFS_EOS + M);
                if(Profit > 0.006 && M > 0.2){                  //M至少大于0.0001个，eos的最小交易单位.Profit > 0.006 && M > 0.2
                  flag =1;
                  auto order_amount = (uint64_t)(10000*M + 0.5);       //order_amount 须为uint64。四舍五入     10000*M + 0.5                         
                  action( //统计起初拥有多少EOS，存表
                      permission_level{ get_self(), name("active") },
                      get_self(), name("eosasset"),
                      std::make_tuple((uint64_t)0,name("eosio.token"), name("youraccount"),EOS_SYMBOL)
                  ).send(); 

                  action(
                      permission_level{ name("youraccount"), name("active") },
                      name("eosio.token"), name("transfer"),
                      std::make_tuple(name("youraccount"), DFS_CONTRACT, asset(order_amount,symbol("EOS",4)), itr->DFS_swap_memo)
                  ).send();

                  action(
                      permission_level{ get_self(), name("active") },
                      get_self(), name("tokenasset"),
                      std::make_tuple(itr->token_contract_address, name("youraccount"),itr->token_symbol)
                  ).send();
                
                  action(
                      permission_level{ get_self(), name("active") },
                      get_self(), name("selltoken"),
                      std::make_tuple(name("youraccount"), BOX_CONTRACT, itr->token_contract_address, itr->BOX_swap_memo)
                  ).send();                 


                  
                  action( 
                      permission_level{ get_self(), name("active") },
                      get_self(), name("eosasset"),
                      std::make_tuple((uint64_t)1,name("eosio.token"), name("youraccount"),EOS_SYMBOL)
                  ).send(); 
                  
                  action( 
                      permission_level{ get_self(), name("active") },
                      get_self(), name("judge"),
                      std::make_tuple()
                  ).send();      
                  
                  
                }
              }  

              if(Price_BOX < Price_DFS){
                auto M = (sqrt(reserve_BOX_EOS*reserve_BOX_TOKEN*reserve_DFS_EOS*reserve_DFS_TOKEN) - reserve_DFS_TOKEN*reserve_BOX_EOS)/(reserve_DFS_TOKEN + reserve_BOX_TOKEN);
                auto trade_price_box = (reserve_BOX_EOS + M)/reserve_BOX_TOKEN;
                auto trade_price_dfs = reserve_DFS_EOS*(reserve_BOX_EOS + M)/(reserve_DFS_TOKEN*reserve_BOX_EOS + reserve_DFS_TOKEN*M + reserve_BOX_TOKEN*M);
                auto Profit = (trade_price_dfs - trade_price_box)*reserve_BOX_TOKEN/(reserve_BOX_EOS + M);
                if(Profit > 0.006 && M > 0.2){   
                  flag =1;         
                  auto order_amount = (uint64_t)(10000*M + 0.5);       //order_amount 须为uint64。四舍五入 10000*M + 0.5
                  action( //统计起初拥有多少EOS，存表
                      permission_level{ get_self(), name("active") },
                      get_self(), name("eosasset"),
                      std::make_tuple((uint64_t)0,name("eosio.token"), name("youraccount"),EOS_SYMBOL)
                  ).send(); 

                  action(
                      permission_level{ name("youraccount"), name("active") },
                      name("eosio.token"), name("transfer"),
                      std::make_tuple(name("youraccount"), BOX_CONTRACT, asset(order_amount,symbol("EOS",4)), itr->BOX_swap_memo)
                  ).send();

                  action(
                      permission_level{ get_self(), name("active") },
                      get_self(), name("tokenasset"),
                      std::make_tuple(itr->token_contract_address, name("youraccount"),itr->token_symbol)
                  ).send();
                  
                  action(
                      permission_level{ get_self(), name("active") },
                      get_self(), name("selltoken"),
                      std::make_tuple(name("youraccount"), DFS_CONTRACT, itr->token_contract_address, itr->DFS_swap_memo)
                  ).send();
                  

                  action( 
                      permission_level{ get_self(), name("active") },
                      get_self(), name("eosasset"),
                      std::make_tuple((uint64_t)1,name("eosio.token"), name("youraccount"),EOS_SYMBOL)
                  ).send(); 

                  action( 
                      permission_level{ get_self(), name("active") },
                      get_self(), name("judge"),
                      std::make_tuple()
                  ).send();  

              
                }
              }     
          }
      }

     check(flag,"detect no profit,come on");
    }
    [[eosio::action]] //defibox-hamburgerswp
    void shoot(int64_t time) {                        
      require_auth(get_self()); 
      auto flag =0;
      hbgbox_index hbgboxs( "youraccount"_n, "youraccount"_n.value );
      for(auto itr = hbgboxs.begin();itr != hbgboxs.end();itr++){

          pair_index pairs("swap.defi"_n,"swap.defi"_n.value);
          auto iterator_BOX = pairs.find(itr->BOX_id);
          pairhbg_index pairhbgs("hamburgerswp"_n,"hamburgerswp"_n.value);
          auto iterator_HBG = pairhbgs.find(itr->HBG_id);

          if( iterator_BOX != pairs.end() && iterator_HBG != pairhbgs.end() ){              
              double reserve_BOX_EOS;
              double reserve_BOX_TOKEN;
              if(iterator_BOX->reserve0.symbol == EOS_SYMBOL){
                reserve_BOX_EOS = (iterator_BOX->reserve0.amount)*0.0001;   //.amount 后类型变为uint64_t了,精度为4
                reserve_BOX_TOKEN = (iterator_BOX->reserve1.amount)*itr->token_precision;
              }
              else{
                reserve_BOX_EOS = (iterator_BOX->reserve1.amount)*0.0001;   //.amount 后类型变为uint64_t了,精度为4
                reserve_BOX_TOKEN = (iterator_BOX->reserve0.amount)*itr->token_precision;
              }
              double reserve_HBG_EOS;
              double reserve_HBG_TOKEN;
              if(iterator_HBG->reserve0.symbol == EOS_SYMBOL){
                reserve_HBG_EOS = (iterator_HBG->reserve0.amount)*0.0001;
                reserve_HBG_TOKEN = (iterator_HBG->reserve1.amount)*itr->token_precision;              
              }
              else{
                reserve_HBG_EOS = (iterator_HBG->reserve1.amount)*0.0001;
                reserve_HBG_TOKEN = (iterator_HBG->reserve0.amount)*itr->token_precision;
              }

              auto Price_BOX = reserve_BOX_EOS/reserve_BOX_TOKEN;
              auto Price_HBG = reserve_HBG_EOS/reserve_HBG_TOKEN;

              if(Price_BOX > Price_HBG ){
                auto M = (sqrt(reserve_HBG_EOS*reserve_HBG_TOKEN*reserve_BOX_EOS*reserve_BOX_TOKEN) - reserve_BOX_TOKEN*reserve_HBG_EOS)/(reserve_BOX_TOKEN + reserve_HBG_TOKEN);
                auto trade_price_hbg = (reserve_HBG_EOS + M)/reserve_HBG_TOKEN;
                auto trade_price_box = reserve_BOX_EOS*(reserve_HBG_EOS + M)/(reserve_BOX_TOKEN*reserve_HBG_EOS + reserve_BOX_TOKEN*M + reserve_HBG_TOKEN*M);
                auto Profit = (trade_price_box - trade_price_hbg)*reserve_HBG_TOKEN/(reserve_HBG_EOS + M);
                if(Profit > 0.006 && M > 0.2){                  //M至少大于0.0001个，eos的最小交易单位.Profit > 0.001 && M > 0.3
                  flag =1;
                  auto order_amount = (uint64_t)(10000*M + 0.5);       //order_amount 须为uint64。四舍五入                             
                  action( 
                      permission_level{ get_self(), name("active") },
                      get_self(), name("eosasset"),
                      std::make_tuple((uint64_t)0,name("eosio.token"), name("youraccount"),EOS_SYMBOL)
                  ).send(); 

                  action(
                      permission_level{ name("youraccount"), name("active") },
                      name("eosio.token"), name("transfer"),
                      std::make_tuple(name("youraccount"), HBG_CONTRACT, asset(order_amount,symbol("EOS",4)), itr->HBG_swap_memo)
                  ).send();

                  action(
                      permission_level{ get_self(), name("active") },
                      get_self(), name("tokenasset"),
                      std::make_tuple(itr->token_contract_address, name("youraccount"),itr->token_symbol)
                  ).send();
                
                  action(
                      permission_level{ get_self(), name("active") },
                      get_self(), name("selltoken"),
                      std::make_tuple(name("youraccount"), BOX_CONTRACT, itr->token_contract_address, itr->BOX_swap_memo)
                  ).send();    

                  action( 
                      permission_level{ get_self(), name("active") },
                      get_self(), name("eosasset"),
                      std::make_tuple((uint64_t)1,name("eosio.token"), name("youraccount"),EOS_SYMBOL)
                  ).send(); 

                  action( 
                      permission_level{ get_self(), name("active") },
                      get_self(), name("judge"),
                      std::make_tuple()
                  ).send();                
                  
                  swaplog("youraccount"_n,"swaplog id=" + std::to_string(itr->id) + " Profit=" + std::to_string(Profit) + "  M = " + std::to_string(M) + "  order_amount = " + std::to_string(order_amount));
                  
                }
              }  

              if(Price_BOX < Price_HBG ){
                auto M = (sqrt(reserve_BOX_EOS*reserve_BOX_TOKEN*reserve_HBG_EOS*reserve_HBG_TOKEN) - reserve_HBG_TOKEN*reserve_BOX_EOS)/(reserve_HBG_TOKEN + reserve_BOX_TOKEN);
                auto trade_price_box = (reserve_BOX_EOS + M)/reserve_BOX_TOKEN;
                auto trade_price_hbg = reserve_HBG_EOS*(reserve_BOX_EOS + M)/(reserve_HBG_TOKEN*reserve_BOX_EOS + reserve_HBG_TOKEN*M + reserve_BOX_TOKEN*M);
                auto Profit = (trade_price_hbg - trade_price_box)*reserve_BOX_TOKEN/(reserve_BOX_EOS + M);
                if(Profit > 0.006 && M > 0.2){  
                  flag =1;         
                  auto order_amount = (uint64_t)(10000*M + 0.5);       //order_amount 须为uint64。四舍五入
                  action( //统计起初拥有多少EOS，存表
                      permission_level{ get_self(), name("active") },
                      get_self(), name("eosasset"),
                      std::make_tuple((uint64_t)0,name("eosio.token"), name("youraccount"),EOS_SYMBOL)
                  ).send(); 

                  action(
                      permission_level{ name("youraccount"), name("active") },
                      name("eosio.token"), name("transfer"),
                      std::make_tuple(name("youraccount"), BOX_CONTRACT, asset(order_amount,symbol("EOS",4)), itr->BOX_swap_memo)
                  ).send();

                  action(
                      permission_level{ get_self(), name("active") },
                      get_self(), name("tokenasset"),
                      std::make_tuple(itr->token_contract_address, name("youraccount"),itr->token_symbol)
                  ).send();
                  
                  action(
                      permission_level{ get_self(), name("active") },
                      get_self(), name("selltoken"),
                      std::make_tuple(name("youraccount"), HBG_CONTRACT, itr->token_contract_address, itr->HBG_swap_memo)
                  ).send();

                  action( 
                      permission_level{ get_self(), name("active") },
                      get_self(), name("eosasset"),
                      std::make_tuple((uint64_t)1,name("eosio.token"), name("youraccount"),EOS_SYMBOL)
                  ).send(); 

                  action( 
                      permission_level{ get_self(), name("active") },
                      get_self(), name("judge"),
                      std::make_tuple()
                  ).send();   

                  swaplog("youraccount"_n,"swaplog id=" + std::to_string(itr->id) + " Profit=" + std::to_string(Profit) + "  M = " + std::to_string(M) + "  order_amount = " + std::to_string(order_amount));
              
                }
              }     
          }
      }

     check(flag,"hbgbox no profit,come on");
    }

    [[eosio::action]] //defibox-newdex
    void boom(int64_t time){
      require_auth(get_self()); 
      auto flag =0;
      boxnew_index boxnews( get_self(), get_first_receiver().value );
      for(auto itrboxnew = boxnews.begin();itrboxnew != boxnews.end();itrboxnew++){
          pair_index pairs("swap.defi"_n,"swap.defi"_n.value);
          auto iterator_BOX = pairs.find(itrboxnew->BOX_id);
          exchangepair_index exchangepair("newdexpublic"_n,"newdexpublic"_n.value);
          auto iterator_NEWDEX = exchangepair.find(itrboxnew->NEWDEX_id);
          if( iterator_BOX != pairs.end() && iterator_NEWDEX != exchangepair.end() ){

              double reserve_BOX_EOS;
              double reserve_BOX_TOKEN;
              if(iterator_BOX->reserve0.symbol == EOS_SYMBOL){
                reserve_BOX_EOS = (iterator_BOX->reserve0.amount)*0.0001;   //.amount 后类型变为uint64_t了,精度为4
                reserve_BOX_TOKEN = (iterator_BOX->reserve1.amount)*itrboxnew->token_precision;
              }
              else{
                reserve_BOX_EOS = (iterator_BOX->reserve1.amount)*0.0001;   //.amount 后类型变为uint64_t了,精度为4
                reserve_BOX_TOKEN = (iterator_BOX->reserve0.amount)*itrboxnew->token_precision;
              }
              auto Price_BOX = reserve_BOX_EOS/reserve_BOX_TOKEN;

              auto buyprice_NEWDEX = buyprice(itrboxnew->NEWDEX_id);
              auto sellprice_NEWDEX = sellprice(itrboxnew->NEWDEX_id);
              auto buyamount_NEWDEX = buyamount(itrboxnew->NEWDEX_id)*0.0001;  //0.0001是newdex挂单簿上的精度
              auto sellamount_NEWDEX = sellamount(itrboxnew->NEWDEX_id)*0.0001;
              if(Price_BOX < buyprice_NEWDEX){ 
                auto M = sqrt(reserve_BOX_EOS*reserve_BOX_TOKEN*buyprice_NEWDEX) - reserve_BOX_EOS;
                if( M > buyamount_NEWDEX ){ M = buyamount_NEWDEX;}
                auto trade_price_box = (reserve_BOX_EOS+M)/reserve_BOX_TOKEN;
                auto Profit = (buyprice_NEWDEX - trade_price_box)*reserve_BOX_TOKEN/(reserve_BOX_EOS + M);
                if(Profit > 0.005 && M > 0.2){
                  flag =2;
                  //开始交易，BOX买入M,在NEWDEX卖，Profit > 0.001 && M > 0.3
                  auto order_amount = (uint64_t)(10000*M + 0.5);       //order_amount 须为uint64。四舍五入
                  action( //
                      permission_level{ get_self(), name("active") },
                      get_self(), name("eosasset"),
                      std::make_tuple((uint64_t)0,name("eosio.token"), name("youraccount"),EOS_SYMBOL)
                  ).send(); 

                  action(
                      permission_level{ name("youraccount"), name("active") },
                      name("eosio.token"), name("transfer"),
                      std::make_tuple(name("youraccount"), BOX_CONTRACT, asset(order_amount,symbol("EOS",4)), itrboxnew->BOX_swap_memo)
                  ).send();

                  action(
                      permission_level{ get_self(), name("active") },
                      get_self(), name("tokenasset"),
                      std::make_tuple(itrboxnew->token_contract_address, name("youraccount"),itrboxnew->token_symbol)
                  ).send();
                
                  action(
                      permission_level{ get_self(), name("active") },
                      get_self(), name("selltoken"),
                      std::make_tuple(name("youraccount"), NEWDEX_CONTRACT, itrboxnew->token_contract_address, itrboxnew->NEWDEX_sell_memo)
                  ).send(); 

                  action( //
                      permission_level{ get_self(), name("active") },
                      get_self(), name("eosasset"),
                      std::make_tuple((uint64_t)1,name("eosio.token"), name("youraccount"),EOS_SYMBOL)
                  ).send(); 

                  action( 
                      permission_level{ get_self(), name("active") },
                      get_self(), name("judge"),
                      std::make_tuple()
                  ).send();                   
                  
                  swaplog("youraccount"_n,"swaplog id=" + std::to_string(itrboxnew->id) + " Profit=" + std::to_string(Profit) + "  M = " + std::to_string(M) + "  order_amount = " + std::to_string(order_amount));

                  
                }

              }
              if(Price_BOX > sellprice_NEWDEX){
                auto M = sqrt(reserve_BOX_EOS*reserve_BOX_TOKEN*sellprice_NEWDEX) - reserve_BOX_TOKEN*sellprice_NEWDEX;
                if( M > sellamount_NEWDEX ){ M = sellamount_NEWDEX;}
                auto trade_price_box = reserve_BOX_EOS*sellprice_NEWDEX/(reserve_BOX_TOKEN*sellprice_NEWDEX + M);
                auto Profit = (trade_price_box - sellprice_NEWDEX)/sellprice_NEWDEX;
                if(Profit > 0.005 && M >0.2 ){ //Profit > 0.001 && M >0.3
                  //开始交易，在NEWDEX买入M个eos的token,在box卖出
                  flag=2;
                  auto order_amount = (uint64_t)(10000*M + 0.5);       //order_amount 须为uint64。四舍五入
                  action( //
                      permission_level{ get_self(), name("active") },
                      get_self(), name("eosasset"),
                      std::make_tuple((uint64_t)0,name("eosio.token"), name("youraccount"),EOS_SYMBOL)
                  ).send(); 
                  
                  action(
                      permission_level{ name("youraccount"), name("active") },
                      name("eosio.token"), name("transfer"),
                      std::make_tuple(name("youraccount"), NEWDEX_CONTRACT, asset(order_amount,symbol("EOS",4)), itrboxnew->NEWDEX_buy_memo)
                  ).send();

                  action(
                      permission_level{ get_self(), name("active") },
                      get_self(), name("tokenasset"),
                      std::make_tuple(itrboxnew->token_contract_address, name("youraccount"),itrboxnew->token_symbol)
                  ).send();
                  
                  action(
                      permission_level{ get_self(), name("active") },
                      get_self(), name("selltoken"),
                      std::make_tuple(name("youraccount"), BOX_CONTRACT, itrboxnew->token_contract_address, itrboxnew->BOX_swap_memo)
                  ).send();

                  action( //
                      permission_level{ get_self(), name("active") },
                      get_self(), name("eosasset"),
                      std::make_tuple((uint64_t)1,name("eosio.token"), name("youraccount"),EOS_SYMBOL)
                  ).send();                    
                  action( 
                      permission_level{ get_self(), name("active") },
                      get_self(), name("judge"),
                      std::make_tuple()
                  ).send();   

                  swaplog("youraccount"_n,"swaplog id=" + std::to_string(itrboxnew->id) + " Profit=" + std::to_string(Profit) + "  M = " + std::to_string(M) + "  order_amount = " + std::to_string(order_amount));
                }        
              }
          }
      } 


      check(flag,"boom no profit,come on");

    }

    [[eosio::action]] //dfs-newdex
    void peace(int64_t time){
      require_auth(get_self()); 
      auto flag =0;
      dfsnew_index dfsnews( get_self(), get_first_receiver().value );
      for(auto itrdfsnew = dfsnews.begin();itrdfsnew != dfsnews.end();itrdfsnew++){
          market_index markets("defisswapcnt"_n,"defisswapcnt"_n.value);
          auto iterator_DFS = markets.find(itrdfsnew->DFS_id);
          exchangepair_index exchangepair("newdexpublic"_n,"newdexpublic"_n.value);
          auto iterator_NEWDEX = exchangepair.find(itrdfsnew->NEWDEX_id);
          if( iterator_DFS != markets.end() && iterator_NEWDEX != exchangepair.end() ){

              double reserve_DFS_EOS;
              double reserve_DFS_TOKEN;
              if(iterator_DFS->reserve0.symbol == EOS_SYMBOL){
                reserve_DFS_EOS = (iterator_DFS->reserve0.amount)*0.0001;   //.amount 后类型变为uint64_t了,精度为4
                reserve_DFS_TOKEN = (iterator_DFS->reserve1.amount)*itrdfsnew->token_precision;
              }
              else{
                reserve_DFS_EOS = (iterator_DFS->reserve1.amount)*0.0001;   //.amount 后类型变为uint64_t了,精度为4
                reserve_DFS_TOKEN = (iterator_DFS->reserve0.amount)*itrdfsnew->token_precision;
              }
              auto Price_DFS = reserve_DFS_EOS/reserve_DFS_TOKEN;

              auto buyprice_NEWDEX = buyprice(itrdfsnew->NEWDEX_id);
              auto sellprice_NEWDEX = sellprice(itrdfsnew->NEWDEX_id);
              auto buyamount_NEWDEX = buyamount(itrdfsnew->NEWDEX_id)*0.0001;    //0.001是newdex挂单簿上的精度
              auto sellamount_NEWDEX = sellamount(itrdfsnew->NEWDEX_id)*0.0001;
              if(Price_DFS < buyprice_NEWDEX){ 
                auto M = sqrt(reserve_DFS_EOS*reserve_DFS_TOKEN*buyprice_NEWDEX) - reserve_DFS_EOS;
                if( M > buyamount_NEWDEX ){ M = buyamount_NEWDEX;}
                auto trade_price_dfs = (reserve_DFS_EOS+M)/reserve_DFS_TOKEN;
                auto Profit = (buyprice_NEWDEX - trade_price_dfs)*reserve_DFS_TOKEN/(reserve_DFS_EOS + M);
                //swaplog("youraccount"_n,"test-------swaplog id=" + std::to_string(itrdfsnew->id) + " Profit=" + std::to_string(Profit) + "  M = " + std::to_string(M));

                if(Profit > 0.005 && M > 0.2){
                  flag =2;
                  //开始交易，DFS买入M,在NEWDEX卖，Profit > 0.001 && M > 0.3
                  auto order_amount = (uint64_t)(10000*M + 0.5);       //order_amount 须为uint64。四舍五入
                  action( //
                      permission_level{ get_self(), name("active") },
                      get_self(), name("eosasset"),
                      std::make_tuple((uint64_t)0,name("eosio.token"), name("youraccount"),EOS_SYMBOL)
                  ).send(); 

                  action(
                      permission_level{ name("youraccount"), name("active") },
                      name("eosio.token"), name("transfer"),
                      std::make_tuple(name("youraccount"), DFS_CONTRACT, asset(order_amount,symbol("EOS",4)), itrdfsnew->DFS_swap_memo)
                  ).send();

                  action(
                      permission_level{ get_self(), name("active") },
                      get_self(), name("tokenasset"),
                      std::make_tuple(itrdfsnew->token_contract_address, name("youraccount"),itrdfsnew->token_symbol)
                  ).send();
                
                  action(
                      permission_level{ get_self(), name("active") },
                      get_self(), name("selltoken"),
                      std::make_tuple(name("youraccount"), NEWDEX_CONTRACT, itrdfsnew->token_contract_address, itrdfsnew->NEWDEX_sell_memo)
                  ).send();             

                  action( //
                      permission_level{ get_self(), name("active") },
                      get_self(), name("eosasset"),
                      std::make_tuple((uint64_t)1,name("eosio.token"), name("youraccount"),EOS_SYMBOL)
                  ).send(); 
                   
                  action( 
                      permission_level{ get_self(), name("active") },
                      get_self(), name("judge"),
                      std::make_tuple()
                  ).send();       
                  

                  
                }

              }
              if(Price_DFS > sellprice_NEWDEX){
                auto M = sqrt(reserve_DFS_EOS*reserve_DFS_TOKEN*sellprice_NEWDEX) - reserve_DFS_TOKEN*sellprice_NEWDEX;
                if( M > sellamount_NEWDEX ){ M = sellamount_NEWDEX;}
                auto trade_price_dfs = reserve_DFS_EOS*sellprice_NEWDEX/(reserve_DFS_TOKEN*sellprice_NEWDEX + M);
                auto Profit = (trade_price_dfs - sellprice_NEWDEX)/sellprice_NEWDEX;
                //swaplog("youraccount"_n,"test-------swaplog id=" + std::to_string(itrdfsnew->id) + " Profit=" + std::to_string(Profit) + "  M = " + std::to_string(M) );

                if(Profit > 0.0050 && M > 0.2 ){ //Profit > 0.001 && M >0.3
                  //开始交易，在NEWDEX买入M个eos的token,在dfs卖出
                  flag=2;
                  auto order_amount = (uint64_t)(10000*M + 0.5);       //order_amount 须为uint64。四舍五入
                  action( //统计起初拥有多少EOS，存表
                      permission_level{ get_self(), name("active") },
                      get_self(), name("eosasset"),
                      std::make_tuple((uint64_t)0,name("eosio.token"), name("youraccount"),EOS_SYMBOL)
                  ).send(); 
                  action(
                      permission_level{ name("youraccount"), name("active") },
                      name("eosio.token"), name("transfer"),
                      std::make_tuple(name("youraccount"), NEWDEX_CONTRACT, asset(order_amount,symbol("EOS",4)), itrdfsnew->NEWDEX_buy_memo)
                  ).send();

                  action(
                      permission_level{ get_self(), name("active") },
                      get_self(), name("tokenasset"),
                      std::make_tuple(itrdfsnew->token_contract_address, name("youraccount"),itrdfsnew->token_symbol)
                  ).send();
                  
                  action(
                      permission_level{ get_self(), name("active") },
                      get_self(), name("selltoken"),
                      std::make_tuple(name("youraccount"), DFS_CONTRACT, itrdfsnew->token_contract_address, itrdfsnew->DFS_swap_memo)
                  ).send();

                  action( 
                      permission_level{ get_self(), name("active") },
                      get_self(), name("eosasset"),
                      std::make_tuple((uint64_t)1,name("eosio.token"), name("youraccount"),EOS_SYMBOL)
                  ).send(); 

                  action( 
                      permission_level{ get_self(), name("active") },
                      get_self(), name("judge"),
                      std::make_tuple()
                  ).send();   

                  swaplog("youraccount"_n,"swaplog id=" + std::to_string(itrdfsnew->id) + " Profit=" + std::to_string(Profit) + "  M = " + std::to_string(M) + "  order_amount = " + std::to_string(order_amount));
                }        
              }
          }
      } 


      check(flag,"peace no profit,come on");

    }

    [[eosio::action]] //defibox-romedex
    void rome(int64_t time) {                        
      require_auth(get_self()); 
      auto flag =0;
      romebox_index romeboxs( "youraccount"_n, "youraccount"_n.value );
      for(auto itr = romeboxs.begin();itr != romeboxs.end();itr++){

          pair_index pairs("swap.defi"_n,"swap.defi"_n.value);
          auto iterator_BOX = pairs.find(itr->BOX_id);
          romemarket_index romemarkets("swap.rome"_n,"swap.rome"_n.value);
          auto iterator_ROME = romemarkets.find(itr->ROME_id);

          if( iterator_BOX != pairs.end() && iterator_ROME != romemarkets.end() ){              
              double reserve_BOX_EOS;
              double reserve_BOX_TOKEN;
              if(iterator_BOX->reserve0.symbol == EOS_SYMBOL){
                reserve_BOX_EOS = (iterator_BOX->reserve0.amount)*0.0001;   //.amount 后类型变为uint64_t了,精度为4
                reserve_BOX_TOKEN = (iterator_BOX->reserve1.amount)*itr->token_precision;
              }
              else{
                reserve_BOX_EOS = (iterator_BOX->reserve1.amount)*0.0001;   //.amount 后类型变为uint64_t了,精度为4
                reserve_BOX_TOKEN = (iterator_BOX->reserve0.amount)*itr->token_precision;
              }
              double reserve_ROME_EOS;
              double reserve_ROME_TOKEN;
              if(iterator_ROME->reserve0.symbol == EOS_SYMBOL){
                reserve_ROME_EOS = (iterator_ROME->reserve0.amount)*0.0001;
                reserve_ROME_TOKEN = (iterator_ROME->reserve1.amount)*itr->token_precision;              
              }
              else{
                reserve_ROME_EOS = (iterator_ROME->reserve1.amount)*0.0001;
                reserve_ROME_TOKEN = (iterator_ROME->reserve0.amount)*itr->token_precision;
              }

              auto Price_BOX = reserve_BOX_EOS/reserve_BOX_TOKEN;
              auto Price_ROME = reserve_ROME_EOS/reserve_ROME_TOKEN;

              if(Price_BOX > Price_ROME ){
                auto M = (sqrt(reserve_ROME_EOS*reserve_ROME_TOKEN*reserve_BOX_EOS*reserve_BOX_TOKEN) - reserve_BOX_TOKEN*reserve_ROME_EOS)/(reserve_BOX_TOKEN + reserve_ROME_TOKEN);
                auto trade_price_rome = (reserve_ROME_EOS + M)/reserve_ROME_TOKEN;
                auto trade_price_box = reserve_BOX_EOS*(reserve_ROME_EOS + M)/(reserve_BOX_TOKEN*reserve_ROME_EOS + reserve_BOX_TOKEN*M + reserve_ROME_TOKEN*M);
                auto Profit = (trade_price_box - trade_price_rome)*reserve_ROME_TOKEN/(reserve_ROME_EOS + M);
                if(Profit > 0.006 && M > 0.2){                  //M至少大于0.0001个，eos的最小交易单位.Profit > 0.001 && M > 0.3
                  flag =1;
                  auto order_amount = (uint64_t)(10000*M + 0.5);       //order_amount 须为uint64。四舍五入                             
                  action( 
                      permission_level{ get_self(), name("active") },
                      get_self(), name("eosasset"),
                      std::make_tuple((uint64_t)0,name("eosio.token"), name("youraccount"),EOS_SYMBOL)
                  ).send(); 

                  action(
                      permission_level{ name("youraccount"), name("active") },
                      name("eosio.token"), name("transfer"),
                      std::make_tuple(name("youraccount"), ROME_CONTRACT, asset(order_amount,symbol("EOS",4)), itr->ROME_swap_memo)
                  ).send();

                  action(
                      permission_level{ get_self(), name("active") },
                      get_self(), name("tokenasset"),
                      std::make_tuple(itr->token_contract_address, name("youraccount"),itr->token_symbol)
                  ).send();
                
                  action(
                      permission_level{ get_self(), name("active") },
                      get_self(), name("selltoken"),
                      std::make_tuple(name("youraccount"), BOX_CONTRACT, itr->token_contract_address, itr->BOX_swap_memo)
                  ).send();    

                  action( //
                      permission_level{ get_self(), name("active") },
                      get_self(), name("eosasset"),
                      std::make_tuple((uint64_t)1,name("eosio.token"), name("youraccount"),EOS_SYMBOL)
                  ).send(); 

                   
                  action( 
                      permission_level{ get_self(), name("active") },
                      get_self(), name("judge"),
                      std::make_tuple()
                  ).send();                
                  
                  swaplog("youraccount"_n,"swaplog id=" + std::to_string(itr->id) + " Profit=" + std::to_string(Profit) + "  M = " + std::to_string(M) + "  order_amount = " + std::to_string(order_amount));
                  
                }
              }  

              if(Price_BOX < Price_ROME ){
                auto M = (sqrt(reserve_BOX_EOS*reserve_BOX_TOKEN*reserve_ROME_EOS*reserve_ROME_TOKEN) - reserve_ROME_TOKEN*reserve_BOX_EOS)/(reserve_ROME_TOKEN + reserve_BOX_TOKEN);
                auto trade_price_box = (reserve_BOX_EOS + M)/reserve_BOX_TOKEN;
                auto trade_price_rome = reserve_ROME_EOS*(reserve_BOX_EOS + M)/(reserve_ROME_TOKEN*reserve_BOX_EOS + reserve_ROME_TOKEN*M + reserve_BOX_TOKEN*M);
                auto Profit = (trade_price_rome - trade_price_box)*reserve_BOX_TOKEN/(reserve_BOX_EOS + M);
                if(Profit > 0.006 && M > 0.2){  
                  flag =1;         
                  auto order_amount = (uint64_t)(10000*M + 0.5);       //order_amount 须为uint64。四舍五入
                  action( //
                      permission_level{ get_self(), name("active") },
                      get_self(), name("eosasset"),
                      std::make_tuple((uint64_t)0,name("eosio.token"), name("youraccount"),EOS_SYMBOL)
                  ).send(); 

                  action(
                      permission_level{ name("youraccount"), name("active") },
                      name("eosio.token"), name("transfer"),
                      std::make_tuple(name("youraccount"), BOX_CONTRACT, asset(order_amount,symbol("EOS",4)), itr->BOX_swap_memo)
                  ).send();

                  action(
                      permission_level{ get_self(), name("active") },
                      get_self(), name("tokenasset"),
                      std::make_tuple(itr->token_contract_address, name("youraccount"),itr->token_symbol)
                  ).send();
                  
                  action(
                      permission_level{ get_self(), name("active") },
                      get_self(), name("selltoken"),
                      std::make_tuple(name("youraccount"), ROME_CONTRACT, itr->token_contract_address, itr->ROME_swap_memo)
                  ).send();

                  action( //
                      permission_level{ get_self(), name("active") },
                      get_self(), name("eosasset"),
                      std::make_tuple((uint64_t)1,name("eosio.token"), name("youraccount"),EOS_SYMBOL)
                  ).send(); 

                  action( 
                      permission_level{ get_self(), name("active") },
                      get_self(), name("judge"),
                      std::make_tuple()
                  ).send();   

                  swaplog("youraccount"_n,"swaplog id=" + std::to_string(itr->id) + " Profit=" + std::to_string(Profit) + "  M = " + std::to_string(M) + "  order_amount = " + std::to_string(order_amount));
              
                }
              }     
          }
      }

     check(flag,"romedex no profit,come on");
    }

    [[eosio::action]]
    void judge(){
      require_auth(get_self());
      eosleft_index eoslefts(get_self(),get_first_receiver().value);
      auto eos_before = eoslefts.find(0);
      auto eos_after = eoslefts.find(1);
      check(eos_after->balance.amount > eos_before->balance.amount,"this is not a success arbitrage,come on");
      swaplog("youraccount"_n,"eosbefore = " + std::to_string(eos_before->balance.amount) + " eosafter = " + std::to_string(eos_after->balance.amount) );

    }

      
    [[eosio::action]]     // '["0","10,DMD","0.0000000001","eosdmdtokens","472","swap,0,472","326","swap:326:0:8"]'
    void upserttarget(uint64_t id, symbol token_symbol, double token_precision,name token_contract_address, uint64_t BOX_id, std::string BOX_swap_memo,uint64_t DFS_id, std::string DFS_swap_memo ) {
      require_auth( get_self() );
      target_index targets( get_self(), get_first_receiver().value );
      auto iterator = targets.find(id);
      if( iterator == targets.end() )
      {
        targets.emplace(get_self(), [&]( auto& row ) {         //emplace( name payer, Lambda&& constructor ) 
        row.id = id;
        row.token_symbol = token_symbol;
        row.token_precision = token_precision;
        row.token_contract_address = token_contract_address;
        row.BOX_id = BOX_id;              
        row.BOX_swap_memo = BOX_swap_memo;
        row.DFS_id = DFS_id;
        row.DFS_swap_memo = DFS_swap_memo;      
        });
      }
      else {
        targets.modify(iterator, get_self(), [&]( auto& row ) {    //modify( const T& obj, name payer, Lambda&& updater )
        row.id = id;
        row.token_symbol = token_symbol;
        row.token_precision = token_precision;
        row.token_contract_address = token_contract_address;
        row.BOX_id = BOX_id;              
        row.BOX_swap_memo = BOX_swap_memo;
        row.DFS_id = DFS_id;
        row.DFS_swap_memo = DFS_swap_memo;
        });
      }
    }

    [[eosio::action]]     // '["0","10,DMD","0.0000000001","eosdmdtokens","472","swap,0,472","326","swap:326:0:8"]'
    void upserthbgbox(uint64_t id, symbol token_symbol, double token_precision,name token_contract_address, uint64_t BOX_id, std::string BOX_swap_memo,uint64_t HBG_id, std::string HBG_swap_memo ) {
      require_auth( get_self() );
      hbgbox_index hbgboxs( get_self(), get_first_receiver().value );
      auto iterator = hbgboxs.find(id);
      if( iterator == hbgboxs.end() )
      {
        hbgboxs.emplace(get_self(), [&]( auto& row ) {         //emplace( name payer, Lambda&& constructor ) 
        row.id = id;
        row.token_symbol = token_symbol;
        row.token_precision = token_precision;
        row.token_contract_address = token_contract_address;
        row.BOX_id = BOX_id;              
        row.BOX_swap_memo = BOX_swap_memo;
        row.HBG_id = HBG_id;
        row.HBG_swap_memo = HBG_swap_memo;      
        });
      }
      else {
        hbgboxs.modify(iterator, get_self(), [&]( auto& row ) {    //modify( const T& obj, name payer, Lambda&& updater )
        row.id = id;
        row.token_symbol = token_symbol;
        row.token_precision = token_precision;
        row.token_contract_address = token_contract_address;
        row.BOX_id = BOX_id;              
        row.BOX_swap_memo = BOX_swap_memo;
        row.HBG_id = HBG_id;
        row.HBG_swap_memo = HBG_swap_memo;
        });
      }
    }

    [[eosio::action]]
    void upsertboxnew(uint64_t id, symbol token_symbol, double token_precision,name token_contract_address, uint64_t BOX_id, std::string BOX_swap_memo,uint64_t NEWDEX_id, std::string NEWDEX_buy_memo, std::string NEWDEX_sell_memo ) {
      require_auth( get_self() );
      boxnew_index boxnews( get_self(), get_first_receiver().value );
      auto iterator = boxnews.find(id);
      if( iterator == boxnews.end() )
      {
        boxnews.emplace(get_self(), [&]( auto& row ) {         //emplace( name payer, Lambda&& constructor ) 
        row.id = id;
        row.token_symbol = token_symbol;
        row.token_precision = token_precision;
        row.token_contract_address = token_contract_address;
        row.BOX_id = BOX_id;              
        row.BOX_swap_memo = BOX_swap_memo;
        row.NEWDEX_id = NEWDEX_id;
        row.NEWDEX_buy_memo = NEWDEX_buy_memo; 
        row.NEWDEX_sell_memo = NEWDEX_sell_memo;    
        });
      }
      else {
        boxnews.modify(iterator, get_self(), [&]( auto& row ) {    //modify( const T& obj, name payer, Lambda&& updater )
        row.id = id;
        row.token_symbol = token_symbol;
        row.token_precision = token_precision;
        row.token_contract_address = token_contract_address;
        row.BOX_id = BOX_id;              
        row.BOX_swap_memo = BOX_swap_memo;
        row.NEWDEX_id = NEWDEX_id;
        row.NEWDEX_buy_memo = NEWDEX_buy_memo; 
        row.NEWDEX_sell_memo = NEWDEX_sell_memo; 
        });
      }
    }

    [[eosio::action]]
    void upsertdfsnew(uint64_t id, symbol token_symbol, double token_precision,name token_contract_address, uint64_t DFS_id, std::string DFS_swap_memo,uint64_t NEWDEX_id, std::string NEWDEX_buy_memo, std::string NEWDEX_sell_memo ) {
      require_auth( get_self() );
      dfsnew_index dfsnews( get_self(), get_first_receiver().value );
      auto iterator = dfsnews.find(id);
      if( iterator == dfsnews.end() )
      {
        dfsnews.emplace(get_self(), [&]( auto& row ) {         //emplace( name payer, Lambda&& constructor ) 
        row.id = id;
        row.token_symbol = token_symbol;
        row.token_precision = token_precision;
        row.token_contract_address = token_contract_address;
        row.DFS_id = DFS_id;              
        row.DFS_swap_memo = DFS_swap_memo;
        row.NEWDEX_id = NEWDEX_id;
        row.NEWDEX_buy_memo = NEWDEX_buy_memo; 
        row.NEWDEX_sell_memo = NEWDEX_sell_memo;    
        });
      }
      else {
        dfsnews.modify(iterator, get_self(), [&]( auto& row ) {    //modify( const T& obj, name payer, Lambda&& updater )
        row.id = id;
        row.token_symbol = token_symbol;
        row.token_precision = token_precision;
        row.token_contract_address = token_contract_address;
        row.DFS_id = DFS_id;              
        row.DFS_swap_memo = DFS_swap_memo;
        row.NEWDEX_id = NEWDEX_id;
        row.NEWDEX_buy_memo = NEWDEX_buy_memo; 
        row.NEWDEX_sell_memo = NEWDEX_sell_memo; 
        });
      }
    }

    [[eosio::action]]     // '["0","10,DMD","0.0000000001","eosdmdtokens","472","swap,0,472","326","swap:326:0:8"]'
    void upsertroebox(uint64_t id, symbol token_symbol, double token_precision,name token_contract_address, uint64_t BOX_id, std::string BOX_swap_memo,uint64_t ROME_id, std::string ROME_swap_memo ) {
      require_auth( get_self() );
      romebox_index romeboxs( get_self(), get_first_receiver().value );
      auto iterator = romeboxs.find(id);
      if( iterator == romeboxs.end() )
      {
        romeboxs.emplace(get_self(), [&]( auto& row ) {         //emplace( name payer, Lambda&& constructor ) 
        row.id = id;
        row.token_symbol = token_symbol;
        row.token_precision = token_precision;
        row.token_contract_address = token_contract_address;
        row.BOX_id = BOX_id;              
        row.BOX_swap_memo = BOX_swap_memo;
        row.ROME_id = ROME_id;
        row.ROME_swap_memo = ROME_swap_memo;      
        });
      }
      else {
        romeboxs.modify(iterator, get_self(), [&]( auto& row ) {    //modify( const T& obj, name payer, Lambda&& updater )
        row.id = id;
        row.token_symbol = token_symbol;
        row.token_precision = token_precision;
        row.token_contract_address = token_contract_address;
        row.BOX_id = BOX_id;              
        row.BOX_swap_memo = BOX_swap_memo;
        row.ROME_id = ROME_id;
        row.ROME_swap_memo = ROME_swap_memo;
        });
      }
    }


    [[eosio::action]]
    void erasetarget(uint64_t id) {
      require_auth(get_self());

      target_index targets( get_self(), get_first_receiver().value);

      auto iterator = targets.find(id);
      check(iterator != targets.end(), "Record does not exist");
      targets.erase(iterator);
    }

    [[eosio::action]]
    void eraseboxnew(uint64_t id) {
      require_auth(get_self());

      boxnew_index boxnews( get_self(), get_first_receiver().value);

      auto iterator = boxnews.find(id);
      check(iterator != boxnews.end(), "Record does not exist");
      boxnews.erase(iterator);
    }

    

    [[eosio::action]]
    void erasedfsnew(uint64_t id) {
      require_auth(get_self());

      dfsnew_index dfsnews( get_self(), get_first_receiver().value);

      auto iterator = dfsnews.find(id);
      check(iterator != dfsnews.end(), "Record does not exist");
      dfsnews.erase(iterator);
    }

    [[eosio::action]]
    void eraseromebox(uint64_t id) {
      require_auth(get_self());

      romebox_index romeboxs( get_self(), get_first_receiver().value);

      auto iterator = romeboxs.find(id);
      check(iterator != romeboxs.end(), "Record does not exist");
      romeboxs.erase(iterator);
    }


    [[eosio::action]]
    void getdata(uint64_t pair_id){
      require_auth(get_self());
      auto buy_price = buyprice(pair_id);
      auto buy_amount = buyamount(pair_id);
      auto sell_price = sellprice(pair_id);
      auto sell_amount = sellamount(pair_id);
      swaplog("youraccount"_n,"buyprice=" + std::to_string(buy_price) + " buyamount=" + std::to_string(buy_amount) + " sellprice " + std::to_string(sell_price) + "  sellamount = " + std::to_string(sell_amount));
    }

    [[eosio::action]]
    void selltoken(  const name& from,const name& dex_contract_address,const name& token_contract_address, const std::string& dex_swap_memo ){
      require_auth(get_self());
      tokenleft_index tokenlefts(get_self(),get_first_receiver().value);
      auto token_itr = tokenlefts.find(1); 
      check(token_itr != tokenlefts.end(),"the table tokenlefts have no data");
      action(
          permission_level{ name("youraccount"), name("active") },
          token_contract_address, name("transfer"),
          std::make_tuple(from, dex_contract_address, token_itr->balance, dex_swap_memo)
      ).send();
    swaplog("youraccount"_n,"token.symbol=" + std::to_string(token_itr->balance.symbol.code().raw()) + " token.amount=" + std::to_string(token_itr->balance.amount) );
    }

    [[eosio::action]]
    void tokenasset(  const name& token_contract_account, const name& owner, const symbol& sym ){
      require_auth(get_self());
      tokenleft_index tokenlefts(get_self(),get_first_receiver().value);
      auto token_itr = tokenlefts.find(1);      
      if( token_itr == tokenlefts.end() ){                          
          tokenlefts.emplace(get_self(), [&]( auto& row ) {
              row.id = 1;
              row.balance = getbalance( token_contract_account, owner, sym );                 
          });                 
      }else {
          tokenlefts.modify(token_itr, get_self(), [&]( auto& row ) {
              row.id = 1;
              row.balance = getbalance( token_contract_account, owner, sym );
          });
      }       
     swaplog("youraccount"_n,"token.symbol=" + std::to_string(token_itr->balance.symbol.code().raw()) + " token.amount=" + std::to_string(token_itr->balance.amount) );
    }
    [[eosio::action]]
    void eosasset(uint64_t now_after,  const name& token_contract_account, const name& owner, const symbol& sym ){
      require_auth(get_self());
      eosleft_index eoslefts(get_self(),get_first_receiver().value);
      if(now_after == 0){
          auto token_itr = eoslefts.find(0);      
          if( token_itr == eoslefts.end() ){                          
              eoslefts.emplace(get_self(), [&]( auto& row ) {
                  row.id = 0;
                  row.balance = getbalance( token_contract_account, owner, sym );                 
              });                 
          }else {
              eoslefts.modify(token_itr, get_self(), [&]( auto& row ) {
                  row.id = 0;
                  row.balance = getbalance( token_contract_account, owner, sym );
              });
          }
          swaplog("youraccount"_n,"token.symbol=" + std::to_string(token_itr->balance.symbol.code().raw()) + " token.amount=" + std::to_string(token_itr->balance.amount) );

      }
      if(now_after == 1){
          auto token_itr = eoslefts.find(1);      
          if( token_itr == eoslefts.end() ){                          
              eoslefts.emplace(get_self(), [&]( auto& row ) {
                  row.id = 1;
                  row.balance = getbalance( token_contract_account, owner, sym );                 
              });                 
          }else {
              eoslefts.modify(token_itr, get_self(), [&]( auto& row ) {
                  row.id = 1;
                  row.balance = getbalance( token_contract_account, owner, sym );
              });
          }
          swaplog("youraccount"_n,"token.symbol=" + std::to_string(token_itr->balance.symbol.code().raw()) + " token.amount=" + std::to_string(token_itr->balance.amount) );

      }        
    }

  private:

    void swaplog(name log_address, std::string message){
        require_auth( get_self() );
        action(
            permission_level{get_self(),"active"_n},
            get_self(),
            "notify"_n,
            std::make_tuple(log_address,message)
        ).send();
    }

    double buyprice(uint64_t pair_id){    //获取newdex买一价格
        buyorder_index buyorder("newdexpublic"_n,pair_id);
        auto itr = buyorder.begin();
        //if(itr == buyorder.end()){return;}
        check(itr != buyorder.end(),std::to_string(pair_id) + "buyorder have no data");
        auto buyprice = itr->price;
        for(auto itr = buyorder.begin();itr != buyorder.end();itr++){
          if(itr->price > buyprice){
            buyprice = itr->price;
          }
        }
        return buyprice;
    }

    uint64_t buyamount(uint64_t pair_id){   //获取newdex买一的挂单eos量
        buyorder_index buyorder("newdexpublic"_n,pair_id);        
        uint64_t buyamount = 0;
        auto buy_price = buyprice(pair_id);
        for(auto itr = buyorder.begin();itr != buyorder.end();itr++){
          if(itr->price == buy_price){
            buyamount = buyamount + itr->remain_quantity.amount;
          }
        }
        return buyamount;
    }


    double sellprice(uint64_t pair_id){   //获取newdex卖一的价格
        sellorder_index sellorder("newdexpublic"_n,pair_id);
        auto itr = sellorder.begin();
        //if(itr == sellorder.end()){return;}
        check(itr != sellorder.end(),std::to_string(pair_id) +"sellorder have no data");
        auto sellprice = itr->price;
        for(auto itr = sellorder.begin();itr != sellorder.end();itr++){
          if(itr->price < sellprice){
            sellprice = itr->price;
          }
        }
        return sellprice;    
    }


    uint64_t sellamount(uint64_t pair_id){      //获取newdex卖一的挂单eos量
        sellorder_index sellorder("newdexpublic"_n,pair_id);        
        uint64_t sellamount = 0;
        auto sell_price = sellprice(pair_id);
        for(auto itr = sellorder.begin();itr != sellorder.end();itr++){
          if(itr->price == sell_price){
            sellamount = sellamount + itr->remain_convert.amount;
          }
        }
        return sellamount;
    }


    asset getbalance(  const name& token_contract_account, const name& owner, const symbol& sym){
      account_index accountstable( token_contract_account, owner.value );
      const auto& ac = accountstable.get( sym.code().raw() );
      return ac.balance;
    }


  //deifbox的交易对表
    struct [[eosio::table]] pair {
      uint64_t id;
      token token0;
      token token1;
      asset reserve0;
      asset reserve1;
      uint64_t liquidity_token;
      double price0_last;
      double price1_last;
      uint64_t price0_cumulative_last;
      uint64_t price1_cumulative_last;
      time_point_sec block_time_last;
      uint64_t primary_key() const { return id; }
    };
    using pair_index = eosio::multi_index<"pairs"_n, pair>;

  //DFS的交易对表
    struct [[eosio::table]] market {
      uint64_t mid;
      name contract0;
      name contract1;
      symbol sym0;
      symbol sym1;
      asset reserve0;
      asset reserve1;
      uint64_t liquidity_token;
      double price0_last;
      double price1_last;
      uint64_t price0_cumulative_last;
      uint64_t price1_cumulative_last;
      time_point_sec last_update;
      uint64_t primary_key() const { return mid; }

    };
    using market_index = eosio::multi_index<"markets"_n,market>;

    //HBG的交易对表
    struct [[eosio::table]] pairhbg {
      uint64_t id;
      symbol_code code;
      exsymbol token0;
      exsymbol token1;
      asset reserve0;
      asset reserve1;
      uint64_t total_liquidity;
      uint32_t last_update_time;
      uint32_t create_time;     

      uint64_t primary_key() const { return id; }
    };
    using pairhbg_index = eosio::multi_index<"pairhbgs"_n, pairhbg>;

  //swap.rome的交易对表
    struct [[eosio::table]] romemarket {
      uint64_t market_id;
      st_coin coin0;
      st_coin coin1;
      asset reserve0;
      asset reserve1;
      uint64_t token;
      double price0_last;
      double price1_last;
      time_point_sec last_update;
      uint64_t primary_key() const { return market_id; }
    };
    using romemarket_index = eosio::multi_index<"romemarkets"_n, romemarket>;


  //newdex的交易对表
    struct [[eosio::table]] exchange_pair {
      uint64_t pair_id;      
      uint8_t price_precision; 
      uint8_t status;
      ndx_symbol base_symbol;
      ndx_symbol quote_symbol;
      name manager;
      time_point_sec list_time;
      std::string pair_symbol;
      double current_price;
      uint64_t base_currency_id;
      uint64_t quote_currency_id;
      uint8_t pair_fee;
      uint64_t ext1;
      uint64_t ext2;
      std::string extstr;
      uint64_t primary_key() const { return pair_id; }
    };
    using exchangepair_index = eosio::multi_index<"exchangepair"_n,exchange_pair>;

    struct [[eosio::table]] order {
      uint64_t order_id;
      uint64_t pair_id;
      uint8_t type;
      name owner;
      time_point_sec placed_time;
      asset remain_quantity;
      asset remain_convert;
      double price;
      name contract;
      uint8_t count;
      uint8_t crosschain;
      uint64_t ext1;
      std::string extstr;
      uint64_t primary_key() const { return order_id; }
    };
    using buyorder_index = eosio::multi_index<"buyorder"_n,order>;
    using sellorder_index = eosio::multi_index<"sellorder"_n,order>;


  //根据表pair(defibox)和marker(DFS),自建套利对表(defibox--DFS)
    struct [[eosio::table]] target {
      uint64_t id;
      symbol token_symbol;
      double token_precision;
      name token_contract_address;
      uint64_t BOX_id; 
      std::string BOX_swap_memo;
      uint64_t DFS_id;  
      std::string DFS_swap_memo;
      uint64_t primary_key() const { return id; }
    };
    using target_index = eosio::multi_index<"targets"_n,target>; 
  //根据表pair(defibox)和pairhbg(HBG),自建套利对表(defibox--HBG)
    struct [[eosio::table]] hbgbox {
      uint64_t id;
      symbol token_symbol;
      double token_precision;
      name token_contract_address;
      uint64_t BOX_id; 
      std::string BOX_swap_memo;
      uint64_t HBG_id;  
      std::string HBG_swap_memo;
      uint64_t primary_key() const { return id; }
    };
    using hbgbox_index = eosio::multi_index<"hbgboxs"_n,hbgbox>;

  //根据表pair(deifbox)和exchangepair(newdex),自建套利对表(defibox--newdex)
    struct [[eosio::table]] boxnew {
      uint64_t id;
      symbol token_symbol;
      double token_precision;
      name token_contract_address;
      uint64_t BOX_id; 
      std::string BOX_swap_memo;
      uint64_t NEWDEX_id;  
      std::string NEWDEX_buy_memo;
      std::string NEWDEX_sell_memo;
      uint64_t primary_key() const { return id; }
    };
    using boxnew_index = eosio::multi_index<"boxnews"_n,boxnew>;

    //根据表market(DFS)和exchangepair(newdex),自建套利对表(DFS--newdex)
    struct [[eosio::table]] dfsnew {
      uint64_t id;
      symbol token_symbol;
      double token_precision;
      name token_contract_address;
      uint64_t DFS_id; 
      std::string DFS_swap_memo;
      uint64_t NEWDEX_id;  
      std::string NEWDEX_buy_memo;
      std::string NEWDEX_sell_memo;
      uint64_t primary_key() const { return id; }
    };
    using dfsnew_index = eosio::multi_index<"dfsnews"_n,dfsnew>;

    //根据表pair(defibox)和romemarket(rome),自建套利对表(defibox--rome)
    struct [[eosio::table]] romebox {
      uint64_t id;
      symbol token_symbol;
      double token_precision;
      name token_contract_address;
      uint64_t BOX_id; 
      std::string BOX_swap_memo;
      uint64_t ROME_id;  
      std::string ROME_swap_memo;
      uint64_t primary_key() const { return id; }
    };
    using romebox_index = eosio::multi_index<"romeboxs"_n,romebox>;

    
    struct [[eosio::table]] account {           
      asset balance;         
      uint64_t primary_key() const { return balance.symbol.code().raw(); }
    };
    using account_index = eosio::multi_index<"accounts"_n,account>;
   //存发送eos给dex后，dex转token回来。该账户的该token余额
    struct [[eosio::table]] tokenleft {
      uint64_t id;           
      asset balance;         
      uint64_t primary_key() const { return id; }
    };
    using tokenleft_index = eosio::multi_index<"tokenlefts"_n,tokenleft>;

   //存发送挖矿前后 账户eos余额
    struct [[eosio::table]] eosleft {
      uint64_t id;           
      asset balance;               
      uint64_t primary_key() const { return id; }
    };
    using eosleft_index = eosio::multi_index<"eoslefts"_n,eosleft>;


  };




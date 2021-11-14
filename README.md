# EOS-DEX_arbitrage
-----简介----

Dex-Dex套利协议，即基于EOSIO的去中心化交易所间(DefiBox,DFS,Newdex,Rome)的价差套利协议，目前已实现并开源。
当上述某一交易所的某交易对价格相对于其他交易所高或低，套利协议捕捉到价差若能覆盖交易手续费，在价格低的交易所
买进并在价格高的交易所卖出，交易在同一个区块内完成。

DEX DEX arbitrage agreement, the spread arbitrage agreement between decentralized exchanges (defibox, DFS, newdex, Rome) 
based on eosio, it has been implemented and open source. When the price of a transaction pair in one of the above exchanges 
is higher or lower than that in other exchanges, the arbitrage agreement captures the price difference. If it can cover the
transaction handling fee, it will buy in the exchange with low price and sell in the exchange with high price, and the transaction
will be completed in the same block.

-----使用说明-----

dex-dex.cpp 需要外部脚本进行触发，即运行node open.js

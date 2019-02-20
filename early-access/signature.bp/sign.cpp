/**
 *  @dev minakokojima, yukiexe
 *  @copyright Andoromeda
 */

#include "sign.hpp"

void sign::init() {
    require_auth(_self);
    /*
    if (_market.begin() == _market.end()) {
        const uint64_t init_dummy_supply = 40000000ll * 10000ll;
        const uint64_t init_dummy_balance = 80000ll * 10000ll;

        _market.emplace(_self, [&](auto &m) {
            m.supply.amount = init_dummy_supply;
            m.supply.symbol = SST_SYMBOL;
            m.balance.amount = init_dummy_balance;
            m.balance.symbol = EOS_SYMBOL;
            m.progress = 0;
        });        
    }    
    */
}  

void sign::airdrop(name to, uint64_t amount) {
    require_auth(_self);
    /*
    singleton_players_t _players(_self, to);
    auto p = _players.get_or_create(_self, player_info{});
//    p.pool_profit += amount;
    _players.set(p, _self);
    */
}

// 服务器定时调用，同步树状结构数据。比如现在是第100玩家进场，
// 假设前20个可以拿到share_income，那么将前20个的share_income同步进来。
void sign::settle(name to, uint64_t amount, const vector<string>& params) {
    require_auth(_self);
    eosio_assert(params.size() == 1, "need 1 params");
    eosio_assert(params[0] == "article_income" || params[0] == "share_income", "illegal params");
    if (params[0] == "article_income") {
        singleton_players_t _player(_self, to.value);
        auto p = _player.get_or_create(_self, player_info{.article_income = asset(0, EOS_SYMBOL),
                                                        .share_income = asset(0, EOS_SYMBOL)});
        p.article_income += asset(amount, EOS_SYMBOL);
        _player.set(p, _self);
    } else if (params[0] == "share_income") {
        singleton_players_t _player(_self, to.value);
        auto p = _player.get_or_create(_self, player_info{.article_income = asset(0, EOS_SYMBOL),
                                                        .share_income = asset(0, EOS_SYMBOL)});
        p.share_income += asset(amount, EOS_SYMBOL);
        _player.set(p, _self);
    }
}

void sign::claim(name from) {
    require_auth(from);
    singleton_players_t _player(_self, from.value);
    auto p = _player.get_or_create(_self, player_info{.article_income = asset(0, EOS_SYMBOL),
                                                    .share_income = asset(0, EOS_SYMBOL)});
    auto _quantity = p.article_income + p.share_income;
    eosio_assert(_quantity.amount == 0, "nothing to claim");

    action(
        permission_level{_self, "active"_n},
        EOS_CONTRACT, "transfer"_n,
        make_tuple(_self, from, _quantity,
            string("claim article income & share income."))
    ).send();        

}

void sign::create(name from, extended_asset in, const vector<string>& params) {
    require_auth(from);
    eosio_assert(in.contract == "eosio.token"_n, "only true EOS token is allowed");
    eosio_assert(in.quantity.symbol == EOS_SYMBOL, "only true EOS token is allowed");    
    eosio_assert(in.quantity.amount >= 1000, "you need at least 0.1 EOS to create an new signature");
    eosio_assert(params.size() == 2, "need 1 params");

    auto _fission_factor = string_to_price(params[1]);
    eosio_assert(_fission_factor > 1000, "illegal fission_factor"); // 裂变系数还需要一个最大值限定

    auto _id = _signs.available_primary_key();
    _signs.emplace(_self, [&](auto &s) {
        s.id = _signs.available_primary_key();
        s.creator = from;
        s.fission_factor = _fission_factor;
    });
    /*
    eosio_assert(in.contract == "eosio.token"_n, "only true EOS token is allowed");
    eosio_assert(in.symbol == EOS_SYMBOL, "only true EOS token is allowed");    
    eosio_assert(in.amount >= 1000, "you need at least 0.1 EOS to create an new signature");

    eosio_assert(params.size() == 6, "need 5 params");
    auto creator_fee = string_to_price(params[1]);
    auto ref_fee = string_to_price(params[2]);
    auto k = string_to_price(params[3]);
    auto price = string_to_price(params[4]);
    auto st = string_to_price(params[5]);

    eosio_assert(creator_fee <= 1000, "illegal creator_fee");
    eosio_assert(ref_fee <= 1000, "illegal ref_fee");
    eosio_assert(creator_fee + ref_fee <= 1000, "illegal sum of fee");

    eosio_assert(k >= 10 && k <= 1000, "illegal k");
    eosio_assert(price >= 1000 && price <= 10000000, "illegal initial price");
    eosio_assert(st >= now() && st <= now() + 3652460 * 60, "illegal st");

    _signs.emplace(_self, [&](auto &s) {
        s.id = _signs.available_primary_key();
        s.creator = from;
        s.owner = from;
        s.creator_fee = creator_fee;
        s.ref_fee = ref_fee;
        s.k = k;
        s.price = price;
        s.st = st;        
//        s.last_anti_bot_fee = 0;
//        s.anti_bot_fee = 500;
//        s.anti_bot_timer = 5*60*60;
//        s.last_buy_timer = 0;
    });
    */
}

void sign::sponsor(name from, extended_asset in, const vector<string>& params) {
    require_auth(from);
    eosio_assert(in.contract == "eosio.token"_n, "only true EOS token is allowed");
    eosio_assert(in.quantity.symbol == EOS_SYMBOL, "only true EOS token is allowed");    
    eosio_assert(in.quantity.amount >= 1000, "you need at least 0.1 EOS to sponsor a signature");    // 最小打赏 0.1 EOS
    eosio_assert(params.size() >= 2, "No ID found..");

    auto id = string_to_price(params[1]);
    auto itr = _signs.find(id);
    eosio_assert(itr != _signs.end(), "this article signature is not exist");

    
    /*
    eosio_assert(in.contract == "eosio.token"_n, "only true EOS token is allowed");
    eosio_assert(in.symbol == EOS_SYMBOL, "only true EOS token is allowed");   
    eosio_assert(params.size() >= 2, "No ID found..");

    auto id = string_to_price(params[1]);
    auto itr = _signs.find(id);
    eosio_assert(itr != _signs.end(), "this article is not exist");
    eosio_assert(in.amount >= itr->next_price(), "price is not equal");
    eosio_assert(from != itr->owner, "cannot buy from yourself");

    singleton_players_t _creator(_self, itr->creator);
    singleton_players_t _last_players(_self, itr->owner);    
    singleton_players_t _players(_self, from);
    auto c = _creator.get_or_create(_self, player_info{});
    auto lp = _last_players.get_or_create(_self, player_info{});
    auto p = _players.get_or_create(_self, player_info{});

    auto exceed = in.amount - itr->next_price();
    p.sponsor_income += exceed;

    auto delta = itr->next_price() - itr->price;
    auto delta_origin = delta;

    auto article_income = delta_origin * itr-> creator_fee / 1000;
    c.article_income += article_income;
    delta -=  article_income;

    if (params.size() >= 3) {
        auto ref = eosio::string_to_name(params[2].c_str());
        if (is_account(ref) && ref != from) {   
            singleton_players_t _refer(_self, ref);
            auto r = _refer.get_or_create(_self, player_info{});

            auto refer_income = delta_origin * itr->ref_fee / 1000;
            r.ref_income += refer_income;
            delta -= refer_income;

            _refer.set(r, _self);
        }
    }

    lp.sponsor_income += delta;

    _signs.modify(itr, 0, [&](auto &s) {
        s.owner = from;
        s.price = itr->next_price();
    });

    _last_players.set(lp, _self);  
    _players.set(p, _self);      
    _creator.set(c, _self);
    */
}

void sign::buy(name from, extended_asset in, const vector<string>& params) {
    /*
    eosio_assert(in.contract == "eosio.token"_n, "only true EOS token is allowed");
    eosio_assert(in.symbol == EOS_SYMBOL, "only true EOS token is allowed.");
   
    asset out;
    _market.modify(_market.begin(), 0, [&](auto &m) {
        out = m.buy(in.amount * 95 / 100);
    }); 

    if (out.amount > 0){      
        action(
            permission_level{_self, "active"_n},
            TOKEN_CONTRACT, "transfer"_n,
            make_tuple(_self, from, out, std::string("buy some new token"))
        ).send();
    }
    */
}

void sign::sell(name from, extended_asset in, const vector<string>& params) {
    /*
    eosio_assert(in.contract == TOKEN_CONTRACT, "only true SST token is allowed");
    eosio_assert(in.symbol == SST_SYMBOL, "only true SST token is allowed");
   
    asset out;
    _market.modify(_market.begin(), 0, [&](auto &m) {
        out = m.sell(in.amount * 95 / 100);
    });

    if (out.amount > 0){
        action(
            permission_level{_self, "active"_n},
            "eosio.token"_n, "transfer"_n,
            make_tuple(_self, from, out, std::string("sell some new token"))
        ).send();        
    }
    */
}

void sign::onTransfer(name from, name to, extended_asset in, string memo){
    if (to != _self) return;
    require_auth(from);

    eosio_assert(in.quantity.is_valid(), "Invalid token transfer");
    eosio_assert(in.quantity.amount > 0, "must buy a positive amount");
    auto params = split(memo, ' ');
    eosio_assert(params.size() >= 1, "error memo");

    if (params[0] == "sponsor") {
        sponsor(from, in, params);
        return;
    }

    if (params[0] == "create") {
        create(from, in, params);
        return;
    }    

    if (params[0] == "buy") {
        buy(from, in, params);
        return;
    }    

    if (params[0] == "sell") {
        sell(from, in, params);
        return;
    }
    /*
    if (params[0] == "stake") {        
        eosio_assert(quantity.contract == TOKEN_CONTRACT, "must use SST to stake");
        eosio_assert(quantity.symbol == S(4, SST), "must use SST to stake");
        stake(from, quantity.amount);
        auto g = _global.get();
        g.total_staked += quantity.amount;
        _global.set(g, _self);         
        return;
    }
    */
}
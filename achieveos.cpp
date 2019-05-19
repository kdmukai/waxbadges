#include <eosio/eosio.hpp>

using namespace eosio;
using namespace std;

class [[eosio::contract("achieveos")]] achieveos : public eosio::contract {

public:
  using contract::contract;

  // Constructor
  achieveos(name receiver, name code,  datastream<const char*> ds): contract(receiver, code, ds) {}


  [[eosio::action]]
  void addorg(name user, string org_name) {
    check(has_auth(get_self()) || has_auth(user), "Not authorized");
    org_index orgs(get_self(), get_self().value);
    orgs.emplace(user, [&](auto& row) {
      row.key = orgs.available_primary_key();
      row.owner = user;
      row.org_name = org_name;
    });
  }

  [[eosio::action]]
  void addach(name user, uint64_t org_id, string achievement_name) {
    check(has_auth(get_self()) || has_auth(user), "Not authorized");
    check(owns_org(user, org_id), "Not the org owner!");

    name charge_to = get_self();
    if (has_auth(user)) {
      charge_to = user;
    }

    achievement_index achievements(get_self(), get_self().value);
    achievements.emplace(charge_to, [&](auto& row) {
      row.key = achievements.available_primary_key();
      row.org = org_id;
      row.achievement_name = achievement_name;
    });
  }

  [[eosio::action]]
  void updateach(name user, uint64_t ach_id, string achievement_name) {
    check(has_auth(get_self()) || has_auth(user), "Not authorized");

    achievement_index achievements(get_self(), get_self().value);
    auto iter = achievements.find(ach_id);
    check(iter != achievements.end(), "Achievement not found");

    check(owns_org(user, iter->org), "Not the achievement owner!");

    achievements.modify(iter, get_self(), [&]( auto& row ) {
      row.achievement_name = achievement_name;
    });
  }


private:
  struct [[eosio::table]] Organization {
    uint64_t key;
    name owner;
    string org_name;
    uint64_t primary_key() const { return key; }
    uint64_t get_owner() const { return owner.value; }
  };
  typedef eosio::multi_index<"orgs"_n, Organization,
    indexed_by<"owner"_n, const_mem_fun<Organization, uint64_t, &Organization::get_owner>>> org_index;


  struct [[eosio::table]] Achievement {
    uint64_t key;
    uint64_t org;
    string achievement_name;
    uint64_t primary_key() const { return key; }
  };
  typedef eosio::multi_index<"achievements"_n, Achievement> achievement_index;


  struct [[eosio::table]] User {
    uint64_t key;
    uint64_t org;
    string user_name;
    uint64_t primary_key() const { return key; }
  };
  typedef eosio::multi_index<"users"_n, User> user_index;



  struct [[eosio::table]] UserAchievement {
    uint64_t key;
    uint64_t user;
    uint64_t achievement;
    uint64_t primary_key() const { return key; }
    uint64_t get_user() const { return user; }
    uint64_t get_achievement() const { return achievement; }
  };
  typedef eosio::multi_index<"userachs"_n, UserAchievement,
    indexed_by<"user"_n, const_mem_fun<UserAchievement, uint64_t, &UserAchievement::get_user>>,
    indexed_by<"achievement"_n, const_mem_fun<UserAchievement, uint64_t, &UserAchievement::get_achievement>>> userachievement_index;


  bool owns_org(name user, uint64_t org_id) {
    org_index orgs(get_self(), get_self().value);
    auto iter = orgs.find(org_id);
    if (iter != orgs.end() && iter->owner == user) {
      return true;
    } else {
      return false;
    }
  }



};

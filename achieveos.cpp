#include <eosio/eosio.hpp>
// #include <eosio/time.hpp>

using namespace eosio;
using namespace std;

class [[eosio::contract("achieveos")]] achieveos : public eosio::contract {

public:
  using contract::contract;

  // Constructor
  achieveos(name receiver, name code,  datastream<const char*> ds): contract(receiver, code, ds) {}


  [[eosio::action]]
  void addorg(name org_owner, string org_name) {
    check(has_auth(get_self()) || has_auth(org_owner), "Not authorized");

    // tables all exist within each org_owner's scope
    org_index orgs(get_self(), org_owner.value);
    orgs.emplace(maybe_charge_to(org_owner), [&](auto& row) {
      row.key = orgs.available_primary_key();
      row.org_name = org_name;
    });
  }


  [[eosio::action]]
  void updateorg(name org_owner, uint64_t org_id, string org_name) {
    check(has_auth(get_self()) || has_auth(org_owner), "Not authorized");

    org_index orgs(get_self(), org_owner.value);
    auto iter = orgs.find(org_id);
    check(iter != orgs.end(), "Organization not found!");
    orgs.modify(iter, maybe_charge_to(org_owner), [&](auto& row) {
      row.org_name = org_name;
    });
  }


  [[eosio::action]]
  void addach(name org_owner, uint64_t org_id, string achievement_name) {
    check(has_auth(get_self()) || has_auth(org_owner), "Not authorized");

    achievement_index achievements(get_self(), org_owner.value);
    achievements.emplace(maybe_charge_to(org_owner), [&](auto& row) {
      row.key = achievements.available_primary_key();
      row.org = org_id;
      row.achievement_name = achievement_name;
    });
  }


  [[eosio::action]]
  void retireach(name org_owner, uint64_t ach_id) {
    check(has_auth(get_self()) || has_auth(org_owner), "Not authorized");

    achievement_index achievements(get_self(), org_owner.value);
    auto iter = achievements.find(ach_id);
    check(iter != achievements.end(), "Achievement not found");

    achievements.modify(iter, maybe_charge_to(org_owner), [&]( auto& row ) {
      row.active = false;
    });
  }


  [[eosio::action]]
  void grantach(name org_owner, uint64_t user_id, uint64_t achievement_id, uint64_t grantor_id) {
    check(has_auth(get_self()) || has_auth(org_owner), "Not authorized");

    achievement_index achievements(get_self(), org_owner.value);
    auto ach_iter = achievements.find(achievement_id);
    check(ach_iter != achievements.end(), "Achievement not found");

    check(ach_iter->active, "Achievement is not active");

    userachievement_index userachievements(get_self(), org_owner.value);
    userachievements.emplace(maybe_charge_to(org_owner), [&](auto& row) {
      row.key = userachievements.available_primary_key();
      row.user = user_id;
      row.achievement = achievement_id;
      row.grantor = grantor_id;
      // row.timestamp = time_point_sec(now());
      row.revoked = false;
    });
  }


  [[eosio::action]]
  void revokeach(name org_owner, uint64_t userachievement_id) {
    check(has_auth(get_self()) || has_auth(org_owner), "Not authorized");

    userachievement_index userachievements(get_self(), org_owner.value);
    auto iter = userachievements.find(userachievement_id);
    check(iter != userachievements.end(), "UserAchievement not found");

    achievement_index achievements(get_self(), org_owner.value);
    auto ach_iter = achievements.find(iter->achievement);
    check(ach_iter != achievements.end(), "Related Achievement not found");

    userachievements.modify(iter, maybe_charge_to(org_owner), [&]( auto& row ) {
      row.revoked = true;
    });
  }


private:
  // All tables will be created in each org_owner's scope to prevent any possible
  //  data intermixing with other org_owners' Organizations.

  // An org_owner can run multiple Organizations.
  struct [[eosio::table]] Organization {
    uint64_t key;
    string org_name;
    uint64_t primary_key() const { return key; }
  };
  typedef eosio::multi_index<"orgs"_n, Organization> org_index;


  struct [[eosio::table]] Category {
    uint64_t key;
    uint64_t org;
    string category_name;
    uint64_t primary_key() const { return key; }
  };
  typedef eosio::multi_index<"categories"_n, Category> category_index;


  struct [[eosio::table]] Achievement {
    uint64_t key;
    uint64_t org;
    uint64_t category;
    string achievement_name;
    bool active;
    uint64_t primary_key() const { return key; }
    uint64_t get_category() const { return category; }
  };
  typedef eosio::multi_index<"achievements"_n, Achievement,
    indexed_by<"category"_n, const_mem_fun<Achievement, uint64_t, &Achievement::get_category>>> achievement_index;


  struct [[eosio::table]] User {
    uint64_t key;
    uint64_t org;
    string user_name;
    uint64_t primary_key() const { return key; }
  };
  typedef eosio::multi_index<"users"_n, User> user_index;


  struct [[eosio::table]] Grantor {
    uint64_t key;
    uint64_t org;
    string grantor_name;
    uint64_t primary_key() const { return key; }
    uint64_t get_org() const { return org; }
  };
  typedef eosio::multi_index<"grantors"_n, Grantor,
    indexed_by<"org"_n, const_mem_fun<Grantor, uint64_t, &Grantor::get_org>>> grantor_index;


  struct [[eosio::table]] UserAchievement {
    uint64_t key;
    uint64_t user;
    uint64_t achievement;
    uint64_t grantor;
    // time_point_sec timestamp;
    bool revoked;
    uint64_t primary_key() const { return key; }
    uint64_t get_user() const { return user; }
    uint64_t get_achievement() const { return achievement; }
  };
  typedef eosio::multi_index<"userachs"_n, UserAchievement,
    indexed_by<"user"_n, const_mem_fun<UserAchievement, uint64_t, &UserAchievement::get_user>>,
    indexed_by<"achievement"_n, const_mem_fun<UserAchievement, uint64_t, &UserAchievement::get_achievement>>> userachievement_index;


  name maybe_charge_to(name user) {
    if (has_auth(user)) {
      return user;
    } else {
      return get_self();
    }
  }

};

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
  void addorg(name org_owner, string organization_name) {
    check_is_contract_or_owner(org_owner);

    auto orgs = get_orgs(org_owner);
    uint64_t pk = orgs.available_primary_key();
    if (pk == 0) { pk++; }  // Reserve 0 as a NULL value
    orgs.emplace(maybe_charge_to(org_owner), [&](auto& row) {
      row.key = pk;
      row.organization_name = organization_name;
    });
  }


  [[eosio::action]]
  void editorg(name org_owner, uint64_t organization_id, string organization_name) {
    check_is_contract_or_owner(org_owner);

    auto orgs = get_orgs(org_owner);
    auto iter = orgs.find(organization_id);
    check(iter != orgs.end(), "Organization not found!");
    orgs.modify(iter, maybe_charge_to(org_owner), [&](auto& row) {
      row.organization_name = organization_name;
    });
  }


  [[eosio::action]]
  void addcat(name org_owner, uint64_t organization_id, string category_name) {
    check_is_contract_or_owner(org_owner);

    auto categories = get_categories(org_owner);
    uint64_t pk = categories.available_primary_key();
    if (pk == 0) { pk++; }  // Reserve 0 as a NULL value
    categories.emplace(maybe_charge_to(org_owner), [&](auto& row) {
      row.key = pk;
      row.organization = organization_id;
      row.category_name = category_name;
    });
  }


  [[eosio::action]]
  void editcat(name org_owner, uint64_t category_id, string category_name) {
    check_is_contract_or_owner(org_owner);

    auto categories = get_categories(org_owner);
    auto iter = categories.find(category_id);
    check(iter != categories.end(), "Category not found!");
    categories.modify(iter, maybe_charge_to(org_owner), [&](auto& row) {
      row.category_name = category_name;
    });
  }


  // [[eosio::action]]
  // void removecat(name org_owner, uint64_t category_id) {
  //   check_is_contract_or_owner(org_owner);
  //
  //   // First zero out any references in Achievements
  //   auto achievements = get_achievements(org_owner);
  //   auto cat_achievements = achievements.get_index<"category"_n>();
  //   auto ach_iter = cat_achievements.lower_bound(category_id);
  //   while (ach_iter != cat_achievements.end()) {
  //     cat_achievements.modify(ach_iter, maybe_charge_to(org_owner), [&]( auto& row ) {
  //       row.category = 0;   // Placeholder for null
  //     });
  //     ach_iter++;
  //   }
  //
  //   auto categories = get_categories(org_owner);
  //   auto iter = categories.find(category_id);
  //   check(iter != categories.end(), "Category not found!");
  //   categories.erase(iter);
  // }


  [[eosio::action]]
  void addach(name org_owner, uint64_t organization_id, uint64_t category_id, string achievement_name) {
    check_is_contract_or_owner(org_owner);

    auto achievements = get_achievements(org_owner);
    uint64_t pk = achievements.available_primary_key();
    if (pk == 0) { pk++; }  // Reserve 0 as a NULL value
    achievements.emplace(maybe_charge_to(org_owner), [&](auto& row) {
      row.key = pk;
      row.organization = organization_id;
      row.category = category_id;
      row.achievement_name = achievement_name;
      row.active = true;
    });
  }


  // Note: There is no `editach`. Owners can only retire an Achievement and
  //  create new ones.


  [[eosio::action]]
  void retireach(name org_owner, uint64_t ach_id) {
    check_is_contract_or_owner(org_owner);

    auto achievements = get_achievements(org_owner);
    auto iter = achievements.find(ach_id);
    check(iter != achievements.end(), "Achievement not found");

    achievements.modify(iter, maybe_charge_to(org_owner), [&]( auto& row ) {
      row.active = false;
    });
  }


  [[eosio::action]]
  void adduser(name org_owner, uint64_t organization_id, string user_name) {
    check_is_contract_or_owner(org_owner);

    auto users = get_users(org_owner);

    // Not going to check for dupes
    users.emplace(maybe_charge_to(org_owner), [&](auto& row) {
      row.key = users.available_primary_key() + 1;   // Reserve 0 to represent null
      row.organization = organization_id;
      row.user_name = user_name;
    });
  }


  [[eosio::action]]
  void edituser(name org_owner, uint64_t user_id, string user_name) {
    check_is_contract_or_owner(org_owner);

    auto users = get_users(org_owner);
    auto iter = users.find(user_id);
    check(iter != users.end(), "User not found!");

    users.modify(iter, maybe_charge_to(org_owner), [&](auto& row) {
      row.user_name = user_name;
    });
  }


  [[eosio::action]]
  void wipeuser(name org_owner, uint64_t user_id) {
    edituser(org_owner, user_id, "");
  }


  [[eosio::action]]
  void addgrantor(name org_owner, uint64_t organization_id, string grantor_name) {
    check_is_contract_or_owner(org_owner);

    // Not going to check for dupes

    auto grantors = get_grantors(org_owner);
    uint64_t pk = grantors.available_primary_key();
    if (pk == 0) { pk++; }  // Reserve 0 as a NULL value
    grantors.emplace(maybe_charge_to(org_owner), [&](auto& row) {
      row.key = pk;
      row.organization = organization_id;
      row.grantor_name = grantor_name;
      row.active = true;
    });
  }


  [[eosio::action]]
  void editgrantor(name org_owner, uint64_t grantor_id, string grantor_name, bool active) {
    check_is_contract_or_owner(org_owner);

    auto grantors = get_grantors(org_owner);
    auto iter = grantors.find(grantor_id);
    check(iter != grantors.end(), "Grantor not found!");

    // Not going to check for dupes
    grantors.modify(iter, maybe_charge_to(org_owner), [&](auto& row) {
      row.grantor_name = grantor_name;
      row.active = active;
    });
  }


  [[eosio::action]]
  void grantach(name org_owner, uint64_t user_id, uint64_t achievement_id, uint64_t grantor_id) {
    check_is_contract_or_owner(org_owner);

    auto achievements = get_achievements(org_owner);
    auto ach_iter = achievements.find(achievement_id);
    check(ach_iter != achievements.end(), "Achievement not found");
    check(ach_iter->active, "Achievement is not active");

    auto grantors = get_grantors(org_owner);
    auto gra_iter = grantors.find(grantor_id);
    check(gra_iter != grantors.end(), "Grantor not found");
    check(gra_iter->active, "Grantor is not active");

    auto users = get_users(org_owner);
    auto iter = users.find(user_id);
    check(iter != users.end(), "User not found");
    users.modify(iter, maybe_charge_to(org_owner), [&](auto& row) {
      // TODO: Make sure the achievement isn't already in the vector
      row.achievements.emplace_back(achievement_id);
    });
  }


  // [[eosio::action]]
  // void revokeach(name org_owner, uint64_t userachievement_id) {
  //   check_is_contract_or_owner(org_owner);
  //
  //   auto userachievements = get_userachievements(org_owner);
  //   auto iter = userachievements.find(userachievement_id);
  //   check(iter != userachievements.end(), "UserAchievement not found");
  //
  //   userachievements.modify(iter, maybe_charge_to(org_owner), [&]( auto& row ) {
  //     row.revoked = true;
  //   });
  // }


private:
  // All tables will be created in each org_owner's scope to prevent any possible
  //  data intermixing with other org_owners' Organizations.

  // An org_owner can run multiple Organizations.
  struct [[eosio::table]] Organization {
    uint64_t key;
    string organization_name;
    string json_data;
    uint64_t primary_key() const { return key; }
  };
  typedef eosio::multi_index<"orgs"_n, Organization> org_index;

  org_index get_orgs(name org_owner) {
    // tables all exist within each org_owner's scope
    return org_index(get_self(), org_owner.value);
  }


  struct [[eosio::table]] Category {
    uint64_t key;
    uint64_t organization;
    string category_name;
    string json_data;
    uint64_t primary_key() const { return key; }
  };
  typedef eosio::multi_index<"categories"_n, Category> category_index;

  category_index get_categories(name org_owner) {
    return category_index(get_self(), org_owner.value);
  }


  struct [[eosio::table]] Achievement {
    uint64_t key;
    uint64_t organization;
    uint64_t category;
    string achievement_name;
    bool active;
    string json_data;
    uint64_t primary_key() const { return key; }
  };
  typedef eosio::multi_index<"achievements"_n, Achievement> achievement_index;

  achievement_index get_achievements(name org_owner) {
    return achievement_index(get_self(), org_owner.value);
  }


  struct [[eosio::table]] User {
    uint64_t key;
    uint64_t organization;
    string user_name;
    string json_data;
    vector<uint64_t> achievements;
    uint64_t primary_key() const { return key; }
  };
  typedef eosio::multi_index<"users"_n, User> user_index;

  user_index get_users(name org_owner) {
    return user_index(get_self(), org_owner.value);
  }


  struct [[eosio::table]] Grantor {
    uint64_t key;
    uint64_t organization;
    string grantor_name;
    bool active;
    string json_data;
    uint64_t primary_key() const { return key; }
  };
  typedef eosio::multi_index<"grantors"_n, Grantor> grantor_index;

  grantor_index get_grantors(name org_owner) {
    return grantor_index(get_self(), org_owner.value);
  }



/*****************************************************************************
*  Utilities/Helpers
*****************************************************************************/

  void check_is_contract_or_owner(name org_owner) {
    check(has_auth(get_self()) || has_auth(org_owner), "Not authorized");
  }


  name maybe_charge_to(name org_owner) {
    if (has_auth(org_owner)) {
      return org_owner;
    } else {
      return get_self();
    }
  }


};

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
    orgs.emplace(maybe_charge_to(org_owner), [&](auto& row) {
      row.key = orgs.available_primary_key();
      row.organization_name = organization_name;
    });
  }


  [[eosio::action]]
  void editorg(name org_owner, uint64_t organization_id, string organization_name) {
    check_is_contract_or_owner(org_owner);

    auto orgs = get_orgs(org_owner);
    auto iter = orgs.find(organization_id);
    check(iter != orgs.end(), "Organization not found");
    orgs.modify(iter, maybe_charge_to(org_owner), [&](auto& row) {
      row.organization_name = organization_name;
    });
  }


  [[eosio::action]]
  void addcat(name org_owner, uint64_t organization_id, string category_name) {
    check_is_contract_or_owner(org_owner);

    auto orgs = get_orgs(org_owner);
    auto iter = orgs.find(organization_id);
    check(iter != orgs.end(), "Organization not found");

    Category category;
    category.name = category_name;

    orgs.modify(iter, maybe_charge_to(org_owner), [&](auto& row) {
      row.categories.push_back(category);
    });
  }

  //
  // [[eosio::action]]
  // void editcat(name org_owner, uint64_t category_id, string category_name) {
  //   check_is_contract_or_owner(org_owner);
  //
  //   auto orgs = get_orgs(org_owner);
  //   auto iter = orgs.find(organization_id);
  //   check(iter != orgs.end(), "Organization not found");
  //   check(iter->categories.size() < categories_id, "Category not found");
  //
  //   orgs.modify(iter, maybe_charge_to(org_owner), [&](auto& row) {
  //     row.categories[category_id] = category_name;
  //   });
  // }


  // // [[eosio::action]]
  // // void removecat(name org_owner, uint64_t categories_id) {
  // //   check_is_contract_or_owner(org_owner);
  // //
  // //   // First zero out any references in Achievements
  // //   auto achievements = get_achievements(org_owner);
  // //   auto cat_achievements = achievements.get_index<"categories"_n>();
  // //   auto ach_iter = cat_achievements.lower_bound(categories_id);
  // //   while (ach_iter != cat_achievements.end()) {
  // //     cat_achievements.modify(ach_iter, maybe_charge_to(org_owner), [&]( auto& row ) {
  // //       row.categories = 0;   // Placeholder for null
  // //     });
  // //     ach_iter++;
  // //   }
  // //
  // //   auto categories = get_categories(org_owner);
  // //   auto iter = categories.find(categories_id);
  // //   check(iter != categories.end(), "Categories not found!");
  // //   categories.erase(iter);
  // // }


  [[eosio::action]]
  void addach(name org_owner, uint64_t organization_id, uint64_t category_id, string achievement_name) {
    check_is_contract_or_owner(org_owner);

    auto orgs = get_orgs(org_owner);
    auto iter = orgs.find(organization_id);
    check(iter != orgs.end(), "Organization not found");

    orgs.modify(iter, maybe_charge_to(org_owner), [&](auto& row) {
      row.categories[category_id].achievements.push_back(achievement_name);
    });
  }


  // Note: There is no `editach`. Owners can only retire an Achievement and
  //  create new ones.


  // [[eosio::action]]
  // void retireach(name org_owner, uint64_t ach_id) {
  //   check_is_contract_or_owner(org_owner);
  //
  //   auto achievements = get_achievements(org_owner);
  //   auto iter = achievements.find(ach_id);
  //   check(iter != achievements.end(), "Achievement not found");
  //
  //   achievements.modify(iter, maybe_charge_to(org_owner), [&]( auto& row ) {
  //     row.active = false;
  //   });
  // }


  [[eosio::action]]
  void adduser(name org_owner, uint64_t organization_id, string user_name) {
    check_is_contract_or_owner(org_owner);

    User user;
    user.name = user_name;

    auto orgs = get_orgs(org_owner);
    auto iter = orgs.find(organization_id);
    check(iter != orgs.end(), "Organization not found");

    // TODO: Validate user_name doesn't already exist

    orgs.modify(iter, maybe_charge_to(org_owner), [&](auto& row) {
      row.users.push_back(user);
    });
  }


  // [[eosio::action]]
  // void edituser(name org_owner, uint64_t user_id, string user_name) {
  //   check_is_contract_or_owner(org_owner);
  //
  //   auto users = get_users(org_owner);
  //   auto iter = users.find(user_id);
  //   check(iter != users.end(), "User not found!");
  //
  //   users.modify(iter, maybe_charge_to(org_owner), [&](auto& row) {
  //     row.user_name = user_name;
  //   });
  // }
  //
  //
  // [[eosio::action]]
  // void wipeuser(name org_owner, uint64_t user_id) {
  //   edituser(org_owner, user_id, "");
  // }
  //
  //
  // [[eosio::action]]
  // void addgrantor(name org_owner, uint64_t organization_id, string grantor_name) {
  //   check_is_contract_or_owner(org_owner);
  //
  //   // Not going to check for dupes
  //
  //   auto grantors = get_grantors(org_owner);
  //   uint64_t pk = grantors.available_primary_key();
  //   if (pk == 0) { pk++; }  // Reserve 0 as a NULL value
  //   grantors.emplace(maybe_charge_to(org_owner), [&](auto& row) {
  //     row.key = pk;
  //     row.organization = organization_id;
  //     row.grantor_name = grantor_name;
  //     row.active = true;
  //   });
  // }
  //
  //
  // [[eosio::action]]
  // void editgrantor(name org_owner, uint64_t grantor_id, string grantor_name, bool active) {
  //   check_is_contract_or_owner(org_owner);
  //
  //   auto grantors = get_grantors(org_owner);
  //   auto iter = grantors.find(grantor_id);
  //   check(iter != grantors.end(), "Grantor not found!");
  //
  //   // Not going to check for dupes
  //   grantors.modify(iter, maybe_charge_to(org_owner), [&](auto& row) {
  //     row.grantor_name = grantor_name;
  //     row.active = active;
  //   });
  // }


  [[eosio::action]]
  void grantach(name org_owner, uint64_t organization_id, uint64_t user_id, uint64_t category_id, uint64_t achievement_id, uint64_t grantor_id) {
    check_is_contract_or_owner(org_owner);

    auto orgs = get_orgs(org_owner);
    auto iter = orgs.find(organization_id);
    check(iter != orgs.end(), "Organization not found");

    // TODO: Validate user_id
    // TODO: Validate category_id
    // TODO: Validate achievement_id
    // TODO: Validate grantor_id
    UserAchievement userachievement;
    userachievement.achievement_id = achievement_id;
    userachievement.grantor_id = grantor_id;

    orgs.modify(iter, maybe_charge_to(org_owner), [&](auto& row) {
      row.users[user_id].bycategory[category_id].userachievements.push_back(userachievement);
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

  /**
    An org_owner can run multiple Organizations, each of which will have their
    own entry.
    Categories live within their Organization where category_id is their vector
    index.
  **/
  struct Category {
    string name;
    vector<string> achievements;
  };

  struct UserAchievement {
    uint64_t achievement_id;
    uint64_t grantor_id;
    // timestamp now();
  };

  // Only a separate struct because EOS can't handle directly nested collections
  struct UserAchievementsList {
    vector<UserAchievement> userachievements;
  };

  struct User {
    string name;
    map<uint64_t, UserAchievementsList> bycategory;
  };


  /**
    The ONE and ONLY table that gets written to EOS storage!
  **/
  struct [[eosio::table]] Organization {
    uint64_t key;
    string organization_name;
    vector<Category> categories;
    vector<User> users;
    uint64_t primary_key() const { return key; }
  };
  typedef eosio::multi_index<"orgs"_n, Organization> org_index;   // EOS table names must be <= 12 chars

  org_index get_orgs(name org_owner) {
    // tables all exist within each org_owner's scope
    return org_index(get_self(), org_owner.value);
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

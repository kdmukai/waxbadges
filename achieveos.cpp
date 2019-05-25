#include <eosio/eosio.hpp>
// #include <eosio/time.hpp>

using namespace eosio;
using namespace std;

class [[eosio::contract("achieveos")]] achieveos : public eosio::contract {
public:
  using contract::contract;

  // Constructor
  achieveos(name receiver, name code,  datastream<const char*> ds): contract(receiver, code, ds) {}



  /*****************************************************************************
  * ORGANIZATION
  *****************************************************************************/
  [[eosio::action]]
  void addorg(name org_owner, string organization_name) {
    check_is_contract_or_owner(org_owner);

    auto orgs_table = get_orgs_table_for(org_owner);
    for (auto iter = orgs_table.begin(); iter != orgs_table.end(); iter++) {
      check(iter->name != organization_name, "Organization is not unique");
    }

    orgs_table.emplace(maybe_charge_to(org_owner), [&](auto& org) {
      org.key = orgs_table.available_primary_key();
      org.name = organization_name;
    });
  }


  [[eosio::action]]
  void editorg(name org_owner, uint64_t organization_id, string organization_name) {
    check_is_contract_or_owner(org_owner);

    auto orgs_table = get_orgs_table_for(org_owner);
    auto orgs_iter = orgs_table.find(organization_id);
    check(orgs_iter != orgs_table.end(), "Organization not found");

    for (auto iter = orgs_table.begin(); iter != orgs_table.end(); iter++) {
      check(iter->name != organization_name, "Organization is not unique");
    }

    orgs_table.modify(orgs_iter, maybe_charge_to(org_owner), [&](auto& org) {
      org.name = organization_name;
    });
  }



  /*****************************************************************************
  * CATEGORY
  *****************************************************************************/
  [[eosio::action]]
  void addcat(name org_owner, uint64_t organization_id, string category_name) {
    check_is_contract_or_owner(org_owner);

    auto orgs_table = get_orgs_table_for(org_owner);
    auto orgs_iter = orgs_table.find(organization_id);
    check(orgs_iter != orgs_table.end(), "Organization not found");

    check_name_is_unique(orgs_iter->categories, category_name);

    Category category;
    category.name = category_name;

    orgs_table.modify(orgs_iter, maybe_charge_to(org_owner), [&](auto& org) {
      org.categories.push_back(category);
    });
  }


  [[eosio::action]]
  void editcat(name org_owner, uint64_t organization_id, uint64_t category_id, string category_name) {
    check_is_contract_or_owner(org_owner);

    auto orgs_table = get_orgs_table_for(org_owner);
    auto orgs_iter = orgs_table.find(organization_id);
    check(orgs_iter != orgs_table.end(), "Organization not found");

    check(category_id < orgs_iter->categories.size(), "Category not found");
    check_name_is_unique(orgs_iter->categories, category_name);

    orgs_table.modify(orgs_iter, maybe_charge_to(org_owner), [&](auto& org) {
      org.categories[category_id].name = category_name;
    });
  }


  // // [[eosio::action]]
  // // void removecat(name org_owner, uint64_t categories_id) {
  // //   check_is_contract_or_owner(org_owner);
  // //
  // //   // First zero out any references in Achievements
  // //   auto achievements = get_achievements(org_owner);
  // //   auto cat_achievements = achievements.get_index<"categories"_n>();
  // //   auto ach_iter = cat_achievements.lower_bound(categories_id);
  // //   while (ach_iter != cat_achievements.end()) {
  // //     cat_achievements.modify(ach_iter, maybe_charge_to(org_owner), [&]( auto& org ) {
  // //       org.categories = 0;   // Placeholder for null
  // //     });
  // //     ach_iter++;
  // //   }
  // //
  // //   auto categories = get_categories(org_owner);
  // //   auto orgs_iter = categories.find(categories_id);
  // //   check(orgs_iter != categories.end(), "Categories not found");
  // //   categories.erase(orgs_iter);
  // // }



  /*****************************************************************************
  * ACHIEVEMENT
  *****************************************************************************/
  [[eosio::action]]
  void addach(name org_owner, uint64_t organization_id, uint64_t category_id, string achievement_name) {
    check_is_contract_or_owner(org_owner);

    auto orgs_table = get_orgs_table_for(org_owner);
    auto orgs_iter = orgs_table.find(organization_id);
    check(orgs_iter != orgs_table.end(), "Organization not found");

    check(category_id < orgs_iter->categories.size(), "Category not found");

    auto achievements = orgs_iter->categories[category_id].achievements;
    check_name_is_unique(achievements, achievement_name);

    Achievement achievement;
    achievement.name = achievement_name;
    achievement.active = true;

    orgs_table.modify(orgs_iter, maybe_charge_to(org_owner), [&](auto& org) {
      org.categories[category_id].achievements.push_back(achievement);
    });
  }


  [[eosio::action]]
  void editach(name org_owner, uint64_t organization_id, uint64_t category_id, uint64_t achievement_id, string achievement_name) {
    check_is_contract_or_owner(org_owner);

    auto orgs_table = get_orgs_table_for(org_owner);
    auto orgs_iter = orgs_table.find(organization_id);
    check(orgs_iter != orgs_table.end(), "Organization not found");

    check(category_id < orgs_iter->categories.size(), "Category not found");

    auto achievements = orgs_iter->categories[category_id].achievements;
    check(achievement_id < achievements.size(), "Achievement not found");
    check_name_is_unique(achievements, achievement_name);

    orgs_table.modify(orgs_iter, maybe_charge_to(org_owner), [&](auto& org) {
      org.categories[category_id].achievements[achievement_id].name = achievement_name;
    });
  }


  [[eosio::action]]
  void retireach(name org_owner, uint64_t organization_id, uint64_t category_id, uint64_t achievement_id) {
    check_is_contract_or_owner(org_owner);

    auto orgs_table = get_orgs_table_for(org_owner);
    auto orgs_iter = orgs_table.find(organization_id);
    check(orgs_iter != orgs_table.end(), "Organization not found");

    check(category_id < orgs_iter->categories.size(), "Category not found");

    auto achievements = orgs_iter->categories[category_id].achievements;
    check(achievement_id < achievements.size(), "Achievement not found");

    orgs_table.modify(orgs_iter, maybe_charge_to(org_owner), [&](auto& org) {
      org.categories[category_id].achievements[achievement_id].active = false;
    });
  }



  /*****************************************************************************
  * USER
  *****************************************************************************/
  [[eosio::action]]
  void adduser(name org_owner, uint64_t organization_id, string user_name) {
    check_is_contract_or_owner(org_owner);

    User user;
    user.name = user_name;

    auto orgs_table = get_orgs_table_for(org_owner);
    auto orgs_iter = orgs_table.find(organization_id);
    check(orgs_iter != orgs_table.end(), "Organization not found");

    check_name_is_unique(orgs_iter->users, user_name);

    orgs_table.modify(orgs_iter, maybe_charge_to(org_owner), [&](auto& org) {
      org.users.push_back(user);
    });
  }


  [[eosio::action]]
  void edituser(name org_owner, uint64_t organization_id, uint64_t user_id, string user_name) {
    check_is_contract_or_owner(org_owner);

    auto orgs_table = get_orgs_table_for(org_owner);
    auto orgs_iter = orgs_table.find(organization_id);
    check(orgs_iter != orgs_table.end(), "Organization not found");

    check(user_id < orgs_iter->users.size(), "User not found");

    // Allow blank names in the case of wipeuser
    if (user_name != "") {
      check_name_is_unique(orgs_iter->users, user_name);
    }

    orgs_table.modify(orgs_iter, maybe_charge_to(org_owner), [&](auto& org) {
      org.users[user_id].name = user_name;
    });
  }


  [[eosio::action]]
  void wipeuser(name org_owner, uint64_t organization_id, uint64_t user_id) {
    edituser(org_owner, organization_id, user_id, "");
  }



  /*****************************************************************************
  * GRANTOR
  *****************************************************************************/
  [[eosio::action]]
  void addgrantor(name org_owner, uint64_t organization_id, string grantor_name) {
    check_is_contract_or_owner(org_owner);

    auto orgs_table = get_orgs_table_for(org_owner);
    auto orgs_iter = orgs_table.find(organization_id);
    check(orgs_iter != orgs_table.end(), "Organization not found");

    check_name_is_unique(orgs_iter->grantors, grantor_name);

    Grantor grantor;
    grantor.name = grantor_name;

    orgs_table.modify(orgs_iter, maybe_charge_to(org_owner), [&](auto& org) {
      org.grantors.push_back(grantor);
    });
  }


  [[eosio::action]]
  void editgrantor(name org_owner, uint64_t organization_id, uint64_t grantor_id, string grantor_name) {
    auto orgs_table = get_orgs_table_for(org_owner);
    auto orgs_iter = orgs_table.find(organization_id);
    check(orgs_iter != orgs_table.end(), "Organization not found");

    check(grantor_id < orgs_iter->grantors.size(), "Grantor not found");

    // Allow blank names in the case of wipegrantor
    if (grantor_name != "") {
      check_name_is_unique(orgs_iter->grantors, grantor_name);
    }

    // Not going to check for dupes
    orgs_table.modify(orgs_iter, maybe_charge_to(org_owner), [&](auto& org) {
      org.grantors[grantor_id].name = grantor_name;
    });
  }


  [[eosio::action]]
  void wipegrantor(name org_owner, uint64_t organization_id, uint64_t grantor_id) {
    editgrantor(org_owner, organization_id, grantor_id, "");
  }



  /*****************************************************************************
  * USERACHIEVEMENT
  *****************************************************************************/
  [[eosio::action]]
  void grantach(name org_owner, uint64_t organization_id, uint64_t user_id, uint64_t category_id, uint64_t achievement_id, uint64_t grantor_id) {
    check_is_contract_or_owner(org_owner);

    auto orgs_table = get_orgs_table_for(org_owner);
    auto orgs_iter = orgs_table.find(organization_id);
    check(orgs_iter != orgs_table.end(), "Organization not found");

    check(user_id < orgs_iter->users.size(), "User not found");
    check(category_id < orgs_iter->categories.size(), "Category not found");
    check(achievement_id < orgs_iter->categories[category_id].achievements.size(), "Achievement not found");
    check(orgs_iter->categories[category_id].achievements[achievement_id].active, "Achievement is not active");
    check(grantor_id < orgs_iter->grantors.size(), "Grantor not found");

    UserAchievement userachievement;
    userachievement.achievement_id = achievement_id;
    userachievement.grantor_id = grantor_id;

    orgs_table.modify(orgs_iter, maybe_charge_to(org_owner), [&](auto& org) {
      // We log grants in two directions: on the User and on the Achievement
      org.users[user_id].bycategory[category_id].userachievements.push_back(userachievement);
      org.categories[category_id].achievements[achievement_id].usersgranted.push_back(user_id);
    });
  }


  // [[eosio::action]]
  // void revokeach(name org_owner, uint64_t userachievement_id) {
  //   check_is_contract_or_owner(org_owner);
  //
  //   auto userachievements = get_userachievements(org_owner);
  //   auto orgs_iter = userachievements.find(userachievement_id);
  //   check(orgs_iter != userachievements.end(), "UserAchievement not found");
  //
  //   userachievements.modify(orgs_iter, maybe_charge_to(org_owner), [&]( auto& org ) {
  //     org.revoked = true;
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
  struct Achievement {
    string name;
    bool active = true;
    vector<uint64_t> usersgranted;
  };

  struct Category {
    string name;
    vector<Achievement> achievements;
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

  struct Grantor {
    string name;
  };


  /**
    The ONE and ONLY table that gets written to EOS storage!
  **/
  struct [[eosio::table]] Organization {
    uint64_t key;
    string name;
    vector<Category> categories;
    vector<User> users;
    vector<Grantor> grantors;
    uint64_t primary_key() const { return key; }
  };
  typedef eosio::multi_index<"orgs"_n, Organization> orgs_multi_index;   // EOS table names must be <= 12 chars

  orgs_multi_index get_orgs_table_for(name org_owner) {
    return orgs_multi_index(get_self(), org_owner.value);
  }

  template<class T>
  void check_name_is_unique(vector<T> name_vec, string name) {
    for (auto iter = name_vec.begin(); iter != name_vec.end(); iter++) {
      check(iter->name != name, "Name is not unique");
    }
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

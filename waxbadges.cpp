#include <eosiolib/eosio.hpp>

using namespace eosio;
using namespace std;

inline void check(bool pred, const char* msg) {
   if (!pred) {
      eosio_assert(false, msg);
   }
}

class [[eosio::contract("waxbadges")]] waxbadges : public contract {
public:
  using contract::contract;

  // Constructor
  waxbadges(name receiver, name code,  datastream<const char*> ds)
    : contract(receiver, code, ds) {}



  /*****************************************************************************
  * ECOSYSTEM
  *****************************************************************************/
  [[eosio::action]]
  void addecosys(name ecosystem_owner, string ecosystem_name, string assetbaseurl) {
    check_is_contract_or_owner(ecosystem_owner);

    validateAssetbaseurl(assetbaseurl);

    auto ecosystems_table = get_ecosystems_table_for(ecosystem_owner);
    for (auto iter = ecosystems_table.begin(); iter != ecosystems_table.end(); iter++) {
      check(iter->name != ecosystem_name, "Ecosystem is not unique");
    }

    ecosystems_table.emplace(maybe_charge_to(ecosystem_owner), [&](auto& org) {
      org.key = ecosystems_table.available_primary_key();
      org.name = ecosystem_name;
      org.assetbaseurl = assetbaseurl;  // may be empty string
    });
  }

  void validateAssetbaseurl(string assetbaseurl) {
    check(assetbaseurl.rfind("http", 0) != 0, "assetbaseurl should not include http/https");
    check(assetbaseurl.find("://") == string::npos, "assetbaseurl should not include http:// nor https://");

    check(assetbaseurl.find(" ") == string::npos, "assetbaseurl is not a valid url");
    // TODO: validate actual domain name and path format?
  }


  [[eosio::action]]
  void editecosys(name ecosystem_owner, uint32_t ecosystem_id, string ecosystem_name, string assetbaseurl) {
    check_is_contract_or_owner(ecosystem_owner);

    validateAssetbaseurl(assetbaseurl);

    auto ecosystems_table = get_ecosystems_table_for(ecosystem_owner);
    auto ecosystems_iter = ecosystems_table.find(ecosystem_id);
    check(ecosystems_iter != ecosystems_table.end(), "Ecosystem not found");

    for (auto iter = ecosystems_table.begin(); iter != ecosystems_table.end(); iter++) {
      check(iter->name != ecosystem_name, "Ecosystem is not unique");
    }

    ecosystems_table.modify(ecosystems_iter, maybe_charge_to(ecosystem_owner), [&](auto& org) {
      org.name = ecosystem_name;
      org.assetbaseurl = assetbaseurl;
    });
  }



  /*****************************************************************************
  * CATEGORY
  *****************************************************************************/
  [[eosio::action]]
  void addcat(name ecosystem_owner, uint32_t ecosystem_id, string category_name) {
    check_is_contract_or_owner(ecosystem_owner);

    auto ecosystems_table = get_ecosystems_table_for(ecosystem_owner);
    auto ecosystems_iter = ecosystems_table.find(ecosystem_id);
    check(ecosystems_iter != ecosystems_table.end(), "Ecosystem not found");

    check_name_is_unique(ecosystems_iter->categories, category_name);

    Category category;
    category.name = category_name;

    ecosystems_table.modify(ecosystems_iter, maybe_charge_to(ecosystem_owner), [&](auto& org) {
      org.categories.push_back(category);
    });
  }


  [[eosio::action]]
  void editcat(name ecosystem_owner, uint32_t ecosystem_id, uint32_t category_id, string category_name) {
    check_is_contract_or_owner(ecosystem_owner);

    auto ecosystems_table = get_ecosystems_table_for(ecosystem_owner);
    auto ecosystems_iter = ecosystems_table.find(ecosystem_id);
    check(ecosystems_iter != ecosystems_table.end(), "Ecosystem not found");

    check(category_id < ecosystems_iter->categories.size(), "Category not found");
    check_name_is_unique(ecosystems_iter->categories, category_name);

    ecosystems_table.modify(ecosystems_iter, maybe_charge_to(ecosystem_owner), [&](auto& org) {
      org.categories[category_id].name = category_name;
    });
  }


  // // [[eosio::action]]
  // // void removecat(name ecosystem_owner, uint32_t categories_id) {
  // //   check_is_contract_or_owner(ecosystem_owner);
  // //
  // //   // First zero out any references in Achievements
  // //   auto achievements = get_achievements(ecosystem_owner);
  // //   auto cat_achievements = achievements.get_index<"categories"_n>();
  // //   auto ach_iter = cat_achievements.lower_bound(categories_id);
  // //   while (ach_iter != cat_achievements.end()) {
  // //     cat_achievements.modify(ach_iter, maybe_charge_to(ecosystem_owner), [&]( auto& org ) {
  // //       org.categories = 0;   // Placeholder for null
  // //     });
  // //     ach_iter++;
  // //   }
  // //
  // //   auto categories = get_categories(ecosystem_owner);
  // //   auto ecosystems_iter = categories.find(categories_id);
  // //   check(ecosystems_iter != categories.end(), "Categories not found");
  // //   categories.erase(ecosystems_iter);
  // // }



  /*****************************************************************************
  * ACHIEVEMENT
  *****************************************************************************/
  [[eosio::action]]
  void addach(name ecosystem_owner,
              uint32_t ecosystem_id,
              uint32_t category_id,
              string achievement_name,
              string description,
              string assetname) {
    check_is_contract_or_owner(ecosystem_owner);

    validateAssetname(assetname);

    auto ecosystems_table = get_ecosystems_table_for(ecosystem_owner);
    auto ecosystems_iter = ecosystems_table.find(ecosystem_id);
    check(ecosystems_iter != ecosystems_table.end(), "Ecosystem not found");

    check(category_id < ecosystems_iter->categories.size(), "Category not found");

    auto achievements = ecosystems_iter->categories[category_id].achievements;
    check_name_is_unique(achievements, achievement_name);

    Achievement achievement;
    achievement.name = achievement_name;
    achievement.description = description;
    achievement.assetname = assetname;
    achievement.active = true;

    ecosystems_table.modify(ecosystems_iter, maybe_charge_to(ecosystem_owner), [&](auto& org) {
      org.categories[category_id].achievements.push_back(achievement);
    });
  }


  void validateAssetname(string assetname) {
    check(assetname.find(" ") == string::npos, "Invalid assetname");
  }


  /**
    Editing an achievement risks upsetting our expectation of permanence. For
    now we only allow editing an Achievement that hasn't been granted to any
    Users.
  **/
  [[eosio::action]]
  void editach( name ecosystem_owner,
                uint32_t ecosystem_id,
                uint32_t category_id,
                uint32_t achievement_id,
                string achievement_name,
                string description,
                string assetname) {
    check_is_contract_or_owner(ecosystem_owner);

    validateAssetname(assetname);

    auto ecosystems_table = get_ecosystems_table_for(ecosystem_owner);
    auto ecosystems_iter = ecosystems_table.find(ecosystem_id);
    check(ecosystems_iter != ecosystems_table.end(), "Ecosystem not found");

    check(category_id < ecosystems_iter->categories.size(), "Category not found");

    auto achievements = ecosystems_iter->categories[category_id].achievements;
    check(achievement_id < achievements.size(), "Achievement not found");

    check(
      achievements[achievement_id].usersgranted.size() == 0,
      "Cannot edit an Achievement that has already been granted to a User"
    );

    check_name_is_unique(achievements, achievement_name);

    ecosystems_table.modify(ecosystems_iter, maybe_charge_to(ecosystem_owner), [&](auto& org) {
      org.categories[category_id].achievements[achievement_id].name = achievement_name;
      org.categories[category_id].achievements[achievement_id].description = description;
      org.categories[category_id].achievements[achievement_id].assetname = assetname;
    });
  }


  /**
    For the sake of permanence we cannot allow granted Achievements to be
    deleted. The best we can offer is to "retire" them.

    Note: Achievements that have not been granted to any Users can still be
    edited.
  **/
  [[eosio::action]]
  void retireach(name ecosystem_owner, uint32_t ecosystem_id, uint32_t category_id, uint32_t achievement_id) {
    check_is_contract_or_owner(ecosystem_owner);

    auto ecosystems_table = get_ecosystems_table_for(ecosystem_owner);
    auto ecosystems_iter = ecosystems_table.find(ecosystem_id);
    check(ecosystems_iter != ecosystems_table.end(), "Ecosystem not found");

    check(category_id < ecosystems_iter->categories.size(), "Category not found");

    auto achievements = ecosystems_iter->categories[category_id].achievements;
    check(achievement_id < achievements.size(), "Achievement not found");

    ecosystems_table.modify(ecosystems_iter, maybe_charge_to(ecosystem_owner), [&](auto& org) {
      org.categories[category_id].achievements[achievement_id].active = false;
    });
  }



  /*****************************************************************************
  * USER
  *****************************************************************************/
  [[eosio::action]]
  void adduser(name ecosystem_owner, uint32_t ecosystem_id, string user_name, string userid) {
    check_is_contract_or_owner(ecosystem_owner);

    User user;
    user.name = user_name;
    user.userid = userid;

    auto ecosystems_table = get_ecosystems_table_for(ecosystem_owner);
    auto ecosystems_iter = ecosystems_table.find(ecosystem_id);
    check(ecosystems_iter != ecosystems_table.end(), "Ecosystem not found");

    check_name_is_unique(ecosystems_iter->users, user_name);

    ecosystems_table.modify(ecosystems_iter, maybe_charge_to(ecosystem_owner), [&](auto& org) {
      org.users.push_back(user);
    });
  }


  [[eosio::action]]
  void edituser(name ecosystem_owner, uint32_t ecosystem_id, uint32_t user_id, string user_name, string userid) {
    check_is_contract_or_owner(ecosystem_owner);

    auto ecosystems_table = get_ecosystems_table_for(ecosystem_owner);
    auto ecosystems_iter = ecosystems_table.find(ecosystem_id);
    check(ecosystems_iter != ecosystems_table.end(), "Ecosystem not found");

    check(user_id < ecosystems_iter->users.size(), "User not found");

    // Allow blank names in the case of wipeuser
    if (user_name != "") {
      check_name_is_unique(ecosystems_iter->users, user_name);
    }

    ecosystems_table.modify(ecosystems_iter, maybe_charge_to(ecosystem_owner), [&](auto& org) {
      org.users[user_id].name = user_name;
      org.users[user_id].userid = userid;
    });
  }


  /**
    Meant for possible GDPR, etc compliance issues.
  **/
  [[eosio::action]]
  void wipeusername(name ecosystem_owner, uint32_t ecosystem_id, uint32_t user_id, string userid) {
    edituser(ecosystem_owner, ecosystem_id, user_id, "", userid);
  }



  /*****************************************************************************
  * USER CLAIMS
  *****************************************************************************/
  /**
    The organization owner can authorize its users to "claim" their own User
    entry and link it to the user's WAX account. Logic and permissions around
    claim approvals are entirely up to the organization owner; this contract
    does not do any validation on which WAX accounts should or shouldn't be
    linked to which Users.

    The organization owner must first `approveclaim` below and then the user
    must submit a `claimuser` and pay for the additional RAM themselves.

    Once approved, the organization owner cannot change the linked account.

    TODO: Maybe allow users to transfer their own claims to another account via
    a similar `approve` step that would happen in their MyWaxBadges entry.
  **/
  [[eosio::action]]
  void approveclaim(name ecosystem_owner, uint32_t ecosystem_id, uint32_t user_id, name user_account) {
    // Only the ecosystem_owner can approve User claims
    check(has_auth(ecosystem_owner), "Not authorized");

    auto ecosystems_table = get_ecosystems_table_for(ecosystem_owner);
    auto ecosystems_iter = ecosystems_table.find(ecosystem_id);
    check(ecosystems_iter != ecosystems_table.end(), "Ecosystem not found");

    check(user_id < ecosystems_iter->users.size(), "User not found");

    // Cannot claim an already claimed User
    check(ecosystems_iter->users[user_id].account == "", "User has already been claimed");

    ecosystems_table.modify(ecosystems_iter, maybe_charge_to(ecosystem_owner), [&](auto& org) {
      org.users[user_id].account = user_account.to_string();
    });
  }


  /**
  **/
  bool isapproved(name ecosystem_owner, uint32_t ecosystem_id, uint32_t user_id) {
    auto ecosystems_table = get_ecosystems_table_for(ecosystem_owner);
    auto ecosystems_iter = ecosystems_table.find(ecosystem_id);
    check(ecosystems_iter != ecosystems_table.end(), "Ecosystem not found");

    check(user_id < ecosystems_iter->users.size(), "User not found");

    return ecosystems_iter->users[user_id].account != "";
  }


  /**
    Executed by the User after the ecosystem_owner has completed the `approveclaim`
    step above. If the calling user_account doesn't match the approved
    User.account set above, the claim will fail.

    This will add a MyAchievements entry into the User's own storage
    namespace scope. They will pay the RAM themsevles.
  **/
  [[eosio::action]]
  void claimuser(name ecosystem_owner, uint32_t ecosystem_id, uint32_t user_id, name user_account) {
    // Transaction must be sent by the user_account
    check(has_auth(user_account), "Not authorized");

    auto ecosystems_table = get_ecosystems_table_for(ecosystem_owner);
    auto ecosystems_iter = ecosystems_table.find(ecosystem_id);
    check(ecosystems_iter != ecosystems_table.end(), "Ecosystem not found");

    check(user_id < ecosystems_iter->users.size(), "User not found");

    // Claim must already be approved for this account
    check(ecosystems_iter->users[user_id].account == user_account.to_string(), "User claim has not been approved");

    auto mywaxbadge_table = get_mywaxbadge_table_for(user_account);

    // Make sure the user_account hasn't already claimed this User
    for (auto iter = mywaxbadge_table.begin(); iter != mywaxbadge_table.end(); iter++) {
      check(
        (iter->ecosystem_id != ecosystem_id && iter->user_id != user_id),
        "Already claimed this User");
    }

    // Add the MyWaxBadges entry in the account's namespace.
    // user_account pays for RAM.
    mywaxbadge_table.emplace(user_account, [&](auto& entry) {
      entry.key = mywaxbadge_table.available_primary_key();
      entry.ecosystem_owner = ecosystem_owner.to_string();
      entry.ecosystem_id = ecosystem_id;
      entry.user_id = user_id;
    });

  }


  /*****************************************************************************
  * USERACHIEVEMENT
  *****************************************************************************/
  [[eosio::action]]
  void grantach(name ecosystem_owner, uint32_t ecosystem_id, uint32_t user_id, uint32_t category_id, uint32_t achievement_id, uint32_t timestamp) {
    check_is_contract_or_owner(ecosystem_owner);

    auto ecosystems_table = get_ecosystems_table_for(ecosystem_owner);
    auto ecosystems_iter = ecosystems_table.find(ecosystem_id);
    check(ecosystems_iter != ecosystems_table.end(), "Ecosystem not found");

    check(user_id < ecosystems_iter->users.size(), "User not found");
    check(category_id < ecosystems_iter->categories.size(), "Category not found");
    check(achievement_id < ecosystems_iter->categories[category_id].achievements.size(), "Achievement not found");
    check(ecosystems_iter->categories[category_id].achievements[achievement_id].active, "Achievement is not active");

    UserAchievement userachievement;
    userachievement.achievement_id = achievement_id;
    userachievement.timestamp = timestamp;

    ecosystems_table.modify(ecosystems_iter, maybe_charge_to(ecosystem_owner), [&](auto& org) {
      // We log grants in two directions: on the User and on the Achievement
      org.users[user_id].bycategory[category_id].userachievements.push_back(userachievement);
      org.categories[category_id].achievements[achievement_id].usersgranted.push_back(user_id);
    });
  }



private:
  // All tables will be created in each ecosystem_owner's scope.

  /**
    An ecosystem_owner can run multiple Organizations, each of which will have their
    own entry.
    Categories live within their Ecosystem where category_id is their vector
    index.
  **/
  struct Achievement {
    string name;
    string description;
    string assetname;   // relative to the Org's assetbaseurl: "sometrophy.png" or "subdir/sometrophy.png"
    bool active = true;
    vector<uint32_t> usersgranted;
  };

  struct Category {
    string name;
    vector<Achievement> achievements;
  };

  struct UserAchievement {
    uint32_t achievement_id;
    uint32_t timestamp;   // Unix timestamp
  };

  // Only a separate struct because WAX can't handle directly nested collections
  struct UserAchievementsList {
    vector<UserAchievement> userachievements;
  };

  struct User {
    string name;
    string userid;  // org's internal identifier for this user; can be empty string.
    string account;   // The WAX account that has claimed this user entry, if any. Must be a normal string rather than an eosio::name
    map<uint32_t, UserAchievementsList> bycategory;
  };


  /**
    The ONE and ONLY table that gets written to WAX storage!
  **/
  struct [[eosio::table]] Ecosystem {
    uint32_t key;
    string name;
    string assetbaseurl;  // e.g. "mydomain.com/img/trophies" (omit http/https)
    vector<Category> categories;
    vector<User> users;
    uint32_t primary_key() const { return key; }
  };
  typedef eosio::multi_index<"ecosystems"_n, Ecosystem> ecosystems_multi_index;   // WAX table names must be <= 12 chars

  ecosystems_multi_index get_ecosystems_table_for(name ecosystem_owner) {
    return ecosystems_multi_index(get_self(), ecosystem_owner.value);
  }


  /**
    One additional table so claimed users can quickly identify their various
    Achieveos User entries and associated UserAchievements.
  **/
  struct [[eosio::table]] MyWaxBadges {
    uint32_t key;
    string ecosystem_owner;   // Data resides in ecosystem_owner's storage scope
    uint32_t ecosystem_id;
    uint32_t user_id;
    uint32_t primary_key() const { return key; }
  };
  typedef eosio::multi_index<"mywaxbadges"_n, MyWaxBadges> mywaxbadges_multi_index;   // WAX table names must be <= 12 chars

  mywaxbadges_multi_index get_mywaxbadge_table_for(name account) {
    return mywaxbadges_multi_index(get_self(), account.value);
  }


  /**
    Generic helper class used across multiple structs
  **/
  template<class T>
  void check_name_is_unique(vector<T> name_vec, string name) {
    for (auto iter = name_vec.begin(); iter != name_vec.end(); iter++) {
      check(iter->name != name, "Name is not unique");
    }
  }

/*****************************************************************************
*  Utilities/Helpers
*****************************************************************************/

  /**
    Allow the transaction if it is being executed by the contract account
      itself or if it's the ecosystem_owner acting on his/her own table.
  **/
  void check_is_contract_or_owner(name ecosystem_owner) {
    check(has_auth(get_self()) || has_auth(ecosystem_owner), "Not authorized");
  }


  name maybe_charge_to(name ecosystem_owner) {
    if (has_auth(ecosystem_owner)) {
      return ecosystem_owner;
    } else {
      return get_self();
    }
  }


};


EOSIO_DISPATCH( waxbadges, (addecosys)(editecosys)(addcat)(editcat)(addach)(editach)(retireach)(adduser)(edituser)(wipeusername)(approveclaim)(claimuser)(grantach) )


#include <eosiolib/eosio.hpp>

using namespace eosio;
using namespace std;

inline void check(bool pred, const char* msg) {
   if (!pred) {
      eosio_assert(false, msg);
   }
}

class [[eosio::contract("waxbadge")]] waxbadge : public contract {
public:
  using contract::contract;

  // Constructor
  waxbadge(name receiver, name code,  datastream<const char*> ds)
    : contract(receiver, code, ds) {}



  /*****************************************************************************
  * ORGANIZATION
  *****************************************************************************/
  [[eosio::action]]
  void addorg(name org_owner, string organization_name, string assetbaseurl) {
    check_is_contract_or_owner(org_owner);

    validateAssetbaseurl(assetbaseurl);

    auto orgs_table = get_orgs_table_for(org_owner);
    for (auto iter = orgs_table.begin(); iter != orgs_table.end(); iter++) {
      check(iter->name != organization_name, "Organization is not unique");
    }

    orgs_table.emplace(maybe_charge_to(org_owner), [&](auto& org) {
      org.key = orgs_table.available_primary_key();
      org.name = organization_name;
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
  void editorg(name org_owner, uint32_t organization_id, string organization_name, string assetbaseurl) {
    check_is_contract_or_owner(org_owner);

    validateAssetbaseurl(assetbaseurl);

    auto orgs_table = get_orgs_table_for(org_owner);
    auto orgs_iter = orgs_table.find(organization_id);
    check(orgs_iter != orgs_table.end(), "Organization not found");

    for (auto iter = orgs_table.begin(); iter != orgs_table.end(); iter++) {
      check(iter->name != organization_name, "Organization is not unique");
    }

    orgs_table.modify(orgs_iter, maybe_charge_to(org_owner), [&](auto& org) {
      org.name = organization_name;
      org.assetbaseurl = assetbaseurl;
    });
  }



  /*****************************************************************************
  * CATEGORY
  *****************************************************************************/
  [[eosio::action]]
  void addcat(name org_owner, uint32_t organization_id, string category_name) {
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
  void editcat(name org_owner, uint32_t organization_id, uint32_t category_id, string category_name) {
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
  // // void removecat(name org_owner, uint32_t categories_id) {
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
  void addach(name org_owner,
              uint32_t organization_id,
              uint32_t category_id,
              string achievement_name,
              string description,
              string assetname) {
    check_is_contract_or_owner(org_owner);

    validateAssetname(assetname);

    auto orgs_table = get_orgs_table_for(org_owner);
    auto orgs_iter = orgs_table.find(organization_id);
    check(orgs_iter != orgs_table.end(), "Organization not found");

    check(category_id < orgs_iter->categories.size(), "Category not found");

    auto achievements = orgs_iter->categories[category_id].achievements;
    check_name_is_unique(achievements, achievement_name);

    Achievement achievement;
    achievement.name = achievement_name;
    achievement.description = description;
    achievement.assetname = assetname;
    achievement.active = true;

    orgs_table.modify(orgs_iter, maybe_charge_to(org_owner), [&](auto& org) {
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
  void editach( name org_owner,
                uint32_t organization_id,
                uint32_t category_id,
                uint32_t achievement_id,
                string achievement_name,
                string description,
                string assetname) {
    check_is_contract_or_owner(org_owner);

    validateAssetname(assetname);

    auto orgs_table = get_orgs_table_for(org_owner);
    auto orgs_iter = orgs_table.find(organization_id);
    check(orgs_iter != orgs_table.end(), "Organization not found");

    check(category_id < orgs_iter->categories.size(), "Category not found");

    auto achievements = orgs_iter->categories[category_id].achievements;
    check(achievement_id < achievements.size(), "Achievement not found");

    check(
      achievements[achievement_id].usersgranted.size() == 0,
      "Cannot edit an Achievement that has already been granted to a User"
    );

    check_name_is_unique(achievements, achievement_name);

    orgs_table.modify(orgs_iter, maybe_charge_to(org_owner), [&](auto& org) {
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
  void retireach(name org_owner, uint32_t organization_id, uint32_t category_id, uint32_t achievement_id) {
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
  void adduser(name org_owner, uint32_t organization_id, string user_name, string userid) {
    check_is_contract_or_owner(org_owner);

    User user;
    user.name = user_name;
    user.userid = userid;

    auto orgs_table = get_orgs_table_for(org_owner);
    auto orgs_iter = orgs_table.find(organization_id);
    check(orgs_iter != orgs_table.end(), "Organization not found");

    check_name_is_unique(orgs_iter->users, user_name);

    orgs_table.modify(orgs_iter, maybe_charge_to(org_owner), [&](auto& org) {
      org.users.push_back(user);
    });
  }


  [[eosio::action]]
  void edituser(name org_owner, uint32_t organization_id, uint32_t user_id, string user_name, string userid) {
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
      org.users[user_id].userid = userid;
    });
  }


  /**
    Meant for possible GDPR, etc compliance issues.
  **/
  [[eosio::action]]
  void wipeusername(name org_owner, uint32_t organization_id, uint32_t user_id, string userid) {
    edituser(org_owner, organization_id, user_id, "", userid);
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
  void approveclaim(name org_owner, uint32_t organization_id, uint32_t user_id, name user_account) {
    // Only the org_owner can approve User claims
    check(has_auth(org_owner), "Not authorized");

    auto orgs_table = get_orgs_table_for(org_owner);
    auto orgs_iter = orgs_table.find(organization_id);
    check(orgs_iter != orgs_table.end(), "Organization not found");

    check(user_id < orgs_iter->users.size(), "User not found");

    // Cannot claim an already claimed User
    check(orgs_iter->users[user_id].account == "", "User has already been claimed");

    orgs_table.modify(orgs_iter, maybe_charge_to(org_owner), [&](auto& org) {
      org.users[user_id].account = user_account.to_string();
    });
  }


  /**
  **/
  bool isapproved(name org_owner, uint32_t organization_id, uint32_t user_id) {
    auto orgs_table = get_orgs_table_for(org_owner);
    auto orgs_iter = orgs_table.find(organization_id);
    check(orgs_iter != orgs_table.end(), "Organization not found");

    check(user_id < orgs_iter->users.size(), "User not found");

    return orgs_iter->users[user_id].account != "";
  }


  /**
    Executed by the User after the org_owner has completed the `approveclaim`
    step above. If the calling user_account doesn't match the approved
    User.account set above, the claim will fail.

    This will add a MyAchievements entry into the User's own storage
    namespace scope. They will pay the RAM themsevles.
  **/
  [[eosio::action]]
  void claimuser(name org_owner, uint32_t organization_id, uint32_t user_id, name user_account) {
    // Transaction must be sent by the user_account
    check(has_auth(user_account), "Not authorized");

    auto orgs_table = get_orgs_table_for(org_owner);
    auto orgs_iter = orgs_table.find(organization_id);
    check(orgs_iter != orgs_table.end(), "Organization not found");

    check(user_id < orgs_iter->users.size(), "User not found");

    // Claim must already be approved for this account
    check(orgs_iter->users[user_id].account == user_account.to_string(), "User claim has not been approved");

    auto mywaxbadge_table = get_mywaxbadge_table_for(user_account);

    // Make sure the user_account hasn't already claimed this User
    for (auto iter = mywaxbadge_table.begin(); iter != mywaxbadge_table.end(); iter++) {
      check(
        (iter->organization_id != organization_id && iter->user_id != user_id),
        "Already claimed this User");
    }

    // Add the MyWaxBadges entry in the account's namespace.
    // user_account pays for RAM.
    mywaxbadge_table.emplace(user_account, [&](auto& entry) {
      entry.key = mywaxbadge_table.available_primary_key();
      entry.org_owner = org_owner.to_string();
      entry.organization_id = organization_id;
      entry.user_id = user_id;
    });

  }


  /*****************************************************************************
  * USERACHIEVEMENT
  *****************************************************************************/
  [[eosio::action]]
  void grantach(name org_owner, uint32_t organization_id, uint32_t user_id, uint32_t category_id, uint32_t achievement_id, uint32_t timestamp) {
    check_is_contract_or_owner(org_owner);

    auto orgs_table = get_orgs_table_for(org_owner);
    auto orgs_iter = orgs_table.find(organization_id);
    check(orgs_iter != orgs_table.end(), "Organization not found");

    check(user_id < orgs_iter->users.size(), "User not found");
    check(category_id < orgs_iter->categories.size(), "Category not found");
    check(achievement_id < orgs_iter->categories[category_id].achievements.size(), "Achievement not found");
    check(orgs_iter->categories[category_id].achievements[achievement_id].active, "Achievement is not active");

    UserAchievement userachievement;
    userachievement.achievement_id = achievement_id;
    userachievement.timestamp = timestamp;

    orgs_table.modify(orgs_iter, maybe_charge_to(org_owner), [&](auto& org) {
      // We log grants in two directions: on the User and on the Achievement
      org.users[user_id].bycategory[category_id].userachievements.push_back(userachievement);
      org.categories[category_id].achievements[achievement_id].usersgranted.push_back(user_id);
    });
  }



private:
  // All tables will be created in each org_owner's scope.

  /**
    An org_owner can run multiple Organizations, each of which will have their
    own entry.
    Categories live within their Organization where category_id is their vector
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
  struct [[eosio::table]] Organization {
    uint32_t key;
    string name;
    string assetbaseurl;  // e.g. "mydomain.com/img/trophies" (omit http/https)
    vector<Category> categories;
    vector<User> users;
    uint32_t primary_key() const { return key; }
  };
  typedef eosio::multi_index<"orgs"_n, Organization> orgs_multi_index;   // WAX table names must be <= 12 chars

  orgs_multi_index get_orgs_table_for(name org_owner) {
    return orgs_multi_index(get_self(), org_owner.value);
  }


  /**
    One additional table so claimed users can quickly identify their various
    Achieveos User entries and associated UserAchievements.
  **/
  struct [[eosio::table]] MyWaxBadges {
    uint32_t key;
    string org_owner;   // Data resides in org_owner's storage scope
    uint32_t organization_id;
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
      itself or if it's the org_owner acting on his/her own table.
  **/
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


EOSIO_DISPATCH( waxbadge, (addorg)(editorg)(addcat)(editcat)(addach)(editach)(retireach)(adduser)(edituser)(wipeusername)(approveclaim)(claimuser)(grantach) )


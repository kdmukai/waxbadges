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
  void addecosys(name ecosystem_owner, string ecosystem_name, string description, string website, string assetbaseurl, string logoassetname) {
    check_is_contract_or_owner(ecosystem_owner);

    validateAssetbaseurl(assetbaseurl);

    auto ecosystems_table = get_ecosystems_table_for(get_self());
    for (auto iter = ecosystems_table.begin(); iter != ecosystems_table.end(); iter++) {
      check(iter->name != ecosystem_name, "Ecosystem is not unique");
    }

    ecosystems_table.emplace(maybe_charge_to(ecosystem_owner), [&](auto& ecosystem) {
      ecosystem.key = ecosystems_table.available_primary_key();
      ecosystem.account = ecosystem_owner;
      ecosystem.name = ecosystem_name;
      ecosystem.description = description;
      ecosystem.website = website;
      ecosystem.assetbaseurl = assetbaseurl;  // may be empty string
      ecosystem.logoassetname = logoassetname;
    });
  }

  void validateAssetbaseurl(string assetbaseurl) {
    check(assetbaseurl.rfind("http", 0) != 0, "assetbaseurl should not include http/https");
    check(assetbaseurl.find("://") == string::npos, "assetbaseurl should not include http:// nor https://");

    check(assetbaseurl.find(" ") == string::npos, "assetbaseurl is not a valid url");
    // TODO: validate actual domain name and path format?
  }


  [[eosio::action]]
  void editecosys(  name ecosystem_owner,
                    uint32_t ecosystem_id,
                    string ecosystem_name,
                    string description,
                    string website,
                    string assetbaseurl,
                    string logoassetname) {
    check_is_contract_or_owner(ecosystem_owner);

    validateAssetbaseurl(assetbaseurl);

    auto ecosystems_table = get_ecosystems_table_for(get_self());
    auto ecosystems_iter = ecosystems_table.find(ecosystem_id);
    check(ecosystems_iter != ecosystems_table.end(), "Ecosystem not found");

    for (auto iter = ecosystems_table.begin(); iter != ecosystems_table.end(); iter++) {
      if (iter->key != ecosystem_id) {
        check(iter->name != ecosystem_name, "Ecosystem is not unique");
      }
    }

    // The ecosystem_owner must match the Ecosystem.account to make changes
    check(ecosystems_iter->account == ecosystem_owner, "Not authorized");

    ecosystems_table.modify(ecosystems_iter, maybe_charge_to(ecosystem_owner), [&](auto& ecosystem) {
      ecosystem.name = ecosystem_name;
      ecosystem.description = description;
      ecosystem.website = website;
      ecosystem.assetbaseurl = assetbaseurl;  // may be empty string
      ecosystem.logoassetname = logoassetname;
    });
  }



  [[eosio::action]]
  void ecosysowner( name ecosystem_owner,
                    uint32_t ecosystem_id,
                    name new_owner) {
    check_is_contract_or_owner(ecosystem_owner);

    auto ecosystems_table = get_ecosystems_table_for(get_self());
    auto ecosystems_iter = ecosystems_table.find(ecosystem_id);
    check(ecosystems_iter != ecosystems_table.end(), "Ecosystem not found");

    // The ecosystem_owner must match the Ecosystem.account to make changes
    check(ecosystems_iter->account == ecosystem_owner, "Not authorized");

    ecosystems_table.modify(ecosystems_iter, maybe_charge_to(ecosystem_owner), [&](auto& ecosystem) {
      ecosystem.account = new_owner;
    });
  }



  /**
    For the sake of permanence we can only delete the Ecosystem if there are no
    Achievements or if none of the Achievements have been granted to Users.

    Ecosystems are keyed on a normal table ID via 'available_primary_key()' so
    deleting one won't cause an ID shift the way it would for the embedded
    vector<> objects which are keyed on index position (e.g. Category).
  **/
  [[eosio::action]]
  void rmecosys(name ecosystem_owner, uint32_t ecosystem_id) {
    check_is_contract_or_owner(ecosystem_owner);
  
    auto ecosystems_table = get_ecosystems_table_for(get_self());
    auto ecosystems_iter = ecosystems_table.find(ecosystem_id);
    check(ecosystems_iter != ecosystems_table.end(), "Ecosystem not found");

    // The ecosystem_owner must match the Ecosystem.account to make changes
    check(ecosystems_iter->account == ecosystem_owner, "Not authorized");

    auto categories = ecosystems_iter->categories;

    int num_grants = 0;
    for (auto category_iter = categories.begin(); category_iter < categories.end(); category_iter++) {
      auto achievements = category_iter->achievements;
      for (auto achievement_iter = achievements.begin(); achievement_iter < achievements.end(); achievement_iter++) {
        num_grants += achievement_iter->usergrants.size();
      }
    }

    check(num_grants == 0, "Cannot remove an Ecosystem with granted Achievements");

    ecosystems_table.erase(ecosystems_iter);
  }

  /*****************************************************************************
  * CATEGORY
  *****************************************************************************/
  [[eosio::action]]
  void addcat(name ecosystem_owner, uint32_t ecosystem_id, string category_name) {
    check_is_contract_or_owner(ecosystem_owner);

    auto ecosystems_table = get_ecosystems_table_for(get_self());
    auto ecosystems_iter = ecosystems_table.find(ecosystem_id);
    check(ecosystems_iter != ecosystems_table.end(), "Ecosystem not found");

    // The ecosystem_owner must match the Ecosystem.account to make changes
    check(ecosystems_iter->account == ecosystem_owner, "Not authorized");

    check_name_is_unique(ecosystems_iter->categories, category_name);

    Category category;
    category.name = category_name;

    ecosystems_table.modify(ecosystems_iter, maybe_charge_to(ecosystem_owner), [&](auto& ecosystem) {
      ecosystem.categories.push_back(category);
    });
  }


  [[eosio::action]]
  void editcat(name ecosystem_owner, uint32_t ecosystem_id, uint32_t category_id, string category_name) {
    check_is_contract_or_owner(ecosystem_owner);

    auto ecosystems_table = get_ecosystems_table_for(get_self());
    auto ecosystems_iter = ecosystems_table.find(ecosystem_id);
    check(ecosystems_iter != ecosystems_table.end(), "Ecosystem not found");

    // The ecosystem_owner must match the Ecosystem.account to make changes
    check(ecosystems_iter->account == ecosystem_owner, "Not authorized");

    check(category_id < ecosystems_iter->categories.size(), "Category not found");
    check_name_is_unique(ecosystems_iter->categories, category_name, category_id);

    ecosystems_table.modify(ecosystems_iter, maybe_charge_to(ecosystem_owner), [&](auto& ecosystem) {
      ecosystem.categories[category_id].name = category_name;
    });
  }


  /**
    Deletes the last Category in the Organization. We only allow deleting from
    the end because we rely on vector index position as the Category key;
    deleting from the middle would break references (e.g. social media
    url shares that include specific category_id values).

    For the sake of permanence we can only delete the Category if there are no
    Achievements or if none of the Achievements have been granted to Users.
  **/
  [[eosio::action]]
  void rmlastcat(name ecosystem_owner, uint32_t ecosystem_id) {
    check_is_contract_or_owner(ecosystem_owner);
  
    auto ecosystems_table = get_ecosystems_table_for(get_self());
    auto ecosystems_iter = ecosystems_table.find(ecosystem_id);
    check(ecosystems_iter != ecosystems_table.end(), "Ecosystem not found");

    // The ecosystem_owner must match the Ecosystem.account to make changes
    check(ecosystems_iter->account == ecosystem_owner, "Not authorized");
    check(ecosystems_iter->categories.size() > 0, "No categories to remove");

    auto last_category = ecosystems_iter->categories[ecosystems_iter->categories.size() - 1];
    auto achievements = last_category.achievements;

    int num_grants = 0;
    for (auto achievement_iter = achievements.begin(); achievement_iter < achievements.end(); achievement_iter++) {
      num_grants += achievement_iter->usergrants.size();
    }

    check(num_grants == 0, "Cannot remove a Category with granted Achievements");

    ecosystems_table.modify(ecosystems_iter, maybe_charge_to(ecosystem_owner), [&](auto& ecosystem) {
      ecosystem.categories.pop_back();
    });
  }



  /*****************************************************************************
  * ACHIEVEMENT
  *****************************************************************************/
  [[eosio::action]]
  void addach(name ecosystem_owner,
              uint32_t ecosystem_id,
              uint32_t category_id,
              string achievement_name,
              string description,
              string assetname,
              uint32_t maxquantity) {
    check_is_contract_or_owner(ecosystem_owner);

    validateAssetname(assetname);

    auto ecosystems_table = get_ecosystems_table_for(get_self());
    auto ecosystems_iter = ecosystems_table.find(ecosystem_id);
    check(ecosystems_iter != ecosystems_table.end(), "Ecosystem not found");

    // The ecosystem_owner must match the Ecosystem.account to make changes
    check(ecosystems_iter->account == ecosystem_owner, "Not authorized");

    check(category_id < ecosystems_iter->categories.size(), "Category not found");

    auto achievements = ecosystems_iter->categories[category_id].achievements;
    check_name_is_unique(achievements, achievement_name);

    Achievement achievement;
    achievement.name = achievement_name;
    achievement.description = description;
    achievement.assetname = assetname;
    achievement.maxquantity = maxquantity;
    achievement.active = true;

    ecosystems_table.modify(ecosystems_iter, maybe_charge_to(ecosystem_owner), [&](auto& org) {
      org.categories[category_id].achievements.push_back(achievement);
    });
  }


  void validateAssetname(string assetname) {
    check(assetname.find(" ") == string::npos, "Invalid assetname");
  }


  /**
    Editing an achievement risks upsetting our expectation of permanence. The
    full Achievement details can only be updated if it hasn't already been
    granted to a User (see editachasset to just update the assetname).
  **/
  [[eosio::action]]
  void editach( name ecosystem_owner,
                uint32_t ecosystem_id,
                uint32_t category_id,
                uint32_t achievement_id,
                string achievement_name,
                string description,
                string assetname,
                uint32_t maxquantity) {
    check_is_contract_or_owner(ecosystem_owner);

    validateAssetname(assetname);

    auto ecosystems_table = get_ecosystems_table_for(get_self());
    auto ecosystems_iter = ecosystems_table.find(ecosystem_id);
    check(ecosystems_iter != ecosystems_table.end(), "Ecosystem not found");

    // The ecosystem_owner must match the Ecosystem.account to make changes
    check(ecosystems_iter->account == ecosystem_owner, "Not authorized");

    check(category_id < ecosystems_iter->categories.size(), "Category not found");

    auto achievements = ecosystems_iter->categories[category_id].achievements;
    check(achievement_id < achievements.size(), "Achievement not found");

    check(
      achievements[achievement_id].usergrants.size() == 0,
      "Cannot edit an Achievement that has already been granted to a User"
    );

    check_name_is_unique(achievements, achievement_name, achievement_id);

    ecosystems_table.modify(ecosystems_iter, maybe_charge_to(ecosystem_owner), [&](auto& ecosystem) {
      ecosystem.categories[category_id].achievements[achievement_id].name = achievement_name;
      ecosystem.categories[category_id].achievements[achievement_id].description = description;
      ecosystem.categories[category_id].achievements[achievement_id].assetname = assetname;
      ecosystem.categories[category_id].achievements[achievement_id].maxquantity = maxquantity;
    });
  }


  [[eosio::action]]
  void editachasset(name ecosystem_owner,
                    uint32_t ecosystem_id,
                    uint32_t category_id,
                    uint32_t achievement_id,
                    string assetname) {
    check_is_contract_or_owner(ecosystem_owner);

    validateAssetname(assetname);

    auto ecosystems_table = get_ecosystems_table_for(get_self());
    auto ecosystems_iter = ecosystems_table.find(ecosystem_id);
    check(ecosystems_iter != ecosystems_table.end(), "Ecosystem not found");

    // The ecosystem_owner must match the Ecosystem.account to make changes
    check(ecosystems_iter->account == ecosystem_owner, "Not authorized");

    check(category_id < ecosystems_iter->categories.size(), "Category not found");

    auto achievements = ecosystems_iter->categories[category_id].achievements;
    check(achievement_id < achievements.size(), "Achievement not found");

    // Can update assetname regardless of whether or not the Achievement has
    //  already been granted at least once.
    ecosystems_table.modify(ecosystems_iter, maybe_charge_to(ecosystem_owner), [&](auto& ecosystem) {
      ecosystem.categories[category_id].achievements[achievement_id].assetname = assetname;
    });
  }


  /**
    Deletes the last Achievement in the Category. We only allow deleting from
    the end because we rely on vector index position as the Achievement key;
    deleting from the middle would break references (e.g. social media
    url shares that include specific category_id values).

    For the sake of permanence we can only delete the Achievement if it has not
    been granted to any Users yet.
  **/
  [[eosio::action]]
  void rmlastach( name ecosystem_owner,
                uint32_t ecosystem_id,
                uint32_t category_id) {
    check_is_contract_or_owner(ecosystem_owner);

    auto ecosystems_table = get_ecosystems_table_for(get_self());
    auto ecosystems_iter = ecosystems_table.find(ecosystem_id);
    check(ecosystems_iter != ecosystems_table.end(), "Ecosystem not found");

    // The ecosystem_owner must match the Ecosystem.account to make changes
    check(ecosystems_iter->account == ecosystem_owner, "Not authorized");

    check(category_id < ecosystems_iter->categories.size(), "Category not found");

    auto achievements = ecosystems_iter->categories[category_id].achievements;

    check(
      achievements[achievements.size() - 1].usergrants.size() == 0,
      "Cannot delete an Achievement that has already been granted to a User"
    );

    ecosystems_table.modify(ecosystems_iter, maybe_charge_to(ecosystem_owner), [&](auto& ecosystem) {
      ecosystem.categories[category_id].achievements.pop_back();
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

    auto ecosystems_table = get_ecosystems_table_for(get_self());
    auto ecosystems_iter = ecosystems_table.find(ecosystem_id);
    check(ecosystems_iter != ecosystems_table.end(), "Ecosystem not found");

    // The ecosystem_owner must match the Ecosystem.account to make changes
    check(ecosystems_iter->account == ecosystem_owner, "Not authorized");

    check(category_id < ecosystems_iter->categories.size(), "Category not found");

    auto achievements = ecosystems_iter->categories[category_id].achievements;
    check(achievement_id < achievements.size(), "Achievement not found");

    ecosystems_table.modify(ecosystems_iter, maybe_charge_to(ecosystem_owner), [&](auto& ecosystem) {
      ecosystem.categories[category_id].achievements[achievement_id].active = false;
    });
  }



  /*****************************************************************************
  * USER
  *****************************************************************************/
  [[eosio::action]]
  void adduser(name ecosystem_owner, uint32_t ecosystem_id, string user_name, string userid, string avatarurl) {
    check_is_contract_or_owner(ecosystem_owner);

    User user;
    user.name = user_name;
    user.userid = userid;
    user.avatarurl = avatarurl;

    auto ecosystems_table = get_ecosystems_table_for(get_self());
    auto ecosystems_iter = ecosystems_table.find(ecosystem_id);
    check(ecosystems_iter != ecosystems_table.end(), "Ecosystem not found");

    // The ecosystem_owner must match the Ecosystem.account to make changes
    check(ecosystems_iter->account == ecosystem_owner, "Not authorized");

    check_name_is_unique(ecosystems_iter->users, user_name);

    ecosystems_table.modify(ecosystems_iter, maybe_charge_to(ecosystem_owner), [&](auto& ecosystem) {
      ecosystem.users.push_back(user);
    });
  }


  [[eosio::action]]
  void edituser(name ecosystem_owner, uint32_t ecosystem_id, uint32_t user_id, string user_name, string userid, string avatarurl) {
    check_is_contract_or_owner(ecosystem_owner);

    auto ecosystems_table = get_ecosystems_table_for(get_self());
    auto ecosystems_iter = ecosystems_table.find(ecosystem_id);
    check(ecosystems_iter != ecosystems_table.end(), "Ecosystem not found");

    // The ecosystem_owner must match the Ecosystem.account to make changes
    check(ecosystems_iter->account == ecosystem_owner, "Not authorized");

    check(user_id < ecosystems_iter->users.size(), "User not found");

    check_name_is_unique(ecosystems_iter->users, user_name, user_id);

    ecosystems_table.modify(ecosystems_iter, maybe_charge_to(ecosystem_owner), [&](auto& ecosystem) {
      ecosystem.users[user_id].name = user_name;
      ecosystem.users[user_id].userid = userid;
      ecosystem.users[user_id].avatarurl = avatarurl;      
    });
  }


  /**
    Meant for possible GDPR, etc compliance issues.
  **/
  [[eosio::action]]
  void wipeusername(name ecosystem_owner, uint32_t ecosystem_id, uint32_t user_id) {
    check_is_contract_or_owner(ecosystem_owner);

    auto ecosystems_table = get_ecosystems_table_for(get_self());
    auto ecosystems_iter = ecosystems_table.find(ecosystem_id);
    check(ecosystems_iter != ecosystems_table.end(), "Ecosystem not found");

    // The ecosystem_owner must match the Ecosystem.account to make changes
    check(ecosystems_iter->account == ecosystem_owner, "Not authorized");

    check(user_id < ecosystems_iter->users.size(), "User not found");

    ecosystems_table.modify(ecosystems_iter, maybe_charge_to(ecosystem_owner), [&](auto& ecosystem) {
      ecosystem.users[user_id].name = "";      
    });
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
  **/
  [[eosio::action]]
  void approveclaim(name ecosystem_owner, uint32_t ecosystem_id, uint32_t user_id, name user_account) {
    check_is_contract_or_owner(ecosystem_owner);

    auto ecosystems_table = get_ecosystems_table_for(get_self());
    auto ecosystems_iter = ecosystems_table.find(ecosystem_id);
    check(ecosystems_iter != ecosystems_table.end(), "Ecosystem not found");

    // The ecosystem_owner must match the Ecosystem.account to make changes
    check(ecosystems_iter->account == ecosystem_owner, "Not authorized");

    check(user_id < ecosystems_iter->users.size(), "User not found");

    // Cannot claim an already claimed User
    check(ecosystems_iter->users[user_id].account == "", "User has already been claimed");

    ecosystems_table.modify(ecosystems_iter, maybe_charge_to(ecosystem_owner), [&](auto& ecosystem) {
      ecosystem.users[user_id].account = user_account.to_string();
    });
  }



  /**
    Executed by the User after the ecosystem_owner has completed the `approveclaim`
    step above. If the calling user_account doesn't match the approved
    User.account set above, the claim will fail.

    This will add a MyAchievements entry for the User's claim. They will pay
    the RAM themselves.
  **/
  [[eosio::action]]
  void claimuser(uint32_t ecosystem_id, uint32_t user_id, name user_account) {
    // Transaction must be sent by the user_account
    check(has_auth(user_account), "Not authorized");

    auto ecosystems_table = get_ecosystems_table_for(get_self());
    auto ecosystems_iter = ecosystems_table.find(ecosystem_id);
    check(ecosystems_iter != ecosystems_table.end(), "Ecosystem not found");

    check(user_id < ecosystems_iter->users.size(), "User not found");

    // Claim must already be approved for this account
    check(ecosystems_iter->users[user_id].account == user_account.to_string(), "User claim has not been approved");

    auto mywaxbadge_table = get_mywaxbadge_table_for(get_self());

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
      entry.account = user_account;
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

    auto ecosystems_table = get_ecosystems_table_for(get_self());
    auto ecosystems_iter = ecosystems_table.find(ecosystem_id);
    check(ecosystems_iter != ecosystems_table.end(), "Ecosystem not found");

    // The ecosystem_owner must match the Ecosystem.account to make changes
    check(ecosystems_iter->account == ecosystem_owner, "Not authorized");

    check(user_id < ecosystems_iter->users.size(), "User not found");
    check(category_id < ecosystems_iter->categories.size(), "Category not found");
    check(achievement_id < ecosystems_iter->categories[category_id].achievements.size(), "Achievement not found");

    auto achievement = ecosystems_iter->categories[category_id].achievements[achievement_id];
    check(achievement.active, "Achievement is not active");
    check(achievement.maxquantity == 0 || achievement.usergrants.size() < achievement.maxquantity, "Achievement max quantity has been reached");

    // Avoid duplicate grants
    for (auto usergrant_iter = achievement.usergrants.begin(); usergrant_iter < achievement.usergrants.end(); usergrant_iter++) {
      check(user_id != usergrant_iter->user_id, "Achievement already granted to this User");
    }

    UserAchievement userachievement;
    userachievement.category_id = category_id;
    userachievement.achievement_id = achievement_id;
    userachievement.timestamp = timestamp;

    UserGrant usergrant;
    usergrant.user_id = user_id;
    usergrant.timestamp = timestamp;

    ecosystems_table.modify(ecosystems_iter, maybe_charge_to(ecosystem_owner), [&](auto& ecosystem) {
      // We log grants in two directions: on the User and on the Achievement
      ecosystem.users[user_id].userachievements.push_back(userachievement);
      ecosystem.categories[category_id].achievements[achievement_id].usergrants.push_back(usergrant);
    });
  }




  /*****************************************************************************
  * CONTRACT MIGRATION/MANAGEMENT
  *****************************************************************************/

  /**
    One-time cleanup step before schema change
  **/
  [[eosio::action]]
  void wipetables() {
    check(has_auth(get_self()), "Not authorized");

    auto ecosystems_table = get_ecosystems_table_for(get_self());
    auto it = ecosystems_table.begin();
    while (it != ecosystems_table.end()) {
        it = ecosystems_table.erase(it);
    }
  }



private:
  // All tables will be created in the contract's scope

  struct UserGrant {
    uint32_t user_id;
    uint32_t timestamp;   // Unix timestamp
  };

  struct Achievement {
    string name;
    string description;
    string assetname;   // relative to the Ecosystem's assetbaseurl: "sometrophy.png" or "subdir/sometrophy.png"
    uint32_t maxquantity;   // 0 == no max set
    bool active = true;
    vector<UserGrant> usergrants;
  };

  struct Category {
    string name;
    vector<Achievement> achievements;
  };

  struct UserAchievement {
    uint32_t category_id;
    uint32_t achievement_id;
    uint32_t timestamp;   // Unix timestamp
  };

  struct User {
    string name;      // display name
    string userid;    // org's internal identifier for this user
    string avatarurl; // Asset relative to Ecosystem.assetbaseurl or full http/https url.
    string account;   // The WAX account that has claimed this user entry, if any. Must be a normal string rather than an eosio::name
    vector<UserAchievement> userachievements;
  };


  /**
    An ecosystem_owner can run multiple Ecosystems, each of which will have their
    own Ecosystems entry.
  **/
  struct [[eosio::table]] Ecosystem {
    uint32_t key;
    name account;
    string name;
    string description;
    string website;
    string assetbaseurl;  // e.g. "mydomain.com/img/trophies" (omit http/https)
    string logoassetname;     // just the filename within the above assetbaseurl
    vector<Category> categories;
    vector<User> users;
    uint32_t primary_key() const { return key; }
    uint64_t get_secondary_1() const { return account.value;}
  };
  typedef eosio::multi_index<
    "ecosystems"_n, Ecosystem, indexed_by<
      "byaccount"_n, const_mem_fun<
        Ecosystem, uint64_t, &Ecosystem::get_secondary_1
      >
    >
  > ecosystems_multi_index;

  ecosystems_multi_index get_ecosystems_table_for(name ecosystem_owner) {
    return ecosystems_multi_index(get_self(), ecosystem_owner.value);
  }


  /**
    One additional table so claimed users can quickly identify their various
    Achieveos User entries and associated UserAchievements.
  **/
  struct [[eosio::table]] MyWaxBadges {
    uint32_t key;
    name account;
    uint32_t ecosystem_id;
    uint32_t user_id;
    uint32_t primary_key() const { return key; }
    uint64_t get_secondary_1() const { return account.value;}
  };
  typedef eosio::multi_index<
    "mywaxbadges"_n, MyWaxBadges, indexed_by<
      "byaccount"_n, const_mem_fun<
        MyWaxBadges, uint64_t, &MyWaxBadges::get_secondary_1
      >
    >
  > mywaxbadges_multi_index;

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
  template<class T>
  void check_name_is_unique(vector<T> name_vec, string name, uint32_t key) {
    for (auto iter = name_vec.begin(); iter != name_vec.end(); iter++) {
      uint32_t cur_index = iter - name_vec.begin();
      if (cur_index != key) {
        check(iter->name != name, "Name is not unique");
      }
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


EOSIO_DISPATCH( waxbadges, (addecosys)(editecosys)(ecosysowner)(rmecosys)(addcat)(editcat)(rmlastcat)(addach)(editach)(editachasset)(rmlastach)(retireach)(adduser)(edituser)(wipeusername)(approveclaim)(claimuser)(grantach)(wipetables) )


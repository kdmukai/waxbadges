import argparse
import sys
import time
import unittest

from eosfactory.eosf import *


verbosity([Verbosity.INFO, Verbosity.OUT, Verbosity.TRACE, Verbosity.DEBUG])

CONTRACT_WORKSPACE = "waxbadges"

# Actors of the test:
MASTER = MasterAccount()
CONTRACT = Account()
STUDIOA = Account()
STUDIOB = Account()
STUDIOC = Account()
BOB = Account()
CAROL = Account()
DAVE = Account()
ELLEN = Account()

skip_recompile = False

class Test(unittest.TestCase):

    def run(self, result=None):
        super().run(result)

    @classmethod
    def setUpClass(cls):
        SCENARIO("Reset local node, create test accounts, and run all tests")
        reset()
        create_master_account("MASTER")

        COMMENT("Create test accounts:")
        create_account("STUDIOA", MASTER)
        create_account("STUDIOB", MASTER)
        create_account("STUDIOC", MASTER)
        create_account("BOB", MASTER)
        create_account("CAROL", MASTER)
        create_account("DAVE", MASTER)
        create_account("ELLEN", MASTER)


    def test_0001_deploy(self):
        COMMENT("Create, build and deploy the contract:")
        create_account("CONTRACT", MASTER)
        smart = Contract(CONTRACT, CONTRACT_WORKSPACE)
        if not skip_recompile:
            smart.build()
        smart.deploy()


    def test_0100_account_create_ecosystem(self):
        COMMENT("Should allow an WAX account to create an Ecosystem")
        ecosystem_name = "Tournament of Champions"
        STUDIOA.info()
        CONTRACT.push_action(
            "addecosys",
            {
                "ecosystem_owner": STUDIOA,
                "ecosystem_name": ecosystem_name,
                "description": "This is the Ecosystem description",
                "website": "https://somedomain.com",
                "assetbaseurl": "somedomainname.com/imgs/trophies",
                "logoassetname": "company_logo.png"
            },
            permission=(STUDIOA, Permission.ACTIVE)
        )

        table = CONTRACT.table("ecosystems", CONTRACT)
        self.assertEqual(table.json["rows"][0]["name"], ecosystem_name)
        STUDIOA.info()


    def test_0101_contract_create_ecosystem(self):
        COMMENT("Should allow the contract owner to create an Ecosystem on behalf of ecosystem_owner")
        ecosystem_name = "Some Other STUDIOA Ecosystem"
        CONTRACT.push_action(
            "addecosys",
            {
                "ecosystem_owner": STUDIOA,
                "ecosystem_name": ecosystem_name,
                "description": "This is the Ecosystem description",
                "website": "https://somedomain.com",
                "assetbaseurl": "someotherdomainname.com/assets",
                "logoassetname": "other_logo.png"
            },
            permission=(CONTRACT, Permission.ACTIVE)
        )

        table = CONTRACT.table("ecosystems", CONTRACT)
        self.assertEqual(table.json["rows"][1]["name"], ecosystem_name)


    def test_0102_ecosystem_unique_name(self):
        COMMENT("Should throw an exception when creating an Ecosystem with an existing name")
        ecosystem_name = "Tournament of Champions"
        with self.assertRaises(Exception) as e:
            CONTRACT.push_action(
                "addecosys",
                {
                    "ecosystem_owner": STUDIOA,
                    "ecosystem_name": ecosystem_name,
                    "description": "This is the Ecosystem description",
                    "website": "https://somedomain.com",
                    "assetbaseurl": "someotherdomainname.com/assets",
                    "logoassetname": "company_logo.png"
                },
                permission=(STUDIOA, Permission.ACTIVE)
            )

        err_msg = str(e.exception)
        self.assertTrue("not unique" in err_msg, err_msg)


    def test_0110_other_create_ecosystem(self):
        COMMENT("Should not allow a different account to create an Ecosystem on behalf of ecosystem_owner")
        ecosystem_name = "Not Allowed Ecosystem"
        with self.assertRaises(Exception) as e:
            CONTRACT.push_action(
                "addecosys",
                {
                    "ecosystem_owner": STUDIOA,
                    "ecosystem_name": ecosystem_name,
                    "description": "This is the Ecosystem description",
                    "website": "https://somedomain.com",
                    "assetbaseurl": "someotherdomainname.com/assets",
                    "logoassetname": "bobs_logo.png"
                },
                permission=(BOB, Permission.ACTIVE)
            )

        err_msg = str(e.exception)
        self.assertTrue("Not authorized" in err_msg, err_msg)


    def test_0120_transfer_ecosystem_owner(self):
        COMMENT("Should allow the Ecosystem owner to transfer Ecosystem to a new owner")
        ecosystem_name = "An Ecosystem we'll transfer"

        table = CONTRACT.table("ecosystems", CONTRACT)
        num_ecosystems = len(table.json["rows"])

        CONTRACT.push_action(
            "addecosys",
            {
                "ecosystem_owner": STUDIOA,
                "ecosystem_name": ecosystem_name,
                "description": "This is the Ecosystem description",
                "website": "https://yetanotherdomainname.com",
                "assetbaseurl": "yetanotherdomainname.com/static",
                "logoassetname": "yadn_logo.png"
            },
            permission=(STUDIOA, Permission.ACTIVE)
        )

        table = CONTRACT.table("ecosystems", CONTRACT)
        new_ecosystem_id = len(table.json["rows"]) - 1
        self.assertEqual(table.json["rows"][new_ecosystem_id]["name"], ecosystem_name)

        CONTRACT.push_action(
            "ecosysowner",
            {
                "ecosystem_owner": STUDIOA,
                "ecosystem_id": new_ecosystem_id,
                "new_owner": STUDIOC
            },
            permission=(STUDIOA, Permission.ACTIVE)
        )

        table = CONTRACT.table("ecosystems", CONTRACT)
        self.assertEqual(table.json["rows"][new_ecosystem_id]["account"], STUDIOC.name)

        # And now STUDIOC is free to make owner-only changes
        ecosystem_name = "Renamed by STUDIOC"
        CONTRACT.push_action(
            "editecosys",
            {
                "ecosystem_owner": STUDIOC,
                "ecosystem_id": new_ecosystem_id,
                "ecosystem_name": ecosystem_name,
                "description": "This is the Ecosystem description",
                "website": "https://somedomain.com",
                "assetbaseurl": "someotherdomainname.com/assets",
                "logoassetname": "company_logo.png"
            },
            permission=(STUDIOC, Permission.ACTIVE)
        )

        table = CONTRACT.table("ecosystems", CONTRACT)
        self.assertEqual(table.json["rows"][new_ecosystem_id]["name"], ecosystem_name)


    def test_0130_delete_ecosystem(self):
        COMMENT("Should allow a WAX account to delete an Ecosystem")
        ecosystem_name = "An Ecosystem we'll delete"

        table = CONTRACT.table("ecosystems", CONTRACT)
        num_ecosystems = len(table.json["rows"])

        CONTRACT.push_action(
            "addecosys",
            {
                "ecosystem_owner": STUDIOA,
                "ecosystem_name": ecosystem_name,
                "description": "This is the Ecosystem description",
                "website": "https://yetanotherdomainname.com",
                "assetbaseurl": "yetanotherdomainname.com/static",
                "logoassetname": "yadn_logo.png"
            },
            permission=(STUDIOA, Permission.ACTIVE)
        )

        table = CONTRACT.table("ecosystems", CONTRACT)
        num_ecosystems = len(table.json["rows"])
        new_ecosystem_id = num_ecosystems - 1
        self.assertEqual(table.json["rows"][new_ecosystem_id]["name"], ecosystem_name)

        CONTRACT.push_action(
            "rmecosys",
            {
                "ecosystem_owner": STUDIOA,
                "ecosystem_id": new_ecosystem_id
            },
            permission=(STUDIOA, Permission.ACTIVE)
        )

        table = CONTRACT.table("ecosystems", CONTRACT)
        self.assertEqual(len(table.json["rows"]), num_ecosystems - 1, table.json["rows"])


    def test_0200_account_create_category(self):
        COMMENT("Should allow an WAX account to create a Category in its Ecosystem")
        category_name = "First Category"
        CONTRACT.push_action(
            "addcat",
            {
                "ecosystem_owner": STUDIOA,
                "ecosystem_id": 0,
                "category_name": category_name
            },
            permission=(STUDIOA, Permission.ACTIVE)
        )

        table = CONTRACT.table("ecosystems", CONTRACT)
        self.assertEqual(table.json["rows"][0]["categories"][0]["name"], category_name)


    def test_0300_account_create_achievement(self):
        COMMENT("Should allow an WAX account to create an Achievement")
        achievement_name = "Ate 40 Hot Dogs in 10 Minutes"
        CONTRACT.push_action(
            "addach",
            {
                "ecosystem_owner": STUDIOA,
                "ecosystem_id": 0,
                "category_id": 0,
                "achievement_name": achievement_name,
                "description": "This is the achievement description!",
                "assetname": "hotdog.png",
                "maxquantity": 999
            },
            permission=(STUDIOA, Permission.ACTIVE)
        )

        table = CONTRACT.table("ecosystems", CONTRACT)
        self.assertEqual(table.json["rows"][0]["categories"][0]["achievements"][0]["name"], achievement_name)


    def test_0301_account_create_achievement_with_blank_description(self):
        COMMENT("Should allow an WAX account to create an Achievement w/blank description")
        achievement_name = "Ate 5 Blueberry Pies in 2 Minutes"
        CONTRACT.push_action(
            "addach",
            {
                "ecosystem_owner": STUDIOA,
                "ecosystem_id": 0,
                "category_id": 0,
                "achievement_name": achievement_name,
                "description": "",
                "assetname": "blueberry_pie.png",
                "maxquantity": 999
            },
            permission=(STUDIOA, Permission.ACTIVE)
        )

        table = CONTRACT.table("ecosystems", CONTRACT)
        self.assertEqual(table.json["rows"][0]["categories"][0]["achievements"][1]["name"], achievement_name)


    def test_0310_account_create_another_achievement(self):
        COMMENT("Should allow an WAX account to create an Achievement")
        achievement_name = "Slept For 16 Hours"
        CONTRACT.push_action(
            "addach",
            {
                "ecosystem_owner": STUDIOA,
                "ecosystem_id": 0,
                "category_id": 0,
                "achievement_name": achievement_name,
                "description": "",
                "assetname": "sleepy.png",
                "maxquantity": 999
            },
            permission=(STUDIOA, Permission.ACTIVE)
        )

        table = CONTRACT.table("ecosystems", CONTRACT)
        self.assertEqual(table.json["rows"][0]["categories"][0]["achievements"][2]["name"], achievement_name)


    def test_0320_account_create_limited_achievement(self):
        COMMENT("Should allow an WAX account to create a limited Achievement")
        achievement_name = "First to Respond"
        CONTRACT.push_action(
            "addach",
            {
                "ecosystem_owner": STUDIOA,
                "ecosystem_id": 0,
                "category_id": 0,
                "achievement_name": achievement_name,
                "description": "",
                "assetname": "first.png",
                "maxquantity": 1
            },
            permission=(STUDIOA, Permission.ACTIVE)
        )

        table = CONTRACT.table("ecosystems", CONTRACT)
        self.assertEqual(table.json["rows"][0]["categories"][0]["achievements"][3]["name"], achievement_name)


    def test_0400_account_create_user(self):
        COMMENT("Should allow an WAX account to create a User")
        user_name = "Bob"
        CONTRACT.push_action(
            "adduser",
            {
                "ecosystem_owner": STUDIOA,
                "ecosystem_id": 0,
                "user_name": user_name,
                "userid": "123abc",
                "avatarurl": "bob.png"
            },
            permission=(STUDIOA, Permission.ACTIVE)
        )

        table = CONTRACT.table("ecosystems", CONTRACT)
        self.assertEqual(table.json["rows"][0]["users"][0]["name"], user_name)


    def test_0401_account_create_another_user(self):
        COMMENT("Should allow an WAX account to create a User")
        user_name = "Zeke"
        CONTRACT.push_action(
            "adduser",
            {
                "ecosystem_owner": STUDIOA,
                "ecosystem_id": 0,
                "user_name": user_name,
                "userid": "34563543",
                "avatarurl": "34563543.png"
            },
            permission=(STUDIOA, Permission.ACTIVE)
        )

        table = CONTRACT.table("ecosystems", CONTRACT)
        self.assertEqual(table.json["rows"][0]["users"][1]["name"], user_name)


    def test_0600_account_grant_achievement(self):
        COMMENT("Should allow an WAX account to grant a User an Achievement")
        CONTRACT.push_action(
            "grantach",
            {
                "ecosystem_owner": STUDIOA,
                "ecosystem_id": 0,
                "user_id": 0,
                "category_id": 0,
                "achievement_id": 0,
                "timestamp": round(time.time())
            },
            permission=(STUDIOA, Permission.ACTIVE)
        )

        table = CONTRACT.table("ecosystems", CONTRACT)
        self.assertEqual(table.json["rows"][0]["users"][0]["userachievements"][0]["achievement_id"], 0)


    def test_0601_account_grant_same_achievement_fails(self):
        COMMENT("Should not allow a WAX account to grant a User the same Achievement twice")

        user_id = 0
        achievement_id = 0

        table = CONTRACT.table("ecosystems", CONTRACT)
        self.assertEqual(table.json["rows"][0]["users"][user_id]["userachievements"][0]["achievement_id"], achievement_id)

        with self.assertRaises(Exception) as e:
            CONTRACT.push_action(
                "grantach",
                {
                    "ecosystem_owner": STUDIOA,
                    "ecosystem_id": 0,
                    "user_id": user_id,
                    "category_id": 0,
                    "achievement_id": achievement_id,
                    "timestamp": round(time.time())
                },
                permission=(STUDIOA, Permission.ACTIVE),
                force_unique=True
            )

        err_msg = str(e.exception)
        self.assertTrue("Achievement already granted to this User" in err_msg, err_msg)


    def test_0610_account_grant_another_achievement(self):
        COMMENT("Should allow an WAX account to grant a User an Achievement")
        CONTRACT.push_action(
            "grantach",
            {
                "ecosystem_owner": STUDIOA,
                "ecosystem_id": 0,
                "user_id": 0,
                "category_id": 0,
                "achievement_id": 1,
                "timestamp": round(time.time())
            },
            permission=(STUDIOA, Permission.ACTIVE)
        )

        table = CONTRACT.table("ecosystems", CONTRACT)
        self.assertEqual(table.json["rows"][0]["users"][0]["userachievements"][1]["achievement_id"], 1)


    def test_0620_account_grant_limited_achievement(self):
        COMMENT("Should allow an WAX account to grant a User an Achievement")
        CONTRACT.push_action(
            "grantach",
            {
                "ecosystem_owner": STUDIOA,
                "ecosystem_id": 0,
                "user_id": 0,
                "category_id": 0,
                "achievement_id": 3,
                "timestamp": round(time.time())
            },
            permission=(STUDIOA, Permission.ACTIVE)
        )

        table = CONTRACT.table("ecosystems", CONTRACT)
        self.assertEqual(table.json["rows"][0]["users"][0]["userachievements"][2]["achievement_id"], 3)


    def test_0621_account_grant_limited_achievement_fails_at_max(self):
        COMMENT("Should not allow a WAX account to grant a User an Achievement already at maxquantity")

        table = CONTRACT.table("ecosystems", CONTRACT)
        self.assertEqual(table.json["rows"][0]["categories"][0]["achievements"][3]["maxquantity"], 1)
        self.assertEqual(len(table.json["rows"][0]["categories"][0]["achievements"][3]["usergrants"]), 1)

        with self.assertRaises(Exception) as e:
            CONTRACT.push_action(
                "grantach",
                {
                    "ecosystem_owner": STUDIOA,
                    "ecosystem_id": 0,
                    "user_id": 1,
                    "category_id": 0,
                    "achievement_id": 3,
                    "timestamp": round(time.time())
                },
                permission=(STUDIOA, Permission.ACTIVE)
            )

        err_msg = str(e.exception)
        self.assertTrue("Achievement max quantity" in err_msg, err_msg)


    def test_0700_edit_achievement(self):
        COMMENT("Should allow an WAX account to edit an Achievement that hasn't been granted yet")
        new_achievement_name = "Some new achievement"
        achievement_id = 2
        CONTRACT.push_action(
            "editach",
            {
                "ecosystem_owner": STUDIOA,
                "ecosystem_id": 0,
                "category_id": 0,
                "achievement_id": achievement_id,
                "achievement_name": new_achievement_name,
                "description": "",
                "assetname": "blueberry_pie.png",
                "maxquantity": 999
            },
            permission=(STUDIOA, Permission.ACTIVE)
        )

        table = CONTRACT.table("ecosystems", CONTRACT)
        self.assertEqual(table.json["rows"][0]["categories"][0]["achievements"][achievement_id]["name"], new_achievement_name)


    def test_0710_edit_granted_achievement_fails(self):
        COMMENT("Should prevent an WAX account from editing an Achievement that has been granted at least once")
        achievement_id = 0
        new_achievement_name = "New name that will fail"

        table = CONTRACT.table("ecosystems", CONTRACT)
        self.assertTrue(len(table.json["rows"][0]["categories"][0]["achievements"][achievement_id]["usergrants"]) > 0)

        with self.assertRaises(Exception) as e:
            CONTRACT.push_action(
                "editach",
                {
                    "ecosystem_owner": STUDIOA,
                    "ecosystem_id": 0,
                    "category_id": 0,
                    "achievement_id": achievement_id,
                    "achievement_name": new_achievement_name,
                    "description": "",
                    "assetname": "blueberry_pie.png",
                    "maxquantity": 999
                },
                permission=(STUDIOA, Permission.ACTIVE)
            )

        err_msg = str(e.exception)
        self.assertTrue("Cannot edit" in err_msg, err_msg)

        self.assertTrue(table.json["rows"][0]["categories"][0]["achievements"][achievement_id]["name"] != new_achievement_name)


    def test_0750_rmecosys_with_granted_achievement_fails(self):
        COMMENT("Should prevent an WAX account from deleting an Ecosystem that has an Achievement that has been granted at least once")
        ecosystem_id = 0
        achievement_id = 0

        table = CONTRACT.table("ecosystems", CONTRACT)
        ecosystem_name = table.json["rows"][ecosystem_id]["name"]
        self.assertTrue(len(table.json["rows"][ecosystem_id]["categories"]) == 1)
        self.assertTrue(len(table.json["rows"][ecosystem_id]["categories"][0]["achievements"][achievement_id]["usergrants"]) > 0)

        with self.assertRaises(Exception) as e:
            CONTRACT.push_action(
                "rmecosys",
                {
                    "ecosystem_owner": STUDIOA,
                    "ecosystem_id": ecosystem_id,
                },
                permission=(STUDIOA, Permission.ACTIVE)
            )

        err_msg = str(e.exception)
        self.assertTrue("Cannot remove an Ecosystem with granted Achievements" in err_msg, err_msg)

        table = CONTRACT.table("ecosystems", CONTRACT)
        self.assertEqual(table.json["rows"][ecosystem_id]["name"], ecosystem_name)


    def test_0760_rmlastcat_with_granted_achievement_fails(self):
        COMMENT("Should prevent an WAX account from deleting the last Category that has an Achievement that has been granted at least once")
        ecosystem_id = 0
        achievement_id = 0

        table = CONTRACT.table("ecosystems", CONTRACT)
        self.assertTrue(len(table.json["rows"][ecosystem_id]["categories"]) == 1)
        self.assertTrue(len(table.json["rows"][ecosystem_id]["categories"][0]["achievements"][achievement_id]["usergrants"]) > 0)

        with self.assertRaises(Exception) as e:
            CONTRACT.push_action(
                "rmlastcat",
                {
                    "ecosystem_owner": STUDIOA,
                    "ecosystem_id": ecosystem_id,
                },
                permission=(STUDIOA, Permission.ACTIVE)
            )

        err_msg = str(e.exception)
        self.assertTrue("Cannot remove a Category with granted Achievements" in err_msg, err_msg)

        table = CONTRACT.table("ecosystems", CONTRACT)
        self.assertTrue(len(table.json["rows"][ecosystem_id]["categories"]) == 1)


    def test_0770_rmlastcat_delete_category(self):
        COMMENT("Should allow a WAX account to delete the last Category if it has no Achievements or none that have been granted")
        ecosystem_id = 0

        table = CONTRACT.table("ecosystems", CONTRACT)
        self.assertTrue(len(table.json["rows"][ecosystem_id]["categories"]) == 1)
        initial_category_name = table.json["rows"][ecosystem_id]["categories"][0]["name"]

        # First delete a newly-added empty Category
        category_name = "Second Category"
        CONTRACT.push_action(
            "addcat",
            {
                "ecosystem_owner": STUDIOA,
                "ecosystem_id": ecosystem_id,
                "category_name": category_name
            },
            permission=(STUDIOA, Permission.ACTIVE)
        )

        table = CONTRACT.table("ecosystems", CONTRACT)
        self.assertTrue(len(table.json["rows"][ecosystem_id]["categories"]) == 2)

        CONTRACT.push_action(
            "rmlastcat",
            {
                "ecosystem_owner": STUDIOA,
                "ecosystem_id": ecosystem_id,
            },
            permission=(STUDIOA, Permission.ACTIVE),
            force_unique=True
        )

        table = CONTRACT.table("ecosystems", CONTRACT)
        self.assertTrue(len(table.json["rows"][ecosystem_id]["categories"]) == 1)

        # Now delete a Category that has an Achievement that isn't granted
        category_name = "Another Category"
        CONTRACT.push_action(
            "addcat",
            {
                "ecosystem_owner": STUDIOA,
                "ecosystem_id": 0,
                "category_name": category_name
            },
            permission=(STUDIOA, Permission.ACTIVE)
        )

        table = CONTRACT.table("ecosystems", CONTRACT)
        self.assertTrue(len(table.json["rows"][ecosystem_id]["categories"]) == 2)

        achievement_name = "Some amazing achievement"
        CONTRACT.push_action(
            "addach",
            {
                "ecosystem_owner": STUDIOA,
                "ecosystem_id": ecosystem_id,
                "category_id": 1,
                "achievement_name": achievement_name,
                "description": "This is the achievement description!",
                "assetname": "hotdog.png",
                "maxquantity": 999
            },
            permission=(STUDIOA, Permission.ACTIVE)
        )

        table = CONTRACT.table("ecosystems", CONTRACT)
        self.assertTrue(len(table.json["rows"][ecosystem_id]["categories"][1]["achievements"]) == 1)

        # Can still delete it
        CONTRACT.push_action(
            "rmlastcat",
            {
                "ecosystem_owner": STUDIOA,
                "ecosystem_id": ecosystem_id,
            },
            permission=(STUDIOA, Permission.ACTIVE),
            force_unique=True
        )

        table = CONTRACT.table("ecosystems", CONTRACT)
        self.assertTrue(len(table.json["rows"][ecosystem_id]["categories"]) == 1)
        self.assertEqual(table.json["rows"][ecosystem_id]["categories"][0]["name"], initial_category_name)


    def test_0780_rmlastach_with_granted_achievement_fails(self):
        COMMENT("Should not allow a WAX account to delete an Achievement that has been granted to a User")

        ecosystem_id = 0
        category_id = 0
        last_achievement_id = 3

        table = CONTRACT.table("ecosystems", CONTRACT)
        self.assertEqual(len(table.json["rows"][ecosystem_id]["categories"][category_id]["achievements"]), last_achievement_id + 1)
        self.assertTrue(len(table.json["rows"][ecosystem_id]["categories"][category_id]["achievements"][last_achievement_id]["usergrants"]) > 0)

        with self.assertRaises(Exception) as e:
            CONTRACT.push_action(
                "rmlastach",
                {
                    "ecosystem_owner": STUDIOA,
                    "ecosystem_id": ecosystem_id,
                    "category_id": ecosystem_id,
                },
                permission=(STUDIOA, Permission.ACTIVE)
            )

        err_msg = str(e.exception)
        self.assertTrue("Cannot delete an Achievement that has already been granted to a User" in err_msg, err_msg)


    def test_0790_rmlastach_delete_achievement(self):
        COMMENT("Should allow a WAX account to delete an Achievement that has not been granted to a User")

        ecosystem_id = 0
        category_id = 0

        table = CONTRACT.table("ecosystems", CONTRACT)
        num_achievements = len(table.json["rows"][ecosystem_id]["categories"][category_id]["achievements"])

        achievement_name = "Achievement to delete"
        CONTRACT.push_action(
            "addach",
            {
                "ecosystem_owner": STUDIOA,
                "ecosystem_id": 0,
                "category_id": 0,
                "achievement_name": achievement_name,
                "description": "",
                "assetname": "deleteme.png",
                "maxquantity": 999
            },
            permission=(STUDIOA, Permission.ACTIVE)
        )

        table = CONTRACT.table("ecosystems", CONTRACT)
        new_num_achievements = len(table.json["rows"][ecosystem_id]["categories"][category_id]["achievements"])
        self.assertEqual(num_achievements, new_num_achievements - 1)

        CONTRACT.push_action(
            "rmlastach",
            {
                "ecosystem_owner": STUDIOA,
                "ecosystem_id": ecosystem_id,
                "category_id": ecosystem_id,
            },
            permission=(STUDIOA, Permission.ACTIVE)
        )

        table = CONTRACT.table("ecosystems", CONTRACT)
        new_num_achievements = len(table.json["rows"][ecosystem_id]["categories"][category_id]["achievements"])
        self.assertEqual(num_achievements, new_num_achievements)


    def test_0800_approve_claim(self):
        COMMENT("Should allow the ecosystem owner to approve a User claim")
        user_id = 0
        CONTRACT.push_action(
            "approveclaim",
            {
                "ecosystem_owner": STUDIOA,
                "ecosystem_id": 0,
                "user_id": user_id,
                "user_account": BOB
            },
            permission=(STUDIOA, Permission.ACTIVE)
        )

        table = CONTRACT.table("ecosystems", CONTRACT)
        self.assertEqual(table.json["rows"][0]["users"][user_id]["account"], BOB.name)


    def test_0810_reapprove_claim_fails(self):
        COMMENT("Should not allow the ecosystem owner to approve a new claim on an already-approved User")
        user_id = 0

        with self.assertRaises(Exception) as e:
            CONTRACT.push_action(
                "approveclaim",
                {
                    "ecosystem_owner": STUDIOA,
                    "ecosystem_id": 0,
                    "user_id": user_id,
                    "user_account": CAROL
                },
                permission=(STUDIOA, Permission.ACTIVE)
            )

        err_msg = str(e.exception)
        self.assertTrue("User has already been claimed" in err_msg, err_msg)


    def test_0850_claim_user(self):
        COMMENT("Should allow a WAX account to claim its approved User")
        user_id = 0
        CONTRACT.push_action(
            "claimuser",
            {
                "ecosystem_owner": STUDIOA,
                "ecosystem_id": 0,
                "user_id": user_id,
                "user_account": BOB
            },
            permission=(BOB, Permission.ACTIVE)
        )

        table = CONTRACT.table("mywaxbadges", CONTRACT)

        self.assertEqual(table.json["rows"][0]["account"], BOB.name)
        self.assertEqual(table.json["rows"][0]["ecosystem_id"], 0)
        self.assertEqual(table.json["rows"][0]["user_id"], user_id)


    def test_0851_duplicate_claim_user_fails(self):
        COMMENT("Should not allow an WAX account to claim its approved User a second time")
        user_id = 0

        with self.assertRaises(Exception) as e:
            CONTRACT.push_action(
                "claimuser",
                {
                    "ecosystem_owner": STUDIOA,
                    "ecosystem_id": 0,
                    "user_id": user_id,
                    "user_account": BOB
                },
                permission=(BOB, Permission.ACTIVE),
                force_unique=True
            )

        err_msg = str(e.exception)
        self.assertTrue("Already claimed this User" in err_msg, err_msg)


    def test_0860_unapproved_account_claim_user_fails(self):
        COMMENT("Should not allow an WAX account to claim a User it hasn't been approved to claim")
        user_id = 0

        with self.assertRaises(Exception) as e:
            CONTRACT.push_action(
                "claimuser",
                {
                    "ecosystem_owner": STUDIOA,
                    "ecosystem_id": 0,
                    "user_id": user_id,
                    "user_account": CAROL
                },
                permission=(CAROL, Permission.ACTIVE)
            )

        err_msg = str(e.exception)
        self.assertTrue("User claim has not been approved" in err_msg, err_msg)


    def test_0870_alt_account_claim_user_fails(self):
        COMMENT("Should not allow an WAX account to claim a User on behalf of an approved account")
        user_id = 0

        with self.assertRaises(Exception) as e:
            CONTRACT.push_action(
                "claimuser",
                {
                    "ecosystem_owner": STUDIOA,
                    "ecosystem_id": 0,
                    "user_id": user_id,
                    "user_account": BOB     # BOB's claim has been approved...
                },
                permission=(CAROL, Permission.ACTIVE)   # ...but it's submitted by CAROL
            )

        err_msg = str(e.exception)
        self.assertTrue("Not authorized" in err_msg, err_msg)


    @classmethod
    def tearDownClass(cls):
        stop()


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="""
        This is a unit test for the 'waxbadges' smart contract.
    """)

    parser.add_argument(
        "-s", "--skip_recompile",
        action="store_true",
        help="Don't recompile the contract")

    args = parser.parse_args()

    skip_recompile = args.skip_recompile

    unittest.main(argv=[sys.argv[0]])

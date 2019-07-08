import argparse
import sys
import time
import unittest

from eosfactory.eosf import *


verbosity([Verbosity.INFO, Verbosity.OUT, Verbosity.TRACE, Verbosity.DEBUG])

CONTRACT_WORKSPACE = "achieveos"

# Actors of the test:
MASTER = MasterAccount()
CONTRACT = Account()
STUDIOA = Account()
STUDIOB = Account()
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
        create_account("BOB", MASTER)


    def test_0001_deploy(self):
        COMMENT("Create, build and deploy the contract:")
        create_account("CONTRACT", MASTER)
        smart = Contract(CONTRACT, CONTRACT_WORKSPACE)
        if not skip_recompile:
            smart.build()
        smart.deploy()


    def test_0100_account_create_org(self):
        COMMENT("Should allow an EOS account to create an Organization")
        organization_name = "Tournament of Champions"
        STUDIOA.info()
        CONTRACT.push_action(
            "addorg",
            {
                "org_owner": STUDIOA,
                "organization_name": organization_name,
                "assetbaseurl": "somedomainname.com/imgs/trophies"
            },
            permission=(STUDIOA, Permission.ACTIVE)
        )

        table = CONTRACT.table("orgs", STUDIOA)
        self.assertEqual(table.json["rows"][0]["name"], organization_name)
        STUDIOA.info()


    def test_0101_contract_create_org(self):
        COMMENT("Should allow the contract owner to create an Organization on behalf of org_owner")
        organization_name = "Some Other STUDIOA Org"
        CONTRACT.push_action(
            "addorg",
            {
                "org_owner": STUDIOA,
                "organization_name": organization_name,
                "assetbaseurl": "someotherdomainname.com/assets"
            },
            permission=(CONTRACT, Permission.ACTIVE)
        )

        table = CONTRACT.table("orgs", STUDIOA)
        self.assertEqual(table.json["rows"][1]["name"], organization_name)


    def test_0102_org_unique_name(self):
        COMMENT("Should throw an exception when creating an Organization with an existing name")
        organization_name = "Tournament of Champions"
        with self.assertRaises(Exception) as e:
            CONTRACT.push_action(
                "addorg",
                {
                    "org_owner": STUDIOA,
                    "organization_name": organization_name,
                    "assetbaseurl": "someotherdomainname.com/assets"
                },
                permission=(STUDIOA, Permission.ACTIVE)
            )

        err_msg = str(e.exception)
        self.assertTrue("not unique" in err_msg, err_msg)


    def test_0110_other_create_org(self):
        COMMENT("Should not allow a different account to create an Organization on behalf of org_owner")
        organization_name = "Not Allowed Org"
        with self.assertRaises(Exception) as e:
            CONTRACT.push_action(
                "addorg",
                {
                    "org_owner": STUDIOA,
                    "organization_name": organization_name,
                    "assetbaseurl": "someotherdomainname.com/assets"
                },
                permission=(BOB, Permission.ACTIVE)
            )

        err_msg = str(e.exception)
        self.assertTrue("Not authorized" in err_msg, err_msg)


    def test_0120_account_create_separate_org(self):
        COMMENT("Should allow a different EOS account to create its own isolated Organization")
        organization_name = "A Totally Different Org"
        CONTRACT.push_action(
            "addorg",
            {
                "org_owner": STUDIOB,
                "organization_name": organization_name,
                "assetbaseurl": "yetanotherdomainname.com/static"
            },
            permission=(STUDIOB, Permission.ACTIVE)
        )

        # Because of the table scope, STUDIOB's Org will also have index 0
        table = CONTRACT.table("orgs", STUDIOB)
        self.assertEqual(table.json["rows"][0]["name"], organization_name)

        # While STUDIOA still has her Orgs in her own table scope
        table = CONTRACT.table("orgs", STUDIOA)
        self.assertTrue(table.json["rows"][0]["name"] != organization_name)



    def test_0200_account_create_category(self):
        COMMENT("Should allow an EOS account to create a Category in its Organization")
        category_name = "First Category"
        CONTRACT.push_action(
            "addcat",
            {
                "org_owner": STUDIOA,
                "organization_id": 0,
                "category_name": category_name
            },
            permission=(STUDIOA, Permission.ACTIVE)
        )

        table = CONTRACT.table("orgs", STUDIOA)
        self.assertEqual(table.json["rows"][0]["categories"][0]["name"], category_name)



    def test_0300_account_create_achievement(self):
        COMMENT("Should allow an EOS account to create an Achievement")
        achievement_name = "Ate 40 Hot Dogs in 10 Minutes"
        CONTRACT.push_action(
            "addach",
            {
                "org_owner": STUDIOA,
                "organization_id": 0,
                "category_id": 0,
                "achievement_name": achievement_name,
                "description": "",
                "assetname": "hotdog.png"
            },
            permission=(STUDIOA, Permission.ACTIVE)
        )

        table = CONTRACT.table("orgs", STUDIOA)
        self.assertEqual(table.json["rows"][0]["categories"][0]["achievements"][0]["name"], achievement_name)


    def test_0310_account_create_another_achievement(self):
        COMMENT("Should allow an EOS account to create an Achievement")
        achievement_name = "Slept For 16 Hours"
        CONTRACT.push_action(
            "addach",
            {
                "org_owner": STUDIOA,
                "organization_id": 0,
                "category_id": 0,
                "achievement_name": achievement_name,
                "description": "",
                "assetname": "sleepy.png"
            },
            permission=(STUDIOA, Permission.ACTIVE)
        )

        table = CONTRACT.table("orgs", STUDIOA)
        self.assertEqual(table.json["rows"][0]["categories"][0]["achievements"][1]["name"], achievement_name)



    def test_0400_account_create_user(self):
        COMMENT("Should allow an EOS account to create a User")
        user_name = "Josh U"
        CONTRACT.push_action(
            "adduser",
            {
                "org_owner": STUDIOA,
                "organization_id": 0,
                "user_name": user_name,
                "userid": "123abc"
            },
            permission=(STUDIOA, Permission.ACTIVE)
        )

        table = CONTRACT.table("orgs", STUDIOA)
        self.assertEqual(table.json["rows"][0]["users"][0]["name"], user_name)


    def test_0600_account_grant_achievement(self):
        COMMENT("Should allow an EOS account to grant a User an Achievement")
        CONTRACT.push_action(
            "grantach",
            {
                "org_owner": STUDIOA,
                "organization_id": 0,
                "user_id": 0,
                "category_id": 0,
                "achievement_id": 0,
                "timestamp": round(time.time())
            },
            permission=(STUDIOA, Permission.ACTIVE)
        )

        table = CONTRACT.table("orgs", STUDIOA)
        self.assertEqual(table.json["rows"][0]["users"][0]["bycategory"][0]["value"]["userachievements"][0]["achievement_id"], 0)


    def test_0610_account_grant_another_achievement(self):
        COMMENT("Should allow an EOS account to grant a User an Achievement")
        CONTRACT.push_action(
            "grantach",
            {
                "org_owner": STUDIOA,
                "organization_id": 0,
                "user_id": 0,
                "category_id": 0,
                "achievement_id": 1,
                "timestamp": round(time.time())
            },
            permission=(STUDIOA, Permission.ACTIVE)
        )

        table = CONTRACT.table("orgs", STUDIOA)
        self.assertEqual(table.json["rows"][0]["users"][0]["bycategory"][0]["value"]["userachievements"][1]["achievement_id"], 1)




    @classmethod
    def tearDownClass(cls):
        stop()


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="""
        This is a unit test for the achieveos smart contract.
    """)

    parser.add_argument(
        "-s", "--skip_recompile",
        action="store_true",
        help="Don't recompile the contract")

    args = parser.parse_args()

    skip_recompile = args.skip_recompile

    unittest.main(argv=[sys.argv[0]])

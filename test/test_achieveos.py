import argparse
import sys
import unittest

from eosfactory.eosf import *


verbosity([Verbosity.INFO, Verbosity.OUT, Verbosity.TRACE, Verbosity.DEBUG])

CONTRACT_WORKSPACE = "achieveos"

# Actors of the test:
MASTER = MasterAccount()
HOST = Account()
ALICE = Account()
CAROL = Account()
BOB = Account()

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
        create_account("ALICE", MASTER)
        create_account("CAROL", MASTER)
        create_account("BOB", MASTER)


    def test_001_deploy(self):
        COMMENT("Create, build and deploy the contract:")
        create_account("HOST", MASTER)
        smart = Contract(HOST, CONTRACT_WORKSPACE)
        if not skip_recompile:
            smart.build()
        smart.deploy()


    def test_100_account_create_org(self):
        COMMENT("Should allow an EOS account to create an Organization")
        organization_name = "Tournament of Champions"
        HOST.push_action(
            "addorg", {"org_owner": ALICE, "organization_name": organization_name},
            permission=(ALICE, Permission.ACTIVE),
            force_unique=True)

        table = HOST.table("orgs", ALICE)
        self.assertEqual(table.json["rows"][0]["organization_name"], organization_name)


    def test_100_contract_create_org(self):
        COMMENT("Should allow the contract owner to create an Organization on behalf of org_owner")
        organization_name = "Some Other Alice Org"
        HOST.push_action(
            "addorg", {"org_owner": ALICE, "organization_name": organization_name},
            permission=(HOST, Permission.ACTIVE),
            force_unique=True)

        table = HOST.table("orgs", ALICE)
        self.assertEqual(table.json["rows"][1]["organization_name"], organization_name)


    def test_110_other_create_org(self):
        COMMENT("Should not allow a different account to create an Organization on behalf of org_owner")
        organization_name = "Not Allowed Org"
        with self.assertRaises(Exception) as e:
            HOST.push_action(
                "addorg", {"org_owner": ALICE, "organization_name": organization_name},
                permission=(BOB, Permission.ACTIVE),
                force_unique=True)

        err_msg = str(e.exception)
        self.assertTrue("Not authorized" in err_msg, err_msg)


    def test_120_account_create_separate_org(self):
        COMMENT("Should allow a different EOS account to create its own isolated Organization")
        organization_name = "A Totally Different Org"
        HOST.push_action(
            "addorg", {"org_owner": BOB, "organization_name": organization_name},
            permission=(BOB, Permission.ACTIVE),
            force_unique=True)

        # Because of the table scope, Bob's Org will also have index 0
        table = HOST.table("orgs", BOB)
        self.assertEqual(table.json["rows"][0]["organization_name"], organization_name)

        # While Alice still has her Orgs in her own table scope
        table = HOST.table("orgs", ALICE)
        self.assertTrue(table.json["rows"][0]["organization_name"] != organization_name)


    def test_200_account_create_category(self):
        COMMENT("Should allow an EOS account to create a Category in its Organization")
        category_name = "First Category"
        HOST.push_action(
            "addcat", {"org_owner": ALICE, "organization_id": 1, "category_name": category_name},
            permission=(ALICE, Permission.ACTIVE),
            force_unique=True)

        table = HOST.table("categories", ALICE)
        self.assertEqual(table.json["rows"][0]["category_name"], category_name)
        self.assertEqual(table.json["rows"][0]["key"], 1)


    def test_300_account_create_achievement(self):
        COMMENT("Should allow an EOS account to create an Achievement")
        achievement_name = "Ate 40 Hot Dogs in 10 Minutes"
        HOST.push_action(
            "addach", {"org_owner": ALICE, "organization_id": 1, "category_id": 1, "achievement_name": achievement_name},
            permission=(ALICE, Permission.ACTIVE),
            force_unique=True)

        table = HOST.table("achievements", ALICE)
        self.assertEqual(table.json["rows"][0]["achievement_name"], achievement_name)
        self.assertEqual(table.json["rows"][0]["key"], 1)


    def test_310_account_create_another_achievement(self):
        COMMENT("Should allow an EOS account to create an Achievement")
        achievement_name = "Slept For 16 Hours"
        HOST.push_action(
            "addach", {"org_owner": ALICE, "organization_id": 1, "category_id": 1, "achievement_name": achievement_name},
            permission=(ALICE, Permission.ACTIVE),
            force_unique=True)

        table = HOST.table("achievements", ALICE)
        self.assertEqual(table.json["rows"][1]["achievement_name"], achievement_name)
        self.assertEqual(table.json["rows"][1]["key"], 2)


    def test_400_account_create_user(self):
        COMMENT("Should allow an EOS account to create a User")
        user_name = "Josh U"
        HOST.push_action(
            "adduser", {"org_owner": ALICE, "organization_id": 1, "user_name": user_name},
            permission=(ALICE, Permission.ACTIVE),
            force_unique=True)

        table = HOST.table("users", ALICE)
        self.assertEqual(table.json["rows"][0]["user_name"], user_name)
        self.assertEqual(table.json["rows"][0]["key"], 1)


    def test_500_account_create_grantor(self):
        COMMENT("Should allow an EOS account to create a Grantor")
        grantor_name = "Coach Banzai"
        HOST.push_action(
            "addgrantor", {"org_owner": ALICE, "organization_id": 1, "grantor_name": grantor_name},
            permission=(ALICE, Permission.ACTIVE),
            force_unique=True)

        table = HOST.table("grantors", ALICE)
        self.assertEqual(table.json["rows"][0]["grantor_name"], grantor_name)
        self.assertEqual(table.json["rows"][0]["key"], 1)


    def test_600_account_grant_achievement(self):
        COMMENT("Should allow an EOS account to grant a User an Achievement")
        HOST.push_action(
            "grantach", {"org_owner": ALICE, "organization_id": 1, "user_id": 1, "achievement_id": 1, "grantor_id": 1},
            permission=(ALICE, Permission.ACTIVE),
            force_unique=True)

        table = HOST.table("users", ALICE)
        # self.assertEqual(table.json["rows"][1]["user_name"], user_name)


    def test_610_account_grant_another_achievement(self):
        COMMENT("Should allow an EOS account to grant a User an Achievement")
        HOST.push_action(
            "grantach", {"org_owner": ALICE, "organization_id": 1, "user_id": 1, "achievement_id": 2, "grantor_id": 1},
            permission=(ALICE, Permission.ACTIVE),
            force_unique=True)

        table = HOST.table("users", ALICE)




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
